#include "pch.h"
#include "car.h"
#include "cardefs.h"
#include "configfile.h"
#include "coordinatesystems.h"
#include "collision_world.h"
#include "tracksurface.h"
#include "configfile.h"
#include "settings.h"
#include "../ogre/OgreGame.h"  //+ replay
#include "../ogre/common/Defines.h"
#include "../network/protocol.hpp"
#include "tobullet.h"
#include <OgreLogManager.h>

#ifdef _WIN32
bool isnan(float number) {return (number != number);}
bool isnan(double number) {return (number != number);}
#endif

CAR::CAR() :
	pSet(0), pApp(0),
	last_steer(0),
	debug_wheel_draw(false),
	sector(-1),
	iCamNext(0), bLastChk(0),
	fluidHitOld(0)
{
	//dynamics.pCar = this;
	vInteriorOffset[0]=0;
	vInteriorOffset[1]=0;
	vInteriorOffset[2]=0;
	
	for (int i = 0; i < 4; i++)
	{
		curpatch[i] = NULL;
		//wheelnode[i] = NULL;
		//floatingnode[i] = NULL;
	}
	for (int i=0; i < Ncrashsounds; ++i)
		crashsoundtime[i] = 0.f;
}

///unload any loaded assets
CAR::~CAR()
{	}


//--------------------------------------------------------------------------------------------------------------------------
bool CAR::Load(class App* pApp1,
	SETTINGS* settings,
	CONFIGFILE & carconf,
	const std::string & carpath,
	const std::string & driverpath,
	const std::string & carname,
	const MATHVECTOR <float, 3> & initial_position,
	const QUATERNION <float> & initial_orientation,
	COLLISION_WORLD & world,
	bool soundenabled,
	const SOUNDINFO & sound_device_info,
	const SOUNDBUFFERLIBRARY & soundbufferlibrary,
	bool defaultabs, bool defaulttcs,
  	bool debugmode,
  	std::ostream & info_output,
  	std::ostream & error_output )
{
	pApp = pApp1;
	pSet = settings;
	cartype = carname;
	std::stringstream nullout;

	//load car body graphics
	if (!LoadInto( carpath+"/"+carname+"/body.joe", bodymodel, error_output))
		/*info_output << "No car body model, continuing without one" << std::endl*/;

	//load driver graphics --
	//if (!LoadInto( driverpath+"/body.joe", drivermodel, error_output))
	//	error_output << "Error loading driver graphics: " << driverpath << std::endl;

	//load car interior graphics
	if (!LoadInto( carpath+"/"+carname+"/interior.joe", interiormodel, nullout ))
		/*info_output << "No car interior model exists, continuing without one" << std::endl*/;

	//load car glass graphics
	if (!LoadInto( carpath+"/"+carname+"/glass.joe", glassmodel, nullout ))
		/*info_output << "No car glass model exists, continuing without one" << std::endl*/;

	//load wheel graphics
	for (int i = 0; i < 2; i++)  // front pair
	{
		if (!LoadInto( carpath+"/"+carname+"/wheel_front.joe", wheelmodelfront, error_output))
			/*info_output << "No car wheel_front model, continuing without one" << std::endl*/;

		//load floating elements
		std::stringstream nullout;
		LoadInto( carpath+"/"+carname+"/floating_front.joe", floatingmodelfront, nullout);
	}
	for (int i = 2; i < 4; i++)  // rear pair
	{
		if (!LoadInto( carpath+"/"+carname+"/wheel_rear.joe", wheelmodelrear, error_output))
			/*info_output << "No car wheel_rear model, continuing without one" << std::endl*/;

		//load floating elements
		std::stringstream nullout;
		LoadInto( carpath+"/"+carname+"/floating_rear.joe", floatingmodelrear, nullout);
	}

	// get coordinate system version
	int version = 1;
	carconf.GetParam("version", version);
	
	
	///-  custom interior model offset--
	vInteriorOffset[0] = 0.f;
	vInteriorOffset[1] = 0.f;
	vInteriorOffset[2] = 0.f;
	carconf.GetParam("model_ofs.interior-x", vInteriorOffset[0]);
	carconf.GetParam("model_ofs.interior-y", vInteriorOffset[1]);
	carconf.GetParam("model_ofs.interior-z", vInteriorOffset[2]);
	
	///-  custom exhaust pos for boost particles
	if (carconf.GetParam("model_ofs.exhaust-x", exhaustPosition[0]))
	{
		manualExhaustPos = true;
		carconf.GetParam("model_ofs.exhaust-y", exhaustPosition[1]);
		carconf.GetParam("model_ofs.exhaust-z", exhaustPosition[2]);
	}
	else
		manualExhaustPos = false;
	if (!carconf.GetParam("model_ofs.exhaust-mirror-second", has2exhausts))
		has2exhausts = false;

	///-  custom car collision params
	dynamics.coll_R = 0.3f;  dynamics.coll_H = 0.45f;  dynamics.coll_W = 0.5f;
	dynamics.coll_Lofs = 0.f;  dynamics.coll_Wofs = 0.f;  dynamics.coll_Hofs = 0.f;
	dynamics.coll_manual = false;  // normally auto

	if (carconf.GetParam("collision.manual", dynamics.coll_manual))
	{
		carconf.GetParam("collision.radius", dynamics.coll_R);
		carconf.GetParam("collision.width", dynamics.coll_W);
		carconf.GetParam("collision.height", dynamics.coll_H);

		carconf.GetParam("collision.offsetL", dynamics.coll_Lofs);
		carconf.GetParam("collision.offsetW", dynamics.coll_Wofs);
		carconf.GetParam("collision.offsetH", dynamics.coll_Hofs);
	}
	

	// load cardynamics
	{
		if (!dynamics.Load(carconf, error_output)) return false;

		MATHVECTOR<double, 3> position;
		QUATERNION<double> orientation;
		position = initial_position;		posAtStart = posLastCheck = initial_position;
		orientation = initial_orientation;	rotAtStart = rotLastCheck = initial_orientation;
		
		dynamics.Init(pSet, &pApp->sc, &pApp->fluidsXml,
			world, bodymodel, wheelmodelfront, wheelmodelrear, position, orientation);

		dynamics.SetABS(defaultabs);
		dynamics.SetTCS(defaulttcs);
	}

	// load driver
	{
		float pos[3];
		if (!carconf.GetParam("driver.view-position", pos, error_output)) return false;
		if (version == 2) COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(pos[0], pos[1], pos[2]);
		
		driver_view_position.Set(pos[0], pos[1], pos[2]);
		
		if (!carconf.GetParam("driver.hood-mounted-view-position", pos, error_output)) return false;
		if (version == 2) COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(pos[0], pos[1], pos[2]);
		
		hood_view_position.Set(pos[0], pos[1], pos[2]);
		
		/*if (drivernode) //move the driver model to the coordinates given
		{
			MATHVECTOR <float, 3> floatpos;
			floatpos.Set(pos[0], pos[1], pos[2]);
			drivernode->GetTransform().SetTranslation(floatpos);
		}*/
	}

	//load sounds
	if (soundenabled)
	{
		if (!LoadSounds(carpath, carname, sound_device_info, soundbufferlibrary, info_output, error_output))
			return false;
	}

	mz_nominalmax = (GetTireMaxMz(FRONT_LEFT) + GetTireMaxMz(FRONT_RIGHT))*0.5;

	lookbehind = false;

	return true;
}


