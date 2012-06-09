#include "pch.h"
#include "../Defines.h"

#include "ArrowMaterial.h"
#include "MaterialDefinition.h"
#include "MaterialFactory.h"

#include <OgreMaterial.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreHighLevelGpuProgramManager.h>
using namespace Ogre;

ArrowMaterialGenerator::ArrowMaterialGenerator()
{
	mName = "Arrow";
}

void ArrowMaterialGenerator::generate()
{
	mMaterial = prepareMaterial(mDef->getName());
	
	// -------------------------- Main technique ----------------------------- //
	Ogre::Technique* technique = mMaterial->createTechnique();
	
	// create shader
	mVertexProgram = createVertexProgram();
	mFragmentProgram = createFragmentProgram();

	//  Pass 1 -----------------------------------------------------------
	Ogre::Pass* pass1 = technique->createPass();
	pass1->setColourWriteEnabled(false);
	pass1->setDepthWriteEnabled(true);
	pass1->setDepthFunction(CMPF_ALWAYS_PASS);
	pass1->setVertexProgram(mVertexProgram->getName());
	pass1->setFragmentProgram(mFragmentProgram->getName());
	pass1->setFog(true); // turn off fixed function fog, we use shaders

	//  Pass 2 -----------------------------------------------------------
	Ogre::Pass* pass2 = technique->createPass();
	pass2->setColourWriteEnabled(true);
	pass2->setDepthBias(3.f);
	pass2->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	pass2->setVertexProgram(mVertexProgram->getName());
	pass2->setFragmentProgram(mFragmentProgram->getName());
	pass2->setFog(true); // turn off fixed function fog, we use shaders

	// ----------------------------------------------------------------------- //
	
	createSSAOTechnique();
	createOccluderTechnique();
}

//----------------------------------------------------------------------------------------

HighLevelGpuProgramPtr ArrowMaterialGenerator::createVertexProgram()
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

	StringUtil::StrStreamType outStream;
	
	outStream <<
	"void main_vp( \n"
	"	float4 position : POSITION, \n"
	"	float3	normal					: NORMAL, \n"
	"	uniform float4   eyePosition, \n"
	"	uniform float4x4 ModelView, \n"
	"	uniform float4x4 ModelViewProj, \n"
	"	out float4 oEyeVector : TEXCOORD0, \n"
	"	out float3 oNormal : TEXCOORD1, \n"
	"	out		float4	oPosition				: POSITION \n"
	") \n"
	"{ \n"
	"	oEyeVector = mul( ModelView, position ) - eyePosition; \n"
	"	oNormal = normal; \n"
	"	oPosition = mul( ModelViewProj, position ); \n"
	"} \n";
	
	ret->setSource(outStream.str());
	ret->load();

	GpuProgramParametersSharedPtr params = ret->getDefaultParameters();
	params->setNamedAutoConstant("ModelViewProj", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
	params->setNamedAutoConstant("ModelView", GpuProgramParameters::ACT_WORLD_MATRIX);
	params->setNamedAutoConstant("eyePosition", GpuProgramParameters::ACT_CAMERA_POSITION);
	
	return ret;
}

//----------------------------------------------------------------------------------------

HighLevelGpuProgramPtr ArrowMaterialGenerator::createFragmentProgram()
{
	HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
	std::string progName = mDef->getName() + "_FP";

	HighLevelGpuProgramPtr ret = mgr.getByName(progName);
	if (!ret.isNull())
		mgr.remove(progName);

	ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
			"cg", GPT_FRAGMENT_PROGRAM);

	if(MRTSupported())
	{
		ret->setParameter("profiles", "ps_4_0 ps_3_0 fp40");
	}
	else
	{
		ret->setParameter("profiles", "ps_4_0 ps_2_x arbfp1");	
	}
	ret->setParameter("entry_point", "main_fp");

	StringUtil::StrStreamType outStream;

	outStream <<
	"void main_fp( \n"
	"	in	float4		position			: POSITION, \n"
	"	float3 eyeVector : TEXCOORD0, \n"
	"	float3 normal : TEXCOORD1, \n"
	"	uniform float3 color1, \n"
	"	uniform float3 color2, \n"
	"	uniform float fresnelBias, \n"
	"	uniform float fresnelScale, \n"
	"	uniform float fresnelPower, \n"
	"	out float4 color : COLOR \n"
	") \n"
	"{ \n"
	"	eyeVector = normalize(eyeVector); \n"
	"	normal = normalize(normal); \n"
	"	float colorFactor = fresnelBias + fresnelScale * pow(1 + dot(eyeVector, normal), fresnelPower); \n"
	"	colorFactor = min(colorFactor, 1); \n"
	"	float3 col = lerp(color1, color2, colorFactor); \n"
	"	color = float4(col.xyz, 0.5); \n"
	"} \n";

	ret->setSource(outStream.str());
	ret->load();

	GpuProgramParametersSharedPtr params = ret->getDefaultParameters();
	params->setNamedConstant("color1", Vector3(0.0, 1.0, 0.0));
	params->setNamedConstant("color2", Vector3(0.0, 0.4, 0.0));
	params->setNamedConstant("fresnelBias", Real(0));
	params->setNamedConstant("fresnelScale", Real(0.5));
	params->setNamedConstant("fresnelPower", Real(1));
	
	return ret;
}
