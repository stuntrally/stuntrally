#include "pch.h"
#include "../Defines.h"

#include "ParticleMaterial.h"
#include "MaterialDefinition.h"
#include "MaterialFactory.h"

#include <OgreMaterial.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreHighLevelGpuProgramManager.h>
using namespace Ogre;

ParticleMaterialGenerator::ParticleMaterialGenerator()
{
	mName = "Particle";
}

void ParticleMaterialGenerator::generate()
{
	mMaterial = prepareMaterial(mDef->getName());
	
	// reset some attributes
	resetTexUnitCounter();
	
	// choose textures from list (depending on user iTexSize setting)
	chooseTextures();
	
	// -------------------------- Main technique ----------------------------- //
	Ogre::Technique* technique = mMaterial->createTechnique();

	// particledepth pass
	Ogre::Pass* particleDepthPass = technique->createPass();
	particleDepthPass->setAmbient( mDef->mProps->ambient.x, mDef->mProps->ambient.y, mDef->mProps->ambient.z );
	particleDepthPass->setDiffuse( mDef->mProps->diffuse.x, mDef->mProps->diffuse.y, mDef->mProps->diffuse.z, 1.0 );
	
	particleDepthPass->setSpecular(mDef->mProps->specular.x, mDef->mProps->specular.y, mDef->mProps->specular.z, mDef->mProps->specular.w);
	
	particleDepthPass->setFog(true); // actually this disables fog

	if (!mDef->mProps->lighting)
		particleDepthPass->setLightingEnabled(false);
	
	if (mDef->mProps->sceneBlend == SBM_ALPHA_BLEND)
		particleDepthPass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	else if (mDef->mProps->sceneBlend == SBM_COLOUR_BLEND)
		particleDepthPass->setSceneBlending(SBT_TRANSPARENT_COLOUR);
	else if (mDef->mProps->sceneBlend == SBM_ADD)
		particleDepthPass->setSceneBlending(SBT_ADD);
	else if (mDef->mProps->sceneBlend == SBM_MODULATE)
		particleDepthPass->setSceneBlending(SBT_MODULATE);
		
	particleDepthPass->setDepthWriteEnabled( mDef->mProps->depthWrite );
		
	particleDepthPass->setDepthCheckEnabled( mDef->mProps->depthCheck );
		
	particleDepthPass->setTransparentSortingEnabled( mDef->mProps->transparentSorting );
	
	particleDepthPass->setAlphaRejectFunction( mDef->mProps->alphaRejectFunc );
	particleDepthPass->setAlphaRejectValue( mDef->mProps->alphaRejectValue );

	Ogre::TextureUnitState* tu = particleDepthPass->createTextureUnitState( mDiffuseMap );
	tu->setName("diffuseMap");
	tu->setTextureAddressingMode(mDef->mProps->textureAddressMode);

	tu = particleDepthPass->createTextureUnitState();
	tu->setName("depthMap");
	tu->setTextureAddressingMode(mDef->mProps->textureAddressMode);
	
	// create shaders
	HighLevelGpuProgramPtr fragmentProg, vertexProg;
	try
	{
		vertexProg = createSoftParticleVertexProgram();
		fragmentProg = createSoftParticleFragmentProgram();
	}
	catch (Ogre::Exception& e) {
		LogO(e.getFullDescription());
	}

	if (fragmentProg.isNull() || vertexProg.isNull() || 
		!fragmentProg->isSupported() || !vertexProg->isSupported())
	{
		LogO("[MaterialFactory] WARNING: ambient shader for material '" + mDef->getName()
			+ "' is not supported.");
	}
	else
	{
		particleDepthPass->setVertexProgram(vertexProg->getName());
		particleDepthPass->setFragmentProgram(fragmentProg->getName());
	}

	createSSAOTechnique();
	createOccluderTechnique();

	// indicate we need depth buffer set every frame
	mParent->softMtrs.push_back(mDef->getName());
	
}

//----------------------------------------------------------------------------------------

HighLevelGpuProgramPtr ParticleMaterialGenerator::createSoftParticleVertexProgram()
{
	HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
	std::string progName = mDef->getName() + "_ambient_VP";

	HighLevelGpuProgramPtr ret = mgr.getByName(progName);
	if (!ret.isNull())
		mgr.remove(progName);

	ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		"cg", GPT_VERTEX_PROGRAM);

	ret->setParameter("profiles", "vs_4_0 vs_1_1 arbvp1");
	ret->setParameter("entry_point", "main_vp");

	StringUtil::StrStreamType sourceStr;
	
	sourceStr <<
	"void main_vp( 	float4 position 					: POSITION, \n"
	"	float4 color 						: COLOR,  \n"
	"	float2 texCoord 					: TEXCOORD0,  \n"
	"	out float4 oPosition			 	: POSITION,  \n"
	"	out float4 objectPos				: COLOR,  \n"
	"	out float4 oTexCoord				: TEXCOORD0,  \n"
	"	out float4 oVertexColour				: TEXCOORD1,  \n"
	"	out float4 oScreenPosition				: TEXCOORD2,  \n"
	"	out float4 oWorldPosition				: TEXCOORD3,  \n"
	"	uniform float enableFog,  \n"
	"	uniform float4 fogParams,  \n"
	"	uniform float4x4 wvpMat,  \n"
	"	uniform float4x4 wMat  \n"
	")  \n"
	"{  \n"
	"	oVertexColour = color;  \n"
	"	oPosition = mul(wvpMat, position);  \n"
	"	oWorldPosition = mul(wMat, position);  \n"
	"	oScreenPosition = oPosition; \n"
	"	oTexCoord = float4(texCoord.x, texCoord.y, 1, 1);  \n"
	"	objectPos = position;  \n"
	"	objectPos.w = enableFog * saturate(fogParams.x * (oPosition.z - fogParams.y) * fogParams.w);  \n"
	"} \n";
	
	ret->setSource(sourceStr.str());
	ret->load();
	
	// params
	GpuProgramParametersSharedPtr params = ret->getDefaultParameters();
	params->setIgnoreMissingParams(true);
	params->setNamedAutoConstant("wvpMat", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
	params->setNamedAutoConstant("wMat", GpuProgramParameters::ACT_WORLD_MATRIX);
	params->setNamedAutoConstant("fogParams", GpuProgramParameters::ACT_FOG_PARAMS);
	params->setNamedConstant("enableFog", mDef->mProps->fog ? Real(1.0) : Real(0.0));
	
	return ret;
}

