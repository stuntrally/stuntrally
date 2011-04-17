#include "stdafx.h"

#include "game.h"
#include "unittest.h"
#include "joepack.h"
#include "matrix4.h"
#include "configfile.h"
#include "cardefs.h"

#include "numprocessors.h"
#include "parallel_task.h"
#include "performance_testing.h"
#include "quickprof.h"
#include "tracksurface.h"
#include "../ogre/OgreGame.h"
#include "../ogre/FollowCamera.h"


///start the game with the given arguments
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
	
	eventsystem.Init(info_output);
	
	//load controls
	info_output << "Loading car controls from: " << PATHMANAGER::GetCarControlsFile() << endl;
	if (!PATHMANAGER::FileExists(PATHMANAGER::GetCarControlsFile()))
	{
		info_output << "Car control file " << PATHMANAGER::GetCarControlsFile() << " doesn't exist; using defaults" << endl;
		carcontrols_local.second.Load(PATHMANAGER::GetDefaultCarControlsFile(), info_output, error_output);
		carcontrols_local.second.Save(PATHMANAGER::GetCarControlsFile(), info_output, error_output);
	} else
		carcontrols_local.second.Load(PATHMANAGER::GetCarControlsFile(), info_output, error_output);

	InitializeSound(); //if sound initialization fails, that's okay, it'll disable itself

	//initialize GUI
	std::map<std::string, std::string> optionmap;
	LoadSaveOptions(LOAD, optionmap);
	//if (settings->mousegrab)  eventsystem.SetMouseCursorVisibility(true);

	//initialize force feedback
	#ifdef ENABLE_FORCE_FEEDBACK
	forcefeedback.reset(new FORCEFEEDBACK(settings->GetFFDevice(), error_output, info_output));
		ff_update_time = 0;
	#endif

	if (benchmode)
	{
		if(!NewGame(true))
		{
			error_output << "Error loading benchmark" << endl;
		}
	}
}

bool GAME::InitializeSound()
{
	if (sound.Init(2048, info_output, error_output))
	{
		generic_sounds.SetLibraryPath(PATHMANAGER::GetGenericSoundPath());
		
		if (!generic_sounds.Load("tire_squeal", sound.GetDeviceInfo(), error_output)) return false;
		if (!generic_sounds.Load("grass", sound.GetDeviceInfo(), error_output)) return false;
		if (!generic_sounds.Load("gravel", sound.GetDeviceInfo(), error_output)) return false;
		if (!generic_sounds.Load("bump_front", sound.GetDeviceInfo(), error_output)) return false;
		if (!generic_sounds.Load("bump_rear", sound.GetDeviceInfo(), error_output)) return false;
		if (!generic_sounds.Load("wind", sound.GetDeviceInfo(), error_output)) return false;
		if (!generic_sounds.Load("crash", sound.GetDeviceInfo(), error_output)) return false;
		
		sound.SetMasterVolume(settings->vol_master);
		sound.Pause(false);
	}
	else
	{
		error_output << "Sound initialization failed" << endl;
		return false;
	}

	info_output << "Sound initialization successful" << endl;
	return true;
}

///break up the input into a vector of strings using the token characters given
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

bool GAME::ParseArguments(std::list <std::string> & args)
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

///do any necessary cleanup
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

//void GAME::BeginDraw()
//{
	/*PROFILER.beginBlock("render");
		//send scene information to the graphics subsystem
	if (active_camera)
	{
		MATHVECTOR <float, 3> reflection_sample_location = active_camera->GetPosition();
		if (carcontrols_local.first)
			reflection_sample_location = carcontrols_local.first->GetCenterOfMassPosition();
		
		QUATERNION <float> camlook;
		camlook.Rotate(3.141593*0.5,1,0,0);
		camlook.Rotate(-3.141593*0.5,0,0,1);
		QUATERNION <float> camorient = -(active_camera->GetOrientation()*camlook);
		graphics.SetupScene(settings->GetFOV(), settings->GetViewDistance(), active_camera->GetPosition(), camorient, reflection_sample_location);
	}
	else
		graphics.SetupScene(settings->GetFOV(), settings->GetViewDistance(), MATHVECTOR <float, 3> (), QUATERNION <float> (), MATHVECTOR <float, 3> ());
*/


