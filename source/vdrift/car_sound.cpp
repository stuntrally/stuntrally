#include "pch.h"
#include "par.h"
#include "car.h"
#include "cardefs.h"
#include "configfile.h"
#include "collision_world.h"
#include "tracksurface.h"
#include "configfile.h"
#include "settings.h"
#include "../ogre/CGame.h"  // replay
#include "../ogre/CarModel.h"  // camera pos
#include "../ogre/FollowCamera.h"  // camera pos
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/data/SceneXml.h"
#include "../ogre/common/CScene.h"
#include "../ogre/common/GraphView.h"
#include "../ogre/common/Axes.h"
#include "../network/protocol.hpp"
#include "tobullet.h"
#include "game.h"
#include "../ogre/SplitScreen.h"  // num plr
#include "../sound/SoundMgr.h"
#include "../sound/SoundBase.h"
#include "../sound/SoundBaseMgr.h"
#include <OgreCamera.h>
using namespace std;
using namespace Ogre;


//  Load
//--------------------------------------------------------------------------------------------------------------------------
bool CAR::LoadSounds(const std::string & carpath)
{
	Ogre::Timer ti;
	bool ss = pApp->pSet->game.local_players > 1;
	CARsounds& s = sounds;
	
	SoundMgr* snd = pGame->snd;
	const string& eng = dynamics.engine.sound_name;
	s.engine = snd->createInstance(eng,0);  s.engine->set2D(ss);
	s.engine->setEngine(true);  s.engine->start();

	int i;  float fw = numWheels;
	for (i = 0; i < numWheels; ++i)  // tires
	{
		s.asphalt[i] = snd->createInstance("asphalt", 0);	s.asphalt[i]->set2D(ss);
		s.grass[i]   = snd->createInstance("grass", 0);
		s.grass[i]->seek(float(i)/fw);  s.grass[i]->set2D(ss);
		s.gravel[i]  = snd->createInstance("gravel", 0);
		s.gravel[i]->seek(float(i)/fw);  s.gravel[i]->set2D(ss);
	}
	for (i = 0; i < numWheels; ++i)
	{
		s.bump[i] = snd->createInstance("bump"+toStr(i%4), 0);  s.bump[i]->set2D(ss);
		s.bump[i]->seek(float(i)/fw);
	}

	for (i = 0; i < Ncrashsounds; ++i)  // crashes
	{	string cn = "crash/";  int n=i+1;  cn += toStr(n/10)+toStr(n%10);
		s.crash[i] = snd->createInstance(cn, 0);  s.crash[i]->set2D(ss);
	}
	s.scrap   = snd->createInstance("crash/scrap",  0);  s.scrap->set2D(ss);
	s.screech = snd->createInstance("crash/screech",0);  s.screech->set2D(ss);

	s.wind  = snd->createInstance("wind",  0);  s.wind->set2D(ss);
	s.boost = snd->createInstance("boost", 0);  s.boost->set2D(ss);

	for (i = 0; i < Nwatersounds; ++i)  // fluids
	{	s.water[i] = snd->createInstance("water"+toStr(i+1), 0);  s.water[i]->set2D(ss);  }

	s.mud        = snd->createInstance("mud1", 0);        s.mud->set2D(ss);
	s.mud_cont   = snd->createInstance("mud_cont",   0);  s.mud_cont->set2D(ss);
	s.water_cont = snd->createInstance("water_cont", 0);  s.water_cont->set2D(ss);

	LogO("::: Time car Sounds: "/*+carpath+" "*/+ fToStr(ti.getMilliseconds(),0,3) +" ms");
	return true;
}

