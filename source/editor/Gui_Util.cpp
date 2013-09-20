#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include <boost/filesystem.hpp>
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/Slider.h"
#include "../ogre/common/MessageBox/MessageBox.h"
using namespace MyGUI;
using namespace Ogre;


//  Gui from xml (scene, road), after load
//..........................................................................................................
void CGui::SetGuiFromXmls()
{
	if (!app->mWndEdit)  return;
	bGI = false;
	
	#define _Ed(name, val)  ed##name->setCaption(toStr(val))
	#define _Clr(name, val)  clr##name->setColour(Colour(val.x,val.y,val.z))
	#define _Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) )
	

	//  [Sky]
	//-----------------------------------------------
	_Cmb(cmbSky, sc->skyMtr);
	svSunPitch.Upd();
	svSunYaw.Upd();
	_Ed(LiAmb, sc->lAmb);  _Ed(LiDiff, sc->lDiff);  _Ed(LiSpec, sc->lSpec);
	_Clr(Amb, sc->lAmb);  _Clr(Diff, sc->lDiff);  _Clr(Spec, sc->lSpec);
	//  fog
	_Clr(Fog, sc->fogClr);  _Clr(Fog2, sc->fogClr2);  _Clr(FogH, sc->fogClrH);
	_Ed(FogClr, sc->fogClr);  _Ed(FogClr2, sc->fogClr2);  _Ed(FogClrH, sc->fogClrH);
	svFogStart.Upd();	svFogEnd.Upd();
	svFogHStart.Upd();	svFogHEnd.Upd();
	svFogHeight.Upd();	svFogHDensity.Upd();
	svRain1Rate.Upd();	svRain2Rate.Upd();	
	_Cmb(cmbRain1, sc->rainName);
	_Cmb(cmbRain2, sc->rain2Name);
	
	//  [Terrain]
	//-----------------------------------------------
	updTabHmap();
	svTerTriSize.Upd();
	if (edTerErrorNorm)  edTerErrorNorm->setCaption(fToStr(sc->td.errorNorm,2,4));
	
	tabTerLayer(tabsTerLayers, idTerLay);
	_Cmb(cmbParDust, sc->sParDust);	_Cmb(cmbParMud,  sc->sParMud);
	_Cmb(cmbParSmoke,sc->sParSmoke);

	//  [Vegetation]
	//-----------------------------------------------
	_Ed(GrassDens, sc->densGrass);	_Ed(TreesDens, sc->densTrees);
	_Ed(TrPage, sc->trPage);		_Ed(TrDist, sc->trDist);
	_Ed(GrPage, sc->grPage);		_Ed(GrDist, sc->grDist);

	_Ed(TrRdDist, sc->trRdDist);	_Ed(TrImpDist, sc->trDistImp);
	_Ed(GrDensSmooth, sc->grDensSmooth);
	edSceneryId->setCaption(sc->sceneryId);

	tabGrLayers(tabsGrLayers, idGrLay);
	tabPgLayers(tabsPgLayers, idPgLay);

	//MeshPtr mp = MeshManager::load(sc->pgLayersAll[0].name);
	//mp->getSubMesh(0)->

	//  [Road]
	//-----------------------------------------------
	for (int i=0; i < 4/*MTRs*/; ++i)
	{	_Cmb(cmbRoadMtr[i], app->road->sMtrRoad[i]);
		_Cmb(cmbPipeMtr[i], app->road->sMtrPipe[i]);  }
	_Cmb(cmbRoadWMtr, app->road->sMtrWall);  _Cmb(cmbRoadColMtr, app->road->sMtrCol);
	_Cmb(cmbPipeWMtr, app->road->sMtrWallPipe);

	_Ed(RdTcMul, app->road->tcMul);  _Ed(RdTcMulW, app->road->tcMul);
	_Ed(RdTcMulP, app->road->tcMul); _Ed(RdTcMulPW, app->road->tcMul);  _Ed(RdTcMulC, app->road->tcMul);
	_Ed(RdColN, app->road->colN);		_Ed(RdColR, app->road->colR);
	_Ed(RdLenDim, app->road->lenDiv0);	_Ed(RdWidthSteps,app->road->iw0);
	_Ed(RdPwsM, app->road->iwPmul);		_Ed(RdPlsM, app->road->ilPmul);
	_Ed(RdHeightOfs,app->road->fHeight);
	_Ed(RdSkirtLen,	app->road->skirtLen);	_Ed(RdSkirtH,	app->road->skirtH);
	_Ed(RdMergeLen,	app->road->setMrgLen);	_Ed(RdLodPLen,	app->road->lposLen);
	bGI = true;
}


void CGui::btnNewGame(WP)
{
	if (trkName)  trkName->setCaption(sListTrack.c_str());
	pSet->gui.track = sListTrack;
	pSet->gui.track_user = bListTrackU;  //UpdWndTitle();//? load
	app->LoadTrack();
}
