#include "pch.h"
#include "common/Defines.h"
#include "BaseApp.h"
#include "LoadingBar.h"

#include "../vdrift/pathmanager.h"
#include "../vdrift/settings.h"
#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"

#include "Localization.h"
#include "SplitScreen.h"
#include "common/QTimer.h"
#include "Compositor.h"

#include "CarModel.h"
#include "FollowCamera.h"

#include <OgreFontManager.h>
#include <OgreLogManager.h>
#include <OgreOverlayManager.h>

#include "boost/filesystem.hpp"

#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>
#include "common/MyGUI_D3D11.h"

#include <OgreRTShaderSystem.h>
#include "Compositor.h"

#include "../shiny/Main/Factory.hpp"
#include "../shiny/Platforms/Ogre/OgrePlatform.hpp"

#include "../sdl4ogre/sdlinputwrapper.hpp"
#include "../sdl4ogre/sdlcursormanager.hpp"

#include <SDL_syswm.h>

#include "ResourceImageSetPointerFix.h"


namespace
{
	std::vector<unsigned long> utf8ToUnicode(const std::string& utf8)
	{
		std::vector<unsigned long> unicode;
		size_t i = 0;
		while (i < utf8.size())
		{
			unsigned long uni;
			size_t todo;
			unsigned char ch = utf8[i++];
			if (ch <= 0x7F)
			{
				uni = ch;
				todo = 0;
			}
			else if (ch <= 0xBF)
			{
				throw std::logic_error("not a UTF-8 string");
			}
			else if (ch <= 0xDF)
			{
				uni = ch&0x1F;
				todo = 1;
			}
			else if (ch <= 0xEF)
			{
				uni = ch&0x0F;
				todo = 2;
			}
			else if (ch <= 0xF7)
			{
				uni = ch&0x07;
				todo = 3;
			}
			else
			{
				throw std::logic_error("not a UTF-8 string");
			}
			for (size_t j = 0; j < todo; ++j)
			{
				if (i == utf8.size())
					throw std::logic_error("not a UTF-8 string");
				unsigned char ch = utf8[i++];
				if (ch < 0x80 || ch > 0xBF)
					throw std::logic_error("not a UTF-8 string");
				uni <<= 6;
				uni += ch & 0x3F;
			}
			if (uni >= 0xD800 && uni <= 0xDFFF)
				throw std::logic_error("not a UTF-8 string");
			if (uni > 0x10FFFF)
				throw std::logic_error("not a UTF-8 string");
			unicode.push_back(uni);
		}
		return unicode;
	}

	MyGUI::MouseButton sdlButtonToMyGUI(Uint8 button)
	{
		//The right button is the second button, according to MyGUI
		if(button == SDL_BUTTON_RIGHT)
			button = SDL_BUTTON_MIDDLE;
		else if(button == SDL_BUTTON_MIDDLE)
			button = SDL_BUTTON_RIGHT;

		//MyGUI's buttons are 0 indexed
		return MyGUI::MouseButton::Enum(button - 1);
	}
}


