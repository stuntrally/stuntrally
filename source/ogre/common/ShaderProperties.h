#ifndef SHADERPROPERTIES_H
#define SHADERPROPERTIES_H

struct MaterialProperties;  class MaterialFactory;

struct ShaderProperties
{
	bool envMap; bool fresnel;
	bool diffuseMap;
	bool lightMap;
	bool terrainLightMap;
	bool alphaMap;
	bool blendMap;
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
