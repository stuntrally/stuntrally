#include "stdafx.h"
#include "SplitScreenManager.h"

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
	mWindow->removeAllViewports();
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
	}
	
	// Create gui viewport
	/// TODO
}
