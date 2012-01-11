#ifndef MATERIALPROPERTIES_H
#define MATERIALPROPERTIES_H

#include <OgreStringConverter.h>
#include <OgreString.h>
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreVector4.h>
#include <OgreCommon.h>
#include <OgreTextureUnitState.h>

typedef std::map<unsigned int, std::string> textureMap;

enum CullHardwareMode
{
	CULL_HW_NONE=0x00, CULL_HW_CLOCKWISE=0x01, CULL_HW_ANTICLOCKWISE=0x02,

	// or = for depth shadows on
	CULL_HW_CLOCKWISE_OR_NONE=0x03, CULL_HW_ANTICLOCKWISE_OR_NONE=0x04
};

enum SceneBlendMode
{
	SBM_ADD=0x00, SBM_MODULATE=0x01, SBM_COLOUR_BLEND=0x02, SBM_ALPHA_BLEND=0x03, SBM_DEFAULT=0x04
};

struct MaterialProperties
{
	// set this to 'true' if the material is never used (only a base material that other materials have as parent)
	bool abstract;
	
	// use shaders (there is no reason to set this to false, the fixed function support is very limited and most likely broken)
	bool shaders;
	
	// --------- textures -----------------------
	// textureMap: map of textures and their size
	
	// diffuse map (affects color)
	textureMap diffuseMaps;
	
	// tangent space normal map
	textureMap normalMaps;
	float bumpScale; // default = 1; decrease or increase this to change effect of normal map
	
	// this is an alternative to diffuse maps that can be used to 
	// change the color of certain parts of a material on-the-fly.
	// the lightMap holds the brightness of parts with dynamic color in 'r' channel
	// the blendMap contains parts that should NOT be affected by the color change
	textureMap lightMaps; textureMap blendMaps;
	
	// specular map. holds the specular color in RGB, shininess in alpha (0 to 255).
	textureMap specMaps;
	
	// alpha map, transparency in 'r' channel (has no effect if 'transparent' is false)
	textureMap alphaMaps; bool transparent;
	Ogre::Vector4 lightingAlpha; // multiply alpha with ambient, diffuse, spec, (1-red channel of diffuse map)
	
	// environment cube map (useful for reflections) 
	std::string envMap;
	float reflAmount; // amount of reflection (no effect if fresnel effect is used)
	// fresnel effect (dynamic amount of reflection, based on camera angle)
	bool hasFresnel; float fresnelBias, fresnelScale, fresnelPower;
	
	// specifies the amount of reflection in 'r' channel (is also used with fresnel effect)
	textureMap reflectivityMaps;
	// ------------------------------------------
	
	// shadows, lighting
	bool receivesShadows, receivesDepthShadows;
	Ogre::Vector3 ambient; Ogre::Vector3 diffuse; Ogre::Vector4 specular;
	bool terrainLightMap;
	bool fog; // enable fog
	bool lighting; // enable lighting
	
	// contribute to ssao yes/no
	bool ssao;
	bool ssaoReject; // alpha reject greater 128 for ssao technique
	
	// wind waving effect
	// 0 = off, 1 = for grass (quad), 2 = for trees (mesh)
	// this relies on paged-geom to set the appropriate parameters for the effect
	unsigned int wind;
	
	// multiply the colour output with the vertex colour
	bool vertexColour;
	
	// use custom generator (for very specific materials like water, glass)
	// empty ("") means the default generator
	std::string customGenerator; 
	
	
	// water
	Ogre::Vector2 waveBump;
	float waveHighFreq;
	float waveSpecular;
	Ogre::Vector4 deepColour, shallowColour, reflectionColour;
	
	// priority for various properties (for 'Shader quality' slider)
	// 0 ... 1, default: 0.5
	float envMapPriority, shadowPriority, normalMapPriority;
	
	// regular ogre material settings, refer to the ogre manual
	CullHardwareMode cullHardware;
	Ogre::TextureUnitState::TextureAddressingMode textureAddressMode;
	SceneBlendMode sceneBlend;
	Ogre::CompareFunction alphaRejectFunc; float alphaRejectValue;
	float depthBias; bool depthCheck; bool depthWrite; bool transparentSorting;
	
	//!todo:
	/*
	 * shader features:
	 * - per vertex lighting
	 * - object space normal maps (useful for meshes with mirrored UVs)
	 * - ambient occlusion map (pre-baked, seperate map) - this map would only be used when SSAO effect is off
	 * - diffuseSpec map (specular intensity in diffuse map alpha) 
	 * - seperate ambient map (instead of using the diffuse map for ambient color)
	 * - parallax mapping, normalHeight map (height in normal map alpha)
	 * - displacement mapping
	 * 
	 * portability / reusability:
	 * - support different light types (point, directional, spot)
	 * - support emissive lighting calculations
	 * - support more than 1 light at a time (most likely using pass iteration)
	 * - support shadowing methods aside from PSSM
	 * 
	 * other:
	 * - make it easier to add custom shaders
	 * - shader cache on disk
	 */
	
	MaterialProperties(); // constructor with sensible default values
	void setProperty(const std::string& prop, const std::string& value);
};

#endif
