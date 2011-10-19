#ifndef MATERIALDEFINITION_H
#define MATERIALDEFINITION_H

#include <string>

#include <OgreMaterial.h>
#include <OgreStringConverter.h>
#include <OgreString.h>
#include <OgreVector3.h>
#include <OgreVector4.h>
#include <OgreHighLevelGpuProgram.h>

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
		
	void generate(bool fixedFunction=false); // craft material
	
	/// get/set
	const std::string& getName() { return mName; };
	MaterialProperties* getProps() { return mProps; };
	void setName(const std::string& name) { mName = name; };
	
	MaterialProperties* mProps;

private:
	MaterialFactory* mParent;
	
	bool mFirstRun;
	
	std::string mName; // name of generated material
	
	///  ------------------------ utility methods ---------------------------------------
	// get pointer to material if it exists and delete all techniques, if not, create new
	Ogre::MaterialPtr prepareMaterial(const std::string& matName);
	
	// vertex program
	Ogre::HighLevelGpuProgramPtr createVertexProgram();
	void generateVertexProgramSource(Ogre::StringUtil::StrStreamType& outStream);
	void vertexProgramParams(Ogre::HighLevelGpuProgramPtr program);
	
	// fragment program
	Ogre::HighLevelGpuProgramPtr 	createFragmentProgram();
	void generateFragmentProgramSource(Ogre::StringUtil::StrStreamType& outStream);
	void fragmentProgramParams(Ogre::HighLevelGpuProgramPtr program);
	
	bool needShadows();
	bool needNormalMap();
	bool needEnvMap();
	bool fpNeedWsNormal(); bool fpNeedEyeVector();
	
	std::string getChannel(unsigned int n);

	
	// pick best texture size (not higher than user tex size)
	std::string pickTexture(textureMap* textures);
	/// ----------------------------------------------------------------------------------
};

#endif
