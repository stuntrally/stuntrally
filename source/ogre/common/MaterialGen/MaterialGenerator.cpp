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



bool MaterialGenerator::bUseMRT=false;	


void MaterialGenerator::generate()
{	
	mMaterial = prepareMaterial(mDef->getName());
	
	// reset some attributes
	resetTexUnitCounter();
	
	// choose textures from list (depending on user iTexSize setting)
	chooseTextures();
	
	// -------------------------- Main technique ----------------------------- //
	Ogre::Technique* technique = mMaterial->createTechnique();
	
	// Main pass
	Ogre::Pass* pass = technique->createPass();
	
	pass->setAmbient( mDef->mProps->ambient.x, mDef->mProps->ambient.y, mDef->mProps->ambient.z );
	pass->setDiffuse( mDef->mProps->diffuse.x, mDef->mProps->diffuse.y, mDef->mProps->diffuse.z, 1.0 );
	
	// shader assumes shininess in specular w component
	pass->setSpecular(mDef->mProps->specular.x, mDef->mProps->specular.y, mDef->mProps->specular.z, mDef->mProps->specular.w);
	
	pass->setCullingMode(chooseCullingMode());
	
	pass->setFog(true); // turn off fixed function fog, we use shaders
		
	if (!mDef->mProps->lighting)
		pass->setLightingEnabled(false);
	
	if (mDef->mProps->sceneBlend == SBM_ALPHA_BLEND)
		pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	else if (mDef->mProps->sceneBlend == SBM_COLOUR_BLEND)
		pass->setSceneBlending(SBT_TRANSPARENT_COLOUR);
	else if (mDef->mProps->sceneBlend == SBM_ADD)
		pass->setSceneBlending(SBT_ADD);
	else if (mDef->mProps->sceneBlend == SBM_MODULATE)
		pass->setSceneBlending(SBT_MODULATE);
		
	pass->setDepthWriteEnabled( mDef->mProps->depthWrite );
		
	pass->setDepthCheckEnabled( mDef->mProps->depthCheck );
		
	pass->setTransparentSortingEnabled( mDef->mProps->transparentSorting );
	
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
		
		//!todo put this code into a function for reusability in other material generators
		if (mFragmentProgram.isNull() || mVertexProgram.isNull() || 
			!mFragmentProgram->isSupported() || !mVertexProgram->isSupported())
		{
			LogO("[MaterialFactory] WARNING: shader for material '" + mDef->getName()
				+ "' is not supported");
				
			LogO("[MaterialFactory] Vertex program source: ");
			StringUtil::StrStreamType vSourceStr;
			generateVertexProgramSource(vSourceStr);
			LogO(vSourceStr.str());
			LogO("[MaterialFactory] Fragment program source: ");
			StringUtil::StrStreamType fSourceStr;
			generateFragmentProgramSource(fSourceStr);
			LogO(fSourceStr.str());
			
			mVertexProgram.setNull(); mFragmentProgram.setNull();
			return;
		}
	}
	
	pass->setVertexProgram(mVertexProgram->getName());
	pass->setFragmentProgram(mFragmentProgram->getName());
	
	//set shadow caster
	technique->setShadowCasterMaterial(chooseShadowCasterMaterial());
	if (mShaderCached)
	{
		// set individual material shader params
		individualVertexProgramParams(pass->getVertexProgramParameters());
		individualFragmentProgramParams(pass->getFragmentProgramParameters());
	}
	// ----------------------------------------------------------------------- //
	
	createSSAOTechnique();
	createOccluderTechnique();
		
	// indicate we need enable/disable wind parameter
	// only needed for trees (wind == 2) because the wind effect has to be disabled before rendering impostors
	if (mShader->wind == 2)
		mParent->windMtrs.push_back( mDef->getName() );
		
	if (mDef->mProps->fog)
		mParent->fogMtrs.push_back( mDef->getName() );
	
	
	/// uncomment to export to .material
	//LogO(mDef->getName());
	/**
	//if (mDef->getName() == "Water_cyan")
	/*{
		MaterialSerializer serializer;
		serializer.exportMaterial(mMaterial, "water.material");
	}
	/**/
	
	/// uncomment to see full shader source code in log
	/**
	LogO(mDef->getName());
	if (StringUtil::startsWith(mDef->getName(), "FluidWater" ))  //"water" //_cyan"
	{
		LogO("[MaterialFactory] Vertex program source: ");
		StringUtil::StrStreamType vSourceStr;
		generateVertexProgramSource(vSourceStr);
		LogO(vSourceStr.str());
		LogO("[MaterialFactory] Fragment program source: ");
		StringUtil::StrStreamType fSourceStr;
		generateFragmentProgramSource(fSourceStr);
		LogO(fSourceStr.str());
	}
	/**/
}

//----------------------------------------------------------------------------------------

