#include "pch.h"
#include "../Defines.h"

#include "PipeGlassMaterial.h"
#include "MaterialDefinition.h"
#include "MaterialFactory.h"

#include <OgreMaterial.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreHighLevelGpuProgramManager.h>
using namespace Ogre;

PipeGlassMaterialGenerator::PipeGlassMaterialGenerator()
{
	mName = "PipeGlass";
}

//----------------------------------------------------------------------------------------

void PipeGlassMaterialGenerator::generate()
{
	mMaterial = prepareMaterial(mDef->getName());
	
	// reset some attributes
	resetTexUnitCounter();
	
	// choose textures from list (depending on user iTexSize setting)
	chooseTextures();
	
	// -------------------------- Main technique ----------------------------- //
	Ogre::Technique* technique = mMaterial->createTechnique();

	// Pass 1
	Ogre::Pass* pass1 = technique->createPass();
	pass1->setAmbient( mDef->mProps->ambient.x, mDef->mProps->ambient.y, mDef->mProps->ambient.z );
	pass1->setDiffuse( mDef->mProps->diffuse.x, mDef->mProps->diffuse.y, mDef->mProps->diffuse.z, 1.0 );
	pass1->setSpecular(mDef->mProps->specular.x, mDef->mProps->specular.y, mDef->mProps->specular.z, 1.0);
	pass1->setShininess(mDef->mProps->specular.w);
		
	pass1->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	pass1->setDepthBias( mDef->mProps->depthBias );
	pass1->setDepthWriteEnabled(false);
	pass1->setCullingMode(CULL_CLOCKWISE);
	pass1->setFog(true);

	Ogre::TextureUnitState* tu = pass1->createTextureUnitState( mDiffuseMap );
	tu->setName("diffuseMap");
	tu->setTextureAddressingMode(mDef->mProps->textureAddressMode);
	
	// Pass 2 (only difference: cull anticlockwise)
	Ogre::Pass* pass2 = technique->createPass();
	pass2->setAmbient( mDef->mProps->ambient.x, mDef->mProps->ambient.y, mDef->mProps->ambient.z );
	pass2->setDiffuse( mDef->mProps->diffuse.x, mDef->mProps->diffuse.y, mDef->mProps->diffuse.z, 1.0 );
	pass2->setSpecular(mDef->mProps->specular.x, mDef->mProps->specular.y, mDef->mProps->specular.z, 1.0);
	pass2->setShininess(mDef->mProps->specular.w);
		
	pass2->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	pass2->setDepthBias( mDef->mProps->depthBias );
	pass2->setDepthWriteEnabled(false);
	pass2->setCullingMode(CULL_ANTICLOCKWISE);
	pass2->setFog(true);

	tu = pass2->createTextureUnitState( mDiffuseMap );
	tu->setName("diffuseMap");
	tu->setTextureAddressingMode(mDef->mProps->textureAddressMode);
	
	// create shaders (same for both passes)
	if (!mShaderCached)
	{
		HighLevelGpuProgramPtr fragmentProg, vertexProg;
		try
		{
			mVertexProgram = createPipeVertexProgram();
			mFragmentProgram = createPipeFragmentProgram();
		}
		catch (Ogre::Exception& e) {
			LogO(e.getFullDescription());
		}

		if (mFragmentProgram.isNull() || mVertexProgram.isNull() || 
			!mFragmentProgram->isSupported() || !mVertexProgram->isSupported())
		{
			LogO("[MaterialFactory] WARNING: pipe glass shader for material '" + mDef->getName()
				+ "' is not supported.");
		}
		else
		{
			pass1->setVertexProgram(mVertexProgram->getName());
			pass1->setFragmentProgram(mFragmentProgram->getName());
			pass2->setVertexProgram(mVertexProgram->getName());
			pass2->setFragmentProgram(mFragmentProgram->getName());
		}
	}
	else
	{
		pass1->setVertexProgram(mVertexProgram->getName());
		pass1->setFragmentProgram(mFragmentProgram->getName());
		pass2->setVertexProgram(mVertexProgram->getName());
		pass2->setFragmentProgram(mFragmentProgram->getName());
		
		individualFragmentProgramParams(pass1->getFragmentProgramParameters());
		individualFragmentProgramParams(pass2->getFragmentProgramParameters());
	}

	// ----------------------------------------------------------------------- //
	
	createSSAOTechnique();
	createOccluderTechnique();
}

