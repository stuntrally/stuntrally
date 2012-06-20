#include "OgrePass.hpp"

#include <OgrePass.h>
#include <OgreTechnique.h>

#include "OgreTextureUnitState.hpp"
#include "OgreMaterial.hpp"

namespace sh
{
	OgrePass::OgrePass (OgreMaterial* parent)
		: Pass()
	{
		mPass = parent->getOgreMaterial()->getBestTechnique()->createPass();
	}

	boost::shared_ptr<TextureUnitState> OgrePass::createTextureUnitState ()
	{
		return boost::shared_ptr<TextureUnitState> (new OgreTextureUnitState (this));
	}

	void OgrePass::assignVertexProgram (const VertexProgram& program)
	{
	}

	void OgrePass::assignFragmentProgram (const FragmentProgram& program)
	{
	}

	void OgrePass::assignGeometryProgram (const GeometryProgram& program)
	{
	}

	Ogre::Pass* OgrePass::getOgrePass ()
	{
		return mPass;
	}

	bool OgrePass::setPropertyOverride (const std::string &name, PropertyValuePtr& value)
	{
		bool found = true;

		if (name == "depthWrite")
			mPass->setDepthWriteEnabled(PropertyValue::retrieve<BooleanValue>(value)->get());
		else if (name == "depthCheck")
			mPass->setDepthCheckEnabled(PropertyValue::retrieve<BooleanValue>(value)->get());
		else if (name == "colourWrite")
			mPass->setColourWriteEnabled(PropertyValue::retrieve<BooleanValue>(value)->get());
		else
			found = false;

		return found;
	}
}
