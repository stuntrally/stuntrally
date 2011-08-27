#ifndef _OgreApp_h_
#define _OgreApp_h_

#include "BaseApp.h"
#include "../ogre/common/SceneXml.h"
#include "../ogre/common/BltObjects.h"

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


const int ciShadowNumSizes = 4;
const int ciShadowSizesA[ciShadowNumSizes] = {512,1024,2048,4096};
#define BrushMaxSize  512

//  Gui
const int ciAngSnapsNum = 6;
const Ogre::Real crAngSnaps[ciAngSnapsNum] = {0,15,30,45,90,180};


namespace Forests
{
	class PagedGeometry;
}


class App : public BaseApp, public Ogre::RenderTargetListener
{
public:
	App();  virtual ~App();

	Scene sc;  /// scene.xml
	BltObjects objs;  // veget collision in bullet

	TRACKSURFACE su[8];  bool LoadSurf(), SaveSurf(const Ogre::String& trk);
	class Ogre::Light* sun;  void UpdFog(bool bForce=false), UpdSun();

	void UpdWndTitle(), SaveCam();
	void LoadTrack(), SaveTrack(), UpdateTrack();
protected:
	void LoadTrackEv(), SaveTrackEv(), UpdateTrackEv();
	enum TrkEvent {  TE_None=0, TE_Load, TE_Save, TE_Update  } eTrkEvent;

	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameStarted(const Ogre::FrameEvent& evt);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	virtual bool frameEnded(const Ogre::FrameEvent& evt);

	virtual void processMouse();
	//virtual bool keyPressed( const OIS::KeyEvent &arg );
	bool KeyPress(const CmdKey &arg);  void trkListNext(int rel);
	Ogre::Vector3 vNew;	void editMouse();
	

	//  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	bool bNewHmap, bTrGrUpd;  Ogre::Real terMaxAng;
	Ogre::String resTrk;  void NewCommon(), UpdTrees();
	void CreateTerrain(bool bNewHmap=false, bool bTer=true);
	void GetTerAngles(int xb,int yb,int xe,int ye);
	void CreateTrees(), reloadMtrTex(Ogre::String mtrName);
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
	

	///  terrain
	Ogre::Terrain* terrain;  Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;  bool mPaging;
	Ogre::TerrainPaging* mTerrainPaging;  Ogre::PageManager* mPageManager;

	int iBlendMaps, blendMapSize;	//  mtr from ter  . . . 
	void initBlendMaps(Ogre::Terrain* terrin);
	float Noise(float x, float y, float zoom, int octaves, float persistance);
	float Noise(float x, float zoom, int octaves, float persistence);
	void configureTerrainDefaults(class Ogre::Light* l);
		
	void changeShadows(), UpdPSSMMaterials(), setMtrSplits(Ogre::String sMtrName);
	Ogre::Vector4 splitPoints;  Ogre::ShadowCameraSetupPtr mPSSMSetup;


	//  ter circle mesh
	Ogre::ManualObject* moTerC;  Ogre::SceneNode* ndTerC;
	void TerCircleInit(), TerCircleUpd();

	void createBrushPrv(),updateBrushPrv(bool first=false);  Ogre::TexturePtr brushPrvTex;
	const static int BrPrvSize = 128;  //64-


	///<>  terrain edit, brush
	void updBrush();  bool bTerUpd,bTerUpdBlend;  char sBrushTest[512];  int curBr;
	float mBrSize[ED_ALL],mBrIntens[ED_ALL], *mBrushData, terSetH,
		mBrPow[ED_ALL],mBrFq[ED_ALL];  int mBrOct[ED_ALL];
	enum EBrShape {   BRS_Triangle=0, BRS_Sinus, BRS_Noise, BRS_ALL  } mBrShape[ED_ALL];
	const static Ogre::String csBrShape[BRS_ALL];

	bool getEditRect(Ogre::Vector3& pos, Ogre::Rect& brushrect, Ogre::Rect& maprect, int size, int& cx, int& cy);
	void deform(Ogre::Vector3 &pos, float dtime, float brMul);
	void height(Ogre::Vector3 &pos, float dtime, float brMul);
	void calcSmoothFactor(Ogre::Vector3 &pos, float& avg, int& sample_count);
	void smooth(Ogre::Vector3 &pos, float dtime);
	void smoothTer(Ogre::Vector3 &pos, float avg, float dtime);

