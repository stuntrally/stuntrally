#ifndef _BaseApp_h_
#define _BaseApp_h_
#include "LoadingBar.h"
using namespace Ogre;


class BaseApp :
		public Ogre::FrameListener, public Ogre::WindowEventListener,
		public OIS::KeyListener, public OIS::MouseListener
{
public:
	BaseApp();	virtual ~BaseApp();
	virtual void Run( bool showDialolg );

	SceneNode* ndSky; //-
	class FollowCamera* mFCam;  // cam+
	int roadUpCnt;
	Camera* GetCamera()  {  return mCamera;  }
	LoadingBar mLoadingBar;

protected:
	bool mShowDialog, mShutDown;
	bool setup(), configure();  void updateStats();

	///  create
	virtual void createScene() = 0;
	virtual void destroyScene();

	void createCamera(), createFrameListener(), createViewports();
	void setupResources(), createResourceListener(), loadResources();
	void LoadingOn(), LoadingOff();

	///  frame events
	bool frameRenderingQueued(const FrameEvent& evt);
	bool frameEnded(const FrameEvent& evt);
	virtual bool frameStart(Real time) = 0;
	virtual bool frameEnd(Real time) = 0;
	
	///  input events
	virtual bool keyPressed(const OIS::KeyEvent &arg);  bool keyReleased(const OIS::KeyEvent &arg);
	bool mouseMoved(const OIS::MouseEvent &arg);
	bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	void windowResized(RenderWindow* rw), windowClosed(RenderWindow* rw);


	///  Ogre
	Root *mRoot;  SceneManager* mSceneMgr;
	Camera* mCamera;  Viewport* mViewport;  RenderWindow* mWindow;

	///  input
	OIS::InputManager* mInputManager;
	OIS::Mouse* mMouse;  OIS::Keyboard* mKeyboard;


	///  ovelay
	Overlay* mDebugOverlay, *mFpsOverlay;  // fps stats
	OverlayElement* mOvrFps, *mOvrTris, *mOvrBat, *mOvrDbg;

	bool alt, ctrl, shift;  // key modifiers
	bool mbLeft, mbRight, mbMiddle;  // mouse buttons
	String  mDebugText, mFilText;	// info texts
	bool mbWireFrame, mbShowCamPos;  // on/off


	///  Gui
	bool isFocGui;  // gui shown
	MyGUI::Gui* mGUI;		MyGUI::OgrePlatform* mPlatform;
	MyGUI::WidgetPtr mLayout, mWndOpts;  // options window
	MyGUI::TabPtr mWndTabs;
};

#endif