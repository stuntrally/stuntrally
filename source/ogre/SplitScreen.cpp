#include "pch.h"
#include "Defines.h"
#include "SplitScreen.h"

#include "OgreGame.h"
#include "CarModel.h"
#include "../vdrift/settings.h"
#include "../road/Road.h"
#include "MyGUI_PointerManager.h"

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
	pApp(0), mGuiViewport(0), mGuiSceneMgr(0), mHUDSceneMgr(0)
{
	mWindow = window;
	mSceneMgr = sceneMgr;
	pSet = set;
	
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
	for (std::list<Ogre::Viewport*>::iterator vpIt=mViewports.begin(); vpIt != mViewports.end(); vpIt++)
		(*vpIt)->setBackgroundColour(value);
}

void SplitScreenManager::UpdateCamDist()
{
	for (std::list<Ogre::Camera*>::iterator it=mCameras.begin(); it != mCameras.end(); it++)
		(*it)->setFarClipDistance(pSet->view_distance*1.1f);
}

//  CleanUp
void SplitScreenManager::CleanUp()
{
	for (std::list<Ogre::Viewport*>::iterator vpIt=mViewports.begin(); vpIt != mViewports.end(); vpIt++)
		mWindow->removeViewport( (*vpIt)->getZOrder() );
	mViewports.clear();
	
	for (std::list<Ogre::Viewport*>::iterator vpIt=mHUDViewports.begin(); vpIt != mHUDViewports.end(); vpIt++)
		mWindow->removeViewport( (*vpIt)->getZOrder() );
	mHUDViewports.clear();

	for (std::list<Ogre::Camera*>::iterator it=mCameras.begin(); it != mCameras.end(); it++)
		mSceneMgr->destroyCamera(*it);
	mCameras.clear();
}


///  Align
//------------------------------------------------------------------------------------------------------------------
void SplitScreenManager::Align()
{
	CleanUp();
	
	//  Create HUD scene manager & camera if they dont exist already
	if (!mHUDSceneMgr)
	{
		mHUDSceneMgr = Root::getSingleton().createSceneManager(ST_GENERIC);
		mHUDCamera = mHUDSceneMgr->createCamera("HUDCamera"); //!todo destroy
	}

	//  Create the viewports (sets of 3d render & hud viewports) based on mNumViewports = numPlayers
	for (int i=0; i < mNumViewports; i++)
	{
		//  set dimensions for the viewports
		float dims[4];
		
		#define dim_(l,t,w,h)  dims[0]=l;  dims[1]=t;  dims[2]=w;  dims[3]=h
		
		if (mNumViewports == 1)
		{
			dim_(0.0, 0.0, 1.0, 1.0);
		}
		else if (mNumViewports == 2)
		{
			if (!pSet->split_vertically)
			{	if (i == 0)		{	dim_(0.0, 0.0, 1.0, 0.5);	}
				else if (i == 1){	dim_(0.0, 0.5, 1.0, 0.5);	}
			}else{
				if (i == 0) {		dim_(0.0, 0.0, 0.5, 1.0);	}
				else if (i == 1) {	dim_(0.5, 0.0, 0.5, 1.0);	}	}
		}
		else if (mNumViewports == 3)
		{
			if (!pSet->split_vertically)
			{
				if (i == 0)		{	dim_(0.0, 0.0, 0.5, 0.5);	}
				else if (i == 1){	dim_(0.5, 0.0, 0.5, 0.5);	}
				else if (i == 2){	dim_(0.0, 0.5, 1.0, 0.5);	}
			}else{
				if (i == 0)		{	dim_(0.0, 0.0, 0.5, 1.0);	}
				else if (i == 1){	dim_(0.5, 0.0, 0.5, 0.5);	}
				else if (i == 2){	dim_(0.5, 0.5, 0.5, 0.5);	}
			}
		}
		else if (mNumViewports == 4)
		{
			if (i == 0)		{	dim_(0.0, 0.0, 0.5, 0.5);	}
			else if (i == 1){	dim_(0.5, 0.0, 0.5, 0.5);	}
			else if (i == 2){	dim_(0.0, 0.5, 0.5, 0.5);	}
			else if (i == 3){	dim_(0.5, 0.5, 0.5, 0.5);	}
		}
		else
		{
			LogO("FATAL ERROR: Unsupported number of viewports: " + toStr(mNumViewports));
			return;
		}
		#undef dim_

		// Create camera
		mCameras.push_back(mSceneMgr->createCamera("PlayerCamera" + toStr(i)));
		mCameras.back()->setPosition(Vector3(0,-100,0));
		mCameras.back()->lookAt(Vector3(0,-100,10));
		mCameras.back()->setFarClipDistance(pSet->view_distance*1.1f);
		mCameras.back()->setNearClipDistance(0.2f);
		
		// Create viewport
		// use i as Z order
		mViewports.push_back(mWindow->addViewport( mCameras.back(), i+5, dims[0], dims[1], dims[2], dims[3]));
		
		// HUD viewport
		//mHUDViewports.push_back(mWindow->addViewport( mHUDCamera, i, dims[0], dims[1], dims[2], dims[3]));
		//mHUDViewports.back()->setClearEveryFrame(true, FBT_DEPTH);
		//mHUDViewports.back()->setOverlaysEnabled(false);
		//mHUDViewports.back()->setBackgroundColour(ColourValue(0.0, 0.0, 0.0, 0.0));
	}
	
	// Create gui viewport if not already existing
	if (!mGuiViewport)
	{
		mGuiSceneMgr = Ogre::Root::getSingleton().createSceneManager(ST_GENERIC);
		Ogre::Camera* guiCam = mGuiSceneMgr->createCamera("GuiCam1");  // todo destroy !..
		mGuiViewport = mWindow->addViewport(guiCam, 100);

		Ogre::RTShader::ShaderGenerator *mShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
		if(mShaderGenerator != NULL)
		{
			mShaderGenerator->addSceneManager(mSceneMgr);
		}
	}
	
	mHUDSceneMgr = mSceneMgr;
	
	AdjustRatio();
	
	// Add compositing filters for the new viewports
	if (pApp)  pApp->recreateCompositor();
}


