#pragma once
#include "BaseApp.h"
#include "common/Gui_Def.h"
#include "common/SliderValue.h"

#include <MyGUI_Enumerator.h>
#include "common/MessageBox/MessageBoxStyle.h"

#include "../network/networkcallbacks.hpp"
#include "../oics/ICSInputControlSystem.h"
#include <boost/thread.hpp>

#include "ChampsXml.h"  // progress..
#include "ChallengesXml.h"
#include "CInput.h"

namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;  class Viewport;  class Light;  }
namespace MyGUI{  class MultiList2;  class Slider;  class Message;  class PolygonalSkin;  }
namespace wraps {	class RenderBoxScene;  }
class Scene;  class SplineRoad;  class GAME;  class CHud;  class CData;
class CGuiCom;  class CarInfo;  class GuiPopup;


//  tracks,cars list items - with info for sorting
struct CarL
{
	std::string name;
	const CarInfo* ci;
};


class CGui : public BGui,
			 public GameClientCallback, public MasterClientCallback,
			 public ICS::DetectingBindingListener
{
public:
	App* app;  GAME* pGame;  SETTINGS* pSet;
	Scene* sc;  CData* data;
	CHud* hud;  MyGUI::Gui* mGui;  CGuiCom* gcom;
	
	CGui(App* ap1);
	~CGui();
	//friend class CarModel;
	
	typedef std::list <std::string> strlist;


	///  Gui
	///-----------------------------------------------------------------------------------------------------------------

	bool bGI;  // gui inited  set values
	void InitGui(), GuiUpdate();
	void UpdGuiAfterPreset();

	Txt valTrkNet;
	std::vector<Tab> vSubTabsGame, vSubTabsOpts;

	//  car list
	void CarListUpd(bool resetNotFound=false);
	void AddCarL(std::string name, const CarInfo* ci);
	std::list<CarL> liCar;
	void FillCarList();
	
	const static int colCar[16],colCh[16],colChL[16],colSt[16];


	//  util
	void toggleGui(bool toggle=true);
	void GuiShortcut(MNU_Btns mnu, int tab, int subtab=-1);
	bool loadReadme;
	

	//  hints
	const int iHints;  int iHintCur;  void UpdHint();
	Ck ckShowWelcome;
	Ed edHintTitle, edHintText;
	void btnHintPrev(WP), btnHintNext(WP);
	void btnHintScreen(WP), btnHintInput(WP), btnHintClose(WP);


	///  [Input] tab
	///-----------------------------------------------------------------------------------------------------------------

	//  bind events   . . . . .
	virtual void keyBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  SDL_Keycode key,
		ICS::Control::ControlChangingDirection direction);

	virtual void joystickAxisBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  int deviceId, int axis,
		ICS::Control::ControlChangingDirection direction);

	virtual void joystickButtonBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  int deviceId, unsigned int button,
		ICS::Control::ControlChangingDirection direction);

	//  not needed
	virtual void joystickPOVBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  int deviceId, int pov,
		ICS::InputControlSystem::POVAxis axis,
		ICS::Control::ControlChangingDirection direction) {  return;  }

	virtual void mouseAxisBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  ICS::InputControlSystem::NamedAxis axis,
		ICS::Control::ControlChangingDirection direction) {  return;  }

	virtual void mouseButtonBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  unsigned int button,
		ICS::Control::ControlChangingDirection direction) {  return;  }

	//  init
	void CreateInputTab( int iTab, bool player,
		const std::vector<InputAction>& actions, ICS::InputControlSystem* ICS);
	void InitInputGui();

	//  bind
	void inputBindBtnClicked(WP), inputUnbind(WP), inputBindBtn2(WP, int, int, MyGUI::MouseButton mb);
	enum EBind {  B_Done=0, B_First, B_Second  };
	void UpdateInputButton(Btn button, const InputAction& action, EBind bind = B_Done);

	InputAction* mBindingAction;
	Btn mBindingSender;

	virtual void notifyInputActionBound(bool complete);
	bool actionIsActive(std::string, std::string);

	//  input gui
	Tab tabInput;  void tabInputChg(Tab, size_t);
	Txt txtInpDetail;  WP panInputDetail;  Btn chOneAxis;
	Ed edInputIncrease;

	void editInput(Ed), btnInputInv(WP), chkOneAxis(WP);
	void comboInputKeyAllPreset(CMB);
	void UpdateInputBars(), inputDetailBtn(WP);
	bool TabInputId(int* pId);


	///  [Tweak]  -----------------------------------------
	const static int ciEdCar = 12;
	Ed edCar[ciEdCar],edPerfTest, edTweakCol;
	Txt txtTweakPath, txtTweakTire, txtTweakPathCol;

	Tab tabTweak, tabEdCar;
	void tabCarEdChng(Tab, size_t), tabTweakChng(Tab, size_t);

	///  tire
	Ed edTweakTireSet;  void editTweakTireSet(Ed);
	Li liTwkTiresUser, liTwkTiresOrig;
	void listTwkTiresUser(Li, size_t), listTwkTiresOrig(Li, size_t);
	void btnTweakTireLoad(WP), btnTweakTireReset(WP), btnTweakTireDelete(WP);
	void FillTweakLists();  Ogre::String sTireLoad;
	
	///  surface
	Li liTwkSurfaces;  void listTwkSurfaces(Li, size_t);
	int idTwkSurf;  void btnTwkSurfPick(WP), updSld_TwkSurf(int id);
	SV svSuFrict, svSuFrictX, svSuFrictY, svSuRollDrag, svSuRollRes;
	SV svSuBumpWave, svSuBumpAmp, svSuBumpWave2, svSuBumpAmp2;
	Cmb cmbSurfTire, cmbSurfType;  void comboSurfTire(CMB), comboSurfType(CMB);

	void TweakToggle();
	void TweakCarSave(),TweakCarLoad(), TweakTireSave();

	void TweakColUpd(bool user), TweakColLoad(),TweakColSave();

	const static Ogre::String csLateral[15][2], csLongit[13][2], csAlign[18][2], sCommon;

	void btnTweakCarSave(WP),  btnTweakCarLoad(WP);
	void btnTweakTireSave(WP), btnTweakColSave(WP);

	bool GetCarPath(std::string* pathCar/*out*/,
		std::string* pathSave/*=0*/, std::string* pathSaveDir/*=0*/,
		std::string carname, bool forceOrig=false);

	//  graphs
	Cmb cmbGraphs;  void comboGraphs(CMB);  Txt valGraphsType;


	///  [Options]  game only
	///-----------------------------------------------------------------------------------------------------------------
	//  reflection
	SV svReflSkip, svReflFaces, svReflSize;
	SlV(ReflDist);  SlV(ReflMode);

	//  hud view
	SV svSizeGaug;
	SV svTypeGaug, svLayoutGaug;
	
	SV svSizeMinimap, svZoomMinimap;
	void slHudSize(SV*), slHudCreate(SV*);

	SlV(SizeArrow);
	SLV(CountdownTime);  //-
	SV svDbgTxtClr, svDbgTxtCnt;

	//  sound
	SlV(VolMaster);  SlV(VolHud);
	SV svVolEngine, svVolTires, svVolSusp, svVolEnv;
	SV svVolFlSplash, svVolFlCont, svVolCarCrash, svVolCarScrap;
	Ck ckSndChk, ckSndChkWr, ckReverb;
	
	
	///  Checks  . . . . . . . . . . . . . . . . . . . .
	CK(Reverse);  // track

	//  Options
	SV svParticles, svTrails;
	Ck ckParticles, ckTrails;  void chkParTrl(Ck*);

	//  Hud view
	Ck ckDigits, ckGauges;  void chkHudShow(Ck*);
	//  Minimap
	CK(Arrow);  CK(Beam);
	CK(Minimap);  void chkMiniUpd(Ck*);
	Ck ckMiniZoom, ckMiniRot, ckMiniTer, ckMiniBorder;
	//  Camera
	Ck ckCamInfo, ckCamTilt, ckCamLoop;
	Ck ckCamBnc;  SV svCamBnc;
	SV svFov, svFovMax, svFovSm;
	//  Pacenotes
	Ck ckPaceShow;  SV svPaceDist, svPaceSize, svPaceNext;
	SV svPaceNear, svPaceAlpha;
	void slUpd_Pace(SV*);
	//  Times, opp
	Ck ckTimes, ckOpponents, ckOppSort;

	//  Graphs
	SV svTC_r, svTC_xr;
	SV svTE_yf, svTE_xfx, svTE_xfy, svTE_xpow;
	Ck ckTE_Common, ckTE_Reference;  void chkTEupd(Ck*);
	
	//  Hud dbg,other
	Ck ckFps;  CK(Wireframe);
	//  profiler
	Ck ckProfilerTxt, ckBulletDebug, ckBltProfTxt, ckSoundInfo;
	//  car dbg
	Ck ckCarDbgBars, ckCarDbgTxt, ckCarDbgSurf;
	Ck ckTireVis;  void chkHudCreate(Ck*);
	CK(Graphs);

	//  Startup
	Ck ckAutoStart, ckEscQuits;
	Ck ckStartInMain, ckOgreDialog;
	Ck ckBltLines, ckShowPics;
	Ck ckMouseCapture, ckDevKeys, ckScreenPng;
	void chkMultiThread(WP);
	
	//  [Effects]
	CK(AllEffects);
	Ck ckBoostFOV;
	Ck ckBloom, ckBlur, ckSoftPar, ckSSAO, ckGodRays, ckDoF, ckHDR;
	void chkEffUpd(Ck*), chkEffUpdShd(Ck*);

	SV svBloomInt, svBloomOrig;
	SV svBlurIntens;  // motion blur
	SV svDofFocus, svDofFar;  // depth of field
	void slEffUpd(SV*);
	//  hdr
	SV svHDRParam1, svHDRParam2, svHDRParam3;
	SV svHDRBloomInt, svHDRBloomOrig, svHDRAdaptScale;
	SV svHDRVignRadius, svHDRVignDark;


	///  Car 3d view  ---
	CarModel* viewCar;
	Can viewCanvas;
	wraps::RenderBoxScene* viewBox;  Ogre::Vector3 viewSc;
	MyGUI::IntCoord GetViewSize();
	void InitCarPrv();
	
	WP graphV, graphS;
	MyGUI::PolygonalSkin* graphVel,*graphVGrid, *graphSSS,*graphSGrid;


	///  [Car] color  --===---
	SV svCarClrH, svCarClrS, svCarClrV;
	SV svCarClrGloss, svCarClrRefl;  void slCarClr(SV*);
	void SldUpd_CarClr();
	void UpdCarClrSld(bool upd=true);
	void SetCarClr();


	//  [Setup] car
	Ck ckCarGear, ckCarRear, ckCarRearInv;  void chkGear(Ck*);
	Ck ckAbs, ckTcs;
	Btn bchAbs,bchTcs;
	void chkAbs(WP), chkTcs(WP);

	//  gui car tire set gravel/asphalt
	int iTireSet;
	void tabTireSet(Tab, size_t);
	void SldUpd_TireSet();

	SV svSSSEffect, svSSSVelFactor;
	SV svSteerRangeSurf, svSteerRangeSim;
	void btnSSSReset(WP), btnSteerReset(WP), slSSS(SV*);

	void imgBtnCarClr(WP), btnCarClrRandom(WP);
	
	//  radios
	Btn bRkmh, bRmph;  // km/h, mph
	void radKmh(WP), radMph(WP);

	Btn bRsimEasy, bRsimNorm, bRsimHard;  // sim mode
	void radSimEasy(WP), radSimNorm(WP), radSimHard(WP);
	void setSimMode(std::string mode);
	bool bReloadSim;

	//  [Game] setup
	Ck ckVegetCollis, ckCarCollis, ckRoadWCollis, ckDynamicObjs;
	SV svNumLaps;  SLV(RplNumViewports);  //-
	SV svDamageDec;
	SV svBmin,svBmax,svBpow,svBperKm,svBaddSec;


	///  [Replay]  -----------------------------
	Li rplList;
	void listRplChng(Li, size_t);
	void updReplaysList();

	//  cur rpl stats gui
	Txt valRplName, valRplInfo,
		valRplName2,valRplInfo2;
	
	//  controls percent and time info
	Txt valRplPerc, valRplCur, valRplLen;

	//  gui save
	Ed edRplName, edRplDesc;
	Ogre::String getRplName();
	void btnRplLoad(WP), btnRplSave(WP);
	void btnRplDelete(WP), btnRplRename(WP);
	
	//  chk, options
	Ck ckRplAutoRec, ckRplBestOnly, ckRplGhost, ckRplParticles;
	Ck ckRplRewind, ckRplGhostOther, ckRplTrackGhost;
	SV svGhoHideDist, svGhoHideDistTrk;

	//  list filtering
	Btn rbRplCur, rbRplAll;  // radio
	void btnRplCur(WP),btnRplAll(WP);
	CK(RplGhosts);
	void edRplFind(Ed);  Ogre::String sRplFind;

	//  controls bar buttons
	Btn btRplPl;  void UpdRplPlayBtn();
	Sl slRplPos;  void slRplPosEv(SL);
	bool bRplBack, bRplFwd;
	void btnRplToStart(WP),btnRplToEnd(WP), btnRplPlay(WP);
	void btnRplBackDn(WP,int,int,MyGUI::MouseButton), btnRplBackUp(WP,int,int,MyGUI::MouseButton);
	void btnRplFwdDn(WP,int,int, MyGUI::MouseButton), btnRplFwdUp(WP,int,int, MyGUI::MouseButton);
	void msgRplDelete(MyGUI::Message*, MyGUI::MessageBoxStyle);

	//  tools, convert
	void btnRenameOldTrk(WP), btnConvertAllRpl(WP);
	bool bConvertRpl;
	boost::thread mThrConvert;  void ThreadConvert();
	Txt txtConvert;
	int iConvCur,iConvAll,iConvFiles, iConvPathCur,iConvPathAll;  // files, dirs
	boost::uintmax_t totalConv,totalConvCur,totalConvNew;  // size
	

	//  Game
	///---------------------------------------
	Btn btNewGameCar;
	void btnNewGame(WP), btnNewGameStart(WP);

	//  split
	void btnNumPlayers(WP);
	Txt valLocPlayers;
	Ck ckSplitVert;
	void chkStartOrd(WP);

	//  [Car] list
	int iCurCar;  // current
	Ogre::String sListCar;

	Mli2 carList;
	void listCarChng(Mli2, size_t);
	void btnCarView1(WP), btnCarView2(WP);

	void changeCar(), changeTrack();

	//  [Car] stats
	const static int iCarSt = 10;
	Img barCarSt[iCarSt];
	Txt txCarStTxt[iCarSt], txCarStVals[iCarSt],
		txCarSpeed, txCarType, txCarYear,
		txCarAuthor,txTrackAuthor;
	Img barCarSpeed;
	void UpdCarStats(bool car);
	std::vector<Ogre::String> vsu; //CarStatsUnits

	Img imgCar;  Ed carDesc;
	Cmb cmbBoost, cmbFlip, cmbDamage, cmbRewind;
	void comboBoost(CMB), comboFlip(CMB), comboDamage(CMB), comboRewind(CMB);


	//  key util
	int LNext(Mli2, int rel, int ofs), LNext(Li, int rel, int ofs),
		LNext(Mli, int rel);  // util next in list
	void LNext(int rel);  void tabPlayer(Tab, size_t);

	const Ogre::String& GetGhostFile(std::string* ghCar=NULL);
	std::string GetRplListDir();


	///  championships & challenges
	///-----------------------------------------------------------------------------------------------------------------
	Btn btStTut, btStChamp, btStChall;
	Img imgTut, imgChamp, imgChall;
	//  tabs
	Tab tabTut, tabChamp, tabChall;
	void tabTutType(Tab, size_t), tabChampType(Tab, size_t);
	void tabChallType(Tab, size_t);

	//  stages
	Ed edChInfo, edChDesc;  WP panCh;
	Txt txtCh, valCh, txtChP[3], valChP[3];  // stages info, pass/progress
	void btnStageNext(WP), btnStagePrev(WP);  Txt valStageNum;
	void StageListAdd(int n, Ogre::String name, int laps, Ogre::String progress);
	
	//  xml  [1]= reversed  L= challenge
	ProgressXml progress[2];
	ProgressLXml progressL[2];
	void ProgressSave(bool upgGui=true), ProgressLSave(bool upgGui=true);
	Chall* pChall;  // current challenge or 0 if not

	//  load
	void Ch_XmlLoad(), Ch_LoadEnd();
	void UpdChallDetail(int id);
	//  const
	const static Ogre::String StrPrize(int i/*0 none..3 gold*/), strPrize[4],clrPrize[4];
	const static int ciAddPos[3];  const static float cfSubPoints[3];
	
	//  common
	Mli2 liStages, liNetEnd;  void listStageChng(Mli2, size_t);
	Mli2 liChamps;  void listChampChng(Mli2, size_t);
	Mli2 liChalls;  void listChallChng(Mli2, size_t);

	void btnChampStart(WP), btnChampEndClose(WP), btnChampStageBack(WP), btnChampStageStart(WP);
	void btnChallStart(WP), btnChallEndClose(WP), btnChallStageBack(WP), btnChallStageStart(WP);
	void btnChRestart(WP);  Btn btChRestart;

	Btn btChampStage, btChallStage;
	Ed edChampStage, edChampEnd;  Img imgChampStage, imgChampEndCup;
	Ed edChallStage, edChallEnd;  Img imgChallStage;
	Img imgChallFail, imgChallCup;
	Txt txChallEndC, txChallEndF, txChampEndF;
	int iChSnd;  // snd id to play

	//  main
	void ChampsListUpdate(), ChampFillStageInfo(bool finished), ChampionshipAdvance(float timeCur);
	void ChallsListUpdate(), ChallFillStageInfo(bool finished), ChallengeAdvance(float timeCur);
	void btnChampInfo(WP), UpdChampTabVis();
	CK(ChampRev);

	void ReadTrkStatsChamp(Ogre::String track,bool reverse);
	void updChampListDim();

	//  chall util
	Ogre::String StrChallCars(const Chall& ch);
	bool IsChallCar(Ogre::String name);
	bool isChallGui();  void BackFromChs();

	//  _Tools_
	void ToolGhosts(),ToolGhostsConv(), ToolTestTrkGhosts();


	///  multiplayer game
	///-----------------------------------------------------------------------------------------------------------------
	void rebuildGameList(), rebuildPlayerList();
	void updateGameInfo(), updateGameSet(), updateGameInfoGUI();
	void setNetGuiHosting(bool enabled);
	void gameListChanged(protocol::GameList list);

	void peerConnected(PeerInfo peer), peerDisconnected(PeerInfo peer);
	void peerInfo(PeerInfo peer);
	void peerMessage(PeerInfo peer, std::string msg);
	void peerState(PeerInfo peer, uint8_t state);
	void gameInfo(protocol::GameInfo game);
	void startRace(), returnToLobby();
	void timeInfo(ClientID id, uint8_t lap, double time);
	void error(std::string what);
	void join(std::string host, std::string port, std::string password);
	void uploadGameInfo();

	mutable boost::mutex netGuiMutex;
	protocol::GameInfo netGameInfo;

	///  multiplayer gui  --------------------
	Tab tabsNet;
	WP  panNetServer, panNetGame, panNetTrack;
	Mli listServers, listPlayers;
	int iColLock, iColHost, iColPort;  // ids of columns in listServers

	//  upd gui triggers
	bool bRebuildPlayerList, bRebuildGameList;
	bool bUpdateGameInfo, bUpdChat;
	bool bStartGame, bStartedGame;
	void UpdGuiNetw();

	//  chat,msg  ----
	Ed edNetChat;  // chat area, set text through sChatBuffer
	MyGUI::UString sChatBuffer,sChatLast1,sChatLast2;  int iChatMove;
	void AddChatMsg(const MyGUI::UString& clr, const MyGUI::UString& msg, bool add=true);

	Ed edNetChatMsg;
	Btn btnNetSendMsg;  void chatSendMsg();
	GuiPopup* popup;  // msg with edits

	//  Net gui
	Btn btnNetRefresh, btnNetJoin;    void evBtnNetRefresh(WP), evBtnNetJoin(WP),   evBtnNetJoinLockedClose();
	Btn btnNetCreate,  btnNetDirect;  void evBtnNetCreate(WP),  evBtnNetDirect(WP), evBtnNetDirectClose();
	Btn btnNetReady,   btnNetLeave;	  void evBtnNetReady(WP),   evBtnNetLeave(WP);
	void btnNetEndClose(WP);

	Txt valNetGameInfo, valNetPassword;
	Ed edNetGameName,   edNetPassword;   void evEdNetGameName(Ed),   evEdNetPassword(Ed);
	Ed edNetNick,       edNetLocalPort;  void evEdNetNick(Ed),       evEdNetLocalPort(Ed);
	Ed edNetServerPort, edNetServerIP;   void evEdNetServerPort(Ed), evEdNetServerIP(Ed);
};
