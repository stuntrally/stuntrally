#pragma once
#include "BaseApp.h"
#include "../vdrift/tracksurface.h"
#include "../vdrift/track.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/SliderValue.h"
#include "../ogre/common/data/SceneXml.h"  //Object-

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
class App;  class SETTINGS;  class Scene;  class CData;  class CGuiCom;

enum ED_OBJ {  EO_Move=0, EO_Rotate, EO_Scale  };


class CGui : public BGui
{
public:
	App* app;  SETTINGS* pSet;  Scene* sc;  CData* data;
	MyGUI::Gui* mGui;  CGuiCom* gcom;

	CGui(App* app1);

	typedef std::list <std::string> strlist;


	///  Gui
	///-----------------------------------------------------------------------------------------------------------------	
	
	bool bGI;  // gui inited  set values
	void InitGui(), GuiUpdate();
	void UpdGuiAfterPreset();

	Txt valTrk[2];
	std::vector<Tab> vSubTabsEdit, vSubTabsHelp, vSubTabsOpts;
	
	//  util
	void toggleGui(bool toggle=false);
	void GuiShortcut(WND_Types wnd, int tab, int subtab=-1), NumTabNext(int rel);

	//  ed
	void Status(Ogre::String s, float r,float g,float b);
	void SetGuiFromXmls();  // update gui controls
	bool noBlendUpd;

	//  clr
	const static MyGUI::Colour sUsedClr[8];
	void SetUsedStr(Txt valUsed, int cnt, int yellowAt);
	
	//  _Tools_
	void ToolTexAlpha(), ToolSceneXml(), ToolListSceneryID(), ToolTracksWarnings(), ToolBrushesPrv();	


	//  tool windows texts
	const static int MAX_TXT=11,
		BR_TXT=9, RD_TXT=11, RDS_TXT=9,  //  brush, road, road stats
		ST_TXT=6, FL_TXT=6, OBJ_TXT=6, RI_TXT=6;  //  start, fluids, objects,

	Txt	brTxt[BR_TXT], brVal[BR_TXT], brKey[BR_TXT],
		rdTxt[RD_TXT], rdVal[RD_TXT], rdKey[RD_TXT],
		rdTxtSt[RDS_TXT], rdValSt[RDS_TXT],
		stTxt[ST_TXT], flTxt[FL_TXT], objTxt[OBJ_TXT], riTxt[RI_TXT];
	WP objPan;
	Img brImg;  Tab wndTabs;


	//  [settings]
	Ck ckAutoStart, ckEscQuits;  // startup
	Ck ckStartInMain, ckOgreDialog;

	SlV(SizeMinimap);  SlV(SizeRoadP);
	SV svCamSpeed, svCamInert;
	SV svTerUpd, svMiniUpd;

	CK(Minimap);
	void btnSetCam(WP);
	Ck ckAutoBlendmap;

	CK(InputBar);  CK(CamPos);
	CK(Wireframe);

	//  top view
	void toggleTopView();
	bool bTopView, oldFog;
	Ogre::Vector3 oldPos,oldRot;


	//  [Sky]  ----
	Cmb cmbSky, cmbRain1,cmbRain2;
	void comboSky(CMB), comboRain1(CMB),comboRain2(CMB);

	SV svRain1Rate, svRain2Rate;
	SV svSunPitch, svSunYaw;
	void slUpdSun(SV*), slUpdFog(SV*);

	SV svFogStart, svFogEnd;
	SV svFogHStart, svFogHEnd;  // Hfog
	SV svFogHeight, svFogHDensity;

	Ed edLiAmb,edLiDiff,edLiSpec, edFogClr,edFogClr2,edFogClrH;
	Img clrAmb, clrDiff, clrSpec, clrFog, clrFog2, clrFogH;

	void editFogClr(Ed), editFogClr2(Ed), editFogClrH(Ed);
	void editLiAmb(Ed), editLiDiff(Ed), editLiSpec(Ed);

	CK(Fog);  Ck ckWeather;

	
	///  [Terrain]  --------------------
	Cmb cmbTexDiff, cmbTexNorm;
	void comboTexDiff(CMB), comboTexNorm(CMB);
	Img imgTexDiff;

	Btn chkTerLay, chkTerLNoiseOnly, chkTerLayTripl;  // on
	void chkTerLayOn(WP), chkTerLNoiseOnlyOn(WP), chkTerLayTriplOn(WP);
	
	//  HMap tab
	Tab tabsHmap;  void tabHmap(TAB);
	void updTabHmap();  int getHMapSizeTab();
	
	bool bTexNormAuto;  // auto norm tex
	Btn chkTexNormAuto;  void chkTexNormAutoOn(WP);
	
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
	Ed edTerErrorNorm;  void editTerErrorNorm(Ed);

	Ogre::String getHMapNew();
	void btnTerrainNew(WP), btnTerGenerate(WP);
	void btnTerrainHalf(WP), btnTerrainDouble(WP), btnTerrainMove(WP);  // tools
	
	//  Ter Layer
	int idTerLay;  bool bTerLay;  // help vars
	void sldUpdTerL();
	Tab tabsTerLayers; void tabTerLayer(TAB);