//  Create
//-------------------------------------------------------------------------------------
void BaseApp::createFrameListener()
{
	Ogre::OverlayManager& ovr = Ogre::OverlayManager::getSingleton();
	mFpsOverlay = ovr.getByName("Core/FpsOverlay");  //mFpsOverlay->show();//
	mDebugOverlay = ovr.getByName("Core/DebugOverlay");  //mDebugOverlay->show();//*
	mOvrFps = ovr.getOverlayElement("Core/CurrFps");	mOvrTris= ovr.getOverlayElement("Core/NumTris");
	mOvrBat = ovr.getOverlayElement("Core/NumBatches"); mOvrMem = ovr.getOverlayElement("Core/Memory");
	mOvrDbg = ovr.getOverlayElement("Core/DebugText");


	mInputWrapper = new SFO::InputWrapper(mSDLWindow, mWindow);
	mInputWrapper->setMouseEventCallback(this);
	mInputWrapper->setKeyboardEventCallback(this);
	mCursorManager = new SFO::SDLCursorManager();
	mCursorManager->setEnabled(true);
	onCursorChange(MyGUI::PointerManager::getInstance().getDefaultPointer());

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
	:mRoot(0), mSceneMgr(0), mWindow(0), imgBack(0)
	,mHDRLogic(0), mMotionBlurLogic(0),mSSAOLogic(0), mCameraBlurLogic(0)
	,mGodRaysLogic(0), mSoftParticlesLogic(0), mGBufferLogic(0)
	,mDepthOfFieldLogic(0), mFilmGrainLogic(0)
	,mShaderGenerator(0),mMaterialMgrListener(0)
	,mShowDialog(1), mShutDown(false), bWindowResized(0)
	,alt(0), ctrl(0), shift(0), roadUpdTm(0.f)
	,mbLeft(0), mbRight(0), mbMiddle(0)
	,isFocGui(0),isFocRpl(0), mGUI(0), mPlatform(0)
	,mWndTabsGame(0),mWndTabsOpts(0),mWndTabsHelp(0),mWndTabsRpl(0)
	,mWndMain(0),mWndGame(0),mWndReplays(0),mWndHelp(0),mWndOpts(0)
	,mWndRpl(0), mWndChampStage(0),mWndChampEnd(0), mWndNetEnd(0), mWndTweak(0)
	,bSizeHUD(true), bLoading(false), iLoad1stFrames(0), bAssignKey(false), bLoadingEnd(0)
	,mMasterClient(), mClient(), mLobbyState(DISCONNECTED)
	,mDebugOverlay(0), mFpsOverlay(0), mOvrFps(0), mOvrTris(0), mOvrBat(0), mOvrMem(0), mOvrDbg(0)
	,mbShowCamPos(0), ndSky(0),	mbWireFrame(0)
	,iCurCam(0), mSplitMgr(0), motionBlurIntensity(0.9), pressedKeySender(0)
	,mMouseX(0), mMouseY(0), mCursorManager(NULL), mInputWrapper(NULL)
{
	mLoadingBar = new LoadingBar();

	for (int i=0; i < ciMainBtns; ++i)
	{	mWndMainPanels[i] = 0;  mWndMainBtns[i] = 0;  }
}

//  dtor
//-------------------------------------------------------------------------------------
BaseApp::~BaseApp()
{
	SDL_SetWindowFullscreen(mSDLWindow, 0);
	SDL_DestroyWindow(mSDLWindow);

	delete mFactory;
	//if (mSplitMgr)
		//refreshCompositor(false);

	Ogre::CompositorManager::getSingleton().removeAll();
	delete mLoadingBar;
	delete mSplitMgr;
	
	if (mGUI)  {
		mGUI->shutdown();  delete mGUI;  mGUI = 0;  }
	if (mPlatform)  {
		mPlatform->shutdown();  delete mPlatform;  mPlatform = 0;  }

	delete mInputWrapper;
	delete mCursorManager;

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
		//mRoot->unloadPlugin("RenderSystem_Direct3D11");
	#endif
	mRoot->unloadPlugin("RenderSystem_GL");


	OGRE_DELETE mRoot;
	delete mHDRLogic;  mHDRLogic = 0;
}

//  config
//-------------------------------------------------------------------------------------
bool BaseApp::configure()
{
	Ogre::RenderSystem* rs;
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

	Uint32 flags = SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE;
	if(SDL_WasInit(flags) == 0)
	{
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
		if(SDL_Init(flags) != 0)
		{
			throw std::runtime_error("Could not initialize SDL! " + std::string(SDL_GetError()));
		}
	}

	Ogre::NameValuePairList params;
	params.insert(std::make_pair("title", "Stunt Rally"));
	params.insert(std::make_pair("FSAA", toStr(pSet->fsaa)));
	params.insert(std::make_pair("vsync", pSet->vsync ? "true" : "false"));

	int pos_x = SDL_WINDOWPOS_UNDEFINED,
		pos_y = SDL_WINDOWPOS_UNDEFINED;

	/// \todo For multiple monitors, WINDOWPOS_UNDEFINED is not the best idea. Needs a setting which screen to launch on,
	/// then place the window on that screen (derive x&y pos from SDL_GetDisplayBounds)+
	/*
	if(pSet->fullscreen)
	{
		SDL_Rect display_bounds;
		if(SDL_GetDisplayBounds(settings.screen, &display_bounds) != 0)
			throw std::runtime_error("Couldn't get display bounds!");
		pos_x = display_bounds.x;
		pos_y = display_bounds.y;
	}
	*/

	// Create an application window with the following settings:
	mSDLWindow = SDL_CreateWindow(
	  "Stunt Rally",                  //    window title
	  pos_x,                     //    initial x position
	  pos_y,                     //    initial y position
	  pSet->windowx,                               //    width, in pixels
	  pSet->windowy,                               //    height, in pixels
	  SDL_WINDOW_SHOWN
		| (pSet->fullscreen ? SDL_WINDOW_FULLSCREEN : 0)
	);

	//get the native whnd
	struct SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);

	if(-1 == SDL_GetWindowWMInfo(mSDLWindow, &wmInfo))
		throw std::runtime_error("Couldn't get WM Info!");

	Ogre::String winHandle;

	switch(wmInfo.subsystem)
	{
#ifdef WIN32
	case SDL_SYSWM_WINDOWS:
		// Windows code
		winHandle = Ogre::StringConverter::toString((unsigned long)wmInfo.info.win.window);
		break;
#elif __MACOSX__
	case SDL_SYSWM_COCOA:
		//required to make OGRE play nice with our window
		params.insert(std::make_pair("macAPI", "cocoa"));
		params.insert(std::make_pair("macAPICocoaUseNSView", "true"));

		winHandle  = Ogre::StringConverter::toString(WindowContentViewHandle(wmInfo));
		break;
#else
	case SDL_SYSWM_X11:
		winHandle = Ogre::StringConverter::toString((unsigned long)wmInfo.info.x11.window);
		break;
#endif
	default:
		throw std::runtime_error("Unexpected WM!");
		break;
	}

	/// \todo externalWindowHandle is deprecated according to the source code. Figure out a way to get parentWindowHandle
	/// to work properly. On Linux/X11 it causes an occasional GLXBadDrawable error.
	params.insert(std::make_pair("externalWindowHandle",  winHandle));

	mWindow = mRoot->createRenderWindow("Stunt Rally", pSet->windowx, pSet->windowy, pSet->fullscreen, &params);




	//  use this in local networking tests to render when window inactive
	/// \todo
	//if (pSet->renderNotActive)
		//mWindow->setDeactivateOnFocusChange(false);

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
	mRoot = OGRE_NEW Ogre::Root("", PATHMANAGER::UserConfigDir() + "/ogreset.cfg",
		PATHMANAGER::UserConfigDir() + "/ogre" + (net >= 0 ? toStr(net) : "") + ".log");
	LogO("*** start setup ***");

	#ifdef _DEBUG
	#define D_SUFFIX "_d"
	#else
	#define D_SUFFIX ""
	#endif

	// when show ogre dialog is on, load both rendersystems so user can select
	if (pSet->ogre_dialog)
	{
		mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
		/*try
		{	mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_Direct3D11" + D_SUFFIX);
		} catch(...) {  }/**/
		#endif
	}
	else
	{
		if (pSet->rendersystem == "OpenGL Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		else if (pSet->rendersystem == "Direct3D9 Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
		/*else if (pSet->rendersystem == "Direct3D11 Rendering Subsystem")
		try
		{	mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_Direct3D11" + D_SUFFIX);
		} catch(...) {  }/**/
	}

	mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/Plugin_ParticleFX" + D_SUFFIX);
	//mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/Plugin_OctreeSceneManager" + D_SUFFIX);  // test, bad

	#ifdef _DEBUG
	Ogre::LogManager::getSingleton().setLogDetail(LL_BOREME);//
	#endif

	//RT ShaderSystem
	if (Ogre::RTShader::ShaderGenerator::initialize())
	{ 
		// Grab the shader generator pointer.
		mShaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
 
		// Add the shader libs resource location.
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation(PATHMANAGER::Data()+"/RTShaderLib", "FileSystem");
 
		// Set shader cache path.
		mShaderGenerator->setShaderCachePath(PATHMANAGER::ShaderDir());		
 
		// Create and register the material manager listener.
		mMaterialMgrListener = new MaterialMgrListener(mShaderGenerator);
		Ogre::MaterialManager::getSingleton().addListener(mMaterialMgrListener);
	}

	setupResources();

	if (!configure())
		return false;


	mSceneMgr = mRoot->createSceneManager(/*ST_GENERIC/**/Ogre::ST_EXTERIOR_FAR/**/);
	mSplitMgr = new SplitScreenManager(mSceneMgr, mWindow, pSet);

	if (mShaderGenerator != NULL)
		mShaderGenerator->addSceneManager(mSceneMgr);


	createViewports();  // calls mSplitMgr->Align();

	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

		ti.update();	/// time
		float dt = ti.dt * 1000.f;
		LogO(Ogre::String(":::: Time setup vp: ") + fToStr(dt,0,3) + " ms");


	//  Gui
	//-------------------------------------------------------
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	mPlatform = new MyGUI::OgreD3D11Platform();
	#else
	mPlatform = new MyGUI::OgrePlatform();
	#endif
	
	mPlatform->initialise(mWindow, mSceneMgr, "General", PATHMANAGER::UserConfigDir() + "/MyGUI.log");
	mGUI = new MyGUI::Gui();
	

	mGUI->initialise("");

	MyGUI::FactoryManager::getInstance().registerFactory<ResourceImageSetPointerFix>("Resource", "ResourceImageSetPointer");
	MyGUI::ResourceManager::getInstance().load("core.xml");
	MyGUI::ResourceManager::getInstance().load("MessageBoxResources.xml");

	MyGUI::PointerManager::getInstance().eventChangeMousePointer +=
			MyGUI::newDelegate(this, &BaseApp::onCursorChange);
	MyGUI::PointerManager::getInstance().setVisible(false);

		
	//------------------------- lang ------------------------
	if (pSet->language == "")  // autodetect
	{	pSet->language = getSystemLanguage();
		setlocale(LC_NUMERIC, "C");  }
	
	// valid?
	if (!boost::filesystem::exists(PATHMANAGER::Data() + "/gui/core_language_" + pSet->language + "_tag.xml"))
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
		LogO(Ogre::String(":::: Time setup gui: ") + fToStr(dt,0,3) + " ms");

	createResourceListener();
	loadResources();

		ti.update();  dt = ti.dt * 1000.f;  /// time
		LogO(Ogre::String(":::: Time resources: ") + fToStr(dt,0,3) + " ms");

	LogO("*** createFrameListener ***");
	createFrameListener();

		ti.update();  dt = ti.dt * 1000.f;  /// time
		LogO(Ogre::String(":::: Time createFrameListener: ") + fToStr(dt,0,3) + " ms");

	LogO("*** createScene ***");
	createScene();

		ti.update();  dt = ti.dt * 1000.f;  /// time
		LogO(Ogre::String(":::: Time createScene: ") + fToStr(dt,0,3) + " ms");

	LogO("*** recreateCompositor ***");
	recreateCompositor();

	LogO("*** end setup ***");


	///  material factory setup
	sh::OgrePlatform* platform = new sh::OgrePlatform("General", PATHMANAGER::Data() + "/materials");
	platform->setCacheFolder(PATHMANAGER::ShaderDir());

	mFactory = new sh::Factory(platform);

	postInit();

		ti.update();  dt = ti.dt * 1000.f;  /// time
		LogO(Ogre::String(":::: Time post, mat factory: ") + fToStr(dt,0,3) + " ms");

	ti2.update();  dt = ti2.dt * 1000.f;  /// time2
	LogO(Ogre::String(":::: Time setup total: ") + fToStr(dt,0,3) + " ms");
	
	return true;
}

void BaseApp::destroyScene()
{
	bool bCache = false;

	if (bCache)
	{
		Ogre::String file = PATHMANAGER::ShaderDir() + "/shadercache.txt";
		std::fstream inp;
		inp.open(file.c_str(), std::ios::out | std::ios::binary);
		Ogre::DataStreamPtr shaderCache (OGRE_NEW Ogre::FileStreamDataStream(file, &inp, false));
		Ogre::GpuProgramManager::getSingleton().saveMicrocodeCache(shaderCache);
	}
}

//  Resources
//-------------------------------------------------------------------------------------
void BaseApp::setupResources()
{
	// Load resource paths from config file
	Ogre::ConfigFile cf;
	std::string s = PATHMANAGER::GameConfigDir() +
		(pSet->tex_size > 0 ? "/resources.cfg" : "/resources_s.cfg");
	cf.load(s);

	// Go through all sections & settings in the file
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
				PATHMANAGER::Data() + "/" + archName, typeName, secName);
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
	
	bool bCache=false;
	Ogre::GpuProgramManager::getSingletonPtr()->setSaveMicrocodesToCache(bCache);
	if (bCache)
	{
		Ogre::String file = PATHMANAGER::ShaderDir() + "/shadercache.txt";
		if (boost::filesystem::exists(file) && bCache)
		{
			std::ifstream inp;
			inp.open(file.c_str(), std::ios::in | std::ios::binary);
			Ogre::DataStreamPtr shaderCache(OGRE_NEW Ogre::FileStreamDataStream(file, &inp, false));
			Ogre::GpuProgramManager::getSingleton().loadMicrocodeCache(shaderCache);
		}
	}

	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	if (bar)  LoadingOff();
}


