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

//FIXME
#include <Ogre.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>

#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>

namespace boost { class thread; }
namespace MyGUI { class OgreD3D11Platform; }

#include "../sdl4ogre/events.h"

namespace SFO
{
    class InputWrapper;
    class SDLCursorManager;
}

enum ED_MODE  {
	ED_Deform=0, ED_Smooth, ED_Height, ED_Filter, /*ED_Paint,*/
	ED_Road, ED_Start, ED_PrvCam, ED_Fluids, ED_Objects, ED_Rivers, ED_ALL  };

enum WND_Types {  WND_Edit=0, WND_Help, WND_Options, WND_ALL  };  // pSet->inMenu


class BaseApp :
		public Ogre::FrameListener,
		public SFO::KeyListener, public SFO::MouseListener, public SFO::WindowListener
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

	bool mShutDown;
protected:	
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
	
	///  input events
	/*virtual*/
    bool keyPressed(const SDL_KeyboardEvent &arg) = 0;
    bool keyReleased(const SDL_KeyboardEvent &arg);
    bool mouseMoved( const SFO::MouseMotionEvent &arg );
    bool mousePressed( const SDL_MouseButtonEvent &arg, Uint8 id );
    bool mouseReleased( const SDL_MouseButtonEvent &arg, Uint8 id );
	void textInput(const SDL_TextInputEvent &arg);

	void onCursorChange(const std::string& name);

	virtual void windowResized(int x, int y);


	///  Ogre
	Ogre::Root *mRoot;  Ogre::SceneManager* mSceneMgr;
    Ogre::Viewport* mViewport;
    Ogre::RenderWindow* mWindow;
    SDL_Window* mSDLWindow;
	Ogre::Camera* mCamera;
	Ogre::Vector3 mCamPosOld,mCamDirOld;
	
	///  input
    SFO::InputWrapper* mInputWrapper;
    SFO::SDLCursorManager* mCursorManager;

	///  ovelay
	Ogre::Overlay* mDebugOverlay, *ovBrushPrv, *ovTerPrv;
	Ogre::OverlayElement *ovSt, *ovBrushMtr, *ovTerMtr,
		*ovPos, *ovDbg, *ovInfo, *ovStat, *ovFocus, *ovFocBck;  Ogre::Real fStFade;

	bool alt, ctrl, shift;  // key modifiers
	bool mbLeft, mbRight, mbMiddle;  // mouse buttons
	Ogre::String  mDebugText;	// info texts
	bool mbWireFrame;

	///  camera upd
	bool bMoveCam;	int mx,my,mz;  double mDTime;
	Ogre::Real mRotX, mRotY,  mRotKX, mRotKY,  moveMul, rotMul;
	Ogre::Vector3 mTrans;

	ED_MODE	edMode,edModeOld;


	///  Gui
	bool bGuiFocus;  // gui shown
	MyGUI::Gui* mGUI;  void baseInitGui();
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	MyGUI::OgreD3D11Platform* mPlatform;
	#else
	MyGUI::OgrePlatform* mPlatform;
	#endif
	MyGUI::WidgetPtr  // tool windows
		mWndBrush, mWndCam, mWndStart,
		mWndRoadCur, mWndRoadStats,
		mWndFluids, mWndObjects, mWndRivers;
	MyGUI::VectorWidgetPtr vwGui;  // all widgets to destroy
	MyGUI::ImageBox* imgCur, *bckFps;
	MyGUI::TextBox*	txFps;

	//  main menu
	friend class CGui;
	MyGUI::WidgetPtr mWndMain,mWndEdit,mWndHelp,mWndOpts;  // menu, windows
	MyGUI::TabPtr mWndTabsEdit,mWndTabsHelp,mWndTabsOpts;  // main tabs on windows
	MyGUI::WidgetPtr mWndMainPanels[WND_ALL];  MyGUI::ButtonPtr mWndMainBtns[WND_ALL];

	
public:
	inline bool bCam()  {  return  bMoveCam && !bGuiFocus;  }
	inline bool bEdit() {  return !bMoveCam && !bGuiFocus;  }
	
};
#endif
