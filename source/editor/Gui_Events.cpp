#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../ogre/common/CScene.h"
#include "../ogre/common/data/CData.h"
#include "../ogre/common/data/PresetsXml.h"
#include "../road/Road.h"
#include <fstream>
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/Slider.h"
#include "../ogre/common/MultiList2.h"
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreRectangle2D.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
#include <OgreRenderWindow.h>
#include <MyGUI.h>
using namespace MyGUI;
using namespace Ogre;
using namespace std;


///  used value colors  blue,green,yellow,orange,red,black  ..
const Colour CGui::sUsedClr[8] = {
	Colour(0.2,0.6,1), Colour(0,1,0.6), Colour(0,1,0), Colour(0.5,1,0),
	Colour(1,1,0), Colour(1,0.5,0), Colour(1,0,0), Colour(0.9,0.2,0.2)};

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
	app->scn->DestroyWeather();  app->scn->CreateWeather();
}
void CGui::comboRain2(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);  sc->rain2Name = s;
	app->scn->DestroyWeather();  app->scn->CreateWeather();
}

void CGui::slUpdSky(SV*){	scn->UpdSky();	}
void CGui::slUpdSun(SV*){	scn->UpdSun();	}

//  fog
void CGui::slUpdFog(SV*)
{
	scn->UpdFog();
}

//  chk disable
void CGui::chkFog(Ck*)
{
	scn->UpdFog();
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
}

void CGui::comboGrassClr(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	SGrassLayer* gr = &sc->grLayersAll[idGrLay];
	gr->colorMap = s;
	imgGrClr->setImageTexture(gr->colorMap);
}


///  Grass layers  ----------------------------------------------------------

void CGui::tabGrLayers(Tab wp, size_t id)
{
	idGrLay = id;  // help var
	SldUpd_GrL();
	SldUpd_GrChan();
	const SGrassLayer* gr = &sc->grLayersAll[idGrLay], *g0 = &sc->grLayersAll[0];

	imgGrass->setImageTexture(gr->material + ".png");  // same mtr name as tex
	imgGrClr->setImageTexture(gr->colorMap);

	int used=0;
	for (int i=0; i < sc->ciNumGrLay; ++i)
		if (sc->grLayersAll[i].on)  ++used;
	SetUsedStr(valLGrAll, used, 4);

	#define _Ed(name, val)  ed##name->setCaption(toStr(val));
	#define _Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) );

	btnGrassMtr->setCaption(gr->material);
	for (int i=0; i < liGrs->getItemCount(); ++i)  // upd pick
		if (liGrs->getSubItemNameAt(1,i).substr(7) == gr->material)
			liGrs->setIndexSelected(i);
	_Cmb(cmbGrassClr, gr->colorMap);

	_Ed(GrSwayDistr, g0->swayDistr);
	_Ed(GrSwayLen, g0->swayLen);
	_Ed(GrSwaySpd, g0->swaySpeed);  //g0-
}

//  tab changed, set slider pointer values, and update
void CGui::SldUpd_GrL()
{
	SGrassLayer& gr =  sc->grLayersAll[idGrLay];
	ckGrLayOn.Upd(&gr.on);
	svGrChan.UpdI(&gr.iChan);
	svLGrDens.UpdF(&gr.dens);

	svGrMinX.UpdF(&gr.minSx);  svGrMaxX.UpdF(&gr.maxSx);
	svGrMinY.UpdF(&gr.minSy);  svGrMaxY.UpdF(&gr.maxSy);
}

///  channels
void CGui::tabGrChan(Tab wp, size_t id)
{
	idGrChan = id;  // help var
	SldUpd_GrChan();
}

