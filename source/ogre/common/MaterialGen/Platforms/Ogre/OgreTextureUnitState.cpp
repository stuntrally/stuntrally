#include "OgreTextureUnitState.hpp"

#include "OgrePass.hpp"

namespace sh
{
	OgreTextureUnitState::OgreTextureUnitState (OgrePass* parent)
		: TextureUnitState()
	{
		//mTextureUnitState = parent->getOgrePass()->createTextureUnitState("");
	}
}
