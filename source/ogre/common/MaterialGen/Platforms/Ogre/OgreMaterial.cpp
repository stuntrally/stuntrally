#include "OgreMaterial.hpp"

#include <OgreMaterialManager.h>

namespace sh
{
	OgreMaterial::OgreMaterial (const std::string& name, const std::string& resourceGroup)
		: Material()
	{
		assert (Ogre::MaterialManager::getSingleton().getByName(name).isNull() && "Material already exists");
		mMaterial = Ogre::MaterialManager::getSingleton().create (name, resourceGroup);
	}
}
