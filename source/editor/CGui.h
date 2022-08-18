#pragma once
#include "BaseApp.h"
#include "../vdrift/tracksurface.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/SliderValue.h"

#include <Ogre.h>
// #include <OgreCommon.h>
// #include <OgreVector3.h>
// #include <OgreString.h>

#include <MyGUI_Types.h>
#include <MyGUI_WidgetToolTip.h>
#include <MyGUI_Enumerator.h>
#include <MyGUI_WidgetDefines.h>  //EnumeratorWidgetPtr
#include <MyGUI_Colour.h>
#include "../ogre/common/MessageBox/MessageBoxStyle.h"


namespace wraps {	class RenderBoxScene;  }
class App;  class SETTINGS;  class CScene;  class Scene;  class CData;  class CGuiCom;


class CGui : public BGui
{
public:
	App* app =0;  SETTINGS* pSet =0;
	CScene* scn =0;  Scene* sc =0;  CData* data =0;
	MyGUI::Gui* mGui =0;  CGuiCom* gcom =0;

	CGui(App* app1);

	typedef std::list<std::string> strlist;


	//  main menu
	void InitMainMenu();
	void btnMainMenu(WP);  void tabMainMenu(Tab tab, size_t id);


	///  Gui
	///-----------------------------------------------------------------------------------------------------------------	
	
	bool bGI = false;  // gui inited  set values
	void InitGui(), GuiUpdate();
	void UpdGuiAfterPreset(), FillPickLists();

	Txt valTrk[2] ={0,0};
	std::vector<Tab> vSubTabsTrack, vSubTabsEdit, vSubTabsHelp, vSubTabsOpts;
	
	//  util
	void toggleGui(bool toggle=false);
	void GuiShortcut(WND_Types wnd, int tab, int subtab=-1), NumTabNext(int rel);
	void btnEdTut(WP);


	//  ed
	void Status(Ogre::String s, float r,float g,float b);
	void SetGuiFromXmls(), SetGuiRoadFromXml();  // update gui controls
	bool noBlendUpd = 0;

	///  mode, status
	Img imgCam =0, imgEdit =0, imgGui =0;
	WP panStatus =0;  Txt txtStatus =0;


	//  clr
	const static MyGUI::Colour sUsedClr[8];
	void SetUsedStr(Txt valUsed, int cnt, int yellowAt);
	
	//  _Tools_
	void ToolTexAlpha(), ToolSceneXml(),
		ToolTracksWarnings(), ToolBrushesPrv(), ToolPresets();
	

	//  tool windows texts  ----
	const static int
		BR_TXT=9, RD_TXT=14, RDS_TXT=11,  //  brush, road, road stats
		ST_TXT=6, FL_TXT=6, OBJ_TXT=6,  //  start, fluids, objects
		EMT_TXT=8;  //  emitters

	Txt	brTxt[BR_TXT], brVal[BR_TXT], brKey[BR_TXT],
		rdTxt[RD_TXT], rdVal[RD_TXT], rdKey[RD_TXT],
		rdTxtSt[RDS_TXT], rdValSt[RDS_TXT],
		stTxt[ST_TXT], flTxt[FL_TXT], objTxt[OBJ_TXT],
		emtTxt[EMT_TXT];
	WP objPan =0;
	Img brImg =0, rdImg[RD_TXT];  Tab wndTabs =0;


	//  [settings]
	Ck ckAutoStart, ckEscQuits;  // startup
	Ck ckStartInMain, ckOgreDialog, ckMouseCapture, ckScreenPng;

	SlV(SizeMinimap);  SlV(SizeRoadP);
	SV svCamSpeed, svCamInert;
	SV svTerUpd, svMiniUpd;

	CK(Minimap);
	void btnSetCam(WP);

	CK(Fps);  CK(Wireframe);  Ck ckAllowSave;
	CK(InputBar);  CK(CamPos);

	//  top view
	void toggleTopView();
	bool bTopView = 0, oldFog = 0;
	Ogre::Vector3 oldPos,oldRot;


	//  Color tool wnd  ----
	Wnd wndColor =0;  WP wpClrSet =0;
	SV svHue,svSat,svVal, svAlp,svNeg;
	void btnClrSet(WP), btnClrSetA(WP), slUpdClr(SV*);
	

