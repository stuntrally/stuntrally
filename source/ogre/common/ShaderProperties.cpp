#include "pch.h"
#include "../Defines.h"

#include "ShaderProperties.h"
#include "MaterialFactory.h"
#include "MaterialProperties.h"

#include <OgreString.h>
#include <OgreVector4.h>
using namespace Ogre;

//----------------------------------------------------------------------------------------

ShaderProperties::ShaderProperties( MaterialProperties* props, MaterialFactory* parent )
{
	//!todo priority for envmap, normalmap, shadow
	
	transparent = props->transparent;
	envMap = ((props->envMap != "") && parent->getEnvMap());
	fresnel = (envMap && props->fresnelScale != 0.f);
	diffuseMap = (props->diffuseMaps.size() > 0);
	lightMap = (props->lightMaps.size() > 0);
	terrainLightMap = props->terrainLightMap;
	alphaMap = (transparent && (props->alphaMaps.size() > 0));
	blendMap = (props->blendMaps.size() > 0);
	normalMap = ((props->normalMaps.size() > 0) && parent->getNormalMap());
	lighting = props->lighting;
	shadows = (lighting && (
			    (props->receivesShadows && parent->getShadows()) 
			||  (props->receivesDepthShadows && parent->getShadowsDepth())
	));
	lightingAlpha = (props->lightingAlpha != Vector4::ZERO);
}

//----------------------------------------------------------------------------------------

bool ShaderProperties::isEqual( ShaderProperties* other )
{
	if (other->envMap != envMap) return false;
	if (other->fresnel != fresnel) return false;
	if (other->diffuseMap != diffuseMap) return false;
	if (other->lightMap != lightMap) return false;
	if (other->terrainLightMap != terrainLightMap) return false;
	if (other->alphaMap != alphaMap) return false;
	if (other->blendMap != blendMap) return false;
	if (other->normalMap != normalMap) return false;
	if (other->shadows != shadows) return false;
	if (other->transparent != transparent) return false;
	if (other->lighting != lighting) return false;
	if (other->lightingAlpha != lightingAlpha) return false;
	
	return true;
}
