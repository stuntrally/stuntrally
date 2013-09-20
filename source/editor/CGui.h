#pragma once
#include "BaseApp.h"
#include "../vdrift/tracksurface.h"
#include "../vdrift/track.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/SliderValue.h"
#include "../ogre/common/SceneXml.h"  //Object-

#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreString.h>

#include <MyGUI.h>
#include "../ogre/common/MessageBox/MessageBoxStyle.h"
#include "../ogre/common/RenderBoxScene.h"


namespace MyGUI  {  class MultiList2;  class Slider;  class Message;  }
class App;  class Scene;  class CData;  class SETTINGS;

enum ED_OBJ {  EO_Move=0, EO_Rotate, EO_Scale  };


//  tracks,cars list items - with info for sorting
struct TrkL
{
	std::string name;
	const class TrackInfo* ti;
	int test;  //Test*
};


class CGui //: public BGui
{
public:
	App* app;
	SETTINGS* pSet;
	Scene* sc;
	CData* data;

	CGui(App* app1);
	//virtual ~CGui();

	///-----------------------------------------------------------------------------------------------------------------
	///  Gui
	///-----------------------------------------------------------------------------------------------------------------	
	//  size
	void SizeGUI(); void doSizeGUI(MyGUI::EnumeratorWidgetPtr);
	std::vector<MyGUI::TabControl*> vSubTabsEdit, vSubTabsHelp, vSubTabsOpts;

	
	///  Gui common   --------------------------
	//  graphics
	SlV(Anisotropy);  SlV(ViewDist);  SlV(TerDetail);  SlV(TerDist);  SlV(RoadDist);
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
	void GuiCenterMouse(),GuiInitTooltip(),GuiInitLang(), GuiInitGraphics(),GuiInitTrack();
	Ogre::String GetSceneryColor(Ogre::String name);
	void AddTrkL(std::string name, int user, const class TrackInfo* ti);
	
