#ifndef _OgreGame_h_
#define _OgreGame_h_

#include "BaseApp.h"
#include "common/SceneXml.h"
#include "common/BltObjects.h"

#include "ReplayGame.h"
#include "CarModel.h"
#include "CarReflection.h"

#include <MyGUI.h>
#include <OgreShadowCameraSetup.h>

namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;  class Viewport;  class Light;
	class Terrain;  class TerrainGlobalOptions;  class TerrainGroup;  class TerrainPaging;  class PageManager;  }
namespace Forests {  class PagedGeometry;  }
namespace BtOgre  {  class DebugDrawer;  }



class App : public BaseApp, public GameClientCallback, public MasterClientCallback //, public RenderTargetListener
{
public:
	App();  virtual ~App();
	
	class GAME* pGame;  ///*
	void updatePoses(float time), newPoses();
	void UpdThr();
	
	// translation
	// can't have it in c'tor, because mygui is not initialized
	void setTranslations();
	
	// This list holds new positions info for every CarModel
	std::vector<PosInfo> newPosInfos;
	
	// Utility
	Ogre::Quaternion qFixCar,qFixWh;

	//  replay - full, user saves
	//  ghost - saved when best lap,  ghplay - ghost ride replay, loaded if was on disk
	Replay replay, ghost, ghplay;  ReplayFrame fr;
	const Ogre::String& GetGhostFile();

	Scene sc;  /// scene.xml
	BltObjects objs;  // veget collision in bullet
	Ogre::Light* sun;  void UpdFog(bool bForce=false), UpdSun();
	
	// Rain, snow
	Ogre::ParticleSystem *pr,*pr2;
	
	//  trees
	Forests::PagedGeometry *trees, *grass;
	
	void UpdateHUD(int carId, class CarModel* pCarM, class CAR* pCar,
		float time, Ogre::Viewport* vp=NULL), SizeHUD(bool full, Ogre::Viewport* vp=NULL, int carId=-1);
	void UpdHUDRot(int carId, CarModel* pCarM, float vel);

protected:
	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameStart(Ogre::Real time);
	virtual bool frameEnd(Ogre::Real time);
	virtual bool keyPressed( const OIS::KeyEvent &arg );
		
	BtOgre::DebugDrawer *dbgdraw;  /// blt dbg

	//  mtr reload
	enum eMaterials {
		Mtr_CarBody, Mtr_CarInterior, Mtr_CarGlass,
		Mtr_CarTireFront, Mtr_CarTireRear,
		Mtr_Road,  NumMaterials  };
	Ogre::String sMtr[NumMaterials];
	void reloadMtrTex(Ogre::String mtrName);

	//  2D, hud  ----
	float asp,  xcRpm, ycRpm, xcVel, ycVel,
		fMiniX,fMiniY, scX,scY, ofsX,ofsY, minX,maxX, minY,maxY;  // minimap

	Ogre::SceneNode *nrpmB, *nvelBk,*nvelBm, *nrpm, *nvel;  // gauges
	Ogre::SceneNode *ndPos[5], *ndMap, *ndLine;  // car pos on minimap
	Ogre::ManualObject* mrpm, *mvel, *mpos[5], *miniC;
	Ogre::ManualObject* Create2D(const Ogre::String& mat, Ogre::SceneManager* sceneMgr,
		Ogre::Real size, bool dyn = false, bool clr = false);

	Ogre::OverlayElement* hudGear,*hudVel, *ovL[5],*ovR[5],*ovS[5],*ovU[5],
		*hudAbs,*hudTcs, *hudTimes, *hudWarnChk,*hudWonPlace;
	Ogre::Overlay* ovGear,*ovVel, *ovAbsTcs,*ovCarDbg,*ovCarDbgTxt,
		*ovCam, *ovTimes, *ovWarnWin;

	Ogre::String GetTimeString(float time) const;
	void CreateHUD(), ShowHUD(bool hideAll=false);


	//  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	Ogre::String resCar, resTrk, resDrv;
	void CreateCar();
	void CreateTrack(), CreateRacingLine(), CreateMinimap(), CreateRoadBezier();
	void CreateTerrain(bool bNewHmap=false, bool bTer=true), CreateBltTerrain();
	void GetTerAngles(int xb,int yb, int xe,int ye);
	void CreateTrees(), CreateRoad(), CreateProps();
	void CreateSkyDome(Ogre::String sMater, Ogre::Vector3 scale);
	void NewGame();  void NewGameDoLoad(); bool IsTerTrack();
	
