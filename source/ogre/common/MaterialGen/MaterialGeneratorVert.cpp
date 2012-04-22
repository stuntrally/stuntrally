#include "pch.h"
#include "../Defines.h"

#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreTextureUnitState.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreGpuProgramParams.h>
#include <OgreRoot.h>
using namespace Ogre;

#include "MaterialGenerator.h"
#include "MaterialDefinition.h"
#include "MaterialFactory.h"
#include "ShaderProperties.h"

#ifndef ROAD_EDITOR
	#include "../../OgreGame.h"
#else
	#include "../../../editor/OgreApp.h"
#endif



//  Vertex program
//----------------------------------------------------------------------------------------

HighLevelGpuProgramPtr MaterialGenerator::createVertexProgram()
{
	HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
	std::string progName = mDef->getName() + "_VP";

	HighLevelGpuProgramPtr ret = mgr.getByName(progName);
	if (!ret.isNull())
		mgr.remove(progName);

	ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		"cg", GPT_VERTEX_PROGRAM);

	if(MRTSupported())
	{
		ret->setParameter("profiles", "vs_4_0 vs_3_0 vp40");
	}
	else
	{
		ret->setParameter("profiles", "vs_4_0 vs_2_x arbvp1");
	}
	ret->setParameter("entry_point", "main_vp");

	StringUtil::StrStreamType sourceStr;
	generateVertexProgramSource(sourceStr);
	ret->setSource(sourceStr.str());
	ret->load();
	vertexProgramParams(ret);
	
	return ret;
}

//----------------------------------------------------------------------------------------

void MaterialGenerator::vpShadowingParams(Ogre::StringUtil::StrStreamType& outStream)
{
	
	
	if (needShadows()) {
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
		{
			outStream << "out float4 oLightPosition"+toStr(i)+" : TEXCOORD"+toStr(mTexCoord_i++)+", \n";
		}
		outStream << "\n";
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
		{
			outStream << "uniform float4x4 texWorldViewProjMatrix"+toStr(i)+", \n";
		}
		outStream << "\n";
	}
}

//----------------------------------------------------------------------------------------

