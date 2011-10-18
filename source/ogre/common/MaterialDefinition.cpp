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
	MaterialPtr mat = prepareMaterial(mName);
	mat->setReceiveShadows(false);
		
	// test
	mParent->setShaders(false);
	//mParent->setEnvMap(false);
	
	// only 1 technique
	Ogre::Technique* technique = mat->createTechnique();
	
	// single pass
	Ogre::Pass* pass = technique->createPass();
	
	pass->setAmbient( mProps->ambient.x, mProps->ambient.y, mProps->ambient.z );
	pass->setDiffuse( mProps->diffuse.x, mProps->diffuse.y, mProps->diffuse.z, mProps->diffuse.w );
	
	if (!mParent->getShaders())
	{
		pass->setSpecular(mProps->specular.x, mProps->specular.y, mProps->specular.z, 1.0 );
		pass->setShininess(mProps->specular.w);
	}
	else
	{
		// shader assumes matShininess in specular w component
		pass->setSpecular(mProps->specular.x, mProps->specular.y, mProps->specular.z, mProps->specular.w);
	}
	
	std::string diffuseMap = pickTexture(&mProps->diffuseMaps);
	std::string normalMap = pickTexture(&mProps->normalMaps);
	
	// test
	//pass->setCullingMode(CULL_NONE);
	//pass->setShadingMode(SO_PHONG);
		
	if (!mParent->getShaders())
	{
		pass->setShadingMode(SO_PHONG);
		
		// diffuse map
		Ogre::TextureUnitState* tu = pass->createTextureUnitState( diffuseMap );
		
		if (needEnvMap())
		{
			// env map
			tu = pass->createTextureUnitState();
			tu->setCubicTextureName( mProps->envMap, true );
			tu->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
			
			// blend with diffuse map using 'reflection amount' property
			tu->setColourOperationEx(LBX_BLEND_MANUAL, LBS_CURRENT, LBS_TEXTURE, 
									ColourValue::White, ColourValue::White, 1-mProps->reflAmount);
		}
	}
	else
	{
		// diffuse map
		Ogre::TextureUnitState* tu = pass->createTextureUnitState( diffuseMap );
		tu->setName("diffuseMap");
		
		// env map
		if (needEnvMap())
		{
			tu = pass->createTextureUnitState( mProps->envMap );
			tu->setName("envMap");
		}
		
		// normal map
		if (needNormalMap())
		{
			tu = pass->createTextureUnitState( normalMap );
			tu->setName("normalMap");
		}
		
		// shadow maps
		if (needShadows())
		{
			for (int i = 0; i < mParent->getNumShadowTex(); ++i)
			{
				tu = pass->createTextureUnitState();
				tu->setName("shadowMap" + toStr(i));
				tu->setContentType(TextureUnitState::CONTENT_SHADOW);
				tu->setTextureAddressingMode(TextureUnitState::TAM_BORDER);
				tu->setTextureBorderColour(ColourValue::White);
			}
		}
		
		//!todo
	}
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
	return (mProps->normalMaps.size() > 0) && mParent->getNormalMap();
	//!todo normal map priority
}

inline bool MaterialDefinition::needEnvMap()
{
	return (mProps->envMap != "") && mParent->getEnvMap();
	//!todo env map priority
}

//----------------------------------------------------------------------------------------

inline std::string MaterialDefinition::pickTexture(textureMap* textures)
{
	if (textures->size() == 0) return "";
	
	// we assume the textures are sorted by size
	textureMap::iterator it;
	for (it = textures->begin(); it != textures->end(); ++it)
	{
		if ( it->first < mParent->getTexSize() ) continue;
		/* else */ break;
	}
	
	if (it == textures->end()) --it;
	
	return it->second;
}