void MaterialGenerator::createTexUnits(Ogre::Pass* pass)
{
	Ogre::TextureUnitState* tu;
	
	// diffuse / light / blend maps
	if (needDiffuseMap())
	{
		tu = pass->createTextureUnitState( mDiffuseMap );
		tu->setName("diffuseMap");
		tu->setTextureAddressingMode(mDef->mProps->textureAddressMode);
		mDiffuseTexUnit = mTexUnit_i; mTexUnit_i++;
	}
	if (needLightMap())
	{
		tu = pass->createTextureUnitState( mLightMap );
		tu->setName("lightMap");
		tu->setTextureAddressingMode(mDef->mProps->textureAddressMode);
		mLightTexUnit = mTexUnit_i; mTexUnit_i++;
	}
	if (needBlendMap())
	{
		tu = pass->createTextureUnitState( mBlendMap );
		tu->setName("blendMap");
		tu->setTextureAddressingMode(mDef->mProps->textureAddressMode);
		mBlendTexUnit = mTexUnit_i; mTexUnit_i++;
	}
	
	// spec map
	if (needSpecMap())
	{
		tu = pass->createTextureUnitState( mSpecMap );
		tu->setName("specMap");
		tu->setTextureAddressingMode(mDef->mProps->textureAddressMode);
		mSpecTexUnit = mTexUnit_i; mTexUnit_i++;
	}
	
	// reflectivity map
	if (needReflectivityMap())
	{
		tu = pass->createTextureUnitState( mReflMap );
		tu->setName("reflectivityMap");
		tu->setTextureAddressingMode(mDef->mProps->textureAddressMode);
		mReflTexUnit = mTexUnit_i; mTexUnit_i++;
	}
	
	// global terrain lightmap (static)
	if (needTerrainLightMap())
	{
		tu = pass->createTextureUnitState(String("white.png")); // texture name set later (in changeShadows)
		tu->setName("terrainLightMap");
		mTerrainLightTexUnit = mTexUnit_i; mTexUnit_i++;
	}
	
	// alpha map
	if (needAlphaMap())
	{
		tu = pass->createTextureUnitState( mAlphaMap );
		tu->setName("alphaMap");
		mAlphaTexUnit = mTexUnit_i; mTexUnit_i++;
	}
	
	// normal map
	if (needNormalMap())
	{
		tu = pass->createTextureUnitState( mNormalMap );
		tu->setName("normalMap");
		mNormalTexUnit = mTexUnit_i; mTexUnit_i++;
	}

	// env map
	if (needEnvMap())
	{
		tu = pass->createTextureUnitState( mDef->mProps->envMap );
		tu->setName("envMap");
		tu->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
		mEnvTexUnit = mTexUnit_i; mTexUnit_i++;
	}

	// realtime shadow maps
	if (needShadows())
	{
		mShadowTexUnit_start = mTexUnit_i;
		for (int i = 0; i < mParent->getNumShadowTex(); ++i)
		{
			tu = pass->createTextureUnitState();
			tu->setName("shadowMap" + toStr(i));
			tu->setContentType(TextureUnitState::CONTENT_SHADOW);
			tu->setTextureAddressingMode(TextureUnitState::TAM_BORDER);
			tu->setTextureBorderColour(ColourValue::White);
			mTexUnit_i++;
		}
	}
}

//----------------------------------------------------------------------------------------

void MaterialGenerator::resetTexUnitCounter()
{
	mDiffuseTexUnit = 0; mNormalTexUnit = 0; mEnvTexUnit = 0; mAlphaTexUnit = 0;
	mShadowTexUnit_start = 0; mTerrainLightTexUnit = 0; mTexUnit_i = 0;
}

//----------------------------------------------------------------------------------------

void MaterialGenerator::createSSAOTechnique()
{
	Technique* ssaopasstech = mMaterial->createTechnique();
	ssaopasstech->setName("geom");
	ssaopasstech->setSchemeName("geom");
	Pass* ssaopass = ssaopasstech->createPass();
	
	ssaopass->setDepthWriteEnabled( mDef->mProps->depthWrite );	
	ssaopass->setDepthCheckEnabled( mDef->mProps->depthCheck );
	
	HighLevelGpuProgramManager& hmgr = HighLevelGpuProgramManager::getSingleton();
	
	// choose vertex program
	std::string vprogname = "geom_vs";
	if ( mDef->mProps->transparent )
		vprogname = "geom_coord_vs";
	
	// choose fragment program
	std::string fprogname = "geom_ps";
	if ( !mDef->mProps->ssao )
		fprogname = "geom_white_ps"; // no contribution to ssao, will just return (0,0,0,0)
	else if ( mDef->mProps->transparent )
		fprogname = "geom_alpha_ps";
	
	ssaopass->setVertexProgram(vprogname);
	ssaopass->setFragmentProgram(fprogname);
	
	if (mDef->mProps->ssaoReject)
		ssaopass->setAlphaRejectSettings(CMPF_GREATER, 128);
	
	if ( !mDef->mProps->transparent ) 
	{
		ssaopass->setCullingMode( chooseCullingMode() );
	}
	else
	{
		ssaopass->setCullingMode( CULL_NONE );
		ssaopass->setAlphaRejectSettings(CMPF_GREATER_EQUAL, 128);
		ssaopass->createTextureUnitState( mDiffuseMap );
	}
}