///the main game loop
bool GAME::OneLoop()
{
	bool ret = !eventsystem.GetQuit() && (!benchmode || replay.GetPlaying());
	if (ret)
	{
		if (profilingmode && frame % 20 == 0)
			strProfInfo = PROFILER.getAvgSummary(quickprof::MILLISECONDS);

		qtim.update();
		double dt = qtim.dt;
		clocktime += dt;

		Tick(dt);  // do CPU intensive stuff in parallel with the GPU

		//FinishDraw();  // sync CPU and GPU (flip the page)
		
		PROFILER.endCycle();
		
		displayframe++;
	}
	return ret;
}

///deltat is in seconds
void GAME::Tick(float deltat)
{
	const float minfps = 10.0f; //this is the minimum fps the game will run at before it starts slowing down time
	const unsigned int maxticks = (int) (1.0f / (minfps * framerate));
	const float maxtime = 1.0/minfps;
	unsigned int curticks = 0;

	//throw away wall clock time if necessary to keep the framerate above the minimum
	if (deltat > maxtime)
		deltat = maxtime;

	target_time += deltat;

	//increment game logic by however many tick periods have passed since the last GAME::Tick
	while (target_time - TickPeriod()*frame > TickPeriod() && curticks < maxticks)
	{
		frame++;

		AdvanceGameLogic();
		/*__*/
		if (pOgreGame)
		{
			pOgreGame->newPoses();

			if (settings->mult_thr != 1)  // == 0
			{	//  single thread
				pOgreGame->updatePoses(/*framerate*/deltat);
				if (!pause && pOgreGame->mFCam)
				pOgreGame->mFCam->update(framerate/*-deltat*/);
			if (pOgreGame->ndSky)  ///o-
				pOgreGame->ndSky->setPosition(pOgreGame->GetCamera()->getPosition());
			}
		}
		curticks++;
	}
}

///increment game logic by one frame
void GAME::AdvanceGameLogic()
{
	//PROFILER.beginBlock("input-processing");
	
	eventsystem.ProcessEvents();

	//ProcessGUIInputs();

	ProcessGameInputs();
	
	//PROFILER.endBlock("input-processing");

	if (track.Loaded())
	{
		if (pause && carcontrols_local.first)
		{
			sound.Pause(true);

			//this next line is required so that the game will see the unpause key
			carcontrols_local.second.ProcessInput(pOgreGame,
				settings->joytype, eventsystem, carcontrols_local.first->GetLastSteer(), TickPeriod(),
				settings->joy200, carcontrols_local.first->GetSpeed(), settings->speed_sensitivity,
				/*graphics.GetW(), graphics.GetH(),*/1280.f, 960.f,
				settings->button_ramp, settings->hgateshifter);
		}
		else
		{
			/*if (gui.Active()) //keep the game paused when the gui is up
			{
				if (sound.Enabled())
					sound.Pause(true); //stop sounds when the gui is up
			}
			else*/
			{
				if (sound.Enabled())
					sound.Pause(false);
				
				//PROFILER.beginBlock("ai");
				//ai.Visualize(rootnode);
				ai.update(TickPeriod(), &track, cars);
				//PROFILER.endBlock("ai");
				
				PROFILER.beginBlock("physics");
				collision.Update(TickPeriod());
				PROFILER.endBlock("physics");
				
				PROFILER.beginBlock("car");
				for (std::list <CAR>::iterator i = cars.begin(); i != cars.end(); ++i)
				{
					UpdateCar(*i, TickPeriod());
				}
				PROFILER.endBlock("car");
				
				//PROFILER.beginBlock("timer");
				UpdateTimer();
				//PROFILER.endBlock("timer");
			}
		}
	}

	//PROFILER.beginBlock("force-feedback");
	UpdateForceFeedback(TickPeriod());
	//PROFILER.endBlock("force-feedback");
}

///process inputs used only for higher level game functions
void GAME::ProcessGameInputs()
{
	/*if (carcontrols_local.first)
	{
		if (carcontrols_local.second.GetInput(CARINPUT::PAUSE) == 1.0)
		{
			//cout << "Pause input; changing " << pause << " to " << !pause << endl;
			pause = !pause;
		}
	}*/
}

