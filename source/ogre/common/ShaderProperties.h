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
	bool specMap;
	bool normalMap;
	bool lighting;
	bool shadows;
	bool transparent; bool lightingAlpha; // transparency
	unsigned int wind;
	bool vertexColour;
	
	// use custom generator (for very specific materials like water, glass)
	// empty ("") means no custom generator
	std::string customGenerator; 
	
	// compare
	bool isEqual( ShaderProperties* other );
	
	// constructor
	ShaderProperties( MaterialProperties* props, MaterialFactory* parent );
};

#endif
