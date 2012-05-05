#include "pch.h"
#include "common/Defines.h"
#include "BaseApp.h"
#include "LoadingBar.h"

#include "../vdrift/pathmanager.h"
#include "../vdrift/settings.h"
#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"
#include "common/HWMouse.h"

#include "Localization.h"
#include "SplitScreen.h"
#include "common/QTimer.h"

#include "CarModel.h"
#include "FollowCamera.h"

#include <OgreFontManager.h>
#include <OgreLogManager.h>
#include <OgreOverlayManager.h>

#include <OIS/OIS.h>
#include "../oisb/OISB.h"
#include "boost/filesystem.hpp"

#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>
#include "common/MyGUI_D3D11.h"

#include <OgreRTShaderSystem.h>
//#include "common/MaterialGen/MaterialGenerator.h"
using namespace Ogre;



//  Create
//-------------------------------------------------------------------------------------
void BaseApp::createFrameListener()
{
	LogO("*** Initializing OIS ***");

	Ogre::OverlayManager& ovr = OverlayManager::getSingleton();
	mFpsOverlay = ovr.getByName("Core/FpsOverlay");  //mFpsOverlay->show();//
	mDebugOverlay = ovr.getByName("Core/DebugOverlay");  //mDebugOverlay->show();//*
	mOvrFps = ovr.getOverlayElement("Core/CurrFps"),
	mOvrTris= ovr.getOverlayElement("Core/NumTris"),
	mOvrBat = ovr.getOverlayElement("Core/NumBatches"),
	mOvrDbg = ovr.getOverlayElement("Core/DebugText");

	InitKeyNamesMap();

	OIS::ParamList pl;	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	mWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    #if defined OIS_LINUX_PLATFORM
    if (!pSet->x11_capture_mouse)
    {
		pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
		pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
    }

    if (pSet->x11_hwmouse)
        pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
    else
        pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("true")));
    pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
    #endif

	mOISBsys = new OISB::System();
	mInputManager = OIS::InputManager::createInputSystem( pl );
	OISB::System::getSingleton().initialize(mInputManager);

	LogO("*** input load keys.xml ***");
	if (boost::filesystem::exists(PATHMANAGER::GetUserConfigDir() + "/keys.xml"))
		OISB::System::getSingleton().loadActionSchemaFromXMLFile(PATHMANAGER::GetUserConfigDir() + "/keys.xml");
	else
		OISB::System::getSingleton().loadActionSchemaFromXMLFile(PATHMANAGER::GetGameConfigDir() + "/keys-default.xml");

	LogO("*** input set callbacks ***");
	mKeyboard = OISB::System::getSingleton().getOISKeyboard();
	mMouse = OISB::System::getSingleton().getOISMouse();

	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);
	mMouse->capture();
	mKeyboard->capture();

        if (pSet->x11_hwmouse)
                mHWMouse = new HWMouse(windowHnd, 8, 8, "pointer.png");
	
	// add listener for all joysticks
	for (std::vector<OISB::JoyStick*>::iterator it=mOISBsys->mJoysticks.begin();
		it!=mOISBsys->mJoysticks.end(); ++it)
	{
		(*it)->getOISJoyStick()->setEventCallback(this);
	}
	LogO("*** input end ***");

	windowResized(mWindow);
	WindowEventUtilities::addWindowEventListener(mWindow, this);

	mRoot->addFrameListener(this);
}


//  Run
//-------------------------------------------------------------------------------------
void BaseApp::Run( bool showDialog )
{
	mShowDialog = showDialog;
	if (!setup())
		return;
	
	mRoot->startRendering();

	destroyScene();
}

