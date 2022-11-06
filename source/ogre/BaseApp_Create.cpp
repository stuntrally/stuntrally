#include "pch.h"
#include "common/Def_Str.h"
#include "BaseApp.h"
#include "LoadingBar.h"

#include "../vdrift/pathmanager.h"
#include "../settings.h"
#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"

#include "Localization.h"
#include "SplitScreen.h"
#include "Compositor.h"

#include "CarModel.h"
#include "FollowCamera.h"

#include <OgreLogManager.h>
#include <MyGUI_Prerequest.h>
#if OGRE_VERSION >= MYGUI_DEFINE_VERSION(1, 9, 0)
#include <OgreOverlaySystem.h>
#endif
#include "boost/filesystem.hpp"

#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>
//#include "common/MyGUI_D3D11.h"

#include <OgreTimer.h>
#include <OgreOverlayManager.h>
#include <OgreTimer.h>
#include "Compositor.h"

#include "../shiny/Main/Factory.hpp"
#include "../shiny/Platforms/Ogre/OgrePlatform.hpp"

#include "../sdl4ogre/sdlinputwrapper.hpp"
#include "../sdl4ogre/sdlcursormanager.hpp"
#include "../sdl4ogre/sdlwindowhelper.hpp"

#include "common/PointerFix.h"
#include "../oics/ICSInputControlSystem.h"
using namespace Ogre;


namespace
{
	std::vector<unsigned long> utf8ToUnicode(const std::string& utf8)
	{
		std::vector<unsigned long> unicode;
		size_t i = 0;
		while (i < utf8.size())
		{
			unsigned long uni;  size_t todo;
			unsigned char ch = utf8[i++];

				 if (ch <= 0x7F){	uni = ch;	todo = 0;	}
			else if (ch <= 0xBF){	throw std::logic_error("not a UTF-8 string");	}
			else if (ch <= 0xDF){	uni = ch&0x1F;	todo = 1;	}
			else if (ch <= 0xEF){	uni = ch&0x0F;	todo = 2;	}
			else if (ch <= 0xF7){	uni = ch&0x07;	todo = 3;	}
			else				{	throw std::logic_error("not a UTF-8 string");	}

			for (size_t j = 0; j < todo; ++j)
			{
				if (i == utf8.size())	throw std::logic_error("not a UTF-8 string");
				unsigned char ch = utf8[i++];
				if (ch < 0x80 || ch > 0xBF)  throw std::logic_error("not a UTF-8 string");
				uni <<= 6;
				uni += ch & 0x3F;
			}
			if (uni >= 0xD800 && uni <= 0xDFFF)  throw std::logic_error("not a UTF-8 string");
			if (uni > 0x10FFFF)  throw std::logic_error("not a UTF-8 string");
			unicode.push_back(uni);
		}
		return unicode;
	}

	MyGUI::MouseButton sdlButtonToMyGUI(Uint8 button)
	{
		//  The right button is the second button, according to MyGUI
		if (button == SDL_BUTTON_RIGHT)  button = SDL_BUTTON_MIDDLE;
		else if (button == SDL_BUTTON_MIDDLE)  button = SDL_BUTTON_RIGHT;
		//  MyGUI's buttons are 0 indexed
		return MyGUI::MouseButton::Enum(button - 1);
	}
}


//  Create
//-------------------------------------------------------------------------------------
void BaseApp::createFrameListener()
{
	mInputWrapper = new SFO::InputWrapper(mSDLWindow, mWindow);
	mInputWrapper->setMouseEventCallback(this);
	mInputWrapper->setKeyboardEventCallback(this);
	mInputWrapper->setJoyEventCallback(this);
	mInputWrapper->setWindowEventCallback(this);
	mCursorManager = new SFO::SDLCursorManager();
	onCursorChange(MyGUI::PointerManager::getInstance().getDefaultPointer());
	mCursorManager->setEnabled(true);

	std::string file = PATHMANAGER::UserConfigDir()+"/input.xml";
	mInputCtrl = new ICS::InputControlSystem(file, true, mBindListner, NULL, 100);

	for (int j=0; j<SDL_NumJoysticks(); ++j)
		mInputCtrl->addJoystick(j);
	for (int i=0; i<4; ++i)
	{
		file = PATHMANAGER::UserConfigDir()+"/input_p" + toStr(i) + ".xml";
		mInputCtrlPlayer[i] = new ICS::InputControlSystem(file, true, mBindListner, NULL, 100);
		for (int j=0; j<SDL_NumJoysticks(); ++j)
			mInputCtrlPlayer[i]->addJoystick(j);
	}

	bSizeHUD = true;
	bWindowResized = true;
	mSplitMgr->Align();

	mRoot->addFrameListener(this);
}


