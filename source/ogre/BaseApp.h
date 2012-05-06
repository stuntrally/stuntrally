#ifndef _BaseApp_h_
#define _BaseApp_h_

#include <OgreVector3.h>
#include <OgreFrameListener.h>
#include <OgreWindowEventUtilities.h>

#include <OISKeyboard.h>
#include <OISMouse.h>
#include <OISJoyStick.h>

#include <MyGUI_Prerequest.h>
#include <MyGUI_Widget.h>
#include <MyGUI_OgrePlatform.h>

#include <boost/scoped_ptr.hpp>
//#include <boost/thread.hpp>

namespace MyGUI{  class OgreD3D11Platform; }
namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;
	namespace RTShader {  class ShaderGenerator;  }  }
namespace OIS  {  class InputManager;  class Mouse;  class Keyboard;  }
namespace OISB {  class System;  }
class MasterClient;  class P2PGameClient;
class MaterialFactory;


class MaterialMgrListener : public Ogre::MaterialManager::Listener
{
public:
	MaterialMgrListener(Ogre::RTShader::ShaderGenerator* pShaderGenerator)
		: mShaderGenerator(pShaderGenerator)
	{  }

	virtual Ogre::Technique* handleSchemeNotFound(
		unsigned short schemeIndex, const Ogre::String& schemeName,
		Ogre::Material* originalMaterial, unsigned short lodIndex,
		const Ogre::Renderable* rend);
protected:	
	Ogre::RTShader::ShaderGenerator* mShaderGenerator;
};



class BaseApp :
		public Ogre::FrameListener, public Ogre::WindowEventListener,
		public OIS::KeyListener, public OIS::MouseListener, public OIS::JoyStickListener
{
	friend class CarModel;
public:
	BaseApp();
	virtual ~BaseApp();
	virtual void Run( bool showDialog );
	
	bool bLoading,bLoadingEnd;
	
	//  has to be in baseApp for camera mouse move
	typedef std::vector<class CarModel*> CarModels;
	CarModels carModels;
	
	void showMouse(), hideMouse(), updMouse();
	
	//  stuff to be executed in App after BaseApp init
	virtual void postInit() = 0;
	
	virtual void setTranslations() = 0;
		
	///  effects
	class SplitScreenManager* mSplitMgr;
	class HDRLogic* mHDRLogic; class MotionBlurLogic* mMotionBlurLogic;
	class SSAOLogic* mSSAOLogic;
	class GodRaysLogic* mGodRaysLogic;
	class SoftParticlesLogic* mSoftParticlesLogic;
	class DepthOfFieldLogic* mDepthOfFieldLogic;
	class GBufferLogic* mGBufferLogic;
	class FilmGrainLogic* mFilmGrainLogic;
	void recreateCompositor();
	bool AnyEffectEnabled();
	bool NeedMRTBuffer();
	float motionBlurIntensity;
	void CreateRTfixed();

	class SETTINGS* pSet;
	
	MaterialFactory* materialFactory;  // material generation
	
	//  wnd, hud, upl
	bool bWindowResized, bSizeHUD;
	float roadUpdTm;
	class LoadingBar* mLoadingBar;
	Ogre::SceneNode* ndSky;  //-

	Ogre::String StrFromKey(const Ogre::String& skey);  // util for input
	std::map<OIS::KeyCode, Ogre::String> kcMap;  // key names in english
	void InitKeyNamesMap();
	
protected:
	bool mShowDialog, mShutDown;
	bool setup(), configure();  void updateStats();
	
	class HWMouse* mHWMouse;

	///  create
	virtual void createScene() = 0;
	virtual void destroyScene();

	void createFrameListener(), createViewports(), refreshCompositor(bool disableAll=false);
	void setupResources(), createResourceListener(), loadResources();
	void LoadingOn(), LoadingOff();

	///  frame events
	bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	bool frameEnded(const Ogre::FrameEvent& evt);
	virtual bool frameStart(Ogre::Real time) = 0;
	virtual bool frameEnd(Ogre::Real time) = 0;
	
	///  input events
	bool keyReleased(const OIS::KeyEvent &arg);
	bool mouseMoved(const OIS::MouseEvent &arg);
	bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	void windowResized(Ogre::RenderWindow* rw), windowClosed(Ogre::RenderWindow* rw);
	//  joystick
	virtual bool axisMoved( const OIS::JoyStickEvent &e, int axis ) = 0;
    virtual bool buttonPressed( const OIS::JoyStickEvent &e, int button ) = 0;
    virtual bool buttonReleased( const OIS::JoyStickEvent &e, int button ) = 0;


	///  Ogre
	Ogre::Root* mRoot;  Ogre::SceneManager* mSceneMgr;
	Ogre::RenderWindow* mWindow;
	Ogre::RTShader::ShaderGenerator* mShaderGenerator;
	MaterialMgrListener* mMaterialMgrListener;  // Shader generator material manager listener.	

	///  input
	OISB::System* mOISBsys;
	OIS::InputManager* mInputManager;
public:
	OIS::Mouse* mMouse;  OIS::Keyboard* mKeyboard;
	bool isKey(OIS::KeyCode k)  {  return mKeyboard->isKeyDown(k);  }
	
	// this is set to true when the user is asked to assign a new key
	bool bAssignKey;
	MyGUI::Widget* pressedKeySender;

	bool IsFocGuiInput()  {  return isFocGui || isFocRpl;  }
	bool IsFocGui();
protected:

	///  overlay
	Ogre::Overlay* mDebugOverlay, *mFpsOverlay;  // fps stats
	Ogre::OverlayElement* mOvrFps, *mOvrTris, *mOvrBat, *mOvrDbg;

	bool alt, ctrl, shift;  // key modifiers
	bool mbLeft, mbRight, mbMiddle;  // mouse buttons
	Ogre::String  mDebugText, mFilText;	// info texts
	bool mbWireFrame, mbShowCamPos;  // on/off
	int iCurCam;

	///  Gui
	bool isFocGui,isFocRpl;  // gui shown
	MyGUI::Gui* mGUI;
	
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	MyGUI::OgreD3D11Platform* mPlatform;
	#else
	MyGUI::OgrePlatform* mPlatform;
	#endif
	
	//  main menu
	enum WND_Types {  WND_Game=0, WND_Champ, WND_Replays, WND_Help, WND_Options, WND_ALL  };  // pSet->inMenu
	MyGUI::WindowPtr mWndMain,mWndGame,mWndReplays,mWndHelp,mWndOpts,  // menu, windows
		mWndRpl, mWndChampStage,mWndChampEnd, mWndNetEnd;  // rpl controls, champ wnds
	MyGUI::TabPtr mWndTabsGame,mWndTabsOpts,mWndTabsHelp,mWndTabsRpl;  // main tabs on windows
	MyGUI::WidgetPtr mWndMainPanels[WND_ALL];  MyGUI::ButtonPtr mWndMainBtns[WND_ALL];
	MyGUI::VectorWidgetPtr vwGui;  // all widgets to destroy

	///  networking
	boost::scoped_ptr<MasterClient> mMasterClient;
	boost::scoped_ptr<P2PGameClient> mClient;
	enum LobbyState { DISCONNECTED, HOSTING, JOINED } mLobbyState;
};

#endif
