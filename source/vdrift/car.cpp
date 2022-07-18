#include "pch.h"
#include "par.h"
#include "car.h"
#include "cardefs.h"
#include "configfile.h"
#include "collision_world.h"
#include "tracksurface.h"
#include "configfile.h"
#include "settings.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/data/CData.h"
#include "../ogre/common/CScene.h"
#include "../ogre/common/GraphView.h"
#include "../ogre/CGame.h"  //+ replay
#include "../ogre/CarModel.h"  //+ camera pos
#include "../ogre/FollowCamera.h"  //+ camera pos
#include "../road/PaceNotes.h"  //+ pace reset
#include "../network/protocol.hpp"
#include "../sound/SoundMgr.h"
#include "tobullet.h"
#include "game.h"  //sound


///  ctor
CAR::CAR()
	:pSet(0), pApp(0), id(0), pCarM(0)
	,last_steer(0)
	,iCamNext(0), bLastChk(0),bLastChkOld(0)
	,bRewind(0),bRewindOld(0),timeRew(0.f)
	,trackPercentCopy(0), bRemoteCar(0)
	,bResetPos(0)
	,dmgLastCheck(0.f), sphYawAtStart(0.f)
{
	SetNumWheels(4);
	//dynamics.pCar = this;
}

void CAR::SetNumWheels(int n)
{
	numWheels = n;
	suspbump.resize(n);
	sounds.SetNumWheels(n);
}

///  dtor
CAR::~CAR()
{
	sounds.Destroy();
}


//--------------------------------------------------------------------------------------------------------------------------
bool CAR::Load(class App* pApp1,
	CONFIGFILE & cf,
	const std::string & carname,
	const MATHVECTOR<float,3> & init_pos, const QUATERNION<float> & init_rot,
	COLLISION_WORLD & world,
	bool abs, bool tcs,
	bool isRemote, int idCar,
  	bool debugmode)
{
	pApp = pApp1;  pGame = pApp->pGame;  pSet = pApp->pSet;

	cartype = carname;
	bRemoteCar = isRemote;  id = idCar;
	std::string carpath = PATHMANAGER::Cars()+"/"+carname+"/";

	//  get coordinate system version
	int version = 2;
	cf.GetParam("version", version);
	
	//  wheels count
	int nw = 0;
	cf.GetParam("wheels", nw);
	if (nw >= 2 && nw <= MAX_WHEELS)
		SetNumWheels(nw);
	
	
	///-  custom car collision params  (dimensions and sphere placement)
	//..................................................................................
	CARDYNAMICS& cd = dynamics;
	//  com
	cd.com_ofs_L = 0.f;  cf.GetParam("collision.com_ofs_L", cd.com_ofs_L);  //|
	cd.com_ofs_H = 0.f;  cf.GetParam("collision.com_ofs_H", cd.com_ofs_H);
	//  dim
	cd.coll_R   = 0.3f;  cf.GetParam("collision.radius", cd.coll_R);
	cd.coll_R2m = 0.6f;  cf.GetParam("collision.radius2mul", cd.coll_R2m);
	cd.coll_H   = 0.45f; cf.GetParam("collision.height", cd.coll_H);
	cd.coll_W   = 0.5f;  cf.GetParam("collision.width",  cd.coll_W);
	//  ofs
	cd.coll_Lofs = 0.f;  cf.GetParam("collision.offsetL", cd.coll_Lofs);
	cd.coll_Wofs = 0.f;  cf.GetParam("collision.offsetW", cd.coll_Wofs);
	cd.coll_Hofs = 0.f;  cf.GetParam("collision.offsetH", cd.coll_Hofs);
	cd.coll_Lofs -= cd.com_ofs_L;  //|
	cd.coll_Hofs -= cd.com_ofs_H;
	//  L
	cd.coll_posLfront = 1.9f; cf.GetParam("collision.posLfront", cd.coll_posLfront);
	cd.coll_posLback = -1.9f; cf.GetParam("collision.posLrear",  cd.coll_posLback);
	//  w
	cd.coll_FrWmul  = 0.2f;   cf.GetParam("collision.FrWmul",  cd.coll_FrWmul);
	cd.coll_FrHmul  = 1.0f;   cf.GetParam("collision.FrHmul",  cd.coll_FrHmul);
	cd.coll_TopWmul = 0.8f;   cf.GetParam("collision.TopWmul", cd.coll_TopWmul);
	//  Top	 L pos
	cd.coll_TopFr    = 0.4f;  cf.GetParam("collision.TopFr",    cd.coll_TopFr);
	cd.coll_TopMid   =-0.3f;  cf.GetParam("collision.TopMid",   cd.coll_TopMid);
	cd.coll_TopBack  =-1.1f;  cf.GetParam("collision.TopBack",  cd.coll_TopBack);
	//  Top  h mul
	cd.coll_TopFrHm  = 0.2f;  cf.GetParam("collision.TopFrHm",  cd.coll_TopFrHm);
	cd.coll_TopMidHm = 0.4f;  cf.GetParam("collision.TopMidHm", cd.coll_TopMidHm);
	cd.coll_TopBackHm= 0.2f;  cf.GetParam("collision.TopBackHm",cd.coll_TopBackHm);
	//  Front w mul
	cd.coll_FrontWm  = 1.0f;  cf.GetParam("collision.FrontWm",  cd.coll_FrontWm);

	cd.coll_friction = 0.4f;  cf.GetParam("collision.friction",  cd.coll_friction);
	cd.coll_flTrig_H = 0.f;   cf.GetParam("collision.fluidTrigH",cd.coll_flTrig_H);
	cd.coll_flTrig_H -= cd.com_ofs_H;  //|
	
	//  buoyancy  dim
	cd.buoy_X = 1.2f;   cf.GetParam("collision.buoy_X", cd.buoy_X);
	cd.buoy_Y = 0.7f;   cf.GetParam("collision.buoy_Y", cd.buoy_Y);
	cd.buoy_Z = 0.4f;   cf.GetParam("collision.buoy_Z", cd.buoy_Z);
	cd.buoy_Mul = 1.f;  cf.GetParam("collision.buoy_Mul", cd.buoy_Mul);


	//  load cardynamics
	if (!cd.Load(pGame, cf))
		return false;

	MATHVECTOR<double,3> pos = init_pos;
	QUATERNION<double> rot;  rot = init_rot;

	float stOfsY = 0.f;
	cf.GetParam("collision.start-offsetY", stOfsY);
		pos[2] += stOfsY -0.4/**/ + cd.com_ofs_H;  //|

	posAtStart = posLastCheck = pos;
	rotAtStart = rotLastCheck = rot;
	dmgLastCheck = 0.f;
	
	cd.Init(pSet, pApp->scn->sc, pApp->scn->data->fluids,
		world, pos, rot);

	sphYawAtStart = cd.sphereYaw;

	cd.SetABS(abs);  cd.SetTCS(tcs);


	//  load sounds
	if (!pGame->snd->isDisabled())
		LoadSounds(carpath);

	//mz_nominalmax = (GetTireMaxMz(FRONT_LEFT) + GetTireMaxMz(FRONT_RIGHT))*0.5;  //!! ff

	return true;
}


