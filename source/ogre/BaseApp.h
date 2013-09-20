#pragma once
#include <OgreVector3.h>
#include <OgreFrameListener.h>
#include <OgreWindowEventUtilities.h>

#include <MyGUI_Prerequest.h>
#include <MyGUI_Widget.h>
#include <MyGUI_OgrePlatform.h>

#include <boost/scoped_ptr.hpp>
#include "../sdl4ogre/events.h"
namespace SFO {  class InputWrapper;  class SDLCursorManager;  }
namespace ICS {  class InputControlSystem;  class DetectingBindingListener;  }

namespace MyGUI{  class OgreD3D11Platform; }
namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;
	namespace RTShader {  class ShaderGenerator;  }  }
namespace sh   {  class Factory;  }

class MasterClient;  class P2PGameClient;


//  mtr
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


//  gui
enum MNU_Btns {  MNU_Single=0, MNU_Tutorial, MNU_Champ, MNU_Challenge, MNU_Replays, MNU_Help, MNU_Options, ciMainBtns  };
enum TAB_Game {  TAB_Back=0, TAB_Track,TAB_Car, TAB_Setup,TAB_Game, TAB_Multi, TAB_Champs,TAB_Stages,TAB_Stage  };

enum LobbyState { DISCONNECTED, HOSTING, JOINED };



class BaseApp :
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
	virtual void Run( bool showDialog );
	
	bool bLoading,bLoadingEnd,bSimulating;  int iLoad1stFrames;
	
	//  has to be in baseApp for camera mouse move
	typedef std::vector<class CarModel*> CarModels;
	CarModels carModels;
	
	void showMouse(), hideMouse(), updMouse();
	
	//  stuff to be executed in App after BaseApp init
	virtual void postInit() = 0;
		
	///  effects
	class SplitScreenManager* mSplitMgr;
	class HDRLogic* mHDRLogic; class MotionBlurLogic* mMotionBlurLogic;
	class CameraBlurLogic* mCameraBlurLogic;
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

	sh::Factory* mFactory;

	//  wnd, hud, upl
	bool bWindowResized, bSizeHUD;
	float roadUpdTm;
	class LoadingBar* mLoadingBar;
	Ogre::SceneNode* ndSky;  //-
	
protected:
	bool mShowDialog, mShutDown;
	bool setup(), configure();  void updateStats();

	int mMouseX;
	int mMouseY;
	
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
	virtual bool mouseMoved( const SFO::MouseMotionEvent &arg );
	virtual bool mousePressed( const SDL_MouseButtonEvent &arg, Uint8 id );
	virtual bool mouseReleased( const SDL_MouseButtonEvent &arg, Uint8 id );
	virtual void textInput (const SDL_TextInputEvent& arg);
	virtual bool keyPressed(const SDL_KeyboardEvent &arg) = 0;
	virtual bool keyReleased(const SDL_KeyboardEvent &arg);
	virtual bool buttonPressed( const SDL_JoyButtonEvent &evt, int button );
	virtual bool buttonReleased( const SDL_JoyButtonEvent &evt, int button );
	virtual bool axisMoved( const SDL_JoyAxisEvent &arg, int axis );

	void onCursorChange (const std::string& name);

	///  Ogre
	Ogre::Root* mRoot;  Ogre::SceneManager* mSceneMgr;
	Ogre::RenderWindow* mWindow;
	SDL_Window* mSDLWindow;
	Ogre::RTShader::ShaderGenerator* mShaderGenerator;
	MaterialMgrListener* mMaterialMgrListener;  // Shader generator material manager listener.	

	virtual void windowResized (int x, int y);


	///  input
public:
	SFO::InputWrapper* mInputWrapper;
	SFO::SDLCursorManager* mCursorManager;
	ICS::InputControlSystem* mInputCtrl;
	ICS::InputControlSystem* mInputCtrlPlayer[4];
	std::vector<SDL_Joystick*> mJoysticks;
	
	// this is set to true when the user is asked to assign a new key
	bool bAssignKey;
	ICS::DetectingBindingListener* mBindListner;

	bool IsFocGuiInput()  {  return isFocGui || isFocRpl;  }
	bool IsFocGui();
	Ogre::RenderWindow* getWindow()  {  return mWindow;  }
protected:

	///  input
	bool alt, ctrl, shift;  // key modifiers
	bool mbLeft, mbRight, mbMiddle;  // mouse buttons
	bool mbWireFrame, mbShowCamPos;  // on/off
	int iCurCam;

	///  Gui
	bool isFocGui,isFocRpl;  // gui shown
	bool isTweak();
	MyGUI::Gui* mGui;  void baseInitGui(), baseSizeGui();
	MyGUI::ImageBox* bckFps, *imgBack;
	MyGUI::TextBox*	txFps;
public:
	//  loading
	MyGUI::ImageBox *bckLoad,*bckLoadBar,*barLoad, *imgLoad;
	MyGUI::TextBox *txLoadBig,*txLoad;
	int barSizeX, barSizeY;
protected:
	
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	MyGUI::OgreD3D11Platform* mPlatform;
	#else
	MyGUI::OgrePlatform* mPlatform;
	#endif
	
	///  main menu  // pSet->inMenu
	MyGUI::WidgetPtr mWndMainPanels[ciMainBtns];  MyGUI::ButtonPtr mWndMainBtns[ciMainBtns];

	MyGUI::WindowPtr mWndMain,mWndGame,mWndReplays,mWndHelp,mWndOpts,  // menu, windows
		mWndRpl, mWndNetEnd, mWndTweak,  // rpl controls, netw, tools
		mWndChampStage,mWndChampEnd, mWndChallStage,mWndChallEnd;
	MyGUI::TabPtr mWndTabsGame,mWndTabsOpts,mWndTabsHelp,mWndTabsRpl;  // main tabs on windows
	
	MyGUI::VectorWidgetPtr vwGui;  // all widgets to destroy

	///  networking
	boost::scoped_ptr<MasterClient> mMasterClient;
	boost::scoped_ptr<P2PGameClient> mClient;
	LobbyState mLobbyState;
};