//----------------------------------------------------------------------------------------

void MaterialGenerator::createOccluderTechnique()
{
	Technique* occluderpasstech = mMaterial->createTechnique();
	occluderpasstech->setName("occluder");
	occluderpasstech->setSchemeName("occluder");
	Pass* occluderpass = occluderpasstech->createPass();

	HighLevelGpuProgramManager& hmgr = HighLevelGpuProgramManager::getSingleton();
	
	
	// choose vertex program
	std::string vprogname = "occluder_vs";
	if ( mDef->mProps->transparent )
		vprogname = "occluder_coord_vs";

	//TODO:this is a workaround until a valid sun material is available to be used.
	if(StringUtil::startsWith(this->mDef->getName(), "sky/"))
	{
		//use the sky object as the sun 
		vprogname = mVertexProgram->getName();
	}
	// choose fragment program
	std::string fprogname = "occluder_ps";
	if ( mDef->mProps->transparent )
		fprogname = "occluder_alpha_ps";
	//TODO:this is a workaround until a valid sun material is available to be used.
	if(StringUtil::startsWith(this->mDef->getName(), "sky/"))
	{
		//use the sky object as the sun 
		fprogname = mFragmentProgram->getName();
	}
	
	occluderpass->setVertexProgram(vprogname);
	occluderpass->setFragmentProgram(fprogname);
	
	if (mDef->mProps->alphaRejectValue  > 0)
		occluderpass->setAlphaRejectSettings(mDef->mProps->alphaRejectFunc, mDef->mProps->alphaRejectValue);
	
	if ( !mDef->mProps->transparent ) 
	{
		occluderpass->setCullingMode( chooseCullingMode() );
		if(StringUtil::startsWith(this->mDef->getName(), "sky/"))
		{
			//Set the sky object as the sun 
			occluderpass->createTextureUnitState( mDiffuseMap );
		}
	}
	else
	{
		occluderpass->setCullingMode( CULL_NONE );
		occluderpass->setAlphaRejectSettings(CMPF_GREATER_EQUAL, 128);
		occluderpass->createTextureUnitState( mDiffuseMap );
	}
}

//----------------------------------------------------------------------------------------

void MaterialGenerator::chooseTextures()
{
	mDiffuseMap = pickTexture(&mDef->mProps->diffuseMaps);
	mNormalMap = pickTexture(&mDef->mProps->normalMaps);
	mLightMap = pickTexture(&mDef->mProps->lightMaps);
	mAlphaMap = pickTexture(&mDef->mProps->alphaMaps);
	mBlendMap = pickTexture(&mDef->mProps->blendMaps);
	mSpecMap = pickTexture(&mDef->mProps->specMaps);
	mReflMap = pickTexture(&mDef->mProps->reflectivityMaps);
}

//----------------------------------------------------------------------------------------

