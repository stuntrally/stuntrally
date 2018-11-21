#ifndef SH_OGREMATERIALSERIALIZER_H
#define SH_OGREMATERIALSERIALIZER_H

#include <OgrePrerequisites.h>

#if defined(OGRE_VERSION) && OGRE_VERSION > 0x10A00
// MaterialSerializer was dropped in Ogre 1.11, we keep our own copy for now
#include "OgreMainMaterialSerializer.h"
#else
#include <OgreMaterialSerializer.h>
#endif

namespace Ogre
{
	class Pass;
}

namespace sh
{
	/**
	 * @brief This class allows me to let Ogre handle the pass & texture unit properties
	 */
	class OgreMaterialSerializer : public Ogre::MaterialSerializer
	{
	public:
		bool setPassProperty (const std::string& param, std::string value, Ogre::Pass* pass);
		bool setTextureUnitProperty (const std::string& param, std::string value, Ogre::TextureUnitState* t);
		bool setMaterialProperty (const std::string& param, std::string value, Ogre::MaterialPtr m);

	private:
		void reset();
	};

}

#endif