void GAME::UpdateTimer()
{
	//check for cars doing a lap
	for (std::list <CAR>::iterator i = cars.begin(); i != cars.end(); ++i)
	{
	    int carid = cartimerids[&(*i)];

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
			timer.Lap(carid, i->GetSector(), nextsector, (i->GetSector() >= 0), settings->trackreverse); 
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

			timer.UpdateDistance(carid, curpatch->GetDistFromStart() + dist_from_back);
			//std::cout << curpatch->GetDistFromStart() << ", " << dist_from_back << std::endl;
			//std::cout << curpatch->GetDistFromStart() + dist_from_back << std::endl;
        }

		/*info_output << "sector=" << i->GetSector() << ", next=" << track.GetLapSequence(nextsector) << ", ";
		for (int w = 0; w < 4; w++)
		{
			info_output << w << "=" << i->GetCurPatch(w) << ", ";
		}
		info_output << std::endl;*/
	}

	timer.Tick(TickPeriod());
	//timer.DebugPrint(info_output);
}

///check eventsystem state and make updates to the GUI
void GAME::ProcessGUIInputs()
{
	if (eventsystem.GetKeyState(SDLK_ESCAPE).just_down &&
		!eventsystem.GetKeyState(SDLK_LSHIFT).down)
		eventsystem.Quit();  ///*

	if (eventsystem.GetKeyState(SDLK_F9).just_down)  // restart
		NewGame();
	
	//..
}

///send inputs to the car, check for collisions, and so on
void GAME::UpdateCar(CAR & car, double dt)
{
	car.Update(dt);
	UpdateCarInputs(car);
	UpdateDriftScore(car, dt);
}

void GAME::UpdateCarInputs(CAR & car)
{
    std::vector <float> carinputs(CARINPUT::INVALID, 0.0f);

    if (carcontrols_local.first == &car)
	{
	    if (replay.GetPlaying())
	    {
            const std::vector <float> & inputarray = replay.PlayFrame(car);
            assert(inputarray.size() <= carinputs.size());
            for (unsigned int i = 0; i < inputarray.size(); i++)
                carinputs[i] = inputarray[i];
	    }
	    else
            //carinputs = carcontrols_local.second.GetInputs();
            carinputs = carcontrols_local.second.ProcessInput(pOgreGame,
				settings->joytype, eventsystem, car.GetLastSteer(), TickPeriod(),
	            settings->joy200, car.GetSpeed(), settings->speed_sensitivity,
		        /*graphics.GetW(), graphics.GetH(),*/1280.f,960.f,
		        settings->button_ramp, settings->hgateshifter);
	}
	else
	{
	    carinputs = ai.GetInputs(&car);
		assert(carinputs.size() == CARINPUT::INVALID);
	}

	//force brake and clutch during staging and once the race is over
	///...-
	//if (timer.Staging() || ((int)timer.GetCurrentLap(cartimerids[&car]) > race_laps && race_laps > 0))
	//{
 //       carinputs[CARINPUT::BRAKE] = 1.0;
 //       carinputs[CARINPUT::CLUTCH] = 1.0;
	//}

    // mult_thr __ ??
#if 0
    std::vector <float> carinputs2(CARINPUT::INVALID, 0.0f);
    for (int i=0; i < carinputs.size(); ++i)
		carinputs2.push_back(i < 3 ? 1.f : carinputs[i]);
	car.HandleInputs(carinputs2, TickPeriod());
#else
	car.HandleInputs(carinputs, TickPeriod());
#endif

	if (carcontrols_local.first == &car)
	{
		if (replay.GetRecording())
			replay.RecordFrame(carinputs, car);

        //inputgraph.Update(carinputs);

		if (replay.GetPlaying())
		{
			//this next line allows game inputs to be processed
			carcontrols_local.second.ProcessInput(pOgreGame,
				settings->joytype, eventsystem, car.GetLastSteer(), TickPeriod(),
				settings->joy200, car.GetSpeed(), settings->speed_sensitivity,
				/*graphics.GetW(), graphics.GetH(),*/1280.f,960.f,
				settings->button_ramp, settings->hgateshifter);
		}

		/*std::stringstream debug_info1;
		car.DebugPrint(debug_info1, true, false, false, false);

		std::stringstream debug_info2;
		car.DebugPrint(debug_info2, false, true, false, false);

		std::stringstream debug_info3;
		car.DebugPrint(debug_info3, false, false, true, false);

		std::stringstream debug_info4;
		car.DebugPrint(debug_info4, false, false, false, true);*/

		/**/
		//set cockpit sounds
		
		//hide glass if we're inside the car
		//car.EnableGlass(!incar);
		/**/  ///...^
	}
}

