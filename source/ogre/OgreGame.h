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


class App : public BaseApp //, public RenderTargetListener
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
	std::list<PosInfo> newPosInfos;
	
	// Utility
	Ogre::Quaternion qFixCar,qFixWh;

	Replay replay;  ReplayFrame fr;

	Scene sc;  /// scene.xml
	BltObjects objs;  // veget collision in bullet
	Ogre::Light* sun;  void UpdFog(bool bForce=false), UpdSun();
	
	// Rain, snow
	Ogre::ParticleSystem *pr,*pr2;
	
	//  trees
	Forests::PagedGeometry *trees, *grass;
	
	void UpdateHUD(class CAR* pCar, float time, Ogre::Viewport* vp=NULL), SizeHUD(bool full, Ogre::Viewport* vp=NULL);

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
	Ogre::SceneNode *ndPos, *ndMap, *ndLine;  // car pos on minimap
	Ogre::ManualObject* mrpm, *mvel, *mpos;
	Ogre::ManualObject* Create2D(const Ogre::String& mat, Ogre::SceneManager* sceneMgr, Ogre::Real size, bool dyn = false);

	Ogre::OverlayElement* hudGear,*hudVel, *ovL[5],*ovR[5],*ovS[5],*ovU[5], *hudAbs,*hudTcs, *hudTimes,*hudCheck;
	Ogre::Overlay* ovGear,*ovVel, *ovAbsTcs,*ovCarDbg,*ovCarDbgTxt,  *ovCam, *ovTimes;

	Ogre::String GetTimeString(float time) const;
	void CreateHUD(), ShowHUD(bool hideAll=false);


	//  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	Ogre::String resCar, resTrk, resDrv;
	void CreateCar();
	void CreateTrack(), CreateRacingLine(), CreateMinimap(), CreateRoadBezier();
	void CreateTerrain(bool bNewHmap=false, bool bTer=true), CreateBltTerrain();
	void CreateTrees(), CreateRoad(), CreateProps();
	void CreateSkyDome(Ogre::String sMater, Ogre::Vector3 scale);
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
	Ogre::Terrain* terrain;  Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;  bool mPaging;
	Ogre::TerrainPaging* mTerrainPaging;  Ogre::PageManager* mPageManager;
	//Vector3 getNormalAtWorldPosition(Terrain* terrain, Real x, Real z, Real s);

	int iBlendMaps, blendMapSize;	//  mtr from ter  . . . 
	char* blendMtr;  // mtr [blendMapSize x blendMapSize]
	void initBlendMaps(Ogre::Terrain* terrain);
	void configureTerrainDefaults(Ogre::Light* l);
		
	void changeShadows(), UpdPSSMMaterials(), setMtrSplits(Ogre::String sMtrName);
	Ogre::Vector4 splitPoints;  Ogre::ShadowCameraSetupPtr mPSSMSetup;


	//  road
public:	
	class SplineRoad* road;
protected:
	//  start pos, lap
	bool bGetStPos;  Ogre::Matrix4 matStPos;  Ogre::Vector4 vStDist;
	int iInChk, iCurChk, iNextChk, iNumChks;  // cur checkpoint -1 at start
	bool bInSt, bWrongChk;

	///  Gui  ---------------------------------------------------------------------------
	void InitGui(), toggleGui();
	void UpdGuiRdStats(const SplineRoad* rd, const Scene& sc, float time), ReadTrkStats();

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
	void chkFps(WP), chkGauges(WP),	chkDigits(WP), chkMinimap(WP), chkRacingLine(WP),  // view
		chkCamInfo(WP), chkTimes(WP), chkCarDbgBars(WP), chkCarDbgTxt(WP), chkBltDebug(WP), chkBltProfilerTxt(WP),
		chkReverse(WP), chkParticles(WP), chkTrails(WP),
		chkAbs(WP), chkTcs(WP), chkGear(WP), chkRear(WP), chkClutch(WP),  // car
		chkOgreDialog(WP), chkAutoStart(WP), chkEscQuits(WP), chkBltLines(WP),  // startup
		chkVidBloom(WP), chkVidHDR(WP), chkVidBlur(WP),  // video
		chkVidFullscr(WP), chkVidVSync(WP), UpdBloomVals(),
		chkLoadPics(WP), chkVegetCollis(WP), chkCarCollis(WP);

	// language
	void comboLanguage(SL);
	std::map<std::string, std::string> supportedLanguages; // <short name, display name>

	void comboTexFilter(SL);
	MyGUI::ButtonPtr bRkmh, bRmph;  void radKmh(WP), radMph(WP), btnTrGrReset(WP), btnQuit(WP), btnResChng(WP);
	MyGUI::ButtonPtr chDbgT,chDbgB, chBlt,chBltTxt, chFps, chTimes,chMinimp, bnQuit;

	///  replay
	MyGUI::StaticTextPtr valRplPerc, valRplCur, valRplLen,
		valRplName,valRplInfo,valRplName2,valRplInfo2;
	MyGUI::HScrollPtr slRplPos;  void slRplPosEv(SL);
	MyGUI::EditPtr edRplName, edRplDesc;
	void btnRplLoad(WP), btnRplSave(WP), btnRplDelete(WP), btnRplRename(WP),
		chkRplAutoRec(WP),chkRplChkGhost(WP), btnRplCur(WP),btnRplAll(WP),
		btnRplToStart(WP),btnRplToEnd(WP), btnRplBack(WP),btnRplForward(WP),
		btnRplPlay(WP);
	void msgRplDelete(MyGUI::Message*, MyGUI::MessageBoxStyle);
	
	void btnNumPlayers(WP);  void chkSplitVert(WP);
	MyGUI::StaticTextPtr valLocPlayers;
		
public:
	bool bRplPlay,bRplPause, bRplRec, bRplWnd;  //  game
protected:
	MyGUI::ButtonPtr btRplPl;  void UpdRplPlayBtn();

	//  game
	MyGUI::ListPtr carList,trkList, resList, rplList;  void updReplaysList();
	void listRplChng(MyGUI::List* li, size_t pos);
	void listCarChng(MyGUI::List* li, size_t pos),		btnChgCar(WP);
	void listTrackChng(MyGUI::List* li, size_t pos),	btnChgTrack(WP);
	void btnNewGame(WP),btnNewGameStart(WP), btnShadows(WP);
	void trkListNext(int rel), carListNext(int rel);

	Ogre::String sListCar,sListTrack;  int bListTrackU;
	Ogre::String pathTrk[2];  Ogre::String TrkDir();
	Ogre::String PathListTrk(int user=-1);//, PathListTrkPrv(int user=-1);

	#define StTrk 12
	MyGUI::StaticImagePtr imgCar,imgPrv,imgMini,imgTer;  MyGUI::EditPtr trkDesc;
	MyGUI::StaticTextPtr valCar, valTrk, stTrk[StTrk];

	char s[512];
};

#endif
