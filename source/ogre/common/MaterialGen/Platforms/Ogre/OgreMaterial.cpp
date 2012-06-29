#include "OgreMaterial.hpp"

#include <OgreMaterialManager.h>
#include <OgreTechnique.h>

#include "OgrePass.hpp"

namespace sh
{
	static const std::string sDefaultTechniqueName = "SH_DefaultTechnique";

	OgreMaterial::OgreMaterial (const std::string& name, const std::string& resourceGroup)
		: Material()
	{
		assert (Ogre::MaterialManager::getSingleton().getByName(name).isNull() && "Material already exists");
		mMaterial = Ogre::MaterialManager::getSingleton().create (name, resourceGroup);
		mMaterial->removeAllTechniques();
		mMaterial->createTechnique()->setSchemeName (sDefaultTechniqueName);
	}

	OgreMaterial::~OgreMaterial()
	{
		Ogre::MaterialManager::getSingleton().remove(mMaterial->getName());
	}

	boost::shared_ptr<Pass> OgreMaterial::createPass (const std::string& configuration)
	{
		return boost::shared_ptr<Pass> (new OgrePass (this, configuration));
	}

	void OgreMaterial::createConfiguration (const std::string& name)
	{
		Ogre::Technique* t = mMaterial->createTechnique();
		t->setSchemeName (name);
		t->setName (name);
	}

	void OgreMaterial::removeConfiguration (const std::string& name)
	{
		for (int i=0; i<mMaterial->getNumTechniques(); ++i)
		{
			if (mMaterial->getTechnique(i) == mMaterial->getTechnique(name))
				mMaterial->removeTechnique(i);
		}
	}

	Ogre::MaterialPtr OgreMaterial::getOgreMaterial ()
	{
		return mMaterial;
	}

	Ogre::Technique* OgreMaterial::getOgreTechniqueForConfiguration (const std::string& configurationName)
	{
		return mMaterial->getTechnique (configurationName); /// \todo don't use strings here?
	}
}
