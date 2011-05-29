#ifndef _LoadBar_h_
#define _LoadBar_h_

#include <Ogre.h>
#include <time.h>  // for random
#include <OgreResourceGroupManager.h>


class LoadingBar : public Ogre::ResourceGroupListener
{
protected:
	
	Ogre::RenderWindow* mWindow;
	Ogre::Overlay* mLoadOverlay;
	Ogre::Real mInitProportion;

	unsigned short mNumGroupsInit;
	unsigned short mNumGroupsLoad;

	Ogre::Real mProgressBarScriptSize;
	Ogre::Real mProgressBarInc;

public:

	LoadingBar() {}
	virtual ~LoadingBar() {}

	Ogre::OverlayElement* mLoadingBarElement;
	Ogre::OverlayElement* mLoadingDescriptionElement;
	Ogre::OverlayElement* mLoadingCommentElement;
	
	bool bBackgroundImage;

	Ogre::Real mProgressBarMaxSize;

	/** Show the loading bar and start listening.
	*/
	void start( Ogre::RenderWindow* window,
		unsigned short numGroupsInit = 1,
		unsigned short numGroupsLoad = 1,
		Ogre::Real initProportion = 0.0f );

	/** Hide the loading bar and stop listening. 
	*/
	void finish();

	// ResourceGroupListener callbacks
	void resourceGroupScriptingStarted(const Ogre::String& groupName, size_t scriptCount);

	void resourceGroupScriptingEnded(const Ogre::String& groupName);

	void scriptParseStarted(const Ogre::String& scriptName, bool& skipThisScript);

	void scriptParseEnded(const Ogre::String& scriptName, bool skipped);
	
	//  resourceGroup
	void resourceGroupLoadStarted(const Ogre::String& groupName, size_t resourceCount);

	void resourceGroupLoadEnded(const Ogre::String& groupName);

	//  resourceLoad
	void resourceLoadStarted(const Ogre::ResourcePtr& resource);

	void resourceLoadEnded();

	//  worldGeometry
	void worldGeometryStageStarted(const Ogre::String& description);

	void worldGeometryStageEnded();

};

#endif
