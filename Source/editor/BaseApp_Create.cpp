#include "stdafx.h"
#include "BaseApp.h"
#include "OgreApp.h" //

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
DWORD WINAPI TimThread(LPVOID lpParam)
{ 
	BaseApp* pA = (BaseApp*)lpParam;
	while (!pA->mShutDown)
	{
		if (pA->timer.update())
			pA->OnTimer(pA->timer.dt);
		Sleep(pA->timer.iv * 1000.0);  //0-
	}
    return 0;
}
#endif
/**/


//  Camera
//-------------------------------------------------------------------------------------
void BaseApp::createCamera()
{
	mCamera = mSceneMgr->createCamera("Cam");
	mCamera->setPosition(Vector3(0,00,100));
	mCamera->lookAt(Vector3(0,0,0));
	mCamera->setNearClipDistance(0.5f);

	mCameraT = mSceneMgr->createCamera("CamThr");
	mCameraT->setPosition(Vector3(0,00,100));
	mCameraT->lookAt(Vector3(0,0,0));

	mViewport = mWindow->addViewport(mCamera);
	mViewport->setBackgroundColour(ColourValue(0.5,0.65,0.8));  //`
	Real asp = Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight());
	mCamera->setAspectRatio(asp);
}


//  Frame
//-------------------------------------------------------------------------------------
using namespace OIS;

void BaseApp::createFrameListener()
{
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");

	OverlayManager& ovr = OverlayManager::getSingleton();  Overlay* o;
	o = ovr.getByName("Editor/FpsOverlay");  if (o) o->show();
	o = ovr.getByName("Editor/FocusOverlay");  if (o) o->show();
	o = ovr.getByName("Editor/StatusOverlay");  if (o) o->show();
	ovSt = ovr.getOverlayElement("Editor/StatusPanel");  if (ovSt)  ovSt->hide();
	ovStat= ovr.getOverlayElement("Editor/StatusText");
	ovFocus = ovr.getOverlayElement("Editor/FocusText"); ovFocBck = ovr.getOverlayElement("Editor/FocusPanel");
		ovFps= ovr.getOverlayElement("Editor/CurrFps");		ovTri= ovr.getOverlayElement("Editor/NumTris");
		ovBat= ovr.getOverlayElement("Editor/NumBatches");	ovPos= ovr.getOverlayElement("Editor/Pos");
	mDebugOverlay= ovr.getByName("Editor/DebugOverlay");  //mDebugOverlay->show();
	ovDbg = ovr.getOverlayElement("Editor/DebugText");
	ovInfo= ovr.getOverlayElement("Editor/Info");

	OIS::ParamList pl;	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	mWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

	mInputManager = OIS::InputManager::createInputSystem( pl );

	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));
	//mKeyboard->setTextTranslation(OIS::Keyboard::Unicode);
	//mKeyboard->setTextTranslation(OIS::Keyboard::Ascii);

	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);

	windowResized(mWindow);
	WindowEventUtilities::addWindowEventListener(mWindow, this);

	mRoot->addFrameListener(this);

	///  timer thread - input, camera
	/**/timer.iv = 0.001;  ///par 
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	/**/hpr = CreateThread(NULL,0,TimThread,(LPVOID)this,0,NULL);
#endif
}

void BaseApp::destroyScene()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	//**/TerminateThread(hpr, 1);
#endif
}

//  Run
//-------------------------------------------------------------------------------------
void BaseApp::Run( bool showDialolg )
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	ShowCursor(0);
	SetCursor(0);
#endif

	mShowDialog = showDialolg;
	if (!setup())
		return;

	mRoot->startRendering();

	destroyScene();
}

//  ctor
//-------------------------------------------------------------------------------------
BaseApp::BaseApp()
	:mRoot(0), mCamera(0),mCameraT(0), mViewport(0)
	,mSceneMgr(0), mWindow(0), mShowDialog(1), mShutDown(false)
	,mInputManager(0), mMouse(0), mKeyboard(0)
	,alt(0), ctrl(0), shift(0)
	,mbLeft(0), mbRight(0), mbMiddle(0)
	,ndSky(0), road(0)

	,mDebugOverlay(0), ovSt(0), ovFps(0), ovTri(0), ovBat(0)
	,ovPos(0), ovDbg(0), ovInfo(0), ovStat(0)
	,ovFocus(0), ovFocBck(0)

	,mStatsOn(0), mShowCamPos(1), mbWireFrame(0)
	,mx(0),my(0),mz(0),	mGUI(0), mPlatform(0)
	,mWndOpts(0), mWndBrush(0), mWndCam(0)
	,mWndRoadCur(0), mWndRoadNew(0), mWndRoadStats(0)

	,i_cmdKeyPress(0), cmdKeyPress(0)
	,i_cmdKeyRel(0), cmdKeyRel(0)
	,i_cmdMouseMove(0), cmdMouseMove(0)
	,i_cmdMousePress(0), cmdMousePress(0)
	,i_cmdMouseRel(0), cmdMouseRel(0)
{
	cmdKeyPress = new CmdKey[cmd_Max];
	cmdKeyRel = new CmdKey[cmd_Max];
	cmdMouseMove = new CmdMouseMove[cmd_Max];
	cmdMousePress = new CmdMouseBtn[cmd_Max];
	cmdMouseRel = new CmdMouseBtn[cmd_Max];
}

