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
class App;  class SETTINGS;  class Scene;  class CData;  class CGuiCom;


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
	std::vector<Tab> vSubTabsTrack, vSubTabsEdit, vSubTabsHelp, vSubTabsOpts;
	
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
	//  Ter HMap
	Tab tabsHmap;  void tabHmap(TAB);
	void updTabHmap();  int getHMapSizeTab();
	
	//  Ter HMap
	SlV(TerTriSize);  int UpdTxtTerSize(float mul=1.f);
	Ed edTerErrorNorm;  void editTerErrorNorm(Ed);
	
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
	int idTerLay;  bool bTerLay;  // help vars
	void SldUpd_TerL();
	Tab tabsTerLayers;  void tabTerLayer(TAB);

	CK(TerLayOn);  Txt valTerLAll;

	//  texture
	Cmb cmbTexDiff, cmbTexNorm;
	void comboTexDiff(CMB), comboTexNorm(CMB);
	Img imgTexDiff;
	Ck ckTexNormAuto;  bool bTexNormAuto;  // auto norm tex name

	//  Ter blendmap
	SV svTerLScale;
	SV svTerLAngMin, svTerLHMin, svTerLAngSm;
	SV svTerLAngMax, svTerLHMax, svTerLHSm;
	SV svTerLNoise;
	void slTerLay(SV*);
	CK(TerLNoiseOnly);  CK(TerLayTripl);

	//  Ter Particles
	Ed edLDust,edLDustS,edLMud,edLSmoke, edLTrlClr;  Img clrTrail;
	void editLDust(Ed), editLTrlClr(Ed);
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
	SV svGrTerMaxAngle,  svGrTerSmAngle;
	SV svGrTerMinHeight, svGrTerMaxHeight, svGrTerSmHeight;

	SV svTrRdDist;  SV svGrDensSmooth;  Ed edSceneryId;

	//  model view 3d  (veget,objs)
	Can viewCanvas;
	wraps::RenderBoxScene* viewBox;
	MyGUI::IntCoord GetViewSize();
	Ogre::String viewMesh;
	float tiViewUpd;
	void Upd3DView(Ogre::String mesh);
	
	///  paged layers  --------
	int idPgLay;  // tab
	void SldUpd_PgL();
	Tab tabsPgLayers;  void tabPgLayers(TAB);

	CK(PgLayOn);  Txt valLTrAll;
	Cmb cmbPgLay;  void comboPgLay(CMB);

	SV svLTrDens;
	SV svLTrRdDist, svLTrRdDistMax;
	SV svLTrMinSc,  svLTrMaxSc;
	SV svLTrWindFx, svLTrWindFy;
	SV svLTrMaxTerAng;
	SV svLTrMinTerH, svLTrMaxTerH, svLTrFlDepth;

	///  grass layers  --------
	int idGrLay;  // tab
	void SldUpd_GrL();
	Tab tabsGrLayers;  void tabGrLayers(TAB);

	CK(GrLayOn);  Txt valLGrAll;
	SV svLGrDens;
	SV svGrMinX, svGrMaxX;
	SV svGrMinY, svGrMaxY;

	Cmb cmbGrassMtr;  void comboGrassMtr(CMB);
	Cmb cmbGrassClr;  void comboGrassClr(CMB);
	Img imgGrass,imgGrClr;

	
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


	//  [Objects]  ----
	//  gui lists
	Li objListDyn, objListSt, objListRck, objListBld;
	void listObjsChng(Li, size_t);
	

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
