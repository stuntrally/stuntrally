#include "pch.h"
#include "Defines.h"
#include "BaseApp.h"
#include "LoadingBar.h"
#include "FollowCamera.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/settings.h"

#include "Compositor.h"
#include "Localization.h"
#include "SplitScreen.h"
#include "CarModel.h"

#include <OgreFontManager.h>
#include <OgreLogManager.h>
#include <OgreOverlayManager.h>

#include <OIS/OIS.h>
#include "../oisb/OISB.h"
#include "boost/filesystem.hpp"

#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>

using namespace Ogre;

//#define LogDbg(s)
#define LogDbg(s)  LogO(s)


//  Camera
//-------------------------------------------------------------------------------------
void BaseApp::createCamera()
{
}

//  Frame
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
		pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("true")));
		pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
	}
    pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
    #endif

	mOISBsys = new OISB::System();
	mInputManager = OIS::InputManager::createInputSystem( pl );
	OISB::System::getSingleton().initialize(mInputManager);

	LogDbg("*** input load keys.xml ***");
	if (boost::filesystem::exists(PATHMANAGER::GetUserConfigDir() + "/keys.xml"))
		OISB::System::getSingleton().loadActionSchemaFromXMLFile(PATHMANAGER::GetUserConfigDir() + "/keys.xml");
	else
		OISB::System::getSingleton().loadActionSchemaFromXMLFile(PATHMANAGER::GetGameConfigDir() + "/keys-default.xml");

	LogDbg("*** input set callbacks ***");
	mKeyboard = OISB::System::getSingleton().getOISKeyboard();
	mMouse = OISB::System::getSingleton().getOISMouse();

	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);
	
	// add listener for all joysticks
	for (std::vector<OISB::JoyStick*>::iterator it=mOISBsys->mJoysticks.begin();
		it!=mOISBsys->mJoysticks.end(); it++)
	{
		(*it)->getOISJoyStick()->setEventCallback(this);
	}
	LogDbg("*** input end ***");

	windowResized(mWindow);
	WindowEventUtilities::addWindowEventListener(mWindow, this);

	mRoot->addFrameListener(this);
}

void BaseApp::createViewports()
{
	mSplitMgr->mNumViewports = pSet->local_players;
	mSplitMgr->Align();
}

///  Compositor
//-------------------------------------------------------------------------------------
void BaseApp::refreshCompositor(bool disableAll)
{
	for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); it++)
	{
		CompositorManager::getSingleton().setCompositorEnabled((*it), "Bloom", false);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "HDR", false);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "ssao", false);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "Motion Blur", false);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "SSAA", false);
	}

	if (!pSet->all_effects || disableAll)
		return;
	
	//  Set Bloom params (intensity, orig weight)
	try
	{	MaterialPtr bloom = MaterialManager::getSingleton().getByName("Ogre/Compositor/BloomBlend2");
		GpuProgramParametersSharedPtr params = bloom->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
		params->setNamedConstant("OriginalImageWeight", pSet->bloomorig);
		params->setNamedConstant("BlurWeight", pSet->bloomintensity);
	}catch(...)
	{	LogO("!!! Failed to set bloom shader params.");  }
	
	//  HDR params todo..
	//try
	//{	MaterialPtr hdrmat = MaterialManager::getSingleton().getByName("Ogre/Compositor/BloomBlend2");
	//	GpuProgramParametersSharedPtr params = hdrmat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
	//	params->setNamedConstant("Bloom", pSet->);
	//}catch(...)
	//{	LogO("!!! Failed to set hdr shader params.");  }

	//  Set Motion Blur intens
	//try
	//{	MaterialPtr blur = MaterialManager::getSingleton().getByName("Ogre/Compositor/Combine");
	//	GpuProgramParametersSharedPtr params = blur->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
	//	params->setNamedConstant("blur", pSet->motionblurintensity);
	//}catch(...)
	//{	LogO("!!! Failed to set blur shader params.");  }
	

	for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); it++)
	{
		CompositorManager::getSingleton().setCompositorEnabled((*it), "Bloom", pSet->bloom);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "HDR", pSet->hdr);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "Motion Blur", pSet->motionblur);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "SSAA", pSet->ssaa);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "ssao", pSet->ssao);
	}
}