//  ctor
//-------------------------------------------------------------------------------------
BaseApp::BaseApp()
	:mRoot(0), mSceneMgr(0), mWindow(0), mHDRLogic(0), mMotionBlurLogic(0),mSSAOLogic(0)
	,mGodRaysLogic(0), mSoftParticlesLogic(0), mGBufferLogic(0)
	,mDepthOfFieldLogic(0), mFilmGrainLogic(0)
	,mShaderGenerator(0),mMaterialMgrListener(0)
	,mShowDialog(1), mShutDown(false), bWindowResized(0)
	,mInputManager(0), mMouse(0), mKeyboard(0), mOISBsys(0)
	,alt(0), ctrl(0), shift(0), roadUpdTm(0.f)
	,mbLeft(0), mbRight(0), mbMiddle(0)
	,isFocGui(0),isFocRpl(0), mGUI(0), mPlatform(0)
	,mWndTabsGame(0),mWndTabsOpts(0),mWndTabsHelp(0),mWndTabsRpl(0)
	,mWndMain(0),mWndGame(0),mWndReplays(0),mWndHelp(0),mWndOpts(0)
	,mWndRpl(0), mWndChampStage(0),mWndChampEnd(0), mWndNetEnd(0)
	,bSizeHUD(true), bLoading(false), bAssignKey(false), bLoadingEnd(0)
	,mMasterClient(), mClient(), mLobbyState(DISCONNECTED)
	,mDebugOverlay(0), mFpsOverlay(0), mOvrFps(0), mOvrTris(0), mOvrBat(0), mOvrDbg(0)
	,mbShowCamPos(0), ndSky(0),	mbWireFrame(0)
	,iCurCam(0), mSplitMgr(0), motionBlurIntensity(0.9), pressedKeySender(0), materialFactory(0)
{
	mLoadingBar = new LoadingBar();

	for (int i=0; i < WND_ALL; ++i)
	{	mWndMainPanels[i] = 0;  mWndMainBtns[i] = 0;  }
}

//  dtor
//-------------------------------------------------------------------------------------
BaseApp::~BaseApp()
{
	delete materialFactory;

	//if (mSplitMgr)
		//refreshCompositor(false);

	CompositorManager::getSingleton().removeAll();
	delete mLoadingBar;
	delete mSplitMgr;

        if (pSet->x11_hwmouse)
                delete mHWMouse;
	
	if (mGUI)  {
		mGUI->shutdown();  delete mGUI;  mGUI = 0;  }
	if (mPlatform)  {
		mPlatform->shutdown();  delete mPlatform;  mPlatform = 0;  }

	WindowEventUtilities::removeWindowEventListener(mWindow, this);
	windowClosed(mWindow);
	// Unregister the material manager listener.
	if (mMaterialMgrListener != NULL)
	{			
		Ogre::MaterialManager::getSingleton().removeListener(mMaterialMgrListener);
		delete mMaterialMgrListener;
		mMaterialMgrListener = NULL;
	}

	// Finalize RTShader system.
	if (mShaderGenerator != NULL)
	{				
		Ogre::RTShader::ShaderGenerator::finalize();
		mShaderGenerator = NULL;
	}
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		mRoot->unloadPlugin("RenderSystem_Direct3D9");
		mRoot->unloadPlugin("RenderSystem_Direct3D11");
	#endif
	mRoot->unloadPlugin("RenderSystem_GL");
	
	OGRE_DELETE mRoot;
	delete mHDRLogic;  mHDRLogic = 0;
}

//  config
//-------------------------------------------------------------------------------------
bool BaseApp::configure()
{
	if (pSet->ogre_dialog)
	{
		if (!mRoot->showConfigDialog()) return false;
		mWindow = mRoot->initialise(true, "Stunt Rally");
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
		settings.insert(std::make_pair("title", "Stunt Rally"));
		settings.insert(std::make_pair("FSAA", toStr(pSet->fsaa)));
		settings.insert(std::make_pair("vsync", pSet->vsync ? "true" : "false"));

		mWindow = mRoot->createRenderWindow("Stunt Rally", pSet->windowx, pSet->windowy, pSet->fullscreen, &settings);
		//  use this in local networking tests to render when window inactive
		if (pSet->renderNotActive)
			mWindow->setDeactivateOnFocusChange(false);
	}
	mLoadingBar->bBackgroundImage = pSet->loadingbackground;
	return true;
}

