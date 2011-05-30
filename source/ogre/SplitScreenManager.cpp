#include "pch.h"
#include "Defines.h"
#include "SplitScreenManager.h"

#include "OgreGame.h"
#include "CarModel.h"
#include "../vdrift/settings.h"
#include "../road/Road.h"
#include "MyGUI_PointerManager.h"
using namespace Ogre;


SplitScreenManager::SplitScreenManager(Ogre::SceneManager* sceneMgr, Ogre::RenderWindow* window, SETTINGS* set) :
	pApp(0), mGuiViewport(0), mGuiSceneMgr(0)
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
	// Viewports
	for (std::list<Ogre::Viewport*>::iterator vpIt=mViewports.begin(); vpIt != mViewports.end(); vpIt++)
	{
		mWindow->removeViewport( (*vpIt)->getZOrder() );
	}
	mViewports.clear();

	// Cameras
	for (std::list<Ogre::Camera*>::iterator it=mCameras.begin(); it != mCameras.end(); it++)
	{
		mSceneMgr->destroyCamera(*it);
	}
	mCameras.clear();
}


///  Align
//------------------------------------------------------------------------------------------------------------------
void SplitScreenManager::Align()
{
	CleanUp();
	/* 
	 * Manually create the viewports based on numPlayers.
	 * Of course, this could be implemented in a different way to also support 12674*10^4 viewports,
	 * but since we only need 1-4 we do it this way.
	 */
	for (int i=0; i<mNumPlayers; i++)
	{
		// Set dimensions for the viewports
		float dims[4];
		
		// handy macro for initializing the array
		#define dim_(l,t,w,h)  dims[0]=l;  dims[1]=t;  dims[2]=w;  dims[3]=h
		
		if (mNumPlayers == 1)
		{
			//  Only 1 player, full screen viewport
			dim_(0.0, 0.0, 1.0, 1.0);
		}
		else if (mNumPlayers == 2)
		{
			//  2 players, use split_vertically setting
			if (!pSet->split_vertically)
			{	if (i == 0)		{	dim_(0.0, 0.0, 1.0, 0.5);	}
				else if (i == 1){	dim_(0.0, 0.5, 1.0, 0.5);	}
			}else{
				if (i == 0) {		dim_(0.0, 0.0, 0.5, 1.0);	}
				else if (i == 1) {	dim_(0.5, 0.0, 0.5, 1.0);	}	}
		}
		else if (mNumPlayers == 3)
		{
			// 3 players, use split_vertically setting
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
		else if (mNumPlayers == 4)
		{
			// 4 players, 2 viewports at top and 2 at bottom
			if (i == 0)		{	dim_(0.0, 0.0, 0.5, 0.5);	}
			else if (i == 1){	dim_(0.5, 0.0, 0.5, 0.5);	}
			else if (i == 2){	dim_(0.0, 0.5, 0.5, 0.5);	}
			else if (i == 3){	dim_(0.5, 0.5, 0.5, 0.5);	}
		}
		else
		{
			LogO("FATAL ERROR: Unsupported number of viewports: " + toStr(mNumPlayers));
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
		mViewports.push_back(mWindow->addViewport( mCameras.back(), i, dims[0], dims[1], dims[2], dims[3]));
	}
	
	// Create gui viewport if not already existing
	if (!mGuiViewport)
	{
		mGuiSceneMgr = Ogre::Root::getSingleton().createSceneManager(ST_GENERIC);
		Ogre::Camera* guiCam = mGuiSceneMgr->createCamera("GuiCam1");
		mGuiViewport = mWindow->addViewport(guiCam, 100);
	}
	
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
}


///  pre viewport update
//------------------------------------------------------------------------------------------------------------------
void SplitScreenManager::preViewportUpdate(const Ogre::RenderTargetViewportEvent& evt)
{
	if (!pApp)  return;
	if (pApp->bLoading)  return;

	// What kind of viewport is being updated?
	if (evt.source != mGuiViewport)
	{
		// 3d scene viewport
		//  get number of viewport
		std::list<Ogre::Viewport*>::iterator vpIt = mViewports.begin();
		int i = 0;
		while (evt.source != *vpIt)
		{
			i++;
			vpIt++;
		}

		//  get car for this viewport
		std::list<CarModel*>::iterator carIt = pApp->carModels.begin();
		if (pApp->carModels.size() > 0)
		{
			int j = 0;
			while (j <= i)
			{
				if ((*carIt)->eType == CarModel::CT_REMOTE)
					j--;
				else
					if (j == i)
						break;
				j++;
				carIt++;
			}
		}
			
		//  Size HUD
		pApp->SizeHUD(true, evt.source);

		//  Update HUD for this car
		if (pApp->carModels.size() > 0 && *carIt && (*carIt)->pCar)
			pApp->UpdateHUD( (*carIt)->pCar, 1.0f / mWindow->getLastFPS(), evt.source );
		else
			pApp->UpdateHUD( NULL, 1.0f / mWindow->getLastFPS(), evt.source );


		//  Set skybox pos to camera
		if (pApp->ndSky)
			pApp->ndSky->setPosition(evt.source->getCamera()->getPosition());
			

		//  road lod for each viewport
		if (mNumPlayers > 1)
		if (pApp->road)
		{
			pApp->road->mCamera = evt.source->getCamera();
			pApp->road->UpdLodVis(pSet->road_dist);
		}
		
		//  Update rain/snow - depends on camera
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
		// Gui viewport, overlay and mygui
		//  hide stuff we don't want
		pApp->UpdateHUD( NULL, mWindow->getLastFPS() );

		pApp->SizeHUD(false);
		
		// no mouse in key capture mode
		if (pApp->bAssignKey)  MyGUI::PointerManager::getInstance().setVisible(false);
	}
}


void SplitScreenManager::postViewportUpdate(const Ogre::RenderTargetViewportEvent& evt)
{

}
