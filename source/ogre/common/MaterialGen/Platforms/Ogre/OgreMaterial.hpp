#ifndef SH_OGREMATERIAL_H
#define SH_OGREMATERIAL_H

#include <string>

#include <OgreMaterial.h>

#include "../../Platform.hpp"

namespace sh
{
	class OgreMaterial : public Material
	{
	public:
		OgreMaterial (const std::string& name, const std::string& resourceGroup);
		virtual boost::shared_ptr<Pass> createPass ();

		Ogre::MaterialPtr getOgreMaterial();

	private:
		Ogre::MaterialPtr mMaterial;
	};
}

#endif
