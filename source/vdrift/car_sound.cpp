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
#include "game.h"  //sound


//--------------------------------------------------------------------------------------------------------------------------
bool CAR::LoadSounds(
	const std::string & carpath,
	const std::string & carname,
	const SOUNDINFO & sound_device_info,
	const SOUNDBUFFERLIBRARY & sndLib,
	std::ostream & info_output,
	std::ostream & errOut)
{
	//check for sound specification file
	CONFIGFILE aud;
	if (aud.Load(carpath+"/"+carname+"/"+carname+".aud"))  // ?
	{
		std::list <std::string> sections;
		aud.GetSectionList(sections);
		for (std::list <std::string>::iterator i = sections.begin(); i != sections.end(); ++i)
		{
			//load the buffer
			std::string filename;
			if (!aud.GetParam(*i+".filename", filename, errOut)) return false;
			if (!soundbuffers[filename].GetLoaded())
				if (!soundbuffers[filename].Load(carpath+"/"+carname+"/"+filename, sound_device_info, errOut))
				{
					errOut << "Error loading sound: " << carpath+"/"+carname+"/"+filename << std::endl;
					return false;
				}

			enginesounds.push_back(std::pair <ENGINESOUNDINFO, SOUNDSOURCE> ());
			ENGINESOUNDINFO & info = enginesounds.back().first;
			SOUNDSOURCE & sound = enginesounds.back().second;

			if (!aud.GetParam(*i+".MinimumRPM", info.minrpm, errOut)) return false;
			if (!aud.GetParam(*i+".MaximumRPM", info.maxrpm, errOut)) return false;
			if (!aud.GetParam(*i+".NaturalRPM", info.naturalrpm, errOut)) return false;

			std::string powersetting;
			if (!aud.GetParam(*i+".power", powersetting, errOut)) return false;
			if (powersetting == "on")
				info.power = ENGINESOUNDINFO::POWERON;
			else if (powersetting == "off")
				info.power = ENGINESOUNDINFO::POWEROFF;
			else //assume it's used in both ways
				info.power = ENGINESOUNDINFO::BOTH;

			sound.Setup(soundbuffers[filename], true, true, 0.f);
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
	else  // car engine
	{
		if (!soundbuffers["engine.wav"].Load(carpath+"/"+carname+"/engine.wav", sound_device_info, errOut))
		{
			errOut << "Unable to load engine sound: "+carpath+"/"+carname+"/engine.wav" << std::endl;
			return false;
		}
		enginesounds.push_back(std::pair <ENGINESOUNDINFO, SOUNDSOURCE> ());
		SOUNDSOURCE & enginesound = enginesounds.back().second;
		enginesound.Setup(soundbuffers["engine.wav"], true, true, 0.f);
		enginesound.Play();
	}

	int i;
	for (i = 0; i < 4; ++i)  // tires
	{
		if (!tiresqueal[i]	.Setup(sndLib, "tire_squeal",	errOut,  true, true, 0.f))  return false;	tiresqueal[i].Seek4(i);
		if (!gravelsound[i]	.Setup(sndLib, "gravel",		errOut,  true, true, 0.f))  return false;	gravelsound[i].Seek4(i);
		if (!grasssound[i]	.Setup(sndLib, "grass",			errOut,  true, true, 0.f))  return false;	grasssound[i].Seek4(i);

		if (!tirebump[i].Setup(sndLib, i >= 2 ? "bump_rear" : "bump_front", errOut,  true, false,1.f))  return false;
	}

	for (i = 1; i <= Ncrashsounds; ++i)  // crashes
		if (!crashsound[i-1].Setup(sndLib, toStr(i/10)+toStr(i%10), errOut,  true, false,1.f))  return false;

	if (!crashscrap  .Setup(sndLib, "scrap",	errOut,  true, true, 0.f))  return false;  crashscrap.Play();
	if (!crashscreech.Setup(sndLib, "screech",	errOut,  true, true, 0.f))  return false;  crashscreech.Play();

	if (!roadnoise	.Setup(sndLib, "wind",		 errOut,  true, true, 0.f))  return false;  roadnoise.Play();
	if (!boostsnd	.Setup(sndLib, "boost",		 errOut,  true, true, 0.f))  return false;  boostsnd.Play();

	for (i = 0; i < Nwatersounds; ++i)  // fluids
		if (!watersnd[i].Setup(sndLib, "water"+toStr(i+1), errOut,  true, false,0.f))  return false;

	if (!mudsnd		.Setup(sndLib, "mud1",		 errOut,  true, false,0.f))  return false;
	if (!mud_cont	.Setup(sndLib, "mud_cont",	 errOut,  true, true, 0.f))  return false;  mud_cont.Play();
	if (!water_cont	.Setup(sndLib, "water_cont", errOut,  true, true, 0.f))  return false;  water_cont.Play();
	
	return true;
}


//--------------------------------------------------------------------------------------------------------------------------
void CAR::GetSoundList(std::list <SOUNDSOURCE *> & li)
{
	for (std::list <std::pair <ENGINESOUNDINFO, SOUNDSOURCE> >::iterator
		i = enginesounds.begin(); i != enginesounds.end(); ++i)
		li.push_back(&i->second);

	for (int i = 0; i < 4; ++i)  li.push_back(&tiresqueal[i]);
	for (int i = 0; i < 4; ++i)  li.push_back(&grasssound[i]);
	for (int i = 0; i < 4; ++i)  li.push_back(&gravelsound[i]);
	for (int i = 0; i < 4; ++i)  li.push_back(&tirebump[i]);

	for (int i = 0; i < Ncrashsounds; ++i)
		li.push_back(&crashsound[i]);
	li.push_back(&crashscrap);
	li.push_back(&crashscreech);

	li.push_back(&roadnoise);
	li.push_back(&boostsnd);

	for (int i = 0; i < Nwatersounds; ++i)
		li.push_back(&watersnd[i]);
	li.push_back(&mudsnd);
	
	li.push_back(&mud_cont);
	li.push_back(&water_cont);
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

	// engine
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
		sound.SetPosition(engPos);
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

	// tire squeal
	for (int i = 0; i < 4; i++)
	{
		// make sure we don't get overlap
		gravelsound[i].SetGain(0.0);
		grasssound[i].SetGain(0.0);
		tiresqueal[i].SetGain(0.0);

		float maxgain = 0.6, pitchvar = 0.4;

		SOUNDSOURCE * snd;
		switch (surfType[i])
		{
		case TRACKSURFACE::NONE:		snd = tiresqueal;	maxgain = 0.0;	break;
		case TRACKSURFACE::ASPHALT:		snd = tiresqueal;	break;
		case TRACKSURFACE::GRASS:		snd = grasssound;	maxgain = 0.7;	pitchvar = 0.25;	break;
		case TRACKSURFACE::GRAVEL:		snd = gravelsound;	maxgain = 0.7;	break;
		case TRACKSURFACE::CONCRETE:	snd = tiresqueal;	maxgain = 0.6;	pitchvar = 0.25;	break;
		case TRACKSURFACE::SAND:		snd = grasssound;	maxgain = 0.5;  pitchvar = 0.25;	break;
						default:		snd = tiresqueal;	maxgain = 0.0;	break;
		}	///more.. sand,snow,grass-new,mud..

		float pitch = std::min(1.f, std::max(0.f, (whVel[i]-5.0f)*0.1f ));
		pitch = 1.0 - pitch;
		pitch *= pitchvar;
		pitch = pitch + (1.f - pitchvar);
		pitch = std::min(4.f, std::max(0.1f, pitch ));

		snd[i].SetPosition(whPos[i]);
		snd[i].SetGain(squeal[i]*maxgain * pSet->vol_tires);
		snd[i].SetPitch(pitch);
	}

	//  wind
	{
		float gain = dynVel;
		if (gain < 0)	gain = -gain;
		gain *= 0.02;	gain *= gain;
		if (gain > 1.0)	gain = 1.0;
		roadnoise.SetGain(gain * pSet->vol_env);
		roadnoise.SetPosition(engPos); //
		//std::cout << gain << std::endl;
	}

	//  susp bump
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
				tirebump[i].SetGain(gain * pSet->vol_susp);
				tirebump[i].SetPosition(whPos[i]);
				tirebump[i].StopPlay();
			}
		}
	}
	
	//  fluids - hit
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
			snd.SetGain(gain * pSet->vol_fl_splash * (mud ? 0.6f : 1.f));
			snd.SetPosition(engPos);
			snd.StopPlay();
		}

		if (mud)  {  SOUNDSOURCE& snd = mudsnd;
		if (!snd.Audible())
		{
			snd.SetGain(gain * pSet->vol_fl_splash);
			snd.SetPosition(engPos);
			snd.StopPlay();
		}	}
	}
	fluidHitOld = fluidHit;

	//  fluids - continuous
	float velM = mud && whH_all > 0.1f ?
		whMudSpin * 2.5f : 0.f;
	mud_cont.SetGain(std::min(1.f, velM) * pSet->vol_fl_cont * 0.85f);
	mud_cont.SetPitch(std::max(0.7f, std::min(3.f, velM * 0.35f)));
	mud_cont.SetPosition(engPos);

	float velW = !mud && whH_all > 0.1f && whH_all < 3.9f ?
		dynVel / 30.f : 0.f;
	water_cont.SetGain(std::min(1.f, velW * 1.5f) * pSet->vol_fl_cont);
	water_cont.SetPitch(std::max(0.7f, std::min(1.3f, velW)));
	water_cont.SetPosition(engPos);
	
	//  boost
	boostsnd.SetGain(boostVal * 0.55f * pSet->vol_engine);
	boostsnd.SetPosition(engPos); //back?-
	
	// crash
	{
		crashdetection.Update(speed, dt);
		float crashdecel = crashdetection.GetMaxDecel();
		//dynamics.fHitForce4 = crashdecel / 1400.f;
		///todo: ^for old replays..  set blt car pos,rot in rpl for objs..

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
			LogO("crash: "+toStr(i));

			if (/*gain > mingain &&*/ crashsoundtime[i] > /*ti*/0.4f)  //!crashsound.Audible())
			{
				crashsound[i].SetGain(gain * pSet->vol_car_crash);
				crashsound[i].SetPosition(hitPos);
				crashsound[i].StopPlay();
				crashsoundtime[i] = 0.f;
			}
			//LogO("Car Snd: " + toStr(crashdecel));// + " force " + toStr(hit.force) + " vel " + toStr(vlen) + " Nvel " + toStr(normvel));
		}
	}
	//  time played
	for (int i=0; i < Ncrashsounds; ++i)
		if (crashsoundtime[i] < 5.f)
			crashsoundtime[i] += dt;
	

	//  crash scrap  ... save in rpl
	{
		float gain = std::min(1.f, dynamics.fCarScrap);
		if (dynamics.fCarScrap > 0.f)
		{	dynamics.fCarScrap -= (-gain * 0.8f + 1.2f)* dt;
			if (dynamics.fCarScrap < 0.f)  dynamics.fCarScrap = 0.f;
		}
		crashscrap.SetGain(gain * pSet->vol_car_scrap);
		crashscrap.SetPosition(hitPos);
	}
	{
		float gain = std::min(1.f, dynamics.fCarScreech);
		if (dynamics.fCarScreech > 0.f)
		{	dynamics.fCarScreech -= 3.f * dt;
			if (dynamics.fCarScreech < 0.f)  dynamics.fCarScreech = 0.f;
		}
		crashscreech.SetGain(gain * pSet->vol_car_scrap * 0.6f);
		crashscreech.SetPosition(hitPos);
	}

}