	// Loading
	void LoadCleanUp(), LoadGame(), LoadScene(), LoadCar(), LoadTerrain(), LoadTrack(), LoadMisc();
	enum ELoadState { LS_CLEANUP=0, LS_GAME, LS_SCENE, LS_CAR, LS_TER, LS_TRACK, LS_MISC, LS_ALL };
	
	// id, display name, initialised in App()
	// e.g.: 0, Cleaning up or 3, Loading scene
	std::map<unsigned int, std::string> loadingStates;
	// 1 behind map ( map.end() ): loading finished
	std::map<unsigned int, std::string>::iterator currentLoadingState;


	///  terrain
	Ogre::Terrain* terrain;  Ogre::TerrainGlobalOptions* mTerrainGlobals;
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
		
	void changeShadows(), UpdPSSMMaterials(), setMtrSplits(Ogre::String sMtrName);
	Ogre::Vector4 splitPoints;  Ogre::ShadowCameraSetupPtr mPSSMSetup;


	/// joy events
    virtual bool povMoved( const OIS::JoyStickEvent &e, int pov );
	virtual bool axisMoved( const OIS::JoyStickEvent &e, int axis );
    virtual bool sliderMoved( const OIS::JoyStickEvent &e, int sliderID );
    virtual bool buttonPressed( const OIS::JoyStickEvent &e, int button );
    virtual bool buttonReleased( const OIS::JoyStickEvent &e, int button );

	//  road
public:	
	class SplineRoad* road;
protected:
	///  Gui  ---------------------------------------------------------------------------
	void InitGui(), toggleGui();  bool bGI;
	void UpdGuiRdStats(const SplineRoad* rd, const Scene& sc, float time), ReadTrkStats();
	void UpdCarClrSld(bool upd=true);  bool bUpdCarClr;

