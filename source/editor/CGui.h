#pragma once
#include "BaseApp.h"
#include "../vdrift/tracksurface.h"
#include "../vdrift/track.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/SliderValue.h"

#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreString.h>

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
	App* app;  SETTINGS* pSet;
	CScene* scn;  Scene* sc;  CData* data;
	MyGUI::Gui* mGui;  CGuiCom* gcom;

	CGui(App* app1);

	typedef std::list <std::string> strlist;


	///  Gui
	///-----------------------------------------------------------------------------------------------------------------	
	
	bool bGI;  // gui inited  set values
	void InitGui(), GuiUpdate();
	void UpdGuiAfterPreset();

	Txt valTrk[2];
	std::vector<Tab> vSubTabsTrack, vSubTabsEdit, vSubTabsHelp, vSubTabsOpts;
	
	//  util
	void toggleGui(bool toggle=false);
	void GuiShortcut(WND_Types wnd, int tab, int subtab=-1), NumTabNext(int rel);


	//  ed
	void Status(Ogre::String s, float r,float g,float b);
	void SetGuiFromXmls();  // update gui controls
	bool noBlendUpd;

	///  mode, status
	Img imgCam, imgEdit, imgGui;
	WP panStatus;  Txt txtStatus;


	//  clr
	const static MyGUI::Colour sUsedClr[8];
	void SetUsedStr(Txt valUsed, int cnt, int yellowAt);
	
	//  _Tools_
	void ToolTexAlpha(), ToolSceneXml(),
		ToolTracksWarnings(), ToolBrushesPrv(), ToolPresets();
	

	//  tool windows texts
	const static int MAX_TXT=12,
		BR_TXT=9, RD_TXT=12, RDS_TXT=11,  //  brush, road, road stats
		ST_TXT=6, FL_TXT=6, OBJ_TXT=6, RI_TXT=6;  //  start, fluids, objects,

	Txt	brTxt[BR_TXT], brVal[BR_TXT], brKey[BR_TXT],
		rdTxt[RD_TXT], rdVal[RD_TXT], rdKey[RD_TXT],
		rdTxtSt[RDS_TXT], rdValSt[RDS_TXT],
		stTxt[ST_TXT], flTxt[FL_TXT], objTxt[OBJ_TXT], riTxt[RI_TXT];
	WP objPan;
	Img brImg;  Tab wndTabs;


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
	bool bTopView, oldFog;
	Ogre::Vector3 oldPos,oldRot;


	//  Color tool wnd  ----
	Wnd wndColor;  WP wpClrSet;
	SV svHue,svSat,svVal, svAlp,svNeg;
	void btnClrSet(WP), btnClrSetA(WP), slUpdClr(SV*);
	

	//  [Sky]  ----
	Btn btnSky;  // pick
	Cmb cmbRain1,cmbRain2;
	void comboSky(CMB), comboRain1(CMB),comboRain2(CMB);

	SV svRain1Rate, svRain2Rate;
	SV svSunPitch, svSunYaw, svSkyYaw;
	void slUpdSky(SV*), slUpdSun(SV*), slUpdFog(SV*);

	SV svFogStart, svFogEnd;
	SV svFogHStart, svFogHEnd;  // Hfog
	SV svFogHeight, svFogHDensity, svFogHDmg;

	Img clrAmb, clrDiff, clrSpec, clrFog, clrFog2, clrFogH;

	CK(Fog);  Ck ckWeather;

	
	///  [Terrain]  --------------------
	//  Ter HMap
	Tab tabsHmap;  void tabHmap(TAB);
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
	int idTerLay;  // help var
	void SldUpd_TerL();

	Btn btnTexDiff;  // pick
	Tab tabsTerLayers;  void tabTerLayer(TAB);
	CK(TerLayOn);
	Txt valTerLAll,valTriplAll;
	void updUsedTer();
	void btnUpdateLayers(WP);

	//  texture
	Cmb cmbTexNorm;  void comboTexNorm(CMB);
	Img imgTexDiff;
	Ck ckTexNormAuto;  bool bTexNormAuto;  // auto norm tex name
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
	CK(DebugBlend);  bool bDebugBlend;
	Img dbgLclr;

	//  noise btns
	Btn bRn1, bRn2;
	void radN1(WP), radN2(WP), btnNpreset(WP);
	void btnNrandom(WP), btnNswap(WP);

	
	//  Ter Particles
	SV svLDust,svLDustS,svLMud,svLSmoke;  Img clrTrail;
	void SldUpd_Surf();
	Cmb cmbParDust,cmbParMud,cmbParSmoke;  void comboParDust(CMB);
	
	//  Ter Surfaces
	Cmb cmbSurface;  void comboSurface(CMB), UpdSurfInfo();
	Txt txtSurfTire, txtSuBumpWave,txtSuBumpAmp, txtSuRollDrag, txtSuFrict, txtSurfType;
	

	///  [Vegetation]  --------------------
	//  global params
	SV svGrassDens, svTreesDens;
	Ed edGrPage,edGrDist;
	Ed edTrPage,edTrDist, edTrImpDist;
	//  grass
	Ed edGrSwayDistr, edGrSwayLen, edGrSwaySpd;
	void editTrGr(Ed);
	SV svTrRdDist;  SV svGrDensSmooth;

	//  model view 3d  (veget,objs)
	Txt txVHmin,txVHmax,txVWmin,txVWmax, txVCnt;
	void updVegetInfo();

	Can viewCanvas;
	wraps::RenderBoxScene* viewBox;  Ogre::Vector3 viewSc;
	MyGUI::IntCoord GetViewSize();
	Ogre::String viewMesh;
	float tiViewUpd;
	void Upd3DView(Ogre::String mesh);
	
	///  models (paged layers)  --------
	int idPgLay;  // help var
	void SldUpd_PgL();

	Btn btnVeget;  // pick
	Tab tabsPgLayers;  void tabPgLayers(TAB);
	CK(PgLayOn);  Txt valLTrAll;
	void btnUpdateVeget(WP);

	SV svLTrDens;
	SV svLTrRdDist, svLTrRdDistMax;
	SV svLTrMinSc,  svLTrMaxSc;  void slLTrSc(SV*);
	SV svLTrWindFx, svLTrWindFy;
	SV svLTrMaxTerAng;
	SV svLTrMinTerH, svLTrMaxTerH, svLTrFlDepth;

	///  grass layers  --------
	int idGrLay;  // help var
	void SldUpd_GrL();

	Btn btnGrassMtr;  // pick
	Tab tabsGrLayers;  void tabGrLayers(TAB);
	CK(GrLayOn);  Txt valLGrAll;
	void btnUpdateGrass(WP);

	SV svLGrDens, svGrChan;
	SV svGrMinX, svGrMaxX;
	SV svGrMinY, svGrMaxY;

	void comboGrassMtr(CMB);
	Cmb cmbGrassClr;  void comboGrassClr(CMB);
	Img imgGrass,imgGrClr;

	///  grass channels  --------
	int idGrChan;  // help var
	void SldUpd_GrChan();
	Tab tabsGrChan;  void tabGrChan(TAB);

	SV svGrChAngMin, svGrChAngMax, svGrChAngSm;  // ter angle,height
	SV svGrChHMin, svGrChHMax, svGrChHSm, svGrChRdPow;
	SV svGrChNoise, svGrChNfreq, svGrChNoct, svGrChNpers, svGrChNpow;
	
	
	//  [Road]  ----
	//  materials
	int idRdPick;
	Btn btnRoad[4];  // pick
	Cmb cmbPipeMtr[4],
		cmbRoadWMtr, cmbPipeWMtr, cmbRoadColMtr;
	void comboPipeMtr(CMB),
		comboRoadWMtr(CMB), comboPipeWMtr(CMB), comboRoadColMtr(CMB);
	//  params
	SV svRdTcMul,svRdTcMulW, svRdTcMulP,svRdTcMulPW, svRdTcMulC;
	SV svRdLenDim,svRdWidthSteps, svRdPwsM,svRdPlsM;
	SV svRdMergeLen,svRdLodPLen, svRdColN,svRdColR;
	void SldUpd_Road();
	Ed edRdSkirtLen,edRdSkirtH, edRdHeightOfs;
	void editRoad(Ed);

	
	//  [Surfaces]  ----
	int idSurf;  // help var
	struct TerLayer* GetTerRdLay();

	Li surfList;  void listSurf(Li, size_t);
	void UpdSurfList();
	Ck ckRoad1Mtr;

	
	//  [Game]  ----
	SV svDamage, svWind, svGravity;
	CK(DenyReversed);  CK(TiresAsphalt);  CK(TerrainEmissive);
	CK(NoWrongChks);
	void SldUpd_Game();
	//  sound
	Txt txtRevebDescr;
	Cmb cmbReverbs;  void comboReverbs(CMB), UpdRevDescr();
	//  info
	Txt txtEdInfo;  void UpdEdInfo();
	

	//  [Pacenotes]  ----
	SV svPaceShow, svPaceDist, svPaceSize, svPaceNear, svPaceAlpha;
	void slUpd_Pace(SV*);
	CK(TrkReverse);


	//  [Objects]  ----
	//  gui lists
	Li objListDyn, objListSt, objListRck, objListBld, objListCat;
	void listObjsChng(Li, size_t), listObjsNext(int rel);
	void listObjsCatChng(Li, size_t);
	

	//  [Tools]  ----
	//  copy
	Txt valTrkCpySel;
	void btnTrkCopySel(WP);  bool ChkTrkCopy();
	void btnCopySun(WP), btnCopyTerHmap(WP), btnCopyTerLayers(WP);
	void btnCopyVeget(WP), btnCopyRoad(WP), btnCopyRoadPars(WP);
	//  delete
	void btnDeleteRoad(WP),btnDeleteFluids(WP),btnDeleteObjects(WP);
	//  scale
	float fScale, fScaleTer;
	SV svScaleAllMul, svScaleTerHMul;
	void btnScaleAll(WP), btnScaleTerH(WP);
	//  align
	SV svAlignWidthAdd, svAlignWidthMul, svAlignSmooth;


	//  [Warnings]  ----
	Ed edWarn;  Txt txWarn;
	Img imgWarn,imgInfo;
	void WarningsCheck(const class Scene* sc, const SplineRoad* road);

	int cntWarn;  // count
	bool logWarn;  // only log warnings (tool)

	enum eWarn {  ERR=0, WARN, INFO, NOTE, TXT  };
	void Warn(eWarn type, Ogre::String text);

	int iLoadNext;
	Ck ckCheckSave, ckCheckLoad;


	//  [Tweak]  ----
	void CreateGUITweakMtr(), slTweak(SL),edTweak(Ed);
	void TweakSetMtrPar(std::string name, float val);  void comboTweakMtr(CMB);


	//  [Pick]  ----
	Ck ckPickSetPar;  WP panPick;
	Mli2 liSky, liTex, liGrs, liVeg, liRd;
	enum EPick { P_Sky=0, P_Tex, P_Grs, P_Veg, P_Rd, P_All };
	int liPickW[P_All];
	void PickShow(EPick n, bool toggleVis=true);
	int liNext(Mli2 li, int rel);  void keyPickNext(int rel);

	void btnPickSky(WP),   wheelSky(WP, int rel), listPickSky(Mli2 li, size_t pos);
	void btnPickTex(WP),   wheelTex(WP, int rel), listPickTex(Mli2 li, size_t pos);
	void btnPickGrass(WP), wheelGrs(WP, int rel), listPickGrs(Mli2 li, size_t pos);
	void btnPickVeget(WP), wheelVeg(WP, int rel), listPickVeg(Mli2 li, size_t pos);
	void btnPickRoad(WP),  wheelRd(WP, int rel),  listPickRd(Mli2 li, size_t pos);



	///  [Track]  ----
	Ogre::String sCopyTrack;  int bCopyTrackU;  // for copy tools
	Ogre::String PathCopyTrk(int user=-1);
	Ogre::String GetListTrk();

	void btnTrackNew(WP), btnTrackRename(WP);
	void btnTrackDel(WP);  // track
	void msgTrackDel(MyGUI::Message* sender, MyGUI::MessageBoxStyle result);
	void btnNewGame(WP);  // load track

	Ed trkName;  void editTrkDesc(Ed);
	

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
