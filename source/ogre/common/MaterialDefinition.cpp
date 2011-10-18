#include "pch.h"
#include "../Defines.h"

#include "MaterialDefinition.h"
#include "MaterialFactory.h"

#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreTextureUnitState.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreGpuProgramParams.h>
using namespace Ogre;

// constructor with sensible default values
MaterialProperties::MaterialProperties() :
	/*diffuseMap(""), normalMap(""), envMap(""),*/ reflAmount(0.5),
	hasFresnel(0), fresnelBias(0), fresnelScale(0), fresnelPower(0),
	receivesShadows(0), receivesDepthShadows(0),
	ambient(1.0, 1.0, 1.0), diffuse(1.0, 1.0, 1.0, 1.0), specular(0.0, 0.0, 0.0, 0.0)
{}

const inline bool str2bool(const std::string& s)
{
	std::string val = s;
	Ogre::StringUtil::toLowerCase(val);
	if (val == "true") return true;
	/* else */ return false;
}
#define str2float(s) Ogre::StringConverter::parseReal(s)
#define str2vec3(s) Ogre::StringConverter::parseVector3(s)
#define str2vec4(s) Ogre::StringConverter::parseVector4(s)

void MaterialProperties::setProperty(const std::string& prop, const std::string& value)
{
	//if (prop == "diffuseMap") diffuseMap = value;
	//else if (prop == "normalMap") normalMap = value;
	
	if (prop == "envMap") envMap = value;
	else if (prop == "hasFresnel") hasFresnel = str2bool(value);
	else if (prop == "reflAmount") reflAmount = str2float(value);
	else if (prop == "fresnelBias") fresnelBias = str2float(value);
	else if (prop == "fresnelScale") fresnelScale = str2float(value);
	else if (prop == "fresnelPower") fresnelPower = str2float(value);
	else if (prop == "receivesShadows") receivesShadows = str2bool(value);
	else if (prop == "receivesDepthShadows") receivesDepthShadows = str2bool(value);
	else if (prop == "ambient") ambient = str2vec3(value);
	else if (prop == "diffuse") diffuse = str2vec4(value);
	else if (prop == "specular") specular = str2vec4(value);
	
	// diffuse/normal map: tex size in prop string
	else if (Ogre::StringUtil::startsWith(prop, "diffuseMap_", false))
	{
		std::string size = prop.substr(11);
		int isize = Ogre::StringConverter::parseInt(size);
		diffuseMaps.insert( std::make_pair(isize, value) );
	}
	else if (Ogre::StringUtil::startsWith(prop, "normalMap_", false))
	{
		std::string size = prop.substr(10);
		int isize = Ogre::StringConverter::parseInt(size);
		normalMaps.insert( std::make_pair(isize, value) );
	}
}

MaterialDefinition::MaterialDefinition(MaterialFactory* parent, MaterialProperties* props)
{
	mParent = parent;
	mProps = props;
	mName = "";
}

//----------------------------------------------------------------------------------------

MaterialDefinition::~MaterialDefinition()
{
	delete mProps;
}

//----------------------------------------------------------------------------------------

