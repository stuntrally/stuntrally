#ifndef _OgreApp_h_
#define _OgreApp_h_

#include "BaseApp.h"
#include "../ogre/common/SceneXml.h"
#include "../ogre/common/BltObjects.h"
#include "../ogre/common/TracksXml.h"
#include "../ogre/common/FluidsXml.h"
#include "../ogre/common/MessageBox/MessageBox.h"
#include "../ogre/common/MessageBox/MessageBoxStyle.h"
#include "../ogre/common/WaterRTT.h"

#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
#include "../vdrift/tracksurface.h"
#include "../vdrift/track.h"

#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreString.h>

#include <MyGUI.h>
#include "../ogre/common/RenderBoxScene.h"


const int ciShadowNumSizes = 5;
const int ciShadowSizesA[ciShadowNumSizes] = {256,512,1024,2048,4096};
#define BrushMaxSize  512

//  Gui
const int ciAngSnapsNum = 7;
const Ogre::Real crAngSnaps[ciAngSnapsNum] = {0,5,15,30,45,90,180};

namespace sh { class Factory; }


namespace Forests {  class PagedGeometry;  }
namespace MyGUI  {  class MultiList2;  class Slider;  }

namespace Ogre  {  class Terrain;  class TerrainGlobalOptions;  class TerrainGroup;  class TerrainPaging;  class PageManager;  class Light;  }



class App : public BaseApp, public Ogre::RenderTargetListener
{
public:
	App();  virtual ~App();

	Scene* sc;  /// scene.xml
	FluidsXml fluidsXml;  /// fluid params xml
	BltObjects objs;  // veget collision in bullet

	std::vector <TRACKSURFACE> surfaces;  /// New  all surfaces
	std::map <std::string, int> surf_map;  // name to surface id
	bool LoadAllSurfaces();

	Ogre::Light* sun;  void UpdFog(bool bForce=false), UpdSun();

	void UpdWndTitle(), SaveCam();
	void LoadTrack(), SaveTrack(), UpdateTrack();
	
	// stuff to be executed after BaseApp init
	void postInit(), SetFactoryDefaults();
	void SetEdMode(ED_MODE newMode);

protected:
	void LoadTrackEv(), SaveTrackEv(), UpdateTrackEv();
	enum TrkEvent {  TE_None=0, TE_Load, TE_Save, TE_Update  } eTrkEvent;

	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameStarted(const Ogre::FrameEvent& evt);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	virtual bool frameEnded(const Ogre::FrameEvent& evt);

	virtual void processMouse();
	bool KeyPress(const CmdKey &arg);  void trkListNext(int rel);
	Ogre::Vector3 vNew;	void editMouse();
	
	//  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	bool bNewHmap, bTrGrUpd;
	Ogre::String resTrk;  void NewCommon(bool onlyTerVeget), UpdTrees();
	void CreateTerrain(bool bNewHmap=false, bool bTer=true), CreateBltTerrain(), GetTerAngles(int xb=0,int yb=0,int xe=0,int ye=0, bool full=true);
	void CreateTrees(),  CreateObjects(), DestroyObjects(bool clear), UpdObjPick(), PickObject(), ToggleObjSim();
	void CreateFluids(), DestroyFluids(), CreateBltFluids();
	void UpdFluidBox(), UpdateWaterRTT(Ogre::Camera* cam), UpdMtrWaterDepth();
	void CreateSkyDome(Ogre::String sMater, Ogre::Vector3 scale);

	bool GetFolderIndex(std::string folderpath, std::list <std::string> & outputfolderlist, std::string extension="");
	bool IsVdrTrack();
	// vdrift:
	void CreateVdrTrack(std::string strack, class TRACK* pTrack), CreateVdrTrackBlt(), DestroyVdrTrackBlt(),
		CreateRacingLine(), CreateMinimap(), CreateRoadBezier();
	static Ogre::ManualObject* CreateModel(Ogre::SceneManager* sceneMgr, const Ogre::String& mat,
		class VERTEXARRAY* a, Ogre::Vector3 vPofs, bool flip, bool track=false, const Ogre::String& name="");


	///  rnd to tex  minimap  * * * * * * * * *
	Ogre::SceneNode *ndPos;  Ogre::ManualObject* mpos;
	Ogre::ManualObject* Create2D(const Ogre::String& mat, Ogre::Real s, bool dyn=false);
	Ogre::Real asp, xm1,ym1,xm2,ym2;
	void Rnd2TexSetup(), UpdMiniVis();

