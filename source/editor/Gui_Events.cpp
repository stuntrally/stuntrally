#include "pch.h"
#include "../ogre/common/Defines.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include <fstream>
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/Slider.h"
using namespace MyGUI;
using namespace Ogre;


///  used value colors  blue,green,yellow,orange,red,black  ..
const Colour CGui::sUsedClr[8] = {
	Colour(0.2,0.6,1), Colour(0,1,0.6), Colour(0,1,0), Colour(0.5,1,0),
	Colour(1,1,0), Colour(1,0.5,0), Colour(1,0,0), Colour(1,0.5,0.5)};

void CGui::SetUsedStr(MyGUI::StaticTextPtr valUsed, int cnt, int yellowAt)
{
	if (!valUsed)  return;
	valUsed->setCaption(TR("#{Used}") + ": " + toStr(cnt));
	valUsed->setTextColour(sUsedClr[ (int)( std::min(7.f, 4.f*float(cnt)/yellowAt )) ]);
}


///  Gui Events

//  [Sky]
//-----------------------------------------------------------------------------------------------------------

void CGui::comboSky(ComboBoxPtr cmb, size_t val)  // sky materials
{
	String s = cmb->getItemNameAt(val);
	sc->skyMtr = s;  app->UpdateTrack();
}

void CGui::comboRain1(ComboBoxPtr cmb, size_t val)  // rain types
{
	String s = cmb->getItemNameAt(val);  sc->rainName = s;
	app->DestroyWeather();  app->CreateWeather();
}
void CGui::comboRain2(ComboBoxPtr cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);  sc->rain2Name = s;
	app->DestroyWeather();  app->CreateWeather();
}

void CGui::slRain1Rate(SL)  // rain rates
{
	float v = 6000.f * val;		sc->rainEmit = v;
	if (valRain1Rate){	valRain1Rate->setCaption(fToStr(v,0,4));  }  app->UpdSun();
}
void CGui::slRain2Rate(SL)
{
	float v = 6000.f * val;		sc->rain2Emit = v;
	if (valRain2Rate){	valRain2Rate->setCaption(fToStr(v,0,4));  }  app->UpdSun();
}

void CGui::slSunPitch(SL)  // sun pitch, yaw
{
	float v = 90.f * val;	sc->ldPitch = v;
	if (valSunPitch){	valSunPitch->setCaption(fToStr(v,1,4));  }  app->UpdSun();
}
void CGui::slSunYaw(SL)
{
	float v = -180.f + 360.f * val;  sc->ldYaw = v;
	if (valSunYaw){	valSunYaw->setCaption(fToStr(v,1,4));  }  app->UpdSun();
}

