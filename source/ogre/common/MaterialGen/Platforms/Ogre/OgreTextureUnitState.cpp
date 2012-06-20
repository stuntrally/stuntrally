#include "OgreTextureUnitState.hpp"

#include "OgrePass.hpp"

namespace sh
{
	OgreTextureUnitState::OgreTextureUnitState (OgrePass* parent)
		: TextureUnitState()
	{
		mTextureUnitState = parent->getOgrePass()->createTextureUnitState("");
	}

	bool OgreTextureUnitState::setPropertyOverride (const std::string &name, PropertyValuePtr& value)
	{
		bool found = true;

		if (name == "address_mode")
		{
			std::string val = PropertyValue::retrieve<StringValue>(value)->get();
			if (val == "clamp")
				mTextureUnitState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
			else if (val == "border")
				mTextureUnitState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_BORDER);
			else if (val == "mirror")
				mTextureUnitState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_MIRROR);
			else if (val == "wrap")
				mTextureUnitState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
		}
		else
			found = false;

		return found;
	}

	void OgreTextureUnitState::setTextureName (const std::string& textureName)
	{
		mTextureUnitState->setTextureName(textureName);
	}
}
