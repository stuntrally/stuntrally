#ifndef SH_OGREPLATFORM_H
#define SH_OGREPLATFORM_H

#include "../../Platform.hpp"

#include <OgreMaterialManager.h>

namespace sh
{
	class OgrePlatform : public Platform, public Ogre::MaterialManager::Listener
	{
	public:
		OgrePlatform (const std::string& resourceGroupName);
		virtual ~OgrePlatform ();

		virtual Ogre::Technique* handleSchemeNotFound (
			unsigned short schemeIndex, const Ogre::String &schemeName, Ogre::Material *originalMaterial,
			unsigned short lodIndex, const Ogre::Renderable *rend);

		virtual Material createMaterial (const std::string& name);

		virtual VertexProgram createVertexProgram (
			const std::string& name, const std::string& entryPoint,
			const std::string& source, Language lang);
		virtual FragmentProgram createFragmentProgram (
			const std::string& name, const std::string& entryPoint,
			const std::string& source, Language lang);
		virtual GeometryProgram createGeometryProgram (
			const std::string& name, const std::string& entryPoint,
			const std::string& source, Language lang);

	protected:
		virtual bool supportsMaterialQueuedListener ();

		std::string mResourceGroup;
	};
}

#endif