//  Setup
//-------------------------------------------------------------------------------------
bool BaseApp::setup()
{
	QTimer ti;  ti.update();  /// time
	QTimer ti2;  ti2.update();  /// time2

	if (pSet->rendersystem == "Default")
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		pSet->rendersystem = "Direct3D9 Rendering Subsystem";
		#else
		pSet->rendersystem = "OpenGL Rendering Subsystem";
		#endif
	}
	
	// Dynamic plugin loading
	int net = pSet->net_local_plr;
	mRoot = OGRE_NEW Root("", PATHMANAGER::GetUserConfigDir() + "/ogreset.cfg",
		PATHMANAGER::GetUserConfigDir() + "/ogre" + (net >= 0 ? toStr(net) : "") + ".log");
	LogO("*** start setup ***");

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
		try
		{
			mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_Direct3D11" + D_SUFFIX);
		}
		catch(...)
		{
			//Ignore D3D11 plugin is not available
		}
		#endif
	}
	else
	{
		if (pSet->rendersystem == "OpenGL Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		else if (pSet->rendersystem == "Direct3D9 Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
		/*else if (pSet->rendersystem == "Direct3D11 Rendering Subsystem")
		{
			try
			{
				mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_Direct3D11" + D_SUFFIX);
			}
			catch(...)
			{
				//Ignore D3D11 plugin is not available
			}
		}/**/
	}

	mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/Plugin_ParticleFX" + D_SUFFIX);
	mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/Plugin_CgProgramManager" + D_SUFFIX);

	#ifdef _DEBUG
	Ogre::LogManager::getSingleton().setLogDetail(LL_BOREME);//
	#endif

	//RT ShaderSystem
	if (Ogre::RTShader::ShaderGenerator::initialize())
	{ 
		// Grab the shader generator pointer.
		mShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
 
		// Add the shader libs resource location.
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation(PATHMANAGER::GetDataPath()+"/RTShaderLib", "FileSystem");
 
		// Set shader cache path.
		mShaderGenerator->setShaderCachePath(PATHMANAGER::GetShaderCacheDir());		
 
		// Create and register the material manager listener.
		mMaterialMgrListener = new MaterialMgrListener(mShaderGenerator);				
		Ogre::MaterialManager::getSingleton().addListener(mMaterialMgrListener);	
	}

	setupResources();

	if (!configure())
		return false;


	mSceneMgr = mRoot->createSceneManager(/*ST_GENERIC/**/ST_EXTERIOR_FAR/**/);
	mSplitMgr = new SplitScreenManager(mSceneMgr, mWindow, pSet);

	if (mShaderGenerator != NULL)
		mShaderGenerator->addSceneManager(mSceneMgr);


	createViewports();  // calls mSplitMgr->Align();

	TextureManager::getSingleton().setDefaultNumMipmaps(5);

		ti.update();	/// time
		float dt = ti.dt * 1000.f;
		LogO(String(":::: Time setup vp: ") + toStr(dt) + " ms");


	//  Gui
	//-------------------------------------------------------
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	mPlatform = new MyGUI::OgreD3D11Platform();
	#else
	mPlatform = new MyGUI::OgrePlatform();
	#endif
	
	mPlatform->initialise(mWindow, mSceneMgr, "General", PATHMANAGER::GetUserConfigDir() + "/MyGUI.log");
	mGUI = new MyGUI::Gui();
	
	mGUI->initialise("core.xml");
	
	MyGUI::ResourceManager::getInstance().load("MessageBoxResources.xml");

	#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	//MyGUI::PointerManager::getInstance().setPointer("blank");
	#endif

		
	//------------------------- lang ------------------------
	if (pSet->language == "")  // autodetect
	{	pSet->language = getSystemLanguage();
		setlocale(LC_NUMERIC, "C");  }
	
	// valid?
	if (!boost::filesystem::exists(PATHMANAGER::GetDataPath() + "/gui/core_language_" + pSet->language + "_tag.xml"))
		pSet->language = "en";
		
	MyGUI::LanguageManager::getInstance().setCurrentLanguage(pSet->language);
	//-------------------------------------------------------

		
	mPlatform->getRenderManagerPtr()->setSceneManager(mSplitMgr->mGuiSceneMgr);
	mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumViewports);
	
	// After having initialised mygui, we can set translated strings
	setTranslations();

	//--------
	CreateRTfixed();

		ti.update();  dt = ti.dt * 1000.f;  /// time
		LogO(String(":::: Time setup gui: ") + toStr(dt) + " ms");

	createResourceListener();
	loadResources();

		ti.update();  dt = ti.dt * 1000.f;  /// time
		LogO(String(":::: Time resources: ") + toStr(dt) + " ms");

	LogO("*** createFrameListener ***");
	createFrameListener();

		ti.update();  dt = ti.dt * 1000.f;  /// time
		LogO(String(":::: Time createFrameListener: ") + toStr(dt) + " ms");

	LogO("*** createScene ***");
	createScene();

		ti.update();  dt = ti.dt * 1000.f;  /// time
		LogO(String(":::: Time createScene: ") + toStr(dt) + " ms");

	LogO("*** recreateCompositor ***");
	recreateCompositor();

	postInit();
	LogO("*** end setup ***");

		ti.update();  dt = ti.dt * 1000.f;  /// time
		LogO(String(":::: Time post, mat factory: ") + toStr(dt) + " ms");

	ti2.update();  dt = ti2.dt * 1000.f;  /// time2
	LogO(String(":::: Time setup total: ") + toStr(dt) + " ms");
	
	return true;
};

