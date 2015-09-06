#include "pch.h"
#include "../vdrift/par.h"
#include "common/Def_Str.h"
#include "common/RenderConst.h"
#include "common/data/SceneXml.h"
#include "common/CScene.h"
#include "../vdrift/settings.h"
#include "SplitScreen.h"
#include "CGame.h"
#include "CHud.h"
#include "CarModel.h"
#include "../road/Road.h"
#include "../vdrift/car.h"
#include "../shiny/Main/Factory.hpp"

#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreCamera.h>
#include <OgreRenderWindow.h>
#include <OgreSceneManager.h>
#include <OgreLogManager.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreCompositorManager.h>
#include <OgreCompositorChain.h>
#include <OgreSceneNode.h>
#include "MyGUI_PointerManager.h"
using namespace Ogre;


SplitScr::SplitScr(SceneManager* sceneMgr, RenderWindow* window, SETTINGS* set) :
	pApp(0), mGuiViewport(0), mGuiSceneMgr(0),
	mWindow(window), mSceneMgr(sceneMgr), pSet(set)
{
	// Add window listener
	mWindow->addListener(this);
}

SplitScr::~SplitScr()
{
	CleanUp();
	mWindow->removeListener(this);
}

void SplitScr::SetBackground(const ColourValue& value)
{
	for (std::list<Viewport*>::iterator vpIt=mViewports.begin(); vpIt != mViewports.end(); ++vpIt)
		(*vpIt)->setBackgroundColour(value);
}

void SplitScr::UpdateCamDist()
{
	for (std::list<Camera*>::iterator it=mCameras.begin(); it != mCameras.end(); ++it)
		(*it)->setFarClipDistance(pSet->view_distance*1.1f);
}

//  CleanUp
void SplitScr::CleanUp()
{
	for (std::list<Viewport*>::iterator vpIt=mViewports.begin(); vpIt != mViewports.end(); ++vpIt)
	{
		CompositorManager::getSingleton().removeCompositorChain(*vpIt);
		mWindow->removeViewport( (*vpIt)->getZOrder() );
	}
	mViewports.clear();
	
	for (std::list<Camera*>::iterator it=mCameras.begin(); it != mCameras.end(); ++it)
		mSceneMgr->destroyCamera(*it);
	mCameras.clear();
}


///  Align
//------------------------------------------------------------------------------------------------------------------
void SplitScr::Align()
{
	CleanUp();
	LogO("-- Screen Align");
	
	for (int i=0; i < 4; ++i)
		mDims[i].Default();

	//  Create the viewports (sets of 3d render & hud viewports) based on mNumViewports = numPlayers
	for (int i=0; i < mNumViewports; ++i)
	{
		//  set dimensions for the viewports
		float dims[4];  // left,top, width,height
		#define dim_(l,t,w,h)  {  dims[0]=l;  dims[1]=t;  dims[2]=w;  dims[3]=h;  }
		
		if (mNumViewports == 1)
		{
			dim_(0.0, 0.0, 1.0, 1.0);
		}
		else if (mNumViewports == 2)
		{
			if (!pSet->split_vertically)
			{	if (i == 0)	dim_(0.0, 0.0, 1.0, 0.5)
				else		dim_(0.0, 0.5, 1.0, 0.5)
			}else{
				if (i == 0) dim_(0.0, 0.0, 0.5, 1.0)
				else		dim_(0.5, 0.0, 0.5, 1.0)	}
		}
		else if (mNumViewports == 3)
		{
			if (!pSet->split_vertically)
			{
				if (i == 0)			dim_(0.0, 0.0, 0.5, 0.5)
				else if (i == 1)	dim_(0.5, 0.0, 0.5, 0.5)
				else if (i == 2)	dim_(0.0, 0.5, 1.0, 0.5)
			}else{
				if (i == 0)			dim_(0.0, 0.0, 0.5, 1.0)
				else if (i == 1)	dim_(0.5, 0.0, 0.5, 0.5)
				else if (i == 2)	dim_(0.5, 0.5, 0.5, 0.5)
			}
		}
		else if (mNumViewports == 4)
		{
			if (i == 0)			dim_(0.0, 0.0, 0.5, 0.5)
			else if (i == 1)	dim_(0.5, 0.0, 0.5, 0.5)
			else if (i == 2)	dim_(0.0, 0.5, 0.5, 0.5)
			else if (i == 3)	dim_(0.5, 0.5, 0.5, 0.5)
		}
		else
		{
			LogO("ERROR: Unsupported number of viewports: " + toStr(mNumViewports));
			return;
		}
		#undef dim_

		// save dims (for later use by Hud)
		for (int d=0; d<4; ++d)
		{
			mDims[i].left = dims[0]*2-1;  mDims[i].top = dims[1]*2-1;
			mDims[i].width = dims[2]*2;  mDims[i].height = dims[3]*2;
			mDims[i].right = mDims[i].left + mDims[i].width;
			mDims[i].bottom = mDims[i].top + mDims[i].height;
			mDims[i].avgsize = (mDims[i].width + mDims[i].height) * 0.25f;
		}

		// Create camera
		mCameras.push_back(mSceneMgr->createCamera("PlayerCamera" + toStr(i)));
		mCameras.back()->setPosition(Vector3(0,-100,0));
		mCameras.back()->lookAt(Vector3(0,-100,10));
		mCameras.back()->setFarClipDistance(pSet->view_distance*1.1f);
		mCameras.back()->setNearClipDistance(0.2f);
		
		// Create viewport, use i as Z order
		mViewports.push_back(mWindow->addViewport( mCameras.back(), i+5, dims[0], dims[1], dims[2], dims[3]));
	}
	//  always create for 4 cars (replay offset camera view)
	for (int i=mNumViewports; i < 4; ++i)
	{
		mCameras.push_back(mSceneMgr->createCamera("PlayerCamera" + toStr(i)));
		mCameras.back()->setPosition(Vector3(0,-100,0));
		mCameras.back()->lookAt(Vector3(0,-100,10));
		mCameras.back()->setFarClipDistance(pSet->view_distance*1.1f);
		mCameras.back()->setNearClipDistance(0.2f);
	}
		
	// Create gui viewport if not already existing
	if (!mGuiViewport)
	{
		mGuiSceneMgr = Root::getSingleton().createSceneManager(ST_GENERIC);
		Camera* guiCam = mGuiSceneMgr->createCamera("GuiCam1");
		mGuiViewport = mWindow->addViewport(guiCam, 100);
		mGuiViewport->setVisibilityMask(RV_Hud);
	}
	
	AdjustRatio();
	
	// Add compositing filters for the new viewports
	if (pApp)  pApp->recreateCompositor();
}


