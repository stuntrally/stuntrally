#include "stdafx.h"
#include "BaseApp.h"
#include "FollowCamera.h"
#include "../vdrift/pathmanager.h"
#include "CompositorLogics.h"
#include "Locale.h"
#include "OgreFontManager.h"

//  Camera
//-------------------------------------------------------------------------------------
void BaseApp::createCamera()
{
}

//  Frame
//-------------------------------------------------------------------------------------
void BaseApp::createFrameListener()
{
	LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");

	OverlayManager& ovr = OverlayManager::getSingleton();
	mFpsOverlay = ovr.getByName("Core/FpsOverlay");  mFpsOverlay->show();//
	mDebugOverlay = ovr.getByName("Core/DebugOverlay");  //mDebugOverlay->show();//*
	mOvrFps = ovr.getOverlayElement("Core/CurrFps"),
	mOvrTris= ovr.getOverlayElement("Core/NumTris"),
	mOvrBat = ovr.getOverlayElement("Core/NumBatches"),
	mOvrDbg = ovr.getOverlayElement("Core/DebugText");

	OIS::ParamList pl;	size_t windowHnd = 0;
	std::ostringstream windowHndStr;

	mWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    #if defined OIS_LINUX_PLATFORM
    //pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
    //pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
    //pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
    pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
    #endif

	mInputManager = OIS::InputManager::createInputSystem( pl );

	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);

	windowResized(mWindow);
	WindowEventUtilities::addWindowEventListener(mWindow, this);

	mRoot->addFrameListener(this);
}

//void BaseApp::destroyScene()
//{  }

void BaseApp::createViewports()
{
	mSplitMgr->mNumPlayers = pSet->local_players;
	mSplitMgr->Align();
}

///  Compositor
//-------------------------------------------------------------------------------------
void BaseApp::refreshCompositor()
{
	for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); it++)
	{
		CompositorManager::getSingleton().setCompositorEnabled((*it), "Bloom", false);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "HDR", false);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "Motion Blur", false);
	}
	
	// Set bloom settings (intensity, orig weight).
	MaterialPtr blurmat = MaterialManager::getSingleton().getByName("Ogre/Compositor/BloomBlend2");
	GpuProgramParametersSharedPtr gpuparams = blurmat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
	gpuparams->setNamedConstant("OriginalImageWeight", pSet->bloomorig);
	gpuparams->setNamedConstant("BlurWeight", pSet->bloomintensity);
	
	///  HDR params ..
	//MaterialPtr hdrmat = MaterialManager::getSingleton().getByName("Ogre/Compositor/BloomBlend2");
	//GpuProgramParametersSharedPtr gpuparams = hdrmat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
	//gpuparams->setNamedConstant("Bloom", pSet->);

	// Motion blur intens
	blurmat = MaterialManager::getSingleton().getByName("Ogre/Compositor/Combine");
	gpuparams = blurmat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
	gpuparams->setNamedConstant("blur", pSet->motionblurintensity);
	
	for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); it++)
	{
		CompositorManager::getSingleton().setCompositorEnabled((*it), "Bloom", pSet->bloom);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "HDR", pSet->hdr);
		CompositorManager::getSingleton().setCompositorEnabled((*it), "Motion Blur", pSet->motionblur);
	}
}

