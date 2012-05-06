#include "pch.h"
#include "game.h"
#include "unittest.h"
#include "joepack.h"
#include "matrix4.h"
#include "configfile.h"
#include "cardefs.h"
#include <math.h>

#include "numprocessors.h"
#include "parallel_task.h"
#include "performance_testing.h"
#include "quickprof.h"
#include "tracksurface.h"
#include "forcefeedback.h"
#include "../ogre/common/Defines.h"
#include "../ogre/common/QTimer.h"
#include "../ogre/OgreGame.h"
#include "../ogre/FollowCamera.h"
#include "../oisb/OISBSystem.h"


#define M_PI  3.14159265358979323846

using std::string;
using std::endl;
using std::pair;


///  ctor
GAME::GAME(std::ostream & info_out, std::ostream & err_out, SETTINGS* pSettings) :
	settings(pSettings), info_output(info_out), error_output(err_out),
	frame(0), displayframe(0), clocktime(0), target_time(0),
	//framerate(0.01f),  ///~  0.004+  o:0.01
	fps_track(10,0), fps_position(0), fps_min(0), fps_max(0),
	multithreaded(false), benchmode(false), dumpfps(false),
	pause(false), debugmode(false), profilingmode(false),
	particle_timer(0), race_laps(0),
	track(info_out, err_out), /*tracknode(NULL),*/
	framerate(1.0 / pSettings->game_fq)
{
	carcontrols_local.first = NULL;
	//  sim iv from settings
	collision.fixedTimestep = 1.0 / pSettings->blt_fq;
	collision.maxSubsteps = pSettings->blt_iter;
}


//  start the game with the given arguments
void GAME::Start(std::list <string> & args)
{
	if (!ParseArguments(args))
		return;

	info_output << "Starting VDrift-Ogre: 2010-05-01, O/S: ";
	#ifdef _WIN32
		info_output << "Windows" << endl;
	#elif defined(__APPLE__)
		info_output << "Apple" << endl;
	#else
		info_output << "Unix-like" << endl;
	#endif

	//settings->Load(PATHMANAGER::GetSettingsFile());
	
	carcontrols_local.second.Reset();
	
	InitializeSound(); //if sound initialization fails, that's okay, it'll disable itself

	//initialize GUI
	std::map<string, string> optionmap;
	LoadSaveOptions(LOAD, optionmap);

	//initialize force feedback
	#ifdef ENABLE_FORCE_FEEDBACK
	forcefeedback.reset(new FORCEFEEDBACK(settings->ff_device, error_output, info_output));
		ff_update_time = 0;
	#endif
}

bool GAME::InitializeSound()
{
	QTimer ti;  ti.update();  /// time
	
	if (sound.Init(/*2048*/1024/*512*/, info_output, error_output))
	{
		generic_sounds.SetLibraryPath(PATHMANAGER::GetGenericSoundPath());
		
		if (!generic_sounds.Load("tire_squeal", sound.GetDeviceInfo(), error_output))  return false;
		if (!generic_sounds.Load("grass", sound.GetDeviceInfo(), error_output))  return false;
		if (!generic_sounds.Load("gravel", sound.GetDeviceInfo(), error_output))  return false;
		if (!generic_sounds.Load("bump_front", sound.GetDeviceInfo(), error_output))  return false;
		if (!generic_sounds.Load("bump_rear", sound.GetDeviceInfo(), error_output))  return false;
		if (!generic_sounds.Load("wind", sound.GetDeviceInfo(), error_output))  return false;
		for (int i = 1; i <= Ncrashsounds; ++i)
			if (!generic_sounds.Load(toStr(i/10)+toStr(i%10), sound.GetDeviceInfo(), error_output))  return false;
		if (!generic_sounds.Load("scrap", sound.GetDeviceInfo(), error_output))  return false;
		if (!generic_sounds.Load("screech", sound.GetDeviceInfo(), error_output))  return false;

		for (int i = 0; i < Nwatersounds; ++i)
			if (!generic_sounds.Load("water"+toStr(i+1), sound.GetDeviceInfo(), error_output))  return false;

		if (!generic_sounds.Load("mud1", sound.GetDeviceInfo(), error_output))  return false;
		if (!generic_sounds.Load("mud_cont", sound.GetDeviceInfo(), error_output))  return false;
		if (!generic_sounds.Load("water_cont", sound.GetDeviceInfo(), error_output))  return false;
		if (!generic_sounds.Load("boost", sound.GetDeviceInfo(), error_output))  return false;
		
		sound.SetMasterVolume(settings->vol_master);
		sound.Pause(false);
	}
	else
	{
		error_output << "Sound initialization failed" << endl;
		return false;
	}

	info_output << "Sound initialization successful" << endl;

	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	info_output << "::: Time Sounds: " << dt << " ms" << endl;
	return true;
}