void MaterialGenerator::generateVertexProgramSource(Ogre::StringUtil::StrStreamType& outStream)
{
	mTexCoord_i=0;
	
	outStream << 
		"void main_vp( "
		"	float4 position 					: POSITION, \n";
	
	if (vpNeedNormal()) outStream <<
		"	float3 normal			 			: NORMAL, \n";
	
	if (vpNeedTangent()) outStream <<
		"	float3 tangent						: TANGENT, \n";
	outStream << 
	"	float2 texCoord 					: TEXCOORD0, \n";
	
	if (fpNeedEyeVector() || mShader->wind == 1) outStream <<
		"	uniform float4 eyePosition,	 \n";
	outStream <<
	"	out float4 oPosition			 	: POSITION, \n";
		
	if (fpNeedPos()) outStream <<
		"	out float4 oObjPosition				: COLOR, \n";
		
	outStream <<
	"	out float4 oTexCoord				: TEXCOORD"+toStr(mTexCoord_i++)+", \n";
		
	if (mShader->vertexColour) outStream <<
		"	float4 color 						: COLOR, \n"
		"	out float4 oVertexColour				: TEXCOORD"+toStr(mTexCoord_i++)+", \n";
		
	if (mShader->wind == 1)
	{
		outStream <<
		"	uniform float time, \n"
		"	uniform float frequency, \n"
		//"	uniform float3 objSpaceCam, \n"
		"	uniform float fadeRange, \n"
		"	uniform float4 direction, \n"
		"	out float alphaFade : TEXCOORD"+toStr(mTexCoord_i++)+", \n";
	}
	else if (mShader->wind == 2)
	{
		outStream <<
		"	uniform float time, \n"
		"	uniform float enableWind, \n"
		"	float4 windParams 	: TEXCOORD1, \n"
		"	float4 originPos 	: TEXCOORD2, \n";
	}

	if (fpNeedNormal()) 
	{
		if(UsePerPixelNormals())
		{
			outStream <<"	out float4 oNormal  				: COLOR1, \n";
		}
		else
		{
			outStream <<"	out float4 oNormal  				: TEXCOORD"+toStr(mTexCoord_i++)+", \n";	
		}
	}
	
	if (fpNeedEyeVector()) outStream <<
		"	out float4 oEyeVector : TEXCOORD"+toStr(mTexCoord_i++)+", \n";
	
	if (needNormalMap()) outStream <<
		"	out float4 oTangent : TEXCOORD"+toStr(mTexCoord_i++)+", \n";
	
	if(MRTSupported())
	{
		if(!UsePerPixelNormals())
		{
			// view space normal 
			outStream << "	out	float4	oViewNormal	: TEXCOORD"+ toStr( mTexCoord_i++ ) +", \n";
			//outStream << "	out	float4	oViewPosition	: TEXCOORD"+ toStr( mTexCoord_i++ ) +", \n";
		}
	}

	// fog
	outStream <<
        "	uniform float enableFog, \n"
        "	uniform float4 fogParams, \n"
		"	out float fogAmount : FOG, \n";
		
	vpShadowingParams(outStream);

	if (vpNeedWvMat()) outStream <<
		"	uniform float4x4 wvMat, \n";
	if (vpNeedWMat()) outStream <<
		"	uniform float4x4 wMat, \n";
	outStream << 
	"	uniform float4x4 wvpMat \n"
	") \n"
	"{ \n";
	
	if (mShader->vertexColour) outStream <<
		"	oVertexColour = color; \n";
		
	if (vpCalcWPos()) outStream <<
		"	float4 worldPosition = mul(wMat, position); \n";
	
	if (mShader->wind == 1)
	{
		// wave
		outStream <<
		"	float oldposx = position.x; \n"
		"	if (texCoord.y == 0.0f) \n"
		"	{ \n"
		"		float offset = sin(time + oldposx * frequency); \n"
		"		position += direction * offset; \n"
		"	} \n"
		
		// fade
		"	float dist = distance(eyePosition.xz, worldPosition.xz); \n"
		"	alphaFade = (2.0f - (2.0f * dist / (fadeRange))); \n";
	}
	else if (mShader->wind == 2)
	{
		outStream <<
		"	float radiusCoeff = windParams.x; \n"
		"	float heightCoeff = windParams.y; \n"
		"	float factorX = windParams.z; \n"
		"	float factorY = windParams.w; \n";
		/* 
		2 different methods are used to for the sin calculation :
		- the first one gives a better effect but at the cost of a few fps because of the 2 sines
		- the second one uses less ressources but is a bit less realistic

			a sin approximation could be use to optimize performances
		*/
		
		// we can safely make permutations depending on "shaderQuality" since it does not change per material.
		//! might need to revisit this assumption when using more excessive caching (shaders that persist after settings change)
		if (mParent->getShaderQuality() > 0.6) outStream <<
			"	position.y += enableWind * sin(time + originPos.z + position.y + position.x) * radiusCoeff * radiusCoeff * factorY; \n"
			"	position.x += enableWind * sin(time + originPos.z ) * heightCoeff * heightCoeff * factorX ; \n";
		else outStream <<
			"	float sinval = enableWind * sin(time + originPos.z ); \n"
			"	position.y += sinval * radiusCoeff * radiusCoeff * factorY; \n"
			"	position.x += sinval * heightCoeff * heightCoeff * factorX ; \n";
	}
	
	outStream <<
	"	oPosition = mul(wvpMat, position); \n";
		
	if (fpNeedPos()) outStream <<
		"	oObjPosition = position; \n";
		
	if (fpNeedEyeVector()) outStream <<
		"	oEyeVector.xyz = worldPosition.xyz - eyePosition.xyz; \n";

	if (needNormalMap()) outStream <<
		"	oTangent.xyz = tangent.xyz; \n";
		
	if (needShadows())
	{
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
		{
			outStream << "oLightPosition"+toStr(i)+" = mul(texWorldViewProjMatrix"+toStr(i)+", position); \n";
		}
	}
	
	std::string texCoordZ = "1";
	if (needShadows() || MRTSupported()) texCoordZ = "oPosition.z";
	
	std::string texCoordW = "1";
	if (needTerrainLightMap()) texCoordW = "worldPosition.x";
	
	outStream <<
	"	oTexCoord = float4(texCoord.x, texCoord.y, "+texCoordZ+", "+texCoordW+"); \n";
		
	// fog amount
	outStream <<
	"	fogAmount = enableFog * saturate(fogParams.x * (oPosition.z - fogParams.y) * fogParams.w); \n";

	if (fpNeedNormal())
	{
		if(UsePerPixelNormals())
		{
			outStream <<	"	oNormal =  float4(normal,0) ; \n";
		}
		else
		{
			std::string normalW = "1";
			if (needTerrainLightMap()) normalW = "worldPosition.z";
			outStream <<
			"	oNormal = float4(normal.xyz, "+normalW+"); \n";
		}

	}

	if(MRTSupported())
	{
		if(!UsePerPixelNormals())
		{
			//view space normal 			
			outStream <<
			"	oViewNormal = mul(wvMat, float4(normal, 0)); \n"
			"	float3 viewPosition = mul(wvMat, position).xyz; \n";
			
			if (fpNeedNormal() && (!(needEnvMap() || needNormalMap() || fpNeedLighting()))) outStream <<
				"	oNormal.z = viewPosition.x; \n";
			if (!mShader->vertexColour) outStream <<
				"	oEyeVector.w = viewPosition.x; \n";
				
			outStream <<
			"	oObjPosition.w = viewPosition.y; \n"
			"	oViewNormal.w = viewPosition.z; \n";
		}
	}
	outStream <<
	"} \n";
}

//----------------------------------------------------------------------------------------

void MaterialGenerator::vertexProgramParams(HighLevelGpuProgramPtr program)
{
	GpuProgramParametersSharedPtr params = program->getDefaultParameters();
	
	#ifndef _DEBUG
	params->setIgnoreMissingParams(true);
	#endif
		
	if (vpNeedWMat())
		params->setNamedAutoConstant("wMat", GpuProgramParameters::ACT_WORLD_MATRIX);
	params->setNamedAutoConstant("wvpMat", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
	if (vpNeedWvMat())
		params->setNamedAutoConstant("wvMat", GpuProgramParameters::ACT_WORLDVIEW_MATRIX);
	if (fpNeedEyeVector() || mShader->wind == 1)
		params->setNamedAutoConstant("eyePosition", GpuProgramParameters::ACT_CAMERA_POSITION);
	
	params->setNamedAutoConstant("fogParams", GpuProgramParameters::ACT_FOG_PARAMS);
			
	if (mShader->wind == 2)
		params->setNamedConstant("enableWind", Real(1.0));
		
	individualVertexProgramParams(params);
}

//----------------------------------------------------------------------------------------

void MaterialGenerator::individualVertexProgramParams(GpuProgramParametersSharedPtr params)
{
	#ifndef _DEBUG
	params->setIgnoreMissingParams(true);
	#endif
	
	if (needShadows())
	for (int i=0; i<mParent->getNumShadowTex(); ++i)
	{
		params->setNamedAutoConstant("texWorldViewProjMatrix"+toStr(i), GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX, i);
	}
	
	params->setNamedConstant("enableFog", mDef->mProps->fog ? Real(1.0) : Real(0.0));
}

//----------------------------------------------------------------------------------------
