#ifndef _OgreGame_h_
#define _OgreGame_h_

#include "BaseApp.h"
#include "common/Gui_Popup.h"
#include "common/SceneXml.h"
#include "common/BltObjects.h"
#include "common/TracksXml.h"
#include "common/FluidsXml.h"
#include "ChampsXml.h"

#include "ReplayGame.h"
#include "CarModel.h"
#include "CarReflection.h"

#include "common/MessageBox/MessageBox.h"
#include "common/MessageBox/MessageBoxStyle.h"

#include "../network/networkcallbacks.hpp"
#include <boost/thread.hpp>
#include <MyGUI.h>
#include <OgreShadowCameraSetup.h>

namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;  class Viewport;  class Light;
	class Terrain;  class TerrainGlobalOptions;  class TerrainGroup;  class TerrainPaging;  class PageManager;  }
namespace Forests {  class PagedGeometry;  }
namespace BtOgre  {  class DebugDrawer;  }
namespace MyGUI  {  class MultiList2;  }
namespace OISB   {  class AnalogAxisAction;  }
class MaterialFactory;

class App : public BaseApp, public GameClientCallback, public MasterClientCallback //, public RenderTargetListener
{
public:
	App();  virtual ~App();
	
	class GAME* pGame;  ///*
	void updatePoses(float time), newPoses();
	void UpdThr();
	
	// stuff to be executed after BaseApp init
	void postInit();
	
	void setTranslations();
	
	// This list holds new positions info for every CarModel
	std::vector<PosInfo> newPosInfos;
	std::map<int,int> carsCamNum;  // picked camera number for cars
	
	// Utility
	Ogre::Quaternion qFixCar,qFixWh;

	//  replay - full, user saves
	//  ghost - saved when best lap,  ghplay - ghost ride replay, loaded if was on disk
	Replay replay, ghost, ghplay;  ReplayFrame fr;
	const Ogre::String& GetGhostFile();

	Scene sc;  /// scene.xml
	FluidsXml fluidsXml;  /// fluid params xml
	BltObjects objs;  // veget collision in bullet
	Ogre::Light* sun;  void UpdFog(bool bForce=false), UpdSun();
	
	// Rain, snow
	Ogre::ParticleSystem *pr,*pr2;
	
	//  trees
	Forests::PagedGeometry *trees, *grass;
	
	Ogre::SceneNode* arrowNode; // checkpoint arrow
	Ogre::SceneNode* arrowRotNode; // seperate node for rotation
	Ogre::Quaternion arrowAnimStart, arrowAnimEnd, arrowAnimCur; // smooth animation
		
	void UpdateHUD(int carId, class CarModel* pCarM, class CAR* pCar,
		float time, Ogre::Viewport* vp=NULL), SizeHUD(bool full, Ogre::Viewport* vp=NULL, int carId=-1);
	void UpdHUDRot(int carId, CarModel* pCarM, float vel, float rpm, bool miniOnly=false);
	
	MaterialFactory* materialFactory; // material generation
	void recreateCarMtr();
	
	Ogre::SceneManager* sceneMgr() { return mSceneMgr; };

protected:
	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameStart(Ogre::Real time);
	virtual bool frameEnd(Ogre::Real time);
	virtual bool keyPressed( const OIS::KeyEvent &arg );
		
	BtOgre::DebugDrawer *dbgdraw;  /// blt dbg
	void bltDumpRecursive(class CProfileIterator* profileIterator, int spacing, std::stringstream& os);
	void bltDumpAll(std::stringstream& os);

	//  mtr reload
	enum eMaterials {
		Mtr_CarBody, Mtr_CarInterior, Mtr_CarGlass,
		Mtr_CarTireFront, Mtr_CarTireRear,
		Mtr_Road,  NumMaterials  };
	Ogre::String sMtr[NumMaterials];

	//  2D, hud  ----
	float asp,  xcRpm, ycRpm, xcVel, ycVel,
		fMiniX,fMiniY, scX,scY, ofsX,ofsY, minX,maxX, minY,maxY;  // minimap

