#include "stdafx.h"
#include "BaseApp.h"
#include "FollowCamera.h"
#include "../vdrift/pathmanager.h"

//  Camera
//-------------------------------------------------------------------------------------
void BaseApp::createCamera()
{
	mCamera = mSceneMgr->createCamera("PlayerCam");

	mCamera->setPosition(Vector3(0,-100,0));
	mCamera->lookAt(Vector3(0,-100,10));
	//mCamera->setNearClipDistance(0.5f);
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
	//  Create one viewport, entire window
	mViewport = mWindow->addViewport(mCamera);
	mViewport->setBackgroundColour(ColourValue(0.5,0.65,0.8));  //`
	mCamera->setAspectRatio( Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));
}

void BaseApp::createCompositor()
{
	if (pSet->bloom)
	{
		//create bloom compositor
		Ogre::CompositorPtr comp = Ogre::CompositorManager::getSingleton().create(
				"Bloom", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
		{
			Ogre::CompositionTechnique *t = comp->createTechnique();
			{
				Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt0");
				def->width = 128;
				def->height = 128;
				def->formatList.push_back(Ogre::PF_A8R8G8B8);
			}
			{
				Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt1");
				def->width = 128;
				def->height = 128;
				def->formatList.push_back(Ogre::PF_A8R8G8B8);
			}
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
				tp->setOutputName("rt1");
			}
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
				tp->setOutputName("rt0");
				Ogre::CompositionPass *pass = tp->createPass();
				pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/Blur0");
				pass->setInput(0, "rt1");
			}
			{
				Ogre::CompositionTargetPass *tp = t->createTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
				tp->setOutputName("rt1");
				Ogre::CompositionPass *pass = tp->createPass();
				pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/Blur1");
				pass->setInput(0, "rt0");
			}
			{
				Ogre::CompositionTargetPass *tp = t->getOutputTargetPass();
				tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
				{ Ogre::CompositionPass *pass = tp->createPass();
				pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Ogre/Compositor/BloomBlend");
				pass->setInput(0, "rt1");
				}
			}
		}
		Ogre::CompositorManager::getSingleton().addCompositor(mViewport, "Bloom");
	
		Ogre::CompositorManager::getSingleton().setCompositorEnabled(mViewport, "Bloom", true);
	}
	else if (pSet->hdr)
	{
		; // TODO
	}
}

//  Run
//-------------------------------------------------------------------------------------
void BaseApp::Run( bool showDialolg )
{
	mShowDialog = showDialolg;
	if (!setup())
		return;

	createCompositor();

	mRoot->startRendering();

	destroyScene();
}

//  ctor
//-------------------------------------------------------------------------------------
BaseApp::BaseApp() :
	mRoot(0), mCamera(0), mSceneMgr(0), mWindow(0), mViewport(0),
	mShowDialog(1), mShutDown(false),
	mInputManager(0), mMouse(0), mKeyboard(0),
	alt(0), ctrl(0), shift(0), mFCam(0), roadUpCnt(0),
	mbLeft(0), mbRight(0), mbMiddle(0), 
	isFocGui(0), mGUI(0), mPlatform(0), mWndOpts(0), mWndTabs(0),

	mDebugOverlay(0), mFpsOverlay(0), mOvrFps(0), mOvrTris(0), mOvrBat(0), mOvrDbg(0),
	mbShowCamPos(0), ndSky(0),	mbWireFrame(0) //*
{
}

BaseApp::~BaseApp()
{
	if (mGUI)  {
		mGUI->shutdown();	delete mGUI;	mGUI = 0;  }
	if (mPlatform)  {
		mPlatform->shutdown();	delete mPlatform;	mPlatform = 0;  }

	delete mFCam;  mFCam = 0;

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
	string p = PATHMANAGER::GetUserConfigDir() + string("/ogreset.cfg");
	HANDLE h = FindFirstFileA( (LPCSTR)p.c_str(), &fd );
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
		ok = false;
#endif

	if (ok)
	{	mWindow = mRoot->initialise( true, "Stunt Rally" );
		return true;
	}

	return false;
}

//  Setup
//-------------------------------------------------------------------------------------
bool BaseApp::setup()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	#ifdef _DEBUG
	mRoot = OGRE_NEW Root(PATHMANAGER::GetGameConfigDir() + "/plugins_win_d.cfg", PATHMANAGER::GetUserConfigDir() + "/ogreset.cfg", PATHMANAGER::GetUserConfigDir() + "/ogre.log");
	#else
	mRoot = OGRE_NEW Root(PATHMANAGER::GetGameConfigDir() + "/plugins_win.cfg", PATHMANAGER::GetUserConfigDir() + "/ogreset.cfg", PATHMANAGER::GetUserConfigDir() + "/ogre.log");
	#endif
#else
	#ifdef _DEBUG
	mRoot = OGRE_NEW Root(PATHMANAGER::GetGameConfigDir() + "/plugins_nix_d.cfg", PATHMANAGER::GetUserConfigDir() + "/ogreset.cfg", PATHMANAGER::GetUserConfigDir() + "/ogre.log");
	#else
	mRoot = OGRE_NEW Root(PATHMANAGER::GetGameConfigDir() + "/plugins_nix.cfg", PATHMANAGER::GetUserConfigDir() + "/ogreset.cfg", PATHMANAGER::GetUserConfigDir() + "/ogre.log");
	#endif
#endif
	Ogre::LogManager::getSingleton().setLogDetail(LL_BOREME);//

	setupResources();

	if (!configure())
		return false;

	mSceneMgr = mRoot->createSceneManager(/*ST_GENERIC/**/ST_EXTERIOR_FAR/**/);
	createCamera();
	createViewports();

	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	createResourceListener();
	loadResources();

	// Gui
	mPlatform = new MyGUI::OgrePlatform();
	mPlatform->initialise(mWindow, mSceneMgr);
	mGUI = new MyGUI::Gui();
	mGUI->initialise(/*"core.xml", "mygui.log"*/);

	createFrameListener();
	createScene();//^before

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
	cf.load(PATHMANAGER::GetGameConfigDir() + "/resources.cfg");

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
	mViewport->setBackgroundColour(ColourValue(0.15,0.165,0.18));
	mLoadingBar.start(mWindow, 1, 1, 1 );

	// Turn off  rendering except overlays
	mSceneMgr->clearSpecialCaseRenderQueues();
	mSceneMgr->addSpecialCaseRenderQueue(RENDER_QUEUE_OVERLAY);
	mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_INCLUDE);
}
void BaseApp::LoadingOff()
{
	// Turn On  full rendering
	mViewport->setBackgroundColour(ColourValue(0.5,0.65,0.8));
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
	if (mFCam)
		mFCam->Move( mbLeft, mbRight, mbMiddle, shift,
			arg.state.X.rel, arg.state.Y.rel, arg.state.Z.rel );

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
