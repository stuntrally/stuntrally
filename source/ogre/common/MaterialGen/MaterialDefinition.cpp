#include "pch.h"
#include "../../Defines.h"

#include "MaterialDefinition.h"
#include "MaterialFactory.h"
#include "MaterialProperties.h"

using namespace Ogre;

MaterialDefinition::MaterialDefinition(MaterialFactory* parent, MaterialProperties* props)
{
	mParent = parent;
	mProps = props;
	mName = "";
}

//----------------------------------------------------------------------------------------

MaterialDefinition::~MaterialDefinition()
{
	delete mProps;
}

//----------------------------------------------------------------------------------------

MaterialProperties* MaterialDefinition::getProps()
{
	return mProps;
}