//  do any necessary cleanup
void GAME::End()
{
	if (benchmode)
	{
		float mean_fps = displayframe / clocktime;
		info_output << "Elapsed time: " << clocktime << " seconds\n";
		info_output << "Average frame-rate: " << mean_fps << " frames per second\n";
		info_output << "Min / Max frame-rate: " << fps_min << " / " << fps_max << " frames per second" << endl;
	}
	
	if (profilingmode)
		info_output << "Profiling summary:\n" << PROFILER.getSummary(quickprof::PERCENT) << endl;
	
	info_output << "Shutting down..." << endl;

	LeaveGame();

	if (sound.Enabled())
		sound.Pause(true); //stop the sound thread

	///+
	settings->Save(PATHMANAGER::GetSettingsFile()); //save settings first incase later deinits cause crashes

	collision.Clear();
	track.Clear();
}

void GAME::Test()
{
	QT_RUN_TESTS;

	info_output << endl;
}


///  the main game loop
//----------------------------------------------------------------------------------------------------------------------------
bool GAME::OneLoop(double dt)
{
	PROFILER.beginBlock(" oneLoop");

	clocktime += dt;  //only for stats
	
	//LogO(Ogre::String("Ld: dt ")+fToStr(dt,6,8));

	Tick(dt);  // do CPU intensive stuff in parallel with the GPU
	
	displayframe++;  //only for stats

	PROFILER.endBlock(" oneLoop");
	return true;
}

///  step game required amount of ticks
void GAME::Tick(double deltat)
{
	const float minfps = 10.0f; //this is the minimum fps the game will run at before it starts slowing down time
	const unsigned int maxticks = (int) (1.0f / (minfps * framerate));
	const float maxtime = 1.0/minfps;
	unsigned int curticks = 0;

	//  throw away wall clock time if necessary to keep the framerate above the minimum
	if (deltat > maxtime)
		deltat = maxtime;

	target_time += deltat;
	double tickperriod = TickPeriod();

	//  increment game logic by however many tick periods have passed since the last GAME::Tick
	while (target_time > tickperriod && curticks < maxticks)
	{
		frame++;
		AdvanceGameLogic(deltat > 0.f ? tickperriod : 0.f);

		if (pOgreGame)
			pOgreGame->newPoses(tickperriod);

		curticks++;
		target_time -= tickperriod;
	}
}

