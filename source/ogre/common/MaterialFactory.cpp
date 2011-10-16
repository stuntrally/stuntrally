#include "MaterialFactory.h"
#include "MaterialGenerator.h"

#include "CarMaterialGen.h"

#include "../Defines.h"

MaterialFactory::MaterialFactory() : 
	bShaders(1), bNormalMap(1), bEnvMap(1), bShadows(1), bShadowDepth(1), iTexSize(2048), 
	bSettingsChanged(1) // always have to generate at start
{
	// add our generators
	mGenerators.push_back( (new CarMaterialGenerator())->setParent(this) );

}

MaterialFactory::~MaterialFactory()
{
	for (std::vector<MaterialGenerator*>::iterator it=mGenerators.begin();
			it != mGenerators.end(); ++it)
	{
		delete (*it);
	}
}

void MaterialFactory::generate()
{
	if (bSettingsChanged)
	{
		LogO("[MaterialFactory] generating new materials...");
		for (std::vector<MaterialGenerator*>::iterator it=mGenerators.begin();
				it != mGenerators.end(); ++it)
		{
			(*it)->generate();
		}
		bSettingsChanged = false;
	}
	else
		LogO("[MaterialFactory] settings not changed, using old materials");
}
