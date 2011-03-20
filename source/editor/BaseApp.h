#ifndef _BaseApp_h_
#define _BaseApp_h_
#include <OgreRenderTargetListener.h>
#include "../ogre/QTimer.h"
using namespace Ogre;

class BaseApp :
		public Ogre::FrameListener, public Ogre::WindowEventListener,
		public OIS::KeyListener, public OIS::MouseListener
		//public Ogre::RenderTargetListener
{
public:
	BaseApp();	virtual ~BaseApp();
	virtual void Run( bool showDialolg );

	class SplineRoad* road; //-

	//AppThr appThr;
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	HANDLE hpr;
#endif
	QTimer timer;
	void OnTimer(double dTime);
	bool mShutDown;
protected:
	bool mShowDialog;//, mShutDown;
	bool setup(), configure();  void updateStats();

	///  create
	virtual void createScene() = 0;
	virtual void destroyScene();

	void createCamera(), createFrameListener();
	void setupResources(), createResourceListener(), loadResources();
	SceneNode* ndSky; //-

	///  frame events
	virtual bool frameStarted(const FrameEvent& evt);
	virtual bool frameRenderingQueued(const FrameEvent& evt);
	virtual bool frameEnded(const FrameEvent& evt);
	virtual void processMouse() { }
	
	///  input events
	/*virtual*/
	virtual bool keyPressed(const OIS::KeyEvent &arg);
	virtual bool keyReleased(const OIS::KeyEvent &arg);

	bool mouseMoved(const OIS::MouseEvent &arg);
	bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	void windowResized(RenderWindow* rw), windowClosed(RenderWindow* rw);

	///  input event queues  for gui  ------------------------------------
	struct CmdKey {  public:
		//OIS::KeyEvent key;
		OIS::KeyCode key;
		unsigned int text;  bool gui;
		CmdKey() : key(OIS::KC_UNASSIGNED), text(0) {  }
		CmdKey(const OIS::KeyEvent& k) : key(k.key), text(k.text) {  }
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
	//std::deque<CmdMouseMove> cmdMouseMove;
	//std::deque<CmdMouseBtn> cmdMousePress, cmdMouseRel;
	CmdKey* cmdKeyPress, *cmdKeyRel;  int i_cmdKeyPress, i_cmdKeyRel;
	CmdMouseMove* cmdMouseMove;  int i_cmdMouseMove;
	CmdMouseBtn* cmdMousePress, *cmdMouseRel;  int i_cmdMousePress, i_cmdMouseRel;


	///  Ogre
	Root *mRoot;  SceneManager* mSceneMgr;
	Viewport* mViewport;  RenderWindow* mWindow;
	Camera* mCamera, *mCameraT;
	Vector3 /*mCamPosPrv,mCamDirPrv,*/mCamPosOld,mCamDirOld;
	
	///  input
	OIS::InputManager* mInputManager;
	OIS::Mouse* mMouse;  OIS::Keyboard* mKeyboard;


	///  ovelay
	Overlay* mDebugOverlay;
	OverlayElement* ovFps, *ovTri, *ovBat,  *ovSt,
		*ovPos, *ovDbg, *ovInfo, *ovStat, *ovFocus, *ovFocBck;  Real fStFade;

	bool alt, ctrl, shift;  // key modifiers
	bool mbLeft, mbRight, mbMiddle;  // mouse buttons
	String  mDebugText, mFilText;	// info texts
	bool mbWireFrame, mShowCamPos, mStatsOn;
	char s[512];


	///  camera upd
	bool bMoveCam;	int mx,my,mz;  double mDTime;
	Real mRotX, mRotY,  mRotKX, mRotKY,  moveMul, rotMul;
	Vector3 mTrans;
	enum ED_MODE {  ED_Deform=0, ED_Smooth, /*ED_Paint,*/ ED_Road, ED_Start, ED_PrvCam, ED_ALL  }
		edMode,edModeOld;


	///  Gui
	bool bGuiFocus;  // gui shown
	MyGUI::Gui* mGUI;	MyGUI::OgrePlatform* mPlatform;
	MyGUI::WidgetPtr mWndOpts, mWndBrush, mWndCam,
		mWndRoadCur, mWndRoadNew, mWndRoadStats;  // gui windows
	MyGUI::TabPtr mWndTabs;
	
public:
	inline bool bCam()  {  return  bMoveCam && !bGuiFocus;  }
	inline bool bEdit() {  return !bMoveCam && !bGuiFocus;  }
	
};
#endif
