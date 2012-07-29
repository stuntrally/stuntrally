#include "pch.h"
#include "../ogre/common/Defines.h"

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include "BaseApp.h"
#include "OgreApp.h" //

#include "../vdrift/pathmanager.h"
#include "../ogre/Localization.h"
#include "../ogre/common/HWMouse.h"

#include <OgreConfigFile.h>
#include <OISInputManager.h>
using namespace Ogre;

#include "../ogre/common/MyGUI_D3D11.h"

void TimThread(BaseApp* pA)
{
	while (pA->inputThreadRunning)
	{
		if (pA->timer.update())
			pA->OnTimer(pA->timer.dt);
		boost::this_thread::sleep(boost::posix_time::milliseconds(pA->timer.iv*1000));
	}
}

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
	//mViewport->setBackgroundColour(ColourValue(0.5,0.65,0.8));  //`
	mViewport->setBackgroundColour(ColourValue(0.2,0.3,0.4));  //`
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
	ovBrushPrv = ovr.getByName("Editor/BrushPrvOverlay");  //ovBrushPrv->show();
	ovBrushMtr = ovr.getOverlayElement("Editor/BrushPrvPanel");

	OIS::ParamList pl;	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	mWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
	
    #if defined OIS_LINUX_PLATFORM
    if (!pSet->x11_capture_mouse)
    {
		pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
		pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("true")));
		pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
	}
    pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
    #endif

	mInputManager = OIS::InputManager::createInputSystem( pl );

	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));
	//mKeyboard->setTextTranslation(OIS::Keyboard::Unicode);
	//mKeyboard->setTextTranslation(OIS::Keyboard::Ascii);

	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);
	
	//mMouse->capture();
	//mKeyboard->capture();
	//mHWMouse = new HWMouse(windowHnd, 8, 8, "pointer.png");

	windowResized(mWindow);
	WindowEventUtilities::addWindowEventListener(mWindow, this);

	mRoot->addFrameListener(this);

	///  timer thread - input, camera
	/**/timer.iv = 0.005;  ///par 
	mThread = new boost::thread(TimThread, this);
}

void BaseApp::destroyScene()
{
}

//  Run
//-------------------------------------------------------------------------------------
void BaseApp::Run( bool showDialog )
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	if (!pSet->ogre_dialog)
	{	ShowCursor(0);
		SetCursor(0);	}
#endif

	mShowDialog = showDialog;
	if (!setup())
		return;

	mRoot->startRendering();

	destroyScene();
}

//  ctor
//-------------------------------------------------------------------------------------
BaseApp::BaseApp()
	:mRoot(0), mCamera(0),mCameraT(0), mViewport(0)
	,mSceneMgr(0), mWindow(0)
	,mShowDialog(1), mShutDown(false), bWindowResized(0), bFirstRenderFrame(true)
	,mInputManager(0), mMouse(0), mKeyboard(0)
	,alt(0), ctrl(0), shift(0)
	,mbLeft(0), mbRight(0), mbMiddle(0)
	,ndSky(0), road(0)

	,mDebugOverlay(0), ovSt(0), ovFps(0), ovTri(0), ovBat(0)
	,ovPos(0), ovDbg(0), ovInfo(0), ovStat(0)
	,ovFocus(0), ovFocBck(0), ovBrushPrv(0), ovBrushMtr(0)

	,mStatsOn(0), mShowCamPos(1), mbWireFrame(0)
	,mx(0),my(0),mz(0),	mGUI(0), mPlatform(0)

	,mWndMain(0),mWndEdit(0),mWndHelp(0),mWndOpts(0)
	,mWndTabsEdit(0),mWndTabsHelp(0),mWndTabsOpts(0)
	,mWndBrush(0), mWndCam(0), mWndStart(0)
	,mWndRoadCur(0), mWndRoadStats(0), mWndFluids(0), mWndObjects(0)

	,i_cmdKeyPress(0), cmdKeyPress(0)
	,i_cmdKeyRel(0), cmdKeyRel(0)
	,i_cmdMouseMove(0), cmdMouseMove(0)
	,i_cmdMousePress(0), cmdMousePress(0)
	,i_cmdMouseRel(0), cmdMouseRel(0)
	
	,pSet(0), bMoveCam(0), mDTime(0), edMode(ED_Deform), edModeOld(ED_Deform), bGuiFocus(0)
	,mThread(0)
{
	inputThreadRunning = true;
	cmdKeyPress = new CmdKey[cmd_Max];
	cmdKeyRel = new CmdKey[cmd_Max];
	cmdMouseMove = new CmdMouseMove[cmd_Max];
	cmdMousePress = new CmdMouseBtn[cmd_Max];
	cmdMouseRel = new CmdMouseBtn[cmd_Max];
}

BaseApp::~BaseApp()
{
	//delete mHWMouse;
	
	if (mGUI)  {
		mGUI->shutdown();	delete mGUI;	mGUI = 0;  }
	if (mPlatform)  {
		mPlatform->shutdown();	delete mPlatform;	mPlatform = 0;  }

	WindowEventUtilities::removeWindowEventListener(mWindow, this);
	windowClosed(mWindow);
	OGRE_DELETE mRoot;
	
	delete[] cmdKeyPress;
	delete[] cmdKeyRel;
	delete[] cmdMouseMove;
	delete[] cmdMousePress;
	delete[] cmdMouseRel;
}

