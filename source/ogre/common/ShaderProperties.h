#ifndef SHADERPROPERTIES_H
#define SHADERPROPERTIES_H

class MaterialProperties;  class MaterialFactory;

struct ShaderProperties
{
	bool envMap; bool fresnel;
	bool diffuseMap;
	bool alphaMap;
	bool normalMap;
	bool lighting;
	bool shadows;
	bool transparent; bool lightingAlpha; // transparency
	
	// compare
	bool isEqual( ShaderProperties* other );
	
	// constructor
	ShaderProperties( MaterialProperties* props, MaterialFactory* parent );
};

#endif
