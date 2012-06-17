#include "PassDefinition.hpp"

namespace sh
{
	TextureUnitStateDefinition* PassDefinition::createTextureUnitStateDefinition()
	{
		TextureUnitStateDefinition newTex;
		mTextureUnitStates.push_back(newTex);
		return &mTextureUnitStates.back();
	}
}
