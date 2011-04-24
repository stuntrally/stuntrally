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
	SplitScreenManager(Ogre::SceneManager* sceneMgr, Ogre::RenderWindow* window, class SETTINGS* set);
	
	~SplitScreenManager();
	
	// Number of viewports / cameras
	unsigned int mNumPlayers;
	
	// Lists for player viewports & cameras
	std::list<Ogre::Viewport*> mViewports;
	std::list<Ogre::Camera*> mCameras;
	
	// This method should always be called after mNumPlayers is changed.
	// It will create new viewports and cameras and arrange them.
	void Align();
	
	// Adjust viewport size / camera aspect ratio
	// Should be called whenever the window size changes
	void AdjustRatio();
	
	// Set background color for all viewports
	void SetBackground(const Ogre::ColourValue& color);
	
	// Update the view distance for all cameras.
	// This will be called when the view distance slider in gui is changed.
	void UpdateCamDist();
	
	class App* pApp;
	
	void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
	void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);

private:
	class SETTINGS* pSet;

	// Scene manager to use
	Ogre::SceneManager* mSceneMgr;
	
	// Render window to use
	Ogre::RenderWindow* mWindow;
};

#endif