//--------------------------------------------------------------------------------------------------------------------------
void CAR::Update(double dt)
{
	dynamics.Update();
	
	UpdateSounds(dt);  // and damage
	
	///  graphs new values  .-_/\_.-
	if (pApp->pSet->show_graphs && id == 0)  // for 1st car
		GraphsNewVals(dt);  // implementation in Hud_Graphs.cpp
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

	//  Brakes
	if (!bRemoteCar)
	{
		float brake = !rear ? inputs[CARINPUT::BRAKE] : inputs[CARINPUT::THROTTLE];
		dynamics.SetBrake(brake);
	}
	#ifdef CAR_PRV
	dynamics.SetHandBrake(1.f);
	#else
	dynamics.SetHandBrake(inputs[CARINPUT::HANDBRAKE]);
	#endif
	
	//  boost, flip over
	if (!bRemoteCar)
		dynamics.doBoost = inputs[CARINPUT::BOOST];
	dynamics.doFlip = inputs[CARINPUT::FLIP];

	//  Steering
	if (!bRemoteCar)
	{
		float steer_value = inputs[CARINPUT::STEER_RIGHT];
		if (std::abs(inputs[CARINPUT::STEER_LEFT]) > std::abs(inputs[CARINPUT::STEER_RIGHT])) //use whichever control is larger
			steer_value = -inputs[CARINPUT::STEER_LEFT];

		#ifdef CAR_PRV
		//if (!dynamics.hover && !dynamics.sphere)
			steer_value = -1.f;
		#endif
		dynamics.SetSteering(steer_value, pGame->GetSteerRange());
		last_steer = steer_value;
	}

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

	//  Throttle
	float throttle = !rear ? inputs[CARINPUT::THROTTLE] : inputs[CARINPUT::BRAKE];
	float clutch = 1 - inputs[CARINPUT::CLUTCH]; // 

	dynamics.ShiftGear(new_gear);
	dynamics.SetThrottle(throttle);
	dynamics.SetClutch(clutch);

	//  abs tcs
	///TODO: car setup separated for all (4) players: (auto shift, auto rear, rear inv, abs, tcs)  ...
	//if (inputs[CARINPUT::ABS_TOGGLE])	dynamics.SetABS(!dynamics.GetABSEnabled());
	//if (inputs[CARINPUT::TCS_TOGGLE])	dynamics.SetTCS(!dynamics.GetTCSEnabled());

	
	//  Camera
	iCamNext = -inputs[CARINPUT::PREV_CAM] + inputs[CARINPUT::NEXT_CAM];
	
	bLastChkOld = bLastChk;
	bLastChk = inputs[CARINPUT::LAST_CHK];
	
	if (bResetPos)  // reset game
	{	ResetPos(true);  bResetPos = false;  pGame->bResetObj = true;  }
	else
	if (bLastChk && !bLastChkOld)
		ResetPos(false);  // goto last checkpoint
	

	///  Rewind  with cooldown
	bool bRew = inputs[CARINPUT::REWIND] > 0.2f;
	if (timeRew > 0.f)
		timeRew -= dt;
	if (!bRew && bRewindOld)
		timeRew = gPar.rewindCooldown;

	bRewind = timeRew <= 0.f && bRew;  // car input
	bRewindOld = bRewind;
}


