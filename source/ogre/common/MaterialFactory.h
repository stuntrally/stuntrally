#ifndef MATERIALFACTORY_H
#define MATERIALFACTORY_H

#include <vector>

class App;  class MaterialDefinition;

#include <OgreConfigFile.h>

class MaterialFactory
{
public:
	MaterialFactory();
	~MaterialFactory();
	
	// maximum of 4 shadow textures
	static const unsigned int SHADOWTEX_NUM_MAX = 4;
	
	void generate();
	
	/// user settings get/set ---------------------------------------------
	#define setIfChanged(s) if (p != s) { s = p; bSettingsChanged = true; }
	
	void setShaders(bool p) { setIfChanged(bShaders) };
	void setNormalMap(bool p) { setIfChanged(bNormalMap) };
	void setEnvMap(bool p) { setIfChanged(bEnvMap) };
	void setShadows(bool p) { setIfChanged(bShadows) };
	void setShadowsDepth(bool p) { setIfChanged(bShadowsDepth) };
	void setTexSize(unsigned int p) { setIfChanged(iTexSize) };
	void setNumShadowTex(unsigned int p) { setIfChanged(iNumShadowTex) };
	
	const bool getShaders() { return bShaders; };
	const bool getNormalMap() { return bNormalMap; };
	const bool getEnvMap() { return bEnvMap; };
	const bool getShadows() { return bShadows; };
	const bool getShadowsDepth() { return bShadowsDepth; };
	const unsigned int getTexSize() { return iTexSize; };
	const unsigned int getNumShadowTex() { return iNumShadowTex; };
	///--------------------------------------------------------------------
	
	std::vector<std::string> splitMtrs; // list of materials that need pssm split points
	
	App* pApp;

private:
	/// user settings definition ---------------------------------
	bool bShaders, bNormalMap, bEnvMap, bShadows, bShadowsDepth;
	unsigned int iTexSize; unsigned int iNumShadowTex;
	/// -------------------------------------------------------

	std::vector<MaterialDefinition*> mDefinitions;
	
	Ogre::ConfigFile mFile; // for loading mat def's from file

	//!todo decide which materials actually need to be generated
	/// (if they are not used in track, no need to generate)

	// if false, generate() doesn't do anything
	bool bSettingsChanged;
	
	void loadDefsFromFile(const std::string& file); // load MaterialDefinition's from file (can be multiple in 1 file)
};

#endif
