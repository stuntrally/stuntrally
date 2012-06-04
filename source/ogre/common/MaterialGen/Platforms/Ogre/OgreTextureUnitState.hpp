#ifndef SH_OGRETEXTUREUNITSTATE_H
#define SH_OGRETEXTUREUNITSTATE_H

#include <OgreTextureUnitState.h>

#include "../../Platform.hpp"

namespace sh
{
	class OgrePass;

	class OgreTextureUnitState : public TextureUnitState
	{
	public:
		OgreTextureUnitState (OgrePass* parent);

	private:
		Ogre::TextureUnitState* mTextureUnitState;
	};
}

#endif
