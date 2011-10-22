#include "pch.h"
#include "../Defines.h"

#include "MaterialDefinition.h"
#include "MaterialFactory.h"

using namespace Ogre;

// constructor with sensible default values
MaterialProperties::MaterialProperties() :
	envMap(""), reflAmount(0.2), bumpScale(1.0), cullHardware(CULL_HW_CLOCKWISE),
	hasFresnel(0), fresnelBias(0), fresnelScale(0), fresnelPower(0),
	receivesShadows(0), receivesDepthShadows(0), shaders(1), transparent(0),
	ambient(0.5, 0.5, 0.5), diffuse(1.0, 1.0, 1.0, 1.0), specular(0.2, 0.2, 0.2, 128),
	depthBias(0), depthCheck(true)
{}

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

void MaterialProperties::setProperty(const std::string& prop, const std::string& value)
{	
	if (prop == "envMap") envMap = value;
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
	else if (prop == "depthBias") depthBias = str2int(value);
	else if (prop == "depthCheck") depthCheck = str2bool(value);
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
	else if (prop == "diffuse") diffuse = str2vec4(value);
	else if (prop == "specular") specular = str2vec4(value);
	
	// tex size in prop string
	else if (Ogre::StringUtil::startsWith(prop, "diffuseMap_", false))
	{
		std::string size = prop.substr(11);
		int isize = Ogre::StringConverter::parseInt(size);
		diffuseMaps.insert( std::make_pair(isize, value) );
	}
	else if (Ogre::StringUtil::startsWith(prop, "normalMap_", false))
	{
		std::string size = prop.substr(10);
		int isize = Ogre::StringConverter::parseInt(size);
		normalMaps.insert( std::make_pair(isize, value) );
	}
	else if (Ogre::StringUtil::startsWith(prop, "alphaMap_", false))
	{
		std::string size = prop.substr(10);
		int isize = Ogre::StringConverter::parseInt(size);
		alphaMaps.insert( std::make_pair(isize, value) );
	}
}

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