	//  [Sky]  ----
	Btn btnSky =0;  // pick
	Cmb cmbRain1 =0, cmbRain2 =0;
	void comboSky(CMB), comboRain1(CMB),comboRain2(CMB);

	SV svRain1Rate, svRain2Rate;
	SV svSunPitch, svSunYaw, svSkyYaw;
	void slUpdSky(SV*), slUpdSun(SV*), slUpdFog(SV*);

	SV svFogStart, svFogEnd;
	SV svFogHStart, svFogHEnd;  // Hfog
	SV svFogHeight, svFogHDensity, svFogHDmg;

	Img clrAmb =0, clrDiff =0, clrSpec =0;
	Img clrFog =0, clrFog2 =0, clrFogH =0;

	CK(Fog);  Ck ckWeather;

	
	///  [Terrain]  --------------------
	//  Ter HMap
	Tab tabsHmap =0;  void tabHmap(TAB);
	void updTabHmap();  int getHMapSizeTab();
	
	//  Ter HMap
	SlV(TerTriSize);  int UpdTxtTerSize(float mul=1.f);
	SV svTerErrorNorm;  void slTerErrorNorm(SV*);
	SV svTerNormScale, svTerSpecPow, svTerSpecPowEm;  void slTerPar(SV*);
	
	Ogre::String getHMapNew();
	void btnTerrainNew(WP), btnTerGenerate(WP);
	void btnTerrainHalf(WP), btnTerrainDouble(WP), btnTerrainMove(WP);  // tools
	
	void btnBrushPreset(WP);
	
	//  Ter Generator
	SV svTerGenScale, svTerGenOfsX, svTerGenOfsY;
	SV svTerGenFreq, svTerGenOct, svTerGenPers, svTerGenPow;
	SV svTerGenMul, svTerGenOfsH, svTerGenRoadSm;
	SV svTerGenAngMin, svTerGenAngMax, svTerGenAngSm;
	SV svTerGenHMin, svTerGenHMax, svTerGenHSm;
	void slTerGen(SV*);
	
	
	//  Ter Layer  ----
	int idTerLay = 0;  // help var
	void SldUpd_TerL();

	Btn btnTexDiff =0;  // pick
	Tab tabsTerLayers =0;  void tabTerLayer(TAB);
	CK(TerLayOn);
	Txt valTerLAll =0, valTriplAll =0;
	void updUsedTer();
	void btnUpdateLayers(WP);

	//  texture
	Cmb cmbTexNorm =0;  void comboTexNorm(CMB);
	Img imgTexDiff =0;
	Ck ckTexNormAuto;  bool bTexNormAuto =1;  // auto norm tex name
	void btnTerLmoveL(WP),btnTerLmoveR(WP);

	//  Ter blendmap
	SV svTerLScale;
	SV svTerLAngMin, svTerLHMin, svTerLAngSm;
	SV svTerLAngMax, svTerLHMax, svTerLHSm;
	//  noise
	SV svTerLNoise, svTerLNprev, svTerLNnext2, svTerLDmg;
	SV svTerLN_Freq[2], svTerLN_Oct[2], svTerLN_Pers[2], svTerLN_Pow[2];
	void slTerLay(SV*), SldUpd_TerLNvis();

	CK(TerLNOnly);  CK(TerLayTripl);
	CK(DebugBlend);  bool bDebugBlend =0;
	Img dbgLclr =0;

	//  noise btns
	Btn bRn1 =0, bRn2 =0;
	void radN1(WP), radN2(WP), btnNpreset(WP);
	void btnNrandom(WP), btnNswap(WP);

	
	//  Ter Particles
	SV svLDust, svLDustS, svLMud, svLSmoke;  Img clrTrail =0;
	void SldUpd_Surf();
	Cmb cmbParDust =0, cmbParMud =0, cmbParSmoke =0;  void comboParDust(CMB);
	
	//  Ter Surfaces
	Cmb cmbSurface =0;  void comboSurface(CMB), UpdSurfInfo();
	Txt txtSurfTire =0, txtSuBumpWave =0, txtSuBumpAmp =0;
	Txt txtSuRollDrag =0, txtSuFrict =0, txtSurfType =0;
	

