#ifndef _OgreGame_h_
#define _OgreGame_h_

#include "BaseApp.h"
#include "../btOgre/BtOgreDebug.h"
#include "../paged-geom/PagedGeometry.h"
#include "common/SceneXml.h"
#include "common/BltObjects.h"
#include "ReplayGame.h"
#include "CarModel.h"

using namespace Ogre;
using namespace MyGUI;


#include "CarReflection.h" //ciShadowSizesA, ciShadowNumSizes


/// Callback for updating serverlist
struct GameInfoListener: public MasterClientCallback {
	GameInfoListener(MultiListPtr list): MasterClientCallback(), mList(list) {}
	void listChanged(protocol::GameList list);
private:
	MultiListPtr mList;
};


class App : public BaseApp, public GameClientCallback //, public RenderTargetListener
{
public:
	App();  virtual ~App();
	
	class GAME* pGame;  ///*
	void updatePoses(float time), newPoses();
	void UpdThr();
	
	// translation
	// can't have it in c'tor, because mygui is not initialized
	void setTranslations();

	/// car ----------------
	//std::list<CarModel*> carModels; //in BaseApp
	
	// This list holds new positions info for every CarModel
	std::list<PosInfo> newPosInfos;
	
	// Utility
	Quaternion qFixCar,qFixWh;

	Replay replay;  ReplayFrame fr;

	Scene sc;  /// scene.xml
	BltObjects objs;  // veget collision in bullet
	Light* sun;  void UpdFog(bool bForce=false), UpdSun();
	
	// Rain, snow
	ParticleSystem *pr,*pr2;
	
	//  trees
	class Forests::PagedGeometry *trees, *grass;
	
	void UpdateHUD(class CAR* pCar, float time, Viewport* vp=NULL), SizeHUD(bool full, Viewport* vp=NULL);

protected:
	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameStart(Real time);
	virtual bool frameEnd(Real time);
	virtual bool keyPressed( const OIS::KeyEvent &arg );
		
	class BtOgre::DebugDrawer *dbgdraw;  /// blt dbg

	//  mtr reload
	enum eMaterials {
		Mtr_CarBody, Mtr_CarInterior, Mtr_CarGlass,
		Mtr_CarTireFront, Mtr_CarTireRear,
		Mtr_Road,  NumMaterials  };
	String sMtr[NumMaterials];
	void reloadMtrTex(String mtrName);

	//  2D, hud  ----
	float asp,  xcRpm, ycRpm, xcVel, ycVel,
		fMiniX,fMiniY, scX,scY, ofsX,ofsY, minX,maxX, minY,maxY;  // minimap

	SceneNode *nrpmB, *nvelBk,*nvelBm, *nrpm, *nvel;  // gauges
	SceneNode *ndPos, *ndMap, *ndLine;  // car pos on minimap
	ManualObject* mrpm, *mvel, *mpos;
	ManualObject* Create2D(const String& mat, SceneManager* sceneMgr, Real size, bool dyn = false);

	OverlayElement* hudGear,*hudVel, *ovL[5],*ovR[5],*ovS[5],*ovU[5], *hudAbs,*hudTcs, *hudTimes,*hudCheck;
	Overlay* ovGear,*ovVel, *ovAbsTcs,*ovCarDbg,*ovCarDbgTxt,  *ovCam, *ovTimes;

	String GetTimeString(float time) const;
	void CreateHUD(), ShowHUD(bool hideAll=false);


	//  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	String resCar, resTrk, resDrv;
	void CreateCar();
	void CreateTrack(), CreateRacingLine(), CreateMinimap(), CreateRoadBezier();
	void CreateTerrain(bool bNewHmap=false, bool bTer=true), CreateBltTerrain();
	void CreateTrees(), CreateRoad(), CreateProps();
	void CreateSkyDome(String sMater, Vector3 scale);
	void NewGame();  void NewGameDoLoad(); bool IsTerTrack();
	
