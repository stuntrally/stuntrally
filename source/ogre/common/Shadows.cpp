#include "pch.h"
#include "../common/Def_Str.h"
#include "../common/RenderConst.h"
#include "../common/data/SceneXml.h"
#include "../common/QTimer.h"
#ifdef SR_EDITOR
	#include "../../editor/CApp.h"
	#include "../../editor/CGui.h"
	#include "../../editor/settings.h"
	#include "../../road/Road.h"
#else
	#include "../CGame.h"
	#include "../../vdrift/settings.h"
	#include "../../road/Road.h"
	#include "../SplitScreen.h"
#endif
#include "../../paged-geom/PagedGeometry.h"
#include "../../paged-geom/GrassLoader.h"

#include <OgreTerrain.h>
#include <OgreShadowCameraSetupLiSPSM.h>
#include <OgreShadowCameraSetupPSSM.h>
#include <OgreMaterialManager.h>
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreOverlayManager.h>
#include <OgreStaticGeometry.h>

#include "../../shiny/Main/Factory.hpp"
#include "../../shiny/Platforms/Ogre/OgreMaterial.hpp"

using namespace Ogre;
using namespace Forests;


///  Shadows config
//---------------------------------------------------------------------------------------------------
void App::changeShadows()
{	
	QTimer ti;  ti.update();  /// time
	
	//  get settings
	bool enabled = pSet->shadow_type != Sh_None;
	bool bDepth = pSet->shadow_type >= Sh_Depth;
	bool bSoft = pSet->shadow_type == Sh_Soft;
	
	pSet->shadow_size = std::max(0,std::min(ciShadowSizesNum-1, pSet->shadow_size));
	int fTex = ciShadowSizesA[pSet->shadow_size], fTex2 = fTex/2;
	int num = pSet->shadow_count;

	sh::Vector4* fade = new sh::Vector4(
		pSet->shadow_dist,
		pSet->shadow_dist * 0.6, // fade start
		0, 0);

	mFactory->setSharedParameter("shadowFar_fadeStart", sh::makeProperty<sh::Vector4>(fade));

	if (terrain)
	{
		sh::Factory::getInstance().setSharedParameter("terrainWorldSize", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(terrain->getWorldSize())));
		sh::Factory::getInstance().setTextureAlias("TerrainLightMap", terrain->getLightmap()->getName());
	}
		
	// disable 4 shadow textures (does not work because no texcoord's left in shader)
	if (num == 4)  num = 3;


	if (!enabled)  {
		mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);  /*return;*/ //
	}
	else
	{
		// General scene setup
		//mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
		mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED);
		mSceneMgr->setShadowFarDistance(pSet->shadow_dist);  // 3000
		mSceneMgr->setShadowTextureCountPerLightType(Light::LT_DIRECTIONAL, num);

		if (num == 1)  // 1 tex, fast
		{
			ShadowCameraSetupPtr mShadowCameraSetup = ShadowCameraSetupPtr(new LiSPSMShadowCameraSetup());
			mSceneMgr->setShadowCameraSetup(mShadowCameraSetup);
		}else
		{	if (mPSSMSetup.isNull())  // pssm
			{
				PSSMShadowCameraSetup* pssmSetup = new PSSMShadowCameraSetup();
				#ifndef SR_EDITOR
				pssmSetup->setSplitPadding(mSplitMgr->mCameras.front()->getNearClipDistance());
				pssmSetup->calculateSplitPoints(num, mSplitMgr->mCameras.front()->getNearClipDistance(), mSceneMgr->getShadowFarDistance());
				#else
				pssmSetup->setSplitPadding(mCamera->getNearClipDistance());
				pssmSetup->calculateSplitPoints(num, mCamera->getNearClipDistance(), mSceneMgr->getShadowFarDistance());
				#endif
				for (int i=0; i < num; ++i)
				{	//int size = i==0 ? fTex : fTex2;
					const Real cAdjfA[5] = {2, 1, 0.5, 0.25, 0.125};
					pssmSetup->setOptimalAdjustFactor(i, cAdjfA[std::min(i, 4)]);
				}
				mPSSMSetup.bind(pssmSetup);
			}
			mSceneMgr->setShadowCameraSetup(mPSSMSetup);
		}

		mSceneMgr->setShadowTextureCount(num);
		for (int i=0; i < num; ++i)
		{	int size = i==0 ? fTex : fTex2;
		
			PixelFormat pf;
			if (bDepth && !bSoft) pf = PF_FLOAT32_R;
			else if (bSoft) pf = PF_FLOAT16_RGB;
			else pf = PF_X8B8G8R8;
			
			mSceneMgr->setShadowTextureConfig(i, size, size, pf);
		}
		
		mSceneMgr->setShadowTextureSelfShadow(bDepth ? true : false);  //-?
		mSceneMgr->setShadowCasterRenderBackFaces((bDepth && !bSoft) ? true : false);
		
		String shadowCasterMat;
		if (bDepth) shadowCasterMat = "PSSM/shadow_caster";

		else shadowCasterMat = ""; //StringUtil::BLANK;
		
		mSceneMgr->setShadowTextureCasterMaterial(shadowCasterMat);
	}

	mSceneMgr->setShadowColour(Ogre::ColourValue(0,0,0,1));


	mFactory->setGlobalSetting("shadows", "false");
	mFactory->setGlobalSetting("shadows_pssm", b2s(pSet->shadow_type > Sh_None /*&& pSet->shadow_count > 1*/));
	mFactory->setGlobalSetting("shadows_depth", b2s(pSet->shadow_type >= Sh_Depth));
	
	mFactory->setGlobalSetting("terrain_specular", b2s(pSet->ter_mtr >= 1));
	mFactory->setGlobalSetting("terrain_normal",   b2s(pSet->ter_mtr >= 2));
	mFactory->setGlobalSetting("terrain_parallax", b2s(pSet->ter_mtr >= 3));
	mFactory->setGlobalSetting("terrain_triplanarType", toStr(pSet->ter_tripl));
	mFactory->setGlobalSetting("terrain_triplanarLayer", toStr(sc->td.triplanar1Layer));

	mFactory->setGlobalSetting("water_reflect", b2s(pSet->water_reflect));
	mFactory->setGlobalSetting("water_refract", b2s(pSet->water_refract));


