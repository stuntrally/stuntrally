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
#include "../settings.h"

namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;  class Viewport;  class Light;  }
namespace MyGUI{  class MultiList2;  class Slider;  class Message;  class PolygonalSkin;  }
namespace wraps {	class RenderBoxScene;  }
class Scene;  class SplineRoad;  class GAME;  class CHud;  class CData;
class CGuiCom;  class CarInfo;  class GuiPopup;


//  cars list items - with info for sorting
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
	App* app =0;  GAME* pGame =0;  SETTINGS* pSet =0;
	Scene* sc =0;  CData* data =0;
	CHud* hud =0;  MyGUI::Gui* mGui =0;  CGuiCom* gcom =0;

	CGui(App* ap1);
	~CGui();
	//friend class CarModel;

	typedef std::list <std::string> strlist;


	//  main menu
	void InitMainMenu();
	void btnMainMenu(WP);  void tabMainMenu(Tab tab, size_t id);

	Cmb simList;  void comboSim(CMB);
	Cmb diffList;  void comboDiff(CMB);


	///  Gui
	///-----------------------------------------------------------------------------------------------------------------

	bool bGI =0;  // gui inited  set values
	void InitGui(), GuiUpdate();
	void UpdGuiAfterPreset();

	Txt valTrkNet =0;
	std::vector<Tab> vSubTabsGame, vSubTabsOpts;

	//  car list
	void CarListUpd(bool resetNotFound=false);
	void AddCarL(std::string name, const CarInfo* ci);
	std::list<CarL> liCar;
	void FillCarList();

	const static int colCar[16],colCh[16],colChL[16],colSt[16];


	//  util
	void toggleGui(bool toggle=true);
	void GuiShortcut(EMenu menu, int tab, int subtab=-1);
	bool loadReadme = 1;  void FillHelpTxt();
	void ImgPrvClk(WP), ImgTerClk(WP), ImgPrvClose(WP);


	//  hints
	Ck ckShowWelcome;
	const static int iHints;  int iHintCur = 0;
	Ed edHintTitle =0, edHintText =0, rplSubText =0;
	Img imgHint =0, rplSubImg =0;

	void UpdHint(), setHintImg(Img img, int h);
	void btnHintPrev(WP), btnHintNext(WP);
	void btnHintScreen(WP), btnHintInput(WP), btnHintClose(WP);
	void btnHowToBack(WP), btnLesson(WP);

	struct Subtitle  // for replay lessons
	{
		std::string txt;
		float beg, end;  // time
		int hintImg;
		Subtitle(float beginTime, float endTime, int imgId, std::string text)
			:txt(text), beg(beginTime), end(endTime), hintImg(imgId)
		{	}
	};
	std::list<Subtitle> rplSubtitles;


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

	InputAction* mBindingAction =0;
	Btn mBindingSender =0;

	virtual void notifyInputActionBound(bool complete);
	bool actionIsActive(std::string, std::string);

	//  input gui
	Tab tabInput =0;  void tabInputChg(Tab, size_t);
	Txt txtInpDetail =0;  WP panInputDetail =0;  Btn chOneAxis =0;
	Ed edInputIncrease =0;

	void editInput(Ed), btnInputInv(WP), chkOneAxis(WP);
	void comboInputKeyAllPreset(CMB);
	void UpdateInputBars(), inputDetailBtn(WP);
	bool TabInputId(int* pId);


	///  [Tweak]  -----------------------------------------
	const static int ciEdCar = 12;
	Ed edCar[ciEdCar] ={0,}, edPerfTest =0, edTweakCol =0;
	Txt txtTweakPath =0, txtTweakTire =0, txtTweakPathCol;

	Tab tabTweak =0, tabEdCar =0;
	void tabCarEdChng(Tab, size_t), tabTweakChng(Tab, size_t);

	///  tire
	Ed edTweakTireSet =0;  void editTweakTireSet(Ed);
	Li liTwkTiresUser =0, liTwkTiresOrig =0;
	void listTwkTiresUser(Li, size_t), listTwkTiresOrig(Li, size_t);
	void btnTweakTireLoad(WP), btnTweakTireReset(WP), btnTweakTireDelete(WP);
	void FillTweakLists();  Ogre::String sTireLoad;

	///  surface
	Li liTwkSurfaces =0;  void listTwkSurfaces(Li, size_t);
	int idTwkSurf = -1;  void btnTwkSurfPick(WP), updSld_TwkSurf(int id);
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
	Cmb cmbGraphs =0;  void comboGraphs(CMB);  Txt valGraphsType =0;


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
	SV svFov, svFovBoost, svFovSm;
	//  Pacenotes
	Ck ckPaceShow;  SV svPaceDist, svPaceSize, svPaceNext;
	SV svPaceNear, svPaceAlpha;
	void slUpd_Pace(SV*);
	//  Trail
	CK(TrailShow);
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
	CarModel* viewCar =0;
	Can viewCanvas;
	wraps::RenderBoxScene* viewBox =0;  Ogre::Vector3 viewSc;
	MyGUI::IntCoord GetViewSize();
	void InitCarPrv();

	WP graphV =0, graphS =0;
	MyGUI::PolygonalSkin* graphVel =0,*graphVGrid =0, *graphSSS =0,*graphSGrid =0;


	///  [Car] color  --===---
	SV svCarClrH, svCarClrS, svCarClrV;
	SV svCarClrGloss, svCarClrRefl;  void slCarClr(SV*);
	void SldUpd_CarClr();
	void UpdCarClrSld(bool upd=true);
	void SetCarClr(), UpdImgClr();


	//  [Setup] car
	Ck ckCarGear, ckCarRear, ckCarRearInv;  void chkGear(Ck*);
	Ck ckAbs, ckTcs;
	Btn bchAbs =0, bchTcs =0;
	void chkAbs(WP), chkTcs(WP);

	//  gui car tire set gravel/asphalt
	int iTireSet = 0;
	void tabTireSet(Tab, size_t);
	void SldUpd_TireSet();

	SV svSSSEffect, svSSSVelFactor;
	SV svSteerRangeSurf, svSteerRangeSim;
	void btnSSSReset(WP), btnSteerReset(WP), slSSS(SV*);

	void imgBtnCarClr(WP), btnCarClrRandom(WP);  Img imgCarClr =0;

	//  radios
	Btn bRkmh =0, bRmph =0;  // km/h, mph
	void radKmh(WP), radMph(WP), radUpd(bool kmh);

	bool bReloadSim = 1;

	//  [Game] setup
	Ck ckVegetCollis, ckCarCollis, ckRoadWCollis, ckDynamicObjs;
	SV svNumLaps;  SLV(RplNumViewports);  //-
	SV svDamageDec;
	SV svBmin,svBmax,svBpow,svBperKm,svBaddSec;


	///  [Replay]  -----------------------------
	Li rplList =0;
	void listRplChng(Li, size_t);
	void updReplaysList();

	//  cur rpl stats gui
	Txt valRplName =0, valRplInfo =0,
		valRplName2 =0,valRplInfo2 =0;

	//  controls percent and time info
	Txt valRplPerc =0, valRplCur =0, valRplLen =0;

	//  gui save
	Ed edRplName =0, edRplDesc =0;
	Ogre::String getRplName();
	void btnRplLoad(WP), btnRplSave(WP), btnRplLoadFile(std::string file);
	void btnRplDelete(WP), btnRplRename(WP);
	bool bLesson =0;

	//  chk, options
	Ck ckRplAutoRec, ckRplBestOnly, ckRplGhost, ckRplParticles;
	Ck ckRplRewind, ckRplGhostOther, ckRplTrackGhost;
	SV svGhoHideDist, svGhoHideDistTrk;
	Ck ckRplHideHudAids;

	//  list filtering
	Btn rbRplCur =0, rbRplAll =0;  // radio
	void btnRplCur(WP),btnRplAll(WP);
	CK(RplGhosts);
	void edRplFind(Ed);  Ogre::String sRplFind;

	//  controls bar buttons
	Btn btRplPl =0;  void UpdRplPlayBtn();
	Sl slRplPos =0;  void slRplPosEv(SL);
	bool bRplBack =0, bRplFwd =0;
	void btnRplToStart(WP),btnRplToEnd(WP), btnRplPlay(WP);
	void btnRplBackDn(WP,int,int,MyGUI::MouseButton), btnRplBackUp(WP,int,int,MyGUI::MouseButton);
	void btnRplFwdDn(WP,int,int, MyGUI::MouseButton), btnRplFwdUp(WP,int,int, MyGUI::MouseButton);
	void msgRplDelete(MyGUI::Message*, MyGUI::MessageBoxStyle);


	//  Game
	///---------------------------------------
	Btn btNewGameCar =0;
	void btnNewGame(WP), btnNewGameStart(WP);

	//  split
	void btnNumPlayers(WP);
	Txt valLocPlayers =0;
	Ck ckSplitVert;
	void chkStartOrd(WP);

	//  [Car] list  (all Car = Vehicle)
	int iCurCar = 0;  // current
	Ogre::String sListCar;

	Mli2 carList =0;
	void listCarChng(Mli2, size_t);
	void btnCarView1(WP), btnCarView2(WP);

	void changeCar(), changeTrack();

	//  [Car] stats
	const static int iCarSt = 10, iDrvSt = 3;
	Img barCarSt[iCarSt], barCarSpeed =0;
	Txt txCarStTxt[iCarSt], txCarStVals[iCarSt],
		txCarSpeed =0, txCarType =0, txCarYear =0,
		txCarRating =0, txCarDiff =0, txCarWidth =0,
		txCarAuthor =0, txTrackAuthor =0,
		txTrkDrivab[iDrvSt] = {0,0,0};
	Img imgTrkDrivab[iDrvSt] = {0,0,0};
	Tab tbPlr =0, tbPlr2 =0;

	void UpdCarStats(bool car), UpdDrivability(std::string trk, bool track_user);

	Img imgCar =0;  Ed carDesc =0;
	Cmb cmbBoost =0, cmbFlip =0, cmbDamage =0, cmbRewind =0;
	void comboBoost(CMB), comboFlip(CMB), comboDamage(CMB), comboRewind(CMB);


	//  key util
	int LNext(Mli2, int rel, int ofs), LNext(Li, int rel, int ofs),
		LNext(Mli, int rel);  // util next in list
	void LNext(int rel);  void tabPlayer(Tab, size_t);

	const Ogre::String& GetGhostFile(std::string* ghCar=NULL);
	std::string GetRplListDir();


	///  championships & challenges
	///-----------------------------------------------------------------------------------------------------------------
	Btn btStTut =0, btStChamp =0, btStChall =0;
	Img imgTut =0, imgChamp =0, imgChall =0;
	//  tabs
	Tab tabChamp =0, tabChall =0;

	//void tabTutType(Tab, size_t);
	void tabChampType(Tab, size_t), tabChallType(Tab, size_t);

	//  stages
	Ed edChInfo =0, edChDesc =0;  WP panCh =0;
	Txt txtCh =0, valCh =0, txtChP[3] ={0,0,0}, valChP[3] ={0,0,0};  // stages info, pass/progress

	void btnStageNext(WP), btnStagePrev(WP);  Txt valStageNum =0;
	void StageListAdd(int n, Ogre::String name, int laps, Ogre::String progress);

	//  xml  [1]= reversed  L= challenge
	ProgressXml progress[2];
	ProgressLXml progressL[2];

	void ProgressSave(bool upgGui=true), ProgressLSave(bool upgGui=true);
	Chall* pChall =0;  // current challenge or 0 if not

	//  load
	void Ch_XmlLoad(), Ch_LoadEnd();
	void UpdChallDetail(int id);
	//  const
	const static Ogre::String StrPrize(int i/*0 none..3 gold*/), strPrize[4],clrPrize[4];
	const static int ciAddPos[3];  const static float cfSubPoints[3];

	//  common
	Mli2 liStages =0, liNetEnd =0;  void listStageChng(Mli2, size_t);
	Mli2 liChamps =0;  void listChampChng(Mli2, size_t);
	Mli2 liChalls =0;  void listChallChng(Mli2, size_t);

	void btnChampStart(WP), btnChampEndClose(WP), btnChampStageBack(WP), btnChampStageStart(WP);
	void btnChallStart(WP), btnChallEndClose(WP), btnChallStageBack(WP), btnChallStageStart(WP);
	void btnChRestart(WP);  Btn btChRestart =0;

	Btn btChampStage =0, btChallStage =0;
	Ed edChampStage =0, edChampEnd =0;  Img imgChampStage =0, imgChampEndCup =0;
	Ed edChallStage =0, edChallEnd =0;  Img imgChallStage =0;
	Img imgChallFail =0, imgChallCup =0;
	Txt txChallEndC =0, txChallEndF =0, txChampEndF =0;
	int iChSnd = 0;  // snd id to play

	//  main
	void fillChampsList(std::vector<int> vIds), fillChallsList(std::vector<int> vIds);  // Ids from champs/challs .all
	void ChampsListUpdate(), ChampFillStageInfo(bool finished), ChampionshipAdvance(float timeCur);
	void ChallsListUpdate(), ChallFillStageInfo(bool finished), ChallengeAdvance(float timeCur);
	void btnChampInfo(WP), UpdChampTabVis();
	CK(ChampRev);  CK(Ch_All);

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
	Tab tabsNet =0;
	WP  panNetServer =0,panNetServer2 =0, panNetGame =0, panNetTrack =0;
	Mli listServers =0, listPlayers =0;
	int iColLock =0, iColHost =0, iColPort =0;  // ids of columns in listServers

	//  upd gui triggers
	bool bRebuildPlayerList =0, bRebuildGameList =0;
	bool bUpdateGameInfo =0, bUpdChat =0;
	bool bStartGame =0, bStartedGame =0;
	void UpdGuiNetw();

	//  chat,msg  ----
	Ed edNetChat =0;  // chat area, set text through sChatBuffer
	MyGUI::UString sChatBuffer,sChatLast1,sChatLast2;  int iChatMove = 0;
	void AddChatMsg(const MyGUI::UString& clr, const MyGUI::UString& msg, bool add=true);

	Ed edNetChatMsg =0;
	Btn btnNetSendMsg =0;  void chatSendMsg();
	GuiPopup* popup =0;  // msg with edits

	//  Net gui
	Btn btnNetRefresh =0, btnNetJoin =0;    void evBtnNetRefresh(WP), evBtnNetJoin(WP),   evBtnNetJoinLockedClose();
	Btn btnNetCreate =0,  btnNetDirect =0;  void evBtnNetCreate(WP),  evBtnNetDirect(WP), evBtnNetDirectClose();
	Btn btnNetReady =0,   btnNetLeave =0;   void evBtnNetReady(WP),   evBtnNetLeave(WP);
	void btnNetEndClose(WP);

	Txt valNetGameInfo =0, valNetPassword =0;
	Ed edNetGameName =0,   edNetPassword =0;   void evEdNetGameName(Ed),   evEdNetPassword(Ed);
	Ed edNetNick =0,       edNetLocalPort =0;  void evEdNetNick(Ed),       evEdNetLocalPort(Ed);
	Ed edNetServerPort =0, edNetServerIP =0;   void evEdNetServerPort(Ed), evEdNetServerIP(Ed);

	//  open urls
	void btnWelcome(WP), btnWebsite(WP), btnWiki(WP), btnWikiInput(WP);
	void btnForum(WP), btnSources(WP), btnEdTut(WP), btnTransl(WP), btnDonations(WP);
};