	// Loading
	void LoadCleanUp(), LoadGame(), LoadScene(), LoadCar(), LoadTerrain(), LoadTrack(), LoadMisc();
	enum ELoadState { LS_CLEANUP=0, LS_GAME, LS_SCENE, LS_CAR, LS_TER, LS_TRACK, LS_MISC, LS_ALL };
	
	// id, display name
	// e.g.: 0, Cleaning up or 3, Loading scene
	// initialised in App()
	std::map<unsigned int, std::string> loadingStates;
	// 1 behind map ( map.end() ): loading finished
	std::map<unsigned int, std::string>::iterator currentLoadingState;


	///  terrain
	Terrain* terrain;	TerrainGlobalOptions* mTerrainGlobals;
	TerrainGroup* mTerrainGroup;  bool mPaging;
	TerrainPaging* mTerrainPaging;	PageManager* mPageManager;
	//Vector3 getNormalAtWorldPosition(Terrain* terrain, Real x, Real z, Real s);

	int iBlendMaps, blendMapSize;	//  mtr from ter  . . . 
	char* blendMtr;  // mtr [blendMapSize x blendMapSize]
	void initBlendMaps(Terrain* terrain);
	void configureTerrainDefaults(Light* l);
		
	void changeShadows(), UpdPSSMMaterials(), setMtrSplits(String sMtrName);
	Vector4 splitPoints;  ShadowCameraSetupPtr mPSSMSetup;


	//  road
public:	
	class SplineRoad* road;
protected:
	//  start pos, lap
	bool bGetStPos;  Matrix4 matStPos;	Vector4 vStDist;
	int iInChk, iCurChk, iNextChk, iNumChks;  // cur checkpoint -1 at start
	bool bInSt, bWrongChk;

	///  Gui  ---------------------------------------------------------------------------
	void InitGui();
	void UpdGuiRdStats(const SplineRoad* rd, const Scene& sc, float time), ReadTrkStats();

	//  tooltips
	WidgetPtr mToolTip;  EditPtr mToolTipTxt;
	void setToolTips(EnumeratorWidgetPtr widgets);
	void notifyToolTip(Widget* sender, const ToolTipInfo& info);
	void boundedMove(Widget *moving, const IntPoint & point);

	//  Gui events
	typedef WidgetPtr WP;
	typedef std::list <std::string> strlist;
		//  slider event and its text field for value
	#define SLV(name)  void sl##name(SL);  StaticTextPtr val##name;
	#define SL  WP wp, size_t val	//  slider event args
	
	// control button clicked
	void controlBtnClicked(Widget* sender), InitInputGui();

	//  sliders
	SLV(Anisotropy);  SLV(ViewDist);  SLV(TerDetail);  SLV(TerDist);  SLV(RoadDist);  // detail
	SLV(Particles);  SLV(Trails);
	SLV(Trees);  SLV(Grass);  SLV(TreesDist);  SLV(GrassDist);  // paged
	SLV(ReflSkip);  SLV(ReflSize);  SLV(ReflFaces);  SLV(ReflDist);  // refl
	SLV(Shaders);  SLV(ShadowType);  SLV(ShadowCount);  SLV(ShadowSize);  SLV(ShadowDist);  // shadow
	SLV(SizeGaug);  SLV(SizeMinmap);  // view
	SLV(VolMaster);  SLV(VolEngine);  SLV(VolTires);  SLV(VolEnv);
	SLV(CarClrH);  SLV(CarClrS);  SLV(CarClrV);  // clr
	SLV(BloomInt);  SLV(BloomOrig);  SLV(BlurIntens);  // video