	const static int RTs = 4, RTsAdd = 2;
	struct SRndTrg
	{
		Ogre::Camera* rndCam;  Ogre::RenderTexture* rndTex;
		Ogre::Rectangle2D *rcMini;	Ogre::SceneNode* ndMini;
		SRndTrg() : rndCam(0),rndTex(0),rcMini(0),ndMini(0) {  }
	};
	SRndTrg rt[RTs+RTsAdd];
	virtual void preRenderTargetUpdate(const Ogre::RenderTargetEvent &evt);
	virtual void postRenderTargetUpdate(const Ogre::RenderTargetEvent &evt);
	

	///  fluids to destroy
	std::vector<Ogre::String/*MeshPtr*/> vFlSMesh;
	std::vector<Ogre::Entity*> vFlEnt;
	std::vector<Ogre::SceneNode*> vFlNd;
	int iFlCur;  bool bRecreateFluids;

	// vdrift static
	Ogre::StaticGeometry* mStaticGeom;

	// materials
	sh::Factory* mFactory;
	
	
	///  terrain
public:
	Ogre::Terrain* terrain;
protected:
	Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;  bool mPaging;
	Ogre::TerrainPaging* mTerrainPaging;  Ogre::PageManager* mPageManager;

	int iBlendMaps, blendMapSize;	//  mtr from ter  . . . 
	void initBlendMaps(Ogre::Terrain* terrin, int xb=0,int yb=0, int xe=0,int ye=0, bool full=true);
	void configureTerrainDefaults(Ogre::Light* l);
	float Noise(float x, float zoom, int octaves, float persistence);
	float Noise(float x, float y, float zoom, int octaves, float persistance);
	//     xa  xb
	//1    .___.
	//0__./     \.___
	//   xa-s    xb+s
	inline float linRange(const float& x, const float& xa, const float& xb, const float& s)  // min, max, smooth range
	{
		if (x <= xa-s || x >= xb+s)  return 0.f;
		if (x >= xa && x <= xb)  return 1.f;
		if (x < xa)  return (x-xa)/s+1;
		if (x > xb)  return (xb-x)/s+1;
		return 0.f;
	}
		
	void changeShadows(), UpdPSSMMaterials();
public:
	Ogre::Vector4 splitPoints;
protected:
	Ogre::ShadowCameraSetupPtr mPSSMSetup;

	WaterRTT mWaterRTT;

	//  ter circle mesh
	Ogre::ManualObject* moTerC;  Ogre::SceneNode* ndTerC;
	void TerCircleInit(), TerCircleUpd();

	void createBrushPrv(),updateBrushPrv(bool first=false);  Ogre::TexturePtr brushPrvTex;
	const static int BrPrvSize = 128;  //64-


	///<>  terrain edit, brush
	void updBrush();  bool bTerUpd,bTerUpdBlend;  char sBrushTest[512];  int curBr, brImgSave;
	float mBrSize[ED_ALL],mBrIntens[ED_ALL], *mBrushData, terSetH,
		mBrPow[ED_ALL],mBrFq[ED_ALL],mBrNOf[ED_ALL];  int mBrOct[ED_ALL];
	float* pBrFmask, mBrFilt,mBrFiltOld;
	enum EBrShape {   BRS_Triangle=0, BRS_Sinus, BRS_Noise, BRS_ALL  } mBrShape[ED_ALL];
	const static Ogre::String csBrShape[BRS_ALL];

	bool getEditRect(Ogre::Vector3& pos, Ogre::Rect& brushrect, Ogre::Rect& maprect, int size, int& cx, int& cy);

	void deform(Ogre::Vector3 &pos, float dtime, float brMul);
	void height(Ogre::Vector3 &pos, float dtime, float brMul);

	void smooth(Ogre::Vector3 &pos, float dtime);
	void smoothTer(Ogre::Vector3 &pos, float avg, float dtime);
	void calcSmoothFactor(Ogre::Vector3 &pos, float& avg, int& sample_count);

	void filter(Ogre::Vector3 &pos, float dtime, float brMul);

	//void splat(Ogre::Vector3 &pos, float dtime);
	//void paint(Ogre::Vector3 &pos, float dtime);
	//void splatGrass(Ogre::Vector3 &pos, float dtime);
	//bool update(float dtime);


	///  bullet world
public:
	class btDiscreteDynamicsWorld* world;
protected:
	class btDefaultCollisionConfiguration* config;
	class btCollisionDispatcher* dispatcher;
	class bt32BitAxisSweep3* broadphase;
	class btSequentialImpulseConstraintSolver* solver;