MaterialPtr MaterialGenerator::prepareMaterial(const std::string& name)
{
	MaterialPtr mat;
	
	if (MaterialManager::getSingleton().resourceExists(name))
		mat = MaterialManager::getSingleton().getByName(name);
	else
		mat = MaterialManager::getSingleton().create(name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	
	mat->removeAllTechniques();
	
	return mat;
}

//----------------------------------------------------------------------------------------

bool MaterialGenerator::needShadows()
{
	return mShader->shadows;
}

bool MaterialGenerator::needNormalMap()
{
	return mShader->normalMap;
}

bool MaterialGenerator::needEnvMap()
{
	return mShader->envMap;
}

bool MaterialGenerator::needDiffuseMap()
{
	return mShader->diffuseMap;
}

bool MaterialGenerator::needLightMap()
{
	return mShader->lightMap;
}

bool MaterialGenerator::needTerrainLightMap()
{
	// temporary workaround. terrain lightmap is broken when depth shadows off (texture not found).
	return mShader->terrainLightMap && mParent->getShadowsDepth();
}

bool MaterialGenerator::needBlendMap()
{
	return mShader->blendMap;
}

bool MaterialGenerator::needSpecMap()
{
	return mShader->specMap;
}

bool MaterialGenerator::needReflectivityMap()
{
	return mShader->reflectivityMap;
}

bool MaterialGenerator::needLightingAlpha()
{
	return mShader->lightingAlpha;
}

bool MaterialGenerator::needAlphaMap()
{
	return mShader->alphaMap;
}

bool MaterialGenerator::needFresnel()
{
	return mShader->fresnel;
}

bool MaterialGenerator::fpNeedLighting()
{
	return mShader->lighting;
}

bool MaterialGenerator::fpNeedNormal()
{
	return (needEnvMap() || needNormalMap() || fpNeedLighting() || needTerrainLightMap());
}

bool MaterialGenerator::fpNeedEyeVector()
{
	return needEnvMap() || fpNeedLighting() || (MRTSupported() && !(fpNeedNormal() && (!(needEnvMap() || needNormalMap() || fpNeedLighting()))));
}

bool MaterialGenerator::vpNeedTangent()
{
	return needNormalMap();
}

bool MaterialGenerator::vpNeedWMat()
{
	return vpCalcWPos();
}

bool MaterialGenerator::vpNeedNormal()
{
	return fpNeedNormal() || MRTSupported();
}

bool MaterialGenerator::fpNeedWMat()
{
	return UsePerPixelNormals();
}

bool MaterialGenerator::fpNeedPos()
{
	//return (needTerrainLightMap() || fpNeedLighting() || MRTSupported() || mShader->parallax);
        return true; // always for fog
}

bool MaterialGenerator::vpCalcWPos()
{
	return (fpNeedPos() || fpNeedEyeVector() || mShader->wind == 1);
}

bool MaterialGenerator::vpNeedWvMat()
{
	return MRTSupported();
}

bool MaterialGenerator::UsePerPixelNormals()
{
	return false;//this is not working at the moment
}

bool MaterialGenerator::MRTSupported()
{
	//return false;  //!test-
	// buffer sharing between compositors not possible in ogre 1.7, thus disable MRT
	#if OGRE_VERSION_MINOR < 8
	return false;
	#else
	static bool bMRTSupportCalculated=false;
	if(!bMRTSupportCalculated)
	{
		const RenderSystemCapabilities *caps = Root::getSingleton().getRenderSystem()->getCapabilities();
		if(caps->isShaderProfileSupported("ps_3_0") 
		//	|| caps->isShaderProfileSupported("ps_4_0")
			|| caps->isShaderProfileSupported("fp40")
			)
		{
			bUseMRT=true;
		}
		bMRTSupportCalculated=true;
	}
	return bUseMRT;
	#endif
}

bool MaterialGenerator::vpNeedWITMat()
{
	return fpNeedNormal();
}

std::string MaterialGenerator::getChannel(unsigned int n)
{
	if (n == 0) 		return "x";
	else if (n == 1) 	return "y";
	else if (n == 2)	return "z";
	else 				return "w";
}

//----------------------------------------------------------------------------------------

std::string MaterialGenerator::pickTexture(textureMap* textures)
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

Ogre::CullingMode MaterialGenerator::chooseCullingMode()
{

	if 		(mDef->mProps->cullHardware == CULL_HW_NONE)
		return Ogre::CULL_NONE;
	else if (mDef->mProps->cullHardware == CULL_HW_CLOCKWISE)
		return Ogre::CULL_CLOCKWISE;
	else if (mDef->mProps->cullHardware == CULL_HW_ANTICLOCKWISE)
		return Ogre::CULL_ANTICLOCKWISE;
	else if (mDef->mProps->cullHardware == CULL_HW_CLOCKWISE_OR_NONE)
	{
		if (mParent->getShadowsDepth())
			return Ogre::CULL_NONE;
		else
			return Ogre::CULL_CLOCKWISE;
	}
	else if (mDef->mProps->cullHardware == CULL_HW_ANTICLOCKWISE_OR_NONE)
	{
		if (mParent->getShadowsDepth())
			return Ogre::CULL_NONE;
		else
			return Ogre::CULL_ANTICLOCKWISE;
	}
	return Ogre::CULL_NONE;
}

//----------------------------------------------------------------------------------------

Ogre::String MaterialGenerator::chooseShadowCasterMaterial()
{
	String type = "PSSM/"; if (mParent->getShadowsSoft()) type = "PSVSM/";
	
	
	Ogre::String shadowCasterMaterial = StringUtil::BLANK;
	if(!mDef->mProps->transparent)
	{
		if(mParent->getShadowsDepth())
		{
			Ogre::CullingMode cmode = chooseCullingMode();
			/*if(mShader->wind == 2)
			{
				shadowCasterMaterial = "PSSM/shadow_caster_wind";				
			}
			else*/ if(cmode == Ogre::CULL_NONE)
			{
				shadowCasterMaterial = type+"shadow_caster_nocull";				
			}
			else
			{
				shadowCasterMaterial = type+"shadow_caster_noalpha";
			}
		}
	}
	
	return shadowCasterMaterial;
}

//----------------------------------------------------------------------------------------

