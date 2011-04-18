/*
 * CarModel is the "Ogre" part of the car.
 * It is mainly used to put the different meshes together,
 * but also for e.g. particle emitters and color change. 
 */
 
#ifndef _CarModel_H_
#define _CarModel_H_
 
#include "FollowCamera.h"
#include "CarReflection.h"
#include "../vdrift/car.h"
#include "../vdrift/game.h"
#include "../vdrift/settings.h"
#include "common/SceneXml.h"

enum eCarType {  CT_Local=0, CT_Replay, CT_Remote };

// Stores all the needed information about car coming from vdrift
struct PosInfo
{
	Vector3 newPos,newCarY;  
	Vector3 newWhPos[4];  Quaternion newRot, newWhRot[4];  float newWhR[4];
	float newWhVel[4], newWhSlide[4], newWhSqueal[4];  int newWhMtr[4];
};

class CarModel
{
public:
	//--- not used
	eCarType type;

	// Constructor, doesn't really do anything other than assigning some members
	CarModel(unsigned int index, const std::string name, Ogre::SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* sc, Camera* cam); 
	
	// Destructor - will remove meshes & particle systems, 
	// the VDrift car and the FollowCamera, delete pReflect
	~CarModel();
	
	// Create our car, based on name, color, index.
	// This will put the meshes together and create the particle systems.
	// CarReflection is also created.
	void Create();
	
	// Call once per frame with new position info
	// Also updates CarReflection
	void Update(PosInfo newPosInfo, float time);
	
	// Car color
	// After these values are changed, ChangeClr() should be called
	float hue, sat, val;
	
	// Apply new color
	void ChangeClr();
	
	// Reload materials. Not sure if this will be needed.
	void ReloadMats();
	
	// track surface for wheels
	void UpdWhTerMtr();
	
	// Update trails
	void UpdParsTrails();
	
	// Create ogre model from .joe
	// Static method so VDrift track (TrackVdr.cpp) can use this too
	static ManualObject* CreateModel(SceneManager* sceneMgr, const String& mat, class VERTEXARRAY* a, Vector3 vPofs, bool flip=false, bool track=false);

	// Follow camera for this car.
	// This can be null (for remote [network] cars)
	FollowCamera* fCam;
	
	// Main node.
	// later, we will add sub-nodes for body, interior, glass and wheels.
	Ogre::SceneNode* pMainNode;
		
	int whTerMtr[4];
	// needed to set track surface
	char* blendMtr; int blendMapSize;
	
	Terrain* terrain;
	
private:
	Camera* mCamera;

	// access to vdrift stuff
	GAME* pGame;
	
	// Scene, needed to get particle settings
	Scene* sc;

	// SceneManager to use
	Ogre::SceneManager* pSceneMgr;

	// Material names, will be initialized in Create()
	enum eMaterials {
		Mtr_CarBody, Mtr_CarInterior, Mtr_CarGlass,
		Mtr_CarTireFront, Mtr_CarTireRear,
		NumMaterials  };
	std::string sMtr[NumMaterials];

	// Handles our cube map.
	CarReflection* pReflect;
	
	// Particle systems, trail.
	ParticleSystem* ps[4],*pm[4],*pd[4],*pr,*pr2;  // smoke, mud, dust
	RibbonTrail* whTrl[4];
	Real wht[4];  // spin time (approx tire temp.)
	SceneNode *ndWh[4], *ndWhE[4], *ndRs[4],*ndRd[4];

	// Dir name of car (e.g. ES or RS2)
	std::string sDirname;
	
	// Path to car textures, e.g. /usr/share/stuntrally/data/cars/CT/textures
	std::string resCar;
	
	// index for the car (e.g. when we have 2 cars, they have indices 0 and 1)
	// needed for cloned materials & textures
	unsigned int iIndex;
			
	// VDrift car.
	// For e.g. replay cars that don't 
	// need physics simulation, this can be null.
	CAR* pCar;
	
	// Our settings.
	SETTINGS* pSet;
};

#endif
