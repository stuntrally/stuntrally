#ifndef MATERIALGENERATOR_H
#define MATERIALGENERATOR_H

/*class MaterialDefinition;*/ class MaterialFactory;
#include "MaterialDefinition.h" // textureMap typedef

#include <OgreHighLevelGpuProgram.h>

class MaterialGenerator
{
public:
	MaterialDefinition* mDef;
	MaterialFactory* mParent;
		
	void generate(bool fixedFunction=false); // craft material
	
protected:
	// tex unit indices
	unsigned int mDiffuseTexUnit;
	unsigned int mAlphaTexUnit;
	unsigned int mNormalTexUnit;
	unsigned int mEnvTexUnit;
	unsigned int mShadowTexUnit_start; // start offset for shadow tex units
	
	unsigned int mTexUnit_i; // counter
	
	/// utility methods
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
	
	//unsigned int countTexUnits();
	
	//!todo
	// unsigned int countTexCoords();
	
	bool needShaders();
	bool needShadows();
	bool needNormalMap(); bool needEnvMap(); bool needAlphaMap();
	bool fpNeedWsNormal(); bool fpNeedEyeVector();
	bool fpNeedTangentToCube(); bool vpNeedTangent();
	//bool fpNeedLighting(); // fragment lighting
	//bool vpNeedLighting(); // vertex lighting
	
	// matrices
	bool vpNeedWMat();
	bool vpNeedWITMat();
	
	std::string getChannel(unsigned int n);
	
	// pick best texture size (not higher than user tex size)
	std::string pickTexture(textureMap* textures);


};

#endif
