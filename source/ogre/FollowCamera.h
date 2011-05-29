#ifndef _FolowCamera_h_
#define _FolowCamera_h_

#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <vector>


enum CamTypes
{
	CAM_Follow = 0,	//  0 Follow - car rotY & pos from behind car, smooth
	CAM_Free,		//  1 Free   - free rot, pos from car
	CAM_Arena,		//  2 Arena  - free pos & rot, fly
	CAM_Car,		//  3 Car    - car pos & rot, full
	CAM_ExtAng,		//  4 Extended, angles - car rotY & pos, smooth
	CAM_ALL
};

const char CAM_Str[CAM_ALL][10] = {"Follow", "Free", "Arena", "Car", "ExtAng" };


class CameraAngle
{
public:
	CamTypes  mType;  Ogre::String  mName;
	Ogre::Real  mDist, mSpeed, mSpeedRot;
	Ogre::Radian  mYaw, mPitch;
	Ogre::Vector3 mOffset;
	int mMain, mHideGlass;

	CameraAngle();
};


namespace Ogre {  class TerrainGroup;  class Camera;  class OverlayElement;  class SceneNode;  }


class FollowCamera
{
public:
	Ogre::Camera* mCamera;
	
	Ogre::TerrainGroup* mTerrain;
	class COLLISION_WORLD* mWorld;
	
	// collision objs for raycast
	class btSphereShape* shape;
	class btDefaultMotionState* state;
	class btRigidBody* body;

	const Ogre::SceneNode* mGoalNode;
	Ogre::Vector3  mLook;
	CameraAngle  ca;
	bool  first;
	
	// for ext cam
	Ogre::Quaternion qq;


	FollowCamera(Ogre::Camera* cam);
	~FollowCamera();

	void  update(Ogre::Real time);
	void  updInfo(Ogre::Real time = 0);
	void  Move( bool mbLeft, bool mbRight, bool mbMiddle, bool shift, Ogre::Real mx, Ogre::Real my, Ogre::Real mz );
	Ogre::Real fMoveTime;


	//  Camera Angles
	int  miCount, miCurrent;
	std::vector<CameraAngle>  mCameraAngles;

	bool  loadCameras();  void saveCamera();
	void  updAngle(),  incCur(int dir);
	void  Next(bool bPrev = false, bool bMainOnly = false);
	void  setCamera(int ang);
	
	Ogre::OverlayElement  *ovInfo,*ovName;
};

#endif