	//  track
	void UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const Ogre::String& sTrack, float time, bool champ=false),
		ReadTrkStats();
	MyGUI::MultiList2* trkList;  MyGUI::EditPtr trkDesc[1];
	MyGUI::StaticImagePtr imgPrv[1],imgMini[1],imgTer[1], imgTrkIco1,imgTrkIco2;
	const static int StTrk = 12, InfTrk = 11;
	MyGUI::StaticTextPtr valTrk[1], stTrk[1][StTrk], infTrk[1][InfTrk];  // [1] 2nd is in game (common code)

	void listTrackChng(MyGUI::MultiList2* li, size_t pos), TrackListUpd(bool resetNotFound=false);
	void btnTrkView1(WP),btnTrkView2(WP),ChangeTrackView();
	void updTrkListDim();
	const static int colTrk[32];
	const static Ogre::String clrsDiff[9],clrsRating[6],clrsLong[10];

	void edTrkFind(MyGUI::EditPtr);  Ogre::String sTrkFind;  MyGUI::EditPtr edFind;
	strlist liTracks,liTracksUser;  void FillTrackLists();
	std::list<TrkL> liTrk;

	//  screen
	MyGUI::ListPtr resList;
	void InitGuiScreenRes(), btnResChng(WP), ResizeOptWnd();
	void chkVidFullscr(WP), chkVidVSync(WP);

	void comboGraphicsAll(CMB), comboRenderSystem(CMB);

	///-----------------------------------------
	
	void trkListNext(int rel);
	void Status(Ogre::String s, float r,float g,float b);
	void SetGuiFromXmls();  bool noBlendUpd;

	const static MyGUI::Colour sUsedClr[8];
	void SetUsedStr(MyGUI::StaticTextPtr valUsed, int cnt, int yellowAt);
	
	//  _Tools_
	void ToolTexAlpha(),ToolSceneXml(),ToolListSceneryID(),ToolTracksWarnings(),ToolBrushesPrv();	


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

	SlV(SizeMinmap);  SlV(SizeRoadP);
	SV svCamSpeed, svCamInert;
	SV svTerUpd, svMiniUpd;

	void chkMinimap(WP), btnSetCam(WP);
	void chkAutoBlendmap(WP);  MyGUI::ButtonPtr chAutoBlendmap, chInputBar;
	void chkCamPos(WP), chkInputBar(WP);

	void toggleTopView();  bool bTopView, oldFog;
	Ogre::Vector3 oldPos,oldRot;


	//  [Sky]  ----
	MyGUI::ComboBoxPtr cmbSky, cmbRain1,cmbRain2;
	void comboSky(CMB), comboRain1(CMB),comboRain2(CMB);

	SV svRain1Rate, svRain2Rate;
	SV svSunPitch, svSunYaw;
	void slUpdSun(SV*), slUpdFog(SV*);
	SV svFogStart, svFogEnd;
	SV svFogHStart, svFogHEnd;  // Hfog
	SV svFogHeight, svFogHDensity;

	MyGUI::EditPtr edLiAmb,edLiDiff,edLiSpec, edFogClr,edFogClr2,edFogClrH;
	MyGUI::ImageBox* clrAmb,*clrDiff,*clrSpec, *clrFog,*clrFog2,*clrFogH;
	void editFogClr(MyGUI::EditPtr),editFogClr2(MyGUI::EditPtr),editFogClrH(MyGUI::EditPtr);
	void editLiAmb(MyGUI::EditPtr),editLiDiff(MyGUI::EditPtr),editLiSpec(MyGUI::EditPtr);

	void chkFogDisable(WP),chkWeatherDisable(WP);  MyGUI::ButtonPtr chkFog, chkWeather;

	
	///  [Terrain]  --------------------
	MyGUI::ComboBoxPtr cmbTexDiff, cmbTexNorm;
	void comboTexDiff(CMB), comboTexNorm(CMB);
	MyGUI::StaticImagePtr imgTexDiff;

	MyGUI::ButtonPtr chkTerLay, chkTerLNoiseOnly, chkTerLayTripl;
	void chkTerLayOn(WP), chkTerLNoiseOnlyOn(WP), chkTerLayTriplOn(WP);  // on
	//  HMap tab
	MyGUI::TabPtr tabsHmap;	 void tabHmap(TAB);
	void updTabHmap();  int getHMapSizeTab();
	
	bool bTexNormAuto;  // auto norm tex
	MyGUI::ButtonPtr chkTexNormAuto;  void chkTexNormAutoOn(WP);
	
	void btnBrushPreset(WP);

	//  Ter Generator
	SV svTerGenScale, svTerGenOfsX, svTerGenOfsY;
	SV svTerGenFreq, svTerGenOct, svTerGenPers, svTerGenPow;
	SV svTerGenMul, svTerGenOfsH, svTerGenRoadSm;
	SV svTerGenAngMin, svTerGenAngMax, svTerGenAngSm;
	SV svTerGenHMin, svTerGenHMax, svTerGenHSm;
	void slTerGen(SV*);
	
	
	//  Ter HMap
	SlV(TerTriSize);  int UpdTxtTerSize(float mul=1.f);
	MyGUI::EditPtr edTerErrorNorm;  void editTerErrorNorm(MyGUI::EditPtr);
	void btnTerrainNew(WP), btnTerGenerate(WP);
	void btnTerrainHalf(WP), btnTerrainDouble(WP), btnTerrainMove(WP);
	Ogre::String getHMapNew();
	MyGUI::StaticTextPtr valTerLAll;
	
	//  Ter Layer
	int idTerLay;  bool bTerLay;  // help vars
	void sldUpdTerL();
	MyGUI::TabPtr tabsTerLayers; void tabTerLayer(TAB);

	SV svTerLScale;
	SV svTerLAngMin, svTerLHMin, svTerLAngSm;  // blendmap
	SV svTerLAngMax, svTerLHMax, svTerLHSm;
	SV svTerLNoise;  //TerLNoiseOnly
	void slTerLay(SV*);

	//  Ter Particles
	MyGUI::EditPtr edLDust,edLDustS,edLMud,edLSmoke, edLTrlClr;  MyGUI::ImageBox* clrTrail;
	void editLDust(MyGUI::EditPtr), editLTrlClr(MyGUI::EditPtr);
	MyGUI::ComboBoxPtr cmbParDust,cmbParMud,cmbParSmoke;  void comboParDust(CMB);
	
	//  Ter Surfaces
	MyGUI::ComboBoxPtr cmbSurface;  void comboSurface(CMB), UpdSurfInfo();
	MyGUI::StaticTextPtr txtSurfTire, txtSuBumpWave,txtSuBumpAmp, txtSuRollDrag, txtSuFrict, txtSurfType;
	

	///  [Vegetation]  --------------------
	MyGUI::EditPtr edGrassDens,edTreesDens, edGrPage,edGrDist, edTrPage,edTrDist,
		edGrSwayDistr, edGrSwayLen, edGrSwaySpd, edTrRdDist, edTrImpDist,
		edGrDensSmooth, edSceneryId,
		edGrTerMaxAngle,edGrTerSmAngle, edGrTerMinHeight,edGrTerMaxHeight,edGrTerSmHeight;

	MyGUI::ComboBoxPtr cmbGrassMtr;  void comboGrassMtr(CMB);
	MyGUI::ComboBoxPtr cmbGrassClr;  void comboGrassClr(CMB);
	void editTrGr(MyGUI::EditPtr);

	//  3d model view  (veget,objs)
	MyGUI::Canvas* viewCanvas;  wraps::RenderBoxScene viewBox;  MyGUI::IntCoord GetViewSize();
	Ogre::String viewMesh;  void Upd3DView(Ogre::String mesh);  float tiViewUpd;
	
	//  paged layers  ----
	int idPgLay;  // tab
	void sldUpdPgL();
	MyGUI::TabPtr tabsPgLayers;  void tabPgLayers(TAB);

	MyGUI::ButtonPtr chkPgLay;  void chkPgLayOn(WP);  MyGUI::StaticTextPtr valLTrAll;
	MyGUI::ComboBoxPtr cmbPgLay;  void comboPgLay(CMB);

	SV svLTrDens;
	SV svLTrRdDist, svLTrRdDistMax;
	SV svLTrMinSc, svLTrMaxSc;
	SV svLTrWindFx, svLTrWindFy;
	SV svLTrMaxTerAng;

	MyGUI::EditPtr edLTrMinTerH,edLTrMaxTerH,edLTrFlDepth;
	void editLTrMinTerH(MyGUI::EditPtr),editLTrMaxTerH(MyGUI::EditPtr),editLTrFlDepth(MyGUI::EditPtr);

	//  grass layers  ----
	int idGrLay;  // tab
	void sldUpdGrL();
	MyGUI::TabPtr tabsGrLayers;  void tabGrLayers(TAB);

	SV svLGrDens;
	SV svGrMinX, svGrMaxX;
	SV svGrMinY, svGrMaxY;

	MyGUI::ButtonPtr chkGrLay;  void chkGrLayOn(WP);
	MyGUI::StaticImagePtr imgGrass,imgGrClr;  MyGUI::StaticTextPtr valLGrAll;

	
	//  [Road]  ----
	MyGUI::ComboBoxPtr cmbRoadMtr[4],cmbPipeMtr[4],
		cmbRoadWMtr,cmbPipeWMtr,cmbRoadColMtr;
	void comboRoadMtr(CMB),comboPipeMtr(CMB),
		comboRoadWMtr(CMB),comboPipeWMtr(CMB),comboRoadColMtr(CMB);

	MyGUI::EditPtr edRdTcMul,edRdTcMulW,edRdTcMulP,edRdTcMulPW,edRdTcMulC,
		edRdLenDim,edRdWidthSteps,edRdHeightOfs,
		edRdSkirtLen,edRdSkirtH, edRdMergeLen,edRdLodPLen,
		edRdColN,edRdColR, edRdPwsM,edRdPlsM;
	void editRoad(MyGUI::EditPtr);


	///  [Objects]  ----
	ED_OBJ objEd;
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
	SV svAlignWidthAdd, svAlignWidthMul, svAlignSmooth;

	//  [Warnings]  ----
	MyGUI::EditPtr edWarn;  MyGUI::StaticTextPtr txWarn;
	MyGUI::StaticImagePtr imgWarn,imgInfo;
	void WarningsCheck(const class Scene* sc, const SplineRoad* road);
	int cntWarn;  bool logWarn;  // only log warnings (tool)
	enum eWarn {  ERR=0, WARN, INFO, NOTE, TXT  };
	void Warn(eWarn type, Ogre::String text);
	void chkCheckSave(WP),chkCheckLoad(WP);  int iLoadNext;

	//  Tweak
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
