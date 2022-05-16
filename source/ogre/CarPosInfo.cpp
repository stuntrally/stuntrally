#include "pch.h"
#include "CarPosInfo.h"
#include "Replay.h"
#include "../vdrift/cardynamics.h"
#include "../vdrift/car.h"
#include "common/data/SceneXml.h"
#include "common/Axes.h"
using namespace Ogre;


//  ctor
PosInfo::PosInfo()
	:bNew(false)  // not inited
	,pos(0,-200,0), percent(0.f), braking(0)
	,speed(0.f), fboost(0.f), steer(0.f)
	,hov_roll(0.f), hov_throttle(0.f)
	,fHitTime(0.f), fParIntens(0.f), fParVel(0.f)
{
	camPos = camOfs = Vector3::ZERO;
	rot = camRot = Quaternion::IDENTITY;
	carY = -Vector3::UNIT_Y;
	for (int w=0; w < MAX_WHEELS; ++w)
	{
		whPos[w] = Vector3::ZERO;
		whRot[w] = Quaternion::IDENTITY;
		whVel[w] = whSlide[w] = whSqueal[w] = 0.f;
		whTerMtr[w]=0;  whRoadMtr[w]=0;  whP[w]=0;
		whH[w] = whAngVel[w] = whSteerAng[w] = 0.f;
	}
	vHitPos = vHitNorm = Vector3::UNIT_Y;
}

///  pos from new Replay/ghost  New
//-----------------------------------------------------------------------
void PosInfo::FromRpl2(const ReplayFrame2* rf, CARDYNAMICS* cd)
{
	//  car
	Axes::toOgre(pos, rf->pos);
	if (cd && cd->vtype == V_Sphere)
	{
		cd->sphereYaw = rf->hov_roll;
		rot.FromAngleAxis(Radian(-rf->hov_roll), Vector3::UNIT_Y);
		carY = Vector3::UNIT_Y;
	}else
	{	rot = Axes::toOgre(rf->rot);
		carY = rot * Vector3::UNIT_Y;
	}
	//  hud
	speed = rf->speed;
	fboost = rf->fboost /255.f;  steer = rf->steer /127.f;
	braking = rf->get(b_braking);  percent = rf->percent /255.f*100.f;
	hov_roll = rf->hov_roll;	hov_throttle = rf->throttle /255.f;

	fHitTime = rf->fHitTime;
	if (!rf->hit.empty())
	{	
		const RHit& h = rf->hit[0];  //fHitForce = h.fHitForce;
		fParIntens = h.fParIntens;  fParVel = h.fParVel;
		vHitPos = h.vHitPos;  vHitNorm = h.vHitNorm;
	}
	//get(b_scrap) in car_sound

	//  wheels
	int ww = rf->wheels.size();
	for (int w=0; w < ww; ++w)
	{
		const RWheel& wh = rf->wheels[w];
		Axes::toOgre(whPos[w], wh.pos);
		whRot[w] = Axes::toOgreW(wh.rot);
		//whR[w] = outside
		
		whVel[w] = wh.whVel;
		whSlide[w] = wh.slide;  whSqueal[w] = wh.squeal;

		whTerMtr[w] = wh.whTerMtr;  whRoadMtr[w] = wh.whRoadMtr;

		whH[w] = wh.whH;  whP[w] = wh.whP;
		whAngVel[w] = wh.whAngVel;
		whSteerAng[w] = wh.whSteerAng;
	}
}


///  pos from Simulation
//-----------------------------------------------------------------------
void PosInfo::FromCar(CAR* pCar)
{
	const CARDYNAMICS* cd = &pCar->dynamics;
	//  car
	Axes::toOgre(pos, cd->GetPosition());
	if (cd->vtype == V_Sphere)
	{	rot.FromAngleAxis(Radian(-cd->sphereYaw), Vector3::UNIT_Y);
		carY = Vector3::UNIT_Y;
		hov_roll = -cd->sphereYaw;
	}else
	{	rot = Axes::toOgre(cd->GetOrientation());
		carY = rot * Vector3::UNIT_Y;
		hov_roll = cd->hov_roll;
	}
	//  hud
	speed = pCar->GetSpeed();
	fboost = cd->boostVal;	//posInfo.steer = cd->steer;
	braking = cd->IsBraking();  //percent = outside
	hov_throttle = cd->hov_throttle;
	
	fHitTime = cd->fHitTime;  fParIntens = cd->fParIntens;  fParVel = cd->fParVel;
	vHitPos = cd->vHitPos;  vHitNorm = cd->vHitNorm;

	//  wheels
	for (int w=0; w < cd->numWheels; ++w)
	{	WHEEL_POSITION wp = WHEEL_POSITION(w);

		Axes::toOgre(whPos[w], cd->GetWheelPosition(wp));
		whRot[w] = Axes::toOgreW(cd->GetWheelOrientation(wp));

		whVel[w] = cd->GetWheelVelocity(wp).Magnitude();
		whSlide[w] = -1.f;  whSqueal[w] = pCar->GetTireSquealAmount(wp, &whSlide[w]);  //!?

		whTerMtr[w] = cd->whTerMtr[w];  whRoadMtr[w] = cd->whRoadMtr[w];

		whH[w] = cd->whH[w];  whP[w] = cd->whP[w];
		whAngVel[w] = cd->wheel[w].GetAngularVelocity();
		if (w < 2)  whSteerAng[w] = cd->wheel[w].GetSteerAngle();
	}
	camOfs = Axes::toOgre(cd->cam_body.GetPosition());  ///..
}


