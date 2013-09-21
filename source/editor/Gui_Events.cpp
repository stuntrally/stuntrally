#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include <fstream>
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/Slider.h"
#include <MyGUI.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreRectangle2D.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
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

//  sun pitch, yaw
void CGui::slUpdSun(SV*)
{
	app->UpdSun();
}

//  light clrs
void CGui::editLiAmb(Edit* ed)
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
void CGui::slUpdFog(SV*)
{
	app->UpdFog();
}

void CGui::editFogClr(Edit* ed)
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

//  chk disable
void CGui::chkFogDisable(WP wp)
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
	sldUpdGrL();
	const SGrassLayer* gr = &sc->grLayersAll[idGrLay], *g0 = &sc->grLayersAll[0];

	chkGrLay->setStateSelected(gr->on);
	if (imgGrass)	imgGrass->setImageTexture(gr->material + ".png");  // same mtr name as tex
	if (imgGrClr)	imgGrClr->setImageTexture(gr->colorMap);

	int used=0;  for (int i=0; i < sc->ciNumGrLay; ++i)  if (sc->grLayersAll[i].on)  ++used;
	SetUsedStr(valLGrAll, used, 4);

	#define _Ed(name, val)  ed##name->setCaption(toStr(val));
	#define _Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) );

	_Cmb(cmbGrassMtr, gr->material);
	_Cmb(cmbGrassClr, gr->colorMap);

	_Ed(GrSwayDistr, g0->swayDistr);
	_Ed(GrSwayLen, g0->swayLen);
	_Ed(GrSwaySpd, g0->swaySpeed);
	
	_Ed(GrTerMaxAngle, g0->terMaxAng);  _Ed(GrTerSmAngle, g0->terAngSm);
	_Ed(GrTerMinHeight, g0->terMinH);  _Ed(GrTerSmHeight, g0->terHSm);
	_Ed(GrTerMaxHeight, g0->terMaxH);
}

//  tab changed, set slider pointer values, and update
void CGui::sldUpdGrL()
{
	SGrassLayer& gr =  sc->grLayersAll[idGrLay];
	SV* sv;
	sv = &svGrMinX;  sv->pFloat = &gr.minSx;  sv->Upd();
	sv = &svGrMaxX;  sv->pFloat = &gr.maxSx;  sv->Upd();
	sv = &svGrMinY;  sv->pFloat = &gr.minSy;  sv->Upd();
	sv = &svGrMaxY;  sv->pFloat = &gr.maxSy;  sv->Upd();
	sv = &svLGrDens; sv->pFloat = &gr.dens;   sv->Upd();
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
	sldUpdPgL();
	const PagedLayer& lay = sc->pgLayersAll[idPgLay];

	chkPgLay->setStateSelected(lay.on);
	cmbPgLay->setIndexSelected( cmbPgLay->findItemIndexWith(lay.name.substr(0,lay.name.length()-5)) );
	Upd3DView(lay.name);
	SetUsedStr(valLTrAll, sc->pgLayers.size(), 5);

	if (edLTrMinTerH)  edLTrMinTerH->setCaption(toStr(lay.minTerH));
	if (edLTrMaxTerH)  edLTrMaxTerH->setCaption(toStr(lay.maxTerH));
	if (edLTrFlDepth)  edLTrFlDepth->setCaption(toStr(lay.maxDepth));
}

//  tab changed
void CGui::sldUpdPgL()
{
	PagedLayer& lay = sc->pgLayersAll[idPgLay];
	SV* sv;
	sv = &svLTrDens;    sv->pFloat = &lay.dens;  sv->Upd();

	sv = &svLTrRdDist;     sv->pInt = &lay.addRdist;  sv->Upd();
	sv = &svLTrRdDistMax;  sv->pInt = &lay.maxRdist;  sv->Upd();

	sv = &svLTrMinSc;   sv->pFloat = &lay.minScale;  sv->Upd();
	sv = &svLTrMaxSc;   sv->pFloat = &lay.maxScale;  sv->Upd();

	sv = &svLTrWindFx;  sv->pFloat = &lay.windFx;  sv->Upd();
	sv = &svLTrWindFy;  sv->pFloat = &lay.windFy;  sv->Upd();

	sv = &svLTrMaxTerAng;  sv->pFloat = &lay.maxTerAng;  sv->Upd();
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


//  [Settings]  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

//  Startup
void CGui::chkStartInMain(WP wp){	ChkEv(startInMain);    }
void CGui::chkAutoStart(WP wp)	{	ChkEv(autostart);	}
void CGui::chkEscQuits(WP wp)	{	ChkEv(escquit);		}
void CGui::chkOgreDialog(WP wp)	{	ChkEv(ogre_dialog);	}

void CGui::chkCamPos(WP wp){		ChkEv(camPos);
	if (pSet->camPos)  app->ovPos->show();  else  app->ovPos->hide();  }

void CGui::chkInputBar(WP wp){		ChkEv(inputBar);
	if (pSet->inputBar)  app->mDebugOverlay->show();  else  app->mDebugOverlay->hide();  }

void CGui::chkCheckSave(WP wp)	{	ChkEv(check_save);  }
void CGui::chkCheckLoad(WP wp)	{	ChkEv(check_load);  }


void CGui::slSizeRoadP(SV*)
{
	if (app->road)
	{	app->road->fMarkerScale = pSet->road_sphr;
		app->road->UpdAllMarkers();  }
}

void CGui::slSizeMinmap(SV*)
{
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
