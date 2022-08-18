#pragma once
#include "../ogre/common/Gui_Def.h"
#include <Ogre.h>
// #include <OgreVector3.h>
// #include <OgreString.h>
// #include <OgreFrameListener.h>
#include "../sdl4ogre/events.h"
#include "enums.h"
struct SDL_Window;
namespace SFO  {  class InputWrapper;  class SDLCursorManager;  }
namespace MyGUI{  class OgreD3D11Platform;  class OgrePlatform;  }
namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;
	class Viewport;  class Camera;  class Overlay;  class OverlayElement;  }
class SplineRoad;  class SETTINGS;
	

class BaseApp : public BGui,
		public Ogre::FrameListener,
		public SFO::KeyListener, public SFO::MouseListener, public SFO::WindowListener
{
public:
	virtual ~BaseApp();
	virtual void Run( bool showDialog );

	friend class CGui;
	friend class CGuiCom;
	friend class CScene;

	SETTINGS* pSet =0;

	bool bWindowResized =1;
	Ogre::SceneNode* ndSky =0;  //- out to CScene?
	
	// stuff to be executed in App after BaseApp init
	virtual void postInit() = 0;

	bool mShutDown =0;
protected:	
	bool mShowDialog =0;//, mShutDown;
	bool setup(), configure();  void updateStats();
	
	bool bFirstRenderFrame =1;
	
	///  create
	virtual void createScene() = 0;
	virtual void destroyScene() = 0;

	void createCamera(), createFrameListener();
	void setupResources(), loadResources();

	///  frame events
	virtual bool frameStarted(const Ogre::FrameEvent& evt);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	virtual bool frameEnded(const Ogre::FrameEvent& evt);
	
	///  input events
	virtual bool keyPressed( const SDL_KeyboardEvent &arg) =0;
	bool keyReleased(const SDL_KeyboardEvent &arg);
	bool mouseMoved( const SFO::MouseMotionEvent &arg );
	bool mousePressed( const SDL_MouseButtonEvent &arg, Uint8 id );
	bool mouseReleased(const SDL_MouseButtonEvent &arg, Uint8 id );
	void textInput(const SDL_TextInputEvent &arg);

	void onCursorChange(const std::string& name);

	virtual void windowResized(int x, int y);
	virtual void windowClosed();

	///  Ogre
public:
	Ogre::Root *mRoot =0;
protected:	
	Ogre::SceneManager* mSceneMgr =0;
	Ogre::Viewport* mViewport =0;
	Ogre::RenderWindow* mWindow =0;
	SDL_Window* mSDLWindow =0;
	Ogre::Camera* mCamera =0;
	Ogre::Vector3 mCamPosOld, mCamDirOld;


	///  input
	SFO::InputWrapper* mInputWrapper =0;
	SFO::SDLCursorManager* mCursorManager =0;

	///  ovelay
	Ogre::Overlay *ovBrushPrv =0, *ovTerPrv =0;
	Ogre::OverlayElement *ovBrushMtr =0, *ovTerMtr =0;
	float fStFade =0.f;


	bool alt =0, ctrl =0, shift =0;  // key modifiers
	bool mbLeft =0, mbRight =0, mbMiddle =0;  // mouse buttons

	Ogre::String  mDebugText;	// info texts
	bool mbWireFrame =0;  void UpdWireframe();

	///  camera upd
	bool bMoveCam  =0;
	int mx =0, my =0, mz =0;  double mDTime =0.0;
	Ogre::Real mRotX =0, mRotY =0,  mRotKX =0, mRotKY =0,  moveMul =0, rotMul =0;
	Ogre::Vector3 mTrans;

	ED_MODE	edMode =ED_Deform, edModeOld =ED_Deform;


	///  Gui  ..........................
	bool bGuiFocus =0;  // gui shown
	MyGUI::Gui* mGui =0;
	void baseInitGui();

	MyGUI::OgrePlatform* mPlatform;
	Wnd mWndBrush =0, mWndCam =0, mWndStart =0,  // tool windows
		mWndRoadCur =0, mWndRoadStats =0,
		mWndFluids =0, mWndObjects =0, mWndParticles =0;

	//MyGUI::VectorWidgetPtr
	std::vector<WP> vwGui;  // all widgets to destroy
	Img bckFps =0, imgCur =0, bckInput =0;
	Txt txFps =0, txCamPos =0, txInput =0;

	Wnd mWndMain =0, mWndTrack =0, mWndEdit =0, mWndHelp =0, mWndOpts =0;  // menu, windows
	Wnd mWndTrkFilt =0, mWndPick =0;
	Tab mWndTabsTrack =0, mWndTabsEdit =0, mWndTabsHelp =0, mWndTabsOpts =0;  // main tabs on windows

	///  main menu
	WP mWndMainPanels[WND_ALL] ={0,};
	Btn mWndMainBtns[WND_ALL] ={0,};

	
public:
	inline bool bCam()  {  return  bMoveCam && !bGuiFocus;  }
	inline bool bEdit() {  return !bMoveCam && !bGuiFocus;  }
	
};
