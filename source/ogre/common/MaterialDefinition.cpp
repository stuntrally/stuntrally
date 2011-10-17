#include "pch.h"
#include "../Defines.h"

#include "MaterialDefinition.h"
#include "MaterialFactory.h"

#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreTextureUnitState.h>
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

void MaterialDefinition::generate()
{
	//!todo
}

//----------------------------------------------------------------------------------------

MaterialPtr MaterialDefinition::prepareMaterial(const std::string& name)
{
	MaterialPtr mat;
	if (MaterialManager::getSingleton().resourceExists(name))
	{
		mat = MaterialManager::getSingleton().getByName(name);
		mat->removeAllTechniques();
	}
	else
	{
		mat = MaterialManager::getSingleton().create(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	}
	return mat;
}

//----------------------------------------------------------------------------------------

inline bool MaterialDefinition::needShadows()
{
	return (mProps->receivesShadows && mParent->getShadows())
		|| (mProps->receivesDepthShadows && mParent->getShadowsDepth());
	//!todo shadow priority
}

inline bool MaterialDefinition::needNormalMap()
{
	return (mProps->normalMap != "") && mParent->getNormalMap();
	//!todo normal map priority
}

inline bool MaterialDefinition::needEnvMap()
{
	return (mProps->envMap != "") && mParent->getEnvMap();
	//!todo env map priority
}

//----------------------------------------------------------------------------------------









/*	MaterialPtr mat = prepareMaterial(name);
	
	Technique* technique = mat->createTechnique();
	
	// single pass
	Pass* pass = technique->createPass();
	
	if (!mParent->bShaders)
	{
		// fallback material without shaders
		// only diffuse texture
		TextureUnitState* tus = pass->createTextureUnitState(properties->diffuseMap);
		
		//!todo env map
	}
	else
	{
		//pass->setVertexProgram("main_vs");
		//pass->setFragmentProgram("main_ps");
		pass->setVertexProgram("diffuse_vs");
		pass->setFragmentProgram("diffuse_ps_env");
		
		pass->setAmbient(0.9,0.9,0.9);
		pass->setDiffuse(0.8,0.8,0.8,1.0);
		pass->setSpecular(1,1,1,64);
		
		// diffuse map
		TextureUnitState* tu = pass->createTextureUnitState("body_dyn.png");
		tu->setName("diffuseMap");
		
		// normal map
		tu = pass->createTextureUnitState("flat_n.png"); //!
		tu->setName("normalMap");
		
		// env map
		tu = pass->createTextureUnitState();
		tu->setCubicTextureName("ReflectionCube", true);
		
		//if (  	(properties->receivesShadows && mParent->bShadows) 
		//	 || (properties->receivesDepthShadows && mParent->bShadowDepth)
		//   )
		/*{
			// shadow textures
			for (int i = 0; i < 3; ++i) //!todo num textures
			{
				tu = pass->createTextureUnitState();
				tu->setName("shadowMap" + toStr(i));
				tu->setContentType(TextureUnitState::CONTENT_SHADOW);
				tu->setTextureAddressingMode(TextureUnitState::TAM_BORDER);
				tu->setTextureBorderColour(ColourValue::White);
			}
		}*/
/*	}
}*/
