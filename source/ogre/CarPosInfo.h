#pragma once
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
#include "../vdrift/cardefs.h"

struct ReplayFrame;
class CAR;


//  Stores all the needed information about car coming from vdrift
//  position,rotation of car and wheels
//  and all data needed to update particles emitting rates and sounds
//  todo? remove PosInfo use ReplayFrame?

struct PosInfo
{
	bool bNew;  //  new posinfo available for Update
	//  car
	Ogre::Vector3 pos, carY;
	//  wheel
	const static int W = MAX_WHEELS;
	Ogre::Vector3 whPos[W];
	Ogre::Quaternion rot, whRot[W];  float whR[W];

	float whVel[W], whSlide[W], whSqueal[W];
	int whTerMtr[W],whRoadMtr[W];

	float fboost,steer, percent;  char braking;
	float hov_roll/*= sph_yaw for O*/, hov_throttle;

	//  fluids
	float whH[W],whAngVel[W], speed, whSteerAng[W];  int whP[W];
	
	//  hit sparks
	float fHitTime, fParIntens,fParVel;//, fSndForce, fNormVel;
	Ogre::Vector3 vHitPos,vHitNorm;  // world hit data
	
	//  camera view
	Ogre::Vector3 camPos;  Ogre::Quaternion camRot;
	Ogre::Vector3 camOfs;  // hit bounce offset

	//  ctor
	PosInfo();
	
	//  copy
	void FromRpl(const ReplayFrame* rf);
	void FromCar(CAR* pCar);
};


struct Axes
{
	static void Init();
	static Ogre::Quaternion qFixCar,qFixWh;

	//  to ogre from vdrift
	static void toOgre(Ogre::Vector3& vOut, const MATHVECTOR<float,3>& vIn);
	static Ogre::Vector3 toOgre(const MATHVECTOR<float,3>& vIn);

	static Ogre::Quaternion toOgre(const QUATERNION<float>& vIn);  // car
	static Ogre::Quaternion toOgre(const QUATERNION<double>& vIn);
	static Ogre::Quaternion toOgreW(const QUATERNION<float>& vIn);  // wheels
	static Ogre::Quaternion toOgreW(const QUATERNION<double>& vIn);
};
