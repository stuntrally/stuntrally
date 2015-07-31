#include "pch.h"
#include "CarPosInfo.h"
#include "Replay.h"
#include "../vdrift/cardynamics.h"
#include "../vdrift/car.h"
#include "common/data/SceneXml.h"
using namespace Ogre;


//  ctor
PosInfo::PosInfo()
	:bNew(false)  // not inited
	,pos(0,-200,0), percent(0.f), braking(0)
	,hov_roll(0.f), hov_throttle(0.f)
	//,carY, camPos, camRot
{	}

//  transform axes, vdrift to ogre  car & wheels
//-----------------------------------------------------------------------
Quaternion Axes::qFixCar, Axes::qFixWh;

void Axes::Init()
{
	Quaternion qr;  {
	QUATERNION<double> fix;  fix.Rotate(PI_d, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixCar = qr;  }
	QUATERNION<double> fix;  fix.Rotate(PI_d/2, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixWh = qr;
}

void Axes::toOgre(Vector3& vOut, const MATHVECTOR<float,3>& vIn)
{
	vOut.x = vIn[0];  vOut.y = vIn[2];  vOut.z = -vIn[1];
}
Vector3 Axes::toOgre(const MATHVECTOR<float,3>& vIn)
{
	return Vector3(vIn[0], vIn[2], -vIn[1]);
}

//  car
Quaternion Axes::toOgre(const QUATERNION<float>& vIn)
{
	Quaternion q(vIn[0], -vIn[3], vIn[1], vIn[2]);
	return q * qFixCar;
}
Quaternion Axes::toOgre(const QUATERNION<double>& vIn)
{
	Quaternion q(vIn[0], -vIn[3], vIn[1], vIn[2]);
	return q * qFixCar;
}

//  wheels
Quaternion Axes::toOgreW(const QUATERNION<half>& vIn)
{
	Quaternion q(vIn[0], -vIn[3], vIn[1], vIn[2]);
	return q * qFixWh;
}
Quaternion Axes::toOgreW(const QUATERNION<float>& vIn)
{
	Quaternion q(vIn[0], -vIn[3], vIn[1], vIn[2]);
	return q * qFixWh;
}
Quaternion Axes::toOgreW(const QUATERNION<double>& vIn)
{
	Quaternion q(vIn[0], -vIn[3], vIn[1], vIn[2]);
	return q * qFixWh;
}

	
//  get from new replay/ghost
//-----------------------------------------------------------------------
void PosInfo::FromRpl2(const ReplayFrame2* rf)
{
	//  car
	Axes::toOgre(pos, rf->pos);
	rot = Axes::toOgre(rf->rot);
	carY = rot * Vector3::UNIT_Y;

	speed = rf->speed;
	fboost = rf->fboost;  steer = rf->steer;
	braking = rf->get(b_braking);  percent = rf->percent;
	hov_roll = rf->hov_roll;  hov_throttle = rf->throttle;

	fHitTime = rf->fHitTime;
	if (!rf->hit.empty())
	{	
		const ReplayFrame2::RHit& h = rf->hit[0];
		fHitForce = h.fHitForce;
		fParIntens = h.fParIntens;  fParVel = h.fParVel;
		vHitPos = h.vHitPos;  vHitNorm = h.vHitNorm;
	}
	/*if (rf->scrap.empty())  //!get(b_scrap)
	{
		fCarScrap = 0.f;  fCarSceech = 0.f;
	}else
	{	const ReplayFrame2::RScrap& sc = rf->scrap[0];
		fCarScrap = sc.fScrap;  fCarSceech = sc.fScreech;
	}
	if (get(b_scrap)
	b_fluid, b_hov
	/**/

	//  wheels
	int ww = rf->wheels.size();
	for (int w=0; w < ww; ++w)
	{
		const ReplayFrame2::RWheel& wh = rf->wheels[w];
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

//  get from replay/ghost
//-----------------------------------------------------------------------
void PosInfo::FromRpl(const ReplayFrame* rf)
{
	//  car
	Axes::toOgre(pos, rf->pos);
	rot = Axes::toOgre(rf->rot);
	carY = rot * Vector3::UNIT_Y;

	speed = rf->speed;
	fboost = rf->fboost;  steer = rf->steer;
	braking = rf->braking;  percent = rf->percent;
	hov_roll = rf->hov_roll;  hov_throttle = rf->throttle;

	fHitTime = rf->fHitTime;  fParIntens = rf->fParIntens;  fParVel = rf->fParVel;
	vHitPos = rf->vHitPos;  vHitNorm = rf->vHitNorm;

	//  wheels
	for (int w=0; w < 4; ++w)
	{
		Axes::toOgre(whPos[w], rf->whPos[w]);
		whRot[w] = Axes::toOgreW(rf->whRot[w]);
		//whR[w] = outside
		
		whVel[w] = rf->whVel[w];
		whSlide[w] = rf->slide[w];  whSqueal[w] = rf->squeal[w];

		whTerMtr[w] = rf->whTerMtr[w];  whRoadMtr[w] = rf->whRoadMtr[w];

		whH[w] = rf->whH[w];  whP[w] = rf->whP[w];
		whAngVel[w] = rf->whAngVel[w];
		if (w < 2)  whSteerAng[w] = rf->whSteerAng[w];
	}
}

//  get from simulation
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


//  set from simulation  Old
//-----------------------------------------------------------------------
void ReplayFrame::FromCar(const CAR* pCar)
{
	//  car
	const CARDYNAMICS& cd = pCar->dynamics;
	pos = cd.GetPosition();
	rot = cd.GetOrientation();

	//  wheels
	for (int w=0; w < cd.numWheels; ++w)
	{	WHEEL_POSITION wp = WHEEL_POSITION(w);
		whPos[w] = cd.GetWheelPosition(wp);
		whRot[w] = cd.GetWheelOrientation(wp);

		const TRACKSURFACE* surface = cd.GetWheelContact(wp).GetSurfacePtr();
		surfType[w] = !surface ? TRACKSURFACE::NONE : surface->type;
		//  squeal
		slide[w] = -1.f;  squeal[w] = pCar->GetTireSquealAmount(wp, &slide[w]);
		whVel[w] = cd.GetWheelVelocity(wp).Magnitude();
		//  susp
		suspVel[w] = cd.GetSuspension(wp).GetVelocity();
		suspDisp[w] = cd.GetSuspension(wp).GetDisplacementPercent();

		//replay.header.whR[w] = pCar->GetTireRadius(wp);//
		whTerMtr[w] = cd.whTerMtr[w];  whRoadMtr[w] = cd.whRoadMtr[w];
		//  fluids
		whH[w] = cd.whH[w];  whP[w] = cd.whP[w];
		whAngVel[w] = cd.wheel[w].GetAngularVelocity();
		bool inFl = cd.inFluidsWh[w].size() > 0;
		int idPar = -1;
		if (inFl)
		{	const FluidBox* fb = *cd.inFluidsWh[w].begin();
			idPar = fb->idParticles;  }
		whP[w] = idPar;
		if (w < 2)  whSteerAng[w] = cd.wheel[w].GetSteerAngle();
	}
	//  hud
	vel = pCar->GetSpeedometer();  rpm = pCar->GetEngineRPM();
	gear = pCar->GetGear();  clutch = pCar->GetClutch();
	throttle = cd.GetThrottle();
	steer = pCar->GetLastSteer();
	fboost = cd.doBoost;
	//  eng snd
	posEngn = cd.GetEnginePosition();
	speed = pCar->GetSpeed();
	dynVel = cd.GetVelocity().Magnitude();
	braking = cd.IsBraking();  //// from posInfo?, todo: simplify this code here ^^
	if (cd.vtype == V_Sphere)
		hov_roll = cd.sphereYaw;
	else
		hov_roll = cd.hov_roll;
	
	//  hit sparks
	fHitTime = cd.fHitTime;	fParIntens = cd.fParIntens;	fParVel = cd.fParVel;
	vHitPos = cd.vHitPos;	vHitNorm = cd.vHitNorm;
	whMudSpin = pCar->whMudSpin;
	fHitForce = cd.fHitForce;
	fCarScrap = std::min(1.f, cd.fCarScrap);
	fCarScreech = std::min(1.f, cd.fCarScreech);
}

//  set from simulation  New
//-----------------------------------------------------------------------
void ReplayFrame2::FromCar(const CAR* pCar, half prevHitTime)
{
	//  car
	const CARDYNAMICS& cd = pCar->dynamics;
	pos = cd.GetPosition();
	rot = cd.GetOrientation();

	//  wheels
	//wheels.resize(cd.numWheels);
	//wheels.clear();
	for (int w=0; w < cd.numWheels; ++w)
	{
		ReplayFrame2::RWheel wh;
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
			idPar = fb->idParticles;  }
		wh.whP = idPar;
		wh.whSteerAng = cd.wheel[w].GetSteerAngle();
		wheels.push_back(wh);
	}
	//  hud
	vel = pCar->GetSpeedometer();  rpm = pCar->GetEngineRPM();
	gear = pCar->GetGear();  clutch = pCar->GetClutch();
	throttle = cd.GetThrottle();
	steer = pCar->GetLastSteer();
	fboost = cd.doBoost;
	//  eng snd
	//posEngn = cd.GetEnginePosition();
	speed = pCar->GetSpeed();
	dynVel = cd.GetVelocity().Magnitude();
	set(b_braking, cd.IsBraking());
	
	if (cd.vtype != V_Car)
		hov_roll = cd.vtype == V_Sphere ? cd.sphereYaw : cd.hov_roll;
	
	// fluid
	bool mud = pCar->whMudSpin < 0.01f;
	set(b_fluid, mud);
	whMudSpin = pCar->whMudSpin;

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
