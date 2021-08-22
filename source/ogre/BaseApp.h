#pragma once
#include "common/Gui_Def.h"
#include <OgreFrameListener.h>
#include <OgreWindowEventUtilities.h>
#include <OgreMaterialManager.h>
#include <boost/scoped_ptr.hpp>
#include "../sdl4ogre/events.h"
namespace SFO {  class InputWrapper;  class SDLCursorManager;  }
namespace ICS {  class InputControlSystem;  class DetectingBindingListener;  }

namespace MyGUI{  class OgreD3D11Platform;  class OgrePlatform;  }
namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow; }
namespace sh   {  class Factory;  }

class MasterClient;  class P2PGameClient;


//  main, race menus
enum Menu_Btns {  Menu_Race, Menu_Replays, Menu_Help, Menu_Options,  ciMainBtns };
enum Race_Btns {  Race_Single, Race_Tutorial, Race_Champ, Race_Challenge,
				  Race_Difficulty, Race_Simulation, Race_HowToPlay, Race_Back,  ciRaceBtns };
//  gui
enum TAB_Game    {  TAB_Back=0, TAB_Track,TAB_Car, TAB_Setup, TAB_Split,TAB_Multi, TAB_Champs,TAB_Stages,TAB_Stage  };
enum TAB_Options {  TABo_Back=0, TABo_Screen, TABo_Input, TABo_View, TABo_Graphics, TABo_Sound, TABo_Settings, TABo_Tweak  };
enum LobbyState  {  DISCONNECTED, HOSTING, JOINED  };



