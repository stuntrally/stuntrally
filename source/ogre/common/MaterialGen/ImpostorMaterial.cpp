#include "pch.h"
#include "../../Defines.h"

#include "ImpostorMaterial.h"
#include "MaterialDefinition.h"
#include "MaterialFactory.h"

/*#ifndef ROAD_EDITOR
	#include "../OgreGame.h"
#else
	#include "../../editor/OgreApp.h"
#endif*/

#include <OgreMaterial.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreMaterialManager.h>
using namespace Ogre;

ImpostorMaterialGenerator::ImpostorMaterialGenerator()
{
	mName = "Impostor";
}

//----------------------------------------------------------------------------------------

void ImpostorMaterialGenerator::generate(bool fixedFunction)
{
	//!
	// note that we only create the shaders (Sprite_vp, Sprite_fp) here. 
	// the rest (material creation, shader parameters, ...) is done by paged-geom for convenience.
	
	HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
	
	// ---------------- VERTEX -----------------------------------------------
	
	// ensure shader does not exist already.
	HighLevelGpuProgramPtr vshader = mgr.getByName("Sprite_vp");
	if (vshader.isNull())
	{
		vshader = mgr.createProgram("Sprite_vp", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
			"cg", GPT_VERTEX_PROGRAM);

		if(MRTSupported())
		{
			vshader->setParameter("profiles", "vs_4_0 vs_3_0 vp40");
		}
		else
		{
			vshader->setParameter("profiles", "vs_4_0 vs_2_x arbvp1");
		}
		vshader->setParameter("entry_point", "Sprite_vp");
		
		StringUtil::StrStreamType outStream;
		
		outStream <<
		"void Sprite_vp(	\n"
		"	float4 position : POSITION,	\n"
		"	float3 normal   : NORMAL,	\n"
		"	float4 color	: COLOR,	\n"
		"	float2 uv       : TEXCOORD0,	\n"
		"	out float4 oPosition : POSITION,	\n"
		"	out float2 oUv       : TEXCOORD0,	\n"
		"	out float4 oColor    : COLOR, \n"
		"	out float oFog       : FOG,	\n"
		"	uniform float4 fogParams, \n"
		"	uniform float4x4 worldViewProj,	\n"
		"	uniform float    uScroll, \n"
		"	uniform float    vScroll, \n"
		"	uniform float4   preRotatedQuad[4] )	\n"
		"{	\n"
		//Face the camera
		"	float4 vCenter = float4( position.x, position.y, position.z, 1.0f );	\n"
		"	float4 vScale = float4( normal.x, normal.y, normal.x, 1.0f );	\n"
		"	oPosition = mul( worldViewProj, vCenter + (preRotatedQuad[normal.z] * vScale) );  \n"

		//Color
		"	oColor = color;   \n"

		//UV Scroll
		"	oUv = uv;	\n"
		"	oUv.x += uScroll; \n"
		"	oUv.y += vScroll; \n"

		//Fog
		"	oFog = saturate(fogParams.x * (oPosition.z - fogParams.y) * fogParams.w); \n"
		"}";
		
		vshader->setSource(outStream.str());
		vshader->load();
		
		vshader->getDefaultParameters()->setNamedAutoConstant("fogParams", GpuProgramParameters::ACT_FOG_PARAMS);
	}
	
	
	// ------------- FRAGMENT -----------------------------------------------
	
	// ensure shader does not exist already.
	HighLevelGpuProgramPtr fshader = mgr.getByName("Sprite_fp");
	if (fshader.isNull())
	{
		fshader = mgr.createProgram("Sprite_fp", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
			"cg", GPT_FRAGMENT_PROGRAM);
			
		if(MRTSupported())
		{
			fshader->setParameter("profiles", "ps_4_0 ps_3_0 fp40");
		}
		else
		{
			fshader->setParameter("profiles", "ps_4_0 ps_2_x arbfp1");	
		}
		fshader->setParameter("entry_point", "Sprite_fp");
		
		// simple pixel shader that only reads texture and fog
		StringUtil::StrStreamType outStream;
		outStream <<
		"void Sprite_fp( 	\n"
		"	uniform sampler2D texture, \n"
		"	float4 iPosition : POSITION,	\n"
		"	float2 texCoord : TEXCOORD0, \n"
		"	float4 iColor : COLOR,	\n"
		"	float fog : FOG, \n"
		"	uniform float4 fogColor, \n"
		"	out float4 oColor : COLOR \n"
		") \n"
		"{ \n"
		"	float4 texColor = tex2D(texture, texCoord); \n"
		"	oColor = float4(lerp(texColor.xyz, fogColor.xyz, fog), texColor.a); \n"
		"	clip( oColor.a > 0.0f ? 1:-1); \n"
		"} \n";
		
		fshader->setSource(outStream.str());
		fshader->load();
		
		fshader->getDefaultParameters()->setNamedAutoConstant("fogColor", GpuProgramParameters::ACT_FOG_COLOUR);
	}
}