	class btCollisionObject* trackObject;  // vdrift track col
	class btTriangleIndexVertexArray* trackMesh;
	void BltWorldInit(), BltWorldDestroy(), BltClear(), BltUpdate(float dt);


	// Weather  rain, snow
	Ogre::ParticleSystem *pr,*pr2;  void CreateWeather(),DestroyWeather();
	float mTimer;

	//  trees
	class Forests::PagedGeometry *trees, *grass;

	//  road  -in base
	void SaveGrassDens(), SaveWaterDepth(), AlignTerToRoad();
	int iSnap;  Ogre::Real angSnap;

	//  car start
	bool LoadStartPos(),SaveStartPos(std::string path);  void UpdStartPos();
	std::vector <MATHVECTOR <float, 3> > vStartPos;
	std::vector <QUATERNION <float> >    vStartRot;
	Ogre::SceneNode* ndCar,*ndStBox,*ndFluidBox,*ndObjBox;
	Ogre::Entity*  entCar,*entStBox,*entFluidBox,*entObjBox;
	void togPrvCam();


	///-----------------------------------------------------------------------------------------------------------------
	///  Gui
	///-----------------------------------------------------------------------------------------------------------------	
	std::map<OIS::KeyCode, MyGUI::Char> mKC;
	//  size
	void SizeGUI(); void doSizeGUI(MyGUI::EnumeratorWidgetPtr);
	std::vector<MyGUI::TabControl*> vSubTabsEdit,vSubTabsHelp,vSubTabsOpts;

	//  shortcuts
	typedef MyGUI::WidgetPtr WP;
	typedef std::list <std::string> strlist;
	//  slider event and its text field for value
	#define SLV(name)  void sl##name(SL);  MyGUI::StaticTextPtr val##name;
	#define SL  MyGUI::Slider* wp, float val			//  slider event args
	#define CMB  MyGUI::ComboBoxPtr cmb, size_t val		//  combo event args
	#define TAB  MyGUI::Tab* tab, size_t id			//  tab event args

	
	///  Gui common   --------------------------
	//  graphics
	SLV(Anisotropy);  SLV(ViewDist);  SLV(TerDetail);  SLV(TerDist);  SLV(RoadDist);
	SLV(TexSize);  SLV(TerMtr);  // detail
	SLV(Trees);  SLV(Grass);  SLV(TreesDist);  SLV(GrassDist);  // paged
	SLV(Shaders);  SLV(ShadowType);  SLV(ShadowCount);  SLV(ShadowSize);  SLV(ShadowDist);  //SLV(ShadowFilter); // shadow
	SLV(WaterSize);  SLV(AntiAliasing); // screen
	void comboTexFilter(CMB), btnShadows(WP), btnShaders(WP), btnTrGrReset(WP),
		chkWaterReflect(WP), chkWaterRefract(WP),
		chkUseImposters(WP), chkImpostorsOnly(WP);

	//  tooltip
	WP mToolTip;  MyGUI::EditPtr mToolTipTxt;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(MyGUI::Widget* sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(MyGUI::Widget *moving, const MyGUI::IntPoint & point);

	//  language
	void comboLanguage(CMB);
	std::map<std::string, MyGUI::UString> languages; // <short name, display name>
	bool bGuiReinit;
	MyGUI::ButtonPtr bnQuit;  void btnQuit(WP);

	//  init
	void InitGui();  bool bGI;
	void GuiCenterMouse(),GuiInitTooltip(),GuiInitLang(), GuiInitGraphics(),GuiInitTrack();
	Ogre::String GetSceneryColor(Ogre::String name);
	void AddTrkL(std::string name, int user, const class TrackInfo* ti);
	
	//  track
	void UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const Ogre::String& sTrack, float time, bool champ=false),
		ReadTrkStats();
	MyGUI::MultiList2* trkMList;  MyGUI::EditPtr trkDesc[1];
	MyGUI::StaticImagePtr imgPrv[1],imgMini[1],imgTer[1], imgTrkIco1,imgTrkIco2;
	const static int StTrk = 12, InfTrk = 11;
	MyGUI::StaticTextPtr valTrk[1], stTrk[1][StTrk], infTrk[1][InfTrk];  // [1] 2nd is in game (common code)

