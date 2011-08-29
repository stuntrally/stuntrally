#include "pch.h"
#include "../Defines.h"
#ifdef ROAD_EDITOR
	#include "../../editor/OgreApp.h"
	#include "../../editor/settings.h"
	#include "../../road/Road.h"
	#include "../QTimer.h"
#else
	#include "../OgreGame.h"
	#include "../vdrift/settings.h"
	#include "../road/Road.h"
	#include "../SplitScreen.h"
	#include "../QTimer.h"
#endif
#include <OgreTerrainMaterialGeneratorA.h>
#include <OgreTerrain.h>
#include <OgreShadowCameraSetupLiSPSM.h>
#include <OgreShadowCameraSetupPSSM.h>
#include <OgreMaterialManager.h>
using namespace Ogre;


///  Shadows config
//---------------------------------------------------------------------------------------------------
void App::changeShadows()
{
	QTimer ti;  ti.update();  /// time

	//  get settings
	bool enabled = pSet->shadow_type != 0;
	bool bDepth = pSet->shadow_type == 3;

	pSet->shadow_size = std::max(0,std::min(ciShadowNumSizes-1, pSet->shadow_size));
	int fTex = /*2048*/ ciShadowSizesA[pSet->shadow_size], fTex2 = fTex/2;
	int num = /*3*/ pSet->shadow_count;

	TerrainMaterialGeneratorA::SM2Profile* matProfile = 0;
	if (mTerrainGlobals)
	{
		matProfile = static_cast<TerrainMaterialGeneratorA::SM2Profile*>(
			mTerrainGlobals->getDefaultMaterialGenerator()->getActiveProfile());
		matProfile->setReceiveDynamicShadowsEnabled(enabled);
		matProfile->setReceiveDynamicShadowsLowLod(true);
		matProfile->setGlobalColourMapEnabled(false);

		matProfile->setLayerSpecularMappingEnabled(pSet->ter_mtr >= 1);  // ter mtr
		matProfile->setLayerNormalMappingEnabled(  pSet->ter_mtr >= 2);
		matProfile->setLayerParallaxMappingEnabled(pSet->ter_mtr >= 3);
	}
	
	//  shadows old-
	if (pSet->shadow_type == 1)
	{
		//mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
		mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
		//mSceneMgr->setShadowTextureCount(2);
		//mSceneMgr->setShadowTexturePixelFormat(PF_L8);
		//mSceneMgr->setShadowTextureCasterMaterial(StringUtil::BLANK);
		//mSceneMgr->setShadowTextureSelfShadow(true);
		mSceneMgr->setShadowCasterRenderBackFaces(true);

		mSceneMgr->setShadowTextureSettings(fTex, 1, PF_L8);
		mSceneMgr->setShadowColour(ColourValue(0.4, 0.4, 0.4));
		mSceneMgr->setShadowFarDistance(pSet->shadow_dist / 50.f);  // 36 72
		mSceneMgr->setShadowDirLightTextureOffset(0.5);
		//-ShadowCameraSetupPtr mShadowCameraSetup = ShadowCameraSetupPtr(new PlaneOptimalShadowCameraSetup(mPlane));
			ShadowCameraSetupPtr mShadowCameraSetup = ShadowCameraSetupPtr(new LiSPSMShadowCameraSetup());
			mSceneMgr->setShadowCameraSetup(mShadowCameraSetup);/**/
		mSceneMgr->setShadowTextureCountPerLightType(Light::LT_DIRECTIONAL, 1);
		mSceneMgr->setShadowTextureCount(1);
		return;  // !
	}


	if (!enabled)  {
		mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);  return;  }

	
	// General scene setup
	//mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED);
	mSceneMgr->setShadowFarDistance(pSet->shadow_dist);  // 3000
	mSceneMgr->setShadowTextureCountPerLightType(Light::LT_DIRECTIONAL, num);

	if (mPSSMSetup.isNull())
	{
		// shadow camera setup
		PSSMShadowCameraSetup* pssmSetup = new PSSMShadowCameraSetup();
		#ifndef ROAD_EDITOR
		pssmSetup->setSplitPadding(mSplitMgr->mCameras.front()->getNearClipDistance());
		pssmSetup->calculateSplitPoints(num, mSplitMgr->mCameras.front()->getNearClipDistance(), mSceneMgr->getShadowFarDistance());
		#else
		pssmSetup->setSplitPadding(mCamera->getNearClipDistance());
		pssmSetup->calculateSplitPoints(num, mCamera->getNearClipDistance(), mSceneMgr->getShadowFarDistance());
		#endif
		for (int i=0; i < num; ++i)
		{	int size = i==0 ? fTex : fTex2;
			const Real cAdjfA[5] = {2, 1, 0.5, 0.25, 0.125};
			pssmSetup->setOptimalAdjustFactor(i, cAdjfA[std::min(i, 4)]);
		}
		mPSSMSetup.bind(pssmSetup);
	}
	mSceneMgr->setShadowCameraSetup(mPSSMSetup);

	mSceneMgr->setShadowTextureCount(num);
	for (int i=0; i < num; ++i)
	{	int size = i==0 ? fTex : fTex2;
		mSceneMgr->setShadowTextureConfig(i, size, size, bDepth ? PF_FLOAT32_R : PF_X8B8G8R8);
	}
	mSceneMgr->setShadowTextureSelfShadow(bDepth ? true : false);  //-?
	mSceneMgr->setShadowCasterRenderBackFaces(false/**/);
	mSceneMgr->setShadowTextureCasterMaterial(bDepth ? "PSSM/shadow_caster" : StringUtil::BLANK);

	if (matProfile && terrain)  {
		matProfile->setReceiveDynamicShadowsDepth(bDepth);
		matProfile->setReceiveDynamicShadowsPSSM(static_cast<PSSMShadowCameraSetup*>(mPSSMSetup.get()));
		MaterialPtr mtr = matProfile->generateForCompositeMap(terrain);
		//LogO(mtr->getBestTechnique()->getPass(0)->getTextureUnitState(0)->getName());
		//LogO(String("Ter mtr: ") + mtr->getName());
	}
	UpdPSSMMaterials();

	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Shadows: ") + toStr(dt) + " ms");
}


