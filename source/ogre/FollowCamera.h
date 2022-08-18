#pragma once
#include <Ogre.h>
// #include <OgreVector3.h>
// #include <OgreQuaternion.h>
// #include <OgreString.h>
#include <vector>


enum CamTypes
{
	CAM_Follow = 0,	//  0 Follow - car rotY & pos from behind car, smooth
	CAM_Free,		//  1 Free   - free rot, pos from car
	CAM_Arena,		//  2 Arena  - free pos & rot, fly
	CAM_Car,		//  3 Car    - car pos & rot, full
	CAM_ExtAng,		//  4 Extended, angles - car rotY & pos, smooth, const distance
	CAM_ALL
};

const char CAM_Str[CAM_ALL][10] =
	{"Follow", "Free", "Arena", "Car", "ExtAng" };


class CameraAngle
{
public:
	CamTypes  mType;  Ogre::String  mName;
	Ogre::Real  mDist, mSpeed, mSpeedRot, mOfsMul;
	Ogre::Radian  mYaw, mPitch;
	Ogre::Vector3 mOffset;
	int mMain, mHideGlass;

	CameraAngle();
};


namespace Ogre {  class TerrainGroup;  class Camera;  class OverlayElement;  class SceneNode;  }
struct PosInfo;  class SETTINGS;


//#define CAM_TILT_DBG  // show wheels in ray hit poses
class COLLISION_WORLD;


class FollowCamera
{
public:
	FollowCamera(Ogre::Camera* cam, SETTINGS* pSet1);
	~FollowCamera();

	SETTINGS* pSet;

	//  ogre
	Ogre::Camera* mCamera;
	Ogre::TerrainGroup* mTerrain;
	class btRigidBody* chassis;

	///  state vars
	Ogre::Vector3 mLook, mPosNodeOld;  Ogre::Real mVel;
	Ogre::Quaternion qq;  // for ext cam
	Ogre::Radian mAPitch,mAYaw, mATilt;  // for arena cam, smoothing
	Ogre::Vector3 camPosFinal;  Ogre::Real mDistReduce;  //float dbgLen;

	#ifdef CAM_TILT_DBG
		Ogre::Vector3 posHit[4];
	#endif

	///  update, simulates camera
	void update(Ogre::Real time, const PosInfo& posInPrev, PosInfo* posOut, COLLISION_WORLD* world, bool bounce, bool sphere);
	bool updInfo(Ogre::Real time = 0);  char ss[512];
	Ogre::String sName;  bool updName;

	//  apply, sets mCamera's pos and rot
	void Apply(const PosInfo& posIn);

	void Move( bool mbLeft, bool mbRight, bool mbMiddle, bool shift, Ogre::Real mx, Ogre::Real my, Ogre::Real mz );
	Ogre::Real fMoveTime;


	//  Camera Angles
	CameraAngle* ca;
	int miCount, miCurrent;
	std::vector<CameraAngle*> mCameraAngles;
	//  first (after change, reset etc)
	bool first;  int iFirst;  void First();

	bool loadCameras();  void saveCamera(), Destroy();
	void updAngle(), incCur(int dir);
	void Next(bool bPrev = false, bool bMainOnly = false);
	void setCamera(int ang);
	
	bool TypeCar() {  return ca && ca->mType == CAM_Car;  }
	
	//  info text formats
	Ogre::String sFmt_Follow, sFmt_Free, sFmt_ExtAng, sFmt_Arena, sFmt_Car;
	void updFmtTxt();
};
