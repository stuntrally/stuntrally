#include "pch.h"
#include "common/Defines.h"
#include "BaseApp.h"

#include "../vdrift/pathmanager.h"
#include "../vdrift/settings.h"

#include "Compositor.h"
#include "SplitScreen.h"

#include <OgreRTShaderSystem.h>
#include "common/MaterialGen/MaterialGenerator.h"
using namespace Ogre;

#if OGRE_VERSION_MINOR >= 8
	#define UI_RENDER "gbufferUIRender"
#else
	#define UI_RENDER "gbufferUIRender17"
#endif



/** This class demonstrates basic usage of the RTShader system.
	It sub class the material manager listener class and when a target scheme callback
	is invoked with the shader generator scheme it tries to create an equivalent shader
	based technique based on the default technique of the given material.
*/
Ogre::Technique* MaterialMgrListener::handleSchemeNotFound(unsigned short schemeIndex, 
	const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, 
	const Ogre::Renderable* rend)
{	
	Ogre::Technique* generatedTech = NULL;

	// Case this is the default shader generator scheme.
	if (schemeName == Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
	{
		bool techniqueCreated;

		// Create shader generated technique for this material.
		techniqueCreated = mShaderGenerator->createShaderBasedTechnique(
			originalMaterial->getName(), 
			Ogre::MaterialManager::DEFAULT_SCHEME_NAME, 
			schemeName);	

		// Case technique registration succeeded.
		if (techniqueCreated)
		{
			// Force creating the shaders for the generated technique.
			mShaderGenerator->validateMaterial(schemeName, originalMaterial->getName());
			
			// Grab the generated technique.
			Ogre::Material::TechniqueIterator itTech = originalMaterial->getTechniqueIterator();

			while (itTech.hasMoreElements())
			{
				Ogre::Technique* curTech = itTech.getNext();

				if (curTech->getSchemeName() == schemeName)
				{
					generatedTech = curTech;
					break;
				}
			}				
		}
	}

	return generatedTech;
}



void BaseApp::createViewports()
{
	mSplitMgr->mNumViewports = pSet->gui.local_players;
	mSplitMgr->Align();
}

bool BaseApp::AnyEffectEnabled()
{
	//any new effect need to be added here to have UI Rendered on it
	return pSet->all_effects && (pSet->softparticles || pSet->bloom || pSet->hdr || pSet->motionblur || pSet->ssaa || pSet->ssao || pSet->godrays || pSet->dof || pSet->filmgrain);
}

bool BaseApp::NeedMRTBuffer()
{
	return pSet->all_effects && (pSet->ssao || pSet->softparticles || pSet->dof || pSet->godrays);
}


///  Refresh
//-------------------------------------------------------------------------------------
void BaseApp::refreshCompositor(bool disableAll)
{
	CompositorManager& cmp = CompositorManager::getSingleton();
	
	for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); ++it)
	{
		if(MaterialGenerator::MRTSupported())
		{
			cmp.setCompositorEnabled((*it), "gbuffer", false);
		}
		cmp.setCompositorEnabled((*it), "gbufferNoMRT", false);
		cmp.setCompositorEnabled((*it), "Bloom", false);
		cmp.setCompositorEnabled((*it), "HDR", false);

		if(MaterialGenerator::MRTSupported())
		{
			cmp.setCompositorEnabled((*it), "ssao", false);
			cmp.setCompositorEnabled((*it), "SoftParticles", false);
			cmp.setCompositorEnabled((*it), "DepthOfField", false);
			cmp.setCompositorEnabled((*it), "GodRays", false);
			cmp.setCompositorEnabled((*it), "gbufferFinalizer", false);
		}else{
			cmp.setCompositorEnabled((*it), "ssaoNoMRT", false);
		}
		cmp.setCompositorEnabled((*it), "Motion Blur", false);
		cmp.setCompositorEnabled((*it), "SSAA", false);
		cmp.setCompositorEnabled((*it), "FilmGrain", false);
		cmp.setCompositorEnabled((*it), UI_RENDER, false);
	}

	if (!pSet->all_effects || disableAll)
		return;
	
	//  Set Bloom params (intensity, orig weight)
	try
	{	MaterialPtr bloom = MaterialManager::getSingleton().getByName("Ogre/Compositor/BloomBlend2");
		if(!bloom.isNull())
		{
			GpuProgramParametersSharedPtr params = bloom->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
			params->setNamedConstant("OriginalImageWeight", pSet->bloomorig);
			params->setNamedConstant("BlurWeight", pSet->bloomintensity);
		}
	}catch(...)
	{	LogO("!!! Failed to set bloom shader params.");  }
	
	//  HDR params todo..
	//try
	//{	MaterialPtr hdrmat = MaterialManager::getSingleton().getByName("Ogre/Compositor/BloomBlend2");
	//	GpuProgramParametersSharedPtr params = hdrmat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
	//	params->setNamedConstant("Bloom", pSet->);
	//}catch(...)
	//{	LogO("!!! Failed to set hdr shader params.");  }

	//  Set Motion Blur intens
	//try
	//{	MaterialPtr blur = MaterialManager::getSingleton().getByName("Ogre/Compositor/Combine");
	//	GpuProgramParametersSharedPtr params = blur->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
	//	params->setNamedConstant("blur", pSet->motionblurintensity);
	//}catch(...)
	//{	LogO("!!! Failed to set blur shader params.");  }
	

	for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); ++it)
	{
		if(MaterialGenerator::MRTSupported())
		{
			//the condition here is any compositor needing the gbuffers like ssao ,soft particles
			cmp.setCompositorEnabled((*it), "gbuffer", NeedMRTBuffer());
		}
		cmp.setCompositorEnabled((*it), "gbufferNoMRT",!NeedMRTBuffer() && AnyEffectEnabled());

		cmp.setCompositorEnabled((*it), "Bloom", pSet->bloom);
		cmp.setCompositorEnabled((*it), "HDR", pSet->hdr);
		cmp.setCompositorEnabled((*it), "Motion Blur", pSet->motionblur);
		cmp.setCompositorEnabled((*it), "SSAA", pSet->ssaa);
		cmp.setCompositorEnabled((*it), "FilmGrain", pSet->filmgrain);

		if(MaterialGenerator::MRTSupported())
		{
			cmp.setCompositorEnabled((*it), "ssao", pSet->ssao);
			cmp.setCompositorEnabled((*it), "SoftParticles", pSet->softparticles);
			cmp.setCompositorEnabled((*it), "DepthOfField", pSet->dof);
			cmp.setCompositorEnabled((*it), "GodRays", pSet->godrays);
			cmp.setCompositorEnabled((*it), "gbufferFinalizer", NeedMRTBuffer() && !pSet->softparticles);
		}else{
			cmp.setCompositorEnabled((*it), "ssaoNoMRT", pSet->ssao);
		}

		cmp.setCompositorEnabled((*it), UI_RENDER, AnyEffectEnabled());
	}
}