///  simulate game by one frame
//----------------------------------------------------------------------------------------------------------------------------
void GAME::AdvanceGameLogic(double dt)
{
	if (track.Loaded())
	{
		if (pause && carcontrols_local.first)
			sound.Pause(true);
		else
		{
			if (sound.Enabled())
				sound.Pause(false);
			
			//PROFILER.beginBlock("ai");
			//ai.Visualize(rootnode);
			//ai.update(TickPeriod(), &track, cars); //-
			//PROFILER.endBlock("ai");
			
			PROFILER.beginBlock("-physics");
			///~~  clear fluids for each car
			for (std::list <CAR>::iterator i = cars.begin(); i != cars.end(); ++i)
			{
				(*i).dynamics.inFluids.clear();
				(*i).dynamics.velPrev = (*i).dynamics.chassis->getLinearVelocity();
				for (int w=0; w < 4; ++w)
					(*i).dynamics.inFluidsWh[w].clear();
			}

			collision.Update(dt, settings->bltProfilerTxt);
			PROFILER.endBlock("-physics");
			
			PROFILER.beginBlock("-car-sim");
			///if (settings->multi_thr == 0)
			///	OISB::System::getSingleton().process(dt);  // input update  single thread

			int i = 0;
			for (std::list <CAR>::iterator it = cars.begin(); it != cars.end(); ++it, ++i)
				UpdateCar(*it, TickPeriod());
			PROFILER.endBlock("-car-sim");
			
			//PROFILER.beginBlock("timer");
			UpdateTimer();
			//PROFILER.endBlock("timer");
		}
	}

	UpdateForceFeedback(TickPeriod());
}


///  send inputs to the car, check for collisions, and so on
//-----------------------------------------------------------
void GAME::UpdateCar(CAR & car, double dt)
{
	car.Update(dt);
	UpdateCarInputs(car);
	UpdateDriftScore(car, dt);
}

void GAME::UpdateCarInputs(CAR & car)
{
    std::vector <float> carinputs(CARINPUT::ALL, 0.0f);
	bool forceBrake = timer.waiting || timer.pretime > 0.f;  // race countdown

    carinputs = carcontrols_local.second.ProcessInput(car.id, forceBrake);

	car.HandleInputs(carinputs, TickPeriod());
}


bool GAME::NewGameDoCleanup()
{
	LeaveGame(); //this should clear out all data
	return true;
}

bool GAME::NewGameDoLoadTrack()
{
	if (!LoadTrack(settings->game.track))
		error_output << "Error during track loading: " << settings->game.track << endl;

	return true;
}

bool GAME::NewGameDoLoadMisc(float pre_time)
{
    //race_laps = num_laps;
    ///-----
    race_laps = 0;

	opponents.clear();

	//send car sounds to the sound subsystem
	for (std::list <CAR>::iterator i = cars.begin(); i != cars.end(); ++i)
	{
		std::list <SOUNDSOURCE *> soundlist;
		i->GetSoundList(soundlist);
		for (std::list <SOUNDSOURCE *>::iterator s = soundlist.begin(); s != soundlist.end(); ++s)
			sound.AddSource(**s);
	}

	//load the timer
	if (!timer.Load(PATHMANAGER::GetTrackRecordsPath()+"/"+settings->game.track+".txt", pre_time, error_output))
		return false;

	//add cars to the timer system
	for (std::list <CAR>::iterator i = cars.begin(); i != cars.end(); ++i)
		timer.AddCar(i->GetCarType());
	timer.AddCar("ghost");

	return true;
}

///  clean up all game data
void GAME::LeaveGame()
{
	//ai.clear_cars();

	carcontrols_local.first = NULL;

	track.Unload();
	collision.Clear();

	if (sound.Enabled())
	{
		for (std::list <CAR>::iterator i = cars.begin(); i != cars.end(); ++i)
		{
			std::list <SOUNDSOURCE *> soundlist;
			i->GetSoundList(soundlist);
			for (std::list <SOUNDSOURCE *>::iterator s = soundlist.begin(); s != soundlist.end(); s++)
				sound.RemoveSource(*s);
		}
	}
	cars.clear();
	timer.Unload();
	pause = false;
}

