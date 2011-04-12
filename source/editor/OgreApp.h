#ifndef _OgreApp_h_
#define _OgreApp_h_

#include "BaseApp.h"
#include "../ogre/common/SceneXml.h"
#include "../paged-geom/PagedGeometry.h"
#include "settings.h"
#include "../ogre/common/BltObjects.h"

#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
#include "../vdrift/tracksurface.h"
using namespace MyGUI;

const int ciShadowNumSizes = 4;
const int ciShadowSizesA[ciShadowNumSizes] = {512,1024,2048,4096};
#define BrushMaxSize  512

//  Gui
#define res  1000000.f
#define Fmt  sprintf


class App : public BaseApp,
		public Ogre::RenderTargetListener
{
public:
	App();  virtual ~App();

	Scene sc;  /// scene.xml
	BltObjects objs;  // veget collision in bullet

	TRACKSURFACE su[8];  void LoadSurf(), SaveSurf(const String& trk);
	Light* sun;  void UpdFog(bool bForce=false), UpdSun();

	void UpdWndTitle(), SaveCam();
	void LoadTrack(), SaveTrack(), UpdateTrack();
protected:
	void LoadTrackEv(), SaveTrackEv(), UpdateTrackEv();
	enum TrkEvent {  TE_None=0, TE_Load, TE_Save, TE_Update  } eTrkEvent;

	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameRenderingQueued(const FrameEvent& evt);
	virtual bool frameEnded(const FrameEvent& evt);

	virtual void processMouse();
	//virtual bool keyPressed( const OIS::KeyEvent &arg );
	bool KeyPress(const CmdKey &arg);  void trkListNext(int rel);
	Vector3 vNew;	void editMouse();
	

	//  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	bool bNewHmap, bTrGrUpd;
	String resTrk;  void NewCommon(), UpdTrees();
	void CreateTerrain(bool bNewHmap=false, bool bTer=true);  //void CreateTrees();
	void CreateTrees(), reloadMtrTex(String mtrName);
	void CreateSkyDome(String sMater, Vector3 scale);
	bool GetFolderIndex(string folderpath, std::list <string> & outputfolderlist, std::string extension="");


	///  rnd to tex  minimap  * * * * * * * * *
	SceneNode *ndPos;  ManualObject* mpos;
	ManualObject* Create2D(const String& mat, Real s, bool dyn=false);
	Real asp, xm1,ym1,xm2,ym2;
	void Rnd2TexSetup(), UpdMiniVis();

	const static int RTs = 4;
	struct SRndTrg
	{
		Camera* rndCam;  RenderTexture* rndTex;
		Rectangle2D *rcMini;	SceneNode* ndMini;
		SRndTrg() : rndCam(0),rndTex(0),rcMini(0),ndMini(0) {  }
	};
	SRndTrg rt[RTs+1];
	virtual void preRenderTargetUpdate(const RenderTargetEvent &evt);
	virtual void postRenderTargetUpdate(const RenderTargetEvent &evt);
	

	///  terrain
	Terrain* terrain;	TerrainGlobalOptions* mTerrainGlobals;
	TerrainGroup* mTerrainGroup;  bool mPaging;
	TerrainPaging* mTerrainPaging;	PageManager* mPageManager;

	int iBlendMaps, blendMapSize;	//  mtr from ter  . . . 
	void initBlendMaps(Terrain* terrain);
	void configureTerrainDefaults(Light* l);
		
	void changeShadows(), UpdPSSMMaterials(), setMtrSplits(String sMtrName);
	Vector4 splitPoints;  ShadowCameraSetupPtr mPSSMSetup;


	//  ter circle mesh
	ManualObject* moTerC;	SceneNode* ndTerC;
	void TerCircleInit(), TerCircleUpd();
	

	///<>  terrain edit, brush
	void updBrush();  bool bTerUpd;  char sBrushTest[512];  int curBr;
	float mBrSize[ED_ALL],mBrIntens[ED_ALL],mBrPow[ED_ALL], *mBrushData;

	bool getEditRect(Vector3& pos, Rect& brushrect, Rect& maprect, int size, int& cx, int& cy);
	void deform(Vector3 &pos, float dtime, float brMul);
	void calcSmoothFactor(Vector3 &pos, float& avg, int& sample_count);
	void smooth(Vector3 &pos, float dtime);
	void smoothTer(Vector3 &pos, float avg, float dtime);

	//void splat(Vector3 &pos, float dtime);
	//void paint(Vector3 &pos, float dtime);
	//void splatGrass(Vector3 &pos, float dtime);
	//bool update(float dtime);

	//  trees
	class Forests::PagedGeometry *trees, *grass;

	//  road  -in base
	void SaveGrassDens();

	//  car starts
	bool LoadStartPos(),SaveStartPos(string path);  void UpdStartPos();
	std::vector <MATHVECTOR <float, 3> > vStartPos;
	std::vector <QUATERNION <float> >    vStartRot;
	SceneNode* ndCar,*ndStBox;	Entity* entCar,*entStBox;
	void togPrvCam();


	///-----------------------------------------------------------------------------------------------------------------
	///  Gui
	///-----------------------------------------------------------------------------------------------------------------
	void InitGui(),  UpdVisGui(), UpdEditWnds();
	void UpdGuiRdStats(const SplineRoad* rd, const Scene& sc, float time), ReadTrkStats();
	void Status(String s, float r,float g,float b);
	void SetGuiFromXmls();//, SetXmlsFromGui();
	

	//  shortcuts
	typedef WidgetPtr WP;
	typedef std::list <std::string> strlist;
		//  slider event and its text field for value
	#define SLV(name)  void sl##name(SL);  StaticTextPtr val##name;
	#define SL  WP wp, size_t val				//  slider event args
	#define CMB  ComboBoxPtr cmb, size_t val	//  combo event args
	#define TAB  TabPtr tab, size_t id			//  tab event args

	//  tooltips
	WidgetPtr mToolTip;  EditPtr mToolTipTxt;
	void setToolTips(EnumeratorWidgetPtr widgets);
	void notifyToolTip(Widget* sender, const ToolTipInfo& info);
	void boundedMove(Widget *moving, const IntPoint & point);

	
	//  brush & road windows texts
	const static int BR_TXT=5, RD_TXT=13, RDS_TXT=9;
	StaticTextPtr brTxt[BR_TXT], rdTxt[RD_TXT],rdTxtSt[RDS_TXT];
	StaticImagePtr brImg;  TabPtr wndTabs;


	//  [Graphics]  sliders
	SLV(Anisotropy);  SLV(ViewDist);  SLV(TerDetail);  SLV(TerDist);  SLV(RoadDist);  // detail
	SLV(Trees);  SLV(Grass);  SLV(TreesDist);  SLV(GrassDist);  // paged
	SLV(Shaders);  SLV(ShadowType);  SLV(ShadowCount);  SLV(ShadowSize);  SLV(ShadowDist);  // shadow
	//  checks
	void chkOgreDialog(WP), chkAutoStart(WP), chkEscQuits(WP);  // startup
	void comboTexFilter(SL), btnTrGrReset(WP);
	
	//  [settings]
	SLV(SizeMinmap);  SLV(CamSpeed);  SLV(CamInert);
	SLV(TerUpd);  SLV(SizeRoadP);  SLV(MiniUpd);
	void chkMinimap(WP), btnSetCam(WP);
	ButtonPtr bnQuit;  void btnQuit(WP);
	

	//  [Sky]  ----
	ComboBoxPtr cmbSky, cmbRain1,cmbRain2;
	void comboSky(CMB), comboRain1(CMB),comboRain2(CMB);
	SLV(Rain1Rate);  SLV(Rain2Rate);
	SLV(SunPitch);  SLV(SunYaw);  SLV(FogStart);  SLV(FogEnd);
	EditPtr edLiAmb,edLiDiff,edLiSpec, edFogClr;
	void editLiAmb(EditPtr),editLiDiff(EditPtr),editLiSpec(EditPtr), editFogClr(EditPtr);
	void chkFogDisable(WP);  ButtonPtr chkFog;

	
	///  [Terrain]  ----
	ComboBoxPtr cmbTexDiff, cmbTexNorm;
	void comboTexDiff(CMB), comboTexNorm(CMB);
	StaticImagePtr imgTexDiff;

	ButtonPtr chkTerLay;  void chkTerLayOn(WP);  // on
	TabPtr tabsHmap;	  void tabHmap(TAB);  // tabs
	TabPtr tabsTerLayers; void tabTerLayer(TAB);
	int idTerLay;  bool bTerLay;  // help vars
	ButtonPtr chkTexNormAuto;  void chkTexNormAutoOn(WP);  bool bTexNormAuto;  // auto

	//  ter size
	SLV(TerTriSize);  SLV(TerLScale);
	EditPtr edTerTriSize, edTerLScale;
	void editTerTriSize(EditPtr), editTerLScale(EditPtr);
	void btnTerrainNew(WP), btnTerrainResize(WP);
	StaticTextPtr valTerLAll;

	//  particles
	EditPtr edLDust,edLDustS, edLMud,edLSmoke, edLTrlClr;
	void editLDust(EditPtr), editLTrlClr(EditPtr);
	ComboBoxPtr cmbParDust,cmbParMud,cmbParSmoke;
	void comboParDust(CMB);
	
	//  surfaces
	ComboBoxPtr cmbSurfType;  void comboSurfType(CMB);
	EditPtr edSuBumpWave, edSuBumpAmp, edSuRollDrag, edSuFrict, edSuFrict2;
	void editSurf(EditPtr);
	

	//  [Vegetation]  ----
	EditPtr edGrassDens,edTreesDens, edGrPage,edGrDist, edTrPage,edTrDist,
		edGrMinX,edGrMaxX, edGrMinY,edGrMaxY,
		edGrSwayDistr, edGrSwayLen, edGrSwaySpd, edTrRdDist, edTrImpDist;
	void editTrGr(EditPtr);
	//  paged layers
	ComboBoxPtr cmbPgLay;  void comboPgLay(CMB);
	ButtonPtr chkPgLay;  void chkPgLayOn(WP);
	TabPtr tabsPgLayers;  void tabPgLayers(TAB);
	int idPgLay;  // tab
	StaticImagePtr imgPaged;  StaticTextPtr valLTrAll;
	SLV(LTrDens);	SLV(LTrRdDist);
	SLV(LTrMinSc);	SLV(LTrMaxSc);	SLV(LTrWindFx);	SLV(LTrWindFy);
	
	
	//  [Road]  ----
	ComboBoxPtr cmbRoadMtr[4],cmbPipeMtr[4];
	void comboRoadMtr(CMB),comboPipeMtr(CMB);
	EditPtr edRdTcMul,edRdLenDim,edRdWidthSteps,edRdHeightOfs,
		edRdSkirtLen,edRdSkirtH, edRdMergeLen,edRdLodPLen,
		edRdColN,edRdColR, edRdPwsM,edRdPlsM;
	void editRoad(EditPtr);
	

	//  [Tools]  ----
	StaticTextPtr valTrkCpySel;
	void btnTrkCopySel(WP);  bool ChkTrkCopy();
	void btnCopySun(WP), btnCopyTerHmap(WP), btnCopyTerLayers(WP),
		btnCopyVeget(WP), btnCopyRoad(WP), btnCopyRoadPars(WP);
	void btnScaleAll(WP), btnDeleteRoad(WP);
	EditPtr edScaleAllMul;  void editScaleAllMul(EditPtr);


	//  [Track]  ----
	String pathTrk[2], pathTrkPrv[2];    // 0 read only  1 //U user paths for save
	string TrkDir();  // path to track dir (from pSet settings)

	std::vector<String> vsTracks;
	std::vector<bool> vbTracksUser;
	
	String sListTrack;  int bListTrackU;
	String sCopyTrack;  int bCopyTrackU;  // for tools
	String PathListTrk(int user=-1), PathListTrkPrv(int user=-1);
	String PathCopyTrk(int user=-1);

	ListPtr trkList;  void TrackListUpd();
	void listTrackChng(List* li, size_t pos), //btnChgTrack(WP),
		btnTrackNew(WP),btnTrackRename(WP),btnTrackDel(WP),  // track
		msgTrackDel(Message* sender, MessageBoxStyle result);
	void btnNewGame(WP), btnShadows(WP);

	#define StTrk 12
	StaticImagePtr imgPrv, imgMini,imgTer;
	StaticTextPtr valTrk, stTrk[StTrk];
	EditPtr trkDesc,trkName;  void editTrkDesc(EditPtr);


	//  system, utils
	void Rename(String from, String to), Delete(String file), DeleteDir(String dir),
		CreateDir(String dir), Copy(String file, String to);
	bool TrackExists(String name);  // util

	std::vector<String> vsMaterials;
	void GetMaterials(String filename, String type="material");
};

#endif
