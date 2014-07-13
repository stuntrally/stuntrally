#include "pch.h"
#include "CarPosInfo.h"
#include "ReplayGame.h"
#include "../vdrift/cardynamics.h"
#include "../vdrift/car.h"
#include "common/data/SceneXml.h"
using namespace Ogre;


//  ctor
PosInfo::PosInfo()
	:bNew(false)  // not inited
	,pos(0,-200,0), percent(0.f), braking(0)
	,hov_roll(0.f)
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
	hov_roll = rf->hov_roll;

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
	if (cd->sphere)
	{	rot.FromAngleAxis(Radian(-cd->sphereYaw), Vector3::UNIT_Y);
		carY = Vector3::UNIT_Y;
	}else
	{	rot = Axes::toOgre(cd->GetOrientation());
		carY = rot * Vector3::UNIT_Y;
	}
	speed = pCar->GetSpeed();
	fboost = cd->boostVal;	//posInfo.steer = cd->steer;
	braking = cd->IsBraking();  //percent = outside
	hov_roll = cd->hov_roll;
	
	fHitTime = cd->fHitTime;  fParIntens = cd->fParIntens;  fParVel = cd->fParVel;
	vHitPos = cd->vHitPos;  vHitNorm = cd->vHitNorm;

	//  wheels
	for (int w=0; w < 4; ++w)
	{	WHEEL_POSITION wp = WHEEL_POSITION(w);

		Axes::toOgre(whPos[w], cd->GetWheelPosition(wp));
		whRot[w] = Axes::toOgreW(cd->GetWheelOrientation(wp));
		whR[w] = pCar->GetTireRadius(wp);

		whVel[w] = cd->GetWheelVelocity(wp).Magnitude();
		whSlide[w] = -1.f;  whSqueal[w] = pCar->GetTireSquealAmount(wp, &whSlide[w]);  //!?

		whTerMtr[w] = cd->whTerMtr[w];  whRoadMtr[w] = cd->whRoadMtr[w];

		whH[w] = cd->whH[w];  whP[w] = cd->whP[w];
		whAngVel[w] = cd->wheel[w].GetAngularVelocity();
		if (w < 2)  whSteerAng[w] = cd->wheel[w].GetSteerAngle();
	}
	camOfs = Axes::toOgre(cd->cam_body.GetPosition());  ///..
}


//  set from simulation
//-----------------------------------------------------------------------
void ReplayFrame::FromCar(const CAR* pCar)
{
	//  car
	const CARDYNAMICS& cd = pCar->dynamics;
	pos = cd.GetPosition();
	rot = cd.GetOrientation();
	//if (cd.hover)
	//	rot = rot * Quaternion(Degree(cd->hov_roll), Vector3::UNIT_X);
	if (cd.sphere)  rot[0] = cd.sphereYaw; //o

	//  wheels
	for (int w=0; w < 4; ++w)
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
	hov_roll = cd.hov_roll;
	//  hit sparks
	fHitTime = cd.fHitTime;	fParIntens = cd.fParIntens;	fParVel = cd.fParVel;
	vHitPos = cd.vHitPos;	vHitNorm = cd.vHitNorm;
	whMudSpin = pCar->whMudSpin;
	fHitForce = cd.fHitForce;
	fCarScrap = std::min(1.f, cd.fCarScrap);
	fCarScreech = std::min(1.f, cd.fCarScreech);
}