///  add a car, optionally controlled by the local player
CAR* GAME::LoadCar(const string & carname, const MATHVECTOR <float, 3> & start_position,
		   const QUATERNION <float> & start_orientation, bool islocal, bool isai, bool isRemote, int idCar)
{
	CONFIGFILE carconf;
	if (!carconf.Load(PATHMANAGER::GetCarPath()+"/"+carname+"/"+carname+".car"))
		return NULL;
	
	cars.push_back(CAR());

	if (!cars.back().Load(pOgreGame, settings, 
		carconf, PATHMANAGER::GetCarPath(),
		PATHMANAGER::GetDriverPath()+"/driver2",
		carname,
		start_position, start_orientation,
		collision,
		sound.Enabled(),
		sound.GetDeviceInfo(),
		generic_sounds,
		settings->abs || isai,
		settings->tcs || isai,
		isRemote,
		idCar,
		debugmode, info_output, error_output))
	{
		error_output << "Error loading car: " << carname << endl;
		cars.pop_back();
		return NULL;
	}
	else
	{
		info_output << "Car loading was successful: " << carname << endl;

		if (islocal)
		{
			//load local controls
			carcontrols_local.first = &cars.back();

			//setup auto clutch and auto shift
			ProcessNewSettings();
			// shift into first gear if autoshift enabled
			if (carcontrols_local.first && settings->autoshift)
                carcontrols_local.first->SetGear(1);
		}
	}
	
	return &cars.back();
}

bool GAME::LoadTrack(const string & trackname)
{
	LoadingScreen(0.0,1.0);

	//load the track
	if (!track.DeferredLoad(
			(settings->game.track_user ? PATHMANAGER::GetTrackPathUser() : PATHMANAGER::GetTrackPath()) + "/" + trackname,
			settings->game.trackreverse,
			/**/0, "large", true, false))
	{
		error_output << "Error loading track: " << trackname << endl;
		return false;
	}
	bool success = true;
	int count = 0;
	while (!track.Loaded() && success)
	{
		int displayevery = track.DeferredLoadTotalObjects() / 50;
		if (displayevery == 0 || count % displayevery == 0)
		{
			LoadingScreen(count, track.DeferredLoadTotalObjects());
		}
		success = track.ContinueDeferredLoad();
		count++;
	}
	
	if (!success)
	{
		error_output << "Error loading track (deferred): " << trackname << endl;
		return false;
	}

	//setup track collision
	collision.SetTrack(&track);
	collision.DebugPrint(info_output);
	
	return true;
}

bool SortStringPairBySecond (const pair<string,string> & first, const pair<string,string> & second)
{
	return first.second < second.second;
}

void GAME::LoadSaveOptions(OPTION_ACTION action, std::map<string, string> & options)
{
	if (action == LOAD) //load from the settings class to the options map
	{
		CONFIGFILE tempconfig;
		settings->Serialize(true, tempconfig);
		std::list <string> paramlistoutput;
		tempconfig.GetParamList(paramlistoutput);
		for (std::list <string>::iterator i = paramlistoutput.begin(); i != paramlistoutput.end(); ++i)
		{
			string val;
			tempconfig.GetParam(*i, val);
			options[*i] = val;
			//std::cout << "LOAD - PARAM: " << *i << " = " << val << endl;
		}
	}
	else //save from the options map to the settings class
	{
		CONFIGFILE tempconfig;
		for (std::map<string, string>::iterator i = options.begin(); i != options.end(); ++i)
		{
			tempconfig.SetParam(i->first, i->second);
			//std::cout << "SAVE - PARAM: " << i->first << " = " << i->second << endl;
		}
		settings->Serialize(false, tempconfig);

		//account for new settings
		ProcessNewSettings();
	}
}

//  update the game with any new setting changes that have just been made
void GAME::ProcessNewSettings()
{
	if (carcontrols_local.first)
	{
		carcontrols_local.first->SetABS(settings->abs);
		carcontrols_local.first->SetTCS(settings->tcs);
		carcontrols_local.first->SetAutoShift(settings->autoshift);
		carcontrols_local.first->SetAutoRear(settings->autorear);
		//carcontrols_local.first->SetAutoClutch(settings->rear_inv);
	}

	sound.SetMasterVolume(settings->vol_master);
}

