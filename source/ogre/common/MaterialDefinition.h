#ifndef MATERIALDEFINITION_H
#define MATERIALDEFINITION_H

#include <string>

#include <OgreMaterial.h>
#include <OgreStringConverter.h>
#include <OgreString.h>

/*
 * MaterialDefinition holds several material parameters
 * (e.g. has normal map, has env map, receives shadows...)
 * and has methods to read these parameters from a .matdef file,
 * as well as a method to craft a specific material out of these
 * parameters, depending on user's settings (retrieved via mParent).
 */
 
class MaterialFactory;

struct MaterialProperties
{
	std::string diffuseMap;
	std::string normalMap;
	std::string envMap;
	bool hasFresnel; float fresnelBias, fresnelScale, fresnelPower;
	bool receivesShadows, receivesDepthShadows;
	
	//!todo:
	// ambient/diffuse/spec colors, PPX on/off, shaders on/off, 
	// normalmap/shadowmap/envmap "priority", twosided yes/no/only_when_receiver,
	// alpha, [casts_shadows (probably not here)]
	
	// constructor with sensible default values
	MaterialProperties() :
		diffuseMap(""), normalMap(""), envMap(""), 
		hasFresnel(0), fresnelBias(0), fresnelScale(0), fresnelPower(0),
		receivesShadows(0), receivesDepthShadows(0)
	{}
	
	const bool str2bool(const std::string& s)
	{
		std::string val = s;
		Ogre::StringUtil::toLowerCase(val);
		if (val == "true") return true;
		/* else */ return false;
	}
	#define str2float(s) Ogre::StringConverter::parseReal(s)
	
	void setProperty(const std::string& prop, const std::string& value)
	{
		if (prop == "diffuseMap") diffuseMap = value;
		else if (prop == "normalMap") normalMap = value;
		else if (prop == "envMap") envMap = value;
		else if (prop == "hasFresnel") hasFresnel = str2bool(value);
		else if (prop == "fresnelBias") fresnelBias = str2float(value);
		else if (prop == "fresnelScale") fresnelScale = str2float(value);
		else if (prop == "fresnelPower") fresnelPower = str2float(value);
		else if (prop == "receivesShadows") receivesShadows = str2bool(value);
		else if (prop == "receivesDepthShadows") receivesDepthShadows = str2bool(value);
	}
};

class MaterialDefinition
{
public:
	MaterialDefinition(MaterialFactory* parent, MaterialProperties* props);
	~MaterialDefinition();
		
	void generate(); // craft material
	
	/// get/set
	const std::string& getName() { return mName; };
	MaterialProperties* getProps() { return mProps; };
	void setName(const std::string& name) { mName = name; };
	
	MaterialProperties* mProps;

private:
	MaterialFactory* mParent;
	
	std::string mName; // name of generated material
	
	/// utility methods
	// get pointer to material if it exists and delete all techniques, if not, create new
	Ogre::MaterialPtr prepareMaterial(const std::string& matName);
	
	bool needShadows();
	bool needNormalMap();
	bool needEnvMap();
};

#endif
