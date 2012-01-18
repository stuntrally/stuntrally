#include "pch.h"
#include "../../Defines.h"

#include "MaterialGenerator.h"
#include "MaterialDefinition.h"
#include "MaterialFactory.h"
#include "ShaderProperties.h"

#ifndef ROAD_EDITOR
	#include "../../OgreGame.h"
#else
	#include "../../../editor/OgreApp.h"
#endif


#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreTextureUnitState.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreGpuProgramParams.h>
#include <OgreRoot.h>
using namespace Ogre;


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
	// note: world position xz for fragment is stored in oTexCoord.w, oWsNormal.w
	mTexCoord_i=0;
	
	outStream << 
		"void main_vp( "
		"	float4 position 					: POSITION, \n";
	
	if (fpNeedWsNormal()) 
	{
		outStream <<"	float3 normal			 			: NORMAL, \n";
	}
	if (vpNeedTangent()) outStream <<
		"	float3 tangent						: TANGENT, \n";
	outStream << 
		"	float2 texCoord 					: TEXCOORD0, \n";
	
	if (fpNeedEyeVector()) outStream <<
		"	uniform float4 eyePosition,	 \n";
	outStream <<
		"	out float4 oPosition			 	: POSITION, \n"
		"	out float4 objectPos				: COLOR, \n" // running out of texcoords so putting this in COLOR since its unused.
		"	out float4 oTexCoord				: TEXCOORD"+toStr(mTexCoord_i++)+", \n";
		
	if (mShader->vertexColour) outStream <<
		"	float4 color 						: COLOR, \n"
		"	out float4 oVertexColour				: TEXCOORD"+toStr(mTexCoord_i++)+", \n";
		
	if (mShader->wind == 1)
	{
		outStream <<
		"	uniform float time, \n"
		"	uniform float frequency, \n"
		"	uniform float3 objSpaceCam, \n"
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

	if (fpNeedWsNormal()) 
	{
		if(UsePerPixelNormals())
		{
			outStream <<"	out float4 oNormal  				: COLOR1, \n";
		}
		else
		{
			if(MRTSupported())
			{
				outStream <<"	out float4 oWsNormal  				: COLOR1, \n";
			}
			else
			{
				outStream <<"	out float4 oWsNormal  				: TEXCOORD"+toStr(mTexCoord_i++)+", \n";	
			}
		}
	}
	if (needNormalMap()) outStream <<
		"	uniform float bumpScale, \n";
		
	if (fpNeedTangentToCube()) 
	{
		 outStream <<"	out	float4	oTangentToCubeSpace0	: TEXCOORD"+ toStr( mTexCoord_i++ ) +", \n"; // tangent to cube (world) space
		 outStream <<"	out	float4	oTangentToCubeSpace1	: TEXCOORD"+ toStr( mTexCoord_i++ ) +", \n";
		 outStream <<"	out	float4	oTangentToCubeSpace2	: TEXCOORD"+ toStr( mTexCoord_i++ ) +", \n";
	}
	if(MRTSupported())
	{
		if(!UsePerPixelNormals())
		{
			//view space normal 
			outStream << "	out	float4	oViewNormal	: TEXCOORD"+ toStr( mTexCoord_i++ ) +", \n";
		}
	}

	// fog
	outStream <<
		"	uniform float enableFog, \n"
		"	uniform float4 fogParams, \n";
		
	vpShadowingParams(outStream);

	if (vpNeedWITMat()) outStream <<
		"	uniform float4x4 wITMat, \n";
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
		"	float dist = distance(objSpaceCam.xz, position.xz); \n"
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
	if (vpNeedWMat()) outStream <<
	"	float4 worldPosition = mul(wMat, position); \n";
	
	if (fpNeedEyeVector()) outStream <<
		"	float3 eyeVector = worldPosition.xyz - eyePosition; \n" // transform eye into view space
		"	oTangentToCubeSpace0.xyzw = eyeVector.xxxx; \n"
		"	oTangentToCubeSpace1.xyzw = eyeVector.yyyy; \n"
		"	oTangentToCubeSpace2.xyzw = eyeVector.zzzz; \n";

	if (needNormalMap()) outStream <<
	"	float3 binormal = cross(tangent, normal); \n"	// calculate binormal
	"	float3x3 tbn = float3x3( tangent * bumpScale, \n"			// build TBN
	"										binormal * bumpScale, \n"
	"										normal ); \n"
	"	float3x3 tbnInv = transpose(tbn); \n" // transpose TBN to get inverse rotation to obj space
	"	float3x3 tmp = mul( (float3x3) wITMat, tbnInv ); \n" // concatenate with the upper-left 3x3 of inverse transpose world matrix 
	"	oTangentToCubeSpace0.xyz = tmp[0]; \n" // store in tangentToCubeSpace to pass this on to fragment shader
	"	oTangentToCubeSpace1.xyz = tmp[1]; \n"
	"	oTangentToCubeSpace2.xyz = tmp[2]; \n";
		
	if (needShadows())
	{
		for (int i=0; i<mParent->getNumShadowTex(); ++i)
		{
			outStream << "oLightPosition"+toStr(i)+" = mul(texWorldViewProjMatrix"+toStr(i)+", position); \n";
		}
	}
	
	std::string texCoordW = "1";
	if (needTerrainLightMap()) texCoordW = "worldPosition.z";
	
	std::string texCoordZ = "1";
	if (needShadows()) texCoordZ = "oPosition.z";
	
	outStream <<
	"	oTexCoord = float4(texCoord.x, texCoord.y, "+texCoordZ+", "+texCoordW+"); \n";
		
	outStream <<
	"	objectPos = position; \n";
	outStream <<
	"	objectPos.w = enableFog * saturate(fogParams.x * (oPosition.z - fogParams.y) * fogParams.w); \n"; // save fog amount in objectPos.w
	if (fpNeedWsNormal())
	{
		if(UsePerPixelNormals())
		{
			outStream <<	"	oNormal =  float4(normal,0) ; \n";
		}
		else
		{
			std::string wsNormalW = "1";
			if (needTerrainLightMap()) wsNormalW = "worldPosition.x";
			outStream <<
			"	oWsNormal = float4(mul( (float3x3) wITMat, normal ), "+wsNormalW+"); \n";
		}

	}

	if(MRTSupported())
	{
		if(!UsePerPixelNormals())
		{
			//view space normal 			
			outStream << " oViewNormal = mul(wvMat, float4(normal, 0)); \n";
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
	if (fpNeedWsNormal())
		params->setNamedAutoConstant("wITMat", GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX);
	if (fpNeedEyeVector())
		params->setNamedAutoConstant("eyePosition", GpuProgramParameters::ACT_CAMERA_POSITION);
	
	params->setNamedAutoConstant("fogParams", GpuProgramParameters::ACT_FOG_PARAMS);
		
	if (mShader->wind == 1)
		params->setNamedAutoConstant("objSpaceCam", GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE);
	
	else if (mShader->wind == 2)
		params->setNamedConstant("enableWind", Real(1.0));
		
	individualVertexProgramParams(params);
}

//----------------------------------------------------------------------------------------

void MaterialGenerator::individualVertexProgramParams(GpuProgramParametersSharedPtr params)
{
	#ifndef _DEBUG
	params->setIgnoreMissingParams(true);
	#endif
	
	if (needNormalMap())
		params->setNamedConstant("bumpScale", mDef->mProps->bumpScale);
	
	if (needShadows())
	for (int i=0; i<mParent->getNumShadowTex(); ++i)
	{
		params->setNamedAutoConstant("texWorldViewProjMatrix"+toStr(i), GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX, i);
	}
	
	params->setNamedConstant("enableFog", mDef->mProps->fog ? Real(1.0) : Real(0.0));
}

//----------------------------------------------------------------------------------------
