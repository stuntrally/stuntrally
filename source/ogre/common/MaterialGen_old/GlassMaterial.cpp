#include "pch.h"
#include "../Defines.h"

#include "GlassMaterial.h"
#include "MaterialDefinition.h"
#include "MaterialFactory.h"

#include <OgreMaterial.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreHighLevelGpuProgramManager.h>
using namespace Ogre;

GlassMaterialGenerator::GlassMaterialGenerator()
{
	mName = "Glass";
}

void GlassMaterialGenerator::generate()
{
	mMaterial = prepareMaterial(mDef->getName());
	
	// reset some attributes
	resetTexUnitCounter();
	
	// choose textures from list (depending on user iTexSize setting)
	chooseTextures();
	
	// -------------------------- Main technique ----------------------------- //
	Ogre::Technique* technique = mMaterial->createTechnique();

	// Ambient pass
	Ogre::Pass* ambientPass = technique->createPass();
	ambientPass->setAmbient( mDef->mProps->ambient.x, mDef->mProps->ambient.y, mDef->mProps->ambient.z );
	ambientPass->setDiffuse( mDef->mProps->diffuse.x, mDef->mProps->diffuse.y, mDef->mProps->diffuse.z, 1.0 );
	
	ambientPass->setSpecular(mDef->mProps->specular.x, mDef->mProps->specular.y, mDef->mProps->specular.z, mDef->mProps->specular.w);
	
	ambientPass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		
	ambientPass->setDepthBias( mDef->mProps->depthBias );
	ambientPass->setDepthWriteEnabled(false);
	ambientPass->setCullingMode(CULL_NONE);
	ambientPass->setFog(true); // turn off fixed function fog, we use shaders

	
	Ogre::TextureUnitState* tu = ambientPass->createTextureUnitState( mDiffuseMap );
	tu->setName("diffuseMap");
	tu->setTextureAddressingMode(mDef->mProps->textureAddressMode);
	
	// create shaders
	HighLevelGpuProgramPtr fragmentProg, vertexProg;
	try
	{
		vertexProg = createAmbientVertexProgram();
		fragmentProg = createAmbientFragmentProgram();
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
		ambientPass->setVertexProgram(vertexProg->getName());
		ambientPass->setFragmentProgram(fragmentProg->getName());
	}
	
	// Main pass
	Ogre::Pass* pass = technique->createPass();
	
	// already have ambient in first pass
	pass->setAmbient(0.0, 0.0, 0.0);
	
	pass->setDiffuse( mDef->mProps->diffuse.x, mDef->mProps->diffuse.y, mDef->mProps->diffuse.z, 1.0 );
	
	// shader assumes shininess in specular w component
	pass->setSpecular(mDef->mProps->specular.x, mDef->mProps->specular.y, mDef->mProps->specular.z, mDef->mProps->specular.w);
	
	pass->setCullingMode(CULL_NONE);
	
	pass->setFog(true); // actually this disables fog
		
	if (!mDef->mProps->lighting)
		pass->setLightingEnabled(false);
	
	pass->setSceneBlending(SBT_ADD);
		
	pass->setDepthWriteEnabled( false );
		
	pass->setAlphaRejectFunction( mDef->mProps->alphaRejectFunc );
	pass->setAlphaRejectValue( mDef->mProps->alphaRejectValue );
	
	if (mDef->mProps->depthBias != 0.f)
		pass->setDepthBias( mDef->mProps->depthBias );
	
	createTexUnits(pass);
	
	// create shaders		
	if (!mShaderCached)
	{
		try
		{
			mVertexProgram = createVertexProgram();
			mFragmentProgram = createFragmentProgram();
		}
		catch (Ogre::Exception& e) {
			LogO(e.getFullDescription());
		}
		
		if (mFragmentProgram.isNull() || mVertexProgram.isNull() || 
			!mFragmentProgram->isSupported() || !mVertexProgram->isSupported())
		{
			LogO("[MaterialFactory] WARNING: shader for material '" + mDef->getName()
				+ "' is not supported");
								
			//LogO("[MaterialFactory] Vertex program source: ");
			StringUtil::StrStreamType vSourceStr;
			generateVertexProgramSource(vSourceStr);
			//LogO(vSourceStr.str());
			//LogO("[MaterialFactory] Fragment program source: ");
			StringUtil::StrStreamType fSourceStr;
			generateFragmentProgramSource(fSourceStr);
			//LogO(fSourceStr.str());
			
			mVertexProgram.setNull(); mFragmentProgram.setNull();
			return;
		}
	}
	
	pass->setVertexProgram(mVertexProgram->getName());
	pass->setFragmentProgram(mFragmentProgram->getName());
	
	if (mShaderCached)
	{
		// set individual material shader params
		individualVertexProgramParams(pass->getVertexProgramParameters());
		individualFragmentProgramParams(pass->getFragmentProgramParameters());
	}
	// ----------------------------------------------------------------------- //
	
	createSSAOTechnique();
	createOccluderTechnique();
}

//----------------------------------------------------------------------------------------

HighLevelGpuProgramPtr GlassMaterialGenerator::createAmbientVertexProgram()
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
	"void main_vp( \n"
	"	in float4 position : POSITION, \n"
	"	in float2 uv :TEXCOORD0, \n"
	"	uniform float4x4 wvpMat, \n"
	"	out float4 oPos : POSITION, out float2 oUV : TEXCOORD0) \n"
	"{ \n"
	"	oPos = mul(wvpMat, position);  oUV = uv; \n"
	"} \n";
	
	ret->setSource(sourceStr.str());
	ret->load();
	
	// params
	GpuProgramParametersSharedPtr params = ret->getDefaultParameters();
	params->setNamedAutoConstant("wvpMat", GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);
	
	return ret;
}

//----------------------------------------------------------------------------------------

HighLevelGpuProgramPtr GlassMaterialGenerator::createAmbientFragmentProgram()
{
	HighLevelGpuProgramManager& mgr = HighLevelGpuProgramManager::getSingleton();
	std::string progName = mDef->getName() + "_ambient_FP";

	HighLevelGpuProgramPtr ret = mgr.getByName(progName);
	if (!ret.isNull())
		mgr.remove(progName);

	ret = mgr.createProgram(progName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		"cg", GPT_FRAGMENT_PROGRAM);

	ret->setParameter("profiles", "ps_4_0 ps_2_0 arbfp1");
	ret->setParameter("entry_point", "main_fp");

	StringUtil::StrStreamType sourceStr;
	
	sourceStr <<
	"float4 main_fp(float4 iPos : POSITION,in float2 uv : TEXCOORD0, \n"
	"	uniform float3 ambient,  uniform float3 globalAmbient,  uniform float4 matDif, \n"
	"	uniform sampler2D diffuseMap): COLOR0 \n"
	"{ \n"
	"	float4 diffuseTex = tex2D(diffuseMap, uv); \n"
	"	return float4(ambient * globalAmbient * matDif.rgb * diffuseTex.rgb, diffuseTex.a); \n"
	"} \n";
	
	ret->setSource(sourceStr.str());
	ret->load();
	
	// params
	GpuProgramParametersSharedPtr params = ret->getDefaultParameters();
	params->setNamedAutoConstant("ambient", GpuProgramParameters::ACT_SURFACE_AMBIENT_COLOUR );
	params->setNamedAutoConstant("globalAmbient", GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR);
	params->setNamedAutoConstant("matDif", GpuProgramParameters::ACT_SURFACE_DIFFUSE_COLOUR );
	
	return ret;
}

