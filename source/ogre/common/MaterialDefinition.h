#ifndef MATERIALDEFINITION_H
#define MATERIALDEFINITION_H

#include <string>

#include <OgreMaterial.h>
#include <OgreStringConverter.h>
#include <OgreString.h>
#include <OgreVector3.h>
#include <OgreVector4.h>

/*
 * MaterialDefinition holds several material properties
 * (e.g. has normal map, has env map, receives shadows...)
 * as well as a method to craft a specific material out of these
 * parameters, depending on user's settings.
 */
 
class MaterialFactory;

typedef std::map<unsigned int, std::string> textureMap;

struct MaterialProperties
{
	// map: tex size, tex name
	textureMap diffuseMaps;
	textureMap normalMaps;
	
	std::string envMap;
	float reflAmount;
	bool hasFresnel; float fresnelBias, fresnelScale, fresnelPower;
	bool receivesShadows, receivesDepthShadows;
	Ogre::Vector3 ambient; Ogre::Vector4 diffuse; Ogre::Vector4 specular;
	
	//!todo:
	// PPX on/off, shaders on/off, 
	// normalmap/shadowmap/envmap "priority", cull yes/no,
	// alpha, [casts_shadows (probably not here)],
	
	// constructor with sensible default values
	MaterialProperties() :
		/*diffuseMap(""), normalMap(""), envMap(""),*/ reflAmount(0.5),
		hasFresnel(0), fresnelBias(0), fresnelScale(0), fresnelPower(0),
		receivesShadows(0), receivesDepthShadows(0),
		ambient(1.0, 1.0, 1.0), diffuse(1.0, 1.0, 1.0, 1.0), specular(0.0, 0.0, 0.0, 0.0)
	{}
	
	const bool str2bool(const std::string& s)
	{
		std::string val = s;
		Ogre::StringUtil::toLowerCase(val);
		if (val == "true") return true;
		/* else */ return false;
	}
	#define str2float(s) Ogre::StringConverter::parseReal(s)
	#define str2vec3(s) Ogre::StringConverter::parseVector3(s)
	#define str2vec4(s) Ogre::StringConverter::parseVector4(s)
	
	void setProperty(const std::string& prop, const std::string& value)
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
	
	// pick best texture size (not higher than user tex size)
	std::string pickTexture(textureMap* textures);
};

#endif