///start a new game.  LeaveGame() is called first thing, which should take care of clearing out all current data.
bool GAME::NewGame(bool playreplay, bool addopponents, int num_laps)
{
	LeaveGame(); //this should clear out all data

	if (playreplay)
	{
		std::stringstream replayfilenamestream;

		if(benchmode)
			replayfilenamestream << PATHMANAGER::GetReplayPath() << "/benchmark.vdr";
		else
			replayfilenamestream << PATHMANAGER::GetReplayPath() << "/" << settings->selected_replay << ".vdr";

		string replayfilename = replayfilenamestream.str();
		info_output << "Loading replay file " << replayfilename << endl;
		if (!replay.StartPlaying(replayfilename, error_output))
			return false;
	}

	//set the track name
	string trackname;
	if (playreplay)
	{
		trackname = replay.GetTrack();
	}
	else
		trackname = settings->track;

	if (!LoadTrack(trackname))
	{
		error_output << "Error during track loading: " << trackname << endl;
		//return false;
	}/*-*/

	//set the car name
	string carname;
	if (playreplay)
		carname = replay.GetCarType();
	else
		carname = settings->car;

	//set the car paint
	/*string carpaint("00");
	if (playreplay)
		carpaint = replay.GetCarPaint();
	else
		carpaint = settings->carpaint;
	/**/

	//load the local player's car
	//cout << "About to load car..." << endl;
	if (playreplay)
	{
		if (!LoadCar(carname, track.GetStart(0).first, track.GetStart(0).second, true, false, replay.GetCarFile()))
			return false;
	}else{
		//cout << "Not playing replay..." << endl;
		if (!LoadCar(carname, track.GetStart(0).first, track.GetStart(0).second, true, false))
			return false;
		//cout << "Loaded car successfully" << endl;
	}
	//cout << "After load car: " << carcontrols_local.first << endl;

    race_laps = num_laps;

	//load AI cars
	/*if (addopponents)
	{
		int carcount = 1;
		for (std::vector <std::pair<std::string, std::string> >::iterator i = opponents.begin(); i != opponents.end(); ++i)
		{
			//int startplace = std::min(carcount, track.GetNumStartPositions()-1);
			int startplace = carcount;
			if (!LoadCar(i->first, i->second, track.GetStart(startplace).first, track.GetStart(startplace).second, false, true))
				return false;
			ai.add_car(&cars.back(), settings->ai_difficulty);
			carcount++;
		}
	}
	else/**/
		opponents.clear();

	//send car sounds to the sound subsystem
	for (std::list <CAR>::iterator i = cars.begin(); i != cars.end(); ++i)
	{
		std::list <SOUNDSOURCE *> soundlist;
		i->GetSoundList(soundlist);
		for (std::list <SOUNDSOURCE *>::iterator s = soundlist.begin(); s != soundlist.end(); ++s)
		{
			sound.AddSource(**s);
		}
	}

	//load the timer
	float pretime = 0.0f;
	if (num_laps > 0)
        pretime = 3.0f;
	if (!timer.Load(PATHMANAGER::GetTrackRecordsPath()+"/"+trackname+".txt", pretime, error_output))
		return false;

	//add cars to the timer system
	int count = 0;
	for (std::list <CAR>::iterator i = cars.begin(); i != cars.end(); ++i)
	{
		cartimerids[&(*i)] = timer.AddCar(i->GetCarType());
		if (carcontrols_local.first == &(*i))
			timer.SetPlayerCarID(count);
		count++;
	}

	//if (settings->mousegrab)
	//	eventsystem.SetMouseCursorVisibility(false);

	//record a replay
	/*if (settings->recordreplay && !playreplay)
	{
		assert(carcontrols_local.first);

		replay.StartRecording(carcontrols_local.first->GetCarType(), settings->carpaint,
			PATHMANAGER::GetCarPath()+"/"+carcontrols_local.first->GetCarType()+"/"+carcontrols_local.first->GetCarType()+".car",
			settings->track, error_output);
	}/**/

	return true;
}

std::string GAME::GetReplayRecordingFilename()
{
	//determine replay filename
	int replay_number = 1;
	for (int i = 1; i < 99; i++)
	{
		std::stringstream s;
		s << PATHMANAGER::GetReplayPath() << "/" << i << ".vdr";
		if (!PATHMANAGER::FileExists(s.str()))
		{
			replay_number = i;
			break;
		}
	}
	std::stringstream s;
	s << PATHMANAGER::GetReplayPath() << "/" << replay_number << ".vdr";
	return s.str();
}