void CGui::SldUpd_GrChan()
{
	SGrassChannel& gr =  sc->grChan[idGrChan];
	svGrChAngMin.UpdF(&gr.angMin);  svGrChAngMax.UpdF(&gr.angMax);  svGrChAngSm.UpdF(&gr.angSm);
	svGrChHMin.UpdF(&gr.hMin);  svGrChHMax.UpdF(&gr.hMax);  svGrChHSm.UpdF(&gr.hSm);
	svGrChRdPow.UpdF(&gr.rdPow);
	svGrChNoise.UpdF(&gr.noise);  svGrChNfreq.UpdF(&gr.nFreq);
	svGrChNoct.UpdI(&gr.nOct);  svGrChNpers.UpdF(&gr.nPers);  svGrChNpow.UpdF(&gr.nPow);
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
	string s = lay.name.substr(0, lay.name.length()-5);

	btnVeget->setCaption(s);
	for (int i=0; i < liVeg->getItemCount(); ++i)  // upd pick
		if (liVeg->getSubItemNameAt(1,i).substr(7) == s)
			liVeg->setIndexSelected(i);
			
	Upd3DView(lay.name);
	SetUsedStr(valLTrAll, sc->pgLayers.size(), 5);
	txVCnt->setCaption(toStr(lay.cnt));
}

void CGui::updVegetInfo()
{
	Vector3 va = viewSc * svLTrMinSc.getF(),
			vb = viewSc * svLTrMaxSc.getF();
	float wa = std::max(va.x, va.z), wb = std::max(vb.x, vb.z);
	txVHmin->setCaption(fToStr(va.y, 1,4));
	txVHmax->setCaption(fToStr(vb.y, 1,4));
	txVWmin->setCaption(fToStr(wa, 1,4));
	txVWmax->setCaption(fToStr(wb, 1,4));
}

void CGui::slLTrSc(SV*)
{
	updVegetInfo();
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

void CGui::Upd3DView(String mesh)
{
	viewMesh = mesh;
	tiViewUpd = 0.f;
}


//  [Road]
//-----------------------------------------------------------------------------------------------------------

void CGui::editTrkDescr(Ed ed)
{
	app->scn->road->sTxtDescr = ed->getCaption();
}
void CGui::editTrkAdvice(Ed ed)
{
	app->scn->road->sTxtAdvice = ed->getCaption();
}

void CGui::editRoad(Ed ed)
{
	if (!app->scn->road)  return;
	Real r = s2r(ed->getCaption());
	String n = ed->getName();

		 if (n=="RdHeightOfs")	app->scn->road->g_Height = r;
	else if (n=="RdSkirtLen")	app->scn->road->g_SkirtLen = r;
	else if (n=="RdSkirtH")		app->scn->road->g_SkirtH = r;
	//app->scn->road->RebuildRoad(true);  //on Enter ?..
}

//  set slider pointer values, and update
void CGui::SldUpd_Road()
{
	if (!app->scn->road)  return;
	SplineRoad& r = *app->scn->road;
	
	svRdTcMul.UpdF(&r.g_tcMul);       svRdTcMulW.UpdF(&r.g_tcMulW);
	svRdTcMulP.UpdF(&r.g_tcMulP);     svRdTcMulPW.UpdF(&r.g_tcMulPW);
	svRdTcMulC.UpdF(&r.g_tcMulC);

	svRdLenDim.UpdF(&r.g_LenDim0);    svRdWidthSteps.UpdI(&r.g_iWidthDiv0);
	svRdPwsM.UpdF(&r.g_P_iw_mul);     svRdPlsM.UpdF(&r.g_P_il_mul);
	svRdColN.UpdI(&r.g_ColNSides);    svRdColR.UpdF(&r.g_ColRadius);

	svRdMergeLen.UpdF(&r.g_MergeLen); svRdLodPLen.UpdF(&r.g_LodPntLen);
	svRdVisDist.UpdF(&r.g_VisDist);   svRdVisBehind.UpdF(&r.g_VisBehind);
}


//  [Game]
//-----------------------------------------------------------------------------------------------------------
void CGui::SldUpd_Game()
{
	svDamage.UpdF(&sc->damageMul);
	svWind.UpdF(&sc->windAmt);
	svGravity.UpdF(&sc->gravity);
}
void CGui::comboReverbs(Cmb cmb, size_t val)  // reverb sets
{
	String s = cmb->getItemNameAt(val);
	sc->sReverbs = s;
	UpdRevDescr();
}
void CGui::UpdRevDescr()
{
	sc->UpdRevSet();
	txtRevebDescr->setCaption(sc->revSet.descr);
}

//  Pacenotes
void CGui::slUpd_Pace(SV*)
{
	scn->UpdPaceParams();
}

void CGui::chkTrkReverse(Ck*)
{
	scn->road->Rebuild(true);
}


//  [Settings]  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void CGui::chkCamPos(Ck*){     app->txCamPos->setVisible(pSet->camPos);  }
void CGui::chkInputBar(Ck*){   app->bckInput->setVisible(pSet->inputBar);  }

void CGui::chkFps(Ck*){        app->bckFps->setVisible(pSet->show_fps);  }
void CGui::chkWireframe(Ck*){  app->UpdWireframe();  }

void CGui::slSizeRoadP(SV*)
{
	if (app->scn->road)
	{	app->scn->road->fMarkerScale = pSet->road_sphr;
		app->scn->road->UpdAllMarkers();  }
}

void CGui::slSizeMinimap(SV*)
{
	Real sz = pSet->size_minimap;  //int all = 0;
	app->asp = float(app->mWindow->getWidth()) / float(app->mWindow->getHeight());
	app->xm1 = 1-sz/app->asp;  app->ym1 = -1+sz;  app->xm2 = 1.0;  app->ym2 = -1.0;
	for (int i=0; i < app->RT_Brush; ++i)  if (i != app->RT_Last)
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
		pSet->bFog = true;  ckFog.Upd();  scn->UpdFog();
	}else
	{	// restore
		cam->setPosition(oldPos);
		cam->setDirection(oldRot);

		pSet->bFog = oldFog;  ckFog.Upd();  scn->UpdFog();
	}
}


