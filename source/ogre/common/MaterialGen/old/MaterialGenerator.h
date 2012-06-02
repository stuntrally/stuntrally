#ifndef MATERIALGENERATOR_H
#define MATERIALGENERATOR_H

#include "MaterialProperties.h" // textureMap typedef

#include <OgreHighLevelGpuProgram.h>
#include <OgreGpuProgramParams.h>
#include <OgreMaterial.h>

class MaterialFactory;  class MaterialDefinition;  struct ShaderProperties;
namespace Ogre { class Pass; }


class MaterialGenerator
{
	friend class MaterialFactory;
public:
	MaterialDefinition* mDef;
	ShaderProperties* mShader;
	MaterialFactory* mParent;
	
	// name of material generator - used for custom (water, glass...)
	// for standard material generator (i.e. this one) name is empty
	std::string mName;
	
	// shader cache
	bool mShaderCached;
	Ogre::HighLevelGpuProgramPtr mVertexProgram;
	Ogre::HighLevelGpuProgramPtr mFragmentProgram;
	
	virtual void generate(); // craft material

	//MRT
	static bool bUseMRT;
	static bool MRTSupported();
	
protected:
	// tex unit indices
	unsigned int mDiffuseTexUnit;
	unsigned int mLightTexUnit;
	unsigned int mBlendTexUnit;
	unsigned int mAlphaTexUnit;
	unsigned int mNormalTexUnit;
	unsigned int mEnvTexUnit;
	unsigned int mSpecTexUnit;
	unsigned int mReflTexUnit;
	unsigned int mTerrainLightTexUnit; // global terrain lightmap
	unsigned int mShadowTexUnit_start; // start offset for shadow tex units
	
	// count
	unsigned int mTexUnit_i;
	unsigned int mTexCoord_i;
	
	// textures
	std::string mDiffuseMap;
	std::string mNormalMap;
	std::string mLightMap;
	std::string mAlphaMap;
	std::string mBlendMap;
	std::string mSpecMap;
	std::string mReflMap;
	virtual void chooseTextures();
	virtual void resetTexUnitCounter();
	virtual void createTexUnits(Ogre::Pass* pass);
	
	// material
	Ogre::MaterialPtr mMaterial;
	
	/// utility methods
	// get pointer to material if it exists and delete all techniques, if not, create new
	virtual Ogre::MaterialPtr prepareMaterial(const std::string& matName);
	
	// techniques
	virtual void createSSAOTechnique();
	virtual void createOccluderTechnique();
	
	// vertex program
	virtual Ogre::HighLevelGpuProgramPtr createVertexProgram();
	virtual void generateVertexProgramSource(Ogre::StringUtil::StrStreamType& outStream);
	virtual void vertexProgramParams(Ogre::HighLevelGpuProgramPtr program);
	virtual void individualVertexProgramParams(Ogre::GpuProgramParametersSharedPtr params);
	
	// fragment program
	virtual Ogre::HighLevelGpuProgramPtr createFragmentProgram();
	virtual void generateFragmentProgramSource(Ogre::StringUtil::StrStreamType& outStream);
	virtual void fragmentProgramParams(Ogre::HighLevelGpuProgramPtr program);
	virtual void individualFragmentProgramParams(Ogre::GpuProgramParametersSharedPtr params);
	
	// source generation helpers
	virtual void vpShadowingParams(Ogre::StringUtil::StrStreamType& outStream);
	virtual void fpRealtimeShadowHelperSource(Ogre::StringUtil::StrStreamType& outStream);
	virtual void fpCalcShadowSource(Ogre::StringUtil::StrStreamType& outStream);
	virtual void fpShadowingParams(Ogre::StringUtil::StrStreamType& outStream);
	
	
	virtual bool needShadows();
	
	// textures
	virtual bool needNormalMap(); virtual bool needEnvMap();
	virtual bool needAlphaMap(); virtual bool needBlendMap();
	virtual bool needDiffuseMap(); virtual bool needLightMap();
	virtual bool needSpecMap();
	virtual bool needTerrainLightMap();
	virtual bool needReflectivityMap();
	
	// vertex shader input
	virtual bool vpNeedTangent();
	virtual bool vpNeedWMat();
	virtual bool vpNeedWITMat();
	virtual bool vpCalcWPos();
	virtual bool vpNeedNormal();
	
	// passtrough (vertex to fragment)
	virtual bool fpNeedWMat();
	virtual bool fpNeedPos();
	virtual bool fpNeedNormal();
	virtual bool fpNeedEyeVector();
	 
	//MRT
	virtual bool vpNeedWvMat();
	virtual bool UsePerPixelNormals();

	// lighting
	virtual bool fpNeedLighting(); // fragment lighting
	//bool vpNeedLighting(); // vertex lighting
	
	virtual bool needFresnel();
	virtual bool needLightingAlpha();
	
	
	virtual std::string getChannel(unsigned int n);
	
	// pick best texture size (not higher than user tex size)
	virtual std::string pickTexture(textureMap* textures);

	virtual Ogre::CullingMode chooseCullingMode();
	virtual Ogre::String chooseShadowCasterMaterial();
};

#endif
