#include "pch.h"
#include "game.h"
#include "car.h"
#include "unittest.h"
#include "matrix4.h"
#include "configfile.h"
#include "cardefs.h"
#include <math.h>
#include "../sound/SoundMgr.h"
#include "../sound/SoundBase.h"
#include "../sound/SoundBaseMgr.h"
#include "numprocessors.h"
#include "quickprof.h"
#include "tracksurface.h"
#include "forcefeedback.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/data/SceneXml.h"
#include "../ogre/common/CScene.h"
#include "../ogre/CGame.h"
#include "../ogre/CInput.h"
#include "../ogre/FollowCamera.h"
#include "../oics/ICSInputControlSystem.h"
#include <OgreTimer.h>
#include <OgreDataStream.h>

#define M_PI  3.14159265358979323846
using namespace std;


///  ctor
GAME::GAME(SETTINGS* pSettings)
	:app(NULL), settings(pSettings)
	,frame(0), displayframe(0), clocktime(0), target_time(0)
	//,framerate(0.01f),  ///~  0.004+  o:0.01
	,fps_min(0), fps_max(0)
	,pause(false), profilingmode(false), benchmode(false)
	,bResetObj(false)
	,framerate(1.0 / pSettings->game_fq)
	,tire_ref_id(0), reloadSimNeed(0),reloadSimDone(0)
	,snd(0)
{
	controls.first = NULL;
	//  sim settings
	collision.fixedTimestep = 1.0 / pSettings->blt_fq;
	collision.maxSubsteps = pSettings->blt_iter;
	
	snd_chk=0; snd_chkwr=0;  snd_lap=0; snd_lapbest=0;
	snd_stage=0; snd_fail=0;
	for (int i=0;i<3;++i)  snd_win[i]=0;
}


//  start the game with the given arguments
void GAME::Start(list <string> & args)
{
	//Test();  return;  // unit tests-

	//debugmode = true;  ///+

	//if (settings->bltLines)
	PROFILER.init(20);
	profilingmode = true;


	//settings->Load(PATHMANAGER::GetSettingsFile());

	controls.second.Reset();

	InitializeSound();  // if it fails, will be disabled

	//ProcessNewSettings();

	//initialize force feedback
	#ifdef ENABLE_FORCE_FEEDBACK
		forcefeedback = make_unique<FORCEFEEDBACK>(settings->ff_device, cerr, cout);
		ff_update_time = 0;
	#endif
	
	//ReloadSimData();  // later (need game.sim_mode)
}

void GAME::ReloadSimData()  /// New
{
	LoadTires();
	LoadAllSurfaces();
	LoadSusp();

	LogO("* * * Simulation: "+settings->game.sim_mode+". Loaded: "+toStr(tires.size()) +" tires, "+ toStr(surfaces.size()) +" surfaces, "+ toStr(suspS.size()) +"="+ toStr(suspD.size()) +" suspensions.");
}


