#include "stdafx.h"
#include "SplitScreenManager.h"
#include "OgreGame.h"
#include "CarModel.h"

SplitScreenManager::SplitScreenManager(Ogre::SceneManager* sceneMgr, Ogre::RenderWindow* window)
{
	mWindow = window;
	mSceneMgr = sceneMgr;
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
		// Create camera
		mCameras.push_back(mSceneMgr->createCamera("PlayerCamera" + toStr(i)));
		
		// Create viewport
		// use i as Z order
		mViewports.push_back(mWindow->addViewport( *(--mCameras.end()), i));
		
		// Add a render target listener for this viewport
		(*(--mViewports.end()))->getTarget()->addListener(this);
		
		// Set dimensions depending on which viewport this is
		if (mNumPlayers == 1)
		{
			// 1 player, 1 fullscreen viewport
			// nothing to do
		}
		else if (mNumPlayers == 2)
		{
			// 2 players, 1 viewport at top and 1 at bottom
			if (i == 0)
				(*(--mViewports.end()))->setDimensions(0.0, 0.0, 1.0, 0.5);
			else if (i == 1)
				(*(--mViewports.end()))->setDimensions(0.0, 0.5, 1.0, 0.5);
		}
		else if (mNumPlayers == 3)
		{
			// 3 players, 2 viewports at top and 1 at bottom
			if (i == 0)
				(*(--mViewports.end()))->setDimensions(0.0, 0.0, 0.5, 0.5);
			else if (i == 1)
				(*(--mViewports.end()))->setDimensions(0.5, 0.0, 0.5, 0.5);
			else if (i == 2)
				(*(--mViewports.end()))->setDimensions(0.0, 0.5, 1.0, 0.5);
		}
		else if (mNumPlayers == 4)
		{
			// 4 players, 2 viewports at top and 2 at bottom
			if (i == 0)
				(*(--mViewports.end()))->setDimensions(0.0, 0.0, 0.5, 0.5);
			else if (i == 1)
				(*(--mViewports.end()))->setDimensions(0.5, 0.0, 0.5, 0.5);
			else if (i == 2)
				(*(--mViewports.end()))->setDimensions(0.0, 0.5, 0.5, 0.5);
			else if (i == 3)
				(*(--mViewports.end()))->setDimensions(0.5, 0.5, 0.5, 0.5);	
		}
	}
	
	AdjustRatio();
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
void SplitScreenManager::preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
	// get number of viewport
	int i=0;
	std::list<Ogre::Viewport*>::iterator vpIt = mViewports.begin();
	while (evt.source != (*vpIt)->getTarget() )
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
	if ((*carIt)->pCar)
		pApp->UpdateHUD( (*carIt)->pCar, 1.0f / evt.source->getLastFPS() );
}
void SplitScreenManager::postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
}