//  [Surface]
//-----------------------------------------------------------------------------------------------------------

TerLayer* CGui::GetTerRdLay()
{
	if (idSurf < 4)  //  terrain
	{	if (idSurf >= sc->td.layers.size())  // could change by on/off ter layers
		{	idSurf = 0;  if (surfList)  surfList->setIndexSelected(idSurf);  }
		return &sc->td.layersAll[sc->td.layers[idSurf]];
	}
	//  road
	return  &sc->td.layerRoad[sc->td.road1mtr ? 0 : idSurf-4];
}

void CGui::listSurf(Li, size_t id)
{
	if (id == ITEM_NONE) {  id = 0;  surfList->setIndexSelected(0);  }
	if (id < 4 && id >= sc->td.layers.size()) {  id = 0;  surfList->setIndexSelected(id);  }  // more than used
	if (id >= 8) {  id = 4;  surfList->setIndexSelected(id);  }
	//TODO: own pipe mtrs..

	idSurf = id;  // help var
	TerLayer* l = GetTerRdLay();
	SldUpd_Surf();

	Vector3 c;  c = l->tclr.GetRGB1();
	clrTrail->setColour(Colour(c.x, c.y, c.z));
	
	//  Surface
	cmbSurface->setIndexSelected( cmbSurface->findItemIndexWith(l->surfName));
	UpdSurfInfo();
}

void CGui::SldUpd_Surf()
{
	TerLayer* l = GetTerRdLay();

	svLDust.UpdF(&l->dust);  svLDustS.UpdF(&l->dustS);
	svLMud.UpdF(&l->mud);    svLSmoke.UpdF(&l->smoke);
}

void CGui::UpdSurfList()
{
	if (!surfList || surfList->getItemCount() != 12)  return;

	for (int n=0; n < 4; ++n)
	{
		String s = n >= app->scn->sc->td.layers.size() ? "" : StringUtil::replaceAll(
			app->scn->sc->td.layersAll[app->scn->sc->td.layers[n]].texFile, "_d.jpg","");
		surfList->setItemNameAt(n  , "#80FF00"+TR("#{Layer} ")+toStr(n+1)+"  "+ s);
		surfList->setItemNameAt(n+4, "#FFB020"+TR("#{Road} ") +toStr(n+1)+"  "+ app->scn->road->sMtrRoad[n]);
		surfList->setItemNameAt(n+8, "#FFFF80"+TR("#{Pipe} ") +toStr(n+1)+"  "+ app->scn->road->sMtrPipe[n]);
	}
	GetTerRdLay();  // fix list pos if cur gone
}

