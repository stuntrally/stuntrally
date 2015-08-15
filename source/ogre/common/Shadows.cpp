#include "pch.h"
#include "../common/Def_Str.h"
#include "../common/RenderConst.h"
#include "../common/data/SceneXml.h"
#include "../common/CScene.h"
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

#include <OgreTimer.h>
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



///  Update shader params
//---------------------------------------------------------------------------------------------------
void CScene::UpdShaderParams()
{
	//  get settings
	const SETTINGS* pSet = app->pSet;
	sh::Factory* mFactory = app->mFactory;
	
	sh::Vector4* fade = new sh::Vector4(
		pSet->shadow_dist,
		pSet->shadow_dist * 0.6, // fade start
		0, 0);

	mFactory->setSharedParameter("shadowFar_fadeStart", sh::makeProperty<sh::Vector4>(fade));

	mFactory->setGlobalSetting("shadows", b2s(pSet->shadow_type > Sh_None));
	mFactory->setGlobalSetting("shadows_pssm", b2s(pSet->shadow_type > Sh_None /*&& pSet->shadow_count > 1*/));
	mFactory->setGlobalSetting("shadows_depth", b2s(pSet->shadow_type >= Sh_Depth));


	mFactory->setSharedParameter("terrainWorldSize", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(sc->td.fTerWorldSize/*terrain->getWorldSize()*/)));

	mFactory->setSharedParameter("ter_scaleNormal", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(1.f / sc->td.normScale)));
	mFactory->setSharedParameter("ter_specular_pow", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(sc->td.specularPow)));
	mFactory->setSharedParameter("ter_specular_pow_em", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(sc->td.specularPowEm)));
	
	mFactory->setGlobalSetting("terrain_specular", b2s(pSet->ter_mtr >= 1));
	mFactory->setGlobalSetting("terrain_normal",   b2s(pSet->ter_mtr >= 2));
	mFactory->setGlobalSetting("terrain_parallax", b2s(pSet->ter_mtr >= 3));

	mFactory->setGlobalSetting("terrain_triplanarType", toStr(pSet->ter_tripl));
	mFactory->setGlobalSetting("terrain_triplanarLayer", toStr(sc->td.triplanarLayer1));
	mFactory->setGlobalSetting("terrain_triplanarLayer2", toStr(sc->td.triplanarLayer2));
	mFactory->setGlobalSetting("terrain_emissive_specular", b2s(sc->td.emissive));


	mFactory->setGlobalSetting("water_reflect", b2s(pSet->water_reflect));
	mFactory->setGlobalSetting("water_refract", b2s(pSet->water_refract));


#if !SR_EDITOR
	mFactory->setGlobalSetting("soft_particles", b2s(pSet->all_effects && pSet->softparticles));
	mFactory->setGlobalSetting("mrt_output", b2s(app->NeedMRTBuffer()));
	mFactory->setGlobalSetting("debug_blend", b2s(false));
#else
	mFactory->setGlobalSetting("debug_blend", b2s(app->gui->bDebugBlend));
#endif

}


///  Shadows config
//---------------------------------------------------------------------------------------------------
void CScene::changeShadows()
{	
	Ogre::Timer ti;
	
	//  get settings
	SETTINGS* pSet = app->pSet;
	bool enabled = pSet->shadow_type != Sh_None;
	bool bDepth = pSet->shadow_type >= Sh_Depth;
	bool bSoft = pSet->shadow_type == Sh_Soft;
	
	pSet->shadow_size = std::max(0,std::min(ciShadowSizesNum-1, pSet->shadow_size));
	int fTex = ciShadowSizesA[pSet->shadow_size], fTex2 = fTex/2;
	int num = pSet->shadow_count;


	sh::Factory* mFactory = app->mFactory;
	SceneManager* mSceneMgr = app->mSceneMgr;

	if (terrain)
		mFactory->setTextureAlias("TerrainLightMap", terrain->getLightmap()->getName());

		
	// disable 4 shadow textures (does not work because no texcoord's left in shader)
	if (num == 4)  num = 3;


	if (!enabled)
		mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);
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
				pssmSetup->setSplitPadding(app->mSplitMgr->mCameras.front()->getNearClipDistance());
				pssmSetup->calculateSplitPoints(num, app->mSplitMgr->mCameras.front()->getNearClipDistance(), mSceneMgr->getShadowFarDistance());
				#else
				pssmSetup->setSplitPadding(app->mCamera->getNearClipDistance());
				pssmSetup->calculateSplitPoints(num, app->mCamera->getNearClipDistance(), app->mSceneMgr->getShadowFarDistance());
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

		mSceneMgr->setShadowTextureCasterMaterial(bDepth ? "shadowcaster_default" : "");
	}

	mSceneMgr->setShadowColour(ColourValue(0,0,0,1));


#if 0  /// TEST overlays
	//  add overlay elements to show shadow or terrain maps
	OverlayManager& mgr = OverlayManager::getSingleton();
	Overlay* overlay = mgr.getByName("DebugOverlay");
	if (overlay)
		mgr.destroy(overlay);
	overlay = mgr.create("DebugOverlay");
	TexturePtr tex;

	#if 0  /// shadow
	for (int i = 0; i < pSet->shadow_count; ++i)
	{	
		TexturePtr tex = mSceneMgr->getShadowTexture(i);
	#else  /// terrain
	for (int i = 0; i < 2/*pSet->shadow_count*/; ++i)
	{	
		TexturePtr tex = !terrain ? mSceneMgr->getShadowTexture(i) :
			i==0 ? terrain->getCompositeMap() : terrain->getLightmap();
	#endif
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
	if (vdrTrack)
	{
		vdrTrack->destroy();
		vdrTrack->build();
	}

	LogO(String("::: Time Shadows: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


/// . . . . . . . . 
void CScene::UpdPSSMMaterials()
{
	if (app->pSet->shadow_type == Sh_None)  return;
	
	if (app->pSet->shadow_count == 1)  // 1 tex
	{
		float dist = app->pSet->shadow_dist;
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
