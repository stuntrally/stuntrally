
/*
 * CarReflection handles cube maps for our cars.
 * One object of CarReflection is created for every car.
 * Depending on the user's configuration, it will either render
 * a seperate cube map for each car, or one cubemap for all cars,
 * or no cube map at all (static map loaded from file)
 */

#include "../vdrift/settings.h"

class CarReflection
{
public:
	// Constructor, creates texture and intializes the camera and render targets.
	CarReflection(SETTINGS* set, unsigned int index);
	
	// Destructor, will delete the texture, and cameras / render targets.
	~CarReflection();

	// Since we support a "Skip frames" feature to increase performance,
	// we have to update the render targets manually.
	// This method should be called once every frame.
	void Update();

private:
	// TODO some members are missing here for cube map implementation

	// index of the car this reflection belongs to.
	// The cube map textures have an index too, so we need this to get the right texture / material.
	unsigned int iIndex;

	// Settings, needed to get the user settings for cube maps
	SETTINGS* pSet;
};
