#ifndef SHADERPROPERTIES_H
#define SHADERPROPERTIES_H

// a set of properties that influence the generated shader source code
// these properties can be changed by user (e.g. turning env map off in his settings)

struct ShaderProperties
{
	bool envMap; bool fresnel;
	bool diffuseMap;
	bool alphaMap;
	bool normalMap;
};

#endif