void BaseApp::destroyScene()
{
	#if OGRE_VERSION_MINOR >= 8
	Ogre::String microcodeCacheFileName =PATHMANAGER::GetCacheDir() + "/" + "shadercache.txt";
	std::fstream inp;
	inp.open(microcodeCacheFileName.c_str(), std::ios::out | std::ios::binary);
	Ogre::DataStreamPtr shaderCache (OGRE_NEW FileStreamDataStream(microcodeCacheFileName, &inp, false));
	GpuProgramManager::getSingleton().saveMicrocodeCache(shaderCache);
	#endif
}

//  Resources
//-------------------------------------------------------------------------------------
void BaseApp::setupResources()
{
	// Load resource paths from config file
	ConfigFile cf;
	std::string s = PATHMANAGER::GetGameConfigDir() + "/resources.cfg";
	cf.load(s);

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
	const bool bar = true;
	if (bar)  LoadingOn();
	
	#if OGRE_VERSION_MINOR >= 8
	bool bCache=false;
	GpuProgramManager::getSingletonPtr()->setSaveMicrocodesToCache(bCache);
	Ogre::String microcodeCacheFileName =PATHMANAGER::GetCacheDir() + "/" + "shadercache.txt";
	if (boost::filesystem::exists(microcodeCacheFileName))
	{
		std::ifstream inp;
		inp.open(microcodeCacheFileName.c_str(), std::ios::in | std::ios::binary);
		Ogre::DataStreamPtr shaderCache(OGRE_NEW FileStreamDataStream(microcodeCacheFileName, &inp, false));
		GpuProgramManager::getSingleton().loadMicrocodeCache(shaderCache);
	}
	#endif

	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	if (bar)  LoadingOff();
}


///  show / hide  Loading bar
//-------------------------------------------------------------------------------------
void BaseApp::LoadingOn()
{
	mSplitMgr->SetBackground(ColourValue(0.15,0.165,0.18));
	mSplitMgr->mGuiViewport->setBackgroundColour(ColourValue(0.15,0.165,0.18,1.0));
	mSplitMgr->mGuiViewport->setClearEveryFrame(true);
	mLoadingBar->start(mWindow, 1, 1, 1 );

	// Turn off  rendering except overlays
	mSceneMgr->clearSpecialCaseRenderQueues();
	mSceneMgr->addSpecialCaseRenderQueue(RENDER_QUEUE_OVERLAY);
	mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_INCLUDE);
}
void BaseApp::LoadingOff()
{
	// Turn On  full rendering
	mSplitMgr->SetBackground(ColourValue(0.5,0.65,0.8));
	mSplitMgr->mGuiViewport->setBackgroundColour(ColourValue(0.5,0.65,0.8));
	mSceneMgr->clearSpecialCaseRenderQueues();
	mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);
	mLoadingBar->finish();
}


