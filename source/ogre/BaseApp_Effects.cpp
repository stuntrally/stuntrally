#include "pch.h"
#include "common/Def_Str.h"
#include "BaseApp.h"

#include "../vdrift/pathmanager.h"
#include "../vdrift/settings.h"

#include "Compositor.h"
#include "SplitScreen.h"
//#include "HDRCompositor.h"

#include <OgreRoot.h>
#include <OgreCompositorManager.h>
#include <OgreCompositionTargetPass.h>
#include <OgreCompositionPass.h>
using namespace Ogre;


void BaseApp::createViewports()
{
	mSplitMgr->mNumViewports = pSet->gui.local_players;
	mSplitMgr->Align();
}

bool BaseApp::AnyEffectEnabled()
{
	//any new effect need to be added here to have UI Rendered on it
	return pSet->all_effects && (pSet->softparticles || /*?*/pSet->bloom || pSet->hdr || pSet->blur || pSet->ssao || pSet->godrays || pSet->dof || pSet->filmgrain);
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
		cmp.setCompositorEnabled((*it), "gbuffer", false);

		cmp.setCompositorEnabled((*it), "gbufferNoMRT", false);
		cmp.setCompositorEnabled((*it), "Bloom", false);
		cmp.setCompositorEnabled((*it), "HDR", false);
		cmp.setCompositorEnabled((*it), "HDRNoMRT", false);
			
		//if (MaterialGenerator::MRTSupported())
			cmp.setCompositorEnabled((*it), "ssao", false);
			cmp.setCompositorEnabled((*it), "SoftParticles", false);
			cmp.setCompositorEnabled((*it), "DepthOfField", false);
			cmp.setCompositorEnabled((*it), "GodRays", false);
			cmp.setCompositorEnabled((*it), "gbufferFinalizer", false);
			//cmp.setCompositorEnabled((*it), "CamBlur", false);

		cmp.setCompositorEnabled((*it), "Motion Blur", false);
		cmp.setCompositorEnabled((*it), "FilmGrain", false);
		cmp.setCompositorEnabled((*it), "gbufferUIRender", false);
	}

	if (!pSet->all_effects || disableAll)
		return;

	//  Set Bloom params (intensity, orig weight)
	try
	{	MaterialPtr bloom = MaterialManager::getSingleton().getByName("Ogre/Compositor/BloomBlend2");
	if(!bloom.isNull())
	{
		GpuProgramParametersSharedPtr params = bloom->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
		params->setNamedConstant("OriginalImageWeight", pSet->bloom_orig);
		params->setNamedConstant("BlurWeight", pSet->bloom_int);
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
		//if(MaterialGenerator::MRTSupported())
			//the condition here is any compositor needing the gbuffers like ssao ,soft particles
			cmp.setCompositorEnabled((*it), "gbuffer", NeedMRTBuffer());

		cmp.setCompositorEnabled((*it), "gbufferNoMRT",!NeedMRTBuffer() && AnyEffectEnabled());

		cmp.setCompositorEnabled((*it), "Bloom", pSet->bloom);
		cmp.setCompositorEnabled((*it), "HDR", pSet->hdr && NeedMRTBuffer());
		cmp.setCompositorEnabled((*it), "HDRNoMRT", pSet->hdr && !NeedMRTBuffer());
		cmp.setCompositorEnabled((*it), "Motion Blur", pSet->blur);
		//cmp.setCompositorEnabled((*it), "CamBlur", pSet->camblur);
		cmp.setCompositorEnabled((*it), "FilmGrain", pSet->hdr);

		//if(MaterialGenerator::MRTSupported())
			cmp.setCompositorEnabled((*it), "ssao", pSet->ssao);
			cmp.setCompositorEnabled((*it), "SoftParticles", pSet->softparticles);
			cmp.setCompositorEnabled((*it), "DepthOfField", pSet->dof);
			cmp.setCompositorEnabled((*it), "GodRays", pSet->godrays);
			cmp.setCompositorEnabled((*it), "gbufferFinalizer", NeedMRTBuffer() && !pSet->softparticles);

		cmp.setCompositorEnabled((*it), "gbufferUIRender", AnyEffectEnabled());
	}
}

//-------------------------------------------------------------------------------------
void BaseApp::recreateCompositor()
{
	CompositorManager& cmp = CompositorManager::getSingleton(); 

	if (!pSet->all_effects)  // disable compositor
	{
		refreshCompositor();
		return;
	}

	//  add when needed
	if (!ResourceGroupManager::getSingleton().resourceGroupExists("Effects"))
	{
		std::string sPath = PATHMANAGER::Data() + "/compositor";
		mRoot->addResourceLocation(sPath, "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/gbuffer", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/bloom", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/hdr", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/motionblur", "FileSystem", "Effects");
		//mRoot->addResourceLocation(sPath + "/camblur", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/ssao", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/softparticles", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/dof", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/godrays", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/filmgrain", "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().initialiseResourceGroup("Effects");
	}

	// hdr has to be first in the compositor queue
	if (!mHDRLogic) 
	{
		mHDRLogic = new HDRLogic;

		cmp.registerCompositorLogic("HDR", mHDRLogic);
		mHDRLogic->setApp(this);	
	}

	if (!mSSAOLogic) 
	{
		mSSAOLogic = new SSAOLogic();
		mSSAOLogic->setApp(this);
		//if(MaterialGenerator::MRTSupported())
        if (1)
			CompositorManager::getSingleton().registerCompositorLogic("ssao", mSSAOLogic);
		else
			cmp.registerCompositorLogic("ssaoNoMRT", mSSAOLogic);
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
	/*if (!mCameraBlurLogic)
	{
		mCameraBlurLogic = new CameraBlurLogic(this);
		cmp.registerCompositorLogic("CamBlur", mCameraBlurLogic);
	}*/


	for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); ++it)
	{
		// remove old comp. first
		cmp.removeCompositorChain( (*it ));

		//if (MaterialGenerator::MRTSupported())
			cmp.addCompositor((*it), "gbuffer");

		cmp.addCompositor((*it), "gbufferNoMRT");
		cmp.addCompositor((*it), "HDRNoMRT");
		//if (MaterialGenerator::MRTSupported())
        if (1)
		{
			cmp.addCompositor((*it), "ssao");
			cmp.addCompositor((*it), "SoftParticles");
			cmp.addCompositor((*it), "DepthOfField");
			cmp.addCompositor((*it), "gbufferFinalizer");
			cmp.addCompositor((*it), "HDR");
		}
		else
			cmp.addCompositor((*it), "ssaoNoMRT");

		cmp.addCompositor((*it), "GodRays");
		cmp.addCompositor((*it), "Bloom");
		//cmp.addCompositor((*it), "CamBlur");
		cmp.addCompositor((*it), "Motion Blur");
		//cmp.addCompositor((*it), "FXAA");
		cmp.addCompositor((*it), "FilmGrain");
		cmp.addCompositor((*it), "gbufferUIRender");

	}
	refreshCompositor();
}

