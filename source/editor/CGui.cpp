#include "pch.h"
#include "CApp.h"
#include "CGui.h"
#include "../vdrift/pathmanager.h"


//  ctor
//----------------------------------------------------------------------------------------------------------------------
CGui::CGui(App* app1)  //  gui wigdets--
	: app(app1)
	,mToolTip(0), mToolTipTxt(0)
	,brImg(0), wndTabs(0), bnQuit(0)  // brush
	,chAutoBlendmap(0), chInputBar(0)  // settings
	// sun
	,cmbSky(0), cmbRain1(0),cmbRain2(0)
	// light
	,edLiAmb(0),edLiDiff(0),edLiSpec(0)
	,clrAmb(0),clrDiff(0),clrSpec(0)
	// fog
	, chkFog(0), chkWeather(0)
	,edFogClr(0),edFogClr2(0),edFogClrH(0)
	,clrFog(0),clrFog2(0),clrFogH(0)
	// terrain
	,cmbTexDiff(0),cmbTexNorm(0), imgTexDiff(0)
	,valTerLAll(0),tabsHmap(0),tabsTerLayers(0)
	, idTerLay(0),bTerLay(1)
	,chkTerLay(0)
	,chkTexNormAuto(0), bTexNormAuto(1)
	,chkTerLNoiseOnly(0), chkTerLayTripl(0)
	,edTerErrorNorm(0)
	// ter particles
	,edLDust(0),edLDustS(0)
	,edLMud(0),edLSmoke(0), edLTrlClr(0)
	,cmbParDust(0),cmbParMud(0),cmbParSmoke(0), clrTrail(0)
	// ter surfaces
	,cmbSurface(0)
	,txtSurfTire(0),txtSurfType(0)
	,txtSuBumpWave(0),txtSuBumpAmp(0)
	,txtSuRollDrag(0),txtSuFrict(0)
	// vegetation
	,edGrassDens(0),edTreesDens(0)
	,edGrPage(0),edGrDist(0)
	,edTrPage(0),edTrDist(0)
	,edGrSwayDistr(0), edGrSwayLen(0), edGrSwaySpd(0)
	,edTrRdDist(0), edTrImpDist(0)
	,edGrDensSmooth(0)
	,edGrTerMaxAngle(0),edGrTerSmAngle(0)
	,edGrTerMinHeight(0),edGrTerMaxHeight(0),edGrTerSmHeight(0)
	,edSceneryId(0)
	,cmbGrassMtr(0), cmbGrassClr(0)
	,viewCanvas(0), tiViewUpd(-1.f)
	// paged layers, veget models
	, idPgLay(0), tabsPgLayers(0)
	,chkPgLay(0), valLTrAll(0)
	,cmbPgLay(0)
	,edLTrMinTerH(0),edLTrMaxTerH(0),edLTrFlDepth(0)
	// grass layers
	, idGrLay(0), tabsGrLayers(0)
	,chkGrLay(0), valLGrAll(0)
	,imgGrass(0),imgGrClr(0)
	// road
	,edRdTcMul(0),edRdTcMulW(0)
	,edRdTcMulP(0),edRdTcMulPW(0),edRdTcMulC(0)
	,edRdLenDim(0),edRdWidthSteps(0),edRdHeightOfs(0)
	,edRdSkirtLen(0),edRdSkirtH(0)
	,edRdMergeLen(0),edRdLodPLen(0)
	,edRdColN(0),edRdColR(0)
	,edRdPwsM(0),edRdPlsM(0)
	,cmbRoadWMtr(0),cmbPipeWMtr(0),cmbRoadColMtr(0)
	// tools
	,edScaleAllMul(0),edScaleTerHMul(0)
	// warnings
	, cntWarn(0), logWarn(0), iLoadNext(0)
	,edWarn(0),txWarn(0)
	,imgWarn(0),imgInfo(0)
	// track
	, bListTrackU(0)
	,trkList(0),trkName(0)
	,imgTrkIco1(0),imgTrkIco2(0), edFind(0)
	// gui
	, bGuiReinit(0), bGI(0), resList(0)
	, bTopView(0)
	,noBlendUpd(0)
	// obj
	,iObjCur(-1), iObjTNew(0), iObjLast(0)
	, objSim(0), objEd(EO_Move)
	,objListSt(0),objListDyn(0),objListBld(0), objPan(0)
{
	imgPrv[0]=0; imgMini[0]=0; imgTer[0]=0;  trkDesc[0]=0;
	
	pathTrk[0] = PATHMANAGER::Tracks() + "/";
	pathTrk[1] = PATHMANAGER::TracksUser() + "/";
	/*resTrk = "";*/  strFSerrors = "";

	int i;  // tool texts
	for (i=0; i<BR_TXT; ++i){  brTxt[i]=0;  brVal[i]=0;  brKey[i]=0;  }
	for (i=0; i<RD_TXT; ++i){  rdTxt[i]=0;  rdVal[i]=0;  rdKey[i]=0;  }
	for (i=0; i<RDS_TXT;++i){  rdTxtSt[i]=0;  rdValSt[i]=0;  }

	for (i=0; i<ST_TXT; ++i)  stTxt[i]=0;
	for (i=0; i<FL_TXT; ++i)  flTxt[i]=0;
	for (i=0; i<OBJ_TXT;++i)  objTxt[i]=0;
	for (i=0; i<RI_TXT; ++i)  riTxt[i]=0;
	
	for (i=0; i < StTrk; ++i)  stTrk[0][i] = 0;
	for (i=0; i < 4; ++i)  {  cmbRoadMtr[i]=0;  cmbPipeMtr[i]=0;  }

	sc = app1->sc;
	pSet = app1->pSet;
	data = app->data;
}