	void listTrackChng(MyGUI::MultiList2* li, size_t pos), TrackListUpd(bool resetNotFound=false);
	TracksXml tracksXml;  void btnTrkView1(WP),btnTrkView2(WP),ChangeTrackView();
	void updTrkListDim();
	const static int TcolW[32];
	const static Ogre::String clrsDiff[9],clrsRating[5],clrsLong[10];

	void edTrkFind(MyGUI::EditPtr);  Ogre::String sTrkFind;  MyGUI::EditPtr edFind;
	strlist liTracks,liTracksUser;  void FillTrackLists();
	std::list<TrkL> liTrk;

	//  screen
	MyGUI::ListPtr resList;
	void InitGuiScrenRes(), btnResChng(WP), ResizeOptWnd();
	void chkVidFullscr(WP), chkVidVSync(WP);

	void comboGraphicsAll(CMB), comboRenderSystem(CMB);

	///-----------------------------------------
	
	void UpdVisGui(), UpdEditWnds();
	void Status(Ogre::String s, float r,float g,float b);
	void SetGuiFromXmls();  bool noBlendUpd;

	const static MyGUI::Colour sUsedClr[8];
	void SetUsedStr(MyGUI::StaticTextPtr valUsed, int cnt, int yellowAt);


	//  tool windows texts
	const static int
		BR_TXT=9, RD_TXT=11, RDS_TXT=9,
		ST_TXT=6, FL_TXT=6, OBJ_TXT=7, RI_TXT=6;
	MyGUI::StaticTextPtr
		brTxt[BR_TXT],brVal[BR_TXT],brKey[BR_TXT],
		rdTxt[RD_TXT],rdVal[RD_TXT],rdKey[RD_TXT],
		rdTxtSt[RDS_TXT],rdValSt[RDS_TXT],
		stTxt[ST_TXT],  flTxt[FL_TXT], objTxt[OBJ_TXT], riTxt[RI_TXT];
	MyGUI::WidgetPtr objPan;
	MyGUI::StaticImagePtr brImg;  MyGUI::TabPtr wndTabs;

	//  main menu
	void toggleGui(bool toggle=false), GuiShortcut(WND_Types wnd, int tab, int subtab=-1), NumTabNext(int rel);
	void MainMenuBtn(MyGUI::WidgetPtr), MenuTabChg(MyGUI::TabPtr, size_t);


	//  [settings]
	void chkMouseCapture(WP), chkOgreDialog(WP),
		chkAutoStart(WP), chkEscQuits(WP), chkStartInMain(WP);  // startup
	SLV(SizeMinmap);  SLV(CamSpeed);  SLV(CamInert);
	SLV(TerUpd);  SLV(SizeRoadP);  SLV(MiniUpd);
	void chkMinimap(WP), btnSetCam(WP);
	void chkAutoBlendmap(WP);  MyGUI::ButtonPtr chAutoBlendmap, chInputBar;
	void chkCamPos(WP), chkInputBar(WP);

	

	//  [Sky]  ----
	MyGUI::ComboBoxPtr cmbSky, cmbRain1,cmbRain2;
	void comboSky(CMB), comboRain1(CMB),comboRain2(CMB);
	SLV(Rain1Rate);  SLV(Rain2Rate);
	SLV(SunPitch);  SLV(SunYaw);
	SLV(FogStart); SLV(FogEnd);  SLV(FogHStart); SLV(FogHEnd);  SLV(FogHeight); SLV(FogHDensity);
	MyGUI::EditPtr edLiAmb,edLiDiff,edLiSpec, edFogClr,edFogClr2,edFogClrH;
	MyGUI::ImageBox* clrAmb,*clrDiff,*clrSpec, *clrFog,*clrFog2,*clrFogH;
	void editFogClr(MyGUI::EditPtr),editFogClr2(MyGUI::EditPtr),editFogClrH(MyGUI::EditPtr);
	void editLiAmb(MyGUI::EditPtr),editLiDiff(MyGUI::EditPtr),editLiSpec(MyGUI::EditPtr);
	void chkFogDisable(WP),chkWeatherDisable(WP);  MyGUI::ButtonPtr chkFog, chkWeather;

	
	///  [Terrain]  ----
	MyGUI::ComboBoxPtr cmbTexDiff, cmbTexNorm;
	void comboTexDiff(CMB), comboTexNorm(CMB);
	MyGUI::StaticImagePtr imgTexDiff;