void MaterialDefinition::generate(bool fixedFunction)
{
	MaterialPtr mat = prepareMaterial(mName);
	mat->setReceiveShadows(false);
		
	// test
	//mParent->setShaders(false);
	//mParent->setEnvMap(false);
	
	// only 1 technique
	Ogre::Technique* technique = mat->createTechnique();
	
	// single pass
	Ogre::Pass* pass = technique->createPass();
	
	pass->setAmbient( mProps->ambient.x, mProps->ambient.y, mProps->ambient.z );
	pass->setDiffuse( mProps->diffuse.x, mProps->diffuse.y, mProps->diffuse.z, mProps->diffuse.w );
	
	if (!mParent->getShaders() || fixedFunction)
	{
		pass->setSpecular(mProps->specular.x, mProps->specular.y, mProps->specular.z, 1.0 );
		pass->setShininess(mProps->specular.w);
	}
	else
	{
		// shader assumes matShininess in specular w component
		pass->setSpecular(mProps->specular.x, mProps->specular.y, mProps->specular.z, mProps->specular.w);
	}
	
	std::string diffuseMap = pickTexture(&mProps->diffuseMaps);
	std::string normalMap = pickTexture(&mProps->normalMaps);
	
	// test
	//pass->setCullingMode(CULL_NONE);
	//pass->setShadingMode(SO_PHONG);
	
	if (!mParent->getShaders() || fixedFunction)
	{
		pass->setShadingMode(SO_PHONG);
		
		// diffuse map
		Ogre::TextureUnitState* tu = pass->createTextureUnitState( diffuseMap );
		
		if (needEnvMap())
		{
			// env map
			tu = pass->createTextureUnitState();
			tu->setCubicTextureName( mProps->envMap, true );
			tu->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
			
			// blend with diffuse map using 'reflection amount' property
			tu->setColourOperationEx(LBX_BLEND_MANUAL, LBS_CURRENT, LBS_TEXTURE, 
									ColourValue::White, ColourValue::White, 1-mProps->reflAmount);
		}
	}
	else
	{
		// create shaders
		HighLevelGpuProgramPtr fragmentProg = createFragmentProgram();
		HighLevelGpuProgramPtr vertexProg = createVertexProgram();
		
		if (!fragmentProg->isSupported() || !vertexProg->isSupported())
		{
			LogO("[MaterialFactory] WARNING: shader for material '" + mName
				+ "' is not supported, falling back to fixed-function");
			generate(true);
			return;
		}
		
		pass->setFragmentProgram(fragmentProg->getName());
		pass->setVertexProgram(vertexProg->getName());
		
		// diffuse map
		Ogre::TextureUnitState* tu = pass->createTextureUnitState( diffuseMap );
		tu->setName("diffuseMap");
		
		// env map
		if (needEnvMap())
		{
			tu = pass->createTextureUnitState( mProps->envMap );
			tu->setName("envMap");
		}
		
		// normal map
		if (needNormalMap())
		{
			tu = pass->createTextureUnitState( normalMap );
			tu->setName("normalMap");
		}
		
		// shadow maps
		if (needShadows())
		{
			for (int i = 0; i < mParent->getNumShadowTex(); ++i)
			{
				tu = pass->createTextureUnitState();
				tu->setName("shadowMap" + toStr(i));
				tu->setContentType(TextureUnitState::CONTENT_SHADOW);
				tu->setTextureAddressingMode(TextureUnitState::TAM_BORDER);
				tu->setTextureBorderColour(ColourValue::White);
			}
		}
	}
}

//----------------------------------------------------------------------------------------

MaterialPtr MaterialDefinition::prepareMaterial(const std::string& name)
{
	MaterialPtr mat;
	if (MaterialManager::getSingleton().resourceExists(name))
	{
		mat = MaterialManager::getSingleton().getByName(name);
		mat->removeAllTechniques();
	}
	else
	{
		mat = MaterialManager::getSingleton().create(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	}
	return mat;
}

//----------------------------------------------------------------------------------------

inline bool MaterialDefinition::needShadows()
{
	return (mProps->receivesShadows && mParent->getShadows())
		|| (mProps->receivesDepthShadows && mParent->getShadowsDepth());
	//!todo shadow priority
}

inline bool MaterialDefinition::needNormalMap()
{
	return (mProps->normalMaps.size() > 0) && mParent->getNormalMap();
	//!todo normal map priority
}

inline bool MaterialDefinition::needEnvMap()
{
	return (mProps->envMap != "") && mParent->getEnvMap();
	//!todo env map priority
}

//----------------------------------------------------------------------------------------

std::string MaterialDefinition::pickTexture(textureMap* textures)
{
	if (textures->size() == 0) return "";
	
	// we assume the textures are sorted by size
	textureMap::iterator it;
	for (it = textures->begin(); it != textures->end(); ++it)
	{
		if ( it->first < mParent->getTexSize() ) continue;
		/* else */ break;
	}
	
	if (it == textures->end()) --it;
	
	return it->second;
}


//----------------------------------------------------------------------------------------

HighLevelGpuProgramPtr MaterialDefinition::createVertexProgram()
{
	HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
	std::string progName = getName() + "_VP";

	HighLevelGpuProgramPtr ret = mgr.getByName(progName);
	if (!ret.isNull())
		mgr.remove(progName);

	ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		"cg", GPT_VERTEX_PROGRAM);

	ret->setParameter("profiles", "vs_1_1 arbvp1");
	ret->setParameter("entry_point", "main_vp");

	StringUtil::StrStreamType sourceStr;
	generateVertexProgramSource(sourceStr);
	ret->setSource(sourceStr.str());
	ret->load();
	vertexProgramParams(ret);
	
	return ret;
}

//----------------------------------------------------------------------------------------