//  ctor
CAR::CARsounds::CARsounds()
	:fluidHitOld(0), whMudSpin(0.f)
{
	int i;
	crashtime.resize(Ncrashsounds);
	for (int i=0; i < Ncrashsounds; ++i)
		crashtime[i] = 0.f;

	engine = 0;
	SetNumWheels(4);
	for (i = 0; i < 4; ++i)  // tires
	{	asphalt[i] = 0;  grass[i] = 0;  gravel[i] = 0;  bump[i] = 0;  }

	crash.resize(Ncrashsounds);
	for (i = 0; i < Ncrashsounds; ++i)  // crashes
		crash[i] = 0;

	scrap = 0;  screech = 0;
	wind = 0;  boost = 0;

	water.resize(Nwatersounds);
	for (i = 0; i < Nwatersounds; ++i)  // fluids
		water[i] = 0;

	mud = 0;  mud_cont = 0;  water_cont = 0;
}

void CAR::CARsounds::SetNumWheels(int n)
{
	asphalt.resize(n);  grass.resize(n);  gravel.resize(n);  bump.resize(n);
	bumptime.resize(n);  bumpvol.resize(n);
	for (int i=0; i < n; ++i)
	{	bumpvol[i]=0.f;  bumptime[i] = 5.f;  }
}

void CAR::CARsounds::Destroy()
{
	delete engine;
	int i;
	for (i = 0; i < gravel.size(); ++i)  // tires
	{
		delete asphalt[i];
		delete grass[i];
		delete gravel[i];
		delete bump[i];
	}

	for (i = 0; i < Ncrashsounds; ++i)  // crashes
		delete crash[i];

	delete scrap;  delete screech;

	delete wind;  delete boost;

	for (i = 0; i < Nwatersounds; ++i)  // fluids
		delete water[i];

	delete mud;  delete mud_cont;  delete water_cont;
}


