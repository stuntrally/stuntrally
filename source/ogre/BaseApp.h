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

#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"

namespace MyGUI { class OgreD3D11Platform; }
namespace Ogre 
{  
	class SceneNode;  class Root;  class SceneManager;  class RenderWindow;
	namespace RTShader{ class ShaderGenerator; };
}
namespace OIS  {  class InputManager;  class Mouse;  class Keyboard;  }
namespace OISB {  class System;  };
class ShaderGeneratorTechniqueResolverListener;


class BaseApp :
		public Ogre::FrameListener, public Ogre::WindowEventListener,
		public OIS::KeyListener, public OIS::MouseListener, public OIS::JoyStickListener
{
	friend class CarModel;
public:
	BaseApp();
	virtual ~BaseApp();
	virtual void Run( bool showDialog );
	
	bool bLoading;
	
	// has to be in baseApp to switch camera on C press
	typedef std::vector<class CarModel*> CarModels;
	CarModels carModels;
	
	// stuff to be executed in App after BaseApp init
	virtual void postInit() = 0;
	
	virtual void setTranslations() = 0;
		
	class SplitScreenManager* mSplitMgr;
	class HDRLogic* mHDRLogic; class MotionBlurLogic* mMotionBlurLogic;
	class SSAOLogic* mSSAOLogic;
	void recreateCompositor();
	
	// motion blur
	float motionBlurIntensity;
	
	class SETTINGS* pSet;
	
	//  wnd, hud, upl
	bool bWindowResized, bSizeHUD;
	int roadUpCnt;
	class LoadingBar* mLoadingBar;
	Ogre::SceneNode* ndSky;  //-

	Ogre::String StrFromKey(const Ogre::String& skey);  // util for input
	std::map<OIS::KeyCode, Ogre::String> kcMap;  // key names in english
	void InitKeyNamesMap();
	
protected:
	bool mShowDialog, mShutDown;
	bool setup(), configure();  void updateStats();
	
	bool bFirstRenderFrame;

	///  create
	virtual void createScene() = 0;
	virtual void destroyScene();

	void createCamera(), createFrameListener(), createViewports(), refreshCompositor(bool disableAll=false);
	void setupResources(), createResourceListener(), loadResources();
	void LoadingOn(), LoadingOff();

	///  frame events
	bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	bool frameEnded(const Ogre::FrameEvent& evt);
	virtual bool frameStart(Ogre::Real time) = 0;
	virtual bool frameEnd(Ogre::Real time) = 0;
	
	///  input events
	virtual bool keyPressed(const OIS::KeyEvent &arg);  bool keyReleased(const OIS::KeyEvent &arg);
	bool mouseMoved(const OIS::MouseEvent &arg);
	bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	void windowResized(Ogre::RenderWindow* rw), windowClosed(Ogre::RenderWindow* rw);
	//  joystick
	virtual bool axisMoved( const OIS::JoyStickEvent &e, int axis ) = 0;
    virtual bool buttonPressed( const OIS::JoyStickEvent &e, int button ) = 0;
    virtual bool buttonReleased( const OIS::JoyStickEvent &e, int button ) = 0;


	///  Ogre
	Ogre::Root *mRoot;  Ogre::SceneManager* mSceneMgr;
	Ogre::RenderWindow* mWindow;
	Ogre::RTShader::ShaderGenerator *mShaderGenerator;
	ShaderGeneratorTechniqueResolverListener*	mMaterialMgrListener;		// Shader generator material manager listener.	

	///  input
	OISB::System* mOISBsys;
	OIS::InputManager* mInputManager;
public:
	OIS::Mouse* mMouse;  OIS::Keyboard* mKeyboard;
	bool isKey(OIS::KeyCode k)  {  return mKeyboard->isKeyDown(k);  }
	
	// this is set to true when the user is asked to assign a new key
	bool bAssignKey;
	MyGUI::Widget* pressedKeySender;

	bool isFocGuiOrRpl()  {  return isFocGui || isFocRpl;  }
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
	
	enum {  WND_Game=0, WND_Champ, WND_Replays, WND_Options, WND_ALL  };  // pSet->inMenu
	MyGUI::WidgetPtr mWndMain,mWndGame,mWndChamp,mWndReplays,mWndOpts,  mWndRpl;  // options window, rpl controls
	MyGUI::TabPtr mWndTabsGame,mWndTabsOpts;  MyGUI::VectorWidgetPtr vwGui;
	MyGUI::WidgetPtr mWndMainPanels[WND_ALL];  MyGUI::ButtonPtr mWndMainBtns[WND_ALL];

	///  networking
	boost::scoped_ptr<MasterClient> mMasterClient;
	boost::scoped_ptr<P2PGameClient> mClient;
	enum LobbyState { DISCONNECTED, HOSTING, JOINED } mLobbyState;
};

#endif
