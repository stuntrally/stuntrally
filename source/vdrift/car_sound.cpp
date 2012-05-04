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
#include "../ogre/CarModel.h"  //+ camera pos
#include "../ogre/FollowCamera.h"  //+ camera pos
#include "../ogre/common/Defines.h"
#include "../ogre/common/GraphView.h"
#include "../network/protocol.hpp"
#include "tobullet.h"
#include <OgreLogManager.h>
#include "game.h"  //sound

#ifdef _WIN32
bool isnan(float number) {return (number != number);}
bool isnan(double number) {return (number != number);}
#endif


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
				++inext;
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


//--------------------------------------------------------------------------------------------------------------------------
void CAR::UpdateSounds(float dt)
{
	float rpm, throttle, speed, dynVel;
	MATHVECTOR <float,3> pos, engPos, whPos[4], hitPos;  // car, engine, wheels pos
	QUATERNION <float> rot;
	TRACKSURFACE::TYPE surfType[4];
	float squeal[4],whVel[4], suspVel[4],suspDisp[4];
	float whH_all = 0.f;  bool mud = false;
	float fHitForce = 0.f, boostVal = 0.f;
	
	///  replay play  ------------------------------------------
	if (pApp->bRplPlay)
	{
		const ReplayFrame& fr = pApp->frm[id];
		pos = fr.pos;  rot = fr.rot;
		rpm = fr.rpm;
		throttle = fr.throttle;
		engPos = fr.posEngn;  // _/could be from car pos,rot and engine offset--
		speed = fr.speed;
		dynVel = fr.dynVel;
		whMudSpin = fr.whMudSpin;
		fHitForce = fr.fHitForce;
		hitPos[0] = fr.vHitPos.x;  hitPos[1] = -fr.vHitPos.z;  hitPos[2] = fr.vHitPos.y;
		boostVal = fr.fboost;

		for (int w=0; w<4; ++w)
		{
			whPos[w] = fr.whPos[w];
			surfType[w] = (TRACKSURFACE::TYPE)fr.surfType[w];
			//  squeal
			squeal[w] = fr.squeal[w];
			whVel[w] = fr.whVel[w];
			//  susp
			suspVel[w] = fr.suspVel[w];
			suspDisp[w] = fr.suspDisp[w];
			//  fluids
			whH_all += fr.whH[w];
			if (fr.whP[w] >= 1)  mud = true;
		}
	}
	else  /// game  ------------------------------------------
	{
		pos = dynamics.GetPosition();  rot = dynamics.GetOrientation();
		rpm = GetEngineRPM();
		throttle = dynamics.GetEngine().GetThrottle();
		engPos = dynamics.GetEnginePosition();
		speed = GetSpeed();
		dynVel = dynamics.GetVelocity().Magnitude();
		fHitForce = dynamics.fHitForce;
		hitPos[0] = dynamics.vHitPos.x;  hitPos[1] = -dynamics.vHitPos.z;  hitPos[2] = dynamics.vHitPos.y;
		boostVal = dynamics.boostVal;
		
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

		//  wheels in mud, spinning intensity
		float mudSpin = 0.f;
		for (int w=0; w < 4; ++w)
		{
			float vel = std::abs(dynamics.wheel[w].GetAngularVelocity());
			if (vel <= 30.f)  continue;
			if (dynamics.whP[w] == 2)
				mudSpin += dynamics.whH[w] * std::min(80.f, 1.5f * vel) / 80.f;
			else if (dynamics.whP[w] == 1)
				mudSpin += dynamics.whH[w] * std::min(160.f, 3.f * vel) / 80.f;
		}
		whMudSpin = mudSpin * 0.5f;
	}
	
	///  listener  ------------------------------------------
	if (!bRemoteCar)
	{
		MATHVECTOR <float,3> campos = pos;
		QUATERNION <float> camrot;// = rot;
		using namespace Ogre;
		if (pCarM && pCarM->fCam)
		{	//  pos
			Vector3 cp = pCarM->fCam->camPosFinal;
			campos[0] = cp.x;  campos[1] =-cp.z;  campos[2] = cp.y;
			//  rot
			Quaternion rr = pCarM->fCam->mCamera->getOrientation() * Object::qrFix2;
			Quaternion b(-rr.w,  -rr.x, -rr.y, -rr.z);
			//  fix pan
			b = b * Quaternion(Degree(90.f), Vector3::UNIT_X) * Quaternion(Degree(-90.f), Vector3::UNIT_Z);
			camrot[0] = b.x;  camrot[1] = b.y;  camrot[2] = b.z;  camrot[3] = b.w;
		}
		if (pApp->pGame->sound.Enabled())
			pApp->pGame->sound.SetListener(campos, camrot, MATHVECTOR <float,3>());

		bool incar = false;//
		std::list <SOUNDSOURCE *> soundlist;
		GetEngineSoundList(soundlist);
		for (std::list <SOUNDSOURCE *>::iterator s = soundlist.begin(); s != soundlist.end(); s++)
			(*s)->Set3DEffects(!incar);
	}

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
		float gain = boostVal;
		boostsnd.SetGain(gain * 0.57f * pSet->vol_engine);
		boostsnd.SetPosition(engPos[0], engPos[1], engPos[2]); //back?-
	}
	
	//update crash sound
	#if 1
	/*if (fHitForce > 0.5f)
	{
		int f = fHitForce * 11.f;
		int i = std::max(1, std::min(Ncrashsounds-1, f));
		float ti = 1.8f - i*0.4f;  if (ti < 0.4f)  ti = 0.4f;
		
		//if (crashsoundtime[i] > ti)  //par  //&& !crashsound[i].Audible()
		{
			crashsound[i].SetGain(1 * pSet->vol_env);
			crashsound[i].SetPosition(engPos[0], engPos[1], engPos[2]); //
			crashsound[i].Stop();
			crashsound[i].Play();
			crashsoundtime[i] = 0.f;
			//LogO("Snd:  i " + toStr(i) + "  parF " + toStr(dynamics.fParIntens) + "  sndF " + toStr(dynamics.fSndForce));
		}
	}
	#else*/
	//update crash sound
	{
		crashdetection.Update(speed, dt);
		float crashdecel = crashdetection.GetMaxDecel();
		//dynamics.fHitForce4 = crashdecel / 1400.f;
		//todo: ^for old replays..  set blt car pos,rot in rpl for objs..

		crashdetection2.Update(-fHitForce, dt);
		crashdetection2.deceltrigger = 1.f;
		float crashdecel2 = crashdetection2.GetMaxDecel();
		dynamics.fHitForce3 = crashdecel2 / 30.f;

		if (crashdecel2 > 0)
		{
			//const float mingainat = 1;  // 40 260
			//const float maxgainat = 15;
			//const float mingain = 0.1;
			//float gain = (crashdecel2-mingainat)/(maxgainat-mingainat);
			//if (gain > 1)		gain = 1;
			//if (gain < mingain)	gain = mingain;
			/*!*/float gain = 0.9f;
			
			int f = crashdecel2 / 30.f * Ncrashsounds;
			int i = std::max(1, std::min(Ncrashsounds-1, f));

			if (/*gain > mingain &&*/ crashsoundtime[i] > /*ti*/0.4f)  //!crashsound.Audible())
			{
				crashsound[i].SetGain(gain * pSet->vol_env);
				crashsound[i].SetPosition(hitPos[0], hitPos[1], hitPos[2]);  //..
				crashsound[i].Stop();
				crashsound[i].Play();
				crashsoundtime[i] = 0.f;
			}
			//LogO("Car Snd: " + toStr(crashdecel));// + " force " + toStr(hit.force) + " vel " + toStr(vlen) + " Nvel " + toStr(normvel));
		}
	}
	#endif

	//  time played
	for (int i=0; i < Ncrashsounds; ++i)
		if (crashsoundtime[i] < 5.f)
			crashsoundtime[i] += dt;
}

