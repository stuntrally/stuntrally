#ifndef SH_OGREPLATFORM_H
#define SH_OGREPLATFORM_H

/**
 * @addtogroup Platforms
 * @{
 */

/**
 * @addtogroup Ogre
 * A set of classes to interact with Ogre's material system
 * @{
 */

#include "../../Main/Platform.hpp"

#include <OgreMaterialManager.h>

namespace sh
{
	class OgrePlatform : public Platform, public Ogre::MaterialManager::Listener
	{
	public:
		OgrePlatform (const std::string& resourceGroupName, const std::string& basePath);
		virtual ~OgrePlatform ();

		virtual void serializeShaders (const std::string& file);
		virtual void deserializeShaders (const std::string& file);

		virtual Ogre::Technique* handleSchemeNotFound (
			unsigned short schemeIndex, const Ogre::String &schemeName, Ogre::Material *originalMaterial,
			unsigned short lodIndex, const Ogre::Renderable *rend);

		virtual boost::shared_ptr<Material> createMaterial (const std::string& name);

		virtual boost::shared_ptr<VertexProgram> createVertexProgram (
			const std::string& compileArguments,
			const std::string& name, const std::string& entryPoint,
			const std::string& source, Language lang);
		virtual boost::shared_ptr<FragmentProgram> createFragmentProgram (
			const std::string& compileArguments,
			const std::string& name, const std::string& entryPoint,
			const std::string& source, Language lang);
		virtual boost::shared_ptr<GeometryProgram> createGeometryProgram (
			const std::string& compileArguments,
			const std::string& name, const std::string& entryPoint,
			const std::string& source, Language lang);

	protected:
		virtual bool supportsShaderSerialization ();
		virtual bool supportsMaterialQueuedListener ();

		virtual void notifyFrameEntered ();

		std::string mResourceGroup;

		std::string mCurrentConfiguration;
	};
}

/**
 * @}
 * @}
 */

#endif
