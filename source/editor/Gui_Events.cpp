#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include <fstream>
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/Slider.h"
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreRectangle2D.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
#include <MyGUI.h>
using namespace MyGUI;
using namespace Ogre;


///  used value colors  blue,green,yellow,orange,red,black  ..
const Colour CGui::sUsedClr[8] = {
	Colour(0.2,0.6,1), Colour(0,1,0.6), Colour(0,1,0), Colour(0.5,1,0),
	Colour(1,1,0), Colour(1,0.5,0), Colour(1,0,0), Colour(1,0.5,0.5)};

void CGui::SetUsedStr(Txt valUsed, int cnt, int yellowAt)
{
	if (!valUsed)  return;
	valUsed->setCaption(TR("#{Used}") + ": " + toStr(cnt));
	valUsed->setTextColour(sUsedClr[ (int)( std::min(7.f, 4.f*float(cnt)/yellowAt )) ]);
}


///  Gui Events

//  [Sky]
//-----------------------------------------------------------------------------------------------------------

void CGui::comboSky(Cmb cmb, size_t val)  // sky materials
{
	String s = cmb->getItemNameAt(val);
	sc->skyMtr = s;  app->UpdateTrack();
}

void CGui::comboRain1(Cmb cmb, size_t val)  // rain types
{
	String s = cmb->getItemNameAt(val);  sc->rainName = s;
	app->DestroyWeather();  app->CreateWeather();
}
void CGui::comboRain2(Cmb cmb, size_t val)
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
void CGui::editLiAmb(Ed ed)
{
	Vector3 c = s2v(ed->getCaption());	sc->lAmb = c;  app->UpdSun();
	clrAmb->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editLiDiff(Ed ed)
{
	Vector3 c = s2v(ed->getCaption());	sc->lDiff = c;  app->UpdSun();
	clrDiff->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editLiSpec(Ed ed)
{
	Vector3 c = s2v(ed->getCaption());	sc->lSpec = c;  app->UpdSun();
	clrSpec->setColour(Colour(c.x,c.y,c.z));
}

//  fog
void CGui::slUpdFog(SV*)
{
	app->UpdFog();
}

void CGui::editFogClr(Ed ed)
{
	Vector4 c = s2v4(ed->getCaption());  sc->fogClr = c;  app->UpdFog();
	clrFog->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editFogClr2(Ed ed)
{
	Vector4 c = s2v4(ed->getCaption());  sc->fogClr2 = c;  app->UpdFog();
	clrFog2->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editFogClrH(Ed ed)
{
	Vector4 c = s2v4(ed->getCaption());  sc->fogClrH = c;  app->UpdFog();
	clrFogH->setColour(Colour(c.x,c.y,c.z));
}

//  chk disable
void CGui::chkFog(Ck*)
{
	app->UpdFog();
}


//  [Vegetation]
//-----------------------------------------------------------------------------------------------------------

void CGui::editTrGr(Ed ed)
{
	Real r = s2r(ed->getCaption());
	String n = ed->getName();
	SGrassLayer* gr = &sc->grLayersAll[idGrLay], *g0 = &sc->grLayersAll[0];

	     if (n=="GrPage")  sc->grPage = r;   else if (n=="GrDist")  sc->grDist = r;
	else if (n=="TrPage")  sc->trPage = r;   else if (n=="TrDist")  sc->trDist = r;
	else if (n=="TrImpDist")  sc->trDistImp = r;

	else if (n=="GrSwayDistr")  g0->swayDistr = r;
	else if (n=="GrSwayLen")  g0->swayLen = r;
	else if (n=="GrSwaySpd")  g0->swaySpeed = r;

	else if (n=="SceneryId")  sc->sceneryId = ed->getCaption();
}

void CGui::comboGrassMtr(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	SGrassLayer* gr = &sc->grLayersAll[idGrLay];
	gr->material = s;
	if (imgGrass)	imgGrass->setImageTexture(gr->material + ".png");  // same mtr name as tex
}
void CGui::comboGrassClr(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	SGrassLayer* gr = &sc->grLayersAll[idGrLay];
	gr->colorMap = s;
	if (imgGrClr)	imgGrClr->setImageTexture(gr->colorMap);
}


///  Grass layers  ----------------------------------------------------------

void CGui::tabGrLayers(Tab wp, size_t id)
{
	idGrLay = id;  // help var
	SldUpd_GrL();
	const SGrassLayer* gr = &sc->grLayersAll[idGrLay], *g0 = &sc->grLayersAll[0];

	if (imgGrass)	imgGrass->setImageTexture(gr->material + ".png");  // same mtr name as tex
	if (imgGrClr)	imgGrClr->setImageTexture(gr->colorMap);

	int used=0;
	for (int i=0; i < sc->ciNumGrLay; ++i)
		if (sc->grLayersAll[i].on)  ++used;
	SetUsedStr(valLGrAll, used, 4);

	#define _Ed(name, val)  ed##name->setCaption(toStr(val));
	#define _Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) );

	_Cmb(cmbGrassMtr, gr->material);
	_Cmb(cmbGrassClr, gr->colorMap);

	_Ed(GrSwayDistr, g0->swayDistr);
	_Ed(GrSwayLen, g0->swayLen);
	_Ed(GrSwaySpd, g0->swaySpeed);
	
	// todo: more, grass channels..
	svGrTerMaxAngle.Upd();  svGrTerSmAngle.Upd();
	svGrTerMinHeight.Upd(); svGrTerMaxHeight.Upd();  svGrTerSmHeight.Upd();
}

//  tab changed, set slider pointer values, and update
void CGui::SldUpd_GrL()
{
	SGrassLayer& gr =  sc->grLayersAll[idGrLay];
	ckGrLayOn.Upd(&gr.on);
	svLGrDens.UpdF(&gr.dens);

	svGrMinX.UpdF(&gr.minSx);
	svGrMaxX.UpdF(&gr.maxSx);
	svGrMinY.UpdF(&gr.minSy);
	svGrMaxY.UpdF(&gr.maxSy);
}

void CGui::chkGrLayOn(Ck*)
{
	int used=0;
	for (int i=0; i < sc->ciNumGrLay; ++i)
		if (sc->grLayersAll[i].on)  ++used;
	SetUsedStr(valLGrAll, used, 4);
}


///  Vegetation layers  -----------------------------------------------------

void CGui::tabPgLayers(Tab wp, size_t id)
{
	idPgLay = id;  // help var
	SldUpd_PgL();
	const PagedLayer& lay = sc->pgLayersAll[idPgLay];

	cmbPgLay->setIndexSelected( cmbPgLay->findItemIndexWith(lay.name.substr(0,lay.name.length()-5)) );
	Upd3DView(lay.name);
	SetUsedStr(valLTrAll, sc->pgLayers.size(), 5);
}

//  tab changed
void CGui::SldUpd_PgL()
{
	PagedLayer& lay = sc->pgLayersAll[idPgLay];
	ckPgLayOn.Upd(&lay.on);
	svLTrDens.UpdF(&lay.dens);

	svLTrRdDist.UpdI(&lay.addRdist);
	svLTrRdDistMax.UpdI(&lay.maxRdist);

	svLTrMinSc.UpdF(&lay.minScale);
	svLTrMaxSc.UpdF(&lay.maxScale);

	svLTrWindFx.UpdF(&lay.windFx);
	svLTrWindFy.UpdF(&lay.windFy);

	svLTrMaxTerAng.UpdF(&lay.maxTerAng);
	svLTrMinTerH.UpdF(&lay.minTerH);
	svLTrMaxTerH.UpdF(&lay.maxTerH);
	svLTrFlDepth.UpdF(&lay.maxDepth);
}

void CGui::chkPgLayOn(Ck*)
{
	sc->UpdPgLayers();
	SetUsedStr(valLTrAll, sc->pgLayers.size(), 5);
}

void CGui::comboPgLay(Cmb cmb, size_t val)
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


//  [Road]
//-----------------------------------------------------------------------------------------------------------

void CGui::editTrkDesc(Ed ed)
{
	app->road->sTxtDesc = ed->getCaption();
}

void CGui::comboRoadMtr(Cmb cmb, size_t val)
{
	String sn = cmb->getName().substr(String("RdMtr").length(), cmb->getName().length());
	int id = atoi(sn.c_str())-1;  if (id < 0 || id >= MTRs)  return;

	String s = cmb->getItemNameAt(val);
	app->road->sMtrRoad[id] = s;  app->road->RebuildRoad(true);  app->UpdPSSMMaterials();
}

void CGui::comboPipeMtr(Cmb cmb, size_t val)
{
	String sn = cmb->getName().substr(String("RdMtrP").length(), cmb->getName().length());
	int id = atoi(sn.c_str())-1;  if (id < 0 || id >= MTRs)  return;

	String s = cmb->getItemNameAt(val);
	app->road->SetMtrPipe(id, s);  app->road->RebuildRoad(true);  app->UpdPSSMMaterials();
}

void CGui::comboRoadWMtr(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	app->road->sMtrWall = s;  app->road->RebuildRoad(true);  app->UpdPSSMMaterials();
}
void CGui::comboPipeWMtr(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	app->road->sMtrWallPipe = s;  app->road->RebuildRoad(true);  app->UpdPSSMMaterials();
}
void CGui::comboRoadColMtr(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	app->road->sMtrCol = s;  app->road->RebuildRoad(true);  app->UpdPSSMMaterials();
}

void CGui::editRoad(Ed ed)
{
	if (!app->road)  return;
	Real r = s2r(ed->getCaption());
	String n = ed->getName();

		 if (n=="RdHeightOfs")	app->road->fHeight = r;
	else if (n=="RdSkirtLen")	app->road->skirtLen = r;else if (n=="RdSkirtH")	app->road->skirtH = r;
	//app->road->RebuildRoad(true);  //on Enter ?..
}

//  set slider pointer values, and update
void CGui::SldUpd_Road()
{
	if (!app->road)  return;
	SplineRoad& r = *app->road;
	
	svRdTcMul.UpdF(&r.tcMul);	svRdTcMulW.UpdF(&r.tcMulW);
	svRdTcMulP.UpdF(&r.tcMulP);	svRdTcMulPW.UpdF(&r.tcMulPW);
	svRdTcMulC.UpdF(&r.tcMulC);
	svRdLenDim.UpdF(&r.lenDiv0);  svRdWidthSteps.UpdI(&r.iw0);
	svRdPwsM.UpdF(&r.iwPmul);  svRdPlsM.UpdF(&r.ilPmul);
	svRdMergeLen.UpdF(&r.setMrgLen);  svRdLodPLen.UpdF(&r.lposLen);
	svRdColN.UpdI(&r.colN);  svRdColR.UpdF(&r.colR);
}


//  [Game]
//-----------------------------------------------------------------------------------------------------------
void CGui::SldUpd_Game()
{
	svDamage.UpdF(&sc->damageMul);
	svWind.UpdF(&sc->windAmt);
	svGravity.UpdF(&sc->gravity);
}


//  [Settings]  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void CGui::chkCamPos(Ck*){
	if (pSet->camPos)  app->ovPos->show();  else  app->ovPos->hide();  }

void CGui::chkInputBar(Ck*){	
	if (pSet->inputBar)  app->mDebugOverlay->show();  else  app->mDebugOverlay->hide();  }

void CGui::chkWireframe(Ck*)
{
	app->UpdWireframe();
}

void CGui::slSizeRoadP(SV*)
{
	if (app->road)
	{	app->road->fMarkerScale = pSet->road_sphr;
		app->road->UpdAllMarkers();  }
}

void CGui::slSizeMinimap(SV*)
{
	Real sz = pSet->size_minimap;  //int all = 0;
	app->xm1 = 1-sz/app->asp;  app->ym1 = -1+sz;  app->xm2 = 1.0;  app->ym2 = -1.0;
	for (int i=0; i < app->RTs+1; ++i)  if (i != app->RTs)
		if (app->rt[i].mini)
			app->rt[i].mini->setCorners(app->xm1, app->ym1, app->xm2, app->ym2);
}

void CGui::chkMinimap(Ck*)
{
	app->UpdMiniVis();
	if (app->ndPos)
		app->ndPos->setVisible(pSet->trackmap);
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
		pSet->bFog = true;  ckFog.Upd();  app->UpdFog();
	}else
	{	// restore
		cam->setPosition(oldPos);
		cam->setDirection(oldRot);

		pSet->bFog = oldFog;  ckFog.Upd();  app->UpdFog();
	}
}
