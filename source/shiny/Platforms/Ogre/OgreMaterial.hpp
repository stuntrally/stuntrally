#ifndef SH_OGREMATERIAL_H
#define SH_OGREMATERIAL_H

#include <string>

#include <OgreMaterial.h>

#include "../../Main/Platform.hpp"

namespace sh
{
	class OgreMaterial : public Material
	{
	public:
		OgreMaterial (const std::string& name, const std::string& resourceGroup);
		virtual ~OgreMaterial();

		virtual boost::shared_ptr<Pass> createPass (const std::string& configuration);
		virtual void createConfiguration (const std::string& name);
		virtual void removeConfiguration (const std::string& name); ///< safe to call if configuration does not exist

		Ogre::MaterialPtr getOgreMaterial();

		Ogre::Technique* getOgreTechniqueForConfiguration (const std::string& configurationName);

	private:
		Ogre::MaterialPtr mMaterial;
	};
}

#endif