	//  checks
	void chkFps(WP), chkGauges(WP),	chkMinimap(WP), chkRacingLine(WP),  // view
		chkCamInfo(WP), chkTimes(WP), chkCarDbgBars(WP), chkCarDbgTxt(WP), chkBltDebug(WP), chkBltProfilerTxt(WP),
		chkReverse(WP), chkParticles(WP), chkTrails(WP),
		chkAbs(WP), chkTcs(WP), chkGear(WP), chkRear(WP), chkClutch(WP),  // car
		chkOgreDialog(WP), chkAutoStart(WP), chkEscQuits(WP), chkBltLines(WP),  // startup
		chkVidBloom(WP), chkVidHDR(WP), chkVidBlur(WP),  // video
		chkVidFullscr(WP), chkVidVSync(WP), UpdBloomVals(),
		chkLoadPics(WP), chkVegetCollis(WP), chkDigits(WP);

	void comboTexFilter(SL);
	ButtonPtr bRkmh, bRmph;  void radKmh(WP), radMph(WP), btnTrGrReset(WP), btnQuit(WP), btnResChng(WP);
	ButtonPtr chDbgT,chDbgB, chBlt,chBltTxt, chFps, chTimes,chMinimp, bnQuit;

	///  replay
	StaticTextPtr valRplPerc, valRplCur, valRplLen,
		valRplName,valRplInfo,valRplName2,valRplInfo2;
	HScrollPtr slRplPos;  void slRplPosEv(SL);
	EditPtr edRplName, edRplDesc;
	void btnRplLoad(WP), btnRplSave(WP), btnRplDelete(WP),
		chkRplAutoRec(WP),chkRplChkGhost(WP), btnRplCur(WP),btnRplAll(WP),
		btnRplToStart(WP),btnRplToEnd(WP), btnRplBack(WP),btnRplForward(WP),
		btnRplPlay(WP);
	
	void btnNumPlayers(WP); void chkSplitVert(WP);
		
public:
	bool bRplPlay,bRplPause, bRplRec;  //  game
protected:
	ButtonPtr btRplPl;  void UpdRplPlayBtn();

	//  game
	ListPtr carList,trkList, resList, rplList;  void updReplaysList();
	void listRplChng(List* li, size_t pos);
	void listCarChng(List* li, size_t pos),		btnChgCar(WP);
	void listTrackChng(List* li, size_t pos),	btnChgTrack(WP);
	void btnNewGame(WP),btnNewGameStart(WP), btnShadows(WP);
	void trkListNext(int rel), carListNext(int rel);

	String sListCar,sListTrack;  int bListTrackU;
	String pathTrk[2];  String TrkDir();
	String PathListTrk(int user=-1);//, PathListTrkPrv(int user=-1);

	#define StTrk 12
	StaticImagePtr imgCar,imgPrv,imgMini,imgTer;  EditPtr trkDesc;
	StaticTextPtr valCar, valTrk, stTrk[StTrk];

	char s[512];

	///  multiplayer

	void peerConnected(PeerInfo peer);
	void peerDisconnected(PeerInfo peer);
	void peerMessage(PeerInfo peer, std::string msg);


	MultiListPtr listServers, listPlayers;
	ListPtr listNetChat;
	boost::scoped_ptr<GameInfoListener> gameInfoListener;

	String getCreateGameButtonCaption() const;
	ButtonPtr btnNetRefresh,btnNetJoin;  void evBtnNetRefresh(WP),evBtnNetJoin(WP);
	ButtonPtr btnNetReady,btnNetLeave;  void evBtnNetReady(WP),evBtnNetLeave(WP);

	StaticImagePtr imgNetTrack;
	StaticTextPtr valNetGames, valNetChat, valNetTrack;
	ButtonPtr btnNetSendMsg;  void evBtnNetSendMsg(WP);
	EditPtr edNetChatMsg,edNetTrackInfo,
		edNetNick, edNetServerIP, edNetServerPort, edNetLocalPort;
	void evEdNetNick(EditPtr),evEdNetServerIP(EditPtr),evEdNetServerPort(EditPtr),evEdNetLocalPort(EditPtr);
};

#endif