void GAME::LoadingScreen(float progress, float max)
{
	//assert(max > 0);
	//loadingscreen.Update(progress/(max+0.001f));	///+-

	//CollapseSceneToDrawlistmap(loadingscreen_node, graphics.GetDrawlistmap(), true);
}

void GAME::UpdateForceFeedback(float dt)
{
#ifdef ENABLE_FORCE_FEEDBACK
	if (carcontrols_local.first)
	{
		//static ofstream file("ff_output.txt");
		ff_update_time += dt;
		const double ffdt = 0.02;
		if (ff_update_time >= ffdt )
		{
			ff_update_time = 0.0;
			double feedback = -carcontrols_local.first->GetFeedback();

			feedback = settings->ff_gain * feedback / 100.0;
			if (settings->ff_invert) feedback = -feedback;

			if (feedback > 1.0)
				feedback = 1.0;
			if (feedback < -1.0)
				feedback = -1.0;
			//feedback += 0.5;
			/*
			static double motion_frequency = 0.1;
			static double motion_amplitude = 4.0;
			static double spring_strength = 1.0;
			*/
			//double center = sin( timefactor * 2 * M_PI * motion_frequency ) * motion_amplitude;
			double force = feedback;

			//std::cout << "ff_update_time: " << ff_update_time << " force: " << force << std::endl;
			forcefeedback->update(force, &feedback, ffdt, error_output);
		}
	}

	if (pause && dt == 0)
	{
		double pos=0;
		forcefeedback->update(0, &pos, 0.02, error_output);
	}
#endif
}

void GAME::UpdateDriftScore(CAR & car, double dt)
{
	bool is_drifting = false;
	bool spin_out = false;

	//make sure the car is not off track
	int wheel_count = 0;
	for (int i=0; i < 4; i++)
	{
		if ( car.GetCurPatch ( WHEEL_POSITION(i) ) ) wheel_count++;
	}

	bool on_track = ( wheel_count > 1 );
	int carId = 0;

	//car's direction on the horizontal plane
	MATHVECTOR <float, 3> car_orientation(1,0,0);
	car.GetOrientation().RotateVector(car_orientation);
	car_orientation[2] = 0;

	//car's velocity on the horizontal plane
	MATHVECTOR <float, 3> car_velocity = car.GetVelocity();
	car_velocity[2] = 0;
	float car_speed = car_velocity.Magnitude();

	//angle between car's direction and velocity
	float car_angle = 0;
	float mag = car_orientation.Magnitude() * car_velocity.Magnitude();
	if (mag > 0.001)
	{
		float dotprod = car_orientation.dot ( car_velocity )/mag;
		if (dotprod > 1.0)
			dotprod = 1.0;
		if (dotprod < -1.0)
			dotprod = -1.0;
		car_angle = acos(dotprod);
	}
	
	assert(car_angle == car_angle); //assert that car_angle isn't NAN

	if ( on_track )
	{
		//velocity must be above 10 m/s
		if ( car_speed > 10 )
		{
			//drift starts when the angle > 0.2 (around 11.5 degrees)
			//drift ends when the angle < 0.1 (aournd 5.7 degrees)
			float angle_threshold(0.2);
			if ( timer.GetIsDrifting(carId) ) angle_threshold = 0.1;

			is_drifting = ( car_angle > angle_threshold && car_angle <= M_PI/2.0 );
			spin_out = ( car_angle > M_PI/2.0 );
		}
	}

	//calculate score
	if ( is_drifting )
	{
		//base score is the drift distance
		timer.IncrementThisDriftScore(carId, dt * car_speed);

		//bonus score calculation is now done in TIMER
		timer.UpdateMaxDriftAngleSpeed(carId, car_angle, car_speed);
		
		//std::cout << timer.GetDriftScore(carId) << " + " << timer.GetThisDriftScore(carId) << endl;
	}

	if (settings->multi_thr != 1)
		timer.SetIsDrifting(carId, is_drifting, on_track && !spin_out);
	
	//std::cout << is_drifting << ", " << on_track << ", " << car_angle << endl;
}