class BaseApp : public BGui,
	public Ogre::FrameListener,
	public SFO::KeyListener, public SFO::MouseListener,
	public SFO::JoyListener, public SFO::WindowListener
{
	friend class CarModel;
	friend class CGame;
	friend class CHud;
	friend class CGui;

public:
	BaseApp();
	virtual ~BaseApp();
	virtual void Run(bool showDialog);
	
	bool bLoading =0, bLoadingEnd =0, bSimulating =0;  int iLoad1stFrames =0;
	
	//  has to be in baseApp for camera mouse move
	typedef std::vector<class CarModel*> CarModels;
	CarModels carModels;
	
	void showMouse(), hideMouse(), updMouse();
	
	//  stuff to be executed in App after BaseApp init
	virtual void postInit() = 0;
		
	///  effects
	class SplitScr* mSplitMgr =0;
	class HDRLogic* mHDRLogic =0;
	class MotionBlurLogic* mMotionBlurLogic =0;
	class CameraBlurLogic* mCameraBlurLogic =0;
	class SSAOLogic* mSSAOLogic =0;
	class GodRaysLogic* mGodRaysLogic =0;
	class SoftParticlesLogic* mSoftParticlesLogic =0;
	class DepthOfFieldLogic* mDepthOfFieldLogic =0;
	class GBufferLogic* mGBufferLogic =0;
	class FilmGrainLogic* mFilmGrainLogic =0;
	void recreateCompositor();
	bool AnyEffectEnabled();
	bool NeedMRTBuffer();
	float motionBlurIntensity =0.9f;

	class SETTINGS* pSet =0;

	sh::Factory* mFactory =0;

	//  wnd, hud, upl
	bool bWindowResized =1, bSizeHUD =1, bRecreateHUD =0;
	float roadUpdTm =0.f;
	class LoadingBar* mLoadingBar =0;
	Ogre::SceneNode* ndSky =0;  //-
	

	bool mShowDialog =0, mShutDown =0;
	bool setup(), configure();  void updateStats();

	int mMouseX =0, mMouseY =0;
	
	///  create
	virtual void createScene() = 0;
	virtual void destroyScene() = 0;

	void createFrameListener(), createViewports(), refreshCompositor(bool disableAll=false);
	void setupResources(), createResourceListener(), loadResources();
	void LoadingOn(), LoadingOff();

	///  frame events
	bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	bool frameEnded(const Ogre::FrameEvent& evt);
	virtual bool frameStart(Ogre::Real time) = 0;
	virtual bool frameEnd(Ogre::Real time) = 0;
	
	///  input events
	virtual bool mouseMoved(const SFO::MouseMotionEvent &arg );
	virtual bool mousePressed(const SDL_MouseButtonEvent &arg, Uint8 id );
	virtual bool mouseReleased(const SDL_MouseButtonEvent &arg, Uint8 id );
	virtual void textInput(const SDL_TextInputEvent& arg);
	virtual bool keyPressed(const SDL_KeyboardEvent &arg) = 0;
	virtual bool keyReleased(const SDL_KeyboardEvent &arg);
	virtual bool buttonPressed(const SDL_JoyButtonEvent &evt, int button );
	virtual bool buttonReleased(const SDL_JoyButtonEvent &evt, int button );
	virtual bool axisMoved(const SDL_JoyAxisEvent &arg, int axis );

	void onCursorChange(const std::string& name);

	///  Ogre
	Ogre::Root* mRoot =0;  Ogre::SceneManager* mSceneMgr =0;
	Ogre::RenderWindow* mWindow =0;
	SDL_Window* mSDLWindow =0;

	virtual void windowResized (int x, int y);
	virtual void windowClosed();

	///  input
	SFO::InputWrapper* mInputWrapper =0;
	SFO::SDLCursorManager* mCursorManager =0;
	ICS::InputControlSystem* mInputCtrl =0;
	ICS::InputControlSystem* mInputCtrlPlayer[4] ={0,};
	std::vector<SDL_Joystick*> mJoysticks;
	
	// this is set to true when the user is asked to assign a new key
	bool bAssignKey =0;
	ICS::DetectingBindingListener* mBindListner =0;

	bool IsFocGuiInput()  {  return isFocGui || isFocRpl;  }
	bool IsFocGui();
	Ogre::RenderWindow* getWindow()  {  return mWindow;  }

	///  input
	bool alt =0, ctrl =0, shift =0;  // key modifiers
	bool mbLeft =0, mbRight =0, mbMiddle =0;  // mouse buttons
	bool mbWireFrame =0;
	int iCurCam =0;

	///  Gui  ..........................
	bool isFocGui =0, isFocRpl =0;  // gui shown
	bool isTweak();
	
	MyGUI::Gui* mGui =0;
	void baseInitGui(), baseSizeGui();

	Img bckFps =0, imgBack =0;
	Txt txFps =0;

	//  loading
	Img bckLoad =0, bckLoadBar =0, barLoad =0, imgLoad =0;
	Txt txLoadBig =0, txLoad =0;
	int barSizeX =0, barSizeY =0;


	MyGUI::OgrePlatform* mPlatform;
	
	///  main menu  // pSet->inMenu
	WP mWndMainPanels[ciMainBtns] ={0,}, mWndRacePanels[ciRaceBtns] ={0,};
	Btn mWndMainBtns[ciMainBtns]  ={0,}, mWndRaceBtns[ciRaceBtns]   ={0,};

	Wnd mWndMain =0, mWndRace =0,
		mWndGame =0,mWndReplays =0,  mWndHelp =0, mWndOpts =0,  // menu, windows
		mWndWelcome =0, mWndHowTo =0, mWndRpl =0, mWndRplTxt =0,
		mWndNetEnd =0, mWndTweak =0, mWndTrkFilt =0,  // rpl controls, netw, tools
		mWndChampStage =0, mWndChampEnd =0,
		mWndChallStage =0, mWndChallEnd =0;
	Tab mWndTabsGame =0, mWndTabsRpl =0, mWndTabsHelp =0, mWndTabsOpts =0;  // main tabs on windows
	
	//MyGUI::VectorWidgetPtr
	std::vector<WP> vwGui;  // all widgets to destroy

	///  networking
	boost::scoped_ptr<MasterClient> mMasterClient;
	boost::scoped_ptr<P2PGameClient> mClient;
	LobbyState mLobbyState =DISCONNECTED;
};
