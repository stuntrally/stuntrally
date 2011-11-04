#ifndef MATERIALPROPERTIES_H
#define MATERIALPROPERTIES_H

#include <OgreStringConverter.h>
#include <OgreString.h>
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
	bool abstract;
	
	bool shaders;
	
	// map: tex size, tex name
	textureMap diffuseMaps; Ogre::TextureUnitState::TextureAddressingMode textureAddressMode;
	textureMap normalMaps; float bumpScale;
	textureMap lightMaps;textureMap blendMaps;
	CullHardwareMode cullHardware;
	CullHardwareMode cullHardwareAmbient; // for ambient pass
	
	SceneBlendMode sceneBlend;
	SceneBlendMode sceneBlendAmbient; // for ambient pass
	textureMap alphaMaps; bool transparent;
	Ogre::Vector4 lightingAlpha; // alpha for ambient, diffuse, spec, diffuse r channel mult
	Ogre::CompareFunction alphaRejectFunc; float alphaRejectValue;
	
	// 2-pass (for pipe glass)
	bool twoPass;
	
	// reflection
	std::string envMap;
	float reflAmount;
	bool hasFresnel; float fresnelBias, fresnelScale, fresnelPower;
	
	// shadows, lighting
	bool receivesShadows, receivesDepthShadows;
	Ogre::Vector3 ambient; Ogre::Vector3 diffuse; Ogre::Vector4 specular;
	bool terrainLightMap;
	bool fog; // enable fog
	bool lighting; // enable lighting
	
	// contribute to ssao yes/no
	bool ssao;
	
	float depthBias; bool depthCheck; bool depthWrite; bool transparentSorting;
	
	
	//!todo:
	// PPX on/off, shading mode (phong etc) for no shaders,
	// [casts_shadows (+priority) (probably not here)],
	// read terrain lightmap on/off
	// specular map (exponent in diffuse map alpha) [or seperate map for trees]
	// normalHeight (height in normal map alpha) [for parallax mapping]
	
	MaterialProperties(); // constructor with sensible default values
	void setProperty(const std::string& prop, const std::string& value);
};

#endif