//--------------------------------------------------------------------------------------------------------------------------
float CAR::GetFeedback()
{
	return dynamics.GetFeedback() / (mz_nominalmax * 0.025);
}

float CAR::GetTireSquealAmount(WHEEL_POSITION i, float* slide, float* s1, float* s2) const
{
	const TRACKSURFACE* surface = dynamics.GetWheelContact(i).GetSurfacePtr();
	if (!surface)  return 0;
	if (surface->type == TRACKSURFACE::NONE)
		return 0;
		
	float d = dynamics.GetWheelContact(i).GetDepth() - 2*GetTireRadius(i);
	bool inAir = d > 0.f;  //d > 0.9f || d < 0.f;  //*!
	if (inAir)  // not on ground 
		return 0;

	MATHVECTOR<float,3> groundvel;
	groundvel = dynamics.GetWheelVelocity(i);
	QUATERNION<float> wheelspace;
	wheelspace = dynamics.GetUprightOrientation(i);
	(-wheelspace).RotateVector(groundvel);
	float carv = groundvel[0];

	float wheelspeed = dynamics.GetWheel(i).GetAngularVelocity() * dynamics.GetWheel(i).GetRadius();
	groundvel[0] -= wheelspeed;
	groundvel[1] *= 2.f;
	groundvel[2] = 0.f;

	float squeal = (groundvel.Magnitude() - 3.f) * 0.2f;
	if (slide)  *slide = squeal + 0.6f;

	Dbl slideratio = dynamics.GetWheel(i).slips.slideratio;
	Dbl slipratio = dynamics.GetWheel(i).slips.slipratio;
	if (s1)  *s1 = slideratio;
	if (s2)  *s2 = slipratio;

	//if (i==0)  LogO(iToStr(i)+"  s "+fToStr(carv,2,3)+"  x "+fToStr(wheelspeed,2,3));//+"  y "+fToStr(groundvel[1],2,3));
	//LogO(fToStr(slipratio,2,3)+" "+fToStr(slideratio,2,3));

	double maxratio = std::max(std::abs(slideratio), std::abs(slipratio));
	float squealfactor = std::max(0.0, maxratio - 1.0);
	squeal *= squealfactor;

	//  locked braking slide
	float brk = (1.f * std::abs(carv - wheelspeed)) * (1.f - 0.1f * std::abs(wheelspeed));
	//if (s2)  *s2 = std::max(-10.f, brk*1.f);  //test
	squeal += std::max(0.f, brk);

	if (squeal < 0.f)  squeal = 0.f;
	if (squeal > 1.f)  squeal = 1.f;  // 0..1
	return squeal;
}


//  Network CAR data send/receive
///------------------------------------------------------------------------------------------------------------------------------
protocol::CarStatePackage CAR::GetCarStatePackage() const
{
	protocol::CarStatePackage csp;
	csp.pos = ToMathVector<float>(dynamics.chassis->getCenterOfMassPosition());
	csp.rot = ToMathQuaternion<float>(dynamics.chassis->getCenterOfMassTransform().getRotation());
	csp.linearVel = GetVelocity();
	csp.angularVel = GetAngularVelocity();

