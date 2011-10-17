#include "pch.h"
#include "../Defines.h"

#include "MaterialFactory.h"
#include "MaterialDefinition.h"

MaterialFactory::MaterialFactory() : 
	bShaders(1), bNormalMap(1), bEnvMap(1), bShadows(1), bShadowsDepth(1), iTexSize(2048), 
	bSettingsChanged(1) // always have to generate at start
{
	// temporary test.
	MaterialDefinition* test = new MaterialDefinition(this, "general.matdef");
	mDefinitions.push_back(test);
	
	//!todo search all resource paths and load *.matdef files
	
	//!todo not clear.. car .matdef's loaded here or in carmodel?
}

//----------------------------------------------------------------------------------------

MaterialFactory::~MaterialFactory()
{
	for (std::vector<MaterialDefinition*>::iterator it=mDefinitions.begin();
		it!=mDefinitions.end(); ++it)
		delete (*it);
}

//----------------------------------------------------------------------------------------

void MaterialFactory::generate()
{
	if (bSettingsChanged)
	{
		LogO("[MaterialFactory] generating new materials...");
		
		for (std::vector<MaterialDefinition*>::iterator it=mDefinitions.begin();
			it!=mDefinitions.end(); ++it)
			(*it)->generate();
		
		bSettingsChanged = false;
	}
	else
		LogO("[MaterialFactory] settings not changed, using old materials");
}

//----------------------------------------------------------------------------------------

