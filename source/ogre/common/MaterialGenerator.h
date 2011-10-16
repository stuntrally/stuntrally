#ifndef MATERIALGENERATOR_H
#define MATERIALGENERATOR_H

#include <OgreMaterial.h>

class MaterialFactory;

class MaterialGenerator
{
public:
	MaterialGenerator* setParent(MaterialFactory* parent);
	
	virtual void generate() = 0;
	
protected:
	MaterialFactory* mParent;
	
	/// utility methods
	// get pointer to material if it exists and delete all techniques, if not, create new
	Ogre::MaterialPtr prepareMaterial(const std::string& matName);
};

#endif
