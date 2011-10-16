#include "pch.h"
#include "MaterialGenerator.h"
#include "MaterialFactory.h"

#include <OgreMaterialManager.h>
using namespace Ogre;

MaterialGenerator* MaterialGenerator::setParent(MaterialFactory* parent)
{
	mParent = parent;
	return this;
}

MaterialPtr MaterialGenerator::prepareMaterial(const std::string& name)
{
	MaterialPtr mat;
	if (MaterialManager::getSingleton().resourceExists(name))
	{
		mat = MaterialManager::getSingleton().getByName(name);
		mat->removeAllTechniques();
	}
	else
	{
		mat = MaterialManager::getSingleton().create(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	}
	return mat;
}