///clean up all game data
void GAME::LeaveGame()
{
	ai.clear_cars();

	carcontrols_local.first = NULL;

	if (replay.GetRecording())
	{
		info_output << "Saving replay to " << GetReplayRecordingFilename() << endl;
		replay.StopRecording(GetReplayRecordingFilename());
		std::list <std::pair <std::string, std::string> > replaylist;
		PopulateReplayList(replaylist);
		//gui.ReplaceOptionMapValues("game.selected_replay", replaylist, error_output);
	}
	if (replay.GetPlaying())
		replay.StopPlaying();

	//gui.SetInGame(false);
	track.Unload();
	/*if (tracknode)
	{
		tracknode->Clear();
		//graphics.ClearStaticDrawlistMap();
	}**/
	collision.Clear();

	if (sound.Enabled())
	{
		for (std::list <CAR>::iterator i = cars.begin(); i != cars.end(); ++i)
		{
			std::list <SOUNDSOURCE *> soundlist;
			i->GetSoundList(soundlist);
			for (std::list <SOUNDSOURCE *>::iterator s = soundlist.begin(); s != soundlist.end(); s++)
			{
				sound.RemoveSource(*s);
			}
		}
	}
	cars.clear();
	timer.Unload();
	pause = false;
}

///add a car, optionally controlled by the local player
bool GAME::LoadCar(const std::string & carname, const MATHVECTOR <float, 3> & start_position,
		   const QUATERNION <float> & start_orientation, bool islocal, bool isai, const string & carfile)
{
	CONFIGFILE carconf;
	if (carfile.empty()) //if no file is passed in, then load it from disk
	{
		if ( !carconf.Load ( PATHMANAGER::GetCarPath()+"/"+carname+"/"+carname+".car" ) )
			return false;
	}
	else
	{
		std::stringstream carstream(carfile);
		if ( !carconf.Load ( carstream ) )
			return false;
	}

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
		debugmode, info_output, error_output))
	{
		error_output << "Error loading car: " << carname << endl;
		cars.pop_back();
		return false;
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
	
	return true;
}

