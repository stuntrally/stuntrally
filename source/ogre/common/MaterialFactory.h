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
		
	void generate();
	
	/// user settings get/set ---------------------------------------------
	#define setIfChanged(s) if (p != s) { s = p; bSettingsChanged = true; }
	
	void setShaders(bool p) { setIfChanged(bShaders) };
	void setNormalMap(bool p) { setIfChanged(bNormalMap) };
	void setEnvMap(bool p) { setIfChanged(bEnvMap) };
	void setShadows(bool p) { setIfChanged(bShadows) };
	void setShadowsDepth(bool p) { setIfChanged(bShadowsDepth) };
	void setTexSize(unsigned int p) { setIfChanged(iTexSize) };
	
	const bool getShaders() { return bShaders; };
	const bool getNormalMap() { return bNormalMap; };
	const bool getEnvMap() { return bEnvMap; };
	const bool getShadows() { return bShadows; };
	const bool getShadowsDepth() { return bShadowsDepth; };
	const unsigned int getTexSize() { return iTexSize; };
	///--------------------------------------------------------------------
	
	/// user settings definition ---------------------------------
	bool bShaders, bNormalMap, bEnvMap, bShadows, bShadowsDepth;
	unsigned int iTexSize;
	/// -------------------------------------------------------
	
	App* pApp;

private:
	std::vector<MaterialDefinition*> mDefinitions;
	
	Ogre::ConfigFile mFile; // for loading mat def's from file

	//!todo split points - maintain a list of materials that need it

	// if false, generate() doesn't do anything
	bool bSettingsChanged;
	
	void loadDefsFromFile(const std::string& file); // load MaterialDefinition's from file (can be multiple in 1 file)
};

#endif