///  Surfaces  all in data/cars/surfaces.cfg
//------------------------------------------------------------------------------------------------------------------------------
bool GAME::LoadAllSurfaces()
{
	surfaces.clear();
	surf_map.clear();

	string path, file = "/" + settings->game.sim_mode + "/surfaces.cfg";
	path = PATHMANAGER::CarSimU() + file;
	if (!PATHMANAGER::FileExists(path))  // user or orig
		path = PATHMANAGER::CarSim() + file;
	else
		LogO("Note: Using user surfaces.");
	
	CONFIGFILE param;
	if (!param.Load(path))
	{
		LogO("Error: Can't find surfaces configfile: "+path);
		return false;
	}
	
	list <string> sectionlist;
	param.GetSectionList(sectionlist);
	
	for (auto section : sectionlist)
	{
		TRACKSURFACE surf;
		surf.name = section;
		
		int id;
		param.GetParam(section + ".ID", id);  // for sound..
		//-assert(indexnum >= 0 && indexnum < (int)tracksurfaces.size());
		surf.setType(id);
		
		float f = 0.f;
		param.GetParamE(section + ".BumpWaveLength", f);	surf.bumpWaveLength = f;
		param.GetParamE(section + ".BumpAmplitude", f);	surf.bumpAmplitude = f;
		if (param.GetParam(section + ".BumpWaveLength2", f))  surf.bumpWaveLength2 = f;
		if (param.GetParam(section + ".BumpAmplitude2", f))   surf.bumpAmplitude2 = f;
		
		param.GetParamE(section + ".FrictionTread", f);	surf.friction = f;
		if (param.GetParam(section + ".FrictionX", f))   surf.frictionX = f;
		if (param.GetParam(section + ".FrictionY", f))   surf.frictionY = f;
		
		if (param.GetParam(section + ".RollResistance", f))	surf.rollingResist = f;
		param.GetParamE(section + ".RollingDrag", f);			surf.rollingDrag = f;


		///---  Tire  ---
		string tireFile;
		if (!param.GetParam(section + "." + "Tire", tireFile))
		{
			tireFile = "gravel";  // default surface if not found
			LogO("Surface: Warning: Tire file not found, using default: "+tireFile);
		}
		id = tires_map[tireFile]-1;
		if (id == -1)
		{	id = 0;
			LogO("Surface: Tire id not found in map, using 0, "+tireFile);
		}
		//LogO("Tires size: "+toStr(pGame->tires.size()));
		surf.tire = &tires[id];
		surf.tireName = tireFile;
		///---
		

		surfaces.push_back(surf);
		surf_map[surf.name] = (int)surfaces.size();  //+1, 0 = not found
	}
	return true;
}


///  Tires  all in data/carsim/normal/tires/*.tire
//------------------------------------------------------------------------------------------------------------------------------
bool GAME::LoadTire(CARTIRE& ct, string path, string& file)
{
	CONFIGFILE c;
	if (!c.Load(path+"/"+file))
	{	LogO("Error loading tire file "+file);
		return false;
	}
	file = file.substr(0, file.length()-5);  // no ext .tire
	float value;

	for (int i = 0; i < 15; ++i)
	{
		int numinfile = i;
		if (i == 11)		numinfile = 111;
		else if (i == 12)	numinfile = 112;
		else if (i > 12)	numinfile -= 1;
		stringstream str;  str << "params.a" << numinfile;
		if (!c.GetParamE(str.str(), value))  return false;
		ct.lateral[i] = value;
	}
	for (int i = 0; i < 11; ++i)
	{
		stringstream str;  str << "params.b" << i;
		if (!c.GetParamE(str.str(), value))  return false;
		ct.longitudinal[i] = value;
	}
	for (int i = 0; i < 18; ++i)
	{
		stringstream str;  str << "params.c" << i;
		if (!c.GetParamE(str.str(), value))  return false;
		ct.aligning[i] = value;
	}
	ct.name = file;
	ct.CalculateSigmaHatAlphaHat();
	return true;
}

bool GAME::LoadTires()
{
	tires.clear();
	tires_map.clear();
	
	//  load from both user and orig dirs
	for (int u=0; u < 2; ++u)
	{
		string path = u == 1 ? PATHMANAGER::CarSimU() : PATHMANAGER::CarSim();
		path += "/" + settings->game.sim_mode + "/tires";
		list <string> li;
		PATHMANAGER::DirList(path, li);

		for (auto file : li)
		{
			if (file.find(".tire") != string::npos)
			{
				CARTIRE ct;
				ct.user = u;
				if (LoadTire(ct, path, file))
				{
					tires.push_back(ct);
					tires_map[file] = (int)tires.size();  //+1, 0 = not found
					TRACKSURFACE::pTireDefault = &ct;  //-
				}else
					LogO("Error Loading tire: "+file);
			}
	}	}
	return true;
}
CARTIRE* TRACKSURFACE::pTireDefault = 0;  //-

//  for graphs only
void GAME::PickTireRef(std::string name)
{
	tire_ref = name;
	int id = tires_map[name]-1;
	if (id == -1)
	{	id = 0;  LogO("Warning: Reference tire not found: "+ name);  }
	tire_ref_id = id;
	/*if (!cars.empty())
		cars.begin()->GraphsNewVals(0.1);*/
}


