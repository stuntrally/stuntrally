#include "stdafx.h"
#include "SplitScreenManager.h"
#include "OgreGame.h"
#include "CarModel.h"

SplitScreenManager::SplitScreenManager(Ogre::SceneManager* sceneMgr, Ogre::SceneManager* guiSceneMgr, Ogre::RenderWindow* window)
{
	mWindow = window;
	mGuiSceneMgr = guiSceneMgr;
	mSceneMgr = sceneMgr;
}
SplitScreenManager::~SplitScreenManager()
{
}
void SplitScreenManager::Align()
{
	// Cleanup old
	// Viewports
	for (std::list<Ogre::Viewport*>::iterator vpIt=mViewports.begin(); vpIt != mViewports.end(); vpIt++)
	{
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
		
		/// TODO dimensions
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