//  Run
//-------------------------------------------------------------------------------------
void BaseApp::Run(bool showDialog)
{
	mShowDialog = showDialog;
	if (!setup())
		return;
	
	if (!pSet->limit_fps)
		mRoot->startRendering();  // default
	else
	{	Ogre::Timer tim;
		while (1)
		{
			WindowEventUtilities::messagePump();
			long min_fps = 1000000.0 / std::max(10.f, pSet->limit_fps_val);
			if (tim.getMicroseconds() > min_fps)
			{
				tim.reset();
				if (!mRoot->renderOneFrame())
					break;
			}else
			if (pSet->limit_sleep >= 0)
				boost::this_thread::sleep(boost::posix_time::milliseconds(pSet->limit_sleep));
	}	}

	destroyScene();
}


//  ctor
//-------------------------------------------------------------------------------------
BaseApp::BaseApp()
	:mMasterClient(), mClient()
{
	mLoadingBar = new LoadingBar(this);
}

//  dtor
//-------------------------------------------------------------------------------------
BaseApp::~BaseApp()
{
	delete mFactory;
	//if (mSplitMgr)
		//refreshCompositor(false);

	CompositorManager::getSingleton().removeAll();
	delete mLoadingBar;
	delete mSplitMgr;
	
	if (mGui)  {
		mGui->shutdown();  delete mGui;  mGui = 0;  }
	if (mPlatform)  {
		mPlatform->shutdown();  delete mPlatform;  mPlatform = 0;  }

	//  save inputs
	mInputCtrl->save(PATHMANAGER::UserConfigDir() + "/input.xml");
	delete mInputCtrl;
	for (int i=0; i<4; ++i)
	{
		mInputCtrlPlayer[i]->save(PATHMANAGER::UserConfigDir() + "/input_p" + toStr(i) + ".xml");
		delete mInputCtrlPlayer[i];
	}

	delete mInputWrapper;
	delete mCursorManager;

	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		mRoot->unloadPlugin("RenderSystem_Direct3D9");
		//mRoot->unloadPlugin("RenderSystem_Direct3D11");
	#endif
	mRoot->unloadPlugin("RenderSystem_GL");


	OGRE_DELETE mRoot;
	delete mHDRLogic;  mHDRLogic = 0;

	SDL_SetWindowFullscreen(mSDLWindow, 0);
	SDL_DestroyWindow(mSDLWindow);
}


