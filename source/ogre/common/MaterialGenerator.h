#ifndef MATERIALGENERATOR_H
#define MATERIALGENERATOR_H

class MaterialFactory;  struct ShaderProperties;
#include "MaterialDefinition.h" // textureMap typedef

#include <OgreHighLevelGpuProgram.h>
#include <OgreGpuProgramParams.h>

class MaterialGenerator
{
public:
	MaterialDefinition* mDef;
	ShaderProperties* mShader;
	MaterialFactory* mParent;
	
	// shader cache
	bool mShaderCached;
	Ogre::HighLevelGpuProgramPtr mVertexProgram;
	Ogre::HighLevelGpuProgramPtr mFragmentProgram;
	
	void generate(bool fixedFunction=false); // craft material
	
protected:
	// tex unit indices
	unsigned int mDiffuseTexUnit;
	unsigned int mLightTexUnit;
	unsigned int mBlendTexUnit;
	unsigned int mAlphaTexUnit;
	unsigned int mNormalTexUnit;
	unsigned int mEnvTexUnit;
	unsigned int mTerrainLightTexUnit; // global terrain lightmap
	unsigned int mShadowTexUnit_start; // start offset for shadow tex units
	
	unsigned int mTexUnit_i; // counter
	
	/// utility methods
	// get pointer to material if it exists and delete all techniques, if not, create new
	Ogre::MaterialPtr prepareMaterial(const std::string& matName);
	
	// vertex program
	Ogre::HighLevelGpuProgramPtr createVertexProgram();
	void generateVertexProgramSource(Ogre::StringUtil::StrStreamType& outStream);
	void vertexProgramParams(Ogre::HighLevelGpuProgramPtr program);
	void individualVertexProgramParams(Ogre::GpuProgramParametersSharedPtr params);
	
	Ogre::HighLevelGpuProgramPtr createAmbientVertexProgram(); // ambient pass vertex program
	
	// fragment program
	Ogre::HighLevelGpuProgramPtr 	createFragmentProgram();
	void generateFragmentProgramSource(Ogre::StringUtil::StrStreamType& outStream);
	void fragmentProgramParams(Ogre::HighLevelGpuProgramPtr program);
	void individualFragmentProgramParams(Ogre::GpuProgramParametersSharedPtr params);
	
	Ogre::HighLevelGpuProgramPtr createAmbientFragmentProgram(); // ambient pass fragment program
	
	bool needShaders();
	bool needShadows();
	
	// textures
	bool needNormalMap(); bool needEnvMap();
	bool needAlphaMap(); bool needBlendMap();
	bool needDiffuseMap(); bool needLightMap();
	bool needTerrainLightMap();
	
	// vertex shader input
	bool vpNeedTangent();
	bool vpNeedWMat();
	bool vpNeedWITMat();
	
	// passtrough (vertex to fragment)
	bool fpNeedTangentToCube();
	bool fpNeedWsNormal();
	bool fpNeedEyeVector();
	 
	//MRT
	bool vpNeedWvMat();
	bool UseMRT();

	// lighting
	bool fpNeedLighting(); // fragment lighting
	//bool vpNeedLighting(); // vertex lighting
	
	bool needFresnel();
	bool needLightingAlpha();
	
	
	std::string getChannel(unsigned int n);
	
	// pick best texture size (not higher than user tex size)
	std::string pickTexture(textureMap* textures);

	Ogre::CullingMode chooseCullingMode();
	Ogre::CullingMode chooseCullingModeAmbient();

};

#endif
