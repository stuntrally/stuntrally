#ifndef MATERIALFACTORY_H
#define MATERIALFACTORY_H

#include <vector>

class App;  class MaterialDefinition;  class MaterialGenerator;  struct ShaderProperties;

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
	
	// maximum of 4 shadow textures
	static const unsigned int SHADOWTEX_NUM_MAX = 4;
	
	// once at start (or after settings changed)
	void generate();
	
	// per-frame updates
	void update();
	
	/// settings that can change runtime
	void setFog(bool fog);
	void setWind(bool wind);
	void setSoftParticleDepth(Ogre::TexturePtr depthtexture);
	void setSoftParticles(bool bEnable);

	
	/// user settings get/set ---------------------------------------------
	#define setIfChanged(s) if (p != s) { s = p; bSettingsChanged = true; }
	
	void setNormalMap(bool p) { setIfChanged(bNormalMap) };
	void setEnvMap(bool p) { setIfChanged(bEnvMap) };
	void setShadows(bool p) { setIfChanged(bShadows) };
	void setShadowsDepth(bool p) { setIfChanged(bShadowsDepth) };
	void setTexSize(unsigned int p) { setIfChanged(iTexSize) };
	void setNumShadowTex(unsigned int p) { setIfChanged(iNumShadowTex) };
	void setShaderQuality(float p) { setIfChanged(fShaderQuality) };
	
	const bool getNormalMap() { return bNormalMap; };
	const bool getEnvMap() { return bEnvMap; };
	const bool getShadows() { return bShadows; };
	const bool getShadowsDepth() { return bShadowsDepth; };
	const unsigned int getTexSize() { return iTexSize; };
	const unsigned int getNumShadowTex() { return iNumShadowTex; };
	const float getShaderQuality() { return fShaderQuality; };
	///--------------------------------------------------------------------
	
	std::vector<std::string> splitMtrs; // materials that need pssm split points
	std::vector<std::string> terrainLightMapMtrs; // materials that need terrain lightmap texture and terrainWorldSize
	std::vector<std::string> fogMtrs; // materials that involve fog
	std::vector<std::string> windMtrs; // wind == 2
	std::vector<std::string> timeMtrs; // for animated materials
	std::vector<std::string> softMtrs; // for soft particle materials

	shaderMap* getShaderCache() { return &mShaderCache; };
	
	App* pApp;

private:
	/// user settings definition ---------------------------------
	bool bNormalMap, bEnvMap, bShadows, bShadowsDepth;
	unsigned int iTexSize; unsigned int iNumShadowTex;
	float fShaderQuality;
	/// -------------------------------------------------------

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
