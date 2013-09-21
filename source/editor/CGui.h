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
	
	//  main menu
	void toggleGui(bool toggle=false);
	void GuiShortcut(WND_Types wnd, int tab, int subtab=-1), NumTabNext(int rel);
	void MainMenuBtn(WP), MenuTabChg(Tab, size_t);

	//  ed
	void Status(Ogre::String s, float r,float g,float b);
	void SetGuiFromXmls();
	bool noBlendUpd;

	//  clr
	const static MyGUI::Colour sUsedClr[8];
	void SetUsedStr(Txt valUsed, int cnt, int yellowAt);
	
	//  _Tools_
	void ToolTexAlpha(), ToolSceneXml(), ToolListSceneryID(), ToolTracksWarnings(), ToolBrushesPrv();	


	//  tool windows texts
	const static int
		BR_TXT=9, RD_TXT=11, RDS_TXT=9,
		ST_TXT=6, FL_TXT=6, OBJ_TXT=6, RI_TXT=6;

	Txt	brTxt[BR_TXT],brVal[BR_TXT],brKey[BR_TXT],
		rdTxt[RD_TXT],rdVal[RD_TXT],rdKey[RD_TXT],
		rdTxtSt[RDS_TXT],rdValSt[RDS_TXT],
		stTxt[ST_TXT],  flTxt[FL_TXT], objTxt[OBJ_TXT], riTxt[RI_TXT];
	WP objPan;
	Img brImg;  Tab wndTabs;


	//  [settings]
	void chkAutoStart(WP), chkEscQuits(WP);  // startup
	void chkStartInMain(WP), chkOgreDialog(WP);

	SlV(SizeMinmap);  SlV(SizeRoadP);
	SV svCamSpeed, svCamInert;
	SV svTerUpd, svMiniUpd;

	void chkMinimap(WP), btnSetCam(WP);
	void chkAutoBlendmap(WP);  Btn chAutoBlendmap, chInputBar;
	void chkCamPos(WP), chkInputBar(WP);

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

	void chkFogDisable(WP), chkWeatherDisable(WP);
	Btn chkFog, chkWeather;

	
	///  [Terrain]  --------------------
	Cmb cmbTexDiff, cmbTexNorm;
	void comboTexDiff(CMB), comboTexNorm(CMB);
	Img imgTexDiff;

	Btn chkTerLay, chkTerLNoiseOnly, chkTerLayTripl;
	void chkTerLayOn(WP), chkTerLNoiseOnlyOn(WP), chkTerLayTriplOn(WP);  // on
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
	void btnTerrainNew(WP), btnTerGenerate(WP);
	void btnTerrainHalf(WP), btnTerrainDouble(WP), btnTerrainMove(WP);
	Ogre::String getHMapNew();
	Txt valTerLAll;
	
	//  Ter Layer
	int idTerLay;  bool bTerLay;  // help vars
	void sldUpdTerL();
	Tab tabsTerLayers; void tabTerLayer(TAB);

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
	Ed edGrassDens,edTreesDens, edGrPage,edGrDist, edTrPage,edTrDist,
		edGrSwayDistr, edGrSwayLen, edGrSwaySpd, edTrRdDist, edTrImpDist,
		edGrDensSmooth, edSceneryId,
		edGrTerMaxAngle,edGrTerSmAngle, edGrTerMinHeight,edGrTerMaxHeight,edGrTerSmHeight;

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
	
	//  paged layers  ----
	int idPgLay;  // tab
	void sldUpdPgL();
	Tab tabsPgLayers;  void tabPgLayers(TAB);

	Btn chkPgLay;  void chkPgLayOn(WP);  Txt valLTrAll;
	Cmb cmbPgLay;  void comboPgLay(CMB);

	SV svLTrDens;
	SV svLTrRdDist, svLTrRdDistMax;
	SV svLTrMinSc, svLTrMaxSc;
	SV svLTrWindFx, svLTrWindFy;
	SV svLTrMaxTerAng;

	Ed edLTrMinTerH, edLTrMaxTerH, edLTrFlDepth;
	void editLTrMinTerH(Ed), editLTrMaxTerH(Ed), editLTrFlDepth(Ed);

	//  grass layers  ----
	int idGrLay;  // tab
	void sldUpdGrL();
	Tab tabsGrLayers;  void tabGrLayers(TAB);

	SV svLGrDens;
	SV svGrMinX, svGrMaxX;
	SV svGrMinY, svGrMaxY;

	Btn chkGrLay;  void chkGrLayOn(WP);
	Img imgGrass,imgGrClr;  Txt valLGrAll;

	
	//  [Road]  ----
	Cmb cmbRoadMtr[4], cmbPipeMtr[4],
		cmbRoadWMtr, cmbPipeWMtr, cmbRoadColMtr;
	void comboRoadMtr(CMB), comboPipeMtr(CMB),
		comboRoadWMtr(CMB), comboPipeWMtr(CMB), comboRoadColMtr(CMB);

	Ed edRdTcMul,edRdTcMulW,edRdTcMulP,edRdTcMulPW,edRdTcMulC,
		edRdLenDim,edRdWidthSteps,edRdHeightOfs,
		edRdSkirtLen,edRdSkirtH, edRdMergeLen,edRdLodPLen,
		edRdColN,edRdColR, edRdPwsM,edRdPlsM;
	void editRoad(Ed);


	///  [Objects]  ----
	ED_OBJ objEd;
	std::vector<std::string> vObjNames;
	void SetObjNewType(int tnew),UpdObjNewNode(), AddNewObj();
	int iObjCur,iObjLast, iObjTNew;  std::set<int> vObjSel;
	bool objSim;  Object objNew;
	Li objListDyn, objListSt, objListBld;  void listObjsChng(Li, size_t);
	

	//  [Tools]  ----
	Txt valTrkCpySel;
	void btnTrkCopySel(WP);  bool ChkTrkCopy();
	void btnCopySun(WP), btnCopyTerHmap(WP), btnCopyTerLayers(WP),
		btnCopyVeget(WP), btnCopyRoad(WP), btnCopyRoadPars(WP);
	void btnScaleAll(WP),btnScaleTerH(WP), btnDeleteRoad(WP),btnDeleteFluids(WP),btnDeleteObjects(WP);
	Ed edScaleAllMul;  void editScaleAllMul(Ed);
	Ed edScaleTerHMul;  void editScaleTerHMul(Ed);
	SV svAlignWidthAdd, svAlignWidthMul, svAlignSmooth;

	//  [Warnings]  ----
	Ed edWarn;  Txt txWarn;
	Img imgWarn,imgInfo;
	void WarningsCheck(const class Scene* sc, const SplineRoad* road);
	int cntWarn;  bool logWarn;  // only log warnings (tool)
	enum eWarn {  ERR=0, WARN, INFO, NOTE, TXT  };
	void Warn(eWarn type, Ogre::String text);
	void chkCheckSave(WP),chkCheckLoad(WP);  int iLoadNext;

	//  Tweak
	void CreateGUITweakMtr(), slTweak(SL),edTweak(Ed);
	void TweakSetMtrPar(std::string name, float val);  void comboTweakMtr(CMB);


	//  [Track]  ----
	Ogre::String sListTrack;  int bListTrackU;
	Ogre::String sCopyTrack;  int bCopyTrackU;  // for tools
	Ogre::String PathCopyTrk(int user=-1);

	void btnTrackNew(WP),btnTrackRename(WP),btnTrackDel(WP),  // track
		msgTrackDel(MyGUI::Message* sender, MyGUI::MessageBoxStyle result);
	void btnNewGame(WP);

	Ed trkName;  void editTrkDesc(Ed);
	


	//  system, utils
	Ogre::String strFSerrors;
	bool Rename(Ogre::String from, Ogre::String to), Delete(Ogre::String file), DeleteDir(Ogre::String dir),
		 CreateDir(Ogre::String dir), Copy(Ogre::String file, Ogre::String to);

	std::vector<Ogre::String> vsMaterials;
	//void GetMaterialsFromDef(Ogre::String filename, bool clear=true);
	void GetMaterials(   Ogre::String filename, bool clear=true, Ogre::String type="material");  // ogre resource
	void GetMaterialsMat(Ogre::String filename, bool clear=true, Ogre::String type="material");  // direct path+file
};
