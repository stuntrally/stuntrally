#include "OgreMaterial.hpp"

#include <OgreMaterialManager.h>

#include "OgrePass.hpp"

namespace sh
{
	OgreMaterial::OgreMaterial (const std::string& name, const std::string& resourceGroup)
		: Material()
	{
		assert (Ogre::MaterialManager::getSingleton().getByName(name).isNull() && "Material already exists");
		mMaterial = Ogre::MaterialManager::getSingleton().create (name, resourceGroup);
	}

	boost::shared_ptr<Pass> OgreMaterial::createPass ()
	{
		return boost::shared_ptr<Pass> (new OgrePass (this));
	}

	Ogre::MaterialPtr OgreMaterial::getOgreMaterial ()
	{
		return mMaterial;
	}
}
