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

//#include <OgrePrerequisites.h>
#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreString.h>
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreTerrainPaging.h>
#include <OgrePageManager.h>

#include <MyGUI.h>

const int ciShadowNumSizes = 5;
const int ciShadowSizesA[ciShadowNumSizes] = {256,512,1024,2048,4096};
#define BrushMaxSize  512

//  Gui
const int ciAngSnapsNum = 6;
const Ogre::Real crAngSnaps[ciAngSnapsNum] = {0,15,30,45,90,180};


namespace Forests {  class PagedGeometry;  }
namespace MyGUI  {  class MultiList2;  class Slider;  }
class MaterialFactory;


class App : public BaseApp, public Ogre::RenderTargetListener
{
public:
	App();  virtual ~App();

	Scene sc;  /// scene.xml
	FluidsXml fluidsXml;  /// fluid params xml
	BltObjects objs;  // veget collision in bullet

	TRACKSURFACE su[8];  bool LoadSurf(), SaveSurf(const Ogre::String& trk);
	class Ogre::Light* sun;  void UpdFog(bool bForce=false), UpdSun();

	void UpdWndTitle(), SaveCam();
	void LoadTrack(), SaveTrack(), UpdateTrack();
	
	// stuff to be executed after BaseApp init
	void postInit();
	
	Ogre::SceneManager* sceneMgr() { return mSceneMgr; };
	
	MaterialFactory* materialFactory;
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
	bool bNewHmap, bTrGrUpd;  Ogre::Real terMaxAng;
	Ogre::String resTrk;  void NewCommon(bool onlyTerVeget), UpdTrees();
	void CreateTerrain(bool bNewHmap=false, bool bTer=true), CreateBltTerrain(), GetTerAngles(int xb,int yb,int xe,int ye);
	void CreateTrees(), CreateObjects(),DestroyObjects(), UpdObjPick();
	void CreateFluids(), DestroyFluids(), CreateBltFluids(), UpdFluidBox(), UpdateWaterRTT(Ogre::Camera* cam);
	void CreateSkyDome(Ogre::String sMater, Ogre::Vector3 scale);
	bool GetFolderIndex(std::string folderpath, std::list <std::string> & outputfolderlist, std::string extension="");


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
	//  objects
	int iObjCur;
	
	
	///  terrain
public:
	Ogre::Terrain* terrain;
protected:
	Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;  bool mPaging;
	Ogre::TerrainPaging* mTerrainPaging;  Ogre::PageManager* mPageManager;

	int iBlendMaps, blendMapSize;	//  mtr from ter  . . . 
	void initBlendMaps(Ogre::Terrain* terrin);
	float Noise(float x, float y, float zoom, int octaves, float persistance);
	float Noise(float x, float zoom, int octaves, float persistence);
	void configureTerrainDefaults(class Ogre::Light* l);
		
	void changeShadows(), UpdPSSMMaterials();
public:
	Ogre::Vector4 splitPoints; void createRoadSelMtr(Ogre::String sMtrName), CreateRoadSelMtrs();
protected:
	Ogre::ShadowCameraSetupPtr mPSSMSetup;

	WaterRTT mWaterRTT;

	//  ter circle mesh
	Ogre::ManualObject* moTerC;  Ogre::SceneNode* ndTerC;
	void TerCircleInit(), TerCircleUpd();

	void createBrushPrv(),updateBrushPrv(bool first=false);  Ogre::TexturePtr brushPrvTex;
	const static int BrPrvSize = 128;  //64-


	///<>  terrain edit, brush
	void updBrush();  bool bTerUpd,bTerUpdBlend;  char sBrushTest[512];  int curBr;
	float mBrSize[ED_ALL],mBrIntens[ED_ALL], *mBrushData, terSetH,
		mBrPow[ED_ALL],mBrFq[ED_ALL];  int mBrOct[ED_ALL];  float* pBrFmask, mBrFilt,mBrFiltOld;
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


	//  trees
	class Forests::PagedGeometry *trees, *grass;

	//  road  -in base
	void SaveGrassDens(), SaveWaterDepth(), AlignTerToRoad();
	class btDiscreteDynamicsWorld* world;  int iSnap;  Ogre::Real angSnap;