///  Create
//-------------------------------------------------------------------------------------
void BaseApp::recreateCompositor()
{
	if (!pSet->all_effects)  // disable compositor
	{
		refreshCompositor();
		return;
	}
	
	//  add when needed
	if (!ResourceGroupManager::getSingleton().resourceGroupExists("Effects"))
	{
		std::string sPath = PATHMANAGER::GetDataPath() + "/compositor";
		mRoot->addResourceLocation(sPath, "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/gbuffer", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/bloom", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/hdr", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/motionblur", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/ssaa", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/ssao", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/softparticles", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/dof", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/godrays", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/filmgrain", "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().initialiseResourceGroup("Effects");
	}

	CompositorManager& cmp = CompositorManager::getSingleton();

	// hdr has to be first in the compositor queue
	if (!mHDRLogic) 
	{
		mHDRLogic = new HDRLogic;
		cmp.registerCompositorLogic("HDR", mHDRLogic);
	}
	
	if (!mSSAOLogic) 
	{
		mSSAOLogic = new SSAOLogic();
		mSSAOLogic->setApp(this);
		if(MaterialGenerator::MRTSupported())
		{
			cmp.registerCompositorLogic("ssao", mSSAOLogic);
		}
		else
		{
			cmp.registerCompositorLogic("ssaoNoMRT", mSSAOLogic);
		}

	}
	if (!mGodRaysLogic) 
	{
		mGodRaysLogic = new GodRaysLogic();
		mGodRaysLogic->setApp(this);
		cmp.registerCompositorLogic("GodRays", mGodRaysLogic);
	}
	if (!mSoftParticlesLogic) 
	{
		mSoftParticlesLogic = new SoftParticlesLogic();
		mSoftParticlesLogic->setApp(this);
		cmp.registerCompositorLogic("SoftParticles", mSoftParticlesLogic);
	}
	if (!mDepthOfFieldLogic) 
	{
		mDepthOfFieldLogic = new DepthOfFieldLogic();
		mDepthOfFieldLogic->setApp(this);
		cmp.registerCompositorLogic("DepthOfField", mDepthOfFieldLogic);
	}
	if (!mFilmGrainLogic) 
	{
		mFilmGrainLogic = new FilmGrainLogic();
		mFilmGrainLogic->setApp(this);
		cmp.registerCompositorLogic("FilmGrain", mFilmGrainLogic);
	}
	if (!mGBufferLogic) 
	{
		mGBufferLogic = new GBufferLogic();
		mGBufferLogic->setApp(this);
		cmp.registerCompositorLogic("GBuffer", mGBufferLogic);
	}

	if (cmp.getByName("Motion Blur").isNull())
	{
		// Motion blur has to be created in code
		CompositorPtr comp3 = cmp.create(
			"Motion Blur", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		CompositionTechnique *t = comp3->createTechnique();
		t->setCompositorLogicName("Motion Blur");
		{
			CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("scene");
			def->width = 0;
			def->height = 0;
			def->formatList.push_back(PF_R8G8B8);
		}
		{
			CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("sum");
			def->width = 0;
			def->height = 0;
			def->formatList.push_back(PF_R8G8B8);
		}
		{
			CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("temp");
			def->width = 0;
			def->height = 0;
			def->formatList.push_back(PF_R8G8B8);
		}
		/// Render scene
		{
			CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
			tp->setOutputName("scene");
		}
		/// Initialisation pass for sum texture
		{
			CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
			tp->setOutputName("sum");
			tp->setOnlyInitial(true);
		}
		/// Do the motion blur
		{
			CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			tp->setOutputName("temp");
			{ CompositionPass *pass = tp->createPass();
			pass->setType(CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("Ogre/Compositor/Combine");
			pass->setIdentifier(120);
			pass->setInput(0, "scene");
			pass->setInput(1, "sum");
			}
		}
		/// Copy back sum texture
		{
			CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			tp->setOutputName("sum");
			{ CompositionPass *pass = tp->createPass();
			pass->setType(CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("Ogre/Compositor/Copyback");
			pass->setInput(0, "temp");
			}
		}
		/// Display result
		{
			CompositionTargetPass *tp = t->getOutputTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			{ CompositionPass *pass = tp->createPass();
			pass->setType(CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("Ogre/Compositor/MotionBlur");
			pass->setInput(0, "sum");
			}
		}
	}


	if (!mMotionBlurLogic)
	{
		mMotionBlurLogic = new MotionBlurLogic(this);
		cmp.registerCompositorLogic("Motion Blur", mMotionBlurLogic);
	}


	for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); ++it)
	{
		// remove old comp. first
		cmp.removeCompositorChain( (*it ));
		
		if (MaterialGenerator::MRTSupported())
		{
			cmp.addCompositor((*it), "gbuffer");
		}
		cmp.addCompositor((*it), "gbufferNoMRT");
		cmp.addCompositor((*it), "HDR");
		if (MaterialGenerator::MRTSupported())
		{
			cmp.addCompositor((*it), "ssao");
			cmp.addCompositor((*it), "SoftParticles");
			cmp.addCompositor((*it), "DepthOfField");
			cmp.addCompositor((*it), "gbufferFinalizer");
		}
		else
		{
			cmp.addCompositor((*it), "ssaoNoMRT");
		}
		cmp.addCompositor((*it), "GodRays");
		cmp.addCompositor((*it), "Bloom");
		cmp.addCompositor((*it), "Motion Blur");
		cmp.addCompositor((*it), "SSAA");
		cmp.addCompositor((*it), "FilmGrain");
		cmp.addCompositor((*it), UI_RENDER);
	}
	
	refreshCompositor();
}


//  util
//-------------------------------------------------------------------------------------
void BaseApp::CreateRTfixed()
{
	if (mShaderGenerator != NULL && mRoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_FIXED_FUNCTION) == false)
	{
		// creates shaders for base material BaseWhite using the RTSS
		Ogre::MaterialPtr baseWhite = Ogre::MaterialManager::getSingleton().getByName("BaseWhite", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);				
		baseWhite->setLightingEnabled(false);
		mShaderGenerator->createShaderBasedTechnique(
			"BaseWhite", 
			Ogre::MaterialManager::DEFAULT_SCHEME_NAME, 
			Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);	
		mShaderGenerator->validateMaterial(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME, 
			"BaseWhite");
		baseWhite->getTechnique(0)->getPass(0)->setVertexProgram(
		baseWhite->getTechnique(1)->getPass(0)->getVertexProgram()->getName());
		baseWhite->getTechnique(0)->getPass(0)->setFragmentProgram(
		baseWhite->getTechnique(1)->getPass(0)->getFragmentProgram()->getName());

		// creates shaders for base material BaseWhiteNoLighting using the RTSS
		mShaderGenerator->createShaderBasedTechnique(
			"BaseWhiteNoLighting", 
			Ogre::MaterialManager::DEFAULT_SCHEME_NAME, 
			Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME);	
		mShaderGenerator->validateMaterial(Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME, 
			"BaseWhiteNoLighting");
		Ogre::MaterialPtr baseWhiteNoLighting = Ogre::MaterialManager::getSingleton().getByName("BaseWhiteNoLighting", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
		baseWhiteNoLighting->getTechnique(0)->getPass(0)->setVertexProgram(
		baseWhiteNoLighting->getTechnique(1)->getPass(0)->getVertexProgram()->getName());
		baseWhiteNoLighting->getTechnique(0)->getPass(0)->setFragmentProgram(
		baseWhiteNoLighting->getTechnique(1)->getPass(0)->getFragmentProgram()->getName());
	}
}
