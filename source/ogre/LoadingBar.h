#ifndef _LoadBar_h_
#define _LoadBar_h_
#include <Ogre.h>
// for random
#include <time.h>
using namespace Ogre;


class LoadingBar : public ResourceGroupListener
{
protected:
	
	RenderWindow* mWindow;
	Overlay* mLoadOverlay;
	Real mInitProportion;

	unsigned short mNumGroupsInit;
	unsigned short mNumGroupsLoad;

	Real mProgressBarScriptSize;
	Real mProgressBarInc;

public:

	LoadingBar() {}
	virtual ~LoadingBar() {}

	OverlayElement* mLoadingBarElement;
	OverlayElement* mLoadingDescriptionElement;
	OverlayElement* mLoadingCommentElement;
	
	bool bBackgroundImage;

	Real mProgressBarMaxSize;

	/** Show the loading bar and start listening.
	*/
	void start( RenderWindow* window, 		unsigned short numGroupsInit = 1, 
		unsigned short numGroupsLoad = 1, 
		Real initProportion = 0.0f );

	/** Hide the loading bar and stop listening. 
	*/
	void finish();

	// ResourceGroupListener callbacks
	void resourceGroupScriptingStarted(const String& groupName, size_t scriptCount);

	void resourceGroupScriptingEnded(const String& groupName);

	void scriptParseStarted(const String& scriptName, bool& skipThisScript);

	void scriptParseEnded(const String& scriptName, bool skipped);
	
	//  resourceGroup
	void resourceGroupLoadStarted(const String& groupName, size_t resourceCount);

	void resourceGroupLoadEnded(const String& groupName);

	//  resourceLoad
	void resourceLoadStarted(const ResourcePtr& resource);

	void resourceLoadEnded();

	//  worldGeometry
	void worldGeometryStageStarted(const String& description);

	void worldGeometryStageEnded();

};

#endif