	//  car starts
	bool LoadStartPos(),SaveStartPos(std::string path);  void UpdStartPos();
	std::vector <MATHVECTOR <float, 3> > vStartPos;
	std::vector <QUATERNION <float> >    vStartRot;
	Ogre::SceneNode* ndCar,*ndStBox,*ndFluidBox,*ndObjBox;
	Ogre::Entity*  entCar,*entStBox,*entFluidBox,*entObjBox;
	void togPrvCam();


	///-----------------------------------------------------------------------------------------------------------------
	///  Gui
	///-----------------------------------------------------------------------------------------------------------------	
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
	SLV(Shaders);  SLV(ShadowType);  SLV(ShadowCount);  SLV(ShadowSize);  SLV(LightmapSize);  SLV(ShadowDist);  SLV(ShadowFilter); // shadow
	SLV(WaterSize);  SLV(AntiAliasing); // screen
	void comboTexFilter(CMB), btnShadows(WP), btnShaders(WP), btnTrGrReset(WP);
	MyGUI::ButtonPtr bnQuit;  void btnQuit(WP);
	void chkWaterReflect(WP), chkWaterRefract(WP);

	//  tooltip
	WP mToolTip;  MyGUI::EditPtr mToolTipTxt;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(MyGUI::Widget* sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(MyGUI::Widget *moving, const MyGUI::IntPoint & point);

	//  language
	void comboLanguage(CMB);
	std::map<std::string, MyGUI::UString> languages; // <short name, display name>
	bool bGuiReinit;

	//  init
	void InitGui();  bool bGI;
	void GuiCenterMouse(),GuiInitTooltip(),GuiInitLang(), GuiInitGraphics(),GuiInitTrack();
	Ogre::String GetSceneryColor(Ogre::String name);
	void AddTrkL(std::string name, int user, const class TrackInfo* ti);
	
	//  track
	void UpdGuiRdStats(const SplineRoad* rd, const Scene& sc, const Ogre::String& sTrack, float time, bool champ=false), ReadTrkStats();
	MyGUI::MultiList2* trkMList;  MyGUI::EditPtr trkDesc[1];
	MyGUI::StaticImagePtr imgPrv[1],imgMini[1],imgTer[1], imgTrkIco1,imgTrkIco2;
	const static int StTrk = 12, InfTrk = 10;
	MyGUI::StaticTextPtr valTrk[1], stTrk[1][StTrk], infTrk[1][InfTrk];  // [1] 2nd is in game (common code)

	void listTrackChng(MyGUI::MultiList2* li, size_t pos), TrackListUpd(bool resetNotFound=false);
	TracksXml tracksXml;  void btnTrkView1(WP),btnTrkView2(WP),ChangeTrackView(),updTrkListDim();
	const static int TcolW[32];

	void edTrkFind(MyGUI::EditPtr);  Ogre::String sTrkFind;  MyGUI::EditPtr edFind;
	strlist liTracks,liTracksUser;  void FillTrackLists();
	std::list<TrkL> liTrk;

	//  screen
	MyGUI::ListPtr resList;
	void InitGuiScrenRes(), btnResChng(WP), ResizeOptWnd();
	void chkVidFullscr(WP), chkVidVSync(WP);

	void comboGraphicsAll(CMB),
		comboRenderSystem(MyGUI::ComboBoxPtr wp, size_t val);

	
	void UpdVisGui(), UpdEditWnds();
	void Status(Ogre::String s, float r,float g,float b);
	void SetGuiFromXmls();  bool noBlendUpd;


	//  tool windows texts
	const static int
		BR_TXT=6, RD_TXT=14,RDS_TXT=9,
		ST_TXT=6, FL_TXT=6, OBJ_TXT=6;
	MyGUI::StaticTextPtr
		brTxt[BR_TXT], rdTxt[RD_TXT],rdTxtSt[RDS_TXT],
		stTxt[ST_TXT], flTxt[FL_TXT], objTxt[OBJ_TXT];
	MyGUI::StaticImagePtr brImg;  MyGUI::TabPtr wndTabs;

	//  main menu
	void toggleGui(bool toggle=false), GuiShortcut(WND_Types wnd, int tab, int subtab=-1);
	void MainMenuBtn(MyGUI::WidgetPtr);
	void MenuTabChg(MyGUI::TabPtr, size_t);


	//  checks
	void chkMouseCapture(WP), chkOgreDialog(WP), chkAutoStart(WP), chkEscQuits(WP);  // startup
	void chkUseImposters(WP wp);

	//  [settings]
	SLV(SizeMinmap);  SLV(CamSpeed);  SLV(CamInert);
	SLV(TerUpd);  SLV(SizeRoadP);  SLV(MiniUpd);
	void chkMinimap(WP), btnSetCam(WP);
	

	//  [Sky]  ----
	MyGUI::ComboBoxPtr cmbSky, cmbRain1,cmbRain2;
	void comboSky(CMB), comboRain1(CMB),comboRain2(CMB);
	SLV(Rain1Rate);  SLV(Rain2Rate);
	SLV(SunPitch);  SLV(SunYaw);  SLV(FogStart);  SLV(FogEnd);
	MyGUI::EditPtr edLiAmb,edLiDiff,edLiSpec, edFogClr;
	void editLiAmb(MyGUI::EditPtr),editLiDiff(MyGUI::EditPtr),editLiSpec(MyGUI::EditPtr), editFogClr(MyGUI::EditPtr);
	void chkFogDisable(WP);  MyGUI::ButtonPtr chkFog;

	
	///  [Terrain]  ----
	MyGUI::ComboBoxPtr cmbTexDiff, cmbTexNorm;
	void comboTexDiff(CMB), comboTexNorm(CMB);
	MyGUI::StaticImagePtr imgTexDiff;

	MyGUI::ButtonPtr chkTerLay,chkTerLNoiseOnly;  void chkTerLayOn(WP),chkTerLNoiseOnlyOn(WP);  // on
	MyGUI::TabPtr tabsHmap;	  void tabHmap(TAB);  // tabs
	MyGUI::TabPtr tabsTerLayers; void tabTerLayer(TAB);
	int idTerLay;  bool bTerLay;  // help vars
	MyGUI::ButtonPtr chkTexNormAuto;  void chkTexNormAutoOn(WP);  bool bTexNormAuto;  // auto

	//  ter generate
	SLV(TerGenScale);  SLV(TerGenOfsX);  SLV(TerGenOfsY);
	SLV(TerGenFreq);  SLV(TerGenOct);  SLV(TerGenPers);  SLV(TerGenPow);
	
	//  ter size
	SLV(TerTriSize);  SLV(TerLScale);
	MyGUI::EditPtr edTerTriSize, edTerLScale;
	void editTerTriSize(MyGUI::EditPtr), editTerLScale(MyGUI::EditPtr);
	void btnTerrainNew(WP), btnTerGenerate(WP), btnTerrainHalf(WP);
	MyGUI::StaticTextPtr valTerLAll;
	
	//  ter blendmap
	SLV(TerLAngMin);  SLV(TerLHMin);  SLV(TerLAngSm);
	SLV(TerLAngMax);  SLV(TerLHMax);  SLV(TerLHSm);
	SLV(TerLNoise);  //Chk("TerLNoiseOnly", chkTerLNoiseOnly, 0);

	//  ter particles
	MyGUI::EditPtr edLDust,edLDustS, edLMud,edLSmoke, edLTrlClr;
	void editLDust(MyGUI::EditPtr), editLTrlClr(MyGUI::EditPtr);
	MyGUI::ComboBoxPtr cmbParDust,cmbParMud,cmbParSmoke;
	void comboParDust(CMB);
	
	//  ter surfaces
	MyGUI::ComboBoxPtr cmbSurfType;  void comboSurfType(CMB);
	MyGUI::EditPtr edSuBumpWave, edSuBumpAmp, edSuRollDrag, edSuFrict, edSuFrict2;
	void editSurf(MyGUI::EditPtr);
	

	///  [Vegetation]  ----
	MyGUI::EditPtr edGrassDens,edTreesDens, edGrPage,edGrDist, edTrPage,edTrDist,
		edGrMinX,edGrMaxX, edGrMinY,edGrMaxY,
		edGrSwayDistr, edGrSwayLen, edGrSwaySpd, edTrRdDist, edTrImpDist,
		edGrDensSmooth, edGrTerMaxAngle,edGrTerMaxHeight, edSceneryId;
	MyGUI::ComboBoxPtr cmbGrassMtr;  void comboGrassMtr(CMB);
	MyGUI::ComboBoxPtr cmbGrassClr;  void comboGrassClr(CMB);
	void editTrGr(MyGUI::EditPtr);

	//  paged layers
	MyGUI::ComboBoxPtr cmbPgLay;  void comboPgLay(CMB);
	MyGUI::ButtonPtr chkPgLay;  void chkPgLayOn(WP);
	MyGUI::TabPtr tabsPgLayers;  void tabPgLayers(TAB);
	int idPgLay;  // tab
	MyGUI::StaticImagePtr imgPaged;  MyGUI::StaticTextPtr valLTrAll;
	SLV(LTrDens);	SLV(LTrRdDist);
	SLV(LTrMinSc);	SLV(LTrMaxSc);	SLV(LTrWindFx);	SLV(LTrWindFy);
	SLV(LTrMaxTerAng);
	MyGUI::EditPtr edLTrMinTerH,edLTrMaxTerH,edLTrFlDepth;
	void editLTrMinTerH(MyGUI::EditPtr),editLTrMaxTerH(MyGUI::EditPtr),editLTrFlDepth(MyGUI::EditPtr);
	
	
	///  [Road]  ----
	MyGUI::ComboBoxPtr cmbRoadMtr[4],cmbPipeMtr[4];
	void comboRoadMtr(CMB),comboPipeMtr(CMB);
	MyGUI::EditPtr edRdTcMul,edRdLenDim,edRdWidthSteps,edRdHeightOfs,
		edRdSkirtLen,edRdSkirtH, edRdMergeLen,edRdLodPLen,
		edRdColN,edRdColR, edRdPwsM,edRdPlsM;
	void editRoad(MyGUI::EditPtr);

	//  [Objects]
	std::vector<std::string> vObjNames;  int iObjNew;
	

	//  [Tools]  ----
	MyGUI::StaticTextPtr valTrkCpySel;
	void btnTrkCopySel(WP);  bool ChkTrkCopy();
	void btnCopySun(WP), btnCopyTerHmap(WP), btnCopyTerLayers(WP),
		btnCopyVeget(WP), btnCopyRoad(WP), btnCopyRoadPars(WP);
	void btnScaleAll(WP), btnDeleteRoad(WP), btnScaleTerH(WP);
	MyGUI::EditPtr edScaleAllMul;  void editScaleAllMul(MyGUI::EditPtr);
	MyGUI::EditPtr edScaleTerHMul;  void editScaleTerHMul(MyGUI::EditPtr);


	//  [Track]  ----
	Ogre::String pathTrk[2], pathTrkPrv[2];    // 0 read only  1 //U user paths for save
	std::string TrkDir();  // path to track dir (from pSet settings)

	Ogre::String sListTrack;  int bListTrackU;
	Ogre::String sCopyTrack;  int bCopyTrackU;  // for tools
	Ogre::String PathListTrk(int user=-1), PathListTrkPrv(int user=-1);
	Ogre::String PathCopyTrk(int user=-1);

	void btnTrackNew(WP),btnTrackRename(WP),btnTrackDel(WP),  // track
		msgTrackDel(MyGUI::Message* sender, MyGUI::MessageBoxStyle result);
	void btnNewGame(WP);

	MyGUI::EditPtr trkName;  void editTrkDesc(MyGUI::EditPtr);


	//  system, utils
	Ogre::String strFSerrors;
	bool Rename(Ogre::String from, Ogre::String to), Delete(Ogre::String file), DeleteDir(Ogre::String dir),
		 CreateDir(Ogre::String dir), Copy(Ogre::String file, Ogre::String to);
	bool TrackExists(Ogre::String name);  // util

	std::vector<Ogre::String> vsMaterials;
	void GetMaterials(Ogre::String filename, bool clear=true, Ogre::String type="material");
	void GetMaterialsFromDef(Ogre::String filename, bool clear=true);
};

#endif