///  show / hide  Loading bar
//-------------------------------------------------------------------------------------
void BaseApp::LoadingOn()
{
	mSplitMgr->SetBackground(Ogre::ColourValue(0.15,0.165,0.18));
	mSplitMgr->mGuiViewport->setBackgroundColour(Ogre::ColourValue(0.15,0.165,0.18,1.0));
	mSplitMgr->mGuiViewport->setClearEveryFrame(true);
	mLoadingBar->start(mWindow, 1, 1, 1 );

	// Turn off  rendering except overlays
	mSceneMgr->clearSpecialCaseRenderQueues();
	mSceneMgr->addSpecialCaseRenderQueue(Ogre::RENDER_QUEUE_OVERLAY);
	mSceneMgr->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_INCLUDE);
}
void BaseApp::LoadingOff()
{
	// Turn On  full rendering
	mSplitMgr->SetBackground(Ogre::ColourValue(0.5,0.65,0.8));
	mSplitMgr->mGuiViewport->setBackgroundColour(Ogre::ColourValue(0.5,0.65,0.8));
	mSceneMgr->clearSpecialCaseRenderQueues();
	mSceneMgr->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_EXCLUDE);
	mLoadingBar->finish();
}


//-------------------------------------------------------------------------------------
//  key, mouse, window
//-------------------------------------------------------------------------------------

