#include "pch.h"
#include "../common/Defines.h"
#include "../common/RenderConst.h"
#include "../common/MaterialGen/TerrainMaterialGen.h"
#include "../common/MaterialGen/MaterialFactory.h"

#ifdef ROAD_EDITOR
	#include "../../editor/OgreApp.h"
	#include "../../editor/settings.h"
	#include "../../road/Road.h"
#else
	#include "../OgreGame.h"
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

#include "../common/QTimer.h"

using namespace Ogre;
using namespace Forests;


///  Shadows config
//---------------------------------------------------------------------------------------------------
void App::changeShadows()
{	
	QTimer ti;  ti.update();  /// time
	
	//  get settings
	bool enabled = pSet->shadow_type != 0;
	bool bDepth = pSet->shadow_type >= 3;
	bool bSoft = pSet->shadow_type == 4;
	
	pSet->shadow_size = std::max(0,std::min(ciShadowNumSizes-1, pSet->shadow_size));
	int fTex = /*2048*/ ciShadowSizesA[pSet->shadow_size], fTex2 = fTex/2;
	int num = /*3*/ pSet->shadow_count;
		
	// disable 4 shadow textures (does not work because no texcoord's left in shader)
	if (num == 4) num = 3;

	TerrainMaterialGeneratorB::SM2Profile* matProfile = 0;
	
	if (mTerrainGlobals)
	{
		matProfile = (TerrainMaterialGeneratorB::SM2Profile*) mTerrainGlobals->getDefaultMaterialGenerator()->getActiveProfile();
		if (matProfile)
		{	matProfile->setReceiveDynamicShadowsEnabled(enabled);
			matProfile->setReceiveDynamicShadowsLowLod(true);
			matProfile->setGlobalColourMapEnabled(false);

			matProfile->setLayerSpecularMappingEnabled(pSet->ter_mtr >= 1);  // ter mtr
			matProfile->setLayerNormalMappingEnabled(  pSet->ter_mtr >= 2);
			matProfile->setLayerParallaxMappingEnabled(pSet->ter_mtr >= 3);
	}	}
	
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
		//return;  // !
	}


	if (!enabled)  {
		mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);  /*return;*/ }

	else
	{
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
			//pssmSetup->setSplitPadding(10);
			pssmSetup->calculateSplitPoints(num, mSplitMgr->mCameras.front()->getNearClipDistance(), mSceneMgr->getShadowFarDistance());
			#else
			pssmSetup->setSplitPadding(mCamera->getNearClipDistance());
			//pssmSetup->setSplitPadding(10);
			pssmSetup->calculateSplitPoints(num, mCamera->getNearClipDistance(), mSceneMgr->getShadowFarDistance());
			#endif
			for (int i=0; i < num; ++i)
			{	//int size = i==0 ? fTex : fTex2;
				const Real cAdjfA[5] = {2, 1, 0.5, 0.25, 0.125};
				pssmSetup->setOptimalAdjustFactor(i, cAdjfA[std::min(i, 4)]);
			}
			materialFactory->setPSSMCameraSetup(pssmSetup);
			mPSSMSetup.bind(pssmSetup);
		}
		mSceneMgr->setShadowCameraSetup(mPSSMSetup);

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
		if (bDepth && !bSoft) shadowCasterMat = "PSSM/shadow_caster";
		else if (bSoft) shadowCasterMat = "PSVSM/shadow_caster";
		else shadowCasterMat = StringUtil::BLANK;
		
		mSceneMgr->setShadowTextureCasterMaterial(shadowCasterMat);

		if (matProfile && terrain)  {
			matProfile->setReceiveDynamicShadowsDepth(bDepth);
			matProfile->setReceiveDynamicShadowsPSSM(static_cast<PSSMShadowCameraSetup*>(mPSSMSetup.get()));
			MaterialPtr mtr = matProfile->generateForCompositeMap(terrain);
			//LogO(mtr->getBestTechnique()->getPass(0)->getTextureUnitState(0)->getName());
			//LogO(String("Ter mtr: ") + mtr->getName());

		}
	}
	
	materialFactory->setTerrain(terrain);
	materialFactory->setNumShadowTex(num);
	materialFactory->setShadows(pSet->shadow_type != 0);
	materialFactory->setShadowsDepth(bDepth);
	materialFactory->setShadowsSoft(bSoft);
	materialFactory->generate();
	
	#if 0	// shadow tex overlay
	// add the overlay elements to show the shadow maps:
	// init overlay elements
	OverlayManager& mgr = OverlayManager::getSingleton();
	Overlay* overlay;
	
	// destroy if already exists
	if (overlay = mgr.getByName("DebugOverlay"))
		mgr.destroy(overlay);
		
	overlay = mgr.create("DebugOverlay");
	
	TexturePtr tex;
	for (size_t i = 0; i < 2; ++i) {
		//TexturePtr tex = mSceneMgr->getShadowTexture(i);
		if (i == 0) tex = TextureManager::getSingleton().getByName("PlaneReflection"); 
		else tex = TextureManager::getSingleton().getByName("PlaneRefraction"); 
		
		// Set up a debug panel to display the shadow
		
		if (MaterialManager::getSingleton().resourceExists("Ogre/DebugTexture" + toStr(i)))
			MaterialManager::getSingleton().remove("Ogre/DebugTexture" + toStr(i));
		MaterialPtr debugMat = MaterialManager::getSingleton().create(
			"Ogre/DebugTexture" + toStr(i), 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			
		debugMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
		TextureUnitState *t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState(tex->getName());
		t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);

		OverlayContainer* debugPanel;
		
		// destroy container if exists
		try
		{
			if (debugPanel = 
				static_cast<OverlayContainer*>(
					mgr.getOverlayElement("Ogre/DebugTexPanel" + toStr(i)
				)))
				mgr.destroyOverlayElement(debugPanel);
		}
		catch (Ogre::Exception&) {}
		
		debugPanel = (OverlayContainer*)
			(OverlayManager::getSingleton().createOverlayElement("Panel", "Ogre/DebugTexPanel" + StringConverter::toString(i)));
		debugPanel->_setPosition(0.8, i*0.25);
		debugPanel->_setDimensions(0.2, 0.24);
		debugPanel->setMaterialName(debugMat->getName());
		debugPanel->show();
		overlay->add2D(debugPanel);
		overlay->show();
	}
	#endif
	
	// -------------------   update the paged-geom materials
	
	// grass is not cloned, just need to set new shader parameters
	if (grass)
	{
		GrassLoader *grassLoader = static_cast<GrassLoader*>(grass->getPageLoader());
		for (std::list<GrassLayer*>::iterator it= grassLoader->getLayerList().begin();
			it != grassLoader->getLayerList().end(); ++it)
		{
			GrassLayer* layer = (*it);
			layer->applyShader();
		}
	}
	
	// trees are more complicated since they are cloned
	//!todo this doesn't work (tree material does not update immediately)
	if(trees)
	{
		trees->reloadGeometry();
		std::vector<ResourcePtr> reosurceToDelete;
		ResourceManager::ResourceMapIterator it = MaterialManager::getSingleton().getResourceIterator();
		while (it.hasMoreElements())
		{
			ResourcePtr material = it.getNext();
			String materialName = material->getName();
			std::string::size_type pos =materialName.find("BatchMat|");
			if( pos != std::string::npos ) {
				reosurceToDelete.push_back(material);
			}
		}
		for(int i=0;i<reosurceToDelete.size();i++)
		{
			MaterialManager::getSingleton().remove(reosurceToDelete[i]);
		}
	}
	UpdPSSMMaterials();

	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Shadows: ") + toStr(dt) + " ms");
}