	//  tooltips
	MyGUI::WidgetPtr mToolTip;  MyGUI::EditPtr mToolTipTxt;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(MyGUI::Widget* sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(MyGUI::Widget *moving, const MyGUI::IntPoint & point);

	//  Gui events
	typedef MyGUI::WidgetPtr WP;
	typedef std::list <std::string> strlist;
		//  slider event and its text field for value
	#define SLV(name)  void sl##name(SL);  MyGUI::StaticTextPtr val##name;
	#define SL  WP wp, size_t val	//  slider event args
	
	// input tab
	void controlBtnClicked(WP), InitInputGui(), UpdateJsButtons();
	void joystickBindChanged(WP, size_t val);
	void joystickSelectionChanged(WP, size_t val);
	void recreateReflections(); // call after refl_mode changed

	//  sliders
	SLV(Anisotropy);  SLV(ViewDist);  SLV(TerDetail);  SLV(TerDist);  SLV(RoadDist);  SLV(TexSize);  // detail
	SLV(Particles);  SLV(Trails);
	SLV(Trees);  SLV(Grass);  SLV(TreesDist);  SLV(GrassDist);  // paged
	SLV(ReflSkip);  SLV(ReflSize);  SLV(ReflFaces);  SLV(ReflDist);  SLV(ReflMode); // refl
	SLV(Shaders);  SLV(ShadowType);  SLV(ShadowCount);  SLV(ShadowSize);  SLV(ShadowDist);  // shadow
	SLV(SizeGaug);  SLV(SizeMinimap);  SLV(ZoomMinimap);  // view
	SLV(VolMaster);  SLV(VolEngine);  SLV(VolTires);  SLV(VolEnv);
	SLV(CarClrH);  SLV(CarClrS);  SLV(CarClrV);  // clr
	SLV(BloomInt);  SLV(BloomOrig);  SLV(BlurIntens);  // video
	SLV(NumLaps);  // setup
	
	//  checks
	void chkFps(WP), chkGauges(WP),	chkDigits(WP),
		chkMinimap(WP), chkMiniZoom(WP), chkMiniRot(WP),  // view
		chkCamInfo(WP), chkTimes(WP), chkCarDbgBars(WP), chkCarDbgTxt(WP), chkBltDebug(WP), chkBltProfilerTxt(WP),
		chkReverse(WP), chkParticles(WP), chkTrails(WP),
		chkAbs(WP), chkTcs(WP), chkGear(WP), chkRear(WP), chkClutch(WP),  // car
		chkOgreDialog(WP), chkAutoStart(WP), chkEscQuits(WP), chkBltLines(WP),  // startup
		chkVidBloom(WP), chkVidHDR(WP), chkVidBlur(WP),  // video
		chkVidFullscr(WP), chkVidVSync(WP), UpdBloomVals(),
		chkLoadPics(WP), chkVegetCollis(WP), chkCarCollis(WP);

	//  language
	void comboLanguage(SL);
	std::map<std::string, std::string> supportedLanguages; // <short name, display name>
	bool bGuiReinit;

	void comboTexFilter(SL), imgBtnCarClr(WP), btnCarClrRandom(WP);
	MyGUI::ButtonPtr bRkmh, bRmph;  void radKmh(WP), radMph(WP), btnTrGrReset(WP), btnQuit(WP), btnResChng(WP);
	MyGUI::ButtonPtr chDbgT,chDbgB, chBlt,chBltTxt, chFps, chTimes,chMinimp, bnQuit;

	///  replay
	MyGUI::StaticTextPtr valRplPerc, valRplCur, valRplLen,
		valRplName,valRplInfo,valRplName2,valRplInfo2;
	MyGUI::HScrollPtr slRplPos;  void slRplPosEv(SL);
	MyGUI::EditPtr edRplName, edRplDesc;
	void btnRplLoad(WP), btnRplSave(WP), btnRplDelete(WP), btnRplRename(WP),  // btn
		chkRplAutoRec(WP),chkRplChkGhost(WP),chkRplChkBestOnly(WP),  // settings
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

	//  game
	void btnNewGame(WP),btnNewGameStart(WP), btnShadows(WP);
	MyGUI::ListPtr carList,trkList, resList, rplList;  void updReplaysList();
	void listRplChng(MyGUI::List* li, size_t pos);
	void listCarChng(MyGUI::List* li, size_t pos),		btnChgCar(WP);
	void listTrackChng(MyGUI::List* li, size_t pos),	btnChgTrack(WP);
	int LNext(MyGUI::ListPtr lp, int rel);  // util next in list
	void trkLNext(int rel), carLNext(int rel), rplLNext(int rel);
	void tabPlayer(MyGUI::TabPtr wp, size_t id);

	Ogre::String sListCar,sListTrack;  int bListTrackU;
	Ogre::String pathTrk[2];  Ogre::String TrkDir();
	Ogre::String PathListTrk(int user=-1);//, PathListTrkPrv(int user=-1);

	#define StTrk 12
	MyGUI::StaticImagePtr imgCar,imgPrv,imgMini,imgTer;  MyGUI::EditPtr trkDesc;
	MyGUI::StaticTextPtr valCar, valTrk, stTrk[StTrk];

	char s[512];

	//  multiplayer

	void rebuildGameList();
	void rebuildPlayerList();
	void setNetGuiHosting(bool enabled);
	void gameListChanged(protocol::GameList list);
	void peerConnected(PeerInfo peer);
	void peerDisconnected(PeerInfo peer);
	void peerInfo(PeerInfo peer);
	void peerMessage(PeerInfo peer, std::string msg);
	void peerState(PeerInfo peer, uint8_t state);
	void join(std::string host, std::string port);

	mutable boost::mutex netGuiMutex;
	MyGUI::UString sChatBuffer;
	bool bRebuildPlayerList;
	bool bRebuildGameList;
	bool bStartGame;

	MyGUI::TabPtr tabsNet;  //void tabNet(TabPtr tab, size_t id);
	MyGUI::WidgetPtr panelNetServer,panelNetGame;
	MyGUI::MultiListPtr listServers, listPlayers;
	MyGUI::EditPtr edNetChat;  // chat area, set text through sChatBuffer

	MyGUI::ButtonPtr btnNetRefresh,btnNetJoin,btnNetCreate,btnNetDirect;
	MyGUI::ButtonPtr btnNetReady,btnNetLeave;
	void evBtnNetRefresh(WP),evBtnNetJoin(WP),evBtnNetCreate(WP),evBtnNetDirect(WP);
	void evBtnNetReady(WP),evBtnNetLeave(WP);

	MyGUI::StaticImagePtr imgNetTrack;
	MyGUI::StaticTextPtr valNetGames, valNetGameName, valNetChat, valNetTrack;
	MyGUI::ButtonPtr btnNetSendMsg;  void chatSendMsg();
	MyGUI::EditPtr edNetGameName, edNetChatMsg, edNetTrackInfo,
		edNetNick, edNetServerIP, edNetServerPort, edNetLocalPort;
	void evEdNetGameName(MyGUI::EditPtr),
		evEdNetNick(MyGUI::EditPtr),evEdNetServerIP(MyGUI::EditPtr),
		evEdNetServerPort(MyGUI::EditPtr),evEdNetLocalPort(MyGUI::EditPtr);
};

#endif