void MaterialDefinition::generateVertexProgramSource(Ogre::StringUtil::StrStreamType& outStream)
{
	//!todo more optizations (needWsNormal, needEyeVector, ... etc)
	outStream << 
		"void main_vp( "
		"	float2 texCoord 					: TEXCOORD0,"
		"	float4 position 							: POSITION," // obj space
		"	float3 normal			 			: NORMAL,"	// obj space
		"	uniform float4 eyePosition,					 "  // obj space
		"	out float4 oPosition			 			: POSITION,"
		"	out float2 oTexCoord	 			: TEXCOORD0,"
		"	out float3 oWsNormal  				: TEXCOORD1," // world space
		"	out	float4	oTangentToCubeSpace0	: TEXCOORD2,"
		"	out	float4	oTangentToCubeSpace1	: TEXCOORD3,"
		"	out	float4	oTangentToCubeSpace2	: TEXCOORD4,"
		"	uniform float4x4 wvpMat,"
		"	uniform float4x4 wMat,"
		"	uniform float4x4 wITMat"
		") \n"
		"{ \n"
		"	oPosition = mul(wvpMat, position); \n"
		"	oTexCoord = texCoord; \n"
		"	oWsNormal = mul( (float3x3) wITMat, normal ); \n"
		"	float3 eyeVector = mul( wMat, position ) - eyePosition; \n" // transform eye into view space
		"	oTangentToCubeSpace0.w = eyeVector.x; \n" // store eye vector
		"	oTangentToCubeSpace1.w = eyeVector.y; \n"
		"	oTangentToCubeSpace2.w = eyeVector.z; \n"
		"} \n";
}

//----------------------------------------------------------------------------------------

void MaterialDefinition::vertexProgramParams(HighLevelGpuProgramPtr program)
{
	GpuProgramParametersSharedPtr params = program->getDefaultParameters();
	params->setIgnoreMissingParams(true);
	params->setNamedAutoConstant("wMat", GpuProgramParameters::ACT_WORLD_MATRIX);
	params->setNamedAutoConstant("wvpMat", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
	params->setNamedAutoConstant("wITMat", GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX);
	params->setNamedAutoConstant("eyePosition", GpuProgramParameters::ACT_CAMERA_POSITION);
}

//----------------------------------------------------------------------------------------

HighLevelGpuProgramPtr MaterialDefinition::createFragmentProgram()
{
	HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
	std::string progName = getName() + "_FP";

	HighLevelGpuProgramPtr ret = mgr.getByName(progName);
	if (!ret.isNull())
		mgr.remove(progName);

	ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		"cg", GPT_FRAGMENT_PROGRAM);

	ret->setParameter("profiles", "ps_2_x arbfp1");
	ret->setParameter("entry_point", "main_fp");

	StringUtil::StrStreamType sourceStr;
	generateFragmentProgramSource(sourceStr);
	ret->setSource(sourceStr.str());
	ret->load();
	fragmentProgramParams(ret);
	
	return ret;
}

//----------------------------------------------------------------------------------------

void MaterialDefinition::generateFragmentProgramSource(Ogre::StringUtil::StrStreamType& outStream)
{
	unsigned int texI = 1; // texture unit counter
	outStream <<
		"void main_fp("
		"	in float2 texCoord : TEXCOORD0,"
		"	in float3 wsNormal : TEXCOORD1,"
		"	in float4 tangentToCubeSpace0 : TEXCOORD2,"
		"	in float4 tangentToCubeSpace1 : TEXCOORD3,"
		"	in float4 tangentToCubeSpace2 : TEXCOORD4,"
		
		"	uniform sampler2D diffuseMap : TEXUNIT0,"
		"	uniform samplerCUBE envMap : TEXUNIT1,"
		
	; if (needEnvMap()) outStream << 
		"	uniform float reflAmount,";
	outStream << 

		"	out float4 oColor : COLOR"
		") \n"
		"{ \n"
		
		"	float3 eyeVector; \n" // fetch view vector
		"	eyeVector.x = tangentToCubeSpace0.w; \n"
		"	eyeVector.y = tangentToCubeSpace1.w; \n"
		"	eyeVector.z = tangentToCubeSpace2.w; \n"
		"	eyeVector = normalize(eyeVector); \n"
		"	wsNormal = normalize(wsNormal); \n"
	
		"	float3 r; \n" // Calculate reflection vector within world (view) space.
		"	r = reflect( eyeVector, wsNormal ); \n"
		
	; if (needEnvMap()) outStream << 
		"	float4 envColor = texCUBE(envMap, r); \n"
	; outStream <<
		"	float4 diffuseColor = tex2D(diffuseMap, texCoord); \n"
		
	; if (needEnvMap()) outStream << 
		"	oColor = lerp(diffuseColor, envColor, reflAmount); \n"
	; else outStream <<
		"	oColor = diffuseColor; \n"
		
	; outStream << 
		"} \n";
}

//----------------------------------------------------------------------------------------

void MaterialDefinition::fragmentProgramParams(HighLevelGpuProgramPtr program)
{
	GpuProgramParametersSharedPtr params = program->getDefaultParameters();
	params->setIgnoreMissingParams(true);
	if (needEnvMap()) {
		params->setNamedConstant("reflAmount", mProps->reflAmount);
	}
}