void App::setMtrSplits(String sMtrName)
{
	MaterialPtr mat = MaterialManager::getSingleton().getByName(sMtrName);
	if (!mat.isNull())
	{
		unsigned short np = mat->getTechnique(0)->getNumPasses()-1;  // last  unsigned!
		try {
			mat->getTechnique(0)->getPass(np)->getFragmentProgramParameters()->setNamedConstant("pssmSplitPoints", splitPoints);
		} catch(...) {  }
		
		#ifdef ROAD_EDITOR
		//  create selected mtr
		String selName = sMtrName + "_sel";
		MaterialPtr selMtr = MaterialManager::getSingleton().getByName(selName);
		if (selMtr.isNull())  {  // once
			LogO("new sel mtr: " +selName);
			MaterialPtr sel = mat->clone(selName);
			Technique* tech = sel->getTechnique(0);  Pass* p = tech->createPass();
			p->setSceneBlending(SBT_ADD);  p->setDepthBias(13.f);//
			p->setAmbient(0,0,0);  p->setDiffuse(0,0,0,0);  p->setSpecular(0,0,0,0);
			p->setDepthCheckEnabled(false);  p->setDepthWriteEnabled(true);
			p->setCullingMode(CULL_NONE);
			p->setFragmentProgram("sel_ps");  //p->setSelfIllumination(0,0.1,0.2);
		}
		#endif
	}
}

void App::UpdPSSMMaterials()	/// . . . . . . . . 
{
	if (pSet->shadow_type == 0)  return;
	if (!mPSSMSetup.get())  return;
	
	//--  pssm params
	PSSMShadowCameraSetup* pssmSetup = static_cast<PSSMShadowCameraSetup*>(mPSSMSetup.get());
	const PSSMShadowCameraSetup::SplitPointList& splitPointList = pssmSetup->getSplitPoints();
	MaterialPtr mat;

	for (size_t i = 0; i < /*3*/splitPointList.size(); ++i)
		splitPoints[i] = splitPointList[i];
	
	//#ifndef ROAD_EDITOR  // GAME
	setMtrSplits("grass_GrassVS_");

	if (!road)  return;
	String txs = road->iTexSize == 0 ? "_s": "";
	for (int i=0; i<MTRs; ++i)
	{
		if (road->sMtrRoad[i] != "") {	setMtrSplits(road->sMtrRoad[i]+txs);  setMtrSplits(road->sMtrRoad[i]+"_ter"+txs);  }
		if (road->sMtrPipe[i] != "")	setMtrSplits(road->sMtrPipe[i]+txs);
	}
}