//  upd ed info txt
void CGui::UpdEdInfo()
{	
	int t = scn->sc->secEdited;
	ostringstream s;  s.fill('0');
	s << fixed << "#C0F0F8" << t/3600 << ":#A0D8E0" << setw(2) << t/60%60 << ":#70A8B0" << setw(2) << t%60;
	txtEdInfo->setCaption(
		TR("#A0C0D0#{Time} [h:m:s]: ") + s.str()+"\n"+
		TR("#808080Base track: ") + scn->sc->baseTrk);
}


//-----------------------------------------------------------------------------------------------------------
///  [Pick window]
//-----------------------------------------------------------------------------------------------------------
int CGui::liNext(Mli2 li, int rel)
{
	int cnt = li->getItemCount()-1;
	if (cnt < 0)  return 0;
	int i = li->getIndexSelected();
	if (i == ITEM_NONE)  i = 0;
	i += rel;  if (i<0) i=0;  if (i>cnt) i=cnt;
	li->setIndexSelected(i);
	li->beginToItemAt(std::min(cnt, std::max(0, i-20)));
	return i;
}

void CGui::keyPickNext(int r)
{
	if (liSky->getVisible())  listPickSky(liSky, liNext(liSky, r));  else
	if (liTex->getVisible())  listPickTex(liTex, liNext(liTex, r));	 else
	if (liGrs->getVisible())  listPickGrs(liGrs, liNext(liGrs, r));  else
	if (liVeg->getVisible())  listPickVeg(liVeg, liNext(liVeg, r));  else
	if (liRd->getVisible())   listPickRd(liRd, liNext(liRd, r));
}

void CGui::wheelSky(WP wp, int rel){  int r = rel < 0 ? 1 : -1;  listPickSky(liSky, liNext(liSky, r));  }
void CGui::wheelTex(WP wp, int rel){  int r = rel < 0 ? 1 : -1;  listPickTex(liTex, liNext(liTex, r));  }
void CGui::wheelGrs(WP wp, int rel){  int r = rel < 0 ? 1 : -1;  listPickGrs(liGrs, liNext(liGrs, r));  }
void CGui::wheelVeg(WP wp, int rel){  int r = rel < 0 ? 1 : -1;  listPickVeg(liVeg, liNext(liVeg, r));  }

void CGui::wheelRd(WP wp, int rel)
{
	idRdPick = atoi(wp->getName().substr(0,1).c_str());
	int r = rel < 0 ? 1 : -1;  listPickRd(liRd, liNext(liRd, r));
}

void CGui::btnPickSky(WP){    PickShow(P_Sky);  }
void CGui::btnPickTex(WP){    PickShow(P_Tex);  }
void CGui::btnPickGrass(WP){  PickShow(P_Grs);  }
void CGui::btnPickVeget(WP){  PickShow(P_Veg);  }

void CGui::btnPickRd(WP wp)
{
	if (!wp) {  PickShow(P_Rd, true);  return;  }
	// int old = idRdPick;
	idRdPick = atoi(wp->getName().substr(0,1).c_str());
	PickShow(P_Rd, false); //old==idRdPick);
}
void CGui::btnPickRoad(WP wp)
{
	idRdType = RdPk_Road;
	btnPickRd(wp);
}
// Btn btnRoad[4], btnPipe[4], btnRoadW =0, btnPipeW =0, btnRoadCol =0;
void CGui::btnPickPipe(WP wp)
{
	idRdType = RdPk_Pipe;
	btnPickRd(wp);
	// String sn = cmb->getName().substr(0,1);
	// int id = atoi(sn.c_str())-1;  if (id < 0 || id >= MTRs)  return;

	// String s = cmb->getItemNameAt(val);
	// app->scn->road->SetMtrPipe(id, s);  app->scn->road->Rebuild(true);  scn->UpdPSSMMaterials();
	// UpdSurfList();
}

