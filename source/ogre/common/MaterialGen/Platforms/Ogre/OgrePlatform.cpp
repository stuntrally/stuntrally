#include "OgrePlatform.hpp"

#include <OgreDataStream.h>
#include <OgreGpuProgramManager.h>

#include "OgreMaterial.hpp"
#include "OgreVertexProgram.hpp"
#include "OgreFragmentProgram.hpp"
#include "OgreGeometryProgram.hpp"

#include "../../MaterialInstance.hpp"

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
	OgrePlatform::OgrePlatform(const std::string& resourceGroupName, const std::string& basePath)
		: Platform(basePath)
		, mResourceGroup(resourceGroupName)
		, mCurrentConfiguration("test2")
	{
		Ogre::MaterialManager::getSingleton().addListener(this);

		Ogre::GpuProgramManager::getSingletonPtr()->setSaveMicrocodesToCache(true);
	}

	OgrePlatform::~OgrePlatform ()
	{
	}

	bool OgrePlatform::supportsShaderSerialization ()
	{
		return true;
	}

	bool OgrePlatform::supportsMaterialQueuedListener ()
	{
		return true;
	}

	boost::shared_ptr<Material> OgrePlatform::createMaterial (const std::string& name)
	{
		OgreMaterial* material = new OgreMaterial(name, mResourceGroup);
		return boost::shared_ptr<Material> (material);
	}

	boost::shared_ptr<VertexProgram> OgrePlatform::createVertexProgram (
		const std::string& compileArguments,
		const std::string& name, const std::string& entryPoint,
		const std::string& source, Language lang)
	{
		OgreVertexProgram* prog = new OgreVertexProgram (compileArguments, name, entryPoint, source, convertLang(lang), mResourceGroup);
		return boost::shared_ptr<VertexProgram> (prog);
	}

	boost::shared_ptr<FragmentProgram> OgrePlatform::createFragmentProgram (
		const std::string& compileArguments,
		const std::string& name, const std::string& entryPoint,
		const std::string& source, Language lang)
	{
		OgreFragmentProgram* prog = new OgreFragmentProgram (compileArguments, name, entryPoint, source, convertLang(lang), mResourceGroup);
		return boost::shared_ptr<FragmentProgram> (prog);
	}

	boost::shared_ptr<GeometryProgram> OgrePlatform::createGeometryProgram (
		const std::string& compileArguments,
		const std::string& name, const std::string& entryPoint,
		const std::string& source, Language lang)
	{
		OgreGeometryProgram* prog = new OgreGeometryProgram (compileArguments, name, entryPoint, source, convertLang(lang), mResourceGroup);
		return boost::shared_ptr<GeometryProgram> (prog);
	}

	Ogre::Technique* OgrePlatform::handleSchemeNotFound (
		unsigned short schemeIndex, const Ogre::String &schemeName, Ogre::Material *originalMaterial,
		unsigned short lodIndex, const Ogre::Renderable *rend)
	{
		if (schemeName != mCurrentConfiguration)
		{
			Ogre::MaterialManager::getSingleton().setActiveScheme(mCurrentConfiguration);
			return 0;
		}

		MaterialInstance* m = fireMaterialRequested(originalMaterial->getName(), schemeName);
		if (m)
		{
			OgreMaterial* _m = static_cast<OgreMaterial*>(m->getMaterial());
			return _m->getOgreTechniqueForConfiguration (schemeName);
		}
		else
			return 0; // material does not belong to us
	}

	void OgrePlatform::serializeShaders (const std::string& file)
	{
		std::fstream output;
		output.open(file.c_str(), std::ios::out | std::ios::binary);
		Ogre::DataStreamPtr shaderCache (OGRE_NEW Ogre::FileStreamDataStream(file, &output, false));
		Ogre::GpuProgramManager::getSingleton().saveMicrocodeCache(shaderCache);
	}

	void OgrePlatform::deserializeShaders (const std::string& file)
	{
		std::ifstream inp;
		inp.open(file.c_str(), std::ios::in | std::ios::binary);
		Ogre::DataStreamPtr shaderCache(OGRE_NEW Ogre::FileStreamDataStream(file, &inp, false));
		Ogre::GpuProgramManager::getSingleton().loadMicrocodeCache(shaderCache);
	}

	void OgrePlatform::notifyFrameEntered ()
	{
		Ogre::MaterialManager::getSingleton().setActiveScheme(mCurrentConfiguration);
	}
}
