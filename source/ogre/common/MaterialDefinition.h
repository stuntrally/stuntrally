#ifndef MATERIALDEFINITION_H
#define MATERIALDEFINITION_H

#include <string>

#include <OgreMaterial.h>
#include <OgreStringConverter.h>
#include <OgreString.h>
#include <OgreVector3.h>
#include <OgreVector4.h>
#include <OgreHighLevelGpuProgram.h>
 
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
	// PPX on/off, shaders on/off, shading mode (phong etc) for no shaders,
	// normalmap/shadowmap/envmap "priority", cull yes/no,
	// alpha, [casts_shadows (probably not here)],
	
	MaterialProperties(); // constructor with sensible default values
	void setProperty(const std::string& prop, const std::string& value);
};

class MaterialDefinition
{
public:
	MaterialDefinition(MaterialFactory* parent, MaterialProperties* props);
	~MaterialDefinition();
	
	/// get/set
	const std::string& getName() { return mName; };
	MaterialProperties* getProps() { return mProps; };
	void setName(const std::string& name) { mName = name; };
	
	MaterialProperties* mProps;

private:
	MaterialFactory* mParent;
	
	bool mFirstRun;
	
	std::string mName; // name of generated material
};

#endif
