#include "pch.h"
#include "../Defines.h"
#include "MaterialProperties.h"

#include <OgreCommon.h>
#include <OgreLogManager.h>
using namespace Ogre;

//----------------------------------------------------------------------------------------

// constructor with sensible default values
MaterialProperties::MaterialProperties() :
	abstract(false),
	envMap(""), reflAmount(0.2), bumpScale(1.0), cullHardware(CULL_HW_CLOCKWISE), cullHardwareAmbient(CULL_HW_CLOCKWISE),
	hasFresnel(false), fresnelBias(0), fresnelScale(0), fresnelPower(0),
	receivesShadows(false), receivesDepthShadows(false), shaders(true), transparent(false),
	ambient(0.5, 0.5, 0.5), diffuse(1.0, 1.0, 1.0), specular(0.0, 0.0, 0.0, 0.0),
	depthBias(0), depthCheck(true), transparentSorting(true), lightingAlpha(0.0, 0.0, 0.0, 0.0),
	sceneBlend(SBM_DEFAULT), depthWrite(true), alphaRejectFunc(CMPF_ALWAYS_PASS), alphaRejectValue(0.0),
	twoPass(false), fog(true), lighting(true), textureAddressMode(TextureUnitState::TAM_WRAP)
{}

//----------------------------------------------------------------------------------------

// utility
const inline bool str2bool(const std::string& s)
{
	std::string val = s;
	Ogre::StringUtil::toLowerCase(val);
	if (val == "true") return true;
	/* else */ return false;
}
#define str2int(s) StringConverter::parseInt(s)
#define str2float(s) StringConverter::parseReal(s)
#define str2vec3(s) StringConverter::parseVector3(s)
#define str2vec4(s) StringConverter::parseVector4(s)

//----------------------------------------------------------------------------------------

void MaterialProperties::setProperty(const std::string& prop, const std::string& value)
{
	if (prop == "abstract") abstract = str2bool(value);
	else if (prop == "envMap") envMap = value;
	else if (prop == "cullHardware")
	{
		if (value == "clockwise") cullHardware = CULL_HW_CLOCKWISE;
		else if (value == "none") cullHardware = CULL_HW_NONE;
		else if (value == "anticlockwise") cullHardware = CULL_HW_ANTICLOCKWISE;
		else if (value == "none_if_depthshadow")
		{
			if (cullHardware == CULL_HW_CLOCKWISE)
				cullHardware = CULL_HW_CLOCKWISE_OR_NONE;
			else if (cullHardware == CULL_HW_ANTICLOCKWISE)
				cullHardware = CULL_HW_ANTICLOCKWISE_OR_NONE;
		}
	}
	else if (prop == "cullHardwareAmbient")
	{
		if (value == "clockwise") cullHardwareAmbient = CULL_HW_CLOCKWISE;
		else if (value == "none") cullHardwareAmbient = CULL_HW_NONE;
		else if (value == "anticlockwise") cullHardwareAmbient = CULL_HW_ANTICLOCKWISE;
		else if (value == "none_if_depthshadow")
		{
			if (cullHardwareAmbient == CULL_HW_CLOCKWISE)
				cullHardwareAmbient = CULL_HW_CLOCKWISE_OR_NONE;
			else if (cullHardware == CULL_HW_ANTICLOCKWISE)
				cullHardwareAmbient = CULL_HW_ANTICLOCKWISE_OR_NONE;
		}
	}
	else if (prop == "sceneBlend")
	{
		if (value == "alpha") sceneBlend = SBM_ALPHA_BLEND;
		else if (value == "colour") sceneBlend = SBM_COLOUR_BLEND;
		else if (value == "add") sceneBlend = SBM_ADD;
		else if (value == "modulate") sceneBlend = SBM_MODULATE;
	}
	else if (prop == "alphaRejectFunc")
	{
		if (value == "always_fail") alphaRejectFunc = CMPF_ALWAYS_FAIL;
		else if (value == "always_pass") alphaRejectFunc = CMPF_ALWAYS_PASS;
		else if (value == "less") alphaRejectFunc = CMPF_LESS;
		else if (value == "equal") alphaRejectFunc = CMPF_EQUAL;
		else if (value == "not_equal") alphaRejectFunc = CMPF_NOT_EQUAL;
		else if (value == "greater_equal") alphaRejectFunc = CMPF_GREATER_EQUAL;
		else if (value == "greater") alphaRejectFunc = CMPF_GREATER;
	}
	else if (prop == "textureAddressMode")
	{
		if (value == "wrap") textureAddressMode = TextureUnitState::TAM_WRAP;
		else if (value == "clamp") textureAddressMode = TextureUnitState::TAM_CLAMP;
		else if (value == "mirror") textureAddressMode = TextureUnitState::TAM_MIRROR;
		else if (value == "border") textureAddressMode = TextureUnitState::TAM_BORDER;
	}
	else if (prop == "lighting") lighting = str2bool(value);
	else if (prop == "fog") fog = str2bool(value);
	else if (prop == "twoPass") twoPass = str2bool(value);
	else if (prop == "alphaRejectValue") alphaRejectValue = str2float(value);
	else if (prop == "depthWrite") depthWrite = str2bool(value);
	else if (prop == "lightingAlpha") lightingAlpha = str2vec4(value);
	else if (prop == "depthBias") depthBias = str2int(value);
	else if (prop == "depthCheck") depthCheck = str2bool(value);
	else if (prop == "transparentSorting") transparentSorting = str2bool(value);
	else if (prop == "shaders") shaders = str2bool(value);
	else if (prop == "transparent") transparent = str2bool(value);
	else if (prop == "bumpScale") bumpScale = str2float(value);
	else if (prop == "hasFresnel") hasFresnel = str2bool(value);
	else if (prop == "reflAmount") reflAmount = str2float(value);
	else if (prop == "fresnelBias") fresnelBias = str2float(value);
	else if (prop == "fresnelScale") fresnelScale = str2float(value);
	else if (prop == "fresnelPower") fresnelPower = str2float(value);
	else if (prop == "receivesShadows") receivesShadows = str2bool(value);
	else if (prop == "receivesDepthShadows") receivesDepthShadows = str2bool(value);
	else if (prop == "ambient") ambient = str2vec3(value);
	else if (prop == "diffuse") diffuse = str2vec3(value);
	else if (prop == "specular") specular = str2vec4(value);
	
	// tex size in prop string
	else if (Ogre::StringUtil::startsWith(prop, "diffuseMap_", false))
	{
		std::string size = prop.substr(11);
		int isize = Ogre::StringConverter::parseInt(size);
		diffuseMaps[isize] = value;
	}
	else if (Ogre::StringUtil::startsWith(prop, "normalMap_", false))
	{
		std::string size = prop.substr(10);
		int isize = Ogre::StringConverter::parseInt(size);
		normalMaps[isize] = value;
	}
	else if (Ogre::StringUtil::startsWith(prop, "alphaMap_", false))
	{
		std::string size = prop.substr(10);
		int isize = Ogre::StringConverter::parseInt(size);
		alphaMaps[isize] = value;
	}
	else
	{
		if (Ogre::StringUtil::startsWith(prop, ";")) // ';' means comment, ignore
			return;
		if (prop == "parent") // parent setting is parsed elsewhere
			return;
			
		LogO("[MaterialFactory] WARNING: Unknown attribute '" + prop + "'");
	}
}