//-------------------------------------------------------------------------------------
//  key, mouse, window
//-------------------------------------------------------------------------------------
bool BaseApp::keyReleased( const OIS::KeyEvent &arg )
{
	if (bAssignKey) return true;
	if (isFocGui && mGUI)  {
		MyGUI::InputManager::getInstance().injectKeyRelease(MyGUI::KeyCode::Enum(arg.key));
		return true;  }

	return true;
}

//  Mouse events
//-------------------------------------------------------------------------------------
bool BaseApp::mouseMoved( const OIS::MouseEvent &arg )
{
	if (bAssignKey) return true;
	if (IsFocGui() && mGUI)  {
		MyGUI::InputManager::getInstance().injectMouseMove(arg.state.X.abs, arg.state.Y.abs, arg.state.Z.abs);
		return true;  }

	///  Follow Camera Controls
	int i = 0;  //Log("cam: "+toStr(iCurCam));
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); ++it,++i)
	if (i == iCurCam)
	{
		if ((*it)->fCam)
			(*it)->fCam->Move( mbLeft, mbRight, mbMiddle, shift, arg.state.X.rel, arg.state.Y.rel, arg.state.Z.rel );
	}
	return true;
}

using namespace OIS;
bool BaseApp::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (bAssignKey) return true;
	if (IsFocGui() && mGUI)  {
		MyGUI::InputManager::getInstance().injectMousePress(arg.state.X.abs, arg.state.Y.abs, MyGUI::MouseButton::Enum(id));
		return true;  }

	if		(id == MB_Left)		mbLeft = true;
	else if (id == MB_Right)	mbRight = true;
	else if (id == MB_Middle)	mbMiddle = true;
	return true;
}

bool BaseApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (bAssignKey) return true;
	if (IsFocGui() && mGUI)  {
		MyGUI::InputManager::getInstance().injectMouseRelease(arg.state.X.abs, arg.state.Y.abs, MyGUI::MouseButton::Enum(id));
		return true;  }

	if		(id == MB_Left)		mbLeft = false;
	else if (id == MB_Right)	mbRight = false;
	else if (id == MB_Middle)	mbMiddle = false;
	return true;
}

//  adjust mouse clipping area
//-------------------------------------------------------
void BaseApp::windowResized(RenderWindow* rw)
{
	unsigned int width, height, depth;  int left, top;
	rw->getMetrics(width, height, depth, left, top);

	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;  ms.height = height;
			
	// adjust hud
	bSizeHUD = true;
	bWindowResized = true;
	
	// Adjust viewports
	//mSplitMgr->AdjustRatio();
	mSplitMgr->Align();
	
	// write new window size to settings
	// crashed on windows when setting fullscreen on
	#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	pSet->windowx = mWindow->getWidth();
	pSet->windowy = mWindow->getHeight();
	#endif
}

void BaseApp::windowClosed(RenderWindow* rw)
{
	if (rw == mWindow)
	if (mInputManager)
	{
		OISB::System::getSingleton().saveActionSchemaToXMLFile(PATHMANAGER::GetUserConfigDir() + "/keys.xml");
		OISB::System::getSingleton().finalize();
		delete mOISBsys;  mOISBsys = 0;
		mInputManager = 0;
	}
}

//  mouse cursor
//-------------------------------------------------------
void BaseApp::showMouse()
{
	if (!mGUI)  return;

        if (pSet->x11_hwmouse)
                mHWMouse->show();
	
	#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
        if (pSet->x11_hwmouse)
                MyGUI::PointerManager::getInstance().setVisible(false);
        else
                MyGUI::PointerManager::getInstance().setVisible(true);
	#else
	MyGUI::PointerManager::getInstance().setVisible(true);
	#endif
}
void BaseApp::hideMouse()
{
	if (!mGUI)  return;

        if (pSet->x11_hwmouse)
                mHWMouse->hide();
                
	MyGUI::PointerManager::getInstance().setVisible(false);
}

void BaseApp::updMouse()
{
	if (IsFocGui())
		showMouse();
	else
		hideMouse();
}