//-------------------------------------------------------------------------------------
void BaseApp::recreateCompositor()
{
	// hdr has to be first in the compositor queue
	if (mHDRLogic) delete mHDRLogic;
	mHDRLogic = new HDRLogic;
	CompositorManager::getSingleton().registerCompositorLogic("HDR", mHDRLogic);
	
	// Motion blur has to be created in code
	CompositorPtr comp3 = CompositorManager::getSingleton().create(
		"Motion Blur", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
	);
	CompositionTechnique *t = comp3->createTechnique();
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

	for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); it++)
	{
		CompositorManager::getSingleton().addCompositor((*it), "HDR");
		CompositorManager::getSingleton().addCompositor((*it), "Bloom");
		CompositorManager::getSingleton().addCompositor((*it), "Motion Blur");
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
BaseApp::BaseApp() :
	mRoot(0), mSceneMgr(0), mWindow(0), mHDRLogic(0),
	mShowDialog(1), mShutDown(false),
	mInputManager(0), mMouse(0), mKeyboard(0),
	alt(0), ctrl(0), shift(0), roadUpCnt(0),
	mbLeft(0), mbRight(0), mbMiddle(0), 
	isFocGui(0), mGUI(0), mPlatform(0),
	mWndOpts(0), mWndTabs(0), mWndRpl(0), bSizeHUD(true), bLoading(false),

	mDebugOverlay(0), mFpsOverlay(0), mOvrFps(0), mOvrTris(0), mOvrBat(0), mOvrDbg(0),
	mbShowCamPos(0), ndSky(0),	mbWireFrame(0) //*
{
}

BaseApp::~BaseApp()
{	
	delete mSplitMgr;
	
	if (mGUI)  {
		mGUI->shutdown();	delete mGUI;	mGUI = 0;  }
	if (mPlatform)  {
		mPlatform->shutdown();	delete mPlatform;	mPlatform = 0;  }

	WindowEventUtilities::removeWindowEventListener(mWindow, this);
	windowClosed(mWindow);
	
	
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		mRoot->unloadPlugin("RenderSystem_Direct3D9");
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
	}
	else
	{
		RenderSystem* rs;
		if (rs = mRoot->getRenderSystemByName(pSet->rendersystem))
		{
			mRoot->setRenderSystem(rs);
		}
		else
		{
			Log("RenderSystem '" + pSet->rendersystem + "' is not available. Exiting.");
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
	mLoadingBar.bBackgroundImage = pSet->loadingbackground;
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
		#endif
	}
	else
	{
		if (pSet->rendersystem == "OpenGL Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_GL" + D_SUFFIX);
		else if (pSet->rendersystem == "Direct3D9 Rendering Subsystem")
			mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/RenderSystem_Direct3D9" + D_SUFFIX);
	}

	mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/Plugin_ParticleFX" + D_SUFFIX);
	mRoot->loadPlugin(PATHMANAGER::GetOgrePluginDir() + "/Plugin_CgProgramManager" + D_SUFFIX);

	Ogre::LogManager::getSingleton().setLogDetail(LL_BOREME);//

	setupResources();

	if (!configure())
		return false;

	mSceneMgr = mRoot->createSceneManager(/*ST_GENERIC/**/ST_EXTERIOR_FAR/**/);
	mSplitMgr = new SplitScreenManager(mSceneMgr, mWindow, pSet);

	createCamera();
	// GUI Viewport
	Ogre::SceneManager* guiSceneMgr = mRoot->createSceneManager(ST_GENERIC);
	Ogre::Camera* guiCam = guiSceneMgr->createCamera("GuiCam1");
	Ogre::Viewport* guiVp = mWindow->addViewport(guiCam, 100);
	// transparent
	guiVp->setBackgroundColour(Ogre::ColourValue(0.0, 0.0, 0.0, 0.0));
	guiVp->setClearEveryFrame(true, FBT_DEPTH);
	createViewports();

	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	// Gui
	mPlatform = new MyGUI::OgrePlatform();
	mPlatform->initialise(mWindow, mSceneMgr);
	mGUI = new MyGUI::Gui();
	mGUI->initialise("core.xml", PATHMANAGER::GetLogDir() + "/MyGUI.log");
	MyGUI::LanguageManager::getInstance().setCurrentLanguage(getSystemLanguage());
	
	mPlatform->getRenderManagerPtr()->setSceneManager(guiSceneMgr);
	mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumPlayers);
	
	// After having initialised mygui, we can set translated strings
	setTranslations();

	createResourceListener();
	loadResources();

	createFrameListener();
	createScene();//^before
	
	recreateCompositor();

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
	string s = PATHMANAGER::GetGameConfigDir() + "/resources.cfg";
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
	mLoadingBar.start(mWindow, 1, 1, 1 );

	// Turn off  rendering except overlays
	mSceneMgr->clearSpecialCaseRenderQueues();
	mSceneMgr->addSpecialCaseRenderQueue(RENDER_QUEUE_OVERLAY);
	mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_INCLUDE);
}
void BaseApp::LoadingOff()
{
	// Turn On  full rendering
	mSplitMgr->SetBackground(ColourValue(0.5,0.65,0.8));
	mSceneMgr->clearSpecialCaseRenderQueues();
	mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);
	mLoadingBar.finish();
}


//-------------------------------------------------------------------------------------
//  key, mouse, window
//-------------------------------------------------------------------------------------
bool BaseApp::keyReleased( const OIS::KeyEvent &arg )
{
	if (isFocGui && mGUI)  {
		mGUI->injectKeyRelease(MyGUI::KeyCode::Enum(arg.key));
		return true;  }

	return true;
}

//  Mouse
//-------------------------------------------------------------------------------------
bool BaseApp::mouseMoved( const OIS::MouseEvent &arg )
{
	if (isFocGui && mGUI)  {
		mGUI->injectMouseMove(arg.state.X.abs, arg.state.Y.abs, arg.state.Z.abs);
		return true;  }

	///  Follow Camera Controls
	// -for all cars
	for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
	{
		if ((*it)->fCam)
			(*it)->fCam->Move( mbLeft, mbRight, mbMiddle, shift, arg.state.X.rel, arg.state.Y.rel, arg.state.Z.rel );
	}

	return true;
}

using namespace OIS;
bool BaseApp::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (isFocGui && mGUI)  {
		mGUI->injectMousePress(arg.state.X.abs, arg.state.Y.abs, MyGUI::MouseButton::Enum(id));
		return true;  }

	if		(id == MB_Left)		mbLeft = true;
	else if (id == MB_Right)	mbRight = true;
	else if (id == MB_Middle)	mbMiddle = true;
	return true;
}

bool BaseApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (isFocGui && mGUI)  {
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
	unsigned int width, height, depth;
	int left, top;
	rw->getMetrics(width, height, depth, left, top);

	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;
			
	// adjust hud
	bSizeHUD = true;
	bWindowResized = true;
	
	// Adjust viewports
	mSplitMgr->AdjustRatio();
	
	// write new window size to settings
	pSet->windowx = mWindow->getWidth();
	pSet->windowy = mWindow->getHeight();
}

void BaseApp::windowClosed(RenderWindow* rw)
{
	if (rw == mWindow)
	if (mInputManager)
	{
		mInputManager->destroyInputObject( mMouse );
		mInputManager->destroyInputObject( mKeyboard );

		OIS::InputManager::destroyInputSystem(mInputManager);
		mInputManager = 0;
	}
}