//  break up the input into a vector of strings using the token characters given
std::vector <string> Tokenize(const string & input, const string & tokens)
{
	std::vector <string> out;
	
	unsigned int pos = 0;
	unsigned int lastpos = 0;
	
	while (pos != (unsigned int) string::npos)
	{
		pos = input.find_first_of(tokens, pos);
		string thisstr = input.substr(lastpos,pos-lastpos);
		if (!thisstr.empty())
			out.push_back(thisstr);
		pos = input.find_first_not_of(tokens, pos);
		lastpos = pos;
	}
	
	return out;
}

bool GAME::ParseArguments(std::list <string> & args)
{
	bool continue_game(true);
	
	std::map <string, string> arghelp;
	std::map <string, string> argmap;

	//generate an argument map
	for (std::list <string>::iterator i = args.begin(); i != args.end(); ++i)
	{
		if ((*i)[0] == '-')
		{
			argmap[*i] = "";
		}

		std::list <string>::iterator n = i;
		n++;
		if (n != args.end())
		{
			if ((*n)[0] != '-')
				argmap[*i] = *n;
		}
	}

	//check for arguments

	if (argmap.find("-test") != argmap.end())
	{
		Test();
		continue_game = false;
	}
	arghelp["-test"] = "Run unit tests.";
	
	if (argmap.find("-debug") != argmap.end())
	{
		debugmode = true;
	}
	///+
	//debugmode = true;
	arghelp["-debug"] = "Display car debugging information.";

	if (!argmap["-cartest"].empty())
	{
		PATHMANAGER::Init(info_output, error_output);
		PERFORMANCE_TESTING perftest;
		perftest.Test(PATHMANAGER::GetCarPath(), argmap["-cartest"], info_output, error_output);
		continue_game = false;
	}
	arghelp["-cartest CAR"] = "Run car performance testing on given CAR.";
	
	if (!argmap["-profile"].empty())
	{
		PATHMANAGER::SetProfile(argmap["-profile"]);
	}
	arghelp["-profile PROFILENAME"] = "Store settings, controls, and records under a separate profile.";
	
	///+
	//if (argmap.find("-profiling") != argmap.end() || argmap.find("-benchmark") != argmap.end())
	//if (settings->bltLines/*bltProfilerTxt*/)
	{
		PROFILER.init(20);
		profilingmode = true;
	}
	arghelp["-profiling"] = "Display game performance data.";
	
	if (argmap.find("-dumpfps") != argmap.end())
	{
		info_output << "Dumping the frame-rate to log." << endl;
		dumpfps = true;
	}
	arghelp["-dumpfps"] = "Continually dump the framerate to the log.";
	

	if (argmap.find("-nosound") != argmap.end())
		sound.DisableAllSound();
	arghelp["-nosound"] = "Disable all sound.";

	if (argmap.find("-benchmark") != argmap.end())
	{
		info_output << "Entering benchmark mode." << endl;
		benchmode = true;
	}
	arghelp["-benchmark"] = "Run in benchmark mode.";
	
	
	arghelp["-help"] = "Display command-line help.";
	if (argmap.find("-help") != argmap.end() || argmap.find("-h") != argmap.end() || argmap.find("--help") != argmap.end() || argmap.find("-?") != argmap.end())
	{
		string helpstr;
		unsigned int longest = 0;
		for (std::map <string,string>::iterator i = arghelp.begin(); i != arghelp.end(); ++i)
			if (i->first.size() > longest)
				longest = i->first.size();
		for (std::map <string,string>::iterator i = arghelp.begin(); i != arghelp.end(); ++i)
		{
			helpstr.append(i->first);
			for (unsigned int n = 0; n < longest+3-i->first.size(); n++)
				helpstr.push_back(' ');
			helpstr.append(i->second + "\n");
		}
		info_output << "Command-line help:\n\n" << helpstr << endl;
		continue_game = false;
	}

	return continue_game;
}


