#include "pch.h"
#include "../Defines.h"

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
		"	oFog = oPosition.z; \n"
		"}";
		
		vshader->setSource(outStream.str());
		vshader->load();
	}
	
	
	// ------------- FRAGMENT -----------------------------------------------
	
	// ensure shader does not exist already.
	/*HighLevelGpuProgramPtr fshader = mgr.getByName("Sprite_fp");
	if (fshader.isNull())
	{
		fshader = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
			"cg", GPT_FRAGMENT_PROGRAM);
			
		if(MRTSupported())
		{
			ret->setParameter("profiles", "ps_4_0 ps_3_0 fp40");
		}
		else
		{
			ret->setParameter("profiles", "ps_4_0 ps_2_x arbfp1");	
		}
		ret->setParameter("entry_point", "Sprite_fp");
		
		// simple pixel shader that only reads texture and fog
		StringUtil::StrStreamType& outStream;
		outStream <<
		"void Sprite_fp( 	\n"
		"	uniform sampler2D texture, \n"
		"	float2 texCoord : TEXCOORD0, \n"
		"	float fog : FOG \n"
		") \n"
		"{ \n"
		"	float4 texColor = tex2D(texture, texCoord); \n"
		
		
	}*/
}