	///  [Vegetation]  --------------------
	//  global params
	SV svGrassDens, svTreesDens;
	Ed edGrPage =0, edGrDist =0;
	Ed edTrPage =0, edTrDist =0, edTrImpDist =0;
	//  grass
	Ed edGrSwayDistr =0, edGrSwayLen =0, edGrSwaySpd =0;
	void editTrGr(Ed);
	SV svTrRdDist;  SV svGrDensSmooth;

	//  model view 3d  (veget,objs)
	Txt txVHmin =0, txVHmax =0, txVWmin =0, txVWmax =0, txVCnt =0;
	void updVegetInfo();

	Can viewCanvas =0;
	wraps::RenderBoxScene* viewBox =0;  Ogre::Vector3 viewSc;
	MyGUI::IntCoord GetViewSize();
	Ogre::String viewMesh;
	float tiViewUpd =0;
	void Upd3DView(Ogre::String mesh);
	
	///  models (paged layers)  --------
	int idPgLay =0;  // help var
	void SldUpd_PgL();

	Btn btnVeget =0;  // pick
	Tab tabsPgLayers =0;  void tabPgLayers(TAB);
	CK(PgLayOn);  Txt valLTrAll =0;
	void btnUpdateVeget(WP);

	SV svLTrDens;
	SV svLTrRdDist, svLTrRdDistMax;
	SV svLTrMinSc,  svLTrMaxSc;  void slLTrSc(SV*);
	SV svLTrWindFx, svLTrWindFy;
	SV svLTrMaxTerAng;
	SV svLTrMinTerH, svLTrMaxTerH, svLTrFlDepth;

	///  grass layers  --------
	int idGrLay =0;  // help var
	void SldUpd_GrL();

	Btn btnGrassMtr =0;  // pick
	Tab tabsGrLayers =0;  void tabGrLayers(TAB);
	CK(GrLayOn);  Txt valLGrAll =0;
	void btnUpdateGrass(WP);

	SV svLGrDens, svGrChan;
	SV svGrMinX, svGrMaxX;
	SV svGrMinY, svGrMaxY;

	void comboGrassMtr(CMB);
	Cmb cmbGrassClr =0;  void comboGrassClr(CMB);
	Img imgGrass =0, imgGrClr =0;

	///  grass channels  --------
	int idGrChan =0;  // help var
	void SldUpd_GrChan();
	Tab tabsGrChan =0;  void tabGrChan(TAB);

	SV svGrChAngMin, svGrChAngMax, svGrChAngSm;  // ter angle,height
	SV svGrChHMin, svGrChHMax, svGrChHSm, svGrChRdPow;
	SV svGrChNoise, svGrChNfreq, svGrChNoct, svGrChNpers, svGrChNpow;
	
	
	//  [Road]  ----
	//  materials, pick
	enum {  RdPk_Road, RdPk_Pipe, RdPk_Wall, RdPk_PipeW, RdPk_Col
	}	idRdType = RdPk_Road;
	int idRdPick = 0;
	Btn btnRoad[4], btnPipe[4], btnRoadW =0, btnPipeW =0, btnRoadCol =0;
	void btnPickRoad(WP), btnPickPipe(WP), btnPickRoadW(WP), btnPickPipeW(WP), btnPickRoadCol(WP);
	void btnPickRd(WP),  wheelRd(WP, int rel),  listPickRd(Mli2 li, size_t pos);

	//  params
	SV svRdTcMul,svRdTcMulW, svRdTcMulP,svRdTcMulPW, svRdTcMulC;
	SV svRdLenDim,svRdWidthSteps, svRdPwsM,svRdPlsM;
	SV svRdMergeLen,svRdLodPLen, svRdColN,svRdColR;
	SV svRdVisDist,svRdVisBehind;
	void SldUpd_Road();
	Ed edRdSkirtLen =0, edRdSkirtH =0, edRdHeightOfs =0;
	void editRoad(Ed);

	
	//  [Surfaces]  ----
	int idSurf = 0;  // help var
	struct TerLayer* GetTerRdLay();

	Li surfList =0;  void listSurf(Li, size_t);
	void UpdSurfList();
	Ck ckRoad1Mtr;

	
	//  [Game]  ----
	SV svDamage, svWind, svGravity;
	CK(DenyReversed);  CK(TiresAsphalt);  CK(TerrainEmissive);
	CK(NoWrongChks);
	void SldUpd_Game();
	//  sound
	Txt txtRevebDescr =0;
	Cmb cmbReverbs =0;  void comboReverbs(CMB), UpdRevDescr();
	//  info
	Txt txtEdInfo =0;  void UpdEdInfo();
	