//  config
//-------------------------------------------------------------------------------------
bool BaseApp::configure()
{
	if (pSet->ogre_dialog)
	{
		if (!mRoot->showConfigDialog()) return false;
		mWindow = mRoot->initialise(true, "SR Editor");
	}else{
		RenderSystem* rs;
		if (rs = mRoot->getRenderSystemByName(pSet->rendersystem))
		{
			mRoot->setRenderSystem(rs);
		}else{
			LogO("RenderSystem '" + pSet->rendersystem + "' is not available. Exiting.");
			return false;
		}
		if (pSet->rendersystem == "OpenGL Rendering Subsystem")  // not on dx
			mRoot->getRenderSystem()->setConfigOption("RTT Preferred Mode", pSet->buffer);

		mRoot->initialise(false);

		NameValuePairList settings;
		settings.insert(std::make_pair("title", "SR Editor"));
		settings.insert(std::make_pair("FSAA", toStr(pSet->fsaa)));
		settings.insert(std::make_pair("vsync", pSet->vsync ? "true" : "false"));

		mWindow = mRoot->createRenderWindow("SR Editor", pSet->windowx, pSet->windowy, pSet->fullscreen, &settings);
	}

	return true;
}

//  Setup
//-------------------------------------------------------------------------------------
bool BaseApp::setup()
{
	QTimer ti;  ti.update();  /// time
	if (pSet->rendersystem == "Default")
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		pSet->rendersystem = "Direct3D9 Rendering Subsystem";
		#else
		pSet->rendersystem = "OpenGL Rendering Subsystem";
		#endif
	}
	
	// Dynamic plugin loading
	mRoot = OGRE_NEW Root("", PATHMANAGER::GetUserConfigDir() + "/ogreset_ed.cfg", PATHMANAGER::GetUserConfigDir() + "/ogre_ed.log");

	#ifdef _DEBUG
		#define D_SUFFIX "_d"
	#else
		#define D_SUFFIX ""
	#endif

	// when show ogre dialog is on, load both rendersystems so user can select
	if (pSet->ogre_dialog)
	{
		mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
		mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_Direct3D11" + D_SUFFIX);
		#endif
	}
	else
	{
		if (pSet->rendersystem == "OpenGL Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		else if (pSet->rendersystem == "Direct3D9 Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
		else if (pSet->rendersystem == "Direct3D11 Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_Direct3D11" + D_SUFFIX);
	}

	mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/Plugin_ParticleFX" + D_SUFFIX);
	mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/Plugin_CgProgramManager" + D_SUFFIX);

	setupResources();

	if (!configure())
		return false;

	mSceneMgr = mRoot->createSceneManager(/*ST_GENERIC/**/ST_EXTERIOR_FAR/**/);
	createCamera();

	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

	createResourceListener();
	loadResources();

	//  Gui
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	mPlatform = new MyGUI::OgreD3D11Platform();
	#else
	mPlatform = new MyGUI::OgrePlatform();
	#endif

	mPlatform->initialise(mWindow, mSceneMgr, "General", PATHMANAGER::GetUserConfigDir() + "/MyGUI.log");
	mGUI = new MyGUI::Gui();
	mGUI->initialise("core.xml");
	
	MyGUI::ResourceManager::getInstance().load("MessageBoxResources.xml");
	
	
	// ------------------------- lang ------------------------
	if (pSet->language == "") // autodetect
	{	pSet->language = getSystemLanguage();
		setlocale(LC_NUMERIC, "C");  }  //needed?		
	
	// valid?
	if (!boost::filesystem::exists(PATHMANAGER::GetDataPath() + "/gui/core_language_" + pSet->language + "_tag.xml"))
		pSet->language = "en";
		
	MyGUI::LanguageManager::getInstance().setCurrentLanguage(pSet->language);
	// -------------------------------------------------------

	createFrameListener();

	ti.update();  float dt = ti.dt * 1000.f;  /// time
	LogO(String("::: Time Ogre Start: ") + toStr(dt) + " ms");

	createScene();
	postInit();

	return true;
};

//  Resources
//-------------------------------------------------------------------------------------
void BaseApp::setupResources()
{
	// Load resource paths from config file
	ConfigFile cf;
	cf.load(PATHMANAGER::GetGameConfigDir() + "/resources_ed.cfg");

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
				PATHMANAGER::GetDataPath() + "/" + archName, typeName, secName);
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
	
	bWindowResized = true;

	// adjust camera asp. ratio
	if (mCamera && mViewport)
		mCamera->setAspectRatio( float(mWindow->getWidth()) / float(mWindow->getHeight()));
	
	// write new window size to settings
	// crashed on windows when setting fullscreen on
	#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	pSet->windowx = mWindow->getWidth();
	pSet->windowy = mWindow->getHeight();
	#endif
}

void BaseApp::windowClosed(RenderWindow* rw)
{
	inputThreadRunning = false;
	mThread->join();
	
	if (rw == mWindow)
	if (mInputManager)
	{
		mInputManager->destroyInputObject( mMouse );  mMouse = 0;
		mInputManager->destroyInputObject( mKeyboard );  mKeyboard = 0;

		OIS::InputManager::destroyInputSystem(mInputManager);  mInputManager = 0;
	}
}
