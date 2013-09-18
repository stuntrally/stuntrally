#include "pch.h"
#include "../ogre/common/Defines.h"

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include "BaseApp.h"
#include "CApp.h" //

#include "../vdrift/pathmanager.h"
#include "../ogre/Localization.h"
#include <OgreConfigFile.h>
#if OGRE_VERSION >= MYGUI_DEFINE_VERSION(1, 9, 0) 
#include <OgreOverlaySystem.h>
#endif
#include <OgreOverlayManager.h>

#include "../ogre/common/MyGUI_D3D11.h"
#include "../sdl4ogre/sdlinputwrapper.hpp"
#include "../sdl4ogre/sdlcursormanager.hpp"
#include "../sdl4ogre/sdlwindowhelper.hpp"

#include "../ogre/common/PointerFix.h"


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
		if (button == SDL_BUTTON_RIGHT)
			button = SDL_BUTTON_MIDDLE;
		else if (button == SDL_BUTTON_MIDDLE)
			button = SDL_BUTTON_RIGHT;

		//MyGUI's buttons are 0 indexed
		return MyGUI::MouseButton::Enum(button - 1);
	}
}



//  Camera
//-------------------------------------------------------------------------------------
void BaseApp::createCamera()
{
	mCamera = mSceneMgr->createCamera("Cam");
	mCamera->setPosition(Ogre::Vector3(0,00,100));
	mCamera->lookAt(Ogre::Vector3(0,0,0));
	mCamera->setNearClipDistance(0.5f);

	mViewport = mWindow->addViewport(mCamera);
	//mViewport->setBackgroundColour(Ogre::ColourValue(0.5,0.65,0.8));  //`
	mViewport->setBackgroundColour(Ogre::ColourValue(0.2,0.3,0.4));  //`
	Ogre::Real asp = Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight());
	mCamera->setAspectRatio(asp);
}


//  Frame
//-------------------------------------------------------------------------------------
void BaseApp::createFrameListener()
{
	Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing ***");

	Ogre::OverlayManager& ovr = Ogre::OverlayManager::getSingleton();  Ogre::Overlay* o;
	o = ovr.getByName("Editor/FpsOverlay");  if (o) o->show();
	o = ovr.getByName("Editor/FocusOverlay");  if (o) o->show();
	o = ovr.getByName("Editor/StatusOverlay");  if (o) o->show();
	ovSt = ovr.getOverlayElement("Editor/StatusPanel");  if (ovSt)  ovSt->hide();
	ovStat= ovr.getOverlayElement("Editor/StatusText");
	ovFocus = ovr.getOverlayElement("Editor/FocusText"); ovFocBck = ovr.getOverlayElement("Editor/FocusPanel");
		ovPos= ovr.getOverlayElement("Editor/Pos");
	mDebugOverlay= ovr.getByName("Editor/DebugOverlay");  //mDebugOverlay->show();
	ovDbg = ovr.getOverlayElement("Editor/DebugText");
	ovInfo= ovr.getOverlayElement("Editor/Info");

	ovBrushPrv = ovr.getByName("Editor/BrushPrvOverlay");  //ovBrushPrv->show();
	ovBrushMtr = ovr.getOverlayElement("Editor/BrushPrvPanel");
	ovTerPrv = ovr.getByName("Editor/TerPrvOverlay");  ovTerPrv->hide();
	ovTerMtr = ovr.getOverlayElement("Editor/TerPrvPanel");


	mInputWrapper = new SFO::InputWrapper(mSDLWindow, mWindow);
	mInputWrapper->setMouseEventCallback(this);
	mInputWrapper->setKeyboardEventCallback(this);
	mInputWrapper->setWindowEventCallback(this);
	mCursorManager = new SFO::SDLCursorManager();
	mCursorManager->setEnabled(true);
	onCursorChange(MyGUI::PointerManager::getInstance().getDefaultPointer());


	mRoot->addFrameListener(this);}