void CGui::btnPickRoadW(WP wp)
{
	idRdType = RdPk_Wall;
	btnPickRd(wp);
}
void CGui::btnPickPipeW(WP wp)
{
	idRdType = RdPk_PipeW;
	btnPickRd(wp);
}
void CGui::btnPickRoadCol(WP wp)
{
	idRdType = RdPk_Col;
	btnPickRd(wp);
}


//  Pick window show
//----------------------------------------------------
void CGui::PickShow(EPick n, bool toggleVis)
{
	liSky->setVisible(n == P_Sky);  liTex->setVisible(n == P_Tex);
	liGrs->setVisible(n == P_Grs);  liVeg->setVisible(n == P_Veg);
	liRd->setVisible(n == P_Rd);
	panPick->setPosition(liPickW[n], 0);

	const int wx = pSet->windowx, wy = pSet->windowy;  ///  pick dim
	app->mWndPick->setCoord(wx * liPickX[n], 12.f, liPickW[n], 0.95f*wy);
	
	if (n == P_Rd)  //  upd pick road
	{	UString s = btnRoad[idRdPick]->getCaption();
		for (int i=0; i < liRd->getItemCount(); ++i)
			if (liRd->getSubItemNameAt(1,i).substr(7) == s)
				liRd->setIndexSelected(i);
	}
	bool vis = app->mWndPick->getVisible();
	if (n != P_Rd || toggleVis || !vis)
		app->mWndPick->setVisible(!vis);
}


///  Road  -------------------------------------------------
void CGui::listPickRd(Mli2 li, size_t pos)
{
	if (pos==ITEM_NONE || pos > data->pre->rd.size())
	{	liRd->setIndexSelected(0);  pos = 0;  }
	
	string s = liRd->getSubItemNameAt(1,pos);
	if (s.length() < 8)  return;
	s = s.substr(7);
	if (s[0] == '-' || s[0] == '=')  return;  // sep

	//  set
	const PRoad* p = data->pre->GetRoad(s);
	// Btn btnRoad[4], btnPipe[4], btnRoadW =0, btnPipeW =0, btnRoadCol =0;
	switch (idRdType)
	{
	case RdPk_Road:
		scn->road->sMtrRoad[idRdPick] = s;
		btnRoad[idRdPick]->setCaption(s);  break;
	case RdPk_Pipe:
		app->scn->road->SetMtrPipe(idRdPick, s);
		btnPipe[idRdPick]->setCaption(s);  break;

	case RdPk_Wall:
		scn->road->sMtrWall = s;
		btnRoadW->setCaption(s);  break;
	case RdPk_PipeW:
		scn->road->sMtrWallPipe = s;
		btnPipeW->setCaption(s);  break;
	case RdPk_Col:
		scn->road->sMtrCol = s;
		btnRoadCol->setCaption(s);  break;
	}	
	//  preset
	if (pSet->pick_setpar && p && idRdType == RdPk_Road)
	{	TerLayer& l = scn->sc->td.layerRoad[idRdPick];
		l.surfName = p->surfName;
		l.dust = p->dust;  l.dustS = p->dustS;
		l.mud = p->mud;  l.tclr = p->tclr;
		listSurf(surfList, idSurf);
	}
	//  upd
	app->scn->road->Rebuild(true);  scn->UpdPSSMMaterials();
	UpdSurfList();
}


///  Sky Mtr  ----------------------------------------------------
void CGui::listPickSky(Mli2 li, size_t pos)
{
	if (pos==ITEM_NONE || pos >= data->pre->sky.size())
	{	liSky->setIndexSelected(0);  pos = 0;  }
	
	string s = liSky->getSubItemNameAt(1,pos);
	//s = "sky/" + s.substr(7);
	s = s.substr(7);
	const PSky* p = data->pre->GetSky(s);  if (!p)  return;

	//  set
	sc->skyMtr = p->mtr;
	if (pSet->pick_setpar)
	{	sc->ldPitch = p->ldPitch;  svSunPitch.Upd();
		sc->ldYaw = p->ldYaw;  svSunYaw.Upd();
		scn->UpdSun();
	}
	//  upd img
	btnSky->setCaption(s);
	app->UpdateTrack();
}