void SplitScr::AdjustRatio()
{
	// Go through all viewports & cameras and adjust camera aspect ratio so that it fits to the viewport.
	std::list<Camera*>::iterator camIt = mCameras.begin();
	for (std::list<Viewport*>::iterator vpIt = mViewports.begin(); vpIt != mViewports.end(); ++vpIt)
	{
		(*camIt)->setAspectRatio( float((*vpIt)->getActualWidth()) / float((*vpIt)->getActualHeight()) );
		++camIt;
	}
}


///  pre viewport update
//------------------------------------------------------------------------------------------------------------------
void SplitScr::preViewportUpdate(const RenderTargetViewportEvent& evt)
{
	if (!pApp || pApp->bLoading || pApp->iLoad1stFrames > -1)  return;

	//  What kind of viewport is being updated?
	const String& vpName = evt.source->getCamera()->getName();
	//*H*/LogO(vpName);  //GuiCam1  PlayerCamera0,1..
	
	if (evt.source != mGuiViewport)
	{
		//  scene viewport
		//  get car for this viewport
		int carId = 0;
		sscanf(vpName.c_str(), "PlayerCamera%d", &carId);

		//  Update HUD for this car
		pApp->hud->ShowVp(true);
		pApp->hud->Update(carId, 1.f / mWindow->getLastFPS());

		///  Set sky pos to camera  - TODO: fix, sky is center only for last player ...
		//  idea: with compositor this needs separate sky nodes (own sky for each player) and showing 1 sky for 1 player
		if (pApp->ndSky)
			pApp->ndSky->setPosition(evt.source->getCamera()->getPosition());
			

		//  road lod for each viewport
		if (mNumViewports > 1)
		if (pApp->scn->road)
		{
			pApp->scn->road->mCamera = evt.source->getCamera();
			pApp->scn->road->UpdLodVis(pSet->road_dist);
		}
		
		//  Update rain/snow - depends on camera
		//  todo: every player/viewport needs own weather particles  pr[carId]
		if (pSet->particles)
			pApp->scn->UpdateWeather(evt.source->getCamera());

		int s = pApp->carModels.size();
		if (carId < s)
		{
			CarModel* cm = pApp->carModels[carId];
			//  split screen, show beam for cur car's viewport only
			if (pSet->check_beam && !pApp->bHideHudBeam && pApp->pSet->game.local_players > 1)
			{
				for (int i=0; i < s; ++i)
				{	CarModel* c = pApp->carModels[i];
					if (c->ndNextChk)
						c->ndNextChk->setVisible(i==carId);
			}	}
				
			//  change FOV when boosting
			if (pApp->pSet->boost_fov)
			{
				CAR* pCar = cm->pCar;
				if (pCar)
				{
					float fov = pSet->fov_min + (pSet->fov_max - pSet->fov_min) * pCar->dynamics.fBoostFov;
					evt.source->getCamera()->setFOVy(Degree(0.5f*fov));
				}
			}else
				evt.source->getCamera()->setFOVy(Degree(0.5f*pSet->fov_min));
		}

		//  update soft particle Depth Target
		if (pApp->pSet->softparticles && pApp->pSet->all_effects)
		{
			CompositorInstance  *compositor = CompositorManager::getSingleton().getCompositorChain(evt.source)->getCompositor("gbuffer");
			if (compositor!=NULL)
			{
				TexturePtr depthTexture =	compositor->getTextureInstance("mrt_output",2);
				if (!depthTexture.isNull())
					sh::Factory::getInstance().setTextureAlias("SceneDepth", depthTexture->getName());
			}
		}
	}
	else
	{
		//  Gui viewport - hide stuff we don't want
		pApp->hud->Update(-1, 1.f / mWindow->getLastFPS());
		pApp->hud->ShowVp(false);
		
		// no mouse in key capture mode
		//if (pApp->bAssignKey)  hideMouse();
	}
}


void SplitScr::postViewportUpdate(const RenderTargetViewportEvent& evt)
{

}
