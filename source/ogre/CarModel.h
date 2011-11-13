/*
 * CarModel is the "Ogre" part of the car.
 * It is mainly used to put the different meshes together,
 * but also for e.g. particle emitters and color change. 
 */
 
#ifndef _CarModel_H_
#define _CarModel_H_
 
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreVector4.h>
#include <OgreMatrix4.h>
#include <OgreColourValue.h>


class SETTINGS;  class GAME;  class CAR;  class Scene;  class App;  class FollowCamera;  class CarReflection;

namespace Ogre {  class SceneNode;  class Terrain;  class Camera;  class SceneManager;
	class ParticleSystem;  class Entity;  class RibbonTrail;  class ManualObject;  }


// Stores all the needed information about car coming from vdrift
struct PosInfo
{
	Ogre::Vector3 pos, carY;  Ogre::Vector2 miniPos;
	Ogre::Vector3 whPos[4];  Ogre::Quaternion rot, whRot[4];  float whR[4];
	float whVel[4], whSlide[4], whSqueal[4];
	int whTerMtr[4],whRoadMtr[4];  float fboost;
	bool bNew;  //  new posinfo available for Update

	PosInfo() : bNew(false), pos(0,0,0), miniPos(0,0)  // not inited
	{}
};

class CarModel
{
public:
	/// -------------------- Car Types ---------------------------
	//              Control         Physics (VDrift car)    Camera
	// CT_LOCAL:    Local player    yes	                    yes
	// CT_REPLAY:   Replay file     no                      yes
	// CT_GHOST:	Replay file		no						no
	// CT_REMOTE:   Network	        yes	                    no
	enum eCarType {  CT_LOCAL=0, CT_REPLAY, CT_GHOST, CT_REMOTE };
	eCarType eType;

	//  Constructor, will assign members and create the vdrift car
	CarModel( unsigned int index, eCarType type, const std::string name,
		Ogre::SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* sc, Ogre::Camera* cam, App* app);
	
	//  Destructor - will remove meshes & particle systems, 
	//  the VDrift car and the FollowCamera, delete pReflect
	~CarModel();
	
	//  Create our car, based on name, color, index.
	//  This will put the meshes together and create the particle systems.
	//  CreateReflection() is also called.
	void Create(int car);
	void CreateReflection();
	
	void RecreateMaterials();
	void setMtrNames(); // assign materials to entity / manualobject
	void setMtrName(const Ogre::String& entName, const Ogre::String& mtrName);
	
	//  Call every vdrift substep with new position info
	void Update(PosInfo& newPosInfo, float time);
	void UpdateKeys();  // for camera X,C, last chk F12
	
	//  Car color, After these values are changed, ChangeClr() should be called
	Ogre::ColourValue color;  // for minimap pos tri color  //float hue, sat, val;
	void ChangeClr(int car);  //  Apply new color
		
	//  track surface for wheels
	void UpdWhTerMtr();
	
	//  Update trails
	void UpdParsTrails(bool visible=true);
	
	//  Create ogre model from .joe, Static method so VDrift track (TrackVdr.cpp) can use this too
	static Ogre::ManualObject* CreateModel( Ogre::SceneManager* sceneMgr, const Ogre::String& mat,
		class VERTEXARRAY* a, Ogre::Vector3 vPofs, bool flip=false, bool track=false, const Ogre::String& name="");

	//  Follow camera for this car.
	//  This can be null (for remote [network] cars)
	FollowCamera* fCam;
	
	//  Main node. later, we will add sub-nodes for body, interior, glass and wheels.
	Ogre::SceneNode* pMainNode;
	
	void setVisible(bool visible);  // hide/show
		
	//  Handles our cube map.
	CarReflection* pReflect;
		
	int whTerMtr[4],whRoadMtr[4];
	//  needed to set track surface
	char* blendMtr; int blendMapSize;
	
	Ogre::Terrain* terrain;
	
	//  VDrift car.  For e.g. replay cars that don't 
	//  need physics simulation, this can be null.
	CAR* pCar;

	//  start pos, lap  chekpoint vars
	bool bGetStPos;  Ogre::Matrix4 matStPos;  Ogre::Vector4 vStDist;
	int iInChk, iCurChk, iNextChk, iNumChks, iWonPlace;  // cur checkpoint -1 at start
	bool bInSt, bWrongChk;  float fChkTime;  int iChkWrong;
	//bool Checkpoint(const PosInfo& posInfo, class SplineRoad* road);  // update
	Ogre::Vector3 vStartPos;  void ResetChecks();
	
private:
	Ogre::Camera* mCamera;

	//  access to vdrift stuff
	GAME* pGame;
	
	//  Scene, needed to get particle settings
	Scene* sc;

	//  SceneManager to use
	Ogre::SceneManager* pSceneMgr;

	//  Material names, will be initialized in Create()
	enum eMaterials {
		Mtr_CarBody, Mtr_CarInterior, Mtr_CarGlass,
		Mtr_CarTireFront, Mtr_CarTireRear,
		NumMaterials  };
	std::string sMtr[NumMaterials];
		
	//  Particle systems, trail
	Ogre::ParticleSystem* ps[4],*pm[4],*pd[4];  // smoke, mud, dust
	Ogre::ParticleSystem* pflW[4],*pflM[4],*pflMs[4];  // water, mud, mud soft
	Ogre::ParticleSystem* pb[2], *ph;  // boost, world hit
	Ogre::RibbonTrail* whTrl[4];
	Ogre::Real wht[4];  // spin time (approx tire temp.)
	Ogre::SceneNode *ndWh[4], *ndWhE[4];
	
	//  Dir name of car (e.g. ES or RS2)
	std::string sDirname;
	
	//  Path to car textures, e.g. /usr/share/stuntrally/data/cars/CT/textures
	std::string resCar;
	
	//  index for the car (e.g. when we have 2 cars, they have indices 0 and 1)
	//  needed for cloned materials & textures
	int iIndex;
	
	//  brake state
	bool bBraking;
	void RefreshBrakingMaterial();
	
	//  lightmap enable/disable depending on dist. to terrain
	bool bLightMapEnabled;
	void UpdateLightMap();
	
	//  cam,chk old states
	int iCamNextOld;
	bool bLastChkOld;
	
	//  Our settings.
	SETTINGS* pSet;
	App* pApp;
};

#endif