#if !SR_EDITOR
	mFactory->setGlobalSetting("soft_particles", b2s(pSet->all_effects && pSet->softparticles));
	mFactory->setGlobalSetting("mrt_output", b2s(NeedMRTBuffer()));
	mFactory->setGlobalSetting("debug_blend", b2s(false));
#else
	mFactory->setGlobalSetting("debug_blend", b2s(gui->bDebugBlend));
#endif

	#if 0
	// shadow tex overlay
	// add the overlay elements to show the shadow maps:
	// init overlay elements
	OverlayManager& mgr = OverlayManager::getSingleton();
	Overlay* overlay;
	
	// destroy if already exists
	if (overlay = mgr.getByName("DebugOverlay"))
		mgr.destroy(overlay);
		
	overlay = mgr.create("DebugOverlay");
	
	TexturePtr tex;
	for (int i = 0; i < pSet->shadow_count; ++i)
	{	
		TexturePtr tex = mSceneMgr->getShadowTexture(i);
		
		// Set up a debug panel to display the shadow
		if (MaterialManager::getSingleton().resourceExists("Ogre/DebugTexture" + toStr(i)))
			MaterialManager::getSingleton().remove("Ogre/DebugTexture" + toStr(i));
		MaterialPtr debugMat = MaterialManager::getSingleton().create(
			"Ogre/DebugTexture" + toStr(i), rgDef);
			
		debugMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
		TextureUnitState *t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState(tex->getName());
		t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);

		OverlayContainer* debugPanel;
		
		// destroy container if exists
		try
		{
			if (debugPanel = static_cast<OverlayContainer*>(mgr.getOverlayElement("Ogre/DebugTexPanel" + toStr(i))))
				mgr.destroyOverlayElement(debugPanel);
		}
		catch (Ogre::Exception&) {}
		
		debugPanel = (OverlayContainer*)
			(OverlayManager::getSingleton().createOverlayElement("Panel", "Ogre/DebugTexPanel" + StringConverter::toString(i)));
		debugPanel->_setPosition(0.8, i*0.31);  //aspect.. 0.25 0.24
		debugPanel->_setDimensions(0.2, 0.3);
		debugPanel->setMaterialName(debugMat->getName());
		debugPanel->show();
		overlay->add2D(debugPanel);
		overlay->show();
	}
	#endif
	
	UpdPSSMMaterials();


	//  rebuild static geom after materials change
	if (mStaticGeom)
	{
		mStaticGeom->destroy();
		mStaticGeom->build();
	}

	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Shadows: ") + fToStr(dt,0,3) + " ms");
}


/// . . . . . . . . 
void App::UpdPSSMMaterials()
{
	if (pSet->shadow_type == Sh_None)  return;
	
	if (pSet->shadow_count == 1)  // 1 tex
	{
		float dist = pSet->shadow_dist;
		sh::Vector3* splits = new sh::Vector3(dist, 0,0);  //dist*2, dist*3);
		sh::Factory::getInstance().setSharedParameter("pssmSplitPoints", sh::makeProperty<sh::Vector3>(splits));
		return;
	}
	
	if (!mPSSMSetup.get())  return;
	
	//--  pssm params
	PSSMShadowCameraSetup* pssmSetup = static_cast<PSSMShadowCameraSetup*>(mPSSMSetup.get());
	const PSSMShadowCameraSetup::SplitPointList& sp = pssmSetup->getSplitPoints();
	const int last = sp.size()-1;

	sh::Vector3* splits = new sh::Vector3(
		sp[std::min(1,last)], sp[std::min(2,last)], sp[std::min(3,last)] );

	sh::Factory::getInstance().setSharedParameter("pssmSplitPoints", sh::makeProperty<sh::Vector3>(splits));
}