bool GAME::LoadTrack(const std::string & trackname)
{
	LoadingScreen(0.0,1.0);

	//load the track
	if (!track.DeferredLoad(
			(settings->track_user ? PATHMANAGER::GetTrackPathUser() : PATHMANAGER::GetTrackPath()) + "/" + trackname,
			settings->trackreverse,
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

void GAME::PopulateReplayList(std::list <std::pair <std::string, std::string> > & replaylist)
{
	replaylist.clear();
	int numreplays = 0;
	std::list <std::string> replayfoldercontents;
	if (PATHMANAGER::GetFolderIndex(PATHMANAGER::GetReplayPath(),replayfoldercontents))
	{
		for (std::list <std::string>::iterator i = replayfoldercontents.begin(); i != replayfoldercontents.end(); ++i)
		{
			if (*i != "benchmark.vdr" && i->find(".vdr") == i->length()-4)
			{
				std::stringstream rnumstr;
				rnumstr << numreplays+1;
				replaylist.push_back(pair<string,string>(rnumstr.str(),*i));
				numreplays++;
			}
		}
	}

	if (numreplays == 0)
	{
		replaylist.push_back(pair<string,string>("0","None"));
		settings->selected_replay = 0; //replay zero is a special value that the GAME class interprets as "None"
	}
	else
		settings->selected_replay = 1;
}

void GAME::PopulateCarPaintList(const std::string & carname, std::list <std::pair <std::string, std::string> > & carpaintlist)
{
	carpaintlist.clear();
	string cartexfolder = PATHMANAGER::GetCarPath()+"/"+carname+"/textures";
	bool exists = true;
	int paintnum = 0;
	while (exists)
	{
		exists = false;

		std::stringstream paintstr;
		paintstr.width(2);  paintstr.fill('0');
		paintstr << paintnum;

		std::string cartexfile = cartexfolder+"/body"+paintstr.str()+".png";
			//std::cout << cartexfile << std::endl;
		ifstream check(cartexfile.c_str());
		if (check)
		{
			exists = true;
			carpaintlist.push_back(pair<string,string>(paintstr.str(),paintstr.str()));
			paintnum++;
		}
	}
}

void GAME::PopulateValueLists(std::map<std::string, std::list <std::pair <std::string, std::string> > > & valuelists)
{
	//populate track list
	{
		std::list <pair<string,string> > tracklist;
		std::list <string> trackfolderlist;
		PATHMANAGER::GetFolderIndex(PATHMANAGER::GetTrackPath(),trackfolderlist);
		for (std::list <string>::iterator i = trackfolderlist.begin(); i != trackfolderlist.end(); ++i)
		{
			ifstream check((PATHMANAGER::GetTrackPath() + "/" + *i + "/track.txt").c_str());
			if (check)
			{
				string displayname;
				getline(check, displayname);
				tracklist.push_back(pair<string,string>(*i,displayname));
			}
		}
		tracklist.sort(SortStringPairBySecond);
		valuelists["tracks"] = tracklist;
	}

	//populate car list
	{
		std::list <pair<string,string> > carlist;
		std::list <string> carfolderlist;
		PATHMANAGER::GetFolderIndex(PATHMANAGER::GetCarPath(),carfolderlist);
		for (std::list <string>::iterator i = carfolderlist.begin(); i != carfolderlist.end(); ++i)
		{
			ifstream check((PATHMANAGER::GetCarPath() + "/" + *i + "/about.txt").c_str());
			if (check)
			{
				carlist.push_back(pair<string,string>(*i,*i));
			}
		}
		valuelists["cars"] = carlist;
	}

	//populate car paints
	/*{
		PopulateCarPaintList(settings->car, valuelists["car_paints"]);
		PopulateCarPaintList(settings->car_ai, valuelists["opponent_car_paints"]);
	}/**/

	//populate replays list
	{
		PopulateReplayList(valuelists["replays"]);
	}

	//populate other lists
	valuelists["joy_indeces"].push_back(pair<string,string>("0","0"));
	//valuelists["skins"].push_back(pair<string,string>("simple","simple"));
}

void GAME::LoadSaveOptions(OPTION_ACTION action, std::map<std::string, std::string> & options)
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

///update the game with any new setting changes that have just been made
void GAME::ProcessNewSettings()
{
	if (carcontrols_local.first)
	{
		carcontrols_local.first->SetABS(settings->abs);
		carcontrols_local.first->SetTCS(settings->tcs);
		carcontrols_local.first->SetAutoClutch(settings->autoclutch);
		carcontrols_local.first->SetAutoShift(settings->autoshift);
		carcontrols_local.first->SetAutoRear(settings->autorear);
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

			//std::cout << "ff_update_time: " << ff_update_time << " force: " << force << endl;
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
	assert(cartimerids.find(&car) != cartimerids.end()); //assert that the car is registered with the timer system

	if ( on_track )
	{
		//velocity must be above 10 m/s
		if ( car_speed > 10 )
		{
			//drift starts when the angle > 0.2 (around 11.5 degrees)
			//drift ends when the angle < 0.1 (aournd 5.7 degrees)
			float angle_threshold(0.2);
			if ( timer.GetIsDrifting(cartimerids[&car]) ) angle_threshold = 0.1;

			is_drifting = ( car_angle > angle_threshold && car_angle <= M_PI/2.0 );
			spin_out = ( car_angle > M_PI/2.0 );
		}
	}

	//calculate score
	if ( is_drifting )
	{
		//base score is the drift distance
		timer.IncrementThisDriftScore(cartimerids[&car], dt * car_speed);

		//bonus score calculation is now done in TIMER
		timer.UpdateMaxDriftAngleSpeed(cartimerids[&car], car_angle, car_speed);
		
		//std::cout << timer.GetDriftScore(cartimerids[&car]) << " + " << timer.GetThisDriftScore(cartimerids[&car]) << endl;
	}

	//if (settings,
	if (settings->mult_thr != 1) // cartimerids.size() > 0)
	timer.SetIsDrifting(cartimerids[&car], is_drifting, on_track && !spin_out);
	
	//std::cout << is_drifting << ", " << on_track << ", " << car_angle << endl;
}

GAME::GAME(std::ostream & info_out, std::ostream & err_out, SETTINGS* pSettings) :
	settings(pSettings), info_output(info_out), error_output(err_out),
	frame(0), displayframe(0), clocktime(0), target_time(0),
	//framerate(0.01f),  ///~  0.004+  o:0.01
	fps_track(10,0), fps_position(0), fps_min(0), fps_max(0),
	multithreaded(false), benchmode(false), dumpfps(false),
	pause(false), debugmode(false), profilingmode(false),
	particle_timer(0), race_laps(0),
	track(info_out, err_out), /*tracknode(NULL),*/

	framerate(1.0/pSettings->game_fq), replay(framerate)
{
	carcontrols_local.first = NULL;
	//  sim iv from settings
	collision.fixedTimestep = 1.0/pSettings->blt_fq;
	collision.maxSubsteps = pSettings->blt_iter;
}
