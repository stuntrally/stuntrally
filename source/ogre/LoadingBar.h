#pragma once
#include <OgreString.h>
#include <OgreResource.h>
#include <OgreResourceGroupManager.h>
#include "common/PreviewTex.h"

namespace Ogre {  class RenderWindow;  }
class BaseApp;


class LoadingBar : public Ogre::ResourceGroupListener
{
protected:
	
	Ogre::RenderWindow* mWindow;

	Ogre::Real mInitProportion;

	unsigned short mNumGroupsInit;
	unsigned short mNumGroupsLoad;

	Ogre::Real mBarInc;

public:

	LoadingBar(BaseApp* app)
		:pApp(app), mWindow(0), bBackgroundImage(false)
		,mInitProportion(0.f), mNumGroupsInit(1), mNumGroupsLoad(1)
		,mBarMaxSizeX(0.f),mBarSizeY(0.f), oldNumber(-1)
	{  }
	virtual ~LoadingBar()
	{  }
	BaseApp* pApp;


	void SetWidth(Ogre::Real pecent);
	
	bool bBackgroundImage;

	Ogre::Real mBarMaxSizeX,mBarSizeY;


	/** Show the loading bar and start listening.
	*/
	void start( Ogre::RenderWindow* window,
		unsigned short numGroupsInit = 1,
		unsigned short numGroupsLoad = 1,
		Ogre::Real initProportion = 0.0f );

	/** Hide the loading bar and stop listening. 
	*/
	void finish();

	int oldNumber;  // var
	PreviewTex loadTex;


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
