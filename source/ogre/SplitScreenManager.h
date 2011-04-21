#ifndef _SplitScreenManager_h_
#define _SplitScreenManager_h_

#include "Ogre.h"

/*
 * SplitScreenManager can be used to split up the screen into several areas.
 * This is useful for "Hotseat" game mode (Multiple players driving against each other on the same PC)
 * 
 * For every player a viewport and a camera will be created.
 * Also there is one transparent fullscreen viewport on top of the others, for the GUI, but it is created in BaseApp, not here.
 * 
 * One instance of SplitScreenManager is created when the game is started,
 * and it will be destroyed when game quits.
 */

class SplitScreenManager : public Ogre::RenderTargetListener
{
public:
	// Constructor, only assign members
	SplitScreenManager(Ogre::SceneManager* sceneMgr, Ogre::SceneManager* guiSceneMgr, Ogre::RenderWindow* window);
	
	~SplitScreenManager();
	
	// Number of viewports / cameras
	unsigned int mNumPlayers;
	
	// Lists for player viewports & cameras
	std::list<Ogre::Viewport*> mViewports;
	std::list<Ogre::Camera*> mCameras;
	
	// This method should always be called after mNumPlayers is changed.
	// It will create new viewports and cameras and arrange them.
	void Align();
	
	// Needed to update HUD on render target event
	class App* pApp;
	
	void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
	void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);

private:
	// Scene manager for the 3d scene
	Ogre::SceneManager* mSceneMgr;

	// Scene manager that is used for the gui viewport
	Ogre::SceneManager* mGuiSceneMgr;
	
	// Render window to use
	Ogre::RenderWindow* mWindow;
};

#endif