bool BaseApp::keyReleased(const SDL_KeyboardEvent& arg)
{
	if (bAssignKey) return true;

	if (mGUI && (isFocGui || isTweak()))  {

		OIS::KeyCode kc = mInputWrapper->sdl2OISKeyCode(arg.keysym.sym);

		MyGUI::InputManager::getInstance().injectKeyRelease(MyGUI::KeyCode::Enum(kc));
		return true;
	}

	return true;
}

//  Mouse events
//-------------------------------------------------------------------------------------

bool BaseApp::mouseMoved(const SFO::MouseMotionEvent &arg)
{
	if (bAssignKey)  return true;

	mMouseX = arg.x;
	mMouseY = arg.y;

	if (IsFocGui() && mGUI)  {
		MyGUI::InputManager::getInstance().injectMouseMove(arg.x, arg.y, arg.z);
		return true;  }

	///  Follow Camera Controls
	int i = 0;  //Log("cam: "+toStr(iCurCam));
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); ++it,++i)
	if (i == iCurCam)
	{
		if ((*it)->fCam)
			(*it)->fCam->Move( mbLeft, mbRight, mbMiddle, shift, arg.xrel, arg.yrel, arg.zrel );
	}
	return true;
}

bool BaseApp::mousePressed( const SDL_MouseButtonEvent& arg, Uint8 id )
{
	if (bAssignKey)  return true;
	if (IsFocGui() && mGUI)  {
		MyGUI::InputManager::getInstance().injectMousePress(arg.x, arg.y, sdlButtonToMyGUI(id));
		return true;  }

	if		(id == SDL_BUTTON_LEFT)		mbLeft = true;
	else if (id == SDL_BUTTON_RIGHT)	mbRight = true;
	else if (id == SDL_BUTTON_MIDDLE)	mbMiddle = true;
	return true;
}