///  Tex Diff  ----------------------------------------------------
void CGui::listPickTex(Mli2 li, size_t pos)
{
	if (pos==ITEM_NONE || pos >= data->pre->ter.size())
	{	liTex->setIndexSelected(0);  pos = 0;  }
	
	string s = liTex->getSubItemNameAt(1,pos);
	s = s.substr(7) + "_d";  // rem #clr
	const PTer* p = data->pre->GetTer(s);  if (!p)  return;
	s += ".jpg";

	//  set
	TerLayer& l = sc->td.layersAll[idTerLay];
	l.texFile = s;
	
	//  auto norm
	{	String sNorm; //old = StringUtil::replaceAll(s,"_d.","_n.");  //_T
		sNorm = p->texNorm+".jpg";

		//  preset
		if (pSet->pick_setpar)
		{	l.tiling = p->tiling;  svTerLScale.Upd();
			l.triplanar = p->triplanar;  ckTerLayTripl.Upd();
			//angMin angMax
			l.surfName = p->surfName;
			l.dust = p->dust;  l.dustS = p->dustS;
			l.mud = p->mud;  l.tclr = p->tclr;
			l.fDamage = p->dmg;  svTerLDmg.Upd();
			listSurf(surfList, idSurf);
		}

		size_t id = cmbTexNorm->findItemIndexWith(sNorm);
		if (id != ITEM_NONE)  // set only if found
		{
			cmbTexNorm->setIndexSelected(id);
			l.texNorm = sNorm;
	}	}

	//  upd img
	btnTexDiff->setCaption(s.substr(0, s.length()-6));  // rem _d.jpg
    imgTexDiff->setImageTexture(s);
	UpdSurfList();
}

///  Grass  -------------------------------------------------------
void CGui::listPickGrs(Mli2 li, size_t pos)
{
	if (pos==ITEM_NONE || pos >= data->pre->gr.size())
	{	liGrs->setIndexSelected(0);  pos = 0;  }
	
	string s = liGrs->getSubItemNameAt(1,pos);
	if (s.length() < 8)  return;
	s = s.substr(7);
	if (s[0] == '-' || s[0] == '=')  return;  // sep

	//  set
	SGrassLayer& l = sc->grLayersAll[idGrLay];
	l.material = s;
	
	//  preset
	const PGrass* p = data->pre->GetGrass(s);
	if (pSet->pick_setpar && p)
	{	l.minSx = p->minSx;  svGrMinX.Upd();
		l.maxSx = p->maxSx;  svGrMaxX.Upd();
		l.minSy = p->minSy;  svGrMinY.Upd();
		l.maxSy = p->maxSy;  svGrMaxY.Upd();
	}
	//  upd img
	btnGrassMtr->setCaption(s);
	imgGrass->setImageTexture(s + ".png");  // same mtr name as tex
}

///  Veget Model  -------------------------------------------------
void CGui::listPickVeg(Mli2 li, size_t pos)
{
	if (pos==ITEM_NONE || pos >= data->pre->veg.size())
	{	liVeg->setIndexSelected(0);  pos = 0;  }
	
	string s = liVeg->getSubItemNameAt(1,pos);
	if (s.length() < 8)  return;
	s = s.substr(7);
	if (s[0] == '-' || s[0] == '=')  return;  // sep
		
	//  set
	const PVeget* p = data->pre->GetVeget(s);
	btnVeget->setCaption(s);
	s += ".mesh";

	auto& l = sc->pgLayersAll[idPgLay];
	l.name = s;
	
	//  preset
	if (pSet->pick_setpar && p)
	{	l.minScale = p->minScale;  svLTrMinSc.Upd();
		l.maxScale = p->maxScale;  svLTrMaxSc.Upd();
		l.windFx = p->windFx;  svLTrWindFx.Upd();
		l.windFy = p->windFy;  svLTrWindFy.Upd();
		l.maxTerAng = p->maxTerAng;  svLTrMaxTerAng.Upd();
		l.maxDepth = p->maxDepth;  svLTrFlDepth.Upd();
		l.addRdist = p->addRdist;  svLTrRdDist.Upd();
	}
	Upd3DView(s);
}
