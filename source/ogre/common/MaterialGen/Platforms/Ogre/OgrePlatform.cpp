#include "OgrePlatform.hpp"

namespace sh
{
	OgrePlatform::OgrePlatform(const std::string& resourceGroupName)
		: Platform()
		, mResourceGroup(resourceGroupName)
	{
		Ogre::MaterialManager::getSingleton().addListener(this);
	}

	OgrePlatform::~OgrePlatform ()
	{
	}

	bool OgrePlatform::supportsMaterialQueuedListener ()
	{
		return true;
	}

	Material& OgrePlatform::createMaterial (const std::string& name)
	{
		assert (Ogre::MaterialManager::getSingleton().getByName(name).isNull() && "Material already exists");
		Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create (name, mResourceGroup);
	}

	VertexProgram& OgrePlatform::createVertexProgram (const std::string& name, const std::string& source, Language lang)
	{
	}

	FragmentProgram& OgrePlatform::createFragmentProgram (const std::string& name, const std::string& source, Language lang)
	{
	}

	GeometryProgram& OgrePlatform::createGeometryProgram (const std::string& name, const std::string& source, Language lang)
	{
	}

	Ogre::Technique* OgrePlatform::handleSchemeNotFound (
		unsigned short schemeIndex, const Ogre::String &schemeName, Ogre::Material *originalMaterial,
		unsigned short lodIndex, const Ogre::Renderable *rend)
	{
	}
}
