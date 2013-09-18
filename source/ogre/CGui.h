#pragma once
#include "BaseApp.h"
#include "common/Defines.h"
#include "common/Gui_Def.h"
#include "CGame.h"  //xml..
#include "../vdrift/settings.h"
#include "common/Slider.h"
#include "common/SliderValue.h"
#include "common/Gui_Popup.h"
#include <MyGUI.h>
//#include "common/MessageBox/MessageBox.h"
#include "common/MessageBox/MessageBoxStyle.h"
#include "../network/networkcallbacks.hpp"
#include "../oics/ICSInputControlSystem.h"
#include "ChampsXml.h"
#include "ChallengesXml.h"
#include "CInput.h"

namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;  class Viewport;  class Light;  }
namespace MyGUI  {  class MultiList2;  class Slider;  class Message;  }
class Scene;
class SplineRoad;
class GAME;
class CHud;


class CGui : public GameClientCallback, public MasterClientCallback,
			 public ICS::DetectingBindingListener
{
public:
	App* app;
	GAME* pGame;
	SETTINGS* pSet;
	Scene* sc;
	CHud* hud;
	
	CGui(App* ap1);
	

	///-----------------------------------------------------------------------------------------------------------------
	///  Gui
	///-----------------------------------------------------------------------------------------------------------------
	//  size
	void SizeGUI(); void doSizeGUI(MyGUI::EnumeratorWidgetPtr);
	std::vector<MyGUI::TabControl*> vSubTabsGame, vSubTabsOpts;


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
	WP mToolTip;  MyGUI::EditPtr mToolTipTxt;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(MyGUI::Widget* sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(MyGUI::Widget *moving, const MyGUI::IntPoint & point);

	//  language
	void comboLanguage(CMB);
	std::map<std::string, MyGUI::UString> languages; // <short name, display name>
	bool bGuiReinit;  void UnfocusLists();
	MyGUI::ButtonPtr bnQuit;  void btnQuit(WP);

	//  init
	void InitGui();  bool bGI;
	void GuiUpdate();
	
	void GuiCenterMouse(),GuiInitTooltip(),GuiInitLang(), GuiInitGraphics(),GuiInitTrack();
	Ogre::String GetSceneryColor(Ogre::String name);
	void AddTrkL(std::string name, int user, const class TrackInfo* ti);

	///  track
	void UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const Ogre::String& sTrack, float timeCur, bool champ=false),
		ReadTrkStats(), ReadTrkStatsChamp(Ogre::String track,bool reverse);
	MyGUI::MultiList2* trkList;  MyGUI::EditPtr trkDesc[2];
	MyGUI::StaticImagePtr imgPrv[2],imgMini[2],imgTer[2], imgTrkIco1,imgTrkIco2;
	const static int StTrk = 12, InfTrk = 11;
	MyGUI::StaticTextPtr valTrkNet, stTrk[2][StTrk], infTrk[2][InfTrk];  // [2] 2nd set is for champs

	void listTrackChng(MyGUI::MultiList2* li, size_t pos), TrackListUpd(bool resetNotFound=false);
	TracksXml tracksXml;  CarsXml carsXml;  UserXml userXml;  //xml
	void btnTrkView1(WP),btnTrkView2(WP),ChangeTrackView();
	void updTrkListDim(), updChampListDim();
	//  const list column widths
	const static int colTrk[32],colCar[16],colCh[16],colChL[16],colSt[16];
	const static Ogre::String clrsDiff[9],clrsRating[6],clrsLong[10];

	void edTrkFind(MyGUI::EditPtr),edRplFind(MyGUI::EditPtr);
	Ogre::String sTrkFind,sRplFind;  MyGUI::EditPtr edFind;
	strlist liTracks,liTracksUser;  void FillTrackLists();
	std::list<TrkL> liTrk;

	void CarListUpd(bool resetNotFound=false);
	void AddCarL(std::string name, const class CarInfo* ci);
	std::list<CarL> liCar;  void FillCarList();

	//  screen
	MyGUI::ListPtr resList;
	void InitGuiScreenRes(), btnResChng(WP), ResizeOptWnd();
	void chkVidFullscr(WP), chkVidVSync(WP);

	void comboGraphicsAll(CMB), comboRenderSystem(CMB);
		
	///-----------------------------------------


	//  main menu
	void toggleGui(bool toggle=true), GuiShortcut(MNU_Btns mnu, int tab, int subtab=-1);
	void MainMenuBtn(MyGUI::WidgetPtr), MenuTabChg(MyGUI::TabPtr, size_t);  bool loadReadme;

	void UpdCarClrSld(bool upd=true), UpdCarMClr();  bool bUpdCarClr;
	void btnNetEndClose(WP);


	///  championships & challenges
	MyGUI::ButtonPtr btStTut, btStChamp, btStChall;
	MyGUI::StaticImagePtr imgTut, imgChamp, imgChall;
	//  tabs
	MyGUI::TabPtr tabTut, tabChamp, tabChall;
	void tabTutType(MyGUI::TabPtr wp, size_t id), tabChampType(MyGUI::TabPtr wp, size_t id);
	void tabChallType(MyGUI::TabPtr wp, size_t id);

	//  stages
	MyGUI::EditBox* edChInfo,*edChDesc;  MyGUI::Widget* panCh;
	MyGUI::TextBox* txtCh,*valCh,*txtChP[3],*valChP[3];  // stages info, pass/progress
	void btnStageNext(WP), btnStagePrev(WP);  MyGUI::StaticText* valStageNum;
	void StageListAdd(int n, Ogre::String name, int laps, Ogre::String progress);
	
	//  xml  [1]= reversed  L= challenge
	ChampsXml champs;  ProgressXml progress[2];
	ChallXml chall;  ProgressLXml progressL[2];
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
	MyGUI::MultiList2* liStages, *liNetEnd;  void listStageChng(MyGUI::MultiList2* li, size_t pos);
	MyGUI::MultiList2* liChamps;  void listChampChng(MyGUI::MultiList2* li, size_t pos);
	MyGUI::MultiList2* liChalls;  void listChallChng(MyGUI::MultiList2* li, size_t pos);

	void btnChampStart(WP), btnChampEndClose(WP), btnChampStageBack(WP), btnChampStageStart(WP);
	void btnChallStart(WP), btnChallEndClose(WP), btnChallStageBack(WP), btnChallStageStart(WP);
	void btnChRestart(WP);  MyGUI::ButtonPtr btChRestart;

	MyGUI::ButtonPtr btChampStage, btChallStage;
	MyGUI::EditBox* edChampStage,*edChampEnd;  MyGUI::ImageBox* imgChampStage,*imgChampEndCup;
	MyGUI::EditBox* edChallStage,*edChallEnd;  MyGUI::ImageBox* imgChallStage;
	MyGUI::ImageBox* imgChallFail,*imgChallCup;  MyGUI::TextBox* txChallEndC,*txChallEndF;

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
	void UpdateInputButton(MyGUI::Button* button, const InputAction& action, EBind bind = B_Done);

	InputAction* mBindingAction;
	MyGUI::Button* mBindingSender;
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
	MyGUI::TabPtr tabInput;  void tabInputChg(MyGUI::TabPtr, size_t);
	MyGUI::StaticTextPtr txtInpDetail;  MyGUI::WidgetPtr panInputDetail;  MyGUI::ButtonPtr chOneAxis;
	MyGUI::EditPtr edInputIncrease;  void editInput(MyGUI::EditPtr), btnInputInv(WP), chkOneAxis(WP);
	void comboInputKeyAllPreset(CMB);  bool TabInputId(int* pId);


	///  tweak  -----------------------------------------
	const static int ciEdCar = 12;
	MyGUI::EditPtr edCar[ciEdCar],edPerfTest, edTweakCol;  MyGUI::TabPtr tabTweak, tabEdCar;
	MyGUI::StaticTextPtr txtTweakPath, txtTweakTire, txtTweakPathCol;
	MyGUI::ComboBoxPtr cmbTweakCarSet, cmbTweakTireSet;
	void CmbTweakCarSet(CMB), CmbTweakTireSet(CMB), CmbEdTweakCarSet(MyGUI::EditPtr), CmbEdTweakTireSet(MyGUI::EditPtr);
	void TweakToggle(), TweakCarSave(),TweakCarLoad(), TweakTireSave();
	void TweakColUpd(bool user), TweakColLoad(),TweakColSave();
	void btnTweakCarSave(WP),btnTweakCarLoad(WP), btnTweakTireSave(WP), btnTweakColSave(WP);
	void tabCarEdChng(MyGUI::TabPtr, size_t);
	//  graphs
	MyGUI::ComboBox* cmbGraphs;  void comboGraphs(CMB);  MyGUI::StaticTextPtr valGraphsType;


	//  sliders  -----------------------------------------
	SLV(Particles);  SLV(Trails);
	SLV(ReflSkip);  SLV(ReflSize);  SLV(ReflFaces);  SLV(ReflDist);  SLV(ReflMode); // refl
	SLV(SizeGaug);  SLV(TypeGaug);  SLV(LayoutGaug);
	SLV(SizeMinimap);  SLV(SizeArrow);  SLV(ZoomMinimap);
	SLV(CountdownTime);  // view
	SLV(DbgTxtClr);  SLV(DbgTxtCnt);
	SLV(VolMaster);  SLV(VolEngine);  SLV(VolTires);  SLV(VolSusp);  SLV(VolEnv);  // sounds
	SLV(VolFlSplash);  SLV(VolFlCont);  SLV(VolCarCrash);  SLV(VolCarScrap);
	
	SLV(CarClrH);  SLV(CarClrS);  SLV(CarClrV);  SLV(CarClrGloss);  SLV(CarClrRefl);  // car clr
	SLV(BloomInt);  SLV(BloomOrig);  SLV(BlurIntens);  // video
	SLV(DepthOfFieldFocus);  SLV(DepthOfFieldFar);  // dof
	SLV(HDRParam1);  SLV(HDRParam2);  SLV(HDRParam3);  // hdr
	SLV(HDRBloomInt);  SLV(HDRBloomOrig);  SLV(HDRAdaptationScale);
	SLV(HDRVignettingRadius);  SLV(HDRVignettingDarkness);
	SLV(NumLaps);  SLV(RplNumViewports);  // setup
	SLV(SSSEffect);  SLV(SSSVelFactor);  SLV(SteerRangeSurf);  SLV(SteerRangeSim);
	
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
	void tabTireSet(MyGUI::TabPtr wp, size_t id);
	MyGUI::ButtonPtr bchAbs,bchTcs;  MyGUI::Slider* slSSSEff,*slSSSVel,*slSteerRngSurf,*slSteerRngSim;

	void imgBtnCarClr(WP), btnCarClrRandom(WP), toggleWireframe();
	MyGUI::ButtonPtr bRkmh, bRmph;  void radKmh(WP), radMph(WP);
	MyGUI::ButtonPtr bRsimEasy, bRsimNorm;  void radSimEasy(WP), radSimNorm(WP);  bool bReloadSim;
	MyGUI::ButtonPtr chFps,chWire, chBlt,chBltTxt, chProfTxt,
		chDbgT,chDbgB,chDbgS, chGraphs, chTireVis,
		chTimes,chMinimp,chOpponents;

	///  replay  -----------------------------
	MyGUI::StaticTextPtr valRplPerc, valRplCur, valRplLen,
		valRplName,valRplInfo,valRplName2,valRplInfo2;
	MyGUI::Slider* slRplPos;  void slRplPosEv(SL);
	MyGUI::EditPtr edRplName, edRplDesc;
	void btnRplLoad(WP), btnRplSave(WP), btnRplDelete(WP), btnRplRename(WP),  // btn
		chkRplAutoRec(WP), chkRplChkGhost(WP), chkRplChkBestOnly(WP), chkRplChkPar(WP),
		chkRplChkRewind(WP), chkRplChkGhostOther(WP), chkRplChkTrackGhost(WP),  // replay
		btnRplToStart(WP),btnRplToEnd(WP), btnRplPlay(WP),  // controls
		btnRplCur(WP),btnRplAll(WP),chkRplGhosts(WP);  // radio
	MyGUI::ButtonPtr btRplPl;  void UpdRplPlayBtn();
	MyGUI::ButtonPtr rbRplCur, rbRplAll;  // radio

	void btnRplBackDn(WP,int,int,MyGUI::MouseButton),btnRplBackUp(WP,int,int,MyGUI::MouseButton);
	void btnRplFwdDn(WP,int,int,MyGUI::MouseButton),btnRplFwdUp(WP,int,int,MyGUI::MouseButton);
	bool bRplBack,bRplFwd;
		
	void msgRplDelete(MyGUI::Message*, MyGUI::MessageBoxStyle);
	
	void btnNumPlayers(WP);  void chkSplitVert(WP), chkStartOrd(WP);
	MyGUI::StaticTextPtr valLocPlayers;
	
	MyGUI::StaticTextPtr txCarStatsTxt,txCarStatsVals,
		txCarSpeed,txCarType, txCarAuthor,txTrackAuthor;
	void UpdCarStatsTxt();  // car stats

	///---------------------------------------

	//  game
	void btnNewGame(WP),btnNewGameStart(WP);
	MyGUI::MultiList2* carList;  MyGUI::ListPtr rplList;  void updReplaysList();
	void listRplChng(MyGUI::List* li, size_t pos),  changeCar();
	void listCarChng(MyGUI::MultiList2* li, size_t pos),  changeTrack();
	int LNext(MyGUI::MultiList2* lp, int rel, int ofs), LNext(MyGUI::ListPtr lp, int rel, int ofs),
		LNext(MyGUI::MultiList* lp, int rel);  // util next in list
	void LNext(int rel);  void tabPlayer(MyGUI::TabPtr wp, size_t id);

	int iCurCar;
	Ogre::String sListCar,sListTrack;  int bListTrackU;
	Ogre::String pathTrk[2];  Ogre::String TrkDir();
	Ogre::String PathListTrk(int user=-1), PathListTrkPrv(int user/*=-1*/, Ogre::String track);

	const Ogre::String& GetGhostFile(std::string* ghCar=NULL);
	std::string GetRplListDir();

	MyGUI::StaticImagePtr imgCar;  MyGUI::EditPtr carDesc;
	MyGUI::ComboBoxPtr cmbBoost, cmbFlip, cmbDamage, cmbRewind;
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
	MyGUI::UString sChatBuffer,sChatLast1,sChatLast2;  int iChatMove;
	void AddChatMsg(const MyGUI::UString& clr, const MyGUI::UString& msg, bool add=true);
	protocol::GameInfo netGameInfo;

	bool bRebuildPlayerList, bRebuildGameList;
	bool bUpdateGameInfo, bStartGame, bStartedGame, bUpdChat;

	///  multiplayer gui  --------------------
	MyGUI::TabPtr tabsNet;  //void tabNet(TabPtr tab, size_t id);
	MyGUI::WidgetPtr panelNetServer,panelNetGame,panelNetTrack;
	MyGUI::MultiListPtr listServers, listPlayers;
	MyGUI::EditPtr edNetChat;  // chat area, set text through sChatBuffer
	int iColLock,iColHost,iColPort;  //ids of columns in listServers, set in gui init

	MyGUI::ButtonPtr btnNetRefresh,btnNetJoin,btnNetCreate,btnNetDirect;
	MyGUI::ButtonPtr btnNetReady,btnNetLeave;
	void evBtnNetRefresh(WP);
	void evBtnNetJoin(WP), evBtnNetJoinLockedClose();
	void evBtnNetCreate(WP);
	void evBtnNetDirect(WP),evBtnNetDirectClose();
	void evBtnNetReady(WP),evBtnNetLeave(WP);

	MyGUI::StaticTextPtr valNetGameName, valNetChat, valNetGameInfo, valNetPassword;
	MyGUI::ButtonPtr btnNetSendMsg;  void chatSendMsg();
	MyGUI::EditPtr edNetGameName, edNetChatMsg, edNetPassword,
		edNetNick, edNetServerIP, edNetServerPort, edNetLocalPort;
	void evEdNetGameName(MyGUI::EditPtr), evEdNetPassword(MyGUI::EditPtr),
		evEdNetNick(MyGUI::EditPtr), evEdNetServerIP(MyGUI::EditPtr),
		evEdNetServerPort(MyGUI::EditPtr), evEdNetLocalPort(MyGUI::EditPtr);
	void UpdGuiNetw();

	bool GetCarPath(std::string* pathCar/*out*/,
		std::string* pathSave/*=0*/, std::string* pathSaveDir/*=0*/,
		std::string carname, /*std::string tweakSetup="",*/ bool forceOrig=false);
};