bool BaseApp::mouseReleased( const SDL_MouseButtonEvent& arg, Uint8 id )
{
	if (bAssignKey)  return true;
	if (IsFocGui() && mGUI)  {
		MyGUI::InputManager::getInstance().injectMouseRelease(arg.x, arg.y, sdlButtonToMyGUI(id));
		return true;  }

	if		(id == SDL_BUTTON_LEFT)		mbLeft = false;
	else if (id == SDL_BUTTON_RIGHT)	mbRight = false;
	else if (id == SDL_BUTTON_MIDDLE)	mbMiddle = false;
	return true;
}

void BaseApp::textInput(const SDL_TextInputEvent &arg)
{
	const char* text = &arg.text[0];
	std::vector<unsigned long> unicode = utf8ToUnicode(std::string(text));
	for (std::vector<unsigned long>::iterator it = unicode.begin(); it != unicode.end(); ++it)
		MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::Enum(0x00), *it);
		// ^ Should be MyGUI::KeyCode::None, but X11 defines a 'None' macro :(
		// TODO: Refactor the code so that SDL_syswm.h (which includes X11) is not needed here,
		// so that it can be changed back to MyGUI::KeyCode::None
}

//  mouse cursor
//-------------------------------------------------------
void BaseApp::showMouse()
{	
	mCursorManager->cursorVisibilityChange(true);
}
void BaseApp::hideMouse()
{                
	mCursorManager->cursorVisibilityChange(false);
}

