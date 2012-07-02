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
#include <OgreGpuProgramParams.h>

namespace sh
{
	class OgrePlatform : public Platform, public Ogre::MaterialManager::Listener
	{
	public:
		OgrePlatform (const std::string& resourceGroupName, const std::string& basePath);
		virtual ~OgrePlatform ();

		virtual bool isProfileSupported (const std::string& profile);

		virtual void serializeShaders (const std::string& file);
		virtual void deserializeShaders (const std::string& file);

		virtual Ogre::Technique* handleSchemeNotFound (
			unsigned short schemeIndex, const Ogre::String &schemeName, Ogre::Material *originalMaterial,
			unsigned short lodIndex, const Ogre::Renderable *rend);

		virtual boost::shared_ptr<Material> createMaterial (const std::string& name);

		virtual boost::shared_ptr<GpuProgram> createGpuProgram (
			GpuProgramType type,
			const std::string& compileArguments,
			const std::string& name, const std::string& profile,
			const std::string& source, Language lang);

		virtual void setSharedParameter (const std::string& name, PropertyValuePtr value);

	protected:
		virtual bool supportsShaderSerialization ();
		virtual bool supportsMaterialQueuedListener ();

		virtual void notifyFrameEntered ();

		std::string mResourceGroup;

		std::string mCurrentConfiguration;

		std::map <std::string, Ogre::GpuSharedParametersPtr> mSharedParameters;
	};
}

/**
 * @}
 * @}
 */

#endif
