#pragma once
#include <OgreVector3.h>
#include <OgreQuaternion.h>


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
	Ogre::Vector3 whPos[4];  Ogre::Quaternion rot, whRot[4];  float whR[4];
	float whVel[4], whSlide[4], whSqueal[4];
	int whTerMtr[4],whRoadMtr[4];

	float fboost,steer, percent;  char braking;

	//  fluids
	float whH[4],whAngVel[4], speed, whSteerAng[4];  int whP[4];
	
	//  hit sparks
	float fHitTime, fParIntens,fParVel;//, fSndForce, fNormVel;
	Ogre::Vector3 vHitPos,vHitNorm;  // world hit data
	
	//  camera view
	Ogre::Vector3 camPos;  Ogre::Quaternion camRot;

	PosInfo() : bNew(false),  // not inited
		pos(0,-200,0), percent(0.f), braking(0)
	{}
};
