#include "pch.h"
#include "common/Def_Str.h"
#include "LoadingBar.h"
#include <time.h>  // for random

#include <OgreRenderWindow.h>
#include <OgreTechnique.h>
#include <MyGUI.h>
using namespace Ogre;
using namespace MyGUI;
#include "CGame.h"
#include "../vdrift/settings.h"
#include "../vdrift/pathmanager.h"


void LoadingBar::start( RenderWindow* window,
	unsigned short numGroupsInit, unsigned short numGroupsLoad, Real initProportion )
{
	mWindow = window;
	mNumGroupsInit = numGroupsInit;  mNumGroupsLoad = numGroupsLoad;
	mInitProportion = initProportion;

	//  show loading
	pApp->bckLoad->setVisible(true);
	mBarMaxSizeX = pApp->barSizeX;  mBarSizeY = pApp->barSizeY;
	pApp->barLoad->setSize(0, mBarSizeY);

	//  self is listener
	ResourceGroupManager::getSingleton().addResourceGroupListener(this);
	
	//  title
	pApp->txLoadBig->setCaption(String(TR("#{LoadingDesc}")));
		
	if (!pApp->pSet->loadingbackground)
	{
		pApp->imgLoad->setVisible(false);
		return;
	}

	int i = 1;  // count background images
	while (ResourceGroupManager::getSingleton().resourceExists("General", "loading" + toStr(i) + ".jpg"))
		++i;

	if (i == 1)  // none
	{
		pApp->imgLoad->setVisible(false);
		return;
	}
	
	//  init random seed
	srand(time(NULL));
	unsigned int imgNumber = rand() % (i-1);
	//  need 2,  not the same number
	while (i > 2 && imgNumber == oldNumber)
		imgNumber = rand() % (i-1);
	oldNumber = imgNumber;

	//  set the loading image
	loadTex.Load(PATHMANAGER::Data()+"/loading/loading" +toStr(imgNumber+1)+ ".jpg");
	pApp->imgLoad->setImageTexture("LoadingTex");
	pApp->imgLoad->setVisible(true);
}


void LoadingBar::finish()
{
	//  hide loading
	pApp->bckLoad->setVisible(false);
	//pApp->imgLoad->setVisible(false); //not here, later..

	//  Unregister listener
	ResourceGroupManager::getSingleton().removeResourceGroupListener(this);
}


void LoadingBar::resourceGroupScriptingStarted(const String& groupName, size_t scriptCount)
{
	assert( mNumGroupsInit > 0 && "You were not going to init ");
	mBarInc = mBarMaxSizeX * mInitProportion / (Real)scriptCount;
	mBarInc /= mNumGroupsInit;
	mWindow->update();
}

void LoadingBar::resourceGroupScriptingEnded(const String& groupName)
{
}

void LoadingBar::scriptParseStarted(const String& scriptName, bool& skipThisScript)
{
	//mComment->setCaption(scriptName);
	pApp->txLoadBig->setCaption(String(TR("#{LoadingDesc}")));
	//mWindow->update();
}

void LoadingBar::scriptParseEnded(const String& scriptName, bool skipped)
{
	IntSize s = pApp->barLoad->getSize();
	pApp->barLoad->setSize(s.width + mBarInc, mBarSizeY);
	mWindow->update();
}

//  resourceGroup
void LoadingBar::resourceGroupLoadStarted(const String& groupName, size_t resourceCount)
{
	assert( mNumGroupsLoad > 0 && "You were not going to load ");
	mBarInc = mBarMaxSizeX * (1-mInitProportion) / (Real)resourceCount;
	mBarInc /= mNumGroupsLoad;
	mWindow->update();
}

void LoadingBar::resourceGroupLoadEnded(const String& groupName)
{
}

//  resourceLoad
void LoadingBar::resourceLoadStarted(const ResourcePtr& resource)
{
	//mComment->setCaption(resource->getName());
	mWindow->update();
}

void LoadingBar::resourceLoadEnded()
{
}

//  worldGeometry
void LoadingBar::worldGeometryStageStarted(const String& description)
{
	pApp->txLoad->setCaption(description);
	mWindow->update();
}

void LoadingBar::worldGeometryStageEnded()
{
	IntSize s = pApp->barLoad->getSize();
	pApp->barLoad->setSize(s.width + mBarInc, mBarSizeY);
	mWindow->update();
}


void LoadingBar::SetWidth(Real pecent)
{
	float p = pecent * 0.01f;
	int s = p * mBarMaxSizeX, w = p * 512.f;
	//pApp->barLoad->setImageCoord( IntCoord(512-w,0, w,64) );
	pApp->barLoad->setSize( s, mBarSizeY );
}