//-------------------------------------------------------------------------------------
void BaseApp::recreateCompositor()
{
	if (!pSet->all_effects)  // disable compositor
	{
		refreshCompositor();
		return;
	}
	
	//  add when needed
	//if (!ResourceGroupManager::getSingleton().isResourceGroupInitialised("Effects"))
	{
		std::string sPath = PATHMANAGER::GetDataPath() + "/compositor";
		mRoot->addResourceLocation(sPath, "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/bloom", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/hdr", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/motionblur", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/ssaa", "FileSystem", "Effects");
		mRoot->addResourceLocation(sPath + "/ssao", "FileSystem", "Effects");
		ResourceGroupManager::getSingleton().initialiseResourceGroup("Effects");
	}

	// hdr has to be first in the compositor queue
	if (!mHDRLogic) 
	{
		mHDRLogic = new HDRLogic;
		CompositorManager::getSingleton().registerCompositorLogic("HDR", mHDRLogic);
	}
	
	if (!mSSAOLogic) 
	{
		mSSAOLogic = new SSAOLogic();
		mSSAOLogic->setApp(this);
		CompositorManager::getSingleton().registerCompositorLogic("ssao", mSSAOLogic);
	}

	if (CompositorManager::getSingleton().getByName("Motion Blur").isNull())
	{
		// Motion blur has to be created in code
		CompositorPtr comp3 = CompositorManager::getSingleton().create(
			"Motion Blur", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		CompositionTechnique *t = comp3->createTechnique();
		t->setCompositorLogicName("Motion Blur");
		{
			CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("scene");
			def->width = 0;
			def->height = 0;
			def->formatList.push_back(PF_R8G8B8);
		}
		{
			CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("sum");
			def->width = 0;
			def->height = 0;
			def->formatList.push_back(PF_R8G8B8);
		}
		{
			CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("temp");
			def->width = 0;
			def->height = 0;
			def->formatList.push_back(PF_R8G8B8);
		}
		/// Render scene
		{
			CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
			tp->setOutputName("scene");
		}
		/// Initialisation pass for sum texture
		{
			CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
			tp->setOutputName("sum");
			tp->setOnlyInitial(true);
		}
		/// Do the motion blur
		{
			CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			tp->setOutputName("temp");
			{ CompositionPass *pass = tp->createPass();
			pass->setType(CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("Ogre/Compositor/Combine");
			pass->setIdentifier(120);
			pass->setInput(0, "scene");
			pass->setInput(1, "sum");
			}
		}
		/// Copy back sum texture
		{
			CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			tp->setOutputName("sum");
			{ CompositionPass *pass = tp->createPass();
			pass->setType(CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("Ogre/Compositor/Copyback");
			pass->setInput(0, "temp");
			}
		}
		/// Display result
		{
			CompositionTargetPass *tp = t->getOutputTargetPass();
			tp->setInputMode(CompositionTargetPass::IM_NONE);
			{ CompositionPass *pass = tp->createPass();
			pass->setType(CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("Ogre/Compositor/MotionBlur");
			pass->setInput(0, "sum");
			}
		}
	}


	if (!mMotionBlurLogic)
	{
		LogO("Creating motion blur logic");
		Ogre::LogManager::getSingleton().logMessage("Creating MotionBlurLogic");
		mMotionBlurLogic = new MotionBlurLogic(this);
		CompositorManager::getSingleton().registerCompositorLogic("Motion Blur", mMotionBlurLogic);
	}


	for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); it++)
	{
		// remove old comp. first
		CompositorManager::getSingleton().removeCompositorChain( (*it ));
		
		CompositorManager::getSingleton().addCompositor((*it), "HDR");
		CompositorManager::getSingleton().addCompositor((*it), "ssao");
		CompositorManager::getSingleton().addCompositor((*it), "Bloom");
		CompositorManager::getSingleton().addCompositor((*it), "Motion Blur");
		CompositorManager::getSingleton().addCompositor((*it), "SSAA");
	}
	
	refreshCompositor();
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
	,mShowDialog(1), mShutDown(false), bWindowResized(0)
	,mInputManager(0), mMouse(0), mKeyboard(0), mOISBsys(0)
	,alt(0), ctrl(0), shift(0), roadUpCnt(0)
	,mbLeft(0), mbRight(0), mbMiddle(0)
	,isFocGui(0),isFocRpl(0), mGUI(0), mPlatform(0)
	,mWndOpts(0), mWndTabs(0), mWndRpl(0)
	,bSizeHUD(true), bLoading(false), bAssignKey(false)

	,mDebugOverlay(0), mFpsOverlay(0), mOvrFps(0), mOvrTris(0), mOvrBat(0), mOvrDbg(0)
	,mbShowCamPos(0), ndSky(0),	mbWireFrame(0)
	,iCurCam(0), mSplitMgr(0), motionBlurIntensity(0.9)
{
	mLoadingBar = new LoadingBar();
}

BaseApp::~BaseApp()
{
	delete mLoadingBar;
	delete mSplitMgr;
	
	if (mGUI)  {
		mGUI->shutdown();	delete mGUI;	mGUI = 0;  }
	if (mPlatform)  {
		mPlatform->shutdown();	delete mPlatform;	mPlatform = 0;  }

	WindowEventUtilities::removeWindowEventListener(mWindow, this);
	windowClosed(mWindow);
	
	
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
	}
	mLoadingBar->bBackgroundImage = pSet->loadingbackground;
	return true;
}

//  Setup
//-------------------------------------------------------------------------------------
bool BaseApp::setup()
{
	if (pSet->rendersystem == "Default")
	{
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		pSet->rendersystem = "Direct3D9 Rendering Subsystem";
		#else
		pSet->rendersystem = "OpenGL Rendering Subsystem";
		#endif
	}
	
	// Dynamic plugin loading
	mRoot = OGRE_NEW Root("", PATHMANAGER::GetUserConfigDir() + "/ogreset.cfg", PATHMANAGER::GetLogDir() + "/ogre.log");

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

	#ifdef _DEBUG
	Ogre::LogManager::getSingleton().setLogDetail(LL_BOREME);//
	#endif

	setupResources();

	if (!configure())
		return false;

	mSceneMgr = mRoot->createSceneManager(/*ST_GENERIC/**/ST_EXTERIOR_FAR/**/);
	mSplitMgr = new SplitScreenManager(mSceneMgr, mWindow, pSet);

	createCamera();
	createViewports(); // calls mSplitMgr->Align();

	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	//  Gui
	mPlatform = new MyGUI::OgrePlatform();
	mPlatform->initialise(mWindow, mSceneMgr, "General", PATHMANAGER::GetLogDir() + "/MyGUI_p.log");
	mGUI = new MyGUI::Gui();
	
	mGUI->initialise("core.xml", PATHMANAGER::GetLogDir() + "/MyGUI.log");
	
	MyGUI::ResourceManager::getInstance().load("MessageBoxResources.xml");

	mGUI->setVisiblePointer(false);
	
	// ------------------------- lang ------------------------
	if (pSet->language == "") // autodetect
	{	pSet->language = getSystemLanguage();
		setlocale(LC_NUMERIC, "C");  }
	
	// valid?
	if (!boost::filesystem::exists(PATHMANAGER::GetDataPath() + "/gui/core_language_" + pSet->language + "_tag.xml"))
		pSet->language = "en";
		
	MyGUI::LanguageManager::getInstance().setCurrentLanguage(pSet->language);
	// -------------------------------------------------------
		
	mPlatform->getRenderManagerPtr()->setSceneManager(mSplitMgr->mGuiSceneMgr);
	mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumViewports*2);
	
	// After having initialised mygui, we can set translated strings
	setTranslations();

	createResourceListener();
	loadResources();

	LogDbg("*** createFrameListener ***");
	createFrameListener();

	LogDbg("*** createScene ***");
	createScene();//^before

	LogDbg("*** recreateCompositor ***");
	recreateCompositor();

	postInit();
	LogDbg("*** end setup ***");
	
	return true;
};

void BaseApp::destroyScene()
{
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
		mGUI->injectKeyRelease(MyGUI::KeyCode::Enum(arg.key));
		return true;  }

	return true;
}

//  Mouse
//-------------------------------------------------------------------------------------
bool BaseApp::mouseMoved( const OIS::MouseEvent &arg )
{
	if (bAssignKey) return true;
	if (isFocGuiOrRpl() && mGUI)  {
		mGUI->injectMouseMove(arg.state.X.abs, arg.state.Y.abs, arg.state.Z.abs);
		return true;  }

	///  Follow Camera Controls
	int i = 0;
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++, i++)
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
	if (isFocGuiOrRpl() && mGUI)  {
		mGUI->injectMousePress(arg.state.X.abs, arg.state.Y.abs, MyGUI::MouseButton::Enum(id));
		return true;  }

	if		(id == MB_Left)		mbLeft = true;
	else if (id == MB_Right)	mbRight = true;
	else if (id == MB_Middle)	mbMiddle = true;
	return true;
}

bool BaseApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (bAssignKey) return true;
	if (isFocGuiOrRpl() && mGUI)  {
		mGUI->injectMouseRelease(arg.state.X.abs, arg.state.Y.abs, MyGUI::MouseButton::Enum(id));
		return true;  }

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
			
	// adjust hud
	bSizeHUD = true;
	bWindowResized = true;
	
	// Adjust viewports
	//mSplitMgr->AdjustRatio();
	mSplitMgr->Align();
	
	// write new window size to settings
	pSet->windowx = mWindow->getWidth();
	pSet->windowy = mWindow->getHeight();
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