	Txt valTerLAll;
	SV svTerLScale;
	SV svTerLAngMin, svTerLHMin, svTerLAngSm;  // blendmap
	SV svTerLAngMax, svTerLHMax, svTerLHSm;
	SV svTerLNoise;  //TerLNoiseOnly
	void slTerLay(SV*);

	//  Ter Particles
	Ed edLDust,edLDustS,edLMud,edLSmoke, edLTrlClr;  Img clrTrail;
	void editLDust(Ed), editLTrlClr(Ed);
	Cmb cmbParDust,cmbParMud,cmbParSmoke;  void comboParDust(CMB);
	
	//  Ter Surfaces
	Cmb cmbSurface;  void comboSurface(CMB), UpdSurfInfo();
	Txt txtSurfTire, txtSuBumpWave,txtSuBumpAmp, txtSuRollDrag, txtSuFrict, txtSurfType;
	

	///  [Vegetation]  --------------------
	//  global params
	Ed edGrassDens,edTreesDens;
	Ed edGrPage,edGrDist, edTrPage,edTrDist;
	Ed edTrRdDist, edTrImpDist;
	//  grass
	Ed edGrSwayDistr, edGrSwayLen, edGrSwaySpd,
		edGrDensSmooth, edSceneryId,
		edGrTerMaxAngle,edGrTerSmAngle,
		edGrTerMinHeight,edGrTerMaxHeight,edGrTerSmHeight;

	Cmb cmbGrassMtr;  void comboGrassMtr(CMB);
	Cmb cmbGrassClr;  void comboGrassClr(CMB);
	void editTrGr(Ed);

	//  model view 3d  (veget,objs)
	Can viewCanvas;
	wraps::RenderBoxScene* viewBox;
	MyGUI::IntCoord GetViewSize();
	Ogre::String viewMesh;
	float tiViewUpd;
	void Upd3DView(Ogre::String mesh);
	
	///  paged layers  --------
	int idPgLay;  // tab
	void sldUpdPgL();
	Tab tabsPgLayers;  void tabPgLayers(TAB);

	Btn chkPgLay;  void chkPgLayOn(WP);  Txt valLTrAll;
	Cmb cmbPgLay;  void comboPgLay(CMB);

	SV svLTrDens;
	SV svLTrRdDist, svLTrRdDistMax;
	SV svLTrMinSc,  svLTrMaxSc;
	SV svLTrWindFx, svLTrWindFy;
	SV svLTrMaxTerAng;

	Ed edLTrMinTerH, edLTrMaxTerH, edLTrFlDepth;
	void editLTrMinTerH(Ed), editLTrMaxTerH(Ed), editLTrFlDepth(Ed);

	///  grass layers  --------
	int idGrLay;  // tab
	void sldUpdGrL();
	Tab tabsGrLayers;  void tabGrLayers(TAB);

	SV svLGrDens;
	SV svGrMinX, svGrMaxX;
	SV svGrMinY, svGrMaxY;

	Btn chkGrLay;  void chkGrLayOn(WP);
	Img imgGrass,imgGrClr;  Txt valLGrAll;

	
	//  [Road]  ----
	//  materials
	Cmb cmbRoadMtr[4], cmbPipeMtr[4],
		cmbRoadWMtr, cmbPipeWMtr, cmbRoadColMtr;
	void comboRoadMtr(CMB), comboPipeMtr(CMB),
		comboRoadWMtr(CMB), comboPipeWMtr(CMB), comboRoadColMtr(CMB);
	//  params
	Ed edRdTcMul,edRdTcMulW,edRdTcMulP,edRdTcMulPW,edRdTcMulC,
		edRdLenDim,edRdWidthSteps,edRdHeightOfs,
		edRdSkirtLen,edRdSkirtH, edRdMergeLen,edRdLodPLen,
		edRdColN,edRdColR, edRdPwsM,edRdPlsM;
	void editRoad(Ed);


	///  [Objects]  ----
	ED_OBJ objEd;  // edit mode
	std::vector<std::string> vObjNames;
	void SetObjNewType(int tnew), UpdObjNewNode();
	void AddNewObj();
	int iObjCur,iObjLast, iObjTNew;  std::set<int> vObjSel;
	bool objSim;  Object objNew;  //Object*..
	//  lists
	Li objListDyn, objListSt, objListBld;  void listObjsChng(Li, size_t);
	

	//  [Tools]  ----
	//  copy
	Txt valTrkCpySel;
	void btnTrkCopySel(WP);  bool ChkTrkCopy();
	void btnCopySun(WP), btnCopyTerHmap(WP), btnCopyTerLayers(WP);
	void btnCopyVeget(WP), btnCopyRoad(WP), btnCopyRoadPars(WP);
	//  delete
	void btnDeleteRoad(WP),btnDeleteFluids(WP),btnDeleteObjects(WP);
	//  scale
	Ed edScaleAllMul;  void editScaleAllMul(Ed);
	Ed edScaleTerHMul; void editScaleTerHMul(Ed);
	void btnScaleAll(WP),btnScaleTerH(WP);
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



	//  [Track]  ----
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
