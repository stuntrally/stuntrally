#ifndef MATERIALFACTORY_H
#define MATERIALFACTORY_H

#include <vector>
#include "MaterialGenerator.h"

class SETTINGS;  class App;

class MaterialFactory
{
public:
	MaterialFactory();
	~MaterialFactory();
	
	void generate();
	
	#define setIfChanged(s) if (p != s) { s = p; bSettingsChanged = true; }
	
	void setShaders(bool p) { setIfChanged(bShaders) };
	void setNormalMap(bool p) { setIfChanged(bNormalMap) };
	void setEnvMap(bool p) { setIfChanged(bEnvMap) };
	void setShadows(bool p) { setIfChanged(bShadows) };
	void setShadowDepth(bool p) { setIfChanged(bShadowDepth) };
	void setTexSize(unsigned int p) { setIfChanged(iTexSize) };
	
	App* pApp;

private:	
	std::vector<MaterialGenerator*> mGenerators;
	
	bool bShaders, bNormalMap, bEnvMap, bShadows, bShadowDepth;
	unsigned int iTexSize; 
	
	// if false, generate() doesn't do anything
	bool bSettingsChanged;
};

#endif