//--------------------------------------------------------------------------------------------------------------------------
void CAR::UpdateSounds(float dt)
{
	//  get data  //
	//  note: Damage is updated here
	bool bSound = !pGame->snd->isDisabled();
	CARsounds& s = sounds;
	
	float rpm, throttle, speed, dynVel;  bool hitp = false;
	MATHVECTOR<float,3> pos, engPos, whPos[MAX_WHEELS], hitPos;  // car, engine, wheels pos
	QUATERNION<float> rot;
	TRACKSURFACE::TYPE surfType[MAX_WHEELS];
	float squeal[MAX_WHEELS],whVel[MAX_WHEELS], suspVel[MAX_WHEELS],suspDisp[MAX_WHEELS];
	float whH_all = 0.f;  bool mud = false;
	float fHitForce = 0.f, boostVal = 0.f, fCarScrap = 0.f, fCarScreech = 0.f;

	bool dmg = pSet->game.damage_type > 0, reduced = pSet->game.damage_type==1;
	bool terminal = dynamics.fDamage >= 100.f;
	float fDmg = pApp->scn->sc->damageMul;

	///  replay play  ------------------------------------------
	if (pApp->bRplPlay)
	{	dmg = false;

		#ifdef DEBUG
		assert(id < pApp->frm.size());
		#endif
		const ReplayFrame2& fr = pApp->frm[id];
		pos = fr.pos;  rot = fr.rot;   rpm = fr.rpm;
		throttle = fr.throttle /255.f;  boostVal = fr.fboost /255.f;
		dynamics.fDamage = fr.damage /255.f*100.f;  //dmg read

		MATHVECTOR<float,3> offset = dynamics.engine.GetPosition();
		rot.RotateVector(offset);
		engPos = offset + pos;

		speed = fr.speed;  dynVel = fr.dynVel;
		s.whMudSpin = fr.get(b_fluid) ? fr.whMudSpin : 0.f;

		if (fr.get(b_scrap))
		{
			const RScrap& sc = fr.scrap[0];
			fCarScrap = sc.fScrap;  fCarScreech = sc.fScreech;
		}

		hitp = fr.get(b_hit);
		if (hitp)
		{
			const RHit& h = fr.hit[0];
			fHitForce = h.fHitForce;
			hitPos[0] = h.vHitPos.x;  hitPos[1] = -h.vHitPos.z;  hitPos[2] = h.vHitPos.y;
		}

		int w, ww = fr.wheels.size();
		for (w=0; w < ww; ++w)
		{
			const RWheel& wh = fr.wheels[w];
			whPos[w] = wh.pos;
			surfType[w] = (TRACKSURFACE::TYPE)wh.surfType;
			squeal[w] = wh.squeal;  whVel[w] = wh.whVel;
			suspVel[w] = wh.suspVel;  suspDisp[w] = wh.suspDisp;
			//  fluids
			if (wh.whP >= 0)  // solid no snd
				whH_all += wh.whH;
			if (wh.whP >= 1)  mud = true;
		}
	}
	else  /// game  ------------------------------------------
	{
		pos = dynamics.GetPosition();  rot = dynamics.GetOrientation();
		rpm = GetEngineRPM();
		throttle = dynamics.GetThrottle();
		engPos = dynamics.GetEnginePosition();
		speed = GetSpeed();
		dynVel = dynamics.GetVelocity().Magnitude();
		fHitForce = dynamics.fHitForce;  hitp = true;
		hitPos[0] = dynamics.vHitPos.x;  hitPos[1] = -dynamics.vHitPos.z;  hitPos[2] = dynamics.vHitPos.y;
		boostVal = dynamics.boostVal;
		
		for (int w=0; w < numWheels; ++w)
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
			if (dynamics.whP[w] >= 0)  // solid no snd
				whH_all += dynamics.whH[w];
			if (dynamics.whP[w] >= 1)  mud = true;
		}

		//  wheels in mud, spinning intensity
		float mudSpin = 0.f;
		for (int w=0; w < numWheels; ++w)
		{
			float vel = std::abs(dynamics.wheel[w].GetAngularVelocity());
			if (vel <= 30.f)  continue;
			if (dynamics.whP[w] == 2)
				mudSpin += dynamics.whH[w] * std::min(80.f, 1.5f * vel) / 80.f;
			else if (dynamics.whP[w] == 1)
				mudSpin += dynamics.whH[w] * std::min(160.f, 3.f * vel) / 80.f;
		}
		s.whMudSpin = mudSpin * 0.5f;

		//  car scrap, screech
		float gain = std::min(1.f, dynamics.fCarScrap);
		if (dynamics.fCarScrap > 0.f)
		{	dynamics.fCarScrap -= (-gain * 0.8f + 1.2f)* dt;
			if (dynamics.fCarScrap < 0.f)  dynamics.fCarScrap = 0.f;
		}
		fCarScrap = gain;

		/// <><> Damage <><>
		if (dmg && !terminal)
			if (reduced)
				dynamics.fDamage += fDmg * fCarScrap * dt * dynamics.fHitDmgA * gPar.dmgFromScrap;
			else  // normal
				dynamics.fDamage += fDmg * fCarScrap * dt * dynamics.fHitDmgA * gPar.dmgFromScrap2;

		gain = std::min(1.f, dynamics.fCarScreech);
		if (dynamics.fCarScreech > 0.f)
		{	dynamics.fCarScreech -= 3.f * dt;
			if (dynamics.fCarScreech < 0.f)  dynamics.fCarScreech = 0.f;
		}
		fCarScreech = gain;
	}
	
	//  engine pos  //todo: vel..
	Vector3 ep, ev = Vector3::ZERO;
	ep = Axes::toOgre(engPos);