	//  [Pacenotes]  ----
	SV svPaceShow, svPaceDist, svPaceSize, svPaceNear, svPaceAlpha;
	void slUpd_Pace(SV*);
	CK(TrkReverse);


	//  [Objects]  ----
	//  gui lists
	Li objListDyn =0, objListSt =0, objListBld =0, objListCat =0;
	void listObjsChng(Li, size_t), listObjsNext(int rel);
	void listObjsCatChng(Li, size_t);
	

	//  [Tools]  ----
	//  copy
	Txt valTrkCpySel;
	void btnTrkCopySel(WP);  bool ChkTrkCopy();
	void btnCopySun(WP), btnCopyTerHmap(WP), btnCopyTerLayers(WP);
	void btnCopyVeget(WP), btnCopyRoad(WP), btnCopyRoadPars(WP);
	//  delete
	void btnDeleteRoad(WP),btnDeleteFluids(WP),btnDeleteObjects(WP),btnDeleteParticles(WP);
	//  scale
	float fScale =1.f, fScaleTer =1.f;
	SV svScaleAllMul, svScaleTerHMul;
	void btnScaleAll(WP), btnScaleTerH(WP);
	//  align
	SV svAlignWidthAdd, svAlignWidthMul, svAlignSmooth;


	//  [Warnings]  ----
	Ed edWarn =0;  Txt txWarn =0;
	Img imgWarn =0, imgInfo =0;
	void WarningsCheck(const class Scene* sc, const SplineRoad* road);

	int cntWarn = 0;  // count
	bool logWarn =0;  // only log warnings (tool)

	enum eWarn {  ERR=0, WARN, INFO, NOTE, TXT  };
	void Warn(eWarn type, Ogre::String text);

	int iLoadNext = 0;
	Ck ckCheckSave, ckCheckLoad;


	//  [Tweak]  ----
	void CreateGUITweakMtr(), slTweak(SL),edTweak(Ed);
	void TweakSetMtrPar(std::string name, float val);  void comboTweakMtr(CMB);


	//  [Pick]  ----
	Ck ckPickSetPar;  WP panPick =0;
	Mli2 liSky =0, liTex =0, liGrs =0, liVeg =0, liRd =0;
	enum EPick { P_Sky=0, P_Tex, P_Grs, P_Veg, P_Rd, P_All };
	float liPickX[P_All];  int liPickW[P_All];  // start pos x, width
	
	void PickShow(EPick n, bool toggleVis=true);
	int liNext(Mli2 li, int rel);  void keyPickNext(int rel);

	void btnPickSky(WP),   wheelSky(WP, int rel), listPickSky(Mli2 li, size_t pos);
	void btnPickTex(WP),   wheelTex(WP, int rel), listPickTex(Mli2 li, size_t pos);
	void btnPickGrass(WP), wheelGrs(WP, int rel), listPickGrs(Mli2 li, size_t pos);
	void btnPickVeget(WP), wheelVeg(WP, int rel), listPickVeg(Mli2 li, size_t pos);



	///  [Track]  ----
	Ogre::String sCopyTrack;  int bCopyTrackU = 0;  // for copy tools
	Ogre::String PathCopyTrk(int user=-1);
	Ogre::String GetListTrk();

	void btnTrackNew(WP), btnTrackRename(WP);
	void btnTrackDel(WP);  // track
	void msgTrackDel(MyGUI::Message* sender, MyGUI::MessageBoxStyle result);
	void btnNewGame(WP);  // load track

	Ed trkName =0;  void editTrkDescr(Ed), editTrkAdvice(Ed);
	

	//  system, utils
	Ogre::String strFSerrors;
	bool Rename(Ogre::String from, Ogre::String to);
	bool Delete(Ogre::String file), DeleteDir(Ogre::String dir);
	bool CreateDir(Ogre::String dir);
	bool Copy(Ogre::String file, Ogre::String to);

	std::vector<Ogre::String> vsMaterials;
	void GetMaterials(   Ogre::String filename, bool clear=true, Ogre::String type="material");  // ogre resource
	void GetMaterialsMat(Ogre::String filename, bool clear=true, Ogre::String type="material");  // direct path+file
};
