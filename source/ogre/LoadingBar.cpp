#include "LoadingBar.h"

void LoadingBar::start( RenderWindow* window, 		unsigned short numGroupsInit, 
	unsigned short numGroupsLoad, 
	Real initProportion )
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
	
	if (bBackgroundImage)
	{
		// figure out how many background images we have.
		unsigned int i=1;
		while (1)
		{
			if (ResourceGroupManager::getSingleton().resourceExists("Loading", "loading" + Ogre::StringConverter::toString(i) + ".png") )
				i++;
			else
				break;
		}
		if (i == 1)
		{
			// no load screens found
			// remove image
			MaterialPtr mat = MaterialManager::getSingleton().getByName("Core/BackgroundMat", "Loading");
			if (mat->getTechnique(0)->getPass(0)->getNumTextureUnitStates() > 0)
				mat->getTechnique(0)->getPass(0)->removeTextureUnitState(0);
			return;
		}
		// init random seed
		srand(time(NULL));
		unsigned int imgNumber;
		imgNumber = rand() % (i-1);
		// set the loading image
		MaterialPtr mat = MaterialManager::getSingleton().getByName("Core/BackgroundMat", "Loading");
		mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("loading" + Ogre::StringConverter::toString(imgNumber+1) + ".png");
	}
	else
	{
		// remove image
		MaterialPtr mat = MaterialManager::getSingleton().getByName("Core/BackgroundMat", "Loading");
		if (mat->getTechnique(0)->getPass(0)->getNumTextureUnitStates() > 0)
			mat->getTechnique(0)->getPass(0)->removeTextureUnitState(0);
	}
}

void LoadingBar::finish()
{
	// hide loading screen
	mLoadOverlay->hide();

	// Unregister listener
	ResourceGroupManager::getSingleton().removeResourceGroupListener(this);
}

void LoadingBar::resourceGroupScriptingStarted(const String& groupName, size_t scriptCount)
{
	assert( mNumGroupsInit > 0 && "You were not going to init ");
	// Lets assume script loading is 70%
	mProgressBarInc = mProgressBarMaxSize * mInitProportion / (Real)scriptCount;
	mProgressBarInc /= mNumGroupsInit;
	//-mLoadingDescriptionElement->setCaption("Parsing scripts...");
	mWindow->update();
}

void LoadingBar::resourceGroupScriptingEnded(const String& groupName)
{
}

void LoadingBar::scriptParseStarted(const String& scriptName, bool& skipThisScript)
{
	mLoadingCommentElement->setCaption(scriptName);
	mWindow->update();
}

void LoadingBar::scriptParseEnded(const String& scriptName, bool skipped)
{
	mLoadingBarElement->setWidth(mLoadingBarElement->getWidth() + mProgressBarInc);
	mWindow->update();
}

//  resourceGroup
void LoadingBar::resourceGroupLoadStarted(const String& groupName, size_t resourceCount)
{
	assert( mNumGroupsLoad > 0 && "You were not going to load ");
	mProgressBarInc = mProgressBarMaxSize * (1-mInitProportion) / (Real)resourceCount;
	mProgressBarInc /= mNumGroupsLoad;
	mLoadingDescriptionElement->setCaption("Loading resources...");
	mWindow->update();
}

void LoadingBar::resourceGroupLoadEnded(const String& groupName)
{
}

//  resourceLoad
void LoadingBar::resourceLoadStarted(const ResourcePtr& resource)
{
	mLoadingCommentElement->setCaption(resource->getName());
	mWindow->update();
}

void LoadingBar::resourceLoadEnded()
{
}

//  worldGeometry
void LoadingBar::worldGeometryStageStarted(const String& description)
{
	mLoadingCommentElement->setCaption(description);
	mWindow->update();
}

void LoadingBar::worldGeometryStageEnded()
{
	mLoadingBarElement->setWidth(mLoadingBarElement->getWidth() + mProgressBarInc);
	mWindow->update();
}
