#ifndef _LoadBar_h_
#define _LoadBar_h_
using namespace Ogre;


class LoadingBar : public ResourceGroupListener
{
protected:
	
	RenderWindow* mWindow;
	Overlay* mLoadOverlay;
	Real mInitProportion;

	unsigned short mNumGroupsInit;
	unsigned short mNumGroupsLoad;

	Real mProgressBarMaxSize;
	Real mProgressBarScriptSize;
	Real mProgressBarInc;

	OverlayElement* mLoadingBarElement;
	OverlayElement* mLoadingDescriptionElement;
	OverlayElement* mLoadingCommentElement;


public:

	LoadingBar() {}
	virtual ~LoadingBar() {}


	/** Show the loading bar and start listening.
	*/
	virtual void start( RenderWindow* window, 
		unsigned short numGroupsInit = 1, 
		unsigned short numGroupsLoad = 1, 
		Real initProportion = 0.0f )
	{
		mWindow = window;
		mNumGroupsInit = numGroupsInit;
		mNumGroupsLoad = numGroupsLoad;
		mInitProportion = initProportion;

		ResourceGroupManager::getSingleton().initialiseResourceGroup("Loading");

		OverlayManager& omgr = OverlayManager::getSingleton();
		mLoadOverlay = (Overlay*)omgr.getByName("Core/LoadOverlay");
		if (!mLoadOverlay)
		{
			OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find loading overlay", "ExampleLoadingBar::start");
		}
		mLoadOverlay->show();

		// Save links to the bar and to the loading text, for updates as we go
		mLoadingBarElement = omgr.getOverlayElement("Core/LoadPanel/Bar/Progress");
		mLoadingCommentElement = omgr.getOverlayElement("Core/LoadPanel/Comment");
		mLoadingDescriptionElement = omgr.getOverlayElement("Core/LoadPanel/Description");

		OverlayElement* barContainer = omgr.getOverlayElement("Core/LoadPanel/Bar");
		mProgressBarMaxSize = barContainer->getWidth();
		mLoadingBarElement->setWidth(0);

		// self is listener
		ResourceGroupManager::getSingleton().addResourceGroupListener(this);
	}


	/** Hide the loading bar and stop listening. 
	*/
	virtual void finish()
	{
		// hide loading screen
		mLoadOverlay->hide();

		// Unregister listener
		ResourceGroupManager::getSingleton().removeResourceGroupListener(this);
	}


	// ResourceGroupListener callbacks
	void resourceGroupScriptingStarted(const String& groupName, size_t scriptCount)
	{
		assert( mNumGroupsInit > 0 && "You were not going to init ");
		// Lets assume script loading is 70%
		mProgressBarInc = mProgressBarMaxSize * mInitProportion / (Real)scriptCount;
		mProgressBarInc /= mNumGroupsInit;
		//-mLoadingDescriptionElement->setCaption("Parsing scripts...");
		mWindow->update();
	}

	void resourceGroupScriptingEnded(const String& groupName)
	{
	}

	void scriptParseStarted(const String& scriptName, bool& skipThisScript)
	{
		mLoadingCommentElement->setCaption(scriptName);
		mWindow->update();
	}

	void scriptParseEnded(const String& scriptName, bool skipped)
	{
		mLoadingBarElement->setWidth(mLoadingBarElement->getWidth() + mProgressBarInc);
		mWindow->update();
	}

	//  resourceGroup
	void resourceGroupLoadStarted(const String& groupName, size_t resourceCount)
	{
		assert( mNumGroupsLoad > 0 && "You were not going to load ");
		mProgressBarInc = mProgressBarMaxSize * (1-mInitProportion) / (Real)resourceCount;
		mProgressBarInc /= mNumGroupsLoad;
		mLoadingDescriptionElement->setCaption("Loading resources...");
		mWindow->update();
	}

	void resourceGroupLoadEnded(const String& groupName)
	{
	}

	//  resourceLoad
	void resourceLoadStarted(const ResourcePtr& resource)
	{
		mLoadingCommentElement->setCaption(resource->getName());
		mWindow->update();
	}

	void resourceLoadEnded()
	{
	}

	//  worldGeometry
	void worldGeometryStageStarted(const String& description)
	{
		mLoadingCommentElement->setCaption(description);
		mWindow->update();
	}

	void worldGeometryStageEnded()
	{
		mLoadingBarElement->setWidth(mLoadingBarElement->getWidth() + mProgressBarInc);
		mWindow->update();
	}

};

#endif