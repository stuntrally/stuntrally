#include "pch.h"
#include "common/Def_Str.h"
#include "BaseApp.h"

#include "../vdrift/pathmanager.h"
#include "../settings.h"

#include "Compositor.h"
#include "SplitScreen.h"
//#include "HDRCompositor.h"

#include <OgreCompositorManager.h>
#include <OgreCompositionTargetPass.h>
#include <OgreCompositionPass.h>
#include <OgreTechnique.h>
#include <OgreCompositor.h>
#include <OgreResourceGroupManager.h>

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
	return pSet->all_effects && (pSet->ssao || pSet->softparticles || pSet->dof || pSet->godrays || pSet->blur);
}


///  Refresh
//-------------------------------------------------------------------------------------
void BaseApp::refreshCompositor(bool disableAll)
{
	return;  //!
	CompositorManager& cmp = CompositorManager::getSingleton();

	for (auto vp : mSplitMgr->mViewports)
	{
	try
	{
		cmp.setCompositorEnabled(vp, "gbuffer", false);

		cmp.setCompositorEnabled(vp, "gbufferNoMRT", false);
		cmp.setCompositorEnabled(vp, "Bloom", false);
		cmp.setCompositorEnabled(vp, "HDR", false);
		cmp.setCompositorEnabled(vp, "HDRNoMRT", false);
			
		//if (MaterialGenerator::MRTSupported())
			cmp.setCompositorEnabled(vp, "ssao", false);
			cmp.setCompositorEnabled(vp, "SoftParticles", false);
			cmp.setCompositorEnabled(vp, "DepthOfField", false);
			cmp.setCompositorEnabled(vp, "GodRays", false);
			cmp.setCompositorEnabled(vp, "gbufferFinalizer", false);

		cmp.setCompositorEnabled(vp, "motionblur", false);
		cmp.setCompositorEnabled(vp, "FilmGrain", false);
		cmp.setCompositorEnabled(vp, "gbufferUIRender", false);
	}catch(...)
	{	LogO("!Warning: Failed to set all compositors false.");  }
	}

	if (!pSet->all_effects || disableAll)
		return;

	//  Set Bloom params (intensity, orig weight)
	try
	{	MaterialPtr bloom = MaterialManager::getSingleton().getByName("Ogre/Compositor/BloomBlend2");
		if (bloom)
		{
			GpuProgramParametersSharedPtr params = bloom->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
			params->setNamedConstant("OriginalImageWeight", pSet->bloom_orig);
			params->setNamedConstant("BlurWeight", pSet->bloom_int);
		}
	}catch(...)
	{	LogO("!Warning: Failed to set bloom shader params.");  }

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


	for (auto vp : mSplitMgr->mViewports)
	{
		//if(MaterialGenerator::MRTSupported())
			//the condition here is any compositor needing the gbuffers like ssao ,soft particles
			cmp.setCompositorEnabled(vp, "gbuffer", NeedMRTBuffer());

		cmp.setCompositorEnabled(vp, "gbufferNoMRT",!NeedMRTBuffer() && AnyEffectEnabled());

		cmp.setCompositorEnabled(vp, "Bloom", pSet->bloom);
		cmp.setCompositorEnabled(vp, "HDR", pSet->hdr && NeedMRTBuffer());
		cmp.setCompositorEnabled(vp, "HDRNoMRT", pSet->hdr && !NeedMRTBuffer());
		cmp.setCompositorEnabled(vp, "motionblur", pSet->blur);
		cmp.setCompositorEnabled(vp, "FilmGrain", pSet->hdr);

		//if(MaterialGenerator::MRTSupported())
			cmp.setCompositorEnabled(vp, "ssao", pSet->ssao);
			cmp.setCompositorEnabled(vp, "SoftParticles", pSet->softparticles);
			cmp.setCompositorEnabled(vp, "DepthOfField", pSet->dof);
			cmp.setCompositorEnabled(vp, "GodRays", pSet->godrays);
			cmp.setCompositorEnabled(vp, "gbufferFinalizer", NeedMRTBuffer() && !pSet->softparticles);

		cmp.setCompositorEnabled(vp, "gbufferUIRender", AnyEffectEnabled());
	}
}

//-------------------------------------------------------------------------------------
void BaseApp::recreateCompositor()
{
	return;  //!
	CompositorManager& cmp = CompositorManager::getSingleton(); 

	if (!pSet->all_effects)  // disable compositor
	{
		try
		{	refreshCompositor();
		}
		catch (InvalidParametersException& e)
		{	// ignore missing compositors
		}
		return;
	}

	//  add when needed
	if (!ResourceGroupManager::getSingleton().resourceGroupExists("Effects"))
	{
		std::string sPath = PATHMANAGER::Data() + "/compositor";
		ResourceGroupManager::getSingleton().addResourceLocation(sPath, "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().addResourceLocation(sPath + "/gbuffer", "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().addResourceLocation(sPath + "/bloom", "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().addResourceLocation(sPath + "/hdr", "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().addResourceLocation(sPath + "/motionblur", "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().addResourceLocation(sPath + "/ssao", "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().addResourceLocation(sPath + "/softparticles", "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().addResourceLocation(sPath + "/dof", "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().addResourceLocation(sPath + "/godrays", "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().addResourceLocation(sPath + "/filmgrain", "FileSystem", "Effects");
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
	if (!mMotionBlurLogic)
	{
		mMotionBlurLogic = new MotionBlurLogic(this);
		cmp.registerCompositorLogic("motionblur", mMotionBlurLogic);
	}

	for (auto vp : mSplitMgr->mViewports)
	{
		// remove old comp. first
		cmp.removeCompositorChain(vp);

		//if (MaterialGenerator::MRTSupported())
			cmp.addCompositor(vp, "gbuffer");

		cmp.addCompositor(vp, "gbufferNoMRT");
		cmp.addCompositor(vp, "HDRNoMRT");
		//if (MaterialGenerator::MRTSupported())
        if (1)
		{
			cmp.addCompositor(vp, "ssao");
			cmp.addCompositor(vp, "SoftParticles");
			cmp.addCompositor(vp, "DepthOfField");
			cmp.addCompositor(vp, "gbufferFinalizer");
			cmp.addCompositor(vp, "HDR");
		}
		else
			cmp.addCompositor(vp, "ssaoNoMRT");

		cmp.addCompositor(vp, "GodRays");
		cmp.addCompositor(vp, "Bloom");
		cmp.addCompositor(vp, "motionblur");
		//cmp.addCompositor(vp, "FXAA");
		cmp.addCompositor(vp, "FilmGrain");
		cmp.addCompositor(vp, "gbufferUIRender");

	}
	refreshCompositor();
}

