#include "stdafx.h"
#include "SplitScreenManager.h"
#include "OgreGame.h"
#include "CarModel.h"
#include "../vdrift/settings.h"

SplitScreenManager::SplitScreenManager(Ogre::SceneManager* sceneMgr, Ogre::RenderWindow* window, SETTINGS* set) :
	pApp(0)
{
	mWindow = window;
	mSceneMgr = sceneMgr;
	pSet = set;
}
SplitScreenManager::~SplitScreenManager()
{
	// Cleanup
	// Viewports
	for (std::list<Ogre::Viewport*>::iterator vpIt=mViewports.begin(); vpIt != mViewports.end(); vpIt++)
	{
		(*vpIt)->getTarget()->removeListener(this);
		mWindow->removeViewport( (*vpIt)->getZOrder() );
	}
	mViewports.clear();
	// Cameras
	for (std::list<Ogre::Camera*>::iterator it=mCameras.begin(); it != mCameras.end(); it++)
	{
		mSceneMgr->destroyCamera( (*it) );
	}
	mCameras.clear();
}
void SplitScreenManager::SetBackground(const Ogre::ColourValue& value)
{
	for (std::list<Ogre::Viewport*>::iterator vpIt=mViewports.begin(); vpIt != mViewports.end(); vpIt++)
	{
		(*vpIt)->setBackgroundColour(value);
	}
}
void SplitScreenManager::UpdateCamDist()
{
	for (std::list<Ogre::Camera*>::iterator it=mCameras.begin(); it != mCameras.end(); it++)
	{
		(*it)->setFarClipDistance(pSet->view_distance*1.1f);
	}
}
void SplitScreenManager::Align()
{
	// Cleanup old
	// Viewports
	for (std::list<Ogre::Viewport*>::iterator vpIt=mViewports.begin(); vpIt != mViewports.end(); vpIt++)
	{
		(*vpIt)->getTarget()->removeListener(this);
		mWindow->removeViewport( (*vpIt)->getZOrder() );
	}
	mViewports.clear();
	// Cameras
	for (std::list<Ogre::Camera*>::iterator it=mCameras.begin(); it != mCameras.end(); it++)
	{
		mSceneMgr->destroyCamera( (*it) );
	}
	mCameras.clear();
		
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
		#define dim_(t,l,h,w) dims[0]=t;dims[1]=l;dims[2]=h;dims[3]=w
		
		if (mNumPlayers == 1)
		{
			// Only 1 player, full screen viewport
			dim_(0.0, 0.0, 1.0, 1.0);
		}
		else if (mNumPlayers == 2)
		{
			// 2 players, 1 viewport at top and 1 at bottom
			if (i == 0) {
				dim_(0.0, 0.0, 1.0, 0.5); }
			else if (i == 1) {
				dim_(0.0, 0.5, 1.0, 0.5); }
		}
		else if (mNumPlayers == 3)
		{
			// 3 players, 2 viewports at top and 1 at bottom
			if (i == 0) {
				dim_(0.0, 0.0, 0.5, 0.5); }
			else if (i == 1) {
				dim_(0.5, 0.0, 0.5, 0.5); }
			else if (i == 2) {
				dim_(0.0, 0.5, 1.0, 0.5); }
		}
		else if (mNumPlayers == 4)
		{
			// 4 players, 2 viewports at top and 2 at bottom
			if (i == 0) {
				dim_(0.0, 0.0, 0.5, 0.5); }
			else if (i == 1) {
				dim_(0.5, 0.0, 0.5, 0.5); }
			else if (i == 2) {
				dim_(0.0, 0.5, 0.5, 0.5); }
			else if (i == 3) {
				dim_(0.5, 0.5, 0.5, 0.5); }
		}
		else
		{
			Log("FATAL ERROR: Unsupported number of viewports: " + toStr(mNumPlayers));
			return;
		}
		#undef dim_

		
		// Create camera
		mCameras.push_back(mSceneMgr->createCamera("PlayerCamera" + toStr(i)));
		
		mCameras.back()->setFarClipDistance(pSet->view_distance*1.1f);
		mCameras.back()->setPosition(Vector3(0,-100,0));
		mCameras.back()->lookAt(Vector3(0,-100,10));

		mCameras.back()->setNearClipDistance(0.2f);
		
		// Create viewport
		// use i+1 as Z order
		mViewports.push_back(mWindow->addViewport( *(--mCameras.end()), i+1, dims[0], dims[1], dims[2], dims[3]));
		// Add a render target listener for this viewport
		mViewports.back()->getTarget()->addListener(this);
	}
	
	AdjustRatio();
	
	// Add compositing filters for the new viewports
	if (pApp) pApp->recreateCompositor();
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
void SplitScreenManager::preViewportUpdate(const Ogre::RenderTargetViewportEvent& evt)
{
	if (!pApp) return;
	if (pApp->bLoading) return;
	if (pApp->carModels.size() < 1)
		return;
	/// Update HUD
	// get number of viewport
	int i=0;
	std::list<Ogre::Viewport*>::iterator vpIt = mViewports.begin();
	while (evt.source != (*vpIt) )
	{
		vpIt++;
		i++;
		if (vpIt == mViewports.end()) return;
	}
	// get car for this viewport
	std::list<CarModel*>::iterator carIt = pApp->carModels.begin();
	int j=0;
	while (j <= i)
	{
		if ((*carIt)->eType == CarModel::CT_REMOTE)
			j--;
		else
		{
			if (j == i)
				break;
		}
		j++;
		carIt++;
		if (carIt == pApp->carModels.end()) return;
	}
	
	// Update HUD for this car
	if ((*carIt) && (*carIt)->pCar)
		pApp->UpdateHUD( (*carIt)->pCar, 1.0f / evt.source->getTarget()->getLastFPS() );
		
	/// Size HUD
	pApp->SizeHUD(true, evt.source);
		
	/// Set skybox pos to camera
	if (pApp->ndSky)
		pApp->ndSky->setPosition(evt.source->getCamera()->getPosition());
}
void SplitScreenManager::postViewportUpdate(const Ogre::RenderTargetViewportEvent& evt)
{
}
