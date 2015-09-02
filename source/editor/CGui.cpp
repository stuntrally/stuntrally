#include "pch.h"
#include "CApp.h"
#include "CGui.h"
#include "../ogre/common/CScene.h"
#include "../vdrift/pathmanager.h"


//  ctor
//--------------------------------------------------------------------------
CGui::CGui(App* app1)  //  gui wigdets--
	: app(app1), gcom(0)
	,bGI(0)
	,brImg(0), wndTabs(0)  // brush
	//  mode
	,imgCam(0), imgEdit(0), imgGui(0)
	,panStatus(0), txtStatus(0)
	// sun
	,btnSky(0),cmbRain1(0),cmbRain2(0)
	// light
	,clrAmb(0),clrDiff(0),clrSpec(0)
	// fog
	,clrFog(0),clrFog2(0),clrFogH(0)
	// terrain
	,cmbTexNorm(0), imgTexDiff(0)
	,valTerLAll(0),tabsHmap(0),tabsTerLayers(0),valTriplAll(0)
	, idTerLay(0)
	,bTexNormAuto(1), bDebugBlend(0), dbgLclr(0)
	,bRn1(0),bRn2(0)
	// ter particles
	,cmbParDust(0),cmbParMud(0),cmbParSmoke(0), clrTrail(0)
	// ter surfaces
	,cmbSurface(0)
	,txtSurfTire(0),txtSurfType(0)
	,txtSuBumpWave(0),txtSuBumpAmp(0)
	,txtSuRollDrag(0),txtSuFrict(0)
	// vegetation
	,edGrPage(0),edGrDist(0)
	,edTrPage(0),edTrDist(0),edTrImpDist(0)
	,edGrSwayDistr(0), edGrSwayLen(0), edGrSwaySpd(0)
	,cmbGrassClr(0)
	,viewCanvas(0), tiViewUpd(-1.f), viewBox(0)
	,txVHmin(0),txVHmax(0),txVWmin(0),txVWmax(0),txVCnt(0)
	// paged layers, veget models
	, idPgLay(0), tabsPgLayers(0), valLTrAll(0)
	// grass layers
	, idGrLay(0), tabsGrLayers(0), valLGrAll(0)
	,imgGrass(0),imgGrClr(0)
	, idGrChan(0)
	// road
	,edRdHeightOfs(0)
	,edRdSkirtLen(0),edRdSkirtH(0)
	,cmbRoadWMtr(0),cmbPipeWMtr(0),cmbRoadColMtr(0)
	, idSurf(0), surfList(0)
	// tools
	,fScale(1.f),fScaleTer(1.f)
	// warnings
	, cntWarn(0), logWarn(0), iLoadNext(0)
	,edWarn(0),txWarn(0)
	,imgWarn(0),imgInfo(0)
	// pick
	, idRdPick(0), panPick(0)
	,btnTexDiff(0), btnGrassMtr(0), btnVeget(0)
	,liTex(0), liGrs(0), liVeg(0), liRd(0)
	// track
	, trkName(0), bCopyTrackU(0)
	// gui
	, bTopView(0), noBlendUpd(0), oldFog(0)
	// obj
	,objListSt(0),objListDyn(0),objListRck(0)
	,objListBld(0),objListCat(0), objPan(0)
	// game
	,txtRevebDescr(0),txtEdInfo(0)
	// clr
	,wndColor(0), wpClrSet(0)
{	
	strFSerrors = "";

	int i;  // tool texts
	for (i=0; i<BR_TXT; ++i){  brTxt[i]=0;  brVal[i]=0;  brKey[i]=0;  }
	for (i=0; i<RD_TXT; ++i){  rdTxt[i]=0;  rdVal[i]=0;  rdKey[i]=0;  }
	for (i=0; i<RDS_TXT;++i){  rdTxtSt[i]=0;  rdValSt[i]=0;  }

	for (i=0; i<ST_TXT; ++i)  stTxt[i]=0;
	for (i=0; i<FL_TXT; ++i)  flTxt[i]=0;
	for (i=0; i<OBJ_TXT;++i)  objTxt[i]=0;
	for (i=0; i<RI_TXT; ++i)  riTxt[i]=0;
	
	for (i=0; i < 4; ++i)  {  btnRoad[i]=0;  cmbPipeMtr[i]=0;  }
	for (i=0; i < 4; ++i)  liPickW[i] = 280;

	sc = app1->scn->sc;
	scn = app1->scn;
	pSet = app1->pSet;
	data = app->scn->data;
	mGui = app->mGui;
}
