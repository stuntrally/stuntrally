/*
 * CarModel is the "Ogre" part of the car.
 * It is mainly used to put the different meshes together,
 * but also for e.g. particle emitters and color change. 
 */
 
#include "FollowCamera.h"
#include "CarReflection.h"
#include "../vdrift/car.h"
#include "../vdrift/game.h"
#include "../vdrift/settings.h"
#include "common/SceneXml.h"

class CarModel
{
public:
	// Constructor, doesn't really do anything other than assigning some members
	CarModel(unsigned int index, const std::string name, Ogre::SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* sc); 
	
	// Destructor - will remove meshes & particle systems, 
	// the VDrift car and the FollowCamera, delete pReflect
	~CarModel();
	
	// Create our car, based on name, color, index.
	// This will put the meshes together and create the particle systems.
	// CarReflection is also created.
	void Create();
	
	// Only calls pReflect->Update();
	// Call once per frame
	void Update();
	
	// Car color
	// After these values are changed, ChangeClr() should be called
	float hue, sat, val;
	
	// Apply new color
	void ChangeClr();
	
	// Reload materials. Not sure if this will be needed.
	void ReloadMats();
	
	// Update trails
	void UpdParsTrails();
	
	// Create ogre model from .joe
	ManualObject* CreateModel(const String& mat, class VERTEXARRAY* a, bool flip=false, bool track=false);

	// Follow camera for this car.
	// This can be null (for remote [network] cars)
	FollowCamera* fCam;
	
private:
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
	int whTerMtr[4];
	SceneNode *ndWh[4], *ndWhE[4], *ndRs[4],*ndRd[4];

	// Dir name of car (e.g. ES or RS2)
	std::string sDirname;
	
	// index for the car (e.g. when we have 2 cars, they have indices 0 and 1)
	// needed for cloned materials & textures
	unsigned int iIndex;
	
	// Offset for the current mesh
	Ogre::Vector3 vPofs;
	
	// Main node.
	// later, we will add sub-nodes for body, interior, glass and wheels.
	Ogre::SceneNode* pMainNode;
	
	// VDrift car.
	// For e.g. replay cars that don't 
	// need physics simulation, this can be null.
	CAR* pCar;
	
	// Our settings.
	SETTINGS* pSet;
};
