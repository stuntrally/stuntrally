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
	transparent = props->transparent;
	envMap = ((props->envMap != "") && parent->getEnvMap())
		&& (1-props->envMapPriority <= parent->getShaderQuality());
	fresnel = (envMap && props->fresnelScale != 0.f) && (parent->getShaderQuality() >= 0.3);
	diffuseMap = (props->diffuseMaps.size() > 0);
	specMap = (props->specMaps.size() > 0) && (parent->getShaderQuality() >= 0.5);
	lightMap = (props->lightMaps.size() > 0);
	terrainLightMap = props->terrainLightMap && (parent->getShaderQuality() >= 0.2);
	alphaMap = (transparent && (props->alphaMaps.size() > 0));
	blendMap = (props->blendMaps.size() > 0);
	reflectivityMap = (props->reflectivityMaps.size() > 0) && envMap;
	normalMap = ((props->normalMaps.size() > 0) && parent->getNormalMap())
		&& (1-props->normalMapPriority <= parent->getShaderQuality());
	parallax = props->parallax && parent->getShaderQuality() > 0.6;
	lighting = props->lighting;
	shadows = ( (props->receivesShadows && parent->getShadows()) 
			||  (props->receivesDepthShadows && parent->getShadowsDepth())
			  ) && (1-props->shadowPriority <= parent->getShaderQuality());
	lightingAlpha = (props->lightingAlpha != Vector4::ZERO);
	wind = (parent->getShaderQuality() > 0.1) ? props->wind : 0;
	customGenerator = props->customGenerator;
	vertexColour = props->vertexColour;
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
	if (other->specMap != specMap) return false;
	if (other->blendMap != blendMap) return false;
	if (other->reflectivityMap != reflectivityMap) return false;
	if (other->normalMap != normalMap) return false;
	if (other->shadows != shadows) return false;
	if (other->transparent != transparent) return false;
	if (other->lighting != lighting) return false;
	if (other->lightingAlpha != lightingAlpha) return false;
	if (other->customGenerator != customGenerator) return false;
	if (other->wind != wind) return false;
	if (other->vertexColour != vertexColour) return false;
	if (other->parallax != parallax) return false;
	
	return true;
}
