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

	TextureUnitState OgrePass::createTextureUnitState ()
	{
		return OgreTextureUnitState (this);
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
}
