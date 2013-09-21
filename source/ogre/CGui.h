#pragma once
#include "BaseApp.h"
#include "common/Gui_Def.h"
//#include "common/Slider.h"
#include "common/SliderValue.h"
#include "common/Gui_Popup.h" //-

#include <MyGUI_Enumerator.h>
#include "common/MessageBox/MessageBoxStyle.h"

#include "../network/networkcallbacks.hpp"
#include "../oics/ICSInputControlSystem.h"

#include "ChampsXml.h"  // progress..
#include "ChallengesXml.h"
#include "CInput.h"

namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;  class Viewport;  class Light;  }
namespace MyGUI  {  class MultiList2;  class Slider;  class Message;  }
class Scene;  class SplineRoad;  class GAME;  class CHud;  class CData;


//  tracks,cars list items - with info for sorting
struct TrkL
{
	std::string name;
	const class TrackInfo* ti;
	int test;  //Test*
};
struct CarL
{
	std::string name;
	const class CarInfo* ci;
};


class CGui : public BGui,
			 public GameClientCallback, public MasterClientCallback,
			 public ICS::DetectingBindingListener
{
public:
	App* app;
	GAME* pGame;
	SETTINGS* pSet;
	Scene* sc;
	CHud* hud;
	CData* data;
	MyGUI::Gui* mGui;
	
	CGui(App* ap1);
	
	typedef std::list <std::string> strlist;

	///-----------------------------------------------------------------------------------------------------------------
	///  Gui
	///-----------------------------------------------------------------------------------------------------------------
	//  size
	void SizeGUI(); void doSizeGUI(MyGUI::EnumeratorWidgetPtr);
	std::vector<Tab> vSubTabsGame, vSubTabsOpts;


	///  Gui common   --------------------------
	//  graphics
	SlV(ViewDist);  SlV(Anisotropy);
	SlV(TerDetail);  SlV(TerDist);  SV svRoadDist;
	SlV(TexSize);  SlV(TerMtr);  SlV(TerTripl);  // detail
	SlV(Trees);  SlV(Grass);  SlV(TreesDist);  SlV(GrassDist);  // paged
	SlV(ShadowType);  SlV(ShadowCount);  SlV(ShadowSize);  SlV(ShadowDist);  // shadow
	SlV(WaterSize);  // screen
	void comboTexFilter(CMB), btnShadows(WP), btnShaders(WP), btnTrGrReset(WP),
		chkWaterReflect(WP), chkWaterRefract(WP),
		chkUseImposters(WP), chkImpostorsOnly(WP), cmbAntiAliasing(CMB);
	void setOrigPos(WP wp, const char* relToWnd);

	//  tooltip
	WP mToolTip;  Ed mToolTipTxt;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(WP sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(WP moving, const MyGUI::IntPoint& point);

	//  language
	void comboLanguage(CMB);
	std::map<std::string, MyGUI::UString> languages; // <short name, display name>
	bool bGuiReinit;  void UnfocusLists();
	Btn bnQuit;  void btnQuit(WP);

	//  init
	bool bGI;
	void GuiCenterMouse(),GuiInitTooltip(),GuiInitLang(), GuiInitGraphics(),GuiInitTrack();
	Ogre::String GetSceneryColor(Ogre::String name);
	void AddTrkL(std::string name, int user, const class TrackInfo* ti);

	///  track
	void UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const Ogre::String& sTrack, float timeCur, bool champ=false),
		ReadTrkStats();
	Mli2 trkList;  Ed trkDesc[2];
	Img imgPrv[2],imgMini[2],imgTer[2], imgTrkIco1,imgTrkIco2;
	const static int StTrk = 6, InfTrk = 11;
	Txt valTrkNet, stTrk[2][StTrk], infTrk[2][InfTrk];  // [2] 2nd set is for champs

	void listTrackChng(Mli2, size_t), TrackListUpd(bool resetNotFound=false);
	void btnTrkView1(WP),btnTrkView2(WP), ChangeTrackView();
	void updTrkListDim();
	//  const list column widths
	const static int colTrk[32];
	const static Ogre::String clrsDiff[9], clrsRating[6], clrsLong[10];

	void edTrkFind(Ed); 
	Ogre::String sTrkFind;  Ed edFind;
	strlist liTracks,liTracksUser;  void FillTrackLists();
	std::list<TrkL> liTrk;

	//  screen
	Li resList;
	void InitGuiScreenRes(), btnResChng(WP), ResizeOptWnd();
	void chkVidFullscr(WP), chkVidVSync(WP);

	void comboGraphicsAll(CMB), comboRenderSystem(CMB);
		
	///-----------------------------------------
	
	void InitGui(), GuiUpdate();

	void CarListUpd(bool resetNotFound=false);
	void AddCarL(std::string name, const class CarInfo* ci);
	std::list<CarL> liCar;  void FillCarList();

	void ReadTrkStatsChamp(Ogre::String track,bool reverse);
	void updChampListDim();

	void edRplFind(Ed);  Ogre::String sRplFind;
	
	const static int colCar[16],colCh[16],colChL[16],colSt[16];


	//  main menu
	void toggleGui(bool toggle=true), GuiShortcut(MNU_Btns mnu, int tab, int subtab=-1);
	void MainMenuBtn(WP), MenuTabChg(Tab, size_t);  bool loadReadme;

	void UpdCarClrSld(bool upd=true), UpdCarMClr();  bool bUpdCarClr;
	void btnNetEndClose(WP);


	///  championships & challenges
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
	friend class CarModel;
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
	Img imgChallFail, imgChallCup;  Txt txChallEndC, txChallEndF;

	//  main
	void ChampsListUpdate(), ChampFillStageInfo(bool finished), ChampionshipAdvance(float timeCur);
	void ChallsListUpdate(), ChallFillStageInfo(bool finished), ChallengeAdvance(float timeCur);
	void btnChampInfo(WP), chkChampRev(WP), UpdChampTabVis();
	void ToolGhosts(),ToolGhostsConv(),ToolPresets();  //  _Tools_

	//  chall util
	Ogre::String StrChallCars(const Chall& ch);
	bool IsChallCar(Ogre::String name);
	bool isChallGui();  void BackFromChs();


	///  input tab
	///-----------------------------------------------------------------------------------------------------------------

	void CreateInputTab(const std::string& title, bool playerTab, const std::vector<InputAction>& actions, ICS::InputControlSystem* ICS);
	void InitInputGui(), inputBindBtnClicked(WP), inputUnbind(WP), inputBindBtn2(WP, int, int, MyGUI::MouseButton mb);
	enum EBind {  B_Done=0, B_First, B_Second  };
	void UpdateInputButton(Btn button, const InputAction& action, EBind bind = B_Done);

	InputAction* mBindingAction;
	Btn mBindingSender;
	virtual void mouseAxisBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		ICS::InputControlSystem::NamedAxis axis, ICS::Control::ControlChangingDirection direction);
	virtual void keyBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		SDL_Keycode key, ICS::Control::ControlChangingDirection direction);
	virtual void mouseButtonBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		unsigned int button, ICS::Control::ControlChangingDirection direction);
	virtual void joystickAxisBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		int deviceId, int axis, ICS::Control::ControlChangingDirection direction);
	virtual void joystickButtonBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		int deviceId, unsigned int button, ICS::Control::ControlChangingDirection direction);
	virtual void joystickPOVBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		int deviceId, int pov, ICS::InputControlSystem::POVAxis axis, ICS::Control::ControlChangingDirection direction);

	virtual void notifyInputActionBound(bool complete);

	bool actionIsActive(std::string, std::string);
	void UpdateInputBars(), inputDetailBtn(WP);

	//  joy events
	Tab tabInput;  void tabInputChg(Tab, size_t);
	Txt txtInpDetail;  WP panInputDetail;  Btn chOneAxis;
	Ed edInputIncrease;  void editInput(Ed), btnInputInv(WP), chkOneAxis(WP);
	void comboInputKeyAllPreset(CMB);  bool TabInputId(int* pId);


	///  tweak  -----------------------------------------
	const static int ciEdCar = 12;
	Ed edCar[ciEdCar],edPerfTest, edTweakCol;  Tab tabTweak, tabEdCar;
	Txt txtTweakPath, txtTweakTire, txtTweakPathCol;
	Cmb cmbTweakCarSet, cmbTweakTireSet;
	void CmbTweakCarSet(CMB), CmbTweakTireSet(CMB), CmbEdTweakCarSet(Ed), CmbEdTweakTireSet(Ed);
	void TweakToggle(), TweakCarSave(),TweakCarLoad(), TweakTireSave();
	void TweakColUpd(bool user), TweakColLoad(),TweakColSave();
	void btnTweakCarSave(WP),btnTweakCarLoad(WP), btnTweakTireSave(WP), btnTweakColSave(WP);
	void tabCarEdChng(Tab, size_t);
	//  graphs
	Cmb cmbGraphs;  void comboGraphs(CMB);  Txt valGraphsType;


	//  sliders  -----------------------------------------
	//  reflection
	SV svParticles, svTrails;
	SV svReflSkip, svReflFaces, svReflSize;
	SlV(ReflDist);  SlV(ReflMode);
	//  hud view
	SV svSizeGaug;
	SV svTypeGaug, svLayoutGaug;
	SV svSizeMinimap, svZoomMinimap;
	void slHudSize(SV*), slHudCreate(SV*);
	SlV(SizeArrow);
	SLV(CountdownTime);//-
	SV svDbgTxtClr, svDbgTxtCnt;
	//  sound
	SlV(VolMaster);
	SV svVolEngine, svVolTires, svVolSusp, svVolEnv;
	SV svVolFlSplash, svVolFlCont, svVolCarCrash, svVolCarScrap;
	
	//  car clr
	SLV(CarClrH);  SLV(CarClrS);  SLV(CarClrV);
	SLV(CarClrGloss);  SLV(CarClrRefl);
	//  video effects
	SV svBloomInt, svBloomOrig;
	SV svBlurIntens;  // motion blur
	SV svDofFocus, svDofFar;  // DepthOfField
	void slBloom(SV*);
	//  hdr
	SV svHDRParam1, svHDRParam2, svHDRParam3;
	SV svHDRBloomInt, svHDRBloomOrig, svHDRAdaptScale;
	SV svHDRVignRadius, svHDRVignDark;
	//  setup
	SV svNumLaps;  SLV(RplNumViewports);
	SLV(SSSEffect);  SLV(SSSVelFactor);
	SLV(SteerRangeSurf);  SLV(SteerRangeSim);
	
	//  checks
	void chkGauges(WP),	chkArrow(WP),chkBeam(WP), chkDigits(WP),
		chkMinimap(WP), chkMiniZoom(WP), chkMiniRot(WP), chkMiniTer(WP), chkMiniBorder(WP),  // view
		chkFps(WP), chkWireframe(WP), 
		chkCamInfo(WP), chkTimes(WP), chkOpponents(WP), chkOpponentsSort(WP), chkCamTilt(WP),
		chkCarDbgBars(WP), chkCarDbgTxt(WP), chkCarDbgSurf(WP), chkGraphs(WP), chkCarTireVis(WP), 
		chkBltDebug(WP), chkBltProfilerTxt(WP), chkProfilerTxt(WP),
		chkReverse(WP), chkParticles(WP), chkTrails(WP),
		chkAbs(WP), chkTcs(WP), chkGear(WP), chkRear(WP), chkRearInv(WP),  // car
		chkOgreDialog(WP), chkAutoStart(WP), chkEscQuits(WP),
		chkStartInMain(WP), chkBltLines(WP), chkLoadPics(WP), chkMultiThread(WP),  // startup
		chkVidEffects(WP), chkVidBloom(WP), chkVidHDR(WP), chkVidBlur(WP), UpdBloomVals(), chkVidSSAO(WP), // effects
		chkVidSoftParticles(WP), chkVidGodRays(WP), chkVidDepthOfField(WP), chkVidBoostFOV(WP),
		chkVegetCollis(WP), chkCarCollis(WP), chkRoadWCollis(WP), chkDynObjects(WP);  //game

	// gui car tire set gravel/asphalt
	int iTireSet;
	void tabTireSet(Tab wp, size_t);
	Btn bchAbs,bchTcs;
	Sl slSSSEff, slSSSVel, slSteerRngSurf, slSteerRngSim;

	void imgBtnCarClr(WP), btnCarClrRandom(WP), toggleWireframe();
	Btn bRkmh, bRmph;  void radKmh(WP), radMph(WP);
	Btn bRsimEasy, bRsimNorm;  void radSimEasy(WP), radSimNorm(WP);  bool bReloadSim;
	Btn chFps,chWire, chBlt,chBltTxt, chProfTxt,
		chDbgT,chDbgB,chDbgS, chGraphs, chTireVis,
		chTimes,chMinimp,chOpponents;

	///  replay  -----------------------------
	Txt valRplPerc, valRplCur, valRplLen,
		valRplName,valRplInfo,valRplName2,valRplInfo2;
	Sl slRplPos;  void slRplPosEv(SL);
	Ed edRplName, edRplDesc;
	void btnRplLoad(WP), btnRplSave(WP), btnRplDelete(WP), btnRplRename(WP),  // btn
		chkRplAutoRec(WP), chkRplChkGhost(WP), chkRplChkBestOnly(WP), chkRplChkPar(WP),
		chkRplChkRewind(WP), chkRplChkGhostOther(WP), chkRplChkTrackGhost(WP),  // replay
		btnRplToStart(WP),btnRplToEnd(WP), btnRplPlay(WP),  // controls
		btnRplCur(WP),btnRplAll(WP),chkRplGhosts(WP);  // radio
	Btn btRplPl;  void UpdRplPlayBtn();
	Btn rbRplCur, rbRplAll;  // radio

	void btnRplBackDn(WP,int,int,MyGUI::MouseButton), btnRplBackUp(WP,int,int,MyGUI::MouseButton);
	void btnRplFwdDn(WP,int,int, MyGUI::MouseButton), btnRplFwdUp(WP,int,int, MyGUI::MouseButton);
	bool bRplBack,bRplFwd;
		
	void msgRplDelete(MyGUI::Message*, MyGUI::MessageBoxStyle);
	
	void btnNumPlayers(WP);  void chkSplitVert(WP), chkStartOrd(WP);
	Txt valLocPlayers;
	
	Txt txCarStatsTxt,txCarStatsVals,
		txCarSpeed,txCarType, txCarAuthor,txTrackAuthor;
	void UpdCarStatsTxt();  // car stats

	///---------------------------------------

	//  game
	void btnNewGame(WP),btnNewGameStart(WP);
	Mli2 carList;  Li rplList;  void updReplaysList();
	void listRplChng(Li, size_t),  changeCar();
	void listCarChng(Mli2, size_t),  changeTrack();
	int LNext(Mli2, int rel, int ofs), LNext(Li, int rel, int ofs),
		LNext(Mli, int rel);  // util next in list
	void LNext(int rel);  void tabPlayer(Tab wp, size_t id);

	int iCurCar;
	Ogre::String sListCar,sListTrack;  int bListTrackU;
	Ogre::String pathTrk[2];  Ogre::String TrkDir();
	Ogre::String PathListTrk(int user=-1), PathListTrkPrv(int user/*=-1*/, Ogre::String track);

	const Ogre::String& GetGhostFile(std::string* ghCar=NULL);
	std::string GetRplListDir();

	Img imgCar;  Ed carDesc;
	Cmb cmbBoost, cmbFlip, cmbDamage, cmbRewind;
	void comboBoost(CMB), comboFlip(CMB), comboDamage(CMB), comboRewind(CMB);

	GuiPopup popup;

	///  multiplayer  ---------------------------------------
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
	//  chat,msg
	MyGUI::UString sChatBuffer,sChatLast1,sChatLast2;  int iChatMove;
	void AddChatMsg(const MyGUI::UString& clr, const MyGUI::UString& msg, bool add=true);

	bool bRebuildPlayerList, bRebuildGameList;
	bool bUpdateGameInfo, bStartGame, bStartedGame, bUpdChat;

	///  multiplayer gui  --------------------
	Tab tabsNet;  //void tabNet(TabPtr tab, size_t id);
	WP panelNetServer, panelNetGame, panelNetTrack;
	Mli listServers, listPlayers;
	Ed edNetChat;  // chat area, set text through sChatBuffer
	int iColLock, iColHost, iColPort;  // ids of columns in listServers, set in gui init

	Btn btnNetRefresh, btnNetJoin, btnNetCreate, btnNetDirect;
	Btn btnNetReady, btnNetLeave;
	void evBtnNetRefresh(WP);
	void evBtnNetJoin(WP), evBtnNetJoinLockedClose();
	void evBtnNetCreate(WP);
	void evBtnNetDirect(WP), evBtnNetDirectClose();
	void evBtnNetReady(WP), evBtnNetLeave(WP);

	Txt valNetGameName, valNetChat, valNetGameInfo, valNetPassword;
	Btn btnNetSendMsg;  void chatSendMsg();
	Ed edNetGameName, edNetChatMsg, edNetPassword,
		edNetNick, edNetServerIP, edNetServerPort, edNetLocalPort;
	void evEdNetGameName(Ed), evEdNetPassword(Ed),
		evEdNetNick(Ed), evEdNetServerIP(Ed),
		evEdNetServerPort(Ed), evEdNetLocalPort(Ed);
	void UpdGuiNetw();

	bool GetCarPath(std::string* pathCar/*out*/,
		std::string* pathSave/*=0*/, std::string* pathSaveDir/*=0*/,
		std::string carname, /*std::string tweakSetup="",*/ bool forceOrig=false);
};
