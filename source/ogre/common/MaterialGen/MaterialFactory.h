#ifndef MATERIALFACTORY_H
#define MATERIALFACTORY_H

#include <vector>

class App;  class MaterialDefinition;  class MaterialGenerator;  struct ShaderProperties;

namespace Ogre { class SceneManager; class Terrain; class PSSMShadowCameraSetup; };

#include <OgreMaterial.h>
#include <OgreConfigFile.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreSingleton.h>

//!todo memory usage: unload old textures after texture size was switched

//!todo more intelligent shader caching
/// - only regenerate when shader properties changed, not when e.g. only tex size was changed
/// - cache vertex and fragment shader seperately (some materials will have same vertex shader, but different fragment)
/// - how about making a hash out of the shader source code, and use this for isEqual comparison?

//!todo paged-geom cloned materials don't update immediately (code for recreating those materials)

// std::map< std::pair< vertexShader, pixelShader > , shaderProperties >
typedef std::map< std::pair< Ogre::HighLevelGpuProgramPtr, Ogre::HighLevelGpuProgramPtr >, ShaderProperties* > shaderMap;

class MaterialFactory : public Ogre::Singleton<MaterialFactory>
{
public:
	MaterialFactory();
	~MaterialFactory();
	
    static MaterialFactory& getSingleton(void);
    static MaterialFactory* getSingletonPtr(void);
	
	// maximum of 3 shadow textures
	static const unsigned int SHADOWTEX_NUM_MAX = 3;
	
	// once at start (or after settings changed)
	void generate();
	
	// per-frame updates
	void update();
	
	// force re-generating shaders when generate() will be run the next time
	void markDirty() { bSettingsChanged = true; };
	
	void setSceneManager(Ogre::SceneManager* pSceneMgr);
	Ogre::SceneManager* getSceneManager();
	
	void setTerrain(Ogre::Terrain* pTerrain);
	Ogre::Terrain* getTerrain();
	
	// set necessary shader parameters for a given material that was created by the material factory.
	// this method does NOT have to be called from outside, except when you've cloned a material.
	// in that case, call this method for all materials that you've cloned
	void setShaderParams(Ogre::MaterialPtr);
	
	/// settings that can change runtime
	void setFog(bool fog);
	void setWind(bool wind);
	void setSoftParticleDepth(Ogre::TexturePtr depthtexture);
	void setSoftParticles(bool bEnable);
	void setShadowsEnabled(bool bEnable);
	
	/// user settings get/set ---------------------------------------------
	#define setIfChanged(s) if (p != s) { s = p; bSettingsChanged = true; } // changes shader source ( need recompile)
	#define setIfChangedP(s) if (p != s) { s = p; } // only changes uniform param value
	
	void setNormalMap(const bool p) { setIfChanged(bNormalMap) };
	void setEnvMap(const bool p) { setIfChanged(bEnvMap) };
	void setReflect(const bool p) { setIfChanged(bReflect) };
	void setRefract(const bool p) { setIfChanged(bRefract) };
	void setShadows(const bool p) { setIfChanged(bShadows) };
	void setShadowsFilterSize(const unsigned int p) { setIfChanged(iShadowsFilterSize) };
	void setShadowsDepth(const bool p) { setIfChanged(bShadowsDepth) };
	void setShadowsSoft(const bool p) { setIfChanged(bShadowsSoft) };
	void setShadowsFade(const bool p) { setIfChanged(bShadowsFade) };
	void setShadowsFadeDistance(const float p) { setIfChangedP(fShadowsFadeDistance) };
	void setPSSMCameraSetup(Ogre::PSSMShadowCameraSetup*);
	void setTexSize(const unsigned int p) { setIfChanged(iTexSize) };
	void setNumShadowTex(const unsigned int p) { if(p> SHADOWTEX_NUM_MAX) { iNumShadowTex=SHADOWTEX_NUM_MAX; } else { setIfChanged(iNumShadowTex) } };
	void setShaderQuality(const float p) { setIfChanged(fShaderQuality) };
		
	const bool getReflect() { return bReflect; };
	const bool getRefract() { return bRefract; };
	const bool getNormalMap() { return bNormalMap; };
	const bool getEnvMap() { return bEnvMap; };
	const bool getShadows() { return bShadows; };
	const unsigned int getShadowsFilterSize() { return iShadowsFilterSize; };
	const bool getShadowsDepth() { return bShadowsDepth; };
	const bool getShadowsSoft() { return bShadowsSoft; };
	const bool getShadowsFade() { return bShadowsFade; };
	const float getShadowsFadeDistance() { return fShadowsFadeDistance; };
	const unsigned int getTexSize() { return iTexSize; };
	const unsigned int getNumShadowTex() { return iNumShadowTex; };
	const float getShaderQuality() { return fShaderQuality; };
	///--------------------------------------------------------------------
	
	std::vector<std::string> fogMtrs; // materials that involve fog
	std::vector<std::string> windMtrs; // wind == 2
	std::vector<std::string> timeMtrs; // for animated materials
	std::vector<std::string> softMtrs; // for soft particle materials

	shaderMap* getShaderCache() { return &mShaderCache; };
			
	App* pApp;

private:
	/// user settings definition ---------------------------------
	bool bNormalMap, bEnvMap, bShadows, bShadowsDepth, bShadowsSoft, bShadowsFade;
	unsigned int iTexSize; unsigned int iNumShadowTex; unsigned int iShadowsFilterSize;
	float fShaderQuality, fShadowsFadeDistance;
	bool bReflect, bRefract;
	/// -------------------------------------------------------
	
	Ogre::SceneManager* mSceneMgr; // scene manager (used for e.g. retrieving shadow settings)
	Ogre::Terrain* mTerrain; // terrain (used for retrieving the lightmap as well as updating shadow fade parameter)
	Ogre::PSSMShadowCameraSetup* mPSSM;

	std::vector<MaterialDefinition*> mDefinitions;
	
	MaterialGenerator* mGenerator;
	std::vector<MaterialGenerator*> mCustomGenerators;
	
	shaderMap mShaderCache;
	void deleteShaderCache(); // cleanup
	
	Ogre::ConfigFile mFile; // for loading mat def's from file

	// if false, generate() doesn't do anything
	bool bSettingsChanged;
		
	void loadDefsFromFile(const std::string& file); // load MaterialDefinition's from file (can be multiple in 1 file)
};

#endif