	Ogre::SceneNode *nrpm, *nvel, *nrpmB, *nvelBk,*nvelBm;  // gauges
	Ogre::ManualObject* mrpm, *mvel, *miniC;
	Ogre::SceneNode *ndMap, *ndLine;  // vdr minimap-
	// car pos on minimap
	std::vector<Ogre::SceneNode*> vNdPos;
	std::vector<Ogre::ManualObject*> vMoPos;
	Ogre::ManualObject* Create2D(const Ogre::String& mat, Ogre::SceneManager* sceneMgr,
		Ogre::Real size, bool dyn = false, bool clr = false);

	Ogre::OverlayElement* hudGear,*hudVel,*hudBoost,*hudCountdown, *ovL[5],*ovR[5],*ovS[5],*ovU[5],
		*hudAbs,*hudTcs, *hudTimes, *hudWarnChk,*hudWonPlace, *hudOpp[5][3],*hudOppB;
	Ogre::Overlay* ovGear,*ovVel,*ovBoost,*ovCountdown, *ovAbsTcs,*ovCarDbg,*ovCarDbgTxt,
		*ovCam, *ovTimes, *ovWarnWin, *ovOpp;

	Ogre::String GetTimeString(float time) const;
	void CreateHUD(), ShowHUD(bool hideAll=false), UpdMiniTer();
	Ogre::Vector2 projectPoint(Ogre::Viewport* vp, const Ogre::Vector3& pos);

	//  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	Ogre::String resCar, resTrk, resDrv;
	void CreateCar(), CreateTrack(), CreateRacingLine(), CreateMinimap(), CreateRoadBezier();
	void CreateTerrain(bool bNewHmap=false, bool bTer=true), CreateBltTerrain();
	void GetTerAngles(int xb,int yb, int xe,int ye);
	void CreateTrees(), CreateRoad(), CreateProps(), CreateFluids();
	void CreateSkyDome(Ogre::String sMater, Ogre::Vector3 scale);
	void NewGame();  void NewGameDoLoad();  bool IsTerTrack();  bool newGameRpl;
	
	// Loading
	void LoadCleanUp(), LoadGame(), LoadScene(), LoadCar(), LoadTerrain(), LoadTrack(), LoadMisc();
	enum ELoadState { LS_CLEANUP=0, LS_GAME, LS_SCENE, LS_CAR, LS_TER, LS_TRACK, LS_MISC, LS_ALL };
	
	// id, display name, initialised in App()
	// e.g.: 0, Cleaning up or 3, Loading scene
	std::map<unsigned int, std::string> loadingStates;
	// 1 behind map ( map.end() ): loading finished
	std::map<unsigned int, std::string>::iterator currentLoadingState;


	///  terrain
public:
	Ogre::Terrain* terrain; 
protected:
	Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;  bool mPaging;
	Ogre::TerrainPaging* mTerrainPaging;  Ogre::PageManager* mPageManager;
	//Vector3 getNormalAtWorldPosition(Terrain* terrain, Real x, Real z, Real s);

	int iBlendMaps, blendMapSize;	//  mtr from ter  . . . 
	char* blendMtr;  // mtr [blendMapSize x blendMapSize]
	void initBlendMaps(Ogre::Terrain* terrain);  bool noBlendUpd;
	void configureTerrainDefaults(Ogre::Light* l);
	float Noise(float x, float zoom, int octaves, float persistance);
	float Noise(float x, float y, float zoom, int octaves, float persistance);
	Ogre::Real terMaxAng;

public:
	void changeShadows(), UpdPSSMMaterials(), setMtrSplits(Ogre::String sMtrName);
	Ogre::Vector4 splitPoints;

protected:
	Ogre::ShadowCameraSetupPtr mPSSMSetup;
	void recreateReflections();  // call after refl_mode changed

	//  road
public:	
	class SplineRoad* road;
protected:

	///-----------------------------------------------------------------------------------------------------------------
	///  Gui
	///-----------------------------------------------------------------------------------------------------------------
	//  size
	void SizeGUI(); void doSizeGUI(MyGUI::EnumeratorWidgetPtr);

	//  shortcuts
	typedef MyGUI::WidgetPtr WP;
	typedef std::list <std::string> strlist;
	//  slider event and its text field for value
	#define SLV(name)  void sl##name(SL);  MyGUI::StaticTextPtr val##name;
	#define SL  MyGUI::ScrollBar* wp, size_t val						//  slider event args
	#define CMB MyGUI::ComboBox* wp, size_t val // combobox event args