void GAME::UpdateTimer()
{
	//check for cars doing a lap - only on VDrift tracks (mostly unused)
	int carId=0;
	for (std::list <CAR>::iterator i = cars.begin(); i != cars.end(); ++i,++carId)
	{
		bool advance = false;
		int nextsector = 0;
		if (track.GetSectors() > 0)
		{
			nextsector = (i->GetSector() + 1) % track.GetSectors();
			//cout << "next " << nextsector << ", cur " << i->GetSector() << ", track " << track.GetSectors() << endl;
			for (int p = 0; p < 4; p++)
			{
				if (i->GetCurPatch(p) == track.GetLapSequence(nextsector))
				{
					advance = true;
					//cout << "Drove over new sector " << nextsector << " patch " << i->GetCurPatch(p) << endl;
					//cout << p << ". " << i->GetCurPatch(p) << ", " << track.GetLapSequence(nextsector) << endl;
				}
				//else cout << p << ". " << i->GetCurPatch(p) << ", " << track.GetLapSequence(nextsector) << endl;
			}
		}

		if (advance)
		{
		    // only count it if the car's current sector isn't -1
		    // which is the default value when the car is loaded
			timer.Lap(carId, i->GetSector(), nextsector, (i->GetSector() >= 0), settings->game.trackreverse); 
			i->SetSector(nextsector);
		}

		//update how far the car is on the track
		const BEZIER * curpatch = i->GetCurPatch(0); //find the patch under the front left wheel
		if (!curpatch)
            curpatch = i->GetCurPatch(1); //try the other wheel
        if (curpatch) //only update if car is on track
        {
            MATHVECTOR <float, 3> pos = i->GetCenterOfMassPosition();
            MATHVECTOR <float, 3> back_left, back_right, front_left;

            if (!track.IsReversed())
            {
                back_left = MATHVECTOR <float, 3> (curpatch->GetBL()[2], curpatch->GetBL()[0], curpatch->GetBL()[1]);
                back_right = MATHVECTOR <float, 3> (curpatch->GetBR()[2], curpatch->GetBR()[0], curpatch->GetBR()[1]);
                front_left = MATHVECTOR <float, 3> (curpatch->GetFL()[2], curpatch->GetFL()[0], curpatch->GetFL()[1]);
            }
            else
            {
                back_left = MATHVECTOR <float, 3> (curpatch->GetFL()[2], curpatch->GetFL()[0], curpatch->GetFL()[1]);
                back_right = MATHVECTOR <float, 3> (curpatch->GetFR()[2], curpatch->GetFR()[0], curpatch->GetFR()[1]);
                front_left = MATHVECTOR <float, 3> (curpatch->GetBL()[2], curpatch->GetBL()[0], curpatch->GetBL()[1]);
            }

            //float dist_from_back = (back_left - back_right).perp_distance (back_left, pos);

            MATHVECTOR <float, 3> forwardvec = front_left - back_left;
            MATHVECTOR <float, 3> relative_pos = pos - back_left;
            float dist_from_back = 0;
			
			if (forwardvec.Magnitude() > 0.0001)
				dist_from_back = relative_pos.dot(forwardvec.Normalize());

			timer.UpdateDistance(carId, curpatch->GetDistFromStart() + dist_from_back);
			//std::cout << curpatch->GetDistFromStart() << ", " << dist_from_back << endl;
			//std::cout << curpatch->GetDistFromStart() + dist_from_back << endl;
        }

		/*info_output << "sector=" << i->GetSector() << ", next=" << track.GetLapSequence(nextsector) << ", ";
		for (int w = 0; w < 4; w++)
		{
			info_output << w << "=" << i->GetCurPatch(w) << ", ";
		}
		info_output << endl;*/
	}

	timer.Tick(TickPeriod());
	//timer.DebugPrint(info_output);
}