	MyGUI::ButtonPtr chkTerLay,chkTerLNoiseOnly;  void chkTerLayOn(WP),chkTerLNoiseOnlyOn(WP);  // on
	MyGUI::TabPtr tabsHmap;	  void tabHmap(TAB);  // tabs
	MyGUI::TabPtr tabsTerLayers; void tabTerLayer(TAB);
	int idTerLay;  bool bTerLay;  // help vars
	MyGUI::ButtonPtr chkTexNormAuto;  void chkTexNormAutoOn(WP);  bool bTexNormAuto;  // auto
	
	struct BrushSet  // brush preset ----
	{
		ED_MODE edMode;  int curBr;
		float Size,Intens,Pow,Fq,NOf;
		int Oct;  EBrShape shape;
		float Filter,HSet;
		Ogre::String name;
	};
	const static int brSetsNum = 20;
	const static BrushSet brSets[brSetsNum];
	void btnBrushPreset(WP), SetBrushPreset(int id);

	//  ter generate
	SLV(TerGenScale);  SLV(TerGenOfsX);  SLV(TerGenOfsY);
	SLV(TerGenFreq);  SLV(TerGenOct);  SLV(TerGenPers);  SLV(TerGenPow);
	
	//  ter size
	SLV(TerTriSize);  SLV(TerLScale);
	MyGUI::EditPtr edTerTriSize, edTerLScale;  MyGUI::Slider* sldTerLScale;
	void editTerTriSize(MyGUI::EditPtr), editTerLScale(MyGUI::EditPtr);
	void btnTerrainNew(WP), btnTerGenerate(WP), btnTerrainHalf(WP), btnTerrainDouble(WP), btnTerrainMove(WP);
	const char* getHMapNew();
	MyGUI::StaticTextPtr valTerLAll;
	
	//  ter blendmap
	SLV(TerLAngMin);  SLV(TerLHMin);  SLV(TerLAngSm);
	SLV(TerLAngMax);  SLV(TerLHMax);  SLV(TerLHSm);
	SLV(TerLNoise);  //Chk("TerLNoiseOnly", chkTerLNoiseOnly, 0);

	//  ter particles
	MyGUI::EditPtr edLDust,edLDustS, edLMud,edLSmoke, edLTrlClr;  MyGUI::ImageBox* clrTrail;
	void editLDust(MyGUI::EditPtr), editLTrlClr(MyGUI::EditPtr);
	MyGUI::ComboBoxPtr cmbParDust,cmbParMud,cmbParSmoke;  void comboParDust(CMB);
	
	//  ter surfaces
	MyGUI::ComboBoxPtr cmbSurface;  void comboSurface(CMB), UpdSurfInfo();
	MyGUI::StaticTextPtr txtSurfTire, txtSuBumpWave,txtSuBumpAmp, txtSuRollDrag, txtSuFrict, txtSurfType;
	

	///  [Vegetation]  ----
	SLV(GrMinX);  SLV(GrMaxX);  SLV(GrMinY);  SLV(GrMaxY);
	MyGUI::EditPtr edGrassDens,edTreesDens, edGrPage,edGrDist, edTrPage,edTrDist,
		edGrSwayDistr, edGrSwayLen, edGrSwaySpd, edTrRdDist, edTrImpDist,
		edGrDensSmooth, edSceneryId,
		edGrTerMaxAngle,edGrTerSmAngle, edGrTerMinHeight,edGrTerMaxHeight,edGrTerSmHeight;
	MyGUI::ComboBoxPtr cmbGrassMtr;  void comboGrassMtr(CMB);
	MyGUI::ComboBoxPtr cmbGrassClr;  void comboGrassClr(CMB);
	void editTrGr(MyGUI::EditPtr);
	//  3d view  (veget,objs)
	MyGUI::Canvas* viewCanvas;  wraps::RenderBoxScene viewBox;  MyGUI::IntCoord GetViewSize();
	Ogre::String viewMesh;  void Upd3DView(Ogre::String mesh);  float tiViewUpd;
	
	//  paged layers
	MyGUI::TabPtr tabsPgLayers;  void tabPgLayers(TAB);
	int idPgLay;  // tab
	MyGUI::ButtonPtr chkPgLay;  void chkPgLayOn(WP);  MyGUI::StaticTextPtr valLTrAll;
	MyGUI::ComboBoxPtr cmbPgLay;  void comboPgLay(CMB);
	SLV(LTrDens);	SLV(LTrRdDist);  SLV(LTrRdDistMax);
	SLV(LTrMinSc);	SLV(LTrMaxSc);	SLV(LTrWindFx);	SLV(LTrWindFy);
	SLV(LTrMaxTerAng);
	MyGUI::EditPtr edLTrMinTerH,edLTrMaxTerH,edLTrFlDepth;
	void editLTrMinTerH(MyGUI::EditPtr),editLTrMaxTerH(MyGUI::EditPtr),editLTrFlDepth(MyGUI::EditPtr);