void CGui::editLiAmb(Edit* ed)  // edit light clrs
{
	Vector3 c = s2v(ed->getCaption());	sc->lAmb = c;  app->UpdSun();
	if (clrAmb)  clrAmb->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editLiDiff(Edit* ed)
{
	Vector3 c = s2v(ed->getCaption());	sc->lDiff = c;  app->UpdSun();
	if (clrDiff)  clrDiff->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editLiSpec(Edit* ed)
{
	Vector3 c = s2v(ed->getCaption());	sc->lSpec = c;  app->UpdSun();
	if (clrSpec)  clrSpec->setColour(Colour(c.x,c.y,c.z));
}

//  fog
void CGui::slFogStart(SL)  // fog start, end
{
	float v = 2000.f * powf(val, 2.f);		sc->fogStart = v;  app->UpdFog();
	if (valFogStart){	valFogStart->setCaption(fToStr(v,0,3));  }
}
void CGui::slFogEnd(SL)
{
	float v = 2000.f * powf(val, 2.f);		sc->fogEnd = v;    app->UpdFog();
	if (valFogEnd){	 valFogEnd->setCaption(fToStr(v,0,3));  }
}
void CGui::slFogHStart(SL)
{
	float v = 2000.f * powf(val, 2.f);		sc->fogHStart = v;  app->UpdFog();
	if (valFogHStart){	valFogHStart->setCaption(fToStr(v,0,3));  }
}
void CGui::slFogHEnd(SL)
{
	float v = 2000.f * powf(val, 2.f);		sc->fogHEnd = v;    app->UpdFog();
	if (valFogHEnd){	 valFogHEnd->setCaption(fToStr(v,0,3));  }
}
void CGui::slFogHeight(SL)  // edit-
{
	float v = -200.f + 400.f * val;			sc->fogHeight = v;  app->UpdFog();
	if (valFogHeight){	valFogHeight->setCaption(fToStr(v,1,4));  }
}
void CGui::slFogHDensity(SL)
{
	float v = 200.f * powf(val, 2.f);		sc->fogHDensity = v;    app->UpdFog();
	if (valFogHDensity){	valFogHDensity->setCaption(fToStr(v,1,4));  }
}

void CGui::editFogClr(Edit* ed)  // edit fog clr
{
	Vector4 c = s2v4(ed->getCaption());  sc->fogClr = c;  app->UpdFog();
	if (clrFog)  clrFog->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editFogClr2(Edit* ed)
{
	Vector4 c = s2v4(ed->getCaption());  sc->fogClr2 = c;  app->UpdFog();
	if (clrFog2)  clrFog2->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editFogClrH(Edit* ed)
{
	Vector4 c = s2v4(ed->getCaption());  sc->fogClrH = c;  app->UpdFog();
	if (clrFogH)  clrFogH->setColour(Colour(c.x,c.y,c.z));
}

void CGui::chkFogDisable(WP wp)  // chk fog disable
{
	ChkEv(bFog);  app->UpdFog();
}
void CGui::chkWeatherDisable(WP wp)
{
	ChkEv(bWeather);
}


//  [Vegetation]
//-----------------------------------------------------------------------------------------------------------

void CGui::editTrGr(EditPtr ed)
{
	Real r = s2r(ed->getCaption());
	String n = ed->getName();
	SGrassLayer* gr = &sc->grLayersAll[idGrLay], *g0 = &sc->grLayersAll[0];

	if (n=="GrassDens")  sc->densGrass = r;    else if (n=="TreesDens")  sc->densTrees = r;
	else if (n=="TrPage")  sc->trPage = r;     else if (n=="TrDist")  sc->trDist = r;
	else if (n=="TrRdDist")  sc->trRdDist = r; else if (n=="TrImpDist")  sc->trDistImp = r;

	else if (n=="GrPage")  sc->grPage = r;   else if (n=="GrDist")  sc->grDist = r;
	else if (n=="GrDensSmooth")  sc->grDensSmooth = r;

	else if (n=="GrSwayDistr")  g0->swayDistr = r;
	else if (n=="GrSwayLen")  g0->swayLen = r;
	else if (n=="GrSwaySpd")  g0->swaySpeed = r;

	else if (n=="GrTerMaxAngle")  g0->terMaxAng = r;  // todo: more, grass channels
	else if (n=="GrTerSmAngle")  g0->terAngSm = r;

	else if (n=="GrTerMinHeight")  g0->terMinH = r;
	else if (n=="GrTerMaxHeight")  g0->terMaxH = r;
	else if (n=="GrTerSmHeight")  g0->terHSm = r;
	
	else if (n=="SceneryId")  sc->sceneryId = ed->getCaption();
}

void CGui::comboGrassMtr(ComboBoxPtr cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	SGrassLayer* gr = &sc->grLayersAll[idGrLay];
	gr->material = s;
	if (imgGrass)	imgGrass->setImageTexture(gr->material + ".png");  // same mtr name as tex
}
void CGui::comboGrassClr(ComboBoxPtr cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	SGrassLayer* gr = &sc->grLayersAll[idGrLay];
	gr->colorMap = s;
	if (imgGrClr)	imgGrClr->setImageTexture(gr->colorMap);
}


///  Grass layers  ----------------------------------------------------------

void CGui::tabGrLayers(TabPtr wp, size_t id)
{
	idGrLay = id;  // help var
	const SGrassLayer* gr = &sc->grLayersAll[idGrLay], *g0 = &sc->grLayersAll[0];

	chkGrLay->setStateSelected(gr->on);
	if (imgGrass)	imgGrass->setImageTexture(gr->material + ".png");  // same mtr name as tex
	if (imgGrClr)	imgGrClr->setImageTexture(gr->colorMap);

	int used=0;  for (int i=0; i < sc->ciNumGrLay; ++i)  if (sc->grLayersAll[i].on)  ++used;
	SetUsedStr(valLGrAll, used, 4);

	#define _Ed(name, val)  ed##name->setCaption(toStr(val));
	#define _Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) );
	Slider* sl;
	Slv(LGrDens, powf((gr->dens-0.001f) /1.0f, 0.5f));
	_Cmb(cmbGrassMtr, gr->material);  _Cmb(cmbGrassClr, gr->colorMap);

	Slv(GrMinX, powf((gr->minSx-0.1f) /4.0f, 1.f/2.f));
	Slv(GrMaxX, powf((gr->maxSx-0.1f) /4.0f, 1.f/2.f));
	Slv(GrMinY, powf((gr->minSy-0.1f) /4.0f, 1.f/2.f));
	Slv(GrMaxY, powf((gr->maxSy-0.1f) /4.0f, 1.f/2.f));

	_Ed(GrSwayDistr, g0->swayDistr);
	_Ed(GrSwayLen, g0->swayLen);
	_Ed(GrSwaySpd, g0->swaySpeed);
	
	_Ed(GrTerMaxAngle, g0->terMaxAng);  _Ed(GrTerSmAngle, g0->terAngSm);
	_Ed(GrTerMinHeight, g0->terMinH);  _Ed(GrTerSmHeight, g0->terHSm);
	_Ed(GrTerMaxHeight, g0->terMaxH);
}

void CGui::chkGrLayOn(WP wp)
{
	sc->grLayersAll[idGrLay].on = !sc->grLayersAll[idGrLay].on;

	ButtonPtr chk = wp->castType<Button>();
	chk->setStateSelected(sc->grLayersAll[idGrLay].on);

	int used=0;  for (int i=0; i < sc->ciNumGrLay; ++i)  if (sc->grLayersAll[i].on)  ++used;
	SetUsedStr(valLGrAll, used, 4);
}


///  Vegetation layers  -----------------------------------------------------

void CGui::tabPgLayers(TabPtr wp, size_t id)
{
	idPgLay = id;  // help var
	const PagedLayer& lay = sc->pgLayersAll[idPgLay];

	chkPgLay->setStateSelected(lay.on);
	cmbPgLay->setIndexSelected( cmbPgLay->findItemIndexWith(lay.name.substr(0,lay.name.length()-5)) );
	Upd3DView(lay.name);
	SetUsedStr(valLTrAll, sc->pgLayers.size(), 5);

	//  set slider values
	Slider* sl;
	Slv(LTrDens, powf((lay.dens-0.001f) /1.0f, 0.5f));
	Slv(LTrRdDist, lay.addRdist /20.f);
	Slv(LTrRdDistMax, lay.maxRdist /20.f);

	Slv(LTrMinSc, powf(lay.minScale /6.0f, 1.f/3.f));
	Slv(LTrMaxSc, powf(lay.maxScale /6.0f, 1.f/3.f));
	Slv(LTrWindFx, powf(lay.windFx /12.0f, 1.f/3.f));
	Slv(LTrWindFy, powf(lay.windFy /12.0f, 1.f/3.f));
	Slv(LTrMaxTerAng, powf(lay.maxTerAng /90.0f, 1.f/2.f));
	if (edLTrMinTerH)  edLTrMinTerH->setCaption(toStr(lay.minTerH));
	if (edLTrMaxTerH)  edLTrMaxTerH->setCaption(toStr(lay.maxTerH));
	if (edLTrFlDepth)  edLTrFlDepth->setCaption(toStr(lay.maxDepth));
}

void CGui::chkPgLayOn(WP wp)
{
	sc->pgLayersAll[idPgLay].on = !sc->pgLayersAll[idPgLay].on;
	sc->UpdPgLayers();
	ButtonPtr chk = wp->castType<Button>();
	chk->setStateSelected(sc->pgLayersAll[idPgLay].on);
	SetUsedStr(valLTrAll, sc->pgLayers.size(), 5);
}

void CGui::comboPgLay(ComboBoxPtr cmb, size_t val)
{
	String s = cmb->getItemNameAt(val) + ".mesh";
	sc->pgLayersAll[idPgLay].name = s;
	Upd3DView(s);
}

void CGui::Upd3DView(String mesh)
{
	viewMesh = mesh;
	tiViewUpd = 0.f;
}


void CGui::slGrMinX(SL)
{
	Real v = 0.1f + 4.0f * powf(val, 2.f);  sc->grLayersAll[idGrLay].minSx = v;
	if (valGrMinX){  valGrMinX->setCaption(fToStr(v,2,4));  }
}
void CGui::slGrMaxX(SL)
{
	Real v = 0.1f + 4.0f * powf(val, 2.f);  sc->grLayersAll[idGrLay].maxSx = v;
	if (valGrMaxX){  valGrMaxX->setCaption(fToStr(v,2,4));  }
}
void CGui::slGrMinY(SL)
{
	Real v = 0.1f + 4.0f * powf(val, 2.f);  sc->grLayersAll[idGrLay].minSy = v;
	if (valGrMinY){  valGrMinY->setCaption(fToStr(v,2,4));  }
}
void CGui::slGrMaxY(SL)
{
	Real v = 0.1f + 4.0f * powf(val, 2.f);  sc->grLayersAll[idGrLay].maxSy = v;
	if (valGrMaxY){  valGrMaxY->setCaption(fToStr(v,2,4));  }
}


void CGui::slLGrDens(SL)  //  sliders
{
	Real v = 0.001f + 1.0f * powf(val, 2.f);  sc->grLayersAll[idGrLay].dens = v;
	if (valLGrDens){  valLGrDens->setCaption(fToStr(v,3,5));  }
}

void CGui::slLTrDens(SL)
{
	Real v = 0.001f + 1.0f * powf(val, 2.f);  sc->pgLayersAll[idPgLay].dens = v;
	if (valLTrDens){  valLTrDens->setCaption(fToStr(v,3,5));  }
}
void CGui::slLTrRdDist(SL)
{
	int v = val * 20.f +slHalf;
	sc->pgLayersAll[idPgLay].addRdist = v;
	if (valLTrRdDist)  valLTrRdDist->setCaption(toStr(v));
}
void CGui::slLTrRdDistMax(SL)
{
	int v = val * 20.f +slHalf;
	sc->pgLayersAll[idPgLay].maxRdist = v;
	if (valLTrRdDistMax)  valLTrRdDistMax->setCaption(toStr(v));
}

void CGui::slLTrMinSc(SL)
{
	Real v = 6.0f * powf(val, 3.f);		sc->pgLayersAll[idPgLay].minScale = v;
	if (valLTrMinSc){  valLTrMinSc->setCaption(fToStr(v,3,5));  }
}
void CGui::slLTrMaxSc(SL)
{
	Real v = 6.0f * powf(val, 3.f);		sc->pgLayersAll[idPgLay].maxScale = v;
	if (valLTrMaxSc){  valLTrMaxSc->setCaption(fToStr(v,3,5));  }
}

void CGui::slLTrWindFx(SL)
{
	Real v = 12.0f * powf(val, 3.f);	sc->pgLayersAll[idPgLay].windFx = v;
	if (valLTrWindFx){  valLTrWindFx->setCaption(fToStr(v,3,5));  }
}
void CGui::slLTrWindFy(SL)
{
	Real v = 12.0f * powf(val, 3.f);	sc->pgLayersAll[idPgLay].windFy = v;
	if (valLTrWindFy){  valLTrWindFy->setCaption(fToStr(v,3,5));  }
}

void CGui::slLTrMaxTerAng(SL)
{
	Real v = 90.0f * powf(val, 2.f);	sc->pgLayersAll[idPgLay].maxTerAng = v;
	if (valLTrMaxTerAng){  valLTrMaxTerAng->setCaption(fToStr(v,1,5));  }
}
void CGui::editLTrMinTerH(EditPtr ed)
{
	sc->pgLayersAll[idPgLay].minTerH = s2r(ed->getCaption());
}
void CGui::editLTrMaxTerH(EditPtr ed)
{
	sc->pgLayersAll[idPgLay].maxTerH = s2r(ed->getCaption());
}
void CGui::editLTrFlDepth(EditPtr ed)
{
	sc->pgLayersAll[idPgLay].maxDepth = s2r(ed->getCaption());
}


//  [Road]
//-----------------------------------------------------------------------------------------------------------

void CGui::editTrkDesc(EditPtr ed)
{
	app->road->sTxtDesc = ed->getCaption();
}

void CGui::comboRoadMtr(ComboBoxPtr cmb, size_t val)
{
	String sn = cmb->getName().substr(String("RdMtr").length(), cmb->getName().length());
	int id = atoi(sn.c_str())-1;  if (id < 0 || id >= MTRs)  return;

	String s = cmb->getItemNameAt(val);
	app->road->sMtrRoad[id] = s;  app->road->RebuildRoad(true);  app->UpdPSSMMaterials();
}

void CGui::comboPipeMtr(ComboBoxPtr cmb, size_t val)
{
	String sn = cmb->getName().substr(String("RdMtrP").length(), cmb->getName().length());
	int id = atoi(sn.c_str())-1;  if (id < 0 || id >= MTRs)  return;

	String s = cmb->getItemNameAt(val);
	app->road->SetMtrPipe(id, s);  app->road->RebuildRoad(true);  app->UpdPSSMMaterials();
}

void CGui::comboRoadWMtr(ComboBoxPtr cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	app->road->sMtrWall = s;  app->road->RebuildRoad(true);  app->UpdPSSMMaterials();
}
void CGui::comboPipeWMtr(ComboBoxPtr cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	app->road->sMtrWallPipe = s;  app->road->RebuildRoad(true);  app->UpdPSSMMaterials();
}
void CGui::comboRoadColMtr(ComboBoxPtr cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	app->road->sMtrCol = s;  app->road->RebuildRoad(true);  app->UpdPSSMMaterials();
}

void CGui::editRoad(EditPtr ed)
{
	if (!app->road)  return;
	Real r = s2r(ed->getCaption());
	String n = ed->getName();

		 if (n=="RdTcMul")		app->road->tcMul = r;	else if (n=="RdTcMulP")	app->road->tcMulP = r;
	else if (n=="RdTcMulW")		app->road->tcMulW = r;	else if (n=="RdTcMulPW") app->road->tcMulPW = r;  else if (n=="RdTcMulC")	app->road->tcMulC = r;
	else if (n=="RdColR")		app->road->colR = r;	else if (n=="RdColN")	app->road->colN = std::max(3.f, r);
	else if (n=="RdLenDim")		app->road->lenDiv0 = r;	
	else if (n=="RdWidthSteps")	app->road->iw0 = r;		else if (n=="RdPwsM")	app->road->iwPmul = r;
	else if (n=="RdHeightOfs")	app->road->fHeight = r;	else if (n=="RdPlsM")	app->road->ilPmul = r;
	else if (n=="RdSkirtLen")	app->road->skirtLen = r;else if (n=="RdSkirtH")	app->road->skirtH = r;
	else if (n=="RdMergeLen")	app->road->setMrgLen = r;
	else if (n=="RdLodPLen")	app->road->lposLen = r;
	//app->road->RebuildRoad(true);  //on Enter ?..
}

void CGui::slAlignWidthAdd(SL)
{
	Real v = 20.f * val;	pSet->al_w_add = v;
	if (valAlignWidthAdd)  valAlignWidthAdd->setCaption(fToStr(v,1,3));
}
void CGui::slAlignWidthMul(SL)
{
	Real v = 1.f + 4.f * val;	pSet->al_w_mul = v;
	if (valAlignWidthMul)  valAlignWidthMul->setCaption(fToStr(v,2,4));
}
void CGui::slAlignSmooth(SL)
{
	Real v = 6.f * val;		pSet->al_smooth = v;
	if (valAlignSmooth)  valAlignSmooth->setCaption(fToStr(v,1,3));
}


//  [Settings]  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

//  Startup
void CGui::chkStartInMain(WP wp)	{	ChkEv(startInMain);    }
void CGui::chkAutoStart(WP wp)	{	ChkEv(autostart);	}
void CGui::chkEscQuits(WP wp)	{	ChkEv(escquit);		}
void CGui::chkOgreDialog(WP wp)	{	ChkEv(ogre_dialog);	}

void CGui::chkCamPos(WP wp){			ChkEv(camPos);
	if (pSet->camPos)  app->ovPos->show();  else  app->ovPos->hide();  }

void CGui::chkInputBar(WP wp){		ChkEv(inputBar);
	if (pSet->inputBar)  app->mDebugOverlay->show();  else  app->mDebugOverlay->hide();  }

void CGui::chkCheckSave(WP wp)	{	ChkEv(check_save);  }
void CGui::chkCheckLoad(WP wp)	{	ChkEv(check_load);  }


void CGui::slMiniUpd(SL)
{
	int v = val * 20.f +slHalf;  pSet->mini_skip = v;
	if (valMiniUpd){	valMiniUpd->setCaption(toStr(v));  }
}

void CGui::slSizeRoadP(SL)
{
	Real v = 0.1f + 11.9f * val;  pSet->road_sphr = v;
	if (valSizeRoadP){	valSizeRoadP->setCaption(fToStr(v,2,4));  }
	if (app->road)
	{	app->road->fMarkerScale = v;
		app->road->UpdAllMarkers();  }
}

void CGui::slCamInert(SL)
{
	Real v = val;  pSet->cam_inert = v;
	if (valCamInert){	valCamInert->setCaption(fToStr(v,2,4));  }
}
void CGui::slCamSpeed(SL)
{
	Real v = 0.1f + 3.9f * powf(val, 1.f);  pSet->cam_speed = v;
	if (valCamSpeed){	valCamSpeed->setCaption(fToStr(v,2,4));  }
}

void CGui::slTerUpd(SL)
{
	int v = val * 20.f +slHalf;  pSet->ter_skip = v;
	if (valTerUpd){	valTerUpd->setCaption(toStr(v));  }
}

void CGui::slSizeMinmap(SL)
{
	float v = 0.15f + 1.85f * val;	pSet->size_minimap = v;
	if (valSizeMinmap){	valSizeMinmap->setCaption(fToStr(v,3,4));  }
	Real sz = pSet->size_minimap;  //int all = 0;
	app->xm1 = 1-sz/app->asp;  app->ym1 = -1+sz;  app->xm2 = 1.0;  app->ym2 = -1.0;
	for (int i=0; i < app->RTs+1; ++i)  if (i != app->RTs)
		if (app->rt[i].rcMini)
			app->rt[i].rcMini->setCorners(app->xm1, app->ym1, app->xm2, app->ym2);
}

void CGui::chkMinimap(WP wp)
{
	ChkEv(trackmap);  app->UpdMiniVis();
	if (app->ndPos)  app->ndPos->setVisible(pSet->trackmap);
}

void CGui::chkAutoBlendmap(WP wp)
{
	ChkEv(autoBlendmap);
}


//  set camera in settings at exit
void App::SaveCam()
{
	if (!mCamera)  return;
	Vector3 p = mCamera->getPosition(), d = mCamera->getDirection();
	if (gui->bTopView)  {  p = gui->oldPos;  d = gui->oldRot;  }
	pSet->cam_x  = p.x;  pSet->cam_y  = p.y;  pSet->cam_z  = p.z;
	pSet->cam_dx = d.x;  pSet->cam_dy = d.y;  pSet->cam_dz = d.z;
}

//  set predefined camera view
void CGui::btnSetCam(WP wp)
{
	String s = wp->getName();
	Real y0 = 20, xz = sc->td.fTerWorldSize*0.5f, r = 45.f * 0.5f*PI_d/180.f, yt = xz / Math::Tan(r);
	Camera* cam = app->mCamera;

		 if (s=="CamView1")	{	cam->setPosition(xz*0.8,60,0);  cam->setDirection(-1,-0.3,0);  }
	else if (s=="CamView2")	{	cam->setPosition(xz*0.6,80,xz*0.6);  cam->setDirection(-1,-0.5,-1);  }
	else if (s=="CamView3")	{	cam->setPosition(-xz*0.7,80,-xz*0.5);  cam->setDirection(0.8,-0.5,0.5);  }
	else if (s=="CamView4")	{
		Vector3 cp = app->ndCar->getPosition();  float cy = app->ndCar->getOrientation().getYaw().valueRadians();
		Vector3 cd = Vector3(cosf(cy),0,-sinf(cy));
		cam->setPosition(cp - cd * 15 + Vector3(0,7,0));  cd.y = -0.3f;
		cam->setDirection(cd);  }

	else if (s=="CamTop")	{	cam->setPosition(0,yt,0);  cam->setDirection(-0.0001,-1,0);  }
	else if (s=="CamLeft")	{	cam->setPosition(0,y0, xz);  cam->setDirection(0,0,-1);  }
	else if (s=="CamRight")	{	cam->setPosition(0,y0,-xz);  cam->setDirection(0,0, 1);  }
	else if (s=="CamFront")	{	cam->setPosition( xz,y0,0);  cam->setDirection(-1,0,0);  }
	else if (s=="CamBack")	{	cam->setPosition(-xz,y0,0);  cam->setDirection( 1,0,0);  }
}

//  toggle top view camera
void CGui::toggleTopView()
{
	bTopView = !bTopView;
	Camera* cam = app->mCamera;
	
	if (bTopView)
	{	// store old
		oldPos = cam->getPosition();
		oldRot = cam->getDirection();
		
		Real xz = sc->td.fTerWorldSize*0.5f, r = 45.f * 0.5f*PI_d/180.f, yt = xz / Math::Tan(r);
		cam->setPosition(0,yt,0);  cam->setDirection(-0.0001,-1,0);

		oldFog = pSet->bFog;
		pSet->bFog = true;  chkFog->setStateSelected(pSet->bFog);  app->UpdFog();
	}else
	{	// restore
		cam->setPosition(oldPos);
		cam->setDirection(oldRot);

		pSet->bFog = oldFog;  chkFog->setStateSelected(pSet->bFog);  app->UpdFog();
	}
}
