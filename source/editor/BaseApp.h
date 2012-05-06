#ifndef _BaseApp_h_
#define _BaseApp_h_

#include "../ogre/common/QTimer.h"
#include "settings.h"

/*#include <OgreVector3.h>
#include <OgreString.h>

#include <OgreOverlay.h>
#include <OgreOverlayElement.h>

#include <OgreRenderTargetListener.h>
#include <OgreFrameListener.h>
#include <OgreWindowEventUtilities.h>*/
#include <Ogre.h>

#include <OISKeyboard.h>
#include <OISMouse.h>

#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>

namespace boost { class thread; }
namespace MyGUI { class OgreD3D11Platform; }


class BaseApp :
		public Ogre::FrameListener, public Ogre::WindowEventListener,
		public OIS::KeyListener, public OIS::MouseListener
		//public Ogre::RenderTargetListener
{
public:
	BaseApp();	virtual ~BaseApp();
	virtual void Run( bool showDialog );

	class SplineRoad* road; //-
	
	SETTINGS* pSet;

	bool bWindowResized;
	Ogre::SceneNode* ndSky; //-
	
	// stuff to be executed in App after BaseApp init
	virtual void postInit() = 0;

	//AppThr appThr;
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	HANDLE hpr;
#endif
	QTimer timer;
	void OnTimer(double dTime);
	bool mShutDown;
	bool inputThreadRunning;
protected:
	boost::thread* mThread;
	
	//class HWMouse* mHWMouse;

	bool mShowDialog;//, mShutDown;
	bool setup(), configure();  void updateStats();
	
	bool bFirstRenderFrame;
	
	///  create
	virtual void createScene() = 0;
	virtual void destroyScene();

	void createCamera(), createFrameListener();
	void setupResources(), createResourceListener(), loadResources();

	///  frame events
	virtual bool frameStarted(const Ogre::FrameEvent& evt);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	virtual bool frameEnded(const Ogre::FrameEvent& evt);
	virtual void processMouse() { }
	
	///  input events
	/*virtual*/
	virtual bool keyPressed(const OIS::KeyEvent &arg);
	virtual bool keyReleased(const OIS::KeyEvent &arg);

	bool mouseMoved(const OIS::MouseEvent &arg);
	bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	void windowResized(Ogre::RenderWindow* rw), windowClosed(Ogre::RenderWindow* rw);

	///  input event queues  for gui  ------------------------------------
	struct CmdKey {  public:
		OIS::KeyCode key;
		unsigned int text;  bool gui;
		CmdKey() : key(OIS::KC_UNASSIGNED), text(0), gui(true) {  }
		CmdKey(const OIS::KeyEvent& k) : key(k.key), text(k.text), gui(true) {  }
	};
	struct CmdMouseMove {  public:
		OIS::MouseState ms;
		CmdMouseMove() {  }
		CmdMouseMove(const OIS::MouseEvent& m) : ms(m.state) {  }
	};
	struct CmdMouseBtn  {  public:
		OIS::MouseState ms;  OIS::MouseButtonID btn;
		CmdMouseBtn() {  }
		CmdMouseBtn(const OIS::MouseEvent& m, OIS::MouseButtonID b) : ms(m.state),btn(b) {  }
	};
	#define cmd_Max 1024
	//std::deque<CmdKey> cmdKeyPress, cmdKeyRel;
	CmdKey* cmdKeyPress, *cmdKeyRel;  int i_cmdKeyPress, i_cmdKeyRel;
	CmdMouseMove* cmdMouseMove;  int i_cmdMouseMove;
	CmdMouseBtn* cmdMousePress, *cmdMouseRel;  int i_cmdMousePress, i_cmdMouseRel;


	///  Ogre
	Ogre::Root *mRoot;  Ogre::SceneManager* mSceneMgr;
	Ogre::Viewport* mViewport;  Ogre::RenderWindow* mWindow;
	Ogre::Camera* mCamera, *mCameraT;
	Ogre::Vector3 mCamPosOld,mCamDirOld;
	
	///  input
	OIS::InputManager* mInputManager;
	OIS::Mouse* mMouse;  OIS::Keyboard* mKeyboard;


	///  ovelay
	Ogre::Overlay* mDebugOverlay, *ovBrushPrv;
	Ogre::OverlayElement* ovFps, *ovTri, *ovBat,  *ovSt, *ovBrushMtr,
		*ovPos, *ovDbg, *ovInfo, *ovStat, *ovFocus, *ovFocBck;  Ogre::Real fStFade;

	bool alt, ctrl, shift;  // key modifiers
	bool mbLeft, mbRight, mbMiddle;  // mouse buttons
	Ogre::String  mDebugText, mFilText;	// info texts
	bool mbWireFrame, mShowCamPos, mStatsOn;

	///  camera upd
	bool bMoveCam;	int mx,my,mz;  double mDTime;
	Ogre::Real mRotX, mRotY,  mRotKX, mRotKY,  moveMul, rotMul;
	Ogre::Vector3 mTrans;

	enum ED_MODE
	{	ED_Deform=0, ED_Smooth, ED_Height, ED_Filter, /*ED_Paint,*/
		ED_Road, ED_Start, ED_PrvCam, ED_Fluids, ED_Objects, ED_ALL
	} edMode,edModeOld;


	///  Gui
	bool bGuiFocus;  // gui shown
	MyGUI::Gui* mGUI;
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	MyGUI::OgreD3D11Platform* mPlatform;
	#else
	MyGUI::OgrePlatform* mPlatform;
	#endif
	MyGUI::WidgetPtr  // tool windows
		mWndBrush, mWndCam, mWndStart,
		mWndRoadCur, mWndRoadStats,
		mWndFluids, mWndObjects;
	MyGUI::VectorWidgetPtr vwGui;  // all widgets to destroy

	//  main menu
	enum WND_Types {  WND_Edit=0, WND_Help, WND_Options, WND_ALL  };  // pSet->inMenu
	MyGUI::WidgetPtr mWndMain,mWndEdit,mWndHelp,mWndOpts;  // menu, windows
	MyGUI::TabPtr mWndTabsEdit,mWndTabsHelp,mWndTabsOpts;  // main tabs on windows
	MyGUI::WidgetPtr mWndMainPanels[WND_ALL];  MyGUI::ButtonPtr mWndMainBtns[WND_ALL];

	
public:
	inline bool bCam()  {  return  bMoveCam && !bGuiFocus;  }
	inline bool bEdit() {  return !bMoveCam && !bGuiFocus;  }
	
};
#endif