BaseApp::~BaseApp()
{
	delete[] cmdKeyPress;
	delete[] cmdKeyRel;
	delete[] cmdMouseMove;
	delete[] cmdMousePress;
	delete[] cmdMouseRel;

	if (mGUI)  {
		mGUI->shutdown();	delete mGUI;	mGUI = 0;  }
	if (mPlatform)  {
		mPlatform->shutdown();	delete mPlatform;	mPlatform = 0;  }

	WindowEventUtilities::removeWindowEventListener(mWindow, this);
	windowClosed(mWindow);
	OGRE_DELETE mRoot;
}

//  config
//-------------------------------------------------------------------------------------
bool BaseApp::configure()
{
	bool ok = false, notFound = false;
	
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        WIN32_FIND_DATAA  fd;
        HANDLE h = FindFirstFileA( "_ogreset_ed.cfg", &fd );
        if (h == INVALID_HANDLE_VALUE)
                notFound = true;
        else
                FindClose(h);

        if (notFound || mShowDialog)
                ok = mRoot->showConfigDialog();
        else
                ok = mRoot->restoreConfig();
#else
        if (mRoot->restoreConfig() ||  mRoot->showConfigDialog())
        {
                ok = true;
        }
        else
                ok=false;
#endif

    if (ok)
    {   mWindow = mRoot->initialise( true, "SR Editor" );
                return true;
    }

	return false;
}

//  Setup
//-------------------------------------------------------------------------------------
bool BaseApp::setup()
{
	#ifdef _DEBUG
	mRoot = new Root("_plugins_d.cfg", "_ogreset_ed.cfg", "_ogre_ed.log");
	#else
	mRoot = new Root("_plugins.cfg", "_ogreset_ed.cfg", "_ogre_ed.log");
	#endif
	
	setupResources();

	if (!configure())
		return false;

	mSceneMgr = mRoot->createSceneManager(/*ST_GENERIC/**/ST_EXTERIOR_FAR/**/);
	createCamera();

	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

	createResourceListener();
	loadResources();

	//  gui
	mPlatform = new MyGUI::OgrePlatform();
	mPlatform->initialise(mWindow, mSceneMgr);
	mGUI = new MyGUI::Gui();
	mGUI->initialise(/*"core.xml", "_mygui.log"*/);

	createFrameListener();
	createScene();

	return true;
};

//  Resources
//-------------------------------------------------------------------------------------
void BaseApp::setupResources()
{
	// Load resource paths from config file
	ConfigFile cf;
	cf.load("_resources_ed.cfg");

	// Go through all sections & settings in the file
	ConfigFile::SectionIterator seci = cf.getSectionIterator();

	String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		ConfigFile::SettingsMultiMap *settings = seci.getNext();
		ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}
}

void BaseApp::createResourceListener()
{
}
void BaseApp::loadResources()
{
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}


//  key, mouse, window
//-------------------------------------------------------------------------------------
bool BaseApp::keyPressed( const OIS::KeyEvent &arg )
{
	cmdKeyPress[i_cmdKeyPress++] = CmdKey(arg);
	return true;
}

bool BaseApp::keyReleased( const OIS::KeyEvent &arg )
{
	//if (bGuiFocus && mGUI)
		cmdKeyRel[i_cmdKeyRel++] = CmdKey(arg);
	//mGUI->injectKeyRelease(MyGUI::KeyCode::Enum(arg.key));
	return true;
}

//  Mouse
//-------------------------------------------------------------------------------------
bool BaseApp::mouseMoved( const OIS::MouseEvent &arg )
{
	#if OGRE_PLATFORM != OGRE_PLATFORM_WIN32
	#define WHEEL_DELTA 120
	#endif
	mx += arg.state.X.rel;  my += arg.state.Y.rel;  mz += arg.state.Z.rel/WHEEL_DELTA;
	if ((bGuiFocus || !bMoveCam) && mGUI && i_cmdMouseMove < cmd_Max)
		cmdMouseMove[i_cmdMouseMove++] = CmdMouseMove(arg);
	return true;
}

bool BaseApp::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (bGuiFocus && mGUI)
		cmdMousePress[i_cmdMousePress++] = CmdMouseBtn(arg,id);

	if		(id == MB_Left)		mbLeft = true;
	else if (id == MB_Right)	mbRight = true;
	else if (id == MB_Middle)	mbMiddle = true;
	return true;
}
bool BaseApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (bGuiFocus && mGUI)
		cmdMouseRel[i_cmdMouseRel++] = CmdMouseBtn(arg,id);

	if		(id == MB_Left)		mbLeft = false;
	else if (id == MB_Right)	mbRight = false;
	else if (id == MB_Middle)	mbMiddle = false;
	return true;
}

//  adjust mouse clipping area
void BaseApp::windowResized(RenderWindow* rw)
{
	unsigned int width, height, depth;  int left, top;
	rw->getMetrics(width, height, depth, left, top);

	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;  ms.height = height;
}

void BaseApp::windowClosed(RenderWindow* rw)
{
	if (rw == mWindow)
	if (mInputManager)
	{
		mInputManager->destroyInputObject( mMouse );  mMouse = 0;
		mInputManager->destroyInputObject( mKeyboard );  mKeyboard = 0;

		OIS::InputManager::destroyInputSystem(mInputManager);  mInputManager = 0;
	}
}