//))  update sounds
if (bSound)
{	/**/	

	///  engine  ====
	float gain = 1.f;

	if (dynamics.vtype >= V_Spaceship)
	{
		s.engine->setPitch(1.f);
		gain = throttle;
	}else
	{	//  car
		gain = throttle * 0.5 + 0.5;
		s.engine->setPitch(rpm);
	}
	s.engine->setPosition(ep, ev);
	s.engine->setGain(gain * dynamics.engine_vol_mul * pSet->vol_engine);


	///  tires  oooo
	for (int i = 0; i < numWheels; ++i)
	{
		Vector3 wh;  wh = Axes::toOgre(whPos[i]);

		float maxgain = 0.6f, pitchvar = 0.4f, pmul = 1.f;

		Sound* snd = s.gravel[i];
		switch (surfType[i])
		{
		case TRACKSURFACE::ASPHALT:		snd = s.asphalt[i];  maxgain = 0.4f;  pitchvar = 0.40f;  pmul = 0.8f;  break;
		case TRACKSURFACE::GRASS:		snd = s.grass[i];    maxgain = 0.7f;  pitchvar = 0.25f;  break;
		case TRACKSURFACE::GRAVEL:		snd = s.gravel[i];   maxgain = 0.7f;  break;
		case TRACKSURFACE::CONCRETE:	snd = s.asphalt[i];  maxgain = 0.5f;  pitchvar = 0.25f;  pmul = 0.7f;  break;
		case TRACKSURFACE::SAND:		snd = s.grass[i];    maxgain = 0.5f;  pitchvar = 0.25f;  break;
		case TRACKSURFACE::NONE:
						default:		snd = s.asphalt[i];  maxgain = 0.0f;  break;
		}
		/// todo: more,sounds.. sand,snow,grass-new,mud..
		// todo: sum slip, spin, stop tire sounds

		float pitch = std::min(1.f, std::max(0.f, (whVel[i]-5.0f)*0.1f ));
		pitch = (1.f - pitch) * pitchvar;
		pitch = pitch + (1.f - pitchvar);
		pitch = std::min(2.f, std::max(0.25f, pitch ));

		snd->setPosition(wh, ev);
		snd->setGain(squeal[i]*maxgain * pSet->vol_tires);
		snd->setPitch(pitch * pmul);
		//  mute others
		if (snd != s.asphalt[i])  s.asphalt[i]->setGain(0.f);
		if (snd != s.grass[i])    s.grass[i]->setGain(0.f);
		if (snd != s.gravel[i])   s.gravel[i]->setGain(0.f);


		//  susp bump  ~~~
		if (dynamics.vtype == V_Car)
		{
			suspbump[i].Update(suspVel[i], suspDisp[i], dt);
			if (suspbump[i].JustSettled())
			{
				float bumpsize = suspbump[i].GetTotalBumpSize();
				float gain = bumpsize * speed * 0.2f;  //par
				gain = std::max(0.f, std::min(1.2f, gain));

				if (gain > 0.2f && //!tirebump[i]->isAudible() &&
					(gain > s.bumpvol[i] || s.bumptime[i] > 0.22f))
				{
					s.bumpvol[i] = gain;
					s.bumptime[i] = 0.f;
					//tirebump[i]->start();
					//LogO("bump "+toStr(i)+" "+fToStr(gain));
				}
			}
			s.bump[i]->setPosition(wh, ev);  //par gain, time fade
			float gain = 0.5f + 0.7f*s.bumpvol[i] - s.bumptime[i]*(2.f+2.f*s.bumpvol[i]);
			gain = std::max(0.f, std::min(1.0f, gain));

			s.bump[i]->setGain(gain * pSet->vol_susp);
			if (s.bumptime[i] < 5.f)
				s.bumptime[i] += dt;
		}
	}
	

	//  wind  ----
	gain = dynVel;
	if (dynamics.vtype == V_Spaceship)   gain *= 0.7f;
	//if (dynamics.sphere)  gain *= 0.9f;
	if (gain < 0.f)	gain = -gain;
	gain *= 0.02f;	gain *= gain;
	if (gain > 1.f)	gain = 1.f;
	
	s.wind->setGain(gain * pSet->vol_env);
	s.wind->setPosition(ep, ev);

	//  boost
	s.boost->setGain(boostVal * 0.55f * pSet->vol_engine);
	s.boost->setPosition(ep, ev);  //back?-


	//  fluids - hit  ~~~~
	bool fluidHit = whH_all > 1.f;
	//LogO(toStr(whH_all) + "  v "+ toStr(dynVel));

	if (fluidHit && !s.fluidHitOld)
	//if (dynVel > 10.f && whH_all > 1.f && )
	{
		int i = std::min(Nwatersounds-1, (int)(dynVel / 15.f));
		float gain = std::min(3.0f, 0.3f + dynVel / 30.f);
		Sound* snd = /*mud ? s.mud : */s.water[i];
		
		//LogO("fluid hit i"+toStr(i)+" g"+toStr(gain)+" "+(mud?"mud":"wtr"));
		if (!snd->isAudible())
		{
			snd->setGain(gain * pSet->vol_fl_splash * (mud ? 0.6f : 1.f));
			snd->setPosition(ep, ev);
			snd->start();
		}

		if (s.mud)  {  Sound* snd = s.mud;
		if (!snd->isAudible())
		{
			snd->setGain(gain * pSet->vol_fl_splash);
			snd->setPosition(ep, ev);
			snd->start();
		}	}
	}
	s.fluidHitOld = fluidHit;

	//  fluids - continuous
	float velM = mud && whH_all > 0.1f ?
		s.whMudSpin * 2.5f : 0.f;
	s.mud_cont->setGain(std::min(1.f, velM) * pSet->vol_fl_cont * 0.85f);
	s.mud_cont->setPitch(std::max(0.7f, std::min(/*3.f*/2.f, velM * 0.35f)));
	s.mud_cont->setPosition(ep, ev);

	float velW = !mud && whH_all > 0.1f && whH_all < 3.9f ?
		dynVel / 30.f : 0.f;
	s.water_cont->setGain(std::min(1.f, velW * 1.5f) * pSet->vol_fl_cont);
	s.water_cont->setPitch(std::max(0.7f, std::min(1.3f, velW)));
	s.water_cont->setPosition(ep, ev);
}
//))  sounds
	
	
	//  crash  ----
	Vector3 hp;  hp = Axes::toOgre(hitPos);
	{
		crashdetection2.Update(-fHitForce, dt);
		crashdetection2.deceltrigger = 1.f;
		float crashdecel2 = crashdetection2.GetMaxDecel();
		dynamics.fHitForce3 = crashdecel2 / 30.f;

		if (crashdecel2 > 0)
		{
			float gain = 0.9f;
			
			int f = crashdecel2 / 30.f * Ncrashsounds;
			int i = std::max(1, std::min(Ncrashsounds-1, f));
			//LogO("crash: "+toStr(i));

			if (s.crashtime[i] > /*ti*/0.4f)  //!crashsound.isAudible())
			{
				if (bSound)
				{
					s.crash[i]->setGain(gain * pSet->vol_car_crash);
					if (hitp)
					s.crash[i]->setPosition(hp, ev);
					s.crash[i]->start();
				}
				s.crashtime[i] = 0.f;
				
				/// <><> Damage <><> 
				if (dmg && !terminal)
					if (reduced)
						dynamics.fDamage += fDmg * crashdecel2 * dynamics.fHitDmgA * gPar.dmgFromHit;
					else  // normal
					{	float f = std::min(1.f, crashdecel2 / 30.f);
						f = powf(f, gPar.dmgPow2);
						dynamics.fDamage += fDmg * crashdecel2 * dynamics.fHitDmgA * gPar.dmgFromHit2 * f;
					}
			}
			//LogO("Car Snd: " + toStr(crashdecel));// + " force " + toStr(hit.force)
		}
	}
	//  time played
	for (int i=0; i < Ncrashsounds; ++i)
		if (s.crashtime[i] < 5.f)
			s.crashtime[i] += dt;
	

	//  crash scrap and screech
	if (bSound)
	{
		s.scrap->setGain(fCarScrap * pSet->vol_car_scrap);
		if (hitp)
		s.scrap->setPosition(hp, ev);

		s.screech->setGain(fCarScreech * pSet->vol_car_scrap * 0.6f);
		if (hitp)
		s.screech->setPosition(hp, ev);
	}


	/// <><> Damage <><> 
	if (dmg)
		if (dynamics.fDamage > 100.f)  dynamics.fDamage = 100.f;
}
