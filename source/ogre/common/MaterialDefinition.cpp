#include "pch.h"
#include "../Defines.h"

#include "MaterialDefinition.h"
#include "MaterialFactory.h"

#ifndef ROAD_EDITOR
	#include "../OgreGame.h"
#else
	#include "../../editor/OgreApp.h"
#endif

#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreTextureUnitState.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreGpuProgramParams.h>
#include <OgreEntity.h>
#include <OgreSubEntity.h>
#include <OgreSceneManager.h>
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

MaterialDefinition::MaterialDefinition(MaterialFactory* parent, MaterialProperties* props) :
	mFirstRun(1)
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
	std::vector<SubEntity*> subEnts;

	/*if (!mFirstRun)
	{
		SceneManager::MovableObjectIterator iterator = mParent->pApp->sceneMgr()->getMovableObjectIterator("Entity");
		while (iterator.hasMoreElements())
		{
			Entity* e = static_cast<Entity*>(iterator.getNext());
			for (int i=0; i<e->getNumSubEntities(); ++i)
			{
				Ogre::SubEntity* subent = e->getSubEntity(i);
				if (subent->getMaterialName() == mName)
				{
					LogO("Found entity that has our material");
					subEnts.push_back(subent);
					subent->setMaterialName("BaseWhite");
				}
			}
		}
	}*/
	
	MaterialPtr mat = prepareMaterial(mName);
	//mat->setReceiveShadows(false);
	
	LogO("needEnvMap: " + toStr(needEnvMap()));
	
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
			tu->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
			
			// blend with diffuse map using 'reflection amount' property
			tu->setColourOperationEx(LBX_BLEND_MANUAL, LBS_CURRENT, LBS_TEXTURE, 
									ColourValue::White, ColourValue::White, 1-mProps->reflAmount);
		}
	}
	else
	{
		// create shaders
		HighLevelGpuProgramPtr fragmentProg, vertexProg;
		try
		{
			vertexProg = createVertexProgram();
			fragmentProg = createFragmentProgram();
		}
		catch (Ogre::Exception& e) {
			LogO(e.getFullDescription());
		}
		
		/*if (fragmentProg.isNull() || vertexProg.isNull() || 
			!fragmentProg->isSupported() || !vertexProg->isSupported())
		{
			LogO("[MaterialFactory] WARNING: shader for material '" + mName
				+ "' is not supported, falling back to fixed-function");
			generate(true);
			return;
		}*/
		
		pass->setVertexProgram(vertexProg->getName());
		pass->setFragmentProgram(fragmentProg->getName());
		
		// diffuse map
		Ogre::TextureUnitState* tu = pass->createTextureUnitState( diffuseMap );
		tu->setName("diffuseMap");
		
		// env map
		if (needEnvMap())
		{
			tu = pass->createTextureUnitState( mProps->envMap );
			tu->setName("envMap");
			tu->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
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
	
	// assign material again
	/*if (!mFirstRun)
	{
		for (std::vector<SubEntity*>::iterator it=subEnts.begin();
			it!=subEnts.end(); ++it)
		{
			LogO("restoring material...");
			(*it)->setMaterialName(mName);
		}
	}*/
	mFirstRun = false;
}

//----------------------------------------------------------------------------------------

MaterialPtr MaterialDefinition::prepareMaterial(const std::string& name)
{
	//!todo
	/// to overcome the "material won't update" bug:
	/// - loop over all entities and their sub entities and check which of them have our material
	/// - unassign material
	/// - after we have generated, assign it again
	/// (its best to not do this on first run)
	
	MaterialPtr mat;
	if (MaterialManager::getSingleton().resourceExists(name))
	{
		mat = MaterialManager::getSingleton().getByName(name);
		//MaterialManager::getSingleton().remove(name);
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

inline bool MaterialDefinition::fpNeedWsNormal()
{
	return needEnvMap();
	//!todo
}

inline bool MaterialDefinition::fpNeedEyeVector()
{
	return needEnvMap();
	//!todo
}

std::string MaterialDefinition::getChannel(unsigned int n)
{
	if (n == 0) 		return "x";
	else if (n == 1) 	return "y";
	else if (n == 2)	return "z";
	else 				return "w";
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
	LogO("Vertex program source:\n");
	LogO(sourceStr.str());
	ret->setSource(sourceStr.str());
	ret->load();
	vertexProgramParams(ret);
	
	return ret;
}

//----------------------------------------------------------------------------------------

void MaterialDefinition::generateVertexProgramSource(Ogre::StringUtil::StrStreamType& outStream)
{
	//!todo more optizations
	//!todo variables for tex unit num of different maps (e.g. TEXUNIT_START_SHADOW, TEXUNIT_ENVMAP..)
	outStream << 
		"void main_vp( "
		"	float2 texCoord 					: TEXCOORD0,"
		"	float4 position 					: POSITION,"
		"	float3 normal			 			: NORMAL,"
		"	uniform float4 eyePosition,					 "
		"	out float4 oPosition			 	: POSITION, \n"
	; if (!needShadows()) outStream <<
		"	out float2 oTexCoord	 			: TEXCOORD0,"
	; else outStream <<
		"	out float3 oTexCoord				: TEXCOORD0,"
		
	; if (fpNeedWsNormal()) outStream << 
		"	out float3 oWsNormal  				: TEXCOORD1,"
	; outStream <<
		"	out	float4	oTangentToCubeSpace0	: TEXCOORD2,"
		"	out	float4	oTangentToCubeSpace1	: TEXCOORD3,"
		"	out	float4	oTangentToCubeSpace2	: TEXCOORD4, \n"	
	;
	
	if (needShadows()) {
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
		{
			outStream << "out float4 oLightPosition"+toStr(i)+" : TEXCOORD"+toStr(i+5)+",";
		}
		outStream << "\n";
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
		{
			outStream << "uniform float4x4 texWorldViewProjMatrix"+toStr(i)+", ";
		}
		outStream << "\n";
	}

		if (fpNeedWsNormal())
			outStream <<
		"	uniform float4x4 wITMat,"
		; outStream << 
		"	uniform float4x4 wvpMat,"
		"	uniform float4x4 wMat"
		") \n"
		"{ \n"
		"	oPosition = mul(wvpMat, position); \n"
		
	; if (needShadows()) {
		 outStream <<
		"	oTexCoord.xy = texCoord; \n"
		"	oTexCoord.z = oPosition.z; \n";
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
		{
			outStream << "oLightPosition"+toStr(i)+" = mul(texWorldViewProjMatrix"+toStr(i)+", position); \n";
		}
	}
	
	else outStream <<
		"	oTexCoord = texCoord; \n"
		
	; if (fpNeedWsNormal()) outStream <<
		"	oWsNormal = mul( (float3x3) wITMat, normal ); \n"
	; outStream <<
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
	//params->setIgnoreMissingParams(true);
	params->setNamedAutoConstant("wMat", GpuProgramParameters::ACT_WORLD_MATRIX);
	params->setNamedAutoConstant("wvpMat", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
	if (fpNeedWsNormal())
		params->setNamedAutoConstant("wITMat", GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX);
	params->setNamedAutoConstant("eyePosition", GpuProgramParameters::ACT_CAMERA_POSITION);
	
	if (needShadows())
	for (int i=0; i<mParent->getNumShadowTex(); ++i)
	{
		params->setNamedAutoConstant("texWorldViewProjMatrix"+toStr(i), GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX, i);
	}
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
	LogO("Fragment program source:\n");
	LogO(sourceStr.str());
	ret->setSource(sourceStr.str());
	ret->load();
	fragmentProgramParams(ret);
	
	return ret;
}

//----------------------------------------------------------------------------------------

void MaterialDefinition::generateFragmentProgramSource(Ogre::StringUtil::StrStreamType& outStream)
{
	if (needShadows())
	{
		/// shadow helper functions
		// 2x2 pcf
		outStream <<
		"float shadowPCF(sampler2D shadowMap, float4 shadowMapPos, float2 offset)"
		"{ \n"
		"	shadowMapPos = shadowMapPos / shadowMapPos.w; \n"
		"	float2 uv = shadowMapPos.xy; \n"
		"	float3 o = float3(offset, -offset.x) * 0.3f; \n"

		"	float c =	(shadowMapPos.z <= tex2D(shadowMap, uv.xy - o.xy).r) ? 1 : 0; \n"
		"	c +=		(shadowMapPos.z <= tex2D(shadowMap, uv.xy + o.xy).r) ? 1 : 0; \n"
		"	c +=		(shadowMapPos.z <= tex2D(shadowMap, uv.xy + o.zy).r) ? 1 : 0; \n"
		"	c +=		(shadowMapPos.z <= tex2D(shadowMap, uv.xy - o.zy).r) ? 1 : 0; \n"
		"	return c / 4;  \n"
		"} \n"
		;
		
		// pssm
		outStream <<
		"float calcPSSMShadow(";
		
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
			outStream << "sampler2D shadowMap"+toStr(i)+", ";
		outStream << "\n";
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
			outStream << "float4 lsPos"+toStr(i)+", ";
		outStream << "\n";
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
			outStream << "float4 invShadowMapSize"+toStr(i)+", ";
		outStream << "\n";
		
		outStream <<
		"	float4 pssmSplitPoints, float camDepth"
		") \n"
		"{ \n"
		"	float shadow; \n"
		
		; for (int i=0; i<mParent->getNumShadowTex(); ++i)
		{
			if (i==0)
				outStream << "if (camDepth <= pssmSplitPoints.y) \n";
			else if (i < mParent->getNumShadowTex()-1)
				outStream << "else if (camDepth <= pssmSplitPoints."+getChannel(i+1)+") \n";
			else
				outStream << "else \n";
				
			outStream <<
			"{ \n"
			"	shadow = shadowPCF(shadowMap"+toStr(i)+", lsPos"+toStr(i)+", invShadowMapSize"+toStr(i)+".xy); \n"
			"} \n";
		}
		
		outStream <<
		"	return shadow; \n"
		"} \n"
		;
	}
	
	outStream <<
		"void main_fp("
	; if (!needShadows()) outStream <<
		"	in float2 texCoord : TEXCOORD0,"
	; else outStream <<
		"	in float3 texCoord : TEXCOORD0,"
	; if (fpNeedWsNormal()) outStream <<
		"	in float3 wsNormal : TEXCOORD1,"
	; outStream <<
		"	in float4 tangentToCubeSpace0 : TEXCOORD2,"
		"	in float4 tangentToCubeSpace1 : TEXCOORD3,"
		"	in float4 tangentToCubeSpace2 : TEXCOORD4, \n"
		
		"	uniform sampler2D diffuseMap : TEXUNIT0, \n"
		
	; if (needEnvMap()) outStream << 
		"	uniform samplerCUBE envMap : TEXUNIT1,"
		"	uniform float reflAmount, \n";
	
	if (needShadows())
	{
		int shadowtexStart = 2;
		if (needNormalMap()) ++shadowtexStart;
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
		{
			outStream <<
		"	uniform sampler2D shadowMap"+toStr(i)+" : TEXUNIT"+toStr(shadowtexStart+i)+", ";
		}
		outStream << "\n";
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
			outStream << "in float4 lightPosition"+toStr(i)+" : TEXCOORD"+toStr(i+5)+", ";
		outStream << "\n";
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
			outStream << "uniform float4 invShadowMapSize"+toStr(i)+", ";
		outStream << "\n";
		outStream << 
		"	uniform float4 pssmSplitPoints,";
	}
	
	
	outStream << 

		"	out float4 oColor : COLOR"
		") \n"
		"{ \n"
		
	// calc shadowing
	; if (needShadows()) {
		outStream <<
		"	float shadowing;"
		"	shadowing = calcPSSMShadow("
		;
	for (int i=0; i<mParent->getNumShadowTex(); ++i)
		outStream << "shadowMap"+toStr(i)+", ";
	for (int i=0; i<mParent->getNumShadowTex(); ++i)
		outStream << "lightPosition"+toStr(i)+", ";
	for (int i=0; i<mParent->getNumShadowTex(); ++i)
		outStream << "invShadowMapSize"+toStr(i)+", ";
		
		outStream <<
		"pssmSplitPoints, texCoord.z); \n";
		
	}
		
	outStream << 
		"	float3 eyeVector; \n" // fetch view vector
		"	eyeVector.x = tangentToCubeSpace0.w; \n"
		"	eyeVector.y = tangentToCubeSpace1.w; \n"
		"	eyeVector.z = tangentToCubeSpace2.w; \n"
		"	eyeVector = normalize(eyeVector); \n"
	; if (fpNeedWsNormal()) outStream <<
		"	wsNormal = normalize(wsNormal); \n"
		
	; if (needEnvMap()) outStream << 
		"	float3 r; \n" // Calculate reflection vector within world (view) space.
		"	r = reflect( eyeVector, wsNormal ); \n"
		"	float4 envColor = texCUBE(envMap, r); \n"
	; outStream <<
		"	float4 diffuseColor = tex2D(diffuseMap, texCoord); \n"
	
	//!todo
	; if (needEnvMap()) outStream << 
		"	float4 color1 = lerp(diffuseColor, envColor, reflAmount); \n"
		
	; else outStream <<
		"	float4 color1 = diffuseColor; \n"
	
	; if (needShadows()) outStream <<
		"	oColor = color1 * shadowing; \n" // test
	
	; else outStream <<
		"	oColor = color1; \n"
		
	; outStream << 
		"} \n";
}

//----------------------------------------------------------------------------------------

void MaterialDefinition::fragmentProgramParams(HighLevelGpuProgramPtr program)
{
	GpuProgramParametersSharedPtr params = program->getDefaultParameters();
	//params->setIgnoreMissingParams(true);
	if (needEnvMap()) {
		params->setNamedConstant("reflAmount", mProps->reflAmount);
	}
	if (needShadows()) {
		params->setNamedConstant("pssmSplitPoints", mParent->pApp->splitPoints);
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
			params->setNamedAutoConstant("invShadowMapSize"+toStr(i), GpuProgramParameters::ACT_INVERSE_TEXTURE_SIZE, i);
	}
}