void BaseApp::updMouse()
{
	if (IsFocGui())
		showMouse();
	else
		hideMouse();

	mInputWrapper->setMouseRelative(!IsFocGui());
	mInputWrapper->setGrabPointer(!IsFocGui());
}

void BaseApp::onCursorChange(const std::string &name)
{
	if(!mCursorManager->cursorChanged(name))
		return; //the cursor manager doesn't want any more info about this cursor
	//See if we can get the information we need out of the cursor resource
	ResourceImageSetPointerFix* imgSetPtr = dynamic_cast<ResourceImageSetPointerFix*>(MyGUI::PointerManager::getInstance().getByName(name));
	if(imgSetPtr != NULL)
	{
		MyGUI::ResourceImageSet* imgSet = imgSetPtr->getImageSet();

		std::string tex_name = imgSet->getIndexInfo(0,0).texture;

		Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().getByName(tex_name);

		//everything looks good, send it to the cursor manager
		if(!tex.isNull())
		{
			Uint8 size_x = imgSetPtr->getSize().width;
			Uint8 size_y = imgSetPtr->getSize().height;
			Uint8 left = imgSetPtr->getTexturePosition().left;
			Uint8 top = imgSetPtr->getTexturePosition().top;
			Uint8 hotspot_x = imgSetPtr->getHotSpot().left;
			Uint8 hotspot_y = imgSetPtr->getHotSpot().top;

			mCursorManager->receiveCursorInfo(name, tex, left, top, size_x, size_y, hotspot_x, hotspot_y);
		}
	}

}
