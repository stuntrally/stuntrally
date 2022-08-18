#pragma once
#include <Ogre.h>
// #include <OgreVector3.h>
// #include <OgreQuaternion.h>
#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
#include "../vdrift/cardefs.h"
#include "half.hpp"

struct ReplayFrame;  struct ReplayFrame2;
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
	Ogre::Quaternion rot, whRot[W];

	float whVel[W], whSlide[W], whSqueal[W];
	int whTerMtr[W],whRoadMtr[W];

	float fboost,steer, percent;  char braking;
	float hov_roll/*= sph_yaw for O*/, hov_throttle;

	//  fluids
	float whH[W],whAngVel[W], speed, whSteerAng[W];  int whP[W];
	
	//  hit sparks
	float fHitTime, /*?fHitForce,*/fParIntens,fParVel;
	Ogre::Vector3 vHitPos,vHitNorm;  // world hit data
	
	//  camera view
	Ogre::Vector3 camPos;  Ogre::Quaternion camRot;
	Ogre::Vector3 camOfs;  // hit bounce offset

	//  ctor
	PosInfo();
	
	//  copy
	void FromRpl(const ReplayFrame* rf);
	void FromRpl2(const ReplayFrame2* rf, class CARDYNAMICS* cd);
	void FromCar(CAR* pCar);
};