	//  steer
	csp.steer = dynamics.GetSteering();
	csp.boost = dynamics.doBoost * 255.f;  // pack to uint8
	csp.brake = dynamics.IsBraking() * 255.f;
	//csp.trackPercent = trackPercentCopy;  // needed from CarModel
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
		newrot = state.rot;  }

	SetPosition(newpos, newrot);

	// No interpolation in velocities
	dynamics.chassis->setLinearVelocity(ToBulletVector(state.linearVel));
	dynamics.chassis->setAngularVelocity(ToBulletVector(state.angularVel));

	dynamics.SynchronizeBody();  // set body from chassis
	dynamics.UpdateWheelContacts();

	//  steer
	dynamics.SetSteering(state.steer, pGame->GetSteerRange());  //peers can have other game settins..
	last_steer = state.steer;
	dynamics.doBoost = state.boost / 255.f;  // unpack from uint8
	dynamics.SetBrake(state.brake / 255.f);
	trackPercentCopy = state.trackPercent / 255.f * 100.f;
}

//------------------------------------------------------------------------------------------------------------------------------
void CAR::SetPosition1(const MATHVECTOR<float,3> & pos)
{
	MATHVECTOR<double,3> dpos = pos;
	dynamics.SetPosition(dpos);
	dynamics.AlignWithGround();
	posAtStart = posLastCheck = pos;
}
void CAR::SetPosition(const MATHVECTOR<float,3> & pos, const QUATERNION<float> & rot)
{
	MATHVECTOR<double,3> dpos = pos;
	dynamics.SetPosition(dpos);
	dynamics.AlignWithGround();

	btTransform tr;
	tr.setOrigin(ToBulletVector(pos));
	tr.setRotation(ToBulletQuaternion(rot));
	dynamics.chassis->setWorldTransform(tr);
	if (dynamics.vtype == V_Sphere)  dynamics.sphereYaw = rot[0]; //o
}
void CAR::SetPosition(const MATHVECTOR<Dbl,3> & pos, const QUATERNION<Dbl> & rot)
{
	dynamics.SetPosition(pos);
	dynamics.AlignWithGround();

	btTransform tr;
	tr.setOrigin(ToBulletVector(pos));
	tr.setRotation(ToBulletQuaternion(rot));
	dynamics.chassis->setWorldTransform(tr);
	if (dynamics.vtype == V_Sphere)  dynamics.sphereYaw = rot[0]; //o
}

///  reset car, pos and state
///------------------------------------------------------------------------------------------------------------------------------
void CAR::ResetPos(bool fromStart)
{
	const MATHVECTOR<Dbl,3>& pos = fromStart ? posAtStart : posLastCheck;
	const QUATERNION<Dbl>&   rot = fromStart ? rotAtStart : rotLastCheck;
	SetPosition(pos, rot);

	dynamics.chassis->setLinearVelocity(btVector3(0,0,0));
	dynamics.chassis->setAngularVelocity(btVector3(0,0,0));

	dynamics.SynchronizeBody();  // set body from chassis
	if (fromStart)
	{
		if (dynamics.vtype == V_Sphere)
			dynamics.sphereYaw = sphYawAtStart;

		dynamics.boostFuel = dynamics.boostFuelStart;  // restore boost fuel
		dynamics.fDamage = 0.f;  // clear damage
		if (pApp->scn->pace)
			pApp->scn->pace->Reset();  //
	}else
		dynamics.fDamage = dmgLastCheck;

	//  engine, wheels
	dynamics.engine.SetInitialConditions();
	for (int w=0; w < numWheels; ++w)
	{
		MATHVECTOR<Dbl,3> zero(0,0,0);
		dynamics.wheel[w].SetAngularVelocity(0);
		//dynamics.wheel_velocity[w] = zero;
	}
	//crashdetection.Update(0.f, 0.1f);  //prevent car hit sound
	dynamics.fHitDmgA = 0.f;

	//dynamics.SynchronizeChassis();
	dynamics.UpdateWheelContacts();
}

///  save car pos and rot
void CAR::SavePosAtCheck()
{
	posLastCheck = dynamics.body.GetPosition();
	rotLastCheck = dynamics.body.GetOrientation();
	dmgLastCheck = dynamics.fDamage;
}

///  set pos, for rewind
void CAR::SetPosRewind(const MATHVECTOR<float,3>& pos, const QUATERNION<float>& rot, const MATHVECTOR<float,3>& vel, const MATHVECTOR<float,3>& angvel)
{
	SetPosition(pos, rot);

	// velocities
	dynamics.chassis->setLinearVelocity(ToBulletVector(vel));
	dynamics.chassis->setAngularVelocity(ToBulletVector(angvel));

	dynamics.SynchronizeBody();  // set body from chassis
	dynamics.UpdateWheelContacts();
	dynamics.fHitDmgA = 0.f;

	//  steer
	//dynamics.SetSteering(steer);  last_steer = steer;
}