//----------------------------------------------------------------------------------------

HighLevelGpuProgramPtr PipeGlassMaterialGenerator::createPipeVertexProgram()
{
	HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
	std::string progName = mDef->getName() + "_pipe_VP";

	HighLevelGpuProgramPtr ret = mgr.getByName(progName);
	if (!ret.isNull())
		mgr.remove(progName);

	ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		"cg", GPT_VERTEX_PROGRAM);

	ret->setParameter("profiles", "vs_4_0 vs_1_1 arbvp1");
	ret->setParameter("entry_point", "main_vp");

	StringUtil::StrStreamType sourceStr;
	
	//!todo optimize
	
	sourceStr <<
	"struct VIn \n"
	"{ \n"
	"	float4 p : POSITION;	float3 n : NORMAL; \n"
	"	float3 t : TANGENT;		float3 uv: TEXCOORD0; \n"
	"	float4 c : COLOR; \n"
	"}; \n"
	"struct VOut \n"
	"{ \n"
	"	float4 p : POSITION;	float3 uv : TEXCOORD0;	float4 wp : TEXCOORD1; \n"
	"	float3 n : TEXCOORD2;	float3 t  : TEXCOORD3;	float3 b  : TEXCOORD4; \n"
	"	float4 c : COLOR; \n"
	"}; \n"
	"VOut main_vp(VIn IN, \n"
	"	uniform float4x4 wMat,  uniform float4x4 wvpMat, \n"
	"	uniform float4 fogParams) \n"
	"{ \n"
	"	VOut OUT;  OUT.uv = IN.uv; \n"
	"	OUT.wp = mul(wMat, IN.p); \n"
	"	OUT.p = mul(wvpMat, IN.p); \n"
	"	OUT.n = IN.n;  OUT.t = IN.t;  OUT.b = cross(IN.t, IN.n); \n"
	"	OUT.c = IN.c;  //clr \n"
	"	OUT.wp.w = saturate(fogParams.x * (OUT.p.z - fogParams.y) * fogParams.w); \n"
	"	return OUT; \n"
	"} \n";
	
	ret->setSource(sourceStr.str());
	ret->load();
	
	// params
	GpuProgramParametersSharedPtr params = ret->getDefaultParameters();
	params->setNamedAutoConstant("wMat", GpuProgramParameters::ACT_WORLD_MATRIX);
	params->setNamedAutoConstant("wvpMat", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
	params->setNamedAutoConstant("fogParams", GpuProgramParameters::ACT_FOG_PARAMS);
	
	return ret;
}

//----------------------------------------------------------------------------------------