//--------------------------------------------------------------------------------------------------------------------------
bool CAR::LoadSounds(
	const std::string & carpath,
	const std::string & carname,
	const SOUNDINFO & sound_device_info,
	const SOUNDBUFFERLIBRARY & soundbufferlibrary,
	std::ostream & info_output,
	std::ostream & error_output)
{
	//check for sound specification file
	CONFIGFILE aud;
	if (aud.Load(carpath+"/"+carname+"/"+carname+".aud"))
	{
		std::list <std::string> sections;
		aud.GetSectionList(sections);
		for (std::list <std::string>::iterator i = sections.begin(); i != sections.end(); ++i)
		{
			//load the buffer
			std::string filename;
			if (!aud.GetParam(*i+".filename", filename, error_output)) return false;
			if (!soundbuffers[filename].GetLoaded())
				if (!soundbuffers[filename].Load(carpath+"/"+carname+"/"+filename, sound_device_info, error_output))
				{
					error_output << "Error loading sound: " << carpath+"/"+carname+"/"+filename << std::endl;
					return false;
				}

			enginesounds.push_back(std::pair <ENGINESOUNDINFO, SOUNDSOURCE> ());
			ENGINESOUNDINFO & info = enginesounds.back().first;
			SOUNDSOURCE & sound = enginesounds.back().second;

			if (!aud.GetParam(*i+".MinimumRPM", info.minrpm, error_output)) return false;
			if (!aud.GetParam(*i+".MaximumRPM", info.maxrpm, error_output)) return false;
			if (!aud.GetParam(*i+".NaturalRPM", info.naturalrpm, error_output)) return false;

			std::string powersetting;
			if (!aud.GetParam(*i+".power", powersetting, error_output)) return false;
			if (powersetting == "on")
				info.power = ENGINESOUNDINFO::POWERON;
			else if (powersetting == "off")
				info.power = ENGINESOUNDINFO::POWEROFF;
			else //assume it's used in both ways
				info.power = ENGINESOUNDINFO::BOTH;

			sound.SetBuffer(soundbuffers[filename]);
			sound.Set3DEffects(true);
			sound.SetLoop(true);
			sound.SetGain(0);
			sound.Play();
		}

		//set blend start and end locations -- requires multiple passes
		std::map <ENGINESOUNDINFO *, ENGINESOUNDINFO *> temporary_to_actual_map;
		std::list <ENGINESOUNDINFO> poweron_sounds;
		std::list <ENGINESOUNDINFO> poweroff_sounds;
		for (std::list <std::pair <ENGINESOUNDINFO, SOUNDSOURCE> >::iterator i = enginesounds.begin(); i != enginesounds.end(); ++i)
		{
			ENGINESOUNDINFO & info = i->first;
			if (info.power == ENGINESOUNDINFO::POWERON)
			{
				poweron_sounds.push_back(info);
				temporary_to_actual_map[&poweron_sounds.back()] = &info;
			}
			else if (info.power == ENGINESOUNDINFO::POWEROFF)
			{
				poweroff_sounds.push_back(info);
				temporary_to_actual_map[&poweroff_sounds.back()] = &info;
			}
		}

		poweron_sounds.sort();
		poweroff_sounds.sort();

		//we only support 2 overlapping sounds at once each for poweron and poweroff; this
		// algorithm fails for other cases (undefined behavior)
		std::list <ENGINESOUNDINFO> * cursounds = &poweron_sounds;
		for (int n = 0; n < 2; n++)
		{
			if (n == 1)
				cursounds = &poweroff_sounds;

			for (std::list <ENGINESOUNDINFO>::iterator i = (*cursounds).begin(); i != (*cursounds).end(); ++i)
			{
				//set start blend
				if (i == (*cursounds).begin())
					i->fullgainrpmstart = i->minrpm;
				//else, the blend start has been set already by the previous iteration

				//set end blend
				std::list <ENGINESOUNDINFO>::iterator inext = i;
				inext++;
				if (inext == (*cursounds).end())
					i->fullgainrpmend = i->maxrpm;
				else
				{
					i->fullgainrpmend = inext->minrpm;
					inext->fullgainrpmstart = i->maxrpm;
				}
			}

			//now assign back to the actual infos
			for (std::list <ENGINESOUNDINFO>::iterator i = (*cursounds).begin(); i != (*cursounds).end(); ++i)
			{
				assert(temporary_to_actual_map.find(&(*i)) != temporary_to_actual_map.end());
				*temporary_to_actual_map[&(*i)] = *i;
			}
		}
	}
	else
	{
		if (!soundbuffers["engine.wav"].Load(carpath+"/"+carname+"/engine.wav", sound_device_info, error_output))
		{
			error_output << "Unable to load engine sound: "+carpath+"/"+carname+"/engine.wav" << std::endl;
			return false;
		}
		enginesounds.push_back(std::pair <ENGINESOUNDINFO, SOUNDSOURCE> ());
		SOUNDSOURCE & enginesound = enginesounds.back().second;
		enginesound.SetBuffer(soundbuffers["engine.wav"]);
		enginesound.Set3DEffects(true);
		enginesound.SetLoop(true);
		enginesound.SetGain(0);
		enginesound.Play();
	}

	//set up tire squeal sounds
	for (int i = 0; i < 4; i++)
	{
		const SOUNDBUFFER * buf = soundbufferlibrary.GetBuffer("tire_squeal");
		if (!buf)
		{
			error_output << "Can't load tire_squeal sound" << std::endl;
			return false;
		}
		tiresqueal[i].SetBuffer(*buf);
		tiresqueal[i].Set3DEffects(true);
		tiresqueal[i].SetLoop(true);
		tiresqueal[i].SetGain(0);
		int samples = tiresqueal[i].GetSoundBuffer().GetSoundInfo().GetSamples();
		tiresqueal[i].SeekToSample((samples/4)*i);
		tiresqueal[i].Play();
	}

	//set up tire gravel sounds
	for (int i = 0; i < 4; i++)
	{
		const SOUNDBUFFER * buf = soundbufferlibrary.GetBuffer("gravel");
		if (!buf)
		{	error_output << "Can't load gravel sound" << std::endl;		return false;
		}
		gravelsound[i].SetBuffer(*buf);		gravelsound[i].Set3DEffects(true);
		gravelsound[i].SetLoop(true);		gravelsound[i].SetGain(0);
		int samples = gravelsound[i].GetSoundBuffer().GetSoundInfo().GetSamples();
		gravelsound[i].SeekToSample((samples/4)*i);
		gravelsound[i].Play();
	}

	//set up tire grass sounds
	for (int i = 0; i < 4; i++)
	{
		const SOUNDBUFFER * buf = soundbufferlibrary.GetBuffer("grass");
		if (!buf)
		{
			error_output << "Can't load grass sound" << std::endl;
			return false;
		}
		grasssound[i].SetBuffer(*buf);		grasssound[i].Set3DEffects(true);
		grasssound[i].SetLoop(true);		grasssound[i].SetGain(0);
		int samples = grasssound[i].GetSoundBuffer().GetSoundInfo().GetSamples();
		grasssound[i].SeekToSample((samples/4)*i);
		grasssound[i].Play();
	}

	//set up bump sounds
	for (int i = 0; i < 4; i++)
	{
		const SOUNDBUFFER * buf = soundbufferlibrary.GetBuffer("bump_front");
		if (i >= 2)
			buf = soundbufferlibrary.GetBuffer("bump_rear");
		if (!buf)
		{	error_output << "Can't load bump sound: " << i << std::endl;	return false;
		}
		tirebump[i].SetBuffer(*buf);	tirebump[i].Set3DEffects(true);
		tirebump[i].SetLoop(false);		tirebump[i].SetGain(1.0);
	}

	//set up crash sounds (many)
	for (int i = 0; i < Ncrashsounds; ++i)
	{
		int n = i+1;
		char name[3] = {'0'+ n/10, '0'+ n%10, 0};
		const SOUNDBUFFER * buf = soundbufferlibrary.GetBuffer(name);
		if (!buf)
		{	error_output << "Can't load crash sound: " << name << std::endl;	return false;
		}
		crashsound[i].SetBuffer(*buf);	crashsound[i].Set3DEffects(true);
		crashsound[i].SetLoop(false);	crashsound[i].SetGain(1.0);
	}

	{
		const SOUNDBUFFER * buf = soundbufferlibrary.GetBuffer("wind");
		if (!buf)
		{	error_output << "Can't load wind sound" << std::endl;	return false;
		}
		roadnoise.SetBuffer(*buf);	roadnoise.Set3DEffects(true);
		roadnoise.SetLoop(true);	roadnoise.SetGain(0);
		roadnoise.SetPitch(1.0);	roadnoise.Play();
	}

	//set up  fluid sounds
	for (int i = 0; i < Nwatersounds; ++i)
	{
		std::string name = "water"+toStr(i+1);
		const SOUNDBUFFER * buf = soundbufferlibrary.GetBuffer(name);
		if (!buf)
		{	error_output << "Can't load water sound: " << name << std::endl;	return false;
		}
		watersnd[i].SetBuffer(*buf);	watersnd[i].Set3DEffects(true);
		watersnd[i].SetLoop(false);		watersnd[i].SetGain(0);
	}
	{
		const SOUNDBUFFER * buf = soundbufferlibrary.GetBuffer("mud1");
		if (!buf)
		{	error_output << "Can't load mud sound " << std::endl;	return false;
		}
		mudsnd.SetBuffer(*buf);		mudsnd.Set3DEffects(true);
		mudsnd.SetLoop(false);		mudsnd.SetGain(0);
	}
	//set up fluid cont. sounds
	{
		const SOUNDBUFFER * buf = soundbufferlibrary.GetBuffer("mud_cont");
		if (!buf)
		{	error_output << "Can't load mud_cont sound" << std::endl;	return false;
		}
		mud_cont.SetBuffer(*buf);	mud_cont.Set3DEffects(true);
		mud_cont.SetLoop(true);		mud_cont.SetGain(0);
		mud_cont.SetPitch(1.0);		mud_cont.Play();
	}
	{
		const SOUNDBUFFER * buf = soundbufferlibrary.GetBuffer("water_cont");
		if (!buf)
		{	error_output << "Can't load water_cont sound" << std::endl;	return false;
		}
		water_cont.SetBuffer(*buf);	water_cont.Set3DEffects(true);
		water_cont.SetLoop(true);	water_cont.SetGain(0);
		water_cont.SetPitch(1.0);	water_cont.Play();
	}
	
	//set up boost sound
	{
		const SOUNDBUFFER * buf = soundbufferlibrary.GetBuffer("boost");
		if (!buf)
		{	error_output << "Can't load boost sound" << std::endl;	return false;
		}
		boostsnd.SetBuffer(*buf);	boostsnd.Set3DEffects(true);
		boostsnd.SetLoop(true);		boostsnd.SetGain(0);
		boostsnd.SetPitch(1.0);		boostsnd.Play();
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------------------
bool CAR::LoadInto(const std::string & joefile, MODEL_JOE03 & output_model,	std::ostream & error_output)
{
	if (!output_model.Loaded())
	{
		std::stringstream nullout;
		if (!output_model.ReadFromFile(joefile.substr(0,std::max((long unsigned int)0,(long unsigned int) joefile.size()-3))+"ova", nullout))
		{
			if (!output_model.Load(joefile, error_output))
			{
				/*error_output << "Error loading model: " << joefile << std::endl;*/
				return false;
			}
		}
	}
	return true;
}

void CAR::SetPosition(const MATHVECTOR <float, 3> & new_position)
{
	MATHVECTOR <double,3> newpos;
	newpos = new_position;
	dynamics.SetPosition(newpos);
	dynamics.AlignWithGround();//--

	QUATERNION <float> rot;
	rot = dynamics.GetOrientation();
}

void CAR::Update(double dt)
{
	dynamics.Update();
	UpdateSounds(dt);
}

void CAR::GetSoundList(std::list <SOUNDSOURCE *> & outputlist)
{
	for (std::list <std::pair <ENGINESOUNDINFO, SOUNDSOURCE> >::iterator
		i = enginesounds.begin(); i != enginesounds.end(); ++i)
		outputlist.push_back(&i->second);

	for (int i = 0; i < 4; i++)  outputlist.push_back(&tiresqueal[i]);
	for (int i = 0; i < 4; i++)  outputlist.push_back(&grasssound[i]);
	for (int i = 0; i < 4; i++)  outputlist.push_back(&gravelsound[i]);
	for (int i = 0; i < 4; i++)  outputlist.push_back(&tirebump[i]);

	for (int i = 0; i < Ncrashsounds; ++i)
		outputlist.push_back(&crashsound[i]);
	outputlist.push_back(&roadnoise);
	outputlist.push_back(&boostsnd);

	for (int i = 0; i < Nwatersounds; ++i)
		outputlist.push_back(&watersnd[i]);
	outputlist.push_back(&mudsnd);
	
	outputlist.push_back(&mud_cont);
	outputlist.push_back(&water_cont);
}

void CAR::GetEngineSoundList(std::list <SOUNDSOURCE *> & outputlist)
{
	for (std::list <std::pair <ENGINESOUNDINFO, SOUNDSOURCE> >::iterator i =
		enginesounds.begin(); i != enginesounds.end(); ++i)
	{
		outputlist.push_back(&i->second);
	}
}


///   Car Inputs  * * * * * * * 
//--------------------------------------------------------------------------------------------------------------------------
void CAR::HandleInputs(const std::vector <float> & inputs, float dt)
{
	assert(inputs.size() == CARINPUT::ALL); //-
	if (pApp && pApp->IsFocGuiInput())
		return;

	dynamics.inputsCopy = inputs;

	int cur_gear = dynamics.GetTransmission().GetGear();
	bool rear = pSet->rear_inv ? cur_gear == -1 : false;  //if (disable_auto_rear)  rear = false;

	//  set brakes
	float brake = !rear ? inputs[CARINPUT::BRAKE] : inputs[CARINPUT::THROTTLE];
	dynamics.SetBrake(brake);
	dynamics.SetHandBrake(inputs[CARINPUT::HANDBRAKE]);
	
	//  boost, flip over
	dynamics.doBoost = inputs[CARINPUT::BOOST];
	dynamics.doFlip = inputs[CARINPUT::FLIP];

	//  steering
	float steer_value = inputs[CARINPUT::STEER_RIGHT];
	if (std::abs(inputs[CARINPUT::STEER_LEFT]) > std::abs(inputs[CARINPUT::STEER_RIGHT])) //use whichever control is larger
		steer_value = -inputs[CARINPUT::STEER_LEFT];
	dynamics.SetSteering(steer_value);
	last_steer = steer_value;

    //start the engine if requested
	//if (inputs[CARINPUT::START_ENGINE])
	//	dynamics.StartEngine();

	//  shifting
	int gear_change = 0;
	if (inputs[CARINPUT::SHIFT_UP] == 1.0)		gear_change = 1;
	if (inputs[CARINPUT::SHIFT_DOWN] == 1.0)	gear_change = -1;
	int new_gear = cur_gear + gear_change;

	/*if (inputs[CARINPUT::REVERSE])	new_gear = -1;
	if (inputs[CARINPUT::FIRST_GEAR])	new_gear = 1;*/

	//  throttle
	float throttle = !rear ? inputs[CARINPUT::THROTTLE] : inputs[CARINPUT::BRAKE];
	float clutch = 1 - inputs[CARINPUT::CLUTCH]; // 

	dynamics.ShiftGear(new_gear);
	dynamics.SetThrottle(throttle);
	dynamics.SetClutch(clutch);

	//  abs tcs
	///TODO: car setup separated for all (4) players: (auto shift, auto rear, rear inv, abs, tcs)  ...
	//if (inputs[CARINPUT::ABS_TOGGLE])	dynamics.SetABS(!dynamics.GetABSEnabled());
	//if (inputs[CARINPUT::TCS_TOGGLE])	dynamics.SetTCS(!dynamics.GetTCSEnabled());
	
	//  cam, chk
	iCamNext = -inputs[CARINPUT::PREV_CAM] + inputs[CARINPUT::NEXT_CAM];
	bLastChk = inputs[CARINPUT::LAST_CHK];
}


//--------------------------------------------------------------------------------------------------------------------------
void CAR::UpdateSounds(float dt)
{
	float rpm, throttle, speed, dynVel;
	MATHVECTOR <float,3> engPos, whPos[4];  // engine, wheels pos
	TRACKSURFACE::TYPE surfType[4];
	float squeal[4],whVel[4], suspVel[4],suspDisp[4];
	float whH_all = 0.f;  bool mud = false;
	
	///  replay play  ------------------------------------------
	if (pApp->bRplPlay)
	{
		rpm = pApp->fr.rpm;
		throttle = pApp->fr.throttle;
		engPos = pApp->fr.posEngn;  // _/could be from car pos,rot and engine offset--
		speed = pApp->fr.speed;
		dynVel = pApp->fr.dynVel;

		for (int w=0; w<4; ++w)
		{
			whPos[w] = pApp->fr.whPos[w];
			surfType[w] = (TRACKSURFACE::TYPE)pApp->fr.surfType[w];
			//  squeal
			squeal[w] = pApp->fr.squeal[w];
			whVel[w] = pApp->fr.whVel[w];
			//  susp
			suspVel[w] = pApp->fr.suspVel[w];
			suspDisp[w] = pApp->fr.suspDisp[w];
			//  fluids
			whH_all += pApp->fr.whH[w];
			if (pApp->fr.whP[w] >= 1)  mud = true;
			//?- whMudSpin = pApp->fr.whMudSpin;
		}
	}
	else  /// game  ------------------------------------------
	{
		rpm = GetEngineRPM();
		throttle = dynamics.GetEngine().GetThrottle();
		engPos = dynamics.GetEnginePosition();
		speed = GetSpeed();
		dynVel = dynamics.GetVelocity().Magnitude();
		
		for (int w=0; w<4; ++w)
		{
			WHEEL_POSITION wp = WHEEL_POSITION(w);
			whPos[w] = dynamics.GetWheelPosition(wp);

			const TRACKSURFACE* surface = dynamics.GetWheelContact(wp).GetSurfacePtr();
			surfType[w] = !surface ? TRACKSURFACE::NONE : surface->type;
			//  squeal
			squeal[w] = GetTireSquealAmount(wp);
			whVel[w] = dynamics.GetWheelVelocity(wp).Magnitude();
			//  susp
			suspVel[w] = dynamics.GetSuspension(wp).GetVelocity();
			suspDisp[w] = dynamics.GetSuspension(wp).GetDisplacementPercent();
			//  fluids
			whH_all += dynamics.whH[w];
			if (dynamics.whP[w] >= 1)  mud = true;
		}
	}
	///  ------------------------------------------

	//update engine sounds
	float total_gain = 0.0, loudest = 0.0;
	std::list <std::pair <SOUNDSOURCE *, float> > gainlist;

	for (std::list <std::pair <ENGINESOUNDINFO, SOUNDSOURCE> >::iterator i = enginesounds.begin(); i != enginesounds.end(); ++i)
	{
		ENGINESOUNDINFO & info = i->first;
		SOUNDSOURCE & sound = i->second;

		float gain = 1.0;

		if (rpm < info.minrpm)	gain = 0;
		else if (rpm < info.fullgainrpmstart && info.fullgainrpmstart > info.minrpm)
			gain *= (rpm - info.minrpm)/(info.fullgainrpmstart-info.minrpm);

		if (rpm > info.maxrpm)	gain = 0;
		else if (rpm > info.fullgainrpmend && info.fullgainrpmend < info.maxrpm)
			gain *= 1.0-(rpm - info.fullgainrpmend)/(info.maxrpm-info.fullgainrpmend);

		if (info.power == ENGINESOUNDINFO::BOTH)
			gain *= throttle * 0.5 + 0.5;
		else if (info.power == ENGINESOUNDINFO::POWERON)
			gain *= throttle;
		else if (info.power == ENGINESOUNDINFO::POWEROFF)
			gain *= (1.0-throttle);

		total_gain += gain;
		if (gain > loudest)  loudest = gain;
		gainlist.push_back(std::pair <SOUNDSOURCE*, float> (&sound, gain));

		float pitch = rpm / info.naturalrpm;
		sound.SetPitch(pitch);
		sound.SetPosition(engPos[0], engPos[1], engPos[2]);
	}

	//normalize gains engine
	//assert(total_gain >= 0.0);
	for (std::list <std::pair <SOUNDSOURCE *, float> >::iterator i = gainlist.begin(); i != gainlist.end(); ++i)
	{
		if (total_gain == 0.0)
			i->first->SetGain(0.0);
		else if (enginesounds.size() == 1 && enginesounds.back().first.power == ENGINESOUNDINFO::BOTH)
			i->first->SetGain(i->second * pSet->vol_engine);
		else
			i->first->SetGain(i->second/total_gain * pSet->vol_engine);

		//if (i->second == loudest) std::cout << i->first->GetSoundBuffer().GetName() << ": " << i->second << std::endl;
	}

	//update tire squeal sounds
	for (int i = 0; i < 4; i++)
	{
		// make sure we don't get overlap
		gravelsound[i].SetGain(0.0);
		grasssound[i].SetGain(0.0);
		tiresqueal[i].SetGain(0.0);

		float maxgain = 0.6, pitchvariation = 0.4;

		SOUNDSOURCE * thesound;
		switch (surfType[i])
		{
		case TRACKSURFACE::NONE:		thesound = tiresqueal;	maxgain = 0.0;	break;
		case TRACKSURFACE::ASPHALT:		thesound = tiresqueal;	break;
		case TRACKSURFACE::GRASS:		thesound = grasssound;	maxgain = 0.7;	pitchvariation = 0.25;	break;
		case TRACKSURFACE::GRAVEL:		thesound = gravelsound;	maxgain = 0.7;	break;
		case TRACKSURFACE::CONCRETE:	thesound = tiresqueal;	maxgain = 0.6;	pitchvariation = 0.25;	break;
		case TRACKSURFACE::SAND:		thesound = grasssound;	maxgain = 0.5;  pitchvariation = 0.25;	break;
						default:		thesound = tiresqueal;	maxgain = 0.0;	break;
		}

		float pitch = (whVel[i]-5.0)*0.1;
		if (pitch < 0)	pitch = 0;
		if (pitch > 1)	pitch = 1;
		pitch = 1.0 - pitch;
		pitch *= pitchvariation;
		pitch = pitch + (1.0-pitchvariation);
		if (pitch < 0.1)	pitch = 0.1;
		if (pitch > 4.0)	pitch = 4.0;

		thesound[i].SetPosition(whPos[i][0], whPos[i][1], whPos[i][2]);
		thesound[i].SetGain(squeal[i]*maxgain * pSet->vol_tires);
		thesound[i].SetPitch(pitch);
	}

	//update road noise sound -wind
	{
		float gain = dynVel;//dynamics.GetVelocity().Magnitude();
		if (gain < 0)	gain = -gain;
		gain *= 0.02;	gain *= gain;
		if (gain > 1.0)	gain = 1.0;
		roadnoise.SetGain(gain * pSet->vol_env);
		roadnoise.SetPosition(engPos[0], engPos[1], engPos[2]); //
		//std::cout << gain << std::endl;
	}

	//update susp bump sound
	for (int i = 0; i < 4; i++)
	{
		suspbump[i].Update(suspVel[i], suspDisp[i], dt);
		if (suspbump[i].JustSettled())
		{
			float bumpsize = suspbump[i].GetTotalBumpSize();

			const float breakevenms = 5.0;
			float gain = bumpsize * speed / breakevenms;
			if (gain > 1)	gain = 1;
			if (gain < 0)	gain = 0;

			if (gain > 0 && !tirebump[i].Audible())
			{
				tirebump[i].SetGain(gain * pSet->vol_env);
				tirebump[i].SetPosition(whPos[i][0], whPos[i][1], whPos[i][2]);
				tirebump[i].Stop();
				tirebump[i].Play();
			}
		}
	}
	
	//update fluids sound - hit
	bool fluidHit = whH_all > 1.f;
	//LogO(toStr(whH_all) + "  v "+ toStr(dynVel));

	if (fluidHit && !fluidHitOld)
	//if (dynVel > 10.f && whH_all > 1.f && )
	{
		int i = std::min(Nwatersounds-1, (int)(dynVel / 15.f));
		float gain = std::min(3.0f, 0.3f + dynVel / 30.f);
		SOUNDSOURCE& snd = /*mud ? mudsnd : */watersnd[i];
		
		//LogO("fluid hit i"+toStr(i)+" g"+toStr(gain)+" "+(mud?"mud":"wtr"));
		if (!snd.Audible())
		{
			snd.SetGain(gain * pSet->vol_env);
			snd.SetPosition(engPos[0], engPos[1], engPos[2]);
			snd.Stop();
			snd.Play();
		}

		if (mud)  {
		SOUNDSOURCE& snd = mudsnd;
		if (!snd.Audible())
		{
			snd.SetGain(gain * pSet->vol_env);
			snd.SetPosition(engPos[0], engPos[1], engPos[2]);
			snd.Stop();
			snd.Play();
		}	}
	}
	fluidHitOld = fluidHit;

	//update fluids sound - continuous
	{
		float vel = mud && whH_all > 0.1f ?
			whMudSpin * 2.5f : 0.f;
		mud_cont.SetGain(std::min(1.f, vel) * pSet->vol_env);
		mud_cont.SetPitch(std::max(0.7f, std::min(3.f, vel * 0.35f)));
		mud_cont.SetPosition(engPos[0], engPos[1], engPos[2]);
	}
	{
		float vel = !mud && whH_all > 0.1f && whH_all < 3.9f ?
			dynVel / 30.f : 0.f;
		water_cont.SetGain(std::min(1.f, vel * 1.5f) * pSet->vol_env);
		water_cont.SetPitch(std::max(0.7f, std::min(1.3f, vel)));
		water_cont.SetPosition(engPos[0], engPos[1], engPos[2]);
	}
	
	//update boost sound
	{
		float gain = dynamics.boostVal;
		boostsnd.SetGain(gain * 0.57f * pSet->vol_engine);
		boostsnd.SetPosition(engPos[0], engPos[1], engPos[2]); //back?-
	}
	
	//update crash sound
	#if 0
	if (dynamics.bHitSnd)// && dynamics.sndHitN >= 0)
	{
		int f = dynamics.fParIntens * 0.04f;  //fSndForce * 0.1f;
		int i = std::max(1, std::min(Ncrashsounds, f));
		//int i = std::max(1, std::min(Ncrashsounds-1, dynamics.sndHitN));
		//int i = Ncrashsounds-2;
		float ti = 1.8f - i*0.4f;  if (ti < 0.4f)  ti = 0.4f;
		if (dynamics.bHitSnd)// && crashsoundtime[i] > /*ti*/0.2f)  //par  //&& !crashsound[i].Audible()
		{	dynamics.bHitSnd = false;
			crashsound[i].SetGain(1 * pSet->vol_env);
			crashsound[i].SetPosition(engPos[0], engPos[1], engPos[2]); //
			crashsound[i].Stop();
			crashsound[i].Play();
			crashsoundtime[i] = 0.f;
			dynamics.sndHitN = -1;
			//LogO("Snd:  i " + toStr(i) + "  parF " + toStr(dynamics.fParIntens) + "  sndF " + toStr(dynamics.fSndForce));
		}/**/
	}
	#endif
	//#else
	//update crash sound
	{
		crashdetection.Update(speed, dt);
		float crashdecel = crashdetection.GetMaxDecel();
		if (crashdecel > 0)
		{
			const float mingainat = 0;  // 40 260
			const float maxgainat = 160;
			const float mingain = 0.1;
			float gain = (crashdecel-mingainat)/(maxgainat-mingainat);
			if (gain > 1)		gain = 1;
			if (gain < mingain)	gain = mingain;

			//int f = (normvel*0.02f + 0.02f*vlen) * Ncrashsounds;
			//int i = std::max(5, std::min(Ncrashsounds, f));
			//cd->bHitSnd = true;//cd->fSndForce > 58;  //true;
			//cd->sndHitN = i;
			//std::cout << crashdecel << ", gain: " << gain << std::endl;
			
			int f = crashdecel / 1400.f * Ncrashsounds;
			int i = std::max(1, std::min(Ncrashsounds-1, f));

			//if (crashsoundtime[i] > /*ti*/0.4f)  //!crashsound.Audible())
			{
				crashsound[i].SetGain(gain * pSet->vol_env);
				crashsound[i].SetPosition(engPos[0], engPos[1], engPos[2]); //
				crashsound[i].Stop();
				crashsound[i].Play();
				crashsoundtime[i] = 0.f;
			}
			//LogO("Car Snd: " + toStr(crashdecel));// + " force " + toStr(hit.force) + " vel " + toStr(vlen) + " Nvel " + toStr(normvel));
		}
	}
	//#endif

	//  time played
	for (int i=0; i < Ncrashsounds; ++i)
		if (crashsoundtime[i] < 5.f)
			crashsoundtime[i] += dt;
}


//--------------------------------------------------------------------------------------------------------------------------
float CAR::GetFeedback()
{
	return dynamics.GetFeedback() / (mz_nominalmax * 0.025);
}

float CAR::GetTireSquealAmount(WHEEL_POSITION i, float* slide, float* s1, float* s2) const
{
	const TRACKSURFACE* surface = dynamics.GetWheelContact(WHEEL_POSITION(i)).GetSurfacePtr();
	if (!surface)  return 0;
	if (surface->type == TRACKSURFACE::NONE)
		return 0;
		
	float d = dynamics.GetWheelContact(WHEEL_POSITION(i)).GetDepth() - 2*GetTireRadius(WHEEL_POSITION(i));
	bool inAir = d > 0.f;  //d > 0.9f || d < 0.f;  //*!
	//if (onGround)  *onGround = !inAir;
	if (inAir)  // not on ground 
		return 0;

	MATHVECTOR <float, 3> groundvel;
	groundvel = dynamics.GetWheelVelocity(WHEEL_POSITION(i));
	QUATERNION <float> wheelspace;
	wheelspace = dynamics.GetUprightOrientation(WHEEL_POSITION(i));
	(-wheelspace).RotateVector(groundvel);
	float wheelspeed = dynamics.GetWheel(WHEEL_POSITION(i)).GetAngularVelocity()*dynamics.GetTire(WHEEL_POSITION(i)).GetRadius();
	groundvel[0] -= wheelspeed;
	groundvel[1] *= 2.0;
	groundvel[2] = 0;
	float squeal = (groundvel.Magnitude() - 3.0) * 0.2;
	if (slide)  *slide = squeal + 0.6;

	std::pair <double, double> slideslip = dynamics.GetTire(i).GetSlideSlipRatios();
	if (s1)  *s1 = slideslip.first;
	if (s2)  *s2 = slideslip.second;
	double maxratio = std::max(std::abs(slideslip.first), std::abs(slideslip.second));
	float squealfactor = std::max(0.0, maxratio - 1.0);
	squeal *= squealfactor;
	if (squeal < 0)  squeal = 0;
	if (squeal > 1)  squeal = 1;
	return squeal;
}

void CAR::EnableGlass(bool enable)
{
	/*if (glassdraw)
	{
		glassdraw->SetDrawEnable(enable);
	}*/
}

bool CAR::Serialize(joeserialize::Serializer & s)
{
	_SERIALIZE_(s,dynamics);
	_SERIALIZE_(s,last_steer);
	return true;
}

protocol::CarStatePackage CAR::GetCarStatePackage() const
{
	protocol::CarStatePackage csp;
	csp.pos = ToMathVector<float>(dynamics.chassis->getCenterOfMassPosition());
	csp.rot = ToMathQuaternion<float>(dynamics.chassis->getCenterOfMassTransform().getRotation());
	csp.linearVel = GetVelocity();
	csp.angularVel = GetAngularVelocity();
	return csp;
}

void CAR::UpdateCarState(const protocol::CarStatePackage& state)
{
	// Velocity based estimation from physics engine works rather well
	// for a while, so we use pos/rot only for lazy corrections
	MATHVECTOR<float,3> curpos = ToMathVector<float>(dynamics.chassis->getCenterOfMassPosition());
	MATHVECTOR<float,3> errorvec = state.pos - curpos;
	MATHVECTOR<float,3> newpos = curpos + (errorvec * 0.05f);
	QUATERNION<float> currot = ToMathQuaternion<float>(dynamics.chassis->getCenterOfMassTransform().getRotation());
	QUATERNION<float> newrot = currot.QuatSlerp(state.rot, 0.5f);

	// If the estimate drifts too far for some reason, do a quick correction
	if (errorvec.MagnitudeSquared() > 9.0f) {
		newpos = state.pos;
		newrot = state.rot;
	}

	SetPosition(newpos);

	btTransform transform;
	transform.setOrigin(ToBulletVector(newpos));
	transform.setRotation(ToBulletQuaternion(newrot));
	dynamics.chassis->setWorldTransform(transform);

	// No interpolation in velocities
	dynamics.chassis->setLinearVelocity(ToBulletVector(state.linearVel));
	dynamics.chassis->setAngularVelocity(ToBulletVector(state.angularVel));

	dynamics.SynchronizeBody();  // set body from chassis
	dynamics.UpdateWheelContacts();
}

///  reset car, pos and state
void CAR::ResetPos(bool fromStart)
{
	MATHVECTOR <float, 3> pos = fromStart ? posAtStart : posLastCheck;
	QUATERNION <float> rot = fromStart ? rotAtStart : rotLastCheck;
	SetPosition(pos);

	btTransform transform;
	transform.setOrigin(ToBulletVector(pos));
	transform.setRotation(ToBulletQuaternion(rot));
	dynamics.chassis->setWorldTransform(transform);

	dynamics.chassis->setLinearVelocity(btVector3(0,0,0));
	dynamics.chassis->setAngularVelocity(btVector3(0,0,0));

	dynamics.SynchronizeBody();  // set body from chassis
	if (fromStart)  // restore boost fuel
		dynamics.boostFuel = gfBoostFuelStart;

	//  engine, wheels
	dynamics.engine.SetInitialConditions();
	for (int w=0; w < 4; ++w)
	{
		MATHVECTOR <CARDYNAMICS::T, 3> zero(0,0,0);
		dynamics.wheel[w].SetAngularVelocity(0);
		//dynamics.wheel_velocity[w] = zero;
	}
	crashdetection.Update(0.f, 0.1f);  //prevent car hit sound

	//dynamics.SynchronizeChassis();
	dynamics.UpdateWheelContacts();
}

///  save car pos and rot
void CAR::SavePosAtCheck()
{
	dynamics.body.GetPosition();
	posLastCheck = dynamics.body.GetPosition();
	rotLastCheck = dynamics.body.GetOrientation();
	//MATHVECTOR <float, 3> pos = fromStart ? posAtStart : posLastCheck;
	//QUATERNION <float> rot = fromStart ? rotAtStart : rotLastCheck;

	//btTransform transform;
	//dynamics.chassis->getWorldTransform();
	//transform.setOrigin(ToBulletVector(pos));
	//transform.setRotation(ToBulletQuaternion(rot));
	//dynamics.chassis->setWorldTransform(transform);
}
