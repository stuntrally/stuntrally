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
	unsigned int mNormalTexUnit;
	unsigned int mEnvMapTexUnit;
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
	
	bool needShadows();
	bool needNormalMap();
	bool needEnvMap();
	bool fpNeedWsNormal(); bool fpNeedEyeVector();
	
	std::string getChannel(unsigned int n);
	
	// pick best texture size (not higher than user tex size)
	std::string pickTexture(textureMap* textures);


};

#endif