	//void splat(Ogre::Vector3 &pos, float dtime);
	//void paint(Ogre::Vector3 &pos, float dtime);
	//void splatGrass(Ogre::Vector3 &pos, float dtime);
	//bool update(float dtime);

	//  trees
	class Forests::PagedGeometry *trees, *grass;

	//  road  -in base
	void SaveGrassDens();  int iSnap;  Ogre::Real angSnap;

	//  car starts
	bool LoadStartPos(),SaveStartPos(std::string path);  void UpdStartPos();
	std::vector <MATHVECTOR <float, 3> > vStartPos;
	std::vector <QUATERNION <float> >    vStartRot;
	Ogre::SceneNode* ndCar,*ndStBox;	Ogre::Entity* entCar,*entStBox;
	void togPrvCam();


	///-----------------------------------------------------------------------------------------------------------------
	///  Gui
	///-----------------------------------------------------------------------------------------------------------------
	void InitGui(),  UpdVisGui(), UpdEditWnds();
		void GuiCenterMouse(),GuiInitTooltip(),GuiInitLang();
	void UpdGuiRdStats(const SplineRoad* rd, const Scene& sc, float time), ReadTrkStats();
	void Status(Ogre::String s, float r,float g,float b);
	void SetGuiFromXmls();  bool noBlendUpd, bGI;
	

	//  shortcuts
	typedef MyGUI::WidgetPtr WP;
	typedef std::list <std::string> strlist;

	//  slider event and its text field for value
	#define SLV(name)  void sl##name(SL);  MyGUI::StaticTextPtr val##name;

	#define SL   WP wp, size_t val						//  slider event args
	#define CMB  MyGUI::ComboBoxPtr cmb, size_t val		//  combo event args
	#define TAB  MyGUI::TabPtr tab, size_t id			//  tab event args


	///  Gui common   --------------------------
	typedef MyGUI::WidgetPtr WP;
	typedef std::list <std::string> strlist;
		//  slider event and its text field for value
	#define SLV(name)  void sl##name(SL);  MyGUI::StaticTextPtr val##name;
	#define SL  WP wp, size_t val	//  slider event args
	
	//  graphics
	SLV(Anisotropy);  SLV(ViewDist);  SLV(TerDetail);  SLV(TerDist);  SLV(RoadDist);
	SLV(TexSize);  SLV(TerMtr);  // detail
	SLV(Trees);  SLV(Grass);  SLV(TreesDist);  SLV(GrassDist);  // paged
	SLV(Shaders);  SLV(ShadowType);  SLV(ShadowCount);  SLV(ShadowSize);  SLV(ShadowDist);  // shadow
	void comboTexFilter(SL), btnShadows(WP), btnTrGrReset(WP);
	MyGUI::ButtonPtr bnQuit;  void btnQuit(WP);

	//  language
	void comboLanguage(SL);
	std::map<std::string, std::string> supportedLanguages; // <short name, display name>
	bool bGuiReinit;  MyGUI::VectorWidgetPtr vwGui;
	///-----------------------------------------


