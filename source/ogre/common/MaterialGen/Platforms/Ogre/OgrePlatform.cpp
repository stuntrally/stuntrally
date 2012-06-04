#include "OgrePlatform.hpp"

#include "OgreMaterial.hpp"
#include "OgreVertexProgram.hpp"
#include "OgreFragmentProgram.hpp"
#include "OgreGeometryProgram.hpp"

namespace
{
	std::string convertLang (sh::Language lang)
	{
		if (lang == sh::Language_CG)
			return "cg";
		else if (lang == sh::Language_HLSL)
			return "hlsl";
		else if (lang == sh::Language_GLSL)
			return "glsl";
	}
}

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

	Material OgrePlatform::createMaterial (const std::string& name)
	{
		OgreMaterial material (name, mResourceGroup);
		return material;
	}

	VertexProgram OgrePlatform::createVertexProgram (
		const std::string& name, const std::string& entryPoint,
		const std::string& source, Language lang)
	{
		OgreVertexProgram prog (name, entryPoint, source, convertLang(lang), mResourceGroup);
		return prog;
	}

	FragmentProgram OgrePlatform::createFragmentProgram (
		const std::string& name, const std::string& entryPoint,
		const std::string& source, Language lang)
	{
		OgreFragmentProgram prog (name, entryPoint, source, convertLang(lang), mResourceGroup);
		return prog;
	}

	GeometryProgram OgrePlatform::createGeometryProgram (
		const std::string& name, const std::string& entryPoint,
		const std::string& source, Language lang)
	{
		OgreGeometryProgram prog (name, entryPoint, source, convertLang(lang), mResourceGroup);
		return prog;
	}

	Ogre::Technique* OgrePlatform::handleSchemeNotFound (
		unsigned short schemeIndex, const Ogre::String &schemeName, Ogre::Material *originalMaterial,
		unsigned short lodIndex, const Ogre::Renderable *rend)
	{
		std::cout << "listener activated " << std::endl;

		return originalMaterial->createTechnique(); /// \todo
	}
}