HighLevelGpuProgramPtr PipeGlassMaterialGenerator::createPipeFragmentProgram()
{
	HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
	std::string progName = mDef->getName() + "_pipe_FP";

	HighLevelGpuProgramPtr ret = mgr.getByName(progName);
	if (!ret.isNull())
		mgr.remove(progName);

	ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		"cg", GPT_FRAGMENT_PROGRAM);

	ret->setParameter("profiles", "ps_4_0 ps_2_x arbfp1");
	ret->setParameter("entry_point", "main_fp");

	StringUtil::StrStreamType sourceStr;
	
	sourceStr <<
	"struct PIn \n"
	"{	float4 p : POSITION;	float3 uv : TEXCOORD0;	float4 wp : TEXCOORD1; \n"
	"	float3 n : TEXCOORD2;	float3 t  : TEXCOORD3;	float3 b  : TEXCOORD4; \n"
	"	float4 c : COLOR; \n"
	"}; \n"
	"float4 main_fp(PIn IN, \n"
	"	uniform float4 alphaPars, \n"
	"	uniform float3 globalAmbient, \n"
	"	uniform float3 ambient,  uniform float3 lightDif0,  uniform float3 lightSpec0, \n"
	"	uniform float4 matDif,   uniform float4 matSpec,	uniform float matShininess, \n"
	"	uniform float3 fogColor, \n"
	"	uniform float4 lightPos0,  uniform float3 camPos, \n"
	"	uniform float4x4 iTWMat, \n"
	"	uniform sampler2D diffuseMap : TEXUNIT0, \n"
	"	uniform sampler2D normalMap : TEXUNIT1): COLOR0 \n"
	"{ \n"
	"	float3 ldir = normalize(lightPos0.xyz - (lightPos0.w * IN.wp.xyz)); \n"
	"	float4 normalTex = tex2D(normalMap, IN.uv.xy); \n"
	"	float3x3 tbn = float3x3(IN.t, IN.b, IN.n); \n"
	"	float3 normal = mul(transpose(tbn), normalTex.xyz * 2.f - 1.f); \n"
	"	normal = normalize(mul((float3x3)iTWMat, normal)); \n"
	"	float3 diffuse = max(dot(ldir, -normal),0); \n"
	"	float3 camDir = normalize(camPos - IN.wp.xyz); \n"
	"	float3 halfVec = normalize(ldir + camDir); \n"
	"	float3 specular = pow(max(dot(-normal, halfVec),0), matShininess); \n"
	"	float4 diffuseTex = tex2D(diffuseMap, IN.uv.xy); \n"
	"	float3 diffC = diffuse * lightDif0 * matDif.rgb  * diffuseTex.rgb; \n"
	"	float3 specC = specular * lightSpec0 * matSpec.rgb; \n"
	"	float3 clrSUM = diffuseTex.rgb * ambient * globalAmbient + diffC + specC; \n"
	"	clrSUM = lerp(clrSUM, fogColor, IN.wp.w); \n"
	"	float alpha = alphaPars.x + alphaPars.y * diffuse.r + alphaPars.z * specular.r + (1 - diffuseTex.r); \n"
	"	return float4(clrSUM, alpha); \n"
	"} \n";
	
	ret->setSource(sourceStr.str());
	ret->load();
	
	// params
	GpuProgramParametersSharedPtr params = ret->getDefaultParameters();
	params->setNamedAutoConstant("ambient", GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR);
	params->setNamedAutoConstant("globalAmbient", GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
	params->setNamedAutoConstant("lightDif0", GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 0);
	params->setNamedAutoConstant("lightSpec0", GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR, 0);
	params->setNamedAutoConstant("matDif", GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR);
	params->setNamedAutoConstant("matSpec", GpuProgramParameters::ACT_SURFACE_SPECULAR_COLOUR);
	params->setNamedAutoConstant("matShininess", GpuProgramParameters::ACT_SURFACE_SHININESS);
	params->setNamedAutoConstant("fogColor", GpuProgramParameters::ACT_FOG_COLOUR);
	params->setNamedAutoConstant("camPos", GpuProgramParameters::ACT_CAMERA_POSITION);
	params->setNamedAutoConstant("lightPos0", GpuProgramParameters::ACT_LIGHT_POSITION, 0);
	params->setNamedAutoConstant("iTWMat", GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX);
	
	individualFragmentProgramParams(params);

	return ret;
}

//----------------------------------------------------------------------------------------

void PipeGlassMaterialGenerator::individualFragmentProgramParams(Ogre::GpuProgramParametersSharedPtr params)
{
	params->setNamedConstant("alphaPars", mDef->mProps->lightingAlpha);
}