void SplitScreenManager::AdjustRatio()
{
	// Go through all viewports & cameras and adjust camera aspect ratio so that it fits to the viewport.
	std::list<Ogre::Camera*>::iterator camIt = mCameras.begin();
	for (std::list<Ogre::Viewport*>::iterator vpIt = mViewports.begin(); vpIt != mViewports.end(); vpIt++)
	{
		(*camIt)->setAspectRatio( float((*vpIt)->getActualWidth()) / float((*vpIt)->getActualHeight()) );
		camIt++;
	}
	
	if (mHUDViewports.size() > 0)
	{
		Ogre::Viewport* firstHUDvp = mHUDViewports.front();
		mHUDCamera->setAspectRatio( float(firstHUDvp->getActualWidth()) / float(firstHUDvp->getActualHeight()) );
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
		// 3d scene viewport
		//  get number of viewport
		/*bool hudVp = false;
		std::list<Ogre::Viewport*>::iterator vpIt = mViewports.begin();
		std::list<Ogre::Viewport*>::iterator hudVpIt = mHUDViewports.begin();
		int i = 0;
		if (vpIt != mViewports.end() && hudVpIt != mHUDViewports.end())
		{
			while (evt.source != *vpIt && evt.source != *hudVpIt)	{	i++;  vpIt++; hudVpIt++;	}
			if (evt.source == *hudVpIt) hudVp = true;
		}*/

		//  get car for this viewport
		int carId = 0;  //-1
		sscanf(vpName.c_str(), "PlayerCamera%d", &carId);
		CarModel* pCarM = pApp->carModels[carId];
			
		//  Size HUD
		pApp->SizeHUD(true, evt.source, carId);

		//  Update HUD for this car
		//*H*/LogO("VP car "+toStr(carId)+" "+toStr(i)+"---------------");
		pApp->UpdateHUD( carId, pCarM, pCarM->pCar, 1.0f / mWindow->getLastFPS(), evt.source );
		
		//if (hudVp) return;  // ?..


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
	}
	else
	{
		//  Gui viewport - hide stuff we don't want
		//*H*/LogO("VP gui --------------------------------------");

		pApp->UpdateHUD(-1, NULL, NULL, mWindow->getLastFPS() );
		pApp->SizeHUD(false);
		
		// no mouse in key capture mode
		if (pApp->bAssignKey)  MyGUI::PointerManager::getInstance().setVisible(false);
	}
}


void SplitScreenManager::postViewportUpdate(const Ogre::RenderTargetViewportEvent& evt)
{

}