///  Suspension factors
//------------------------------------------------------------------------------------------------------------------------------
bool GAME::LoadSusp()
{
	suspS.clear();  suspS_map.clear();
	suspD.clear();  suspD_map.clear();
	
	string path = PATHMANAGER::CarSim() + "/" + settings->game.sim_mode + "/susp";
	list <string> li;
	PATHMANAGER::DirList(path, li);
	for (auto file : li)
	{
		if (file.find(".susp") != string::npos)
		{
			CONFIGFILE c;
			if (!c.Load(path+"/"+file))
			{	LogO("Error loading susp file "+file);
				return false;  }

			file = file.substr(0, file.length()-5);

			//  factor points
			vector <pair <double, double> > damper, spring;
			c.GetPoints("suspension", "damper-factor", damper);
			c.GetPoints("suspension", "spring-factor", spring);

			suspS.push_back(spring);
			suspD.push_back(damper);
			suspS_map[file] = (int)suspS.size();  //+1, 0 = not found
			suspD_map[file] = (int)suspD.size();
		}
	}
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------


///  Sound Init
bool GAME::InitializeSound()
{
	Ogre::Timer ti;
	
	snd = new SoundMgr();
	snd->Init(settings->snd_device, settings->snd_reverb);
	snd->setMasterVolume(0.f);
	using namespace Ogre;


	//  sounds.cfg  ----
	ifstream fi;
	string path = PATHMANAGER::Sounds()+"/sounds.cfg";
	fi.open(path.c_str(), ios_base::binary);
	if (!fi)
	{	LogO("@  Can't load sounds.cfg");  return false;  }
	FileStreamDataStream fd(&fi,false);
	snd->parseScript(&fd);
	fd.close();
	fi.close();

	snd->setMasterVolume(settings->vol_master);


	LogO("::: Time Sounds: "+ fToStr(ti.getMilliseconds(),0,3) +" ms");
	if (snd->sound_mgr->isDisabled())
	{
		LogO("@  Sound init - Disabled.");
		return false;
	}
	LogO("@  Sound init ok.");
	return true;
}


void GAME::LoadHudSounds()
{
	Ogre::Timer ti;
	snd_chk = snd->createInstance("hud/chk",  0);
	snd_chkwr = snd->createInstance("hud/chkwrong",  0);
	snd_lap = snd->createInstance("hud/lap",  0);
	snd_lapbest = snd->createInstance("hud/lapbest",  0);
	snd_stage = snd->createInstance("hud/stage",  0);
	for (int i=0; i < 3; ++i)
	snd_win[i] = snd->createInstance("hud/win"+toStr(i),  0);
	snd_fail = snd->createInstance("hud/fail",  0);
	UpdHudSndVol();
	LogO("::: Time Hud Sounds: "+ fToStr(ti.getMilliseconds(),0,3) +" ms");
}


void GAME::UpdHudSndVol()
{
	float g = settings->vol_hud;
	snd_chk->setGain(g);  snd_chkwr->setGain(g);
	snd_lap->setGain(g);  snd_lapbest->setGain(g);
	snd_stage->setGain(g);
	for (int i=0; i<3; ++i)  snd_win[i]->setGain(g);
	snd_fail->setGain(g);
}


//  do any necessary cleanup
void GAME::End()
{
	if (benchmode)
	{
		float mean_fps = displayframe / clocktime;
		LogO("Elapsed time: "+ fToStr(clocktime) +" seconds");
		LogO("Average frame-rate: "+ fToStr(mean_fps) +" frames per second");
		LogO("Min / Max frame-rate: "+ fToStr(fps_min) +" / "+ fToStr(fps_max) +" frames per second");
	}

	if (profilingmode)
		LogO("Profiling summary:\n" + PROFILER.getSummary(quickprof::PERCENT));

	LogO("Game shutting down.");


	LeaveGame(true);

	//  hud sounds
	delete snd_chk;  delete snd_chkwr;
	delete snd_lap;  delete snd_lapbest;
	delete snd_stage;  delete snd_fail;
	for (int i=0; i < 3; ++i)
	delete snd_win[i];
	
	delete snd;  snd = 0;


	///+  save settings first incase later deinits cause crashes
	settings->Save(PATHMANAGER::SettingsFile());

	collision.Clear();
}


void GAME::Test()
{
	QT_RUN_TESTS;
}


///  the main game loop
//----------------------------------------------------------------------------------------------------------------------------
bool GAME::OneLoop(double dt)
{
	if (reloadSimNeed)
	{	// 	upd tweak tire save
		reloadSimNeed = false;
		ReloadSimData();
		reloadSimDone = true;
	}	

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
	const float minfps = 10.f;  // this is the minimum fps the game will run at before it starts slowing down time
	const unsigned int maxticks = (int)(1.f / (minfps * framerate));
	const float maxtime = 1.f / minfps;
	unsigned int curticks = 0;

	//  throw away wall clock time if necessary to keep the framerate above the minimum
	if (deltat > maxtime)
		deltat = maxtime;
		
	//.  dont simulate before /network start
	if (!app)  return;
	bool sim = app->iLoad1stFrames == -2 && (!timer.waiting || timer.end_sim);

	//  speed up perf test
	if (app && app->bPerfTest)
		deltat *= settings->perf_speed;
	
	target_time += deltat;
	double tickperriod = TickPeriod();

	//  increment game logic by however many tick periods have passed since the last GAME::Tick
	while (target_time > tickperriod && curticks < maxticks)
	{
		frame++;
		AdvanceGameLogic(sim ? tickperriod : 0.0);

		if (app)
			app->newPoses(tickperriod);

		curticks++;
		target_time -= tickperriod;
	}
}


///  simulate game by one frame
//----------------------------------------------------------------------------------------------------------------------------
void GAME::AdvanceGameLogic(double dt)
{
	//if (track.Loaded())
	{
		if (pause && controls.first)
			snd->setPaused(true);
		else
		{	snd->setPaused(false);

			PROFILER.beginBlock("-physics");

			///~~  clear fluids for each car
			for (size_t i = 0; i < cars.size(); ++i)
			{
				CARDYNAMICS& cd = cars[i]->dynamics;
				cd.inFluids.clear();
				cd.velPrev = cd.chassis->getLinearVelocity();
				for (int w=0; w < cd.numWheels; ++w)
					cd.inFluidsWh[w].clear();
			}

			if (bResetObj)
			{	bResetObj = false;
				app->ResetObjects();
			}

			if (dt > 0.0)
				collision.Update(dt, settings->bltProfilerTxt);

			PROFILER.endBlock("-physics");

			PROFILER.beginBlock("-car-sim");
			for (size_t i = 0; i < cars.size(); ++i)
				UpdateCar(*cars[i], TickPeriod());
			PROFILER.endBlock("-car-sim");

			UpdateTimer();
		}
	}

	UpdateForceFeedback(TickPeriod());
}


///  send inputs to the car, check for collisions, and so on
//-----------------------------------------------------------
void GAME::UpdateCar(CAR& car, double dt)
{
	car.Update(dt);
	UpdateCarInputs(car);
}

void GAME::UpdateCarInputs(CAR & car)
{
	vector <float> carinputs(CARINPUT::ALL, 0.0f);
	//  race countdown or loading
	bool forceBrake = timer.waiting || timer.pretime > 0.f || app->iLoad1stFrames > -2;

	int i = app->scn->sc->asphalt ? 1 : 0;
	float sss_eff = settings->sss_effect[i], sss_velf = settings->sss_velfactor[i];
	float carspeed = car.GetSpeedDir();  //car.GetSpeed();
	//LogO(fToStr(car.GetSpeed(),2,6)+" "+fToStr(car.GetSpeedDir(),2,6));

	boost::lock_guard<boost::mutex> lock(app->input->mPlayerInputStateMutex);
	int id = std::min(3, car.id);
	carinputs = controls.second.ProcessInput(
		app->input->mPlayerInputState[id], car.id,
		carspeed, sss_eff, sss_velf,  app->mInputCtrlPlayer[id]->mbOneAxisThrottleBrake,
		forceBrake, app->bPerfTest, app->iPerfTestStage);

	car.HandleInputs(carinputs, TickPeriod());
}


bool GAME::NewGameDoLoadTrack()
{
	// if (!LoadTrack(settings->game.track))
	// 	LogO("Error during track loading: "+settings->game.track);

	return true;
}

bool GAME::NewGameDoLoadMisc(float pre_time)
{
	//  load the timer
	if (!timer.Load(PATHMANAGER::Records()+"/"+ settings->game.sim_mode+"/"+ settings->game.track+".txt", pre_time))
		return false;

	for (size_t i = 0; i < cars.size(); ++i)
		timer.AddCar(cars[i]->GetCarType());
	timer.AddCar("ghost");

	//  sounds
	LoadHudSounds();
	snd->sound_mgr->CreateSources();  ///)
	return true;
}

///  clean up all game data
void GAME::LeaveGame(bool dstTrk)
{
	controls.first = NULL;

	// if (dstTrk)
		// track.Unload();

	//  cars
	bool hadCars = !cars.empty();
	for (size_t i = 0; i < cars.size(); ++i)
		delete cars[i];
	cars.clear();

	//  sounds
	if (snd && hadCars)
		snd->sound_mgr->DestroySources(false);  ///)
	
	timer.Unload();

	if (dstTrk)
		collision.Clear();

	pause = false;
}

///  add a car, optionally controlled by the local player
CAR* GAME::LoadCar(const string& pathCar, const string& carname,
	const MATHVECTOR<float,3>& start_pos, const QUATERNION<float>& start_rot,
	bool islocal, bool isRemote, int idCar)
{
	CONFIGFILE carconf;
	if (!carconf.Load(pathCar))
		return NULL;

	CAR* car = new CAR();

	if (!car->Load(app,  carconf, carname,
		start_pos, start_rot,  collision,
		settings->abs, settings->tcs,  isRemote, idCar, false))
	{
		LogO("-=- Error: loading CAR: "+carname);
		return NULL;
	}
	else
	{
		cars.push_back(car);
		LogO("-=- Car loaded: "+carname);

		if (islocal)
		{
			//load local controls
			controls.first = car;

			//setup auto clutch and auto shift
			ProcessNewSettings();
			// shift into first gear if autoshift enabled
			if (controls.first && settings->autoshift)
				controls.first->SetGear(1);
		}
	}
	return car;
}

bool SortStringPairBySecond (const pair<string,string> & first, const pair<string,string> & second)
{
	return first.second < second.second;
}

//  update the game with any new setting changes that have just been made
void GAME::ProcessNewSettings()
{
	if (controls.first)
	{
		int i = app->scn->sc->asphalt ? 1 : 0;
		controls.first->SetABS(settings->abs[i]);
		controls.first->SetTCS(settings->tcs[i]);
		controls.first->SetAutoShift(settings->autoshift);
		controls.first->SetAutoRear(settings->autorear);
		//controls.first->SetAutoClutch(settings->rear_inv);
	}
	snd->setMasterVolume(settings->vol_master);
}

void GAME::UpdateForceFeedback(float dt)
{
#ifdef ENABLE_FORCE_FEEDBACK
	if (controls.first)
	{
		//static ofstream file("ff_output.txt");
		ff_update_time += dt;
		const double ffdt = 0.02;
		if (ff_update_time >= ffdt)
		{
			ff_update_time = 0.0;
			double feedback = -controls.first->GetFeedback();

			feedback = settings->ff_gain * feedback / 100.0;
			if (settings->ff_invert)  feedback = -feedback;

			if (feedback > 1.0)  feedback = 1.0;
			if (feedback <-1.0)  feedback =-1.0;
			//feedback += 0.5;
			//const static double motion_frequency = 0.1, motion_amplitude = 4.0;
			//double center = sin( timefactor * 2 * M_PI * motion_frequency ) * motion_amplitude;
			double force = feedback;

			//cout << "ff_update_time: " << ff_update_time << " force: " << force << endl;
			forcefeedback->update(force, &feedback, ffdt, cerr);
		}
	}

	if (pause && dt == 0)
	{
		double pos=0;
		forcefeedback->update(0, &pos, 0.02, cerr);
	}
#endif
}



void GAME::UpdateTimer()
{
	if (app->iLoad1stFrames == -2)  // ended loading
		timer.Tick(TickPeriod());
	//timer.DebugPrint(info_output);
}


///  Car Steering range multiplier
float GAME::GetSteerRange() const
{
	float range = settings->steer_sim[settings->gui.sim_mode == "easy" ? 0 : 1];
	range *= settings->steer_range[/*track.asphalt*/0];
	return range;
}
