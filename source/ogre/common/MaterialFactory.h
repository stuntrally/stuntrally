#ifndef MATERIALFACTORY_H
#define MATERIALFACTORY_H

#include <vector>
#include "MaterialGenerator.h"

class SETTINGS;

class MaterialFactory
{
public:
	MaterialFactory();
	~MaterialFactory();
	
	void generate();
	
private:
	// bShaders (& ppx lighting), bEnvmap, bShadows, bDepth, iTexSize
	SETTINGS* pSet;
	
	std::vector<MaterialGenerator> mGenerators;
};

#endif