//  config
//-------------------------------------------------------------------------------------
bool BaseApp::configure()
{
	RenderSystem* rs;
	if (rs = mRoot->getRenderSystemByName(pSet->rendersystem))
	{
		mRoot->setRenderSystem(rs);
	}else
	{	LogO("RenderSystem '" + pSet->rendersystem + "' is not available. Exiting.");
		return false;
	}
	if (pSet->rendersystem == "OpenGL Rendering Subsystem")
		mRoot->getRenderSystem()->setConfigOption("RTT Preferred Mode", pSet->buffer);

	mRoot->initialise(false);

	Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE;
	if (SDL_WasInit(flags) == 0)
	{
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
		if (SDL_Init(flags) != 0)
			throw std::runtime_error("Could not initialize SDL! " + std::string(SDL_GetError()));
	}

	//  Enable joystick events
	SDL_JoystickEventState(SDL_ENABLE);
	//  Open all available joysticks.  TODO: open them when they are required
	for (int i=0; i<SDL_NumJoysticks(); ++i)
	{
		SDL_Joystick* js = SDL_JoystickOpen(i);
		if (js)
		{
			mJoysticks.push_back(js);
			const char* s = SDL_JoystickName(js);
			int axes = SDL_JoystickNumAxes(js);
			int btns = SDL_JoystickNumButtons(js);
			//SDL_JoystickNumBalls SDL_JoystickNumHats
			LogO(String("<Joystick> name: ")+s+"  axes: "+toStr(axes)+"  buttons: "+toStr(btns));
		}
	}
	SDL_StartTextInput();

	NameValuePairList params;
	params.insert(std::make_pair("title", "Stunt Rally"));
	params.insert(std::make_pair("FSAA", toStr(pSet->fsaa)));
	params.insert(std::make_pair("vsync", pSet->vsync ? "true" : "false"));

	int pos_x = SDL_WINDOWPOS_UNDEFINED,
		pos_y = SDL_WINDOWPOS_UNDEFINED;

#if 0  /// _tool_ rearrange window pos for local netw testing
	SDL_Rect screen;
	if (SDL_GetDisplayBounds(/*pSet.screen_id*/0, &screen) != 0)
		LogO("SDL_GetDisplayBounds errror");
		
	if (pSet->net_local_plr <= 0)
	{	pos_x = 0;  pos_y = 0;
	}else
	{	pos_x = screen.w - pSet->windowx;
		pos_y = screen.h - pSet->windowy;
	}
#endif
	/// \todo For multiple monitors, WINDOWPOS_UNDEFINED is not the best idea. Needs a setting which screen to launch on,
	/// then place the window on that screen (derive x&y pos from SDL_GetDisplayBounds)


	//  Create an application window with the following settings:
	mSDLWindow = SDL_CreateWindow(
		"Stunt Rally", pos_x, pos_y, pSet->windowx, pSet->windowy,
		SDL_WINDOW_SHOWN | (pSet->fullscreen ? SDL_WINDOW_FULLSCREEN : 0) | SDL_WINDOW_RESIZABLE);

	SFO::SDLWindowHelper helper(mSDLWindow, pSet->windowx, pSet->windowy, "Stunt Rally", pSet->fullscreen, params);
	helper.setWindowIcon("stuntrally.png");
	mWindow = helper.getWindow();

	return true;
}