	///  Gui common   --------------------------
	//  graphics
	SLV(Anisotropy);  SLV(ViewDist);  SLV(TerDetail);  SLV(TerDist);  SLV(RoadDist);
	SLV(TexSize);  SLV(TerMtr);  // detail
	SLV(Trees);  SLV(Grass);  SLV(TreesDist);  SLV(GrassDist);  // paged
	SLV(Shaders);  SLV(ShadowType);  SLV(ShadowCount);  SLV(ShadowSize);  SLV(ShadowDist);  SLV(ShadowFilter); // shadow
	SLV(AntiAliasing); // screen
	void comboTexFilter(CMB), btnShadows(WP), btnShaders(WP), btnTrGrReset(WP);
	MyGUI::ButtonPtr bnQuit;  void btnQuit(WP);

	//  tooltip
	WP mToolTip;  MyGUI::EditPtr mToolTipTxt;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(MyGUI::Widget* sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(MyGUI::Widget *moving, const MyGUI::IntPoint & point);

	//  language
	void comboLanguage(CMB);
	std::map<std::string, std::string> languages; // <short name, display name>
	bool bGuiReinit;

	//  init
	void InitGui();  bool bGI;
	void GuiCenterMouse(),GuiInitTooltip(),GuiInitLang(), GuiInitGraphics(),GuiInitTrack();
	Ogre::String GetSceneryColor(Ogre::String name);
	void AddTrkL(std::string name, int user, const class TrackInfo* ti);

	//  track
	void UpdGuiRdStats(const SplineRoad* rd, const Scene& sc, float time), ReadTrkStats();
	MyGUI::MultiList2* trkMList;  MyGUI::EditPtr trkDesc;
	MyGUI::StaticImagePtr imgPrv,imgMini,imgTer, imgTrkIco1,imgTrkIco2;
	const static int StTrk = 12, InfTrk = 10;
	MyGUI::StaticTextPtr valTrk, stTrk[StTrk], infTrk[InfTrk];

	void listTrackChng(MyGUI::MultiList2* li, size_t pos), TrackListUpd(bool resetNotFound=false);
	TracksXml tracksXml;  void btnTrkView1(WP),btnTrkView2(WP),ChangeTrackView(bool full);
	void updTrkListDim(),updChampListDim();
	const static int TcolW[32],ChColW[8],StColW[8];

	void edTrkFind(MyGUI::EditPtr);  Ogre::String sTrkFind;  MyGUI::EditPtr edFind;
	strlist liTracks,liTracksUser;  void FillTrackLists();
	std::list<TrkL> liTrk;

	//  screen
	MyGUI::ListPtr resList;
	void InitGuiScrenRes(), btnResChng(WP), ResizeOptWnd();
	void chkVidFullscr(WP), chkVidVSync(WP), chkVidSSAA(WP);

	void comboGraphicsAll(CMB),
		comboRenderSystem(CMB);
		
	///-----------------------------------------

	//  main menu
	void toggleGui(bool toggle=true), GuiShortcut(WND_Types wnd, int tab);
	void UpdCarClrSld(bool upd=true);  bool bUpdCarClr;
	void MainMenuBtn(MyGUI::WidgetPtr);
	void MenuTabChg(MyGUI::TabPtr, size_t);

	///  championships
	ChampsXml champs;  ProgressXml progress;
	void ChampsXmlLoad(), ProgressSave(bool upgGui=true);
	void ChampNewGame(), ChampLoadEnd(), ChampsListUpdate(),
		ChampFillStageInfo(bool finished), ChampionshipAdvance(float timeCur);

	MyGUI::MultiList2* liChamps, *liStages;
	void listChampChng(MyGUI::MultiList2* li, size_t pos);
	void btnChampStart(WP), btnChampStageBack(WP), btnChampStageStart(WP), btnChampEndClose(WP);
	MyGUI::EditBox* edChampStage, *edChampEnd;  MyGUI::ImageBox * imgChampStage;
	

	///  input tab  -----------------------------------------
	void InitInputGui(), inputBindBtnClicked(WP);
	void InputBind(int key, int button=-1, int axis=-1);

	bool actionIsActive(std::string, std::string);
	void cmbJoystick(CMB), UpdateInputBars(), inputDetailBtn(WP);

	Ogre::String GetInputName(const Ogre::String& sName);
	//  joy events
	virtual bool axisMoved( const OIS::JoyStickEvent &e, int axis );
    virtual bool buttonPressed( const OIS::JoyStickEvent &e, int button );
    virtual bool buttonReleased( const OIS::JoyStickEvent &e, int button );
	MyGUI::StaticTextPtr txtJAxis, txtJBtn, txtInpDetail;
	int lastAxis, axisCnt;  std::string joyName;  class OISB::AnalogAxisAction* actDetail;
	MyGUI::EditPtr edInputMin, edInputMax, edInputMul;  void editInput(MyGUI::EditPtr);
	MyGUI::ComboBox* cmbInpDetSet;  void comboInputPreset(CMB);


	//  sliders  -----------------------------------------
	SLV(Particles);  SLV(Trails);
	SLV(ReflSkip);  SLV(ReflSize);  SLV(ReflFaces);  SLV(ReflDist);  SLV(ReflMode); // refl
	SLV(SizeGaug);  SLV(SizeMinimap);  SLV(SizeArrow);  SLV(ZoomMinimap);  SLV(CountdownTime);  // view
	SLV(VolMaster);  SLV(VolEngine);  SLV(VolTires);  SLV(VolEnv);
	SLV(CarClrH);  SLV(CarClrS);  SLV(CarClrV);  // car clr
	SLV(BloomInt);  SLV(BloomOrig);  SLV(BlurIntens);  // video
	SLV(NumLaps);  // setup
	
	//  checks
	void chkFps(WP), chkGauges(WP),	chkArrow(WP), chkDigits(WP),
		chkMinimap(WP), chkMiniZoom(WP), chkMiniRot(WP), chkMiniTer(WP),  // view
		chkCamInfo(WP), chkTimes(WP), chkOpponents(WP),
		chkCarDbgBars(WP), chkCarDbgTxt(WP), chkBltDebug(WP), chkBltProfilerTxt(WP),
		chkReverse(WP), chkParticles(WP), chkTrails(WP),
		chkAbs(WP), chkTcs(WP), chkGear(WP), chkRear(WP), chkRearInv(WP),  // car
		chkOgreDialog(WP), chkAutoStart(WP), chkEscQuits(WP), chkBltLines(WP), chkLoadPics(WP),  // startup
		chkVidEffects(WP), chkVidBloom(WP), chkVidHDR(WP), chkVidBlur(WP), UpdBloomVals(), chkVidSSAO(WP), // video
		chkVidSoftParticles(WP),chkVidGodRays(WP),
		chkVegetCollis(WP), chkCarCollis(WP);  //car
	void chkUseImposters(WP wp);

	void imgBtnCarClr(WP), btnCarClrRandom(WP);
	MyGUI::ButtonPtr bRkmh, bRmph;  void radKmh(WP), radMph(WP);
	MyGUI::ButtonPtr chDbgT,chDbgB, chBlt,chBltTxt, chFps,
		chTimes,chMinimp,chOpponents;

	///  replay  -----------------------------
	MyGUI::StaticTextPtr valRplPerc, valRplCur, valRplLen,
		valRplName,valRplInfo,valRplName2,valRplInfo2;
	MyGUI::ScrollBar* slRplPos;  void slRplPosEv(SL);
	MyGUI::EditPtr edRplName, edRplDesc;
	void btnRplLoad(WP), btnRplSave(WP), btnRplDelete(WP), btnRplRename(WP),  // btn
		chkRplAutoRec(WP),chkRplChkGhost(WP),chkRplChkBestOnly(WP),chkRplChkAlpha(WP),chkRplChkPar(WP),  // replay
		btnRplToStart(WP),btnRplToEnd(WP), btnRplPlay(WP),  // controls
		btnRplCur(WP),btnRplAll(WP),btnRplGhosts(WP);  // radio
	MyGUI::ButtonPtr rbRplCur, rbRplAll, rbRplGhosts;

	void btnRplBackDn(WP,int,int,MyGUI::MouseButton),btnRplBackUp(WP,int,int,MyGUI::MouseButton);
	void btnRplFwdDn(WP,int,int,MyGUI::MouseButton),btnRplFwdUp(WP,int,int,MyGUI::MouseButton);
	bool bRplBack,bRplFwd;
		
	void msgRplDelete(MyGUI::Message*, MyGUI::MessageBoxStyle);
	
	void btnNumPlayers(WP);  void chkSplitVert(WP);
	MyGUI::StaticTextPtr valLocPlayers;

public:
	bool bRplPlay,bRplPause, bRplRec, bRplWnd;  //  game
	int carIdWin, iCurCar;
protected:
	MyGUI::ButtonPtr btRplPl;  void UpdRplPlayBtn();
	///---------------------------------------

	//  game
	void btnNewGame(WP),btnNewGameStart(WP);
	MyGUI::ListPtr carList, rplList;  void updReplaysList();
	void listRplChng(MyGUI::List* li, size_t pos);
	void listCarChng(MyGUI::List* li, size_t pos),  btnChgCar(WP), btnChgTrack(WP);
	int LNext(MyGUI::MultiList2* lp, int rel), LNext(MyGUI::ListPtr lp, int rel),
		LNext(MyGUI::MultiList* lp, int rel);  // util next in list
	void LNext(int rel);  void tabPlayer(MyGUI::TabPtr wp, size_t id);

	Ogre::String sListCar,sListTrack;  int bListTrackU;
	Ogre::String pathTrk[2];  Ogre::String TrkDir();
	Ogre::String PathListTrk(int user=-1);//, PathListTrkPrv(int user=-1);

	MyGUI::StaticImagePtr imgCar;	MyGUI::StaticTextPtr valCar;
	void comboBoost(CMB), comboFlip(CMB);
	
	char s[512];

	GuiPopup popup;

	///  multiplayer
	///---------------------------------------

	void rebuildGameList();
	void rebuildPlayerList();
	void updateGameInfo();
	void updateGameInfoGUI();
	void uploadGameInfo();
	void setNetGuiHosting(bool enabled);
	void gameListChanged(protocol::GameList list);
	void peerConnected(PeerInfo peer);
	void peerDisconnected(PeerInfo peer);
	void peerInfo(PeerInfo peer);
	void peerMessage(PeerInfo peer, std::string msg);
	void peerState(PeerInfo peer, uint8_t state);
	void gameInfo(protocol::GameInfo game);
	void startRace();
	void error(std::string what);
	void join(std::string host, std::string port, std::string password);

	mutable boost::mutex netGuiMutex;
	MyGUI::UString sChatBuffer;
	protocol::GameInfo netGameInfo;
	bool bRebuildPlayerList;
	bool bRebuildGameList;
	bool bUpdateGameInfo;
	bool bStartGame;

	MyGUI::TabPtr tabsNet;  //void tabNet(TabPtr tab, size_t id);
	MyGUI::WidgetPtr panelNetServer,panelNetGame,panelNetTrack;
	MyGUI::MultiListPtr listServers, listPlayers;
	MyGUI::EditPtr edNetChat;  // chat area, set text through sChatBuffer

	MyGUI::ButtonPtr btnNetRefresh,btnNetJoin,btnNetCreate,btnNetDirect;
	MyGUI::ButtonPtr btnNetReady,btnNetLeave;
	void evBtnNetRefresh(WP);
	void evBtnNetJoin(WP), evBtnNetJoinLockedClose();
	void evBtnNetCreate(WP);
	void evBtnNetDirect(WP),evBtnNetDirectClose();
	void evBtnNetReady(WP),evBtnNetLeave(WP);

	MyGUI::StaticImagePtr imgNetTrack;
	MyGUI::StaticTextPtr valNetGames, valNetGameName, valNetChat, valNetTrack, valNetPassword;
	MyGUI::ButtonPtr btnNetSendMsg;  void chatSendMsg();
	MyGUI::EditPtr edNetGameName, edNetChatMsg, edNetTrackInfo, edNetPassword,
		edNetNick, edNetServerIP, edNetServerPort, edNetLocalPort;
	void evEdNetGameName(MyGUI::EditPtr), evEdNetPassword(MyGUI::EditPtr),
		evEdNetNick(MyGUI::EditPtr), evEdNetServerIP(MyGUI::EditPtr),
		evEdNetServerPort(MyGUI::EditPtr), evEdNetLocalPort(MyGUI::EditPtr);
};

#endif
