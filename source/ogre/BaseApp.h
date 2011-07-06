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
namespace MyGUI{  class OgrePlatform;  };

namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;  }
namespace OIS  {  class InputManager;  class Mouse;  class Keyboard;  }
namespace OISB {  class System;  };


class BaseApp :
		public Ogre::FrameListener, public Ogre::WindowEventListener,
		public OIS::KeyListener, public OIS::MouseListener, public OIS::JoyStickListener
{
public:
	BaseApp();	virtual ~BaseApp();
	virtual void Run( bool showDialog );
	
	bool bLoading;
	
	// has to be in baseApp to switch camera on C press
	std::vector<class CarModel*> carModels;
	
	// translation
	// can't have it in c'tor, because mygui is not initialized
	virtual void setTranslations() = 0;
	
	class SplitScreenManager* mSplitMgr;
	
	bool bWindowResized;  bool bSizeHUD;
	class HDRLogic* mHDRLogic;
	
	class SETTINGS* pSet;
	
	void recreateCompositor();

	Ogre::SceneNode* ndSky; //-
	int roadUpCnt;
	class LoadingBar* mLoadingBar;

protected:
	bool mShowDialog, mShutDown;
	bool setup(), configure();  void updateStats();

	///  create
	virtual void createScene() = 0;
	virtual void destroyScene();

	void createCamera(), createFrameListener(), createViewports(), refreshCompositor();
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
    virtual bool povMoved( const OIS::JoyStickEvent &e, int pov ) = 0;
	virtual bool axisMoved( const OIS::JoyStickEvent &e, int axis ) = 0;
    virtual bool sliderMoved( const OIS::JoyStickEvent &e, int sliderID ) = 0;
    virtual bool buttonPressed( const OIS::JoyStickEvent &e, int button ) = 0;
    virtual bool buttonReleased( const OIS::JoyStickEvent &e, int button ) = 0;


	///  Ogre
	Ogre::Root *mRoot;  Ogre::SceneManager* mSceneMgr;
	Ogre::RenderWindow* mWindow;

	///  input
	OISB::System* mOISBsys;
	OIS::InputManager* mInputManager;
public:
	OIS::Mouse* mMouse;  OIS::Keyboard* mKeyboard;
	bool isKey(OIS::KeyCode k)  {  return mKeyboard->isKeyDown(k);  }
	
	// this is set to true when the user is asked to assign a new key
	bool bAssignKey;
	OIS::KeyCode pressedKey;
	MyGUI::Widget* pressedKeySender;
protected:

	///  ovelay
	Ogre::Overlay* mDebugOverlay, *mFpsOverlay;  // fps stats
	Ogre::OverlayElement* mOvrFps, *mOvrTris, *mOvrBat, *mOvrDbg;

	bool alt, ctrl, shift;  // key modifiers
	bool mbLeft, mbRight, mbMiddle;  // mouse buttons
	Ogre::String  mDebugText, mFilText;	// info texts
	bool mbWireFrame, mbShowCamPos;  // on/off
	int iCurCam;

	///  Gui
	bool isFocGuiOrRpl()  {  return isFocGui || isFocRpl;  }
	bool isFocGui,isFocRpl;  // gui shown
	MyGUI::Gui* mGUI;		MyGUI::OgrePlatform* mPlatform;
	MyGUI::WidgetPtr mLayout, mWndOpts, mWndRpl;  // options window
	MyGUI::TabPtr mWndTabs;
	MyGUI::VectorWidgetPtr vwGui;
};

#endif