void BaseApp::destroyScene()
{
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
	:mRoot(0), mCamera(0), mViewport(0)
	,mSceneMgr(0), mWindow(0), imgCur(0)
	,bckFps(0), txFps(0)
	,mShowDialog(1), mShutDown(false), bWindowResized(true), bFirstRenderFrame(true)
	,alt(0), ctrl(0), shift(0)
	,mbLeft(0), mbRight(0), mbMiddle(0)
	,ndSky(0), road(0)

	,mDebugOverlay(0), ovSt(0)
	,ovPos(0), ovDbg(0), ovInfo(0), ovStat(0)
	,ovFocus(0),ovFocBck(0), ovBrushPrv(0),ovBrushMtr(0), ovTerPrv(0),ovTerMtr(0)

	,mbWireFrame(0), mx(0),my(0),mz(0),  mGUI(0), mPlatform(0)

	,mWndMain(0),mWndEdit(0),mWndHelp(0),mWndOpts(0)
	,mWndTabsEdit(0),mWndTabsHelp(0),mWndTabsOpts(0)
	,mWndBrush(0), mWndCam(0), mWndStart(0)
	,mWndRoadCur(0), mWndRoadStats(0)
	,mWndFluids(0), mWndObjects(0), mWndRivers(0)
	
	,pSet(0), bMoveCam(0), mDTime(0), edMode(ED_Deform), edModeOld(ED_Deform), bGuiFocus(0)
	,mInputWrapper(NULL), mSDLWindow(NULL)
{
}

BaseApp::~BaseApp()
{	
	if (mGUI)  {
		mGUI->shutdown();	delete mGUI;	mGUI = 0;  }
	if (mPlatform)  {
		mPlatform->shutdown();	delete mPlatform;	mPlatform = 0;  }

	delete mRoot;
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
	if (SDL_WasInit(flags) == 0)
	{
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
		if (SDL_Init(flags) != 0)
			throw std::runtime_error("Could not initialize SDL! " + std::string(SDL_GetError()));
	}
	SDL_StartTextInput();


	Ogre::NameValuePairList params;
	params.insert(std::make_pair("title", "SR Editor"));
	params.insert(std::make_pair("FSAA", toStr(pSet->fsaa)));
	params.insert(std::make_pair("vsync", pSet->vsync ? "true" : "false"));

	int pos_x = SDL_WINDOWPOS_UNDEFINED,
		pos_y = SDL_WINDOWPOS_UNDEFINED;

	/// \todo For multiple monitors, WINDOWPOS_UNDEFINED is not the best idea. Needs a setting which screen to launch on,
	/// then place the window on that screen (derive x&y pos from SDL_GetDisplayBounds)+
	/*
	if (pSet->fullscreen)
	{
		SDL_Rect display_bounds;
		if (SDL_GetDisplayBounds(settings.screen, &display_bounds) != 0)
			throw std::runtime_error("Couldn't get display bounds!");
		pos_x = display_bounds.x;
		pos_y = display_bounds.y;
	}
	*/

	//  Create window
	mSDLWindow = SDL_CreateWindow(
		"SR Editor", pos_x, pos_y, pSet->windowx, pSet->windowy,
		SDL_WINDOW_SHOWN | (pSet->fullscreen ? SDL_WINDOW_FULLSCREEN : 0) | SDL_WINDOW_RESIZABLE);

	SFO::SDLWindowHelper helper(mSDLWindow, pSet->windowx, pSet->windowy, "SR Editor", pSet->fullscreen, params);
	helper.setWindowIcon("sr-editor.png");
	mWindow = helper.getWindow();

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
	
	//  Dynamic plugin loading
	mRoot = OGRE_NEW Ogre::Root("", PATHMANAGER::UserConfigDir() + "/ogreset_ed.cfg", PATHMANAGER::UserConfigDir() + "/ogre_ed.log");
	//LogManager::getSingleton().setLogDetail(LL_BOREME);  //-

	#ifdef _DEBUG
		#define D_SUFFIX "_d"
	#else
		#define D_SUFFIX ""
	#endif

	//  when show ogre dialog is on, load both rendersystems so user can select
	if (pSet->ogre_dialog)
	{
		mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
		mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_Direct3D11" + D_SUFFIX);
		#endif
	}else{
		if (pSet->rendersystem == "OpenGL Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		else if (pSet->rendersystem == "Direct3D9 Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
		else if (pSet->rendersystem == "Direct3D11 Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_Direct3D11" + D_SUFFIX);
	}

	mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/Plugin_ParticleFX" + D_SUFFIX);

	setupResources();

	if (!configure())
		return false;

	mSceneMgr = mRoot->createSceneManager(/*ST_GENERIC/**/Ogre::ST_EXTERIOR_FAR/**/);

	#if OGRE_VERSION >= MYGUI_DEFINE_VERSION(1, 9, 0) 
	Ogre::OverlaySystem* pOverlaySystem = new Ogre::OverlaySystem();
	mSceneMgr->addRenderQueueListener(pOverlaySystem);
	#endif

	createCamera();

	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	mSceneMgr->setFog(Ogre::FOG_NONE);

	createResourceListener();
	loadResources();

	baseInitGui();

	createFrameListener();

	ti.update();  float dt = ti.dt * 1000.f;  /// time
	LogO(Ogre::String("::: Time Ogre Start: ") + fToStr(dt,0,3) + " ms");

	createScene();

	return true;
}

//  Resources
//-------------------------------------------------------------------------------------
void BaseApp::setupResources()
{
	//  Load resource paths from config file
	Ogre::ConfigFile cf;
	std::string s = PATHMANAGER::GameConfigDir() +
		(pSet->tex_size > 0 ? "/resources_ed.cfg" : "/resources_s_ed.cfg");
	cf.load(s);

	//  Go through all sections & settings in the file
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
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}


//  key, mouse, window
//-------------------------------------------------------------------------------------

bool BaseApp::keyReleased( const SDL_KeyboardEvent &arg )
{
	//if (bGuiFocus)
		MyGUI::InputManager::getInstance().injectKeyRelease(MyGUI::KeyCode::Enum(mInputWrapper->sdl2OISKeyCode(arg.keysym.sym)));
	return true;
}

void BaseApp::textInput(const SDL_TextInputEvent &arg)
{
	const char* text = &arg.text[0];
	if (*text == '`')  return;
	std::vector<unsigned long> unicode = utf8ToUnicode(std::string(text));
	if (bGuiFocus)
	for (std::vector<unsigned long>::iterator it = unicode.begin(); it != unicode.end(); ++it)
		MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::None, *it);
}


//  Mouse
//-------------------------------------------------------------------------------------
bool BaseApp::mouseMoved( const SFO::MouseMotionEvent &arg )
{
	mx += arg.xrel;  my += arg.yrel;  mz += arg.zrel / 50;
	//if (bGuiFocus)
		MyGUI::InputManager::getInstance().injectMouseMove(arg.x, arg.y, arg.z);
	return true;
}

bool BaseApp::mousePressed( const SDL_MouseButtonEvent &arg, Uint8 id )
{
	if (bGuiFocus)
		MyGUI::InputManager::getInstance().injectMousePress(arg.x, arg.y, sdlButtonToMyGUI(id));
	if (id == SDL_BUTTON_LEFT)			mbLeft = true;
	else if (id == SDL_BUTTON_RIGHT)	mbRight = true;
	else if (id == SDL_BUTTON_MIDDLE)	mbMiddle = true;
	return true;

}

bool BaseApp::mouseReleased( const SDL_MouseButtonEvent &arg, Uint8 id )
{
	//if (bGuiFocus)
		MyGUI::InputManager::getInstance().injectMouseRelease(arg.x, arg.y, sdlButtonToMyGUI(id));
	if (id == SDL_BUTTON_LEFT)			mbLeft = false;
	else if (id == SDL_BUTTON_RIGHT)	mbRight = false;
	else if (id == SDL_BUTTON_MIDDLE)	mbMiddle = false;
	return true;
}


void BaseApp::onCursorChange(const std::string &name)
{
	if (!mCursorManager->cursorChanged(name))
		return;  // the cursor manager doesn't want any more info about this cursor
	//  See if we can get the information we need out of the cursor resource
	ResourceImageSetPointerFix* imgSetPtr = dynamic_cast<ResourceImageSetPointerFix*>(MyGUI::PointerManager::getInstance().getByName(name));
	if (imgSetPtr != NULL)
	{
		MyGUI::ResourceImageSet* imgSet = imgSetPtr->getImageSet();
		std::string tex_name = imgSet->getIndexInfo(0,0).texture;
		Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().getByName(tex_name);

		//  everything looks good, send it to the cursor manager
		if (!tex.isNull())
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

void BaseApp::windowResized(int x, int y)
{
	bWindowResized = true;

	// adjust camera asp. ratio
	if (mCamera && mViewport)
		mCamera->setAspectRatio( float(x) / float(y));
	mPlatform->getRenderManagerPtr()->setActiveViewport(0);
}



///  base Init Gui
//--------------------------------------------------------------------------------------------------------------
void BaseApp::baseInitGui()
{
	using namespace MyGUI;
	//  Gui
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
	
	//------------------------ lang
	if (pSet->language == "")  // autodetect
	{	pSet->language = getSystemLanguage();
		setlocale(LC_NUMERIC, "C");  }
	
	if (!boost::filesystem::exists(PATHMANAGER::Data() + "/gui/core_language_" + pSet->language + "_tag.xml"))
		pSet->language = "en";  // use en if not found
		
	MyGUI::LanguageManager::getInstance().setCurrentLanguage(pSet->language);
	//------------------------


	///  create widgets
	//------------------------------------------------
	//  Fps
	bckFps = mGUI->createWidget<ImageBox>("ImageBox",
		0,0, 212,25, Align::Default, "Pointer", "FpsB");
	bckFps->setImageTexture("Border_Center.png");

	txFps = mGUI->createWidget<TextBox>("TextBox",
		1,1, 212,25, Align::Default, "Pointer", "FpsT");
	txFps->setFontName("fps.17");

}
