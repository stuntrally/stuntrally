#pragma once
#include "../ogre/common/Gui_Def.h"
#include <OgreVector3.h>
#include <OgreString.h>
#include <OgreFrameListener.h>
#include "../sdl4ogre/events.h"
namespace SFO  {  class InputWrapper;  class SDLCursorManager;  }
struct SDL_Window;
namespace MyGUI{  class OgreD3D11Platform;  class OgrePlatform;  }
namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;
	class Viewport;  class Camera;  class Overlay;  class OverlayElement;  }
class SplineRoad;  class SETTINGS;
	

enum ED_MODE
{
	ED_Deform=0, ED_Smooth, ED_Height, ED_Filter, /*ED_Paint,*/
	ED_Road, ED_Start, ED_PrvCam, ED_Fluids, ED_Objects, ED_Rivers, ED_ALL
};
enum WND_Types
{	WND_Track=0, WND_Edit, WND_Help, WND_Options, WND_ALL  };  // pSet->inMenu
enum TABS_Edit
{	TAB_Back=0, TAB_Sun, TAB_Terrain, TAB_Layers, TAB_Grass, TAB_Veget, TAB_Surface, TAB_Road, TAB_Objects, TabsEdit_ALL  };


class BaseApp : public BGui,
		public Ogre::FrameListener,
		public SFO::KeyListener, public SFO::MouseListener, public SFO::WindowListener
{
public:
	BaseApp();	virtual ~BaseApp();
	virtual void Run( bool showDialog );

	friend class CGui;
	friend class CGuiCom;
	friend class CScene;

	SETTINGS* pSet;

	bool bWindowResized;
	Ogre::SceneNode* ndSky;  //- out to CScene?
	
	// stuff to be executed in App after BaseApp init
	virtual void postInit() = 0;

	bool mShutDown;
protected:	
	bool mShowDialog;//, mShutDown;
	bool setup(), configure();  void updateStats();
	
	bool bFirstRenderFrame;
	
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
	/*virtual*/
	bool keyPressed( const SDL_KeyboardEvent &arg) = 0;
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
	Ogre::Root *mRoot;
protected:	
	Ogre::SceneManager* mSceneMgr;
	Ogre::Viewport* mViewport;
	Ogre::RenderWindow* mWindow;
	SDL_Window* mSDLWindow;
	Ogre::Camera* mCamera;
	Ogre::Vector3 mCamPosOld,mCamDirOld;
	
	///  input
	SFO::InputWrapper* mInputWrapper;
	SFO::SDLCursorManager* mCursorManager;

	///  ovelay
	Ogre::Overlay *ovBrushPrv, *ovTerPrv;
	Ogre::OverlayElement *ovBrushMtr, *ovTerMtr;
	float fStFade;

	bool alt, ctrl, shift;  // key modifiers
	bool mbLeft, mbRight, mbMiddle;  // mouse buttons

	Ogre::String  mDebugText;	// info texts
	bool mbWireFrame;  void UpdWireframe();

	///  camera upd
	bool bMoveCam;
	int mx,my,mz;  double mDTime;
	Ogre::Real mRotX, mRotY,  mRotKX, mRotKY,  moveMul, rotMul;
	Ogre::Vector3 mTrans;

	ED_MODE	edMode,edModeOld;

	///  Gui  ..........................
	bool bGuiFocus;  // gui shown
	MyGUI::Gui* mGui;
	void baseInitGui();
	
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	MyGUI::OgreD3D11Platform* mPlatform;
	#else
	MyGUI::OgrePlatform* mPlatform;
	#endif
	Wnd mWndBrush, mWndCam, mWndStart,  // tool windows
		mWndRoadCur, mWndRoadStats,
		mWndFluids, mWndObjects, mWndRivers;

	//MyGUI::VectorWidgetPtr
	std::vector<WP> vwGui;  // all widgets to destroy
	Img bckFps, imgCur, bckInput;
	Txt txFps, txCamPos, txInput;

	Wnd mWndMain, mWndTrack,mWndEdit,mWndHelp,mWndOpts;  // menu, windows
	Wnd mWndTrkFilt, mWndPick;
	Tab mWndTabsTrack,mWndTabsEdit,mWndTabsHelp,mWndTabsOpts;  // main tabs on windows

	///  main menu
	WP mWndMainPanels[WND_ALL];
	Btn mWndMainBtns[WND_ALL];

	
public:
	inline bool bCam()  {  return  bMoveCam && !bGuiFocus;  }
	inline bool bEdit() {  return !bMoveCam && !bGuiFocus;  }
	
};