#ifdef ROAD_EDITOR
void App::createRoadSelMtr(String sMtrName)
{
	MaterialPtr mat = MaterialManager::getSingleton().getByName(sMtrName);
	if (!mat.isNull() && mat->getNumTechniques()>0)
	{
		/*unsigned short np = mat->getTechnique(0)->getNumPasses()-1;  // last  unsigned!
		try {
			if (mat->getTechnique(0)->getPass(np)->hasFragmentProgram() 
				&& mat->getTechnique(0)->getPass(np)->getFragmentProgramParameters()->_findNamedConstantDefinition("pssmSplitPoints",false)
				)
				mat->getTechnique(0)->getPass(np)->getFragmentProgramParameters()->setNamedConstant("pssmSplitPoints", splitPoints);
		} catch(...) { }*/
		
		//  create selected materials for road
		if (StringUtil::startsWith(sMtrName,"road",false) || StringUtil::startsWith(sMtrName,"pipe",false) )
		{
		String selName = sMtrName + "_sel";
		MaterialPtr selMtr = MaterialManager::getSingleton().getByName(selName);
		if (selMtr.isNull())  {  // once
			//LogO("new sel mtr: " +selName);
			MaterialPtr sel = mat->clone(selName);
			Technique* tech = sel->getTechnique(0);  Pass* p = tech->createPass();
			p->setSceneBlending(SBT_ADD);  p->setDepthBias(3.f);//
			p->setAmbient(0,0,0);  p->setDiffuse(0,0,0,0);  p->setSpecular(0,0,0,0);
			p->setDepthCheckEnabled(false);  p->setDepthWriteEnabled(true);
			p->setCullingMode(CULL_NONE);
			p->setFragmentProgram("sel_ps");  //p->setSelfIllumination(0,0.1,0.2);
		}	}
	}
}
#endif

void App::UpdPSSMMaterials()	/// . . . . . . . . 
{
	if (pSet->shadow_type == 0)  return;
	if (!mPSSMSetup.get())  return;
	
	//--  pssm params
	PSSMShadowCameraSetup* pssmSetup = static_cast<PSSMShadowCameraSetup*>(mPSSMSetup.get());
	const PSSMShadowCameraSetup::SplitPointList& splitPointList = pssmSetup->getSplitPoints();

	for (size_t i = 0; i < splitPointList.size(); ++i)
		splitPoints[i] = splitPointList[i];

	// mtr splits for all cars (only game)
	#ifndef ROAD_EDITOR
	//if (pSet->shadow_type == 3) 
	{
		recreateCarMtr();
	}
	#endif
}