	//  grass layers
	MyGUI::TabPtr tabsGrLayers;  void tabGrLayers(TAB);
	int idGrLay;  // tab
	MyGUI::ButtonPtr chkGrLay;  void chkGrLayOn(WP);
	MyGUI::StaticImagePtr imgGrass,imgGrClr;  MyGUI::StaticTextPtr valLGrAll;
	SLV(LGrDens);
	
	
	//  [Road]  ----
	MyGUI::ComboBoxPtr cmbRoadMtr[4],cmbPipeMtr[4];
	void comboRoadMtr(CMB),comboPipeMtr(CMB);
	MyGUI::EditPtr edRdTcMul,edRdLenDim,edRdWidthSteps,edRdHeightOfs,
		edRdSkirtLen,edRdSkirtH, edRdMergeLen,edRdLodPLen,
		edRdColN,edRdColR, edRdPwsM,edRdPlsM;
	void editRoad(MyGUI::EditPtr);


	///  [Objects]  ----
	enum ED_OBJ {  EO_Move=0, EO_Rotate, EO_Scale  } objEd;
	std::vector<std::string> vObjNames;
	void SetObjNewType(int tnew),UpdObjNewNode(), AddNewObj();
	int iObjCur,iObjLast, iObjTNew;  std::set<int> vObjSel;
	bool objSim;  Object objNew;
	MyGUI::List* objListDyn,*objListSt,*objListBld;  void listObjsChng(MyGUI::List* l,size_t);
	

	//  [Tools]  ----
	MyGUI::StaticTextPtr valTrkCpySel;
	void btnTrkCopySel(WP);  bool ChkTrkCopy();
	void btnCopySun(WP), btnCopyTerHmap(WP), btnCopyTerLayers(WP),
		btnCopyVeget(WP), btnCopyRoad(WP), btnCopyRoadPars(WP);
	void btnScaleAll(WP),btnScaleTerH(WP), btnDeleteRoad(WP),btnDeleteFluids(WP),btnDeleteObjects(WP);
	MyGUI::EditPtr edScaleAllMul;  void editScaleAllMul(MyGUI::EditPtr);
	MyGUI::EditPtr edScaleTerHMul;  void editScaleTerHMul(MyGUI::EditPtr);
	SLV(AlignWidthAdd);  SLV(AlignWidthMul);  SLV(AlignSmooth);


	//  tweak page
	void CreateGUITweakMtr(), slTweak(SL),edTweak(MyGUI::EditPtr);
	void TweakSetMtrPar(std::string name, float val);  void comboTweakMtr(CMB);


	//  [Track]  ----
	Ogre::String pathTrk[2];    // 0 read only  1 //U user paths for save
	std::string TrkDir();  // path to track dir (from pSet settings)

	Ogre::String sListTrack;  int bListTrackU;
	Ogre::String sCopyTrack;  int bCopyTrackU;  // for tools
	Ogre::String PathListTrk(int user=-1), PathListTrkPrv(int user/*=-1*/, Ogre::String track);
	Ogre::String PathCopyTrk(int user=-1);

	void btnTrackNew(WP),btnTrackRename(WP),btnTrackDel(WP),  // track
		msgTrackDel(MyGUI::Message* sender, MyGUI::MessageBoxStyle result);
	void btnNewGame(WP);

	MyGUI::EditPtr trkName;  void editTrkDesc(MyGUI::EditPtr);
	

	//  vdr trk
public:
	TRACK* track;
protected:
	bool LoadTrackVdr(const std::string & trackname);


	//  system, utils
	Ogre::String strFSerrors;
	bool Rename(Ogre::String from, Ogre::String to), Delete(Ogre::String file), DeleteDir(Ogre::String dir),
		 CreateDir(Ogre::String dir), Copy(Ogre::String file, Ogre::String to);
	bool TrackExists(Ogre::String name);  // util

	std::vector<Ogre::String> vsMaterials;
	//void GetMaterialsFromDef(Ogre::String filename, bool clear=true);
	void GetMaterials(Ogre::String filename, bool clear=true, Ogre::String type="material");  // ogre resource
	void GetMaterialsMat(Ogre::String filename, bool clear=true, Ogre::String type="material");  // direct path+file
};

#endif