//  Setup
//-------------------------------------------------------------------------------------
bool BaseApp::setup()
{
	Ogre::Timer ti,ti2;
	LogO("*** start setup ***");
	
	if (pSet->rendersystem == "Default")
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		pSet->rendersystem = "Direct3D9 Rendering Subsystem";
		#else
		pSet->rendersystem = "OpenGL Rendering Subsystem";
		#endif
	}

	#ifdef _DEBUG
	#define D_SUFFIX ""  // "_d"
	#else
	#define D_SUFFIX ""
	#endif

	//  when show ogre dialog is on, load both rendersystems so user can select
	if (pSet->ogre_dialog)
	{
		mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
		#endif
	}else{
		if (pSet->rendersystem == "OpenGL Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		else if (pSet->rendersystem == "Direct3D9 Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
	}

	mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/Plugin_ParticleFX" + D_SUFFIX);
#if defined(OGRE_VERSION) && OGRE_VERSION >= 0x10B00
    //mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/Codec_STBI" + D_SUFFIX);  // only png
    mRoot->loadPlugin(PATHMANAGER::OgrePluginDir() + "/Codec_FreeImage" + D_SUFFIX);  // for jpg screenshots
#endif

	#ifdef _DEBUG
	LogManager::getSingleton().setMinLogLevel(LML_TRIVIAL);  // all
	#endif

	setupResources();

	if (!configure())
		return false;


	mSceneMgr = mRoot->createSceneManager("DefaultSceneManager");

	#if OGRE_VERSION >= MYGUI_DEFINE_VERSION(1, 9, 0) 
	OverlaySystem* pOverlaySystem = new OverlaySystem();
	mSceneMgr->addRenderQueueListener(pOverlaySystem);
	#endif

	mSplitMgr = new SplitScr(mSceneMgr, mWindow, pSet);

	createViewports();  // calls mSplitMgr->Align();

	TextureManager::getSingleton().setDefaultNumMipmaps(5);

		LogO(String(":::: Time setup vp: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();


	//  Gui
	baseInitGui();

		LogO(String(":::: Time setup gui: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();

	createResourceListener();


	LogO("*** createFrameListener ***");
	createFrameListener();

		LogO(String(":::: Time createFrameListener: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();

	LogO("*** createScene ***");
	createScene();

		LogO(String(":::: Time createScene: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();

	LogO("*** recreateCompositor, does nothing ***");
	recreateCompositor();

	LogO("*** end setup ***");


	///  material factory setup
	sh::OgrePlatform* platform = new sh::OgrePlatform("General", PATHMANAGER::Data() + "/materials");
	platform->setCacheFolder(PATHMANAGER::ShaderDir());

	mFactory = new sh::Factory(platform);

	postInit();

		LogO(String(":::: Time post, mat factory: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();

	loadResources();

		LogO(String(":::: Time resources: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();  

	LogO(String(":::: Time setup total: ") + fToStr(ti2.getMilliseconds(),0,3) + " ms");
	
	return true;
}

void BaseApp::destroyScene()
{
}

//  Resources
//-------------------------------------------------------------------------------------
void BaseApp::setupResources()
{
	// Load resource paths from config file
	ConfigFile cf;
	std::string s = PATHMANAGER::GameConfigDir() +
		(pSet->tex_size > 0 ? "/resources.cfg" : "/resources_s.cfg");
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
				PATHMANAGER::Data() + "/" + archName, typeName, secName);
	}	}

#if defined(OGRE_VERSION) && OGRE_VERSION >= 0x10C00
	// create stubs for Ogre 1.12 core shaders (unused by stuntrally)
	auto& gpm = HighLevelGpuProgramManager::getSingleton();
	for(auto name : {"PointLight", "DirLight", "PointLightFinite", "DirLightFinite"})
	    gpm.createProgram("Ogre/ShadowExtrude"+String(name), "OgreInternal", "unified", GPT_VERTEX_PROGRAM);
	gpm.createProgram("Ogre/ShadowBlendVP", "OgreInternal", "unified", GPT_VERTEX_PROGRAM);
	gpm.createProgram("Ogre/ShadowBlendFP", "OgreInternal", "unified", GPT_FRAGMENT_PROGRAM);
#endif
#if defined(OGRE_MIN_VERSION)
#if OGRE_MIN_VERSION(13, 3, 0)
	auto& matm = MaterialManager::getSingleton();
	for (auto name : {"Ogre/Debug/ShadowVolumes", "Ogre/StencilShadowVolumes", "Ogre/StencilShadowModulationPass", "Ogre/TextureShadowCaster"})
	{
		auto mat = matm.create(name, "OgreInternal");
		mat->createTechnique()->createPass()->setVertexProgram("Ogre/ShadowBlendVP");
	}
#endif
#endif
}

void BaseApp::createResourceListener()
{
}
void BaseApp::loadResources()
{
	LoadingOn();
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	LoadingOff();
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
	mSplitMgr->SetBackground(ColourValue(0.2,0.3,0.4));
	mSplitMgr->mGuiViewport->setBackgroundColour(ColourValue(0.2,0.3,0.4));
	mSceneMgr->clearSpecialCaseRenderQueues();
	mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);
	mLoadingBar->finish();
}


//-------------------------------------------------------------------------------------
//  key, mouse, window
//-------------------------------------------------------------------------------------

bool BaseApp::keyReleased(const SDL_KeyboardEvent& arg)
{
	mInputCtrl->keyReleased(arg);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->keyReleased(arg);

	if (bAssignKey) return true;

	if (mGui && (isFocGui || isTweak()))
	{
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
	mInputCtrl->mouseMoved(arg);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->mouseMoved(arg);

	if (bAssignKey)  return true;

	mMouseX = arg.x;
	mMouseY = arg.y;

	if (IsFocGui() && mGui)  {
		MyGUI::InputManager::getInstance().injectMouseMove(arg.x, arg.y, arg.z);
		return true;  }

	///  Follow Camera Controls
	int i = 0;  //Log("cam: "+toStr(iCurCam));
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); ++it,++i)
		if (i == iCurCam && (*it)->fCam)
			(*it)->fCam->Move( mbLeft, mbRight, mbMiddle, shift, arg.xrel, arg.yrel, arg.zrel );

	return true;
}

bool BaseApp::mousePressed( const SDL_MouseButtonEvent& arg, Uint8 id )
{
	mInputCtrl->mousePressed(arg, id);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->mousePressed(arg, id);

	if (bAssignKey)  return true;
	if (IsFocGui() && mGui)  {
		MyGUI::InputManager::getInstance().injectMousePress(arg.x, arg.y, sdlButtonToMyGUI(id));
		return true;  }

	if		(id == SDL_BUTTON_LEFT)		mbLeft = true;
	else if (id == SDL_BUTTON_RIGHT)	mbRight = true;
	else if (id == SDL_BUTTON_MIDDLE)	mbMiddle = true;
	return true;
}

bool BaseApp::mouseReleased( const SDL_MouseButtonEvent& arg, Uint8 id )
{
	mInputCtrl->mouseReleased(arg, id);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->mouseReleased(arg, id);

	if (bAssignKey)  return true;
	if (IsFocGui() && mGui)  {
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

	if (isFocGui || isTweak())
	for (std::vector<unsigned long>::iterator it = unicode.begin(); it != unicode.end(); ++it)
		MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::None, *it);
}

bool BaseApp::axisMoved(const SDL_JoyAxisEvent &arg, int axis)
{
	mInputCtrl->axisMoved(arg, axis);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->axisMoved(arg, axis);
	return true;
}

bool BaseApp::buttonPressed(const SDL_JoyButtonEvent &evt, int button)
{
	mInputCtrl->buttonPressed(evt, button);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->buttonPressed(evt, button);
	return true;
}

bool BaseApp::buttonReleased(const SDL_JoyButtonEvent &evt, int button)
{
	mInputCtrl->buttonReleased(evt, button);
	for (int i=0; i<4; ++i)  mInputCtrlPlayer[i]->buttonReleased(evt, button);
	return true;
}

//  mouse cursor
//-------------------------------------------------------
void BaseApp::showMouse()
{	
	mInputWrapper->setMouseVisible(true);
}
void BaseApp::hideMouse()
{                
	mInputWrapper->setMouseVisible(false);
}

void BaseApp::updMouse()
{
	if (IsFocGui())	showMouse();
	else			hideMouse();

	mInputWrapper->setAllowGrab(pSet->mouse_capture);

	mInputWrapper->setMouseRelative(!IsFocGui());
	mInputWrapper->setGrabPointer(!IsFocGui());
}

void BaseApp::onCursorChange(const std::string &name)
{
	if (!mCursorManager->cursorChanged(name))
		return; //the cursor manager doesn't want any more info about this cursor
	//See if we can get the information we need out of the cursor resource
	ResourceImageSetPointerFix* imgSetPtr = dynamic_cast<ResourceImageSetPointerFix*>(MyGUI::PointerManager::getInstance().getByName(name));
	if (imgSetPtr != NULL)
	{
		MyGUI::ResourceImageSet* imgSet = imgSetPtr->getImageSet();
		std::string tex_name = imgSet->getIndexInfo(0,0).texture;
		TexturePtr tex = TextureManager::getSingleton().getByName(tex_name);

		//everything looks good, send it to the cursor manager
		if (tex)
		{
			Uint8 size_x = imgSetPtr->getSize().width;
			Uint8 size_y = imgSetPtr->getSize().height;
			Uint8 left = imgSetPtr->getTexturePosition().left;
			Uint8 top = imgSetPtr->getTexturePosition().top;
			Uint8 hotspot_x = imgSetPtr->getHotSpot().left;
			Uint8 hotspot_y = imgSetPtr->getHotSpot().top;

			mCursorManager->receiveCursorInfo(name, tex, left, top, size_x, size_y, hotspot_x, hotspot_y);
	}	}
}

void BaseApp::windowResized(int x, int y)
{
	pSet->windowx = x;  pSet->windowy = y;
	bWindowResized = true;
	
	// Adjust viewports
	mSplitMgr->Align();
	mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumViewports);
}

void BaseApp::windowClosed()
{
	Root::getSingleton().queueEndRendering();
}


///  base Init Gui
//--------------------------------------------------------------------------------------------------------------
void BaseApp::baseInitGui()
{
	using namespace MyGUI;
	mPlatform = new MyGUI::OgrePlatform();
	mPlatform->initialise(mWindow, mSceneMgr, "General", PATHMANAGER::UserConfigDir() + "/MyGUI.log");
	mGui = new MyGUI::Gui();

	mGui->initialise("");

	MyGUI::FactoryManager::getInstance().registerFactory<ResourceImageSetPointerFix>("Resource", "ResourceImageSetPointer");
	MyGUI::ResourceManager::getInstance().load("core.xml");

	MyGUI::PointerManager::getInstance().eventChangeMousePointer +=	MyGUI::newDelegate(this, &BaseApp::onCursorChange);
	MyGUI::PointerManager::getInstance().setVisible(false);

		
	//------------------------ lang
	if (pSet->language == "")  // autodetect
	{	pSet->language = getSystemLanguage();
		setlocale(LC_NUMERIC, "C");  }
	
	if (!boost::filesystem::exists(PATHMANAGER::Data() + "/gui/core_language_" + pSet->language + "_tag.xml"))
		pSet->language = "en";  // use en if not found
		
	MyGUI::LanguageManager::getInstance().setCurrentLanguage(pSet->language);
	//------------------------

		
	mPlatform->getRenderManagerPtr()->setSceneManager(mSplitMgr->mGuiSceneMgr);
	mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumViewports);


	///  create widgets
	//------------------------------------------------
	//  Fps
	bckFps = mGui->createWidget<ImageBox>("ImageBox",
		0,0, 212,25, Align::Default, "Pointer", "FpsB");
	bckFps->setImageTexture("back_fps.png");

	txFps = bckFps->createWidget<TextBox>("TextBox",
		1,1, 212,25, Align::Default, "FpsT");
	txFps->setFontName("hud.fps");

	bckFps->setVisible(false);


	//  loading
	bckLoad = mGui->createWidget<ImageBox>("ImageBox",
		100,100, 500,110, Align::Default, "Pointer", "LoadBck");
	bckLoad->setImageTexture("loading_back.jpg");

	barSizeX = 480;
	bckLoadBar = bckLoad->createWidget<ImageBox>("ImageBox",
		10,43, 480,26, Align::Default, "LoadBckBar");
	bckLoadBar->setImageTexture("loading_bar2.jpg");
	bckLoadBar->setColour(Colour(0.5,0.5,0.5,1));

	barSizeY = 22;
	barLoad = bckLoadBar->createWidget<ImageBox>("ImageBox",
		0,2, 30,22, Align::Default, "LoadBar");
	barLoad->setImageTexture("loading_bar1.jpg");


	txLoadBig = bckLoad->createWidget<TextBox>("TextBox",
		10,8, 400,30, Align::Default, "LoadTbig");
	txLoadBig->setFontName("hud.text");  txLoadBig->setTextColour(Colour(0.7,0.83,1));
	txLoadBig->setCaption(TR("#{LoadingDesc}"));

	txLoad = bckLoad->createWidget<TextBox>("TextBox",
		10,77, 400,24, Align::Default, "LoadT");
	txLoad->setFontName("hud.text");  txLoad->setTextColour(Colour(0.65,0.78,1));
	txLoad->setCaption(TR("#{Initializing}..."));


	///  menu background image
	//  dont show for autoload and no loadingbackground
	if (!(!pSet->loadingbackground && pSet->autostart))
	{
		imgBack = mGui->createWidget<ImageBox>("ImageBox",
			0,0, 800,600, Align::Default, "Back","ImgBack");
		imgBack->setImageTexture("background.jpg");
	}

	///  loading background img
	imgLoad = mGui->createWidget<ImageBox>("ImageBox",
		0,0, 800,600, Align::Default, "Back", "ImgLoad");
	//imgLoad->setImageTexture("background.png");
	//imgLoad->setVisible(true);

	baseSizeGui();
}


///  size gui (on resolution change)
//-------------------------------------------------------------------
void BaseApp::baseSizeGui()
{
	int sx = mWindow->getWidth(), sy = mWindow->getHeight();
	bckLoad->setPosition(sx/2 - 250/*200*/, sy - 140);

	//imgBack->setCoord(0,0, sx, sy);
	//return;

	//  fit image to window, preserve aspect
	int ix = 1920, iy = 1200;  // get org img size ...
	int six, siy;  // sized to window
	int oix=0, oiy=0;  // offset pos
	float sa = float(sx)/sy, si = float(ix)/iy;  // aspects
	
	if (si >= sa)  // wider than screen
	{
		siy = sy;  six = si * siy;  // six/siy = si
		oix = (six - sx) / 2;
	}else
	{	six = sx;  siy = six / si;
		oiy = (siy - sy) / 2;
	}
	imgLoad->setCoord(-oix, -oiy, six, siy);
	if (imgBack)
	imgBack->setCoord(-oix, -oiy, six, siy);
}