	//  tooltips
	WP mToolTip;  MyGUI::EditPtr mToolTipTxt;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(MyGUI::Widget* sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(MyGUI::Widget *moving, const MyGUI::IntPoint & point);

	
	//  brush & road windows texts
	const static int BR_TXT=6, RD_TXT=14, RDS_TXT=9;
	MyGUI::StaticTextPtr brTxt[BR_TXT], rdTxt[RD_TXT],rdTxtSt[RDS_TXT];
	MyGUI::StaticImagePtr brImg;  MyGUI::TabPtr wndTabs;


	//  checks
	void chkOgreDialog(WP), chkAutoStart(WP), chkEscQuits(WP);  // startup
	
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

	//  ter size
	SLV(TerTriSize);  SLV(TerLScale);
	MyGUI::EditPtr edTerTriSize, edTerLScale;
	void editTerTriSize(MyGUI::EditPtr), editTerLScale(MyGUI::EditPtr);
	void btnTerrainNew(WP), btnTerGenerate(WP);
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
	

	//  [Vegetation]  ----
	MyGUI::EditPtr edGrassDens,edTreesDens, edGrPage,edGrDist, edTrPage,edTrDist,
		edGrMinX,edGrMaxX, edGrMinY,edGrMaxY,
		edGrSwayDistr, edGrSwayLen, edGrSwaySpd, edTrRdDist, edTrImpDist;
	void editTrGr(MyGUI::EditPtr);
	//  paged layers
	MyGUI::ComboBoxPtr cmbPgLay;  void comboPgLay(CMB);
	MyGUI::ButtonPtr chkPgLay;  void chkPgLayOn(WP);
	MyGUI::TabPtr tabsPgLayers;  void tabPgLayers(TAB);
	int idPgLay;  // tab
	MyGUI::StaticImagePtr imgPaged;  MyGUI::StaticTextPtr valLTrAll;
	SLV(LTrDens);	SLV(LTrRdDist);
	SLV(LTrMinSc);	SLV(LTrMaxSc);	SLV(LTrWindFx);	SLV(LTrWindFy);
	
	
	//  [Road]  ----
	MyGUI::ComboBoxPtr cmbRoadMtr[4],cmbPipeMtr[4];
	void comboRoadMtr(CMB),comboPipeMtr(CMB);
	MyGUI::EditPtr edRdTcMul,edRdLenDim,edRdWidthSteps,edRdHeightOfs,
		edRdSkirtLen,edRdSkirtH, edRdMergeLen,edRdLodPLen,
		edRdColN,edRdColR, edRdPwsM,edRdPlsM;
	void editRoad(MyGUI::EditPtr);
	

	//  [Tools]  ----
	MyGUI::StaticTextPtr valTrkCpySel;
	void btnTrkCopySel(WP);  bool ChkTrkCopy();
	void btnCopySun(WP), btnCopyTerHmap(WP), btnCopyTerLayers(WP),
		btnCopyVeget(WP), btnCopyRoad(WP), btnCopyRoadPars(WP);
	void btnScaleAll(WP), btnDeleteRoad(WP);
	MyGUI::EditPtr edScaleAllMul;  void editScaleAllMul(MyGUI::EditPtr);


	//  [Track]  ----
	Ogre::String pathTrk[2], pathTrkPrv[2];    // 0 read only  1 //U user paths for save
	std::string TrkDir();  // path to track dir (from pSet settings)

	std::vector<Ogre::String> vsTracks;
	std::vector<bool> vbTracksUser;
	
	Ogre::String sListTrack;  int bListTrackU;
	Ogre::String sCopyTrack;  int bCopyTrackU;  // for tools
	Ogre::String PathListTrk(int user=-1), PathListTrkPrv(int user=-1);
	Ogre::String PathCopyTrk(int user=-1);

	MyGUI::ListPtr trkList;  void TrackListUpd();
	void listTrackChng(MyGUI::List* li, size_t pos), //btnChgTrack(WP),
		btnTrackNew(WP),btnTrackRename(WP),btnTrackDel(WP),  // track
		msgTrackDel(MyGUI::Message* sender, MyGUI::MessageBoxStyle result);
	void btnNewGame(WP);

	#define StTrk 12
	MyGUI::StaticImagePtr imgPrv, imgMini,imgTer;
	MyGUI::StaticTextPtr valTrk, stTrk[StTrk];
	MyGUI::EditPtr trkDesc,trkName;  void editTrkDesc(MyGUI::EditPtr);


	//  system, utils
	Ogre::String strFSerrors;
	bool Rename(Ogre::String from, Ogre::String to), Delete(Ogre::String file), DeleteDir(Ogre::String dir),
		 CreateDir(Ogre::String dir), Copy(Ogre::String file, Ogre::String to);
	bool TrackExists(Ogre::String name);  // util

	std::vector<Ogre::String> vsMaterials;
	void GetMaterials(Ogre::String filename, Ogre::String type="material");
};

#endif
