#include "pch.h"
#include "../Defines.h"

#include "MaterialDefinition.h"
#include "MaterialFactory.h"

// constructor with sensible default values
MaterialProperties::MaterialProperties() :
	/*diffuseMap(""), normalMap(""), envMap(""),*/ reflAmount(0.5),
	hasFresnel(0), fresnelBias(0), fresnelScale(0), fresnelPower(0),
	receivesShadows(0), receivesDepthShadows(0),
	ambient(1.0, 1.0, 1.0), diffuse(1.0, 1.0, 1.0, 1.0), specular(0.0, 0.0, 0.0, 0.0)
{}

const inline bool str2bool(const std::string& s)
{
	std::string val = s;
	Ogre::StringUtil::toLowerCase(val);
	if (val == "true") return true;
	/* else */ return false;
}
#define str2float(s) Ogre::StringConverter::parseReal(s)
#define str2vec3(s) Ogre::StringConverter::parseVector3(s)
#define str2vec4(s) Ogre::StringConverter::parseVector4(s)

void MaterialProperties::setProperty(const std::string& prop, const std::string& value)
{
	//if (prop == "diffuseMap") diffuseMap = value;
	//else if (prop == "normalMap") normalMap = value;
	
	if (prop == "envMap") envMap = value;
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
	
	// diffuse/normal map: tex size in prop string
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
}

MaterialDefinition::MaterialDefinition(MaterialFactory* parent, MaterialProperties* props) :
	mFirstRun(1)
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
