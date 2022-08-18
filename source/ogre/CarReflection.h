
/*
 * CarReflection handles cube maps for our cars.
 * One object of CarReflection is created for every car.
 * Depending on the user's configuration, it will either render
 * a seperate cube map for each car, or one cubemap for all cars,
 * or no cube map at all (static map loaded from file)
 */
#pragma once
#include <Ogre.h>
// #include <OgreVector3.h>
// #include <OgreString.h>
// #include <OgreTexture.h>
#include <vector>


namespace Ogre {  class Camera;  class SceneManager;  }
class SETTINGS;  class App;



class CarReflection
{
public:
	// Constructor, assign members.
	CarReflection(SETTINGS* set, App* app, Ogre::SceneManager* sceneMgr, unsigned int index, char suffix);
	
	// Destructor, will delete the texture, and cameras / render targets.
	~CarReflection();
	
	// Creates texture and intializes the camera and render targets.
	void Create();

	// Since we support a "Skip frames" feature to increase performance,
	// we have to update the render targets manually.
	// This method should be called once every frame.
	void Update(bool first = false);  // use first = true to force update all 6 maps
	
	// Position of the cameras; will be set by CarModel::Update
	Ogre::Vector3 camPosition;
	
	// Names of the materials, with index
	// In c'tor we will iterate through these and replace occurrences of ReflectionCube with ReflectionCube<Index> e.g. ReflectionCube0
	enum eMaterials {
		Mtr_CarBody, Mtr_CarInterior, Mtr_CarGlass,
		Mtr_CarTireFront, Mtr_CarTireRear,
		NumMaterials  };
	Ogre::String sMtr[NumMaterials];
	
	std::vector<Ogre::Camera*> vCams;

private:
	// SceneManager to use, needed to create refl. cameras.
	Ogre::SceneManager* pSceneMgr;
	
	// Pointer to the cubemap texture.
	// if all cars use the same cube map, this is a pointer to the first texture.
	Ogre::TexturePtr cubetex;
	Ogre::String cubetexName;
		
	// RTT cameras.
	// can be null, if static cube maps or only 1 cube map.
	Ogre::Camera* pCams[6];
	
	// RTT targets.
	// same as cam, can be null.
	Ogre::RenderTarget* pRTs[6];
	
	// Used for frame skip
	// When this is 0, render once and then set it to max again
	unsigned int iCounter;
	
	// Index of current cam.
	// Used for faces at once setting.
	unsigned int iCam;

	// index of the car this reflection belongs to.
	// The cube map textures have an index too, so we need this to get the right texture / material.
	unsigned int iIndex;
	char hSuffix;  // also for name (is same index for 2nd and track ghost)
	
	SETTINGS* pSet;
	App* pApp;
};
