#include "pch.h"
#include "common/Defines.h"
#include "common/RenderConst.h"
#include "SplitScreen.h"

#include "OgreGame.h"
#include "CarModel.h"
#include "../vdrift/settings.h"
#include "../road/Road.h"
#include "MyGUI_PointerManager.h"

#include "../shiny/Main/Factory.hpp"

#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreCamera.h>
#include <OgreRenderWindow.h>
#include <OgreSceneManager.h>
#include <OgreLogManager.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreRTShaderSystem.h>

using namespace Ogre;


SplitScreenManager::SplitScreenManager(Ogre::SceneManager* sceneMgr, Ogre::RenderWindow* window, SETTINGS* set) :
	pApp(0), mGuiViewport(0), mGuiSceneMgr(0),
	mWindow(window), mSceneMgr(sceneMgr), pSet(set)
{
	// Add window listener
	mWindow->addListener(this);
}

SplitScreenManager::~SplitScreenManager()
{
	CleanUp();
	mWindow->removeListener(this);
}

void SplitScreenManager::SetBackground(const Ogre::ColourValue& value)
{
	for (std::list<Ogre::Viewport*>::iterator vpIt=mViewports.begin(); vpIt != mViewports.end(); ++vpIt)
		(*vpIt)->setBackgroundColour(value);
}

void SplitScreenManager::UpdateCamDist()
{
	for (std::list<Ogre::Camera*>::iterator it=mCameras.begin(); it != mCameras.end(); ++it)
		(*it)->setFarClipDistance(pSet->view_distance*1.1f);
}

//  CleanUp
void SplitScreenManager::CleanUp()
{
	for (std::list<Ogre::Viewport*>::iterator vpIt=mViewports.begin(); vpIt != mViewports.end(); ++vpIt)
	{
		CompositorManager::getSingleton().removeCompositorChain(*vpIt);
		mWindow->removeViewport( (*vpIt)->getZOrder() );
	}
	mViewports.clear();
	
	for (std::list<Ogre::Camera*>::iterator it=mCameras.begin(); it != mCameras.end(); ++it)
		mSceneMgr->destroyCamera(*it);
	mCameras.clear();
}


///  Align
//------------------------------------------------------------------------------------------------------------------
void SplitScreenManager::Align()
{
	CleanUp();
	LogO("-- Screen Align");
	
	for (int i=0; i < 4; ++i)
		mDims[i].Default();

	//  Create the viewports (sets of 3d render & hud viewports) based on mNumViewports = numPlayers
	for (int i=0; i < mNumViewports; i++)
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
			LogO("FATAL ERROR: Unsupported number of viewports: " + toStr(mNumViewports));
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
	for (int i=mNumViewports; i < 4; i++)
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
		mGuiSceneMgr = Ogre::Root::getSingleton().createSceneManager(ST_GENERIC);
		Ogre::Camera* guiCam = mGuiSceneMgr->createCamera("GuiCam1");
		mGuiViewport = mWindow->addViewport(guiCam, 100);
		mGuiViewport->setVisibilityMask(RV_Hud);

		Ogre::RTShader::ShaderGenerator *mShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
		if(mShaderGenerator != NULL)
		{
			mShaderGenerator->addSceneManager(mSceneMgr);
		}
	}
	
	AdjustRatio();
	
	// Add compositing filters for the new viewports
	if (pApp)  pApp->recreateCompositor();
}


void SplitScreenManager::AdjustRatio()
{
	// Go through all viewports & cameras and adjust camera aspect ratio so that it fits to the viewport.
	std::list<Ogre::Camera*>::iterator camIt = mCameras.begin();
	for (std::list<Ogre::Viewport*>::iterator vpIt = mViewports.begin(); vpIt != mViewports.end(); ++vpIt)
	{
		(*camIt)->setAspectRatio( float((*vpIt)->getActualWidth()) / float((*vpIt)->getActualHeight()) );
		++camIt;
	}
}


///  pre viewport update
//------------------------------------------------------------------------------------------------------------------
void SplitScreenManager::preViewportUpdate(const Ogre::RenderTargetViewportEvent& evt)
{
	if (!pApp || pApp->bLoading)  return;

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
		pApp->ShowHUDvp(true);
		pApp->UpdateHUD(carId, 1.f / mWindow->getLastFPS());


		///  Set skybox pos to camera  - TODO: fix, sky is center only for last player ...
		//  idea: with compositor this needs separate sky nodes (own sky for each player) and showing 1 sky for 1 player
		if (pApp->ndSky)
			pApp->ndSky->setPosition(evt.source->getCamera()->getPosition());
			

		//  road lod for each viewport
		if (mNumViewports > 1)
		if (pApp->road)
		{
			pApp->road->mCamera = evt.source->getCamera();
			pApp->road->UpdLodVis(pSet->road_dist);
		}
		
		//  Update rain/snow - depends on camera
		//  todo: every player/viewport needs own weather particles  pr[carId]
		if (pSet->particles)
		{	
			const Vector3& pos = evt.source->getCamera()->getPosition();
			static Vector3 oldPos = Vector3::ZERO;
			Vector3 vel = (pos-oldPos)/ (1.0f / mWindow->getLastFPS());  oldPos = pos;
			Vector3 dir = evt.source->getCamera()->getDirection();//, up = mCamera->getUp();
			Vector3 par = pos + dir * 12 + vel * 0.4;
			if (pApp->pr && pApp->sc.rainEmit > 0)
			{
				ParticleEmitter* pe = pApp->pr->getEmitter(0);
				pe->setPosition(par);
				pe->setEmissionRate(pApp->sc.rainEmit);
			}
			if (pApp->pr2 && pApp->sc.rain2Emit > 0)
			{
				ParticleEmitter* pe = pApp->pr2->getEmitter(0);
				pe->setPosition(par);	//pe->setDirection(-up);
				pe->setEmissionRate(pApp->sc.rain2Emit);
			}
		}

		//update soft particle Depth Target

		if(pApp->pSet->softparticles && pApp->pSet->all_effects)
		{
			Ogre::CompositorInstance  *compositor= Ogre::CompositorManager::getSingleton().getCompositorChain(evt.source)->getCompositor("gbuffer");
			if(compositor!=NULL)
			{
				Ogre::TexturePtr depthTexture =	compositor->getTextureInstance("mrt_output",2);
				if(!depthTexture.isNull())
				{
					sh::Factory::getInstance ().setTextureAlias ("SceneDepth", depthTexture->getName());
				}
			}
		}
	}
	else
	{
		//  Gui viewport - hide stuff we don't want
		pApp->UpdateHUD(-1, 1.f / mWindow->getLastFPS());
		pApp->ShowHUDvp(false);
		
		// no mouse in key capture mode
		//if (pApp->bAssignKey)  hideMouse();
	}
}


void SplitScreenManager::postViewportUpdate(const Ogre::RenderTargetViewportEvent& evt)
{

}