//----------------------------------------------------------------------------------------

HighLevelGpuProgramPtr ParticleMaterialGenerator::createSoftParticleFragmentProgram()
{
	HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
	std::string progName = mDef->getName() + "_ambient_FP";

	HighLevelGpuProgramPtr ret = mgr.getByName(progName);
	if (!ret.isNull())
		mgr.remove(progName);

	ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		"cg", GPT_FRAGMENT_PROGRAM);

	ret->setParameter("profiles", "ps_4_0 ps_3_0 ps_2_0 arbfp1");
	ret->setParameter("entry_point", "main_fp");

	StringUtil::StrStreamType sourceStr;
	
	sourceStr <<
	"	void main_fp(	in float4 iPosition : POSITION,  \n"
	"	in float4 position : COLOR,  \n"
	"	in float4 texCoord : TEXCOORD0,  \n"
	"	in float4 vertexColour : TEXCOORD1,  \n"
	"	in float4 positionScreen : TEXCOORD2,  \n"
	"	in float4 positionWorld : TEXCOORD3,  \n"
	"	uniform sampler2D diffuseMap : TEXUNIT0,  \n";
	
	if(MRTSupported())
	{
		sourceStr <<
			"	uniform sampler2D depthMap : TEXUNIT1, \n"
			"	uniform float useSoftParticles, \n";
	}
	
	sourceStr <<
	"	out float4 oColor : COLOR0,  \n"
	"	uniform float3 fogColor  \n";
	
	if(MRTSupported())
	{
		sourceStr <<
		",	uniform	float4 viewportSize \n"
		",	uniform float4 cameraPositionWorld	//world space \n"
		",	uniform float far  \n"	
		",  uniform half flip \n";
	}
	
	sourceStr <<
	")  \n"
	"{  \n"
	"	float4 diffuseTex = tex2D(diffuseMap, texCoord.xy);  \n"
	"	float4 color1 = diffuseTex;  \n"
	"	color1 *= vertexColour;  \n"
	"	oColor = lerp(color1, float4(fogColor,1), position.w);  \n";

	if(MRTSupported())
	{
		sourceStr <<
		"	//calculate depth at the real position \n"
		"	positionScreen /= positionScreen.w; \n"
		"	float2 depthTexCoord = float2(positionScreen) * float2(0.5f, -0.5f) + float2(0.5f, 0.5f); \n"
		"	float2 uvOffset= (viewportSize.zw)*0.5; \n"
		"	depthTexCoord += uvOffset; \n"
		"	depthTexCoord.y =(1-saturate(flip))+flip*depthTexCoord.y; \n"
        "	float depth = tex2D(depthMap, depthTexCoord).x; \n"
		"	if(useSoftParticles > 0) \n"	
		"	{ \n"	
		"	float distanceToPixel = length(positionWorld.xyz-cameraPositionWorld.xyz); \n"	
		"	float thickness = 0.5;//position_scale[3] * thicknessColour[0]; \n"
		"	float tNear = distanceToPixel - thickness; \n"
		"	float tFar = distanceToPixel + thickness; \n"
		"	depth *= far; \n"
		"	float depthAlpha = saturate(depth - distanceToPixel); \n"
		//these are debug values for the effect ,please don't remove
		//"	depthAlpha*=depthAlpha; \n"
		//"	//modify depth to get a good looking fog effect \n"
		//"	depthAlpha = log(depthAlpha) * 0.7f; \n"
		//"	oColor = float4(depth/20,depth/20,depth/20,1); \n"
		//"	oColor = float4(distanceToPixel,distanceToPixel,distanceToPixel,1); \n"
		//"	oColor = float4(depthAlpha,depthAlpha,depthAlpha,1); \n"
		"	oColor.a *=depthAlpha; \n"
		"	} \n";
	}

	sourceStr << "}  \n";

	
	ret->setSource(sourceStr.str());
	ret->load();
	
	// params
	GpuProgramParametersSharedPtr params = ret->getDefaultParameters();
	params->setNamedAutoConstant("fogColor", GpuProgramParameters::ACT_FOG_COLOUR);
	if(MRTSupported())
	{
		params->setNamedAutoConstant("far", GpuProgramParameters::ACT_FAR_CLIP_DISTANCE);
		params->setNamedAutoConstant("flip", GpuProgramParameters::ACT_RENDER_TARGET_FLIPPING);
		//depth
		params->setNamedAutoConstant("viewportSize", GpuProgramParameters::ACT_VIEWPORT_SIZE);
		//depthAlpha
		params->setNamedAutoConstant("cameraPositionWorld", GpuProgramParameters::ACT_CAMERA_POSITION);
	}
	return ret;
}
