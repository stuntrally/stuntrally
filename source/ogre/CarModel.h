/*
 * CarModel is the "Ogre" part of the car.
 * It is mainly used to put the different meshes together,
 * but also for e.g. particle emitters and color change. 
 */
 
#include "FollowCamera.h"
#include "CarReflection.h"
#include "../vdrift/car.h"
#include "../vdrift/settings.h"

class CarModel
{
public:
	// Constructor, doesn't really do anything other than assigning some members
	CarModel(unsigned int index, const std::string name, Ogre::SceneManager* sceneMgr, SETTINGS* set); 
	
	// Destructor - will remove meshes & particle systems, 
	// the VDrift car and the FollowCamera, delete pReflect
	~CarModel();
	
	// Create our car, based on name, color, index.
	// This will put the meshes together and create the particle systems.
	// CarReflection is also created.
	void Create();
	
	// Car color
	// After these values are changed, ChangeClr() should be called
	float hue, sat, val;
	
	// Apply new color
	void ChangeClr();
	
	// Follow camera for this car.
	// This can be null (for remote [network] cars)
	FollowCamera* fCam;
	
private:
	// SceneManager to use
	Ogre::SceneManager* pSceneMgr;

	// Handles our cube map.
	CarReflection* pReflect;

	// Dir name of car (e.g. ES or RS2)
	std::string sDirname;
	
	// index for the car (e.g. when we have 2 cars, they have indices 0 and 1)
	// needed for cloned materials & textures
	unsigned int iIndex;
	
	// Main node.
	// later, we will add sub-nodes for body, interior, glass and wheels.
	Ogre::SceneNode* pMainNode;
	
	// VDrift car.
	// For replay cars or remote cars, that don't 
	// need physics simulation, this can be null.
	CAR* pCar;
	
	// Our settings.
	SETTINGS* pSet;
};