///  replay from Simulation  New
//-----------------------------------------------------------------------
void ReplayFrame2::FromCar(const CAR* pCar, half prevHitTime)
{
	//  car
	const CARDYNAMICS& cd = pCar->dynamics;
	pos = cd.GetPosition();
	rot = cd.GetOrientation();

	//  wheels
	//wheels.clear();
	for (int w=0; w < cd.numWheels; ++w)
	{
		RWheel wh;
		WHEEL_POSITION wp = WHEEL_POSITION(w);
		wh.pos = cd.GetWheelPosition(wp);
		wh.rot = cd.GetWheelOrientation(wp);

		const TRACKSURFACE* surface = cd.GetWheelContact(wp).GetSurfacePtr();
		wh.surfType = !surface ? TRACKSURFACE::NONE : surface->type;
		//  squeal
		float slide = -1.f;
		wh.squeal = pCar->GetTireSquealAmount(wp, &slide);  wh.slide = slide;
		wh.whVel = cd.GetWheelVelocity(wp).Magnitude();
		//  susp
		wh.suspVel = cd.GetSuspension(wp).GetVelocity();
		wh.suspDisp = cd.GetSuspension(wp).GetDisplacementPercent();

		wh.whTerMtr = cd.whTerMtr[w];  wh.whRoadMtr = cd.whRoadMtr[w];
		//  fluids
		wh.whH = cd.whH[w];  wh.whP = cd.whP[w];
		wh.whAngVel = cd.wheel[w].GetAngularVelocity();
		bool inFl = cd.inFluidsWh[w].size() > 0;
		int idPar = -1;
		if (inFl)
		{	const FluidBox* fb = *cd.inFluidsWh[w].begin();
			idPar = fb ? fb->idParticles : -1;  }
		wh.whP = idPar;
		wh.whSteerAng = cd.wheel[w].GetSteerAngle();
		wheels.push_back(wh);
	}
	//  hud
	vel = pCar->GetSpeedometer();  rpm = pCar->GetEngineRPM();  gear = pCar->GetGear();
	throttle = cd.GetThrottle() *255.f;  clutch = pCar->GetClutch() *255.f;
	steer = pCar->GetLastSteer() *127.f;  fboost = cd.doBoost *255.f;
	damage = cd.fDamage /100.f*255.f;  //percent set outside

	//  eng snd
	speed = pCar->GetSpeed();  dynVel = cd.GetVelocity().Magnitude();
	set(b_braking, cd.IsBraking());
	
	bool hov = cd.vtype != V_Car;
	set(b_hov, hov);
	if (hov)
		hov_roll = cd.vtype == V_Sphere ? cd.sphereYaw : cd.hov_roll;

	
	// fluid
	bool mud = pCar->sounds.whMudSpin < 0.01f;
	set(b_fluid, mud);
	whMudSpin = pCar->sounds.whMudSpin;

	//  scrap
	bool scr = cd.fCarScrap > 0.01f || cd.fCarScreech > 0.01f;
	set(b_scrap, scr);
	if (scr)
	{	RScrap sc;
		sc.fScrap = std::min(1.f, cd.fCarScrap);
		sc.fScreech = std::min(1.f, cd.fCarScreech);
		scrap.push_back(sc);
	}

	//  hit sparks
	fHitTime = cd.fHitTime;
	bool ht = fHitTime >= prevHitTime;
	set(b_hit, ht);
	if (ht)  // hit, new data
	{
		RHit h;
		h.fHitForce = cd.fHitForce;
		h.fParIntens = cd.fParIntens;  h.fParVel = cd.fParVel;
		h.vHitPos = cd.vHitPos;  h.vHitNorm = cd.vHitNorm;
		hit.push_back(h);
	}
}
