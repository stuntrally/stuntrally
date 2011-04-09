#ifndef _OgreGame_h_
#define _OgreGame_h_

#include "BaseApp.h"
#include "../btOgre/BtOgreDebug.h"
#include "../paged-geom/PagedGeometry.h"
#include "common/SceneXml.h"
#include "BltObjects.h"
//#include "ReplayGame.h"

using namespace Ogre;
using namespace MyGUI;


const int ciShadowNumSizes = 4;
const int ciShadowSizesA[ciShadowNumSizes] = {512,1024,2048,4096};


class App : public BaseApp //, public RenderTargetListener
{
public:
	App();  virtual ~App();
	
	class GAME* pGame;  ///*
	void updatePoses(float time), newPoses();
	void UpdThr();  bool bNew;
	Vector3 newPos,vCarY;  Quaternion newRot;

	//Replay replay;

	Scene sc;  /// scene.xml
	BltObjects objs;  // veget collision in bullet
	Light* sun;  void UpdFog(bool bForce=false), UpdSun();

protected:
	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameStart(Real time);
	virtual bool frameEnd(Real time);
	virtual bool keyPressed( const OIS::KeyEvent &arg );
	//bool KeyPress(const OIS::KeyEvent &arg);
		
	class BtOgre::DebugDrawer *dbgdraw;  /// blt dbg


	//  car  --------
	SceneNode *ndCar, *ndWh[4], *ndWhE[4], *ndRs[4],*ndRd[4];  // car, wheels,emitters
	ManualObject* CreateModel(const String& mat, class VERTEXARRAY* a, bool flip=false, bool track=false);
	Vector3 vPofs;
	//  mtr reload
	enum eMaterials {
		Mtr_CarBody, Mtr_CarInterior, Mtr_CarGlass,
		Mtr_CarTireFront, Mtr_CarTireRear,
		Mtr_Road,  NumMaterials  };
	String sMtr[NumMaterials];
	void CarChangeClr(), reloadMtrTex(String mtrName);
	
	ParticleSystem* ps[4],*pm[4],*pd[4],*pr,*pr2;  // smoke, mud, dust
	RibbonTrail* whTrl[4];
	Real wht[4];  // spin time (approx tire temp.)
	int whTerMtr[4];
	void UpdParsTrails(), UpdWhTerMtr(class CAR* pCar);


	//  2D, hud  ----
	float asp,  xcRpm, ycRpm, xcVel, ycVel,
		fMiniX,fMiniY, scX,scY, ofsX,ofsY, minX,maxX, minY,maxY;  // minimap

	SceneNode *nrpmB, *nvelBk,*nvelBm, *nrpm, *nvel;  // gauges
	SceneNode *ndPos, *ndMap, *ndLine;  // car pos on minimap
	ManualObject* mrpm, *mvel, *mpos;
	ManualObject* Create2D(const String& mat, Real size, bool dyn = false);

	OverlayElement* hudGear,*hudVel, *ovL[5],*ovR[5],*ovS[5],*ovU[5], *hudAbs,*hudTcs, *hudTimes,*hudCheck;
	Overlay* ovGear,*ovVel, *ovAbsTcs,*ovCarDbg,*ovCarDbgTxt,  *ovCam, *ovTimes;

	String GetTimeString(float time) const;
	void CreateHUD(), SizeHUD(bool full), ShowHUD(bool hideAll=false), UpdateHUD(class CAR* pCar, float time);


	//  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	String resCar, resTrk, resDrv;
	void CreateCar();
	void CreateTrack(), CreateRacingLine(), CreateMinimap(), CreateRoadBezier();
	void CreateTerrain(bool bNewHmap=false, bool bTer=true), CreateBltTerrain();
	void CreateTrees(), CreateRoad(), CreateProps();
	void CreateSkyDome(String sMater, Vector3 scale);
	void NewGame();  void NewGameDoLoad(); bool IsTerTrack();
	String TrkDir();
	
	// Loading
	bool bLoading;
	void LoadCleanUp(), LoadGame(), LoadScene(), LoadCar(), LoadTerrain(), LoadTrack(), LoadMisc();
	enum ELoadState { LS_CLEANUP=0, LS_GAME, LS_SCENE, LS_CAR, LS_TER, LS_TRACK, LS_MISC, LS_ALL };
	
	// id, display name
	// e.g.: 0, Cleaning up or 3, Loading scene
	// initialised in App()
	std::map<unsigned int, std::string> loadingStates;
	// 1 behind map ( map.end() ): loading finished
	std::map<unsigned int, std::string>::iterator currentLoadingState;

	bool FileExists(const std::string & filename)
	{
		std::ifstream f(filename.c_str());
		return f;
	}
	

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
	class SplineRoad* road;
	//  start pos, lap
	bool bGetStPos;  Matrix4 matStPos;	Vector4 vStDist;
	int iInChk, iCurChk, iNextChk, iNumChks;  // cur checkpoint -1 at start
	bool bInSt, bWrongChk;


	//  trees
	class Forests::PagedGeometry *trees, *grass;


	///*  Reflections
	void createReflectCams(),destroyReflectCams(), updateReflection();
	//void preRenderTargetUpdate(const RenderTargetEvent &evt);
	//void postRenderTargetUpdate(const RenderTargetEvent &evt);

	TexturePtr cubetex;
	Camera* mReflectCams[6];  RenderTarget* mReflectRT[6];
	int miReflectCam, miReflectCntr;
	bool reflAct, mReflAll1st;


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
	#define SL  WP wp, size_t val
	#define SLV(name)  void sl##name(SL);  StaticTextPtr val##name;

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
		chkCamInfo(WP), chkTimes(WP), chkCarDbgBars(WP), chkCarDbgTxt(WP), chkBltDebug(WP),
		chkReverse(WP), chkParticles(WP), chkTrails(WP),
		chkAbs(WP), chkTcs(WP), chkGear(WP), chkRear(WP), chkClutch(WP),  // car
		chkOgreDialog(WP), chkAutoStart(WP), chkEscQuits(WP), chkBltLines(WP),  // startup
		chkVidBloom(WP), chkVidHDR(WP), chkVidBlur(WP),  // video
		chkVidFullscr(WP), chkVidVSync(WP), UpdBloomVals(),
		chkLoadPics(WP), chkVegetCollis(WP);

	void comboTexFilter(SL);
	ButtonPtr bRkmh, bRmph;  void radKmh(WP), radMph(WP), btnTrGrReset(WP), btnQuit(WP), btnResChng(WP);
	ButtonPtr chDbgT,chDbgB, chBlt,chFps, chTimes,chMinimp, bnQuit;

	//  game
	String sListCar,sListTrack;  ListPtr carList,trkList, resList;
	void listCarChng(List* li, size_t pos),		btnChgCar(WP);
	void listTrackChng(List* li, size_t pos),	btnChgTrack(WP);
	void btnNewGame(WP),btnNewGameStart(WP), btnShadows(WP);
	void trkListNext(int rel), carListNext(int rel);

	#define StTrk 12
	StaticImagePtr imgCar,imgPrv,imgMini,imgTer;  EditPtr trkDesc;
	StaticTextPtr valCar, valTrk, stTrk[StTrk];

	char s[512];
};

#endif
