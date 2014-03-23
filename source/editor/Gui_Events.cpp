#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../ogre/common/CScene.h"
#include "../ogre/common/data/CData.h"
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

//  sun pitch, yaw
void CGui::slUpdSun(SV*)
{
	scn->UpdSun();
}

//  light clrs
void CGui::editLiAmb(Ed ed)
{
	Vector3 c = s2v(ed->getCaption());	sc->lAmb = c;  scn->UpdSun();
	clrAmb->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editLiDiff(Ed ed)
{
	Vector3 c = s2v(ed->getCaption());	sc->lDiff = c;  scn->UpdSun();
	clrDiff->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editLiSpec(Ed ed)
{
	Vector3 c = s2v(ed->getCaption());	sc->lSpec = c;  scn->UpdSun();
	clrSpec->setColour(Colour(c.x,c.y,c.z));
}

//  fog
void CGui::slUpdFog(SV*)
{
	scn->UpdFog();
}

void CGui::editFogClr(Ed ed)
{
	Vector4 c = s2v4(ed->getCaption());  sc->fogClr = c;  scn->UpdFog();
	clrFog->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editFogClr2(Ed ed)
{
	Vector4 c = s2v4(ed->getCaption());  sc->fogClr2 = c;  scn->UpdFog();
	clrFog2->setColour(Colour(c.x,c.y,c.z));
}
void CGui::editFogClrH(Ed ed)
{
	Vector4 c = s2v4(ed->getCaption());  sc->fogClrH = c;  scn->UpdFog();
	clrFogH->setColour(Colour(c.x,c.y,c.z));
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

	else if (n=="SceneryId")  sc->sceneryId = ed->getCaption();
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

void CGui::editTrkDesc(Ed ed)
{
	app->scn->road->sTxtDesc = ed->getCaption();
}

void CGui::comboPipeMtr(Cmb cmb, size_t val)
{
	String sn = cmb->getName().substr(String("RdMtrP").length(), cmb->getName().length());
	int id = atoi(sn.c_str())-1;  if (id < 0 || id >= MTRs)  return;

	String s = cmb->getItemNameAt(val);
	app->scn->road->SetMtrPipe(id, s);  app->scn->road->RebuildRoad(true);  scn->UpdPSSMMaterials();
	UpdSurfList();
}

void CGui::comboRoadWMtr(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	app->scn->road->sMtrWall = s;  app->scn->road->RebuildRoad(true);  scn->UpdPSSMMaterials();
}
void CGui::comboPipeWMtr(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	app->scn->road->sMtrWallPipe = s;  app->scn->road->RebuildRoad(true);  scn->UpdPSSMMaterials();
}
void CGui::comboRoadColMtr(Cmb cmb, size_t val)
{
	String s = cmb->getItemNameAt(val);
	app->scn->road->sMtrCol = s;  app->scn->road->RebuildRoad(true);  scn->UpdPSSMMaterials();
}

void CGui::editRoad(Ed ed)
{
	if (!app->scn->road)  return;
	Real r = s2r(ed->getCaption());
	String n = ed->getName();

		 if (n=="RdHeightOfs")	app->scn->road->fHeight = r;
	else if (n=="RdSkirtLen")	app->scn->road->skirtLen = r;else if (n=="RdSkirtH")	app->scn->road->skirtH = r;
	//app->scn->road->RebuildRoad(true);  //on Enter ?..
}

//  set slider pointer values, and update
void CGui::SldUpd_Road()
{
	if (!app->scn->road)  return;
	SplineRoad& r = *app->scn->road;
	
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
	if (app->scn->road)
	{	app->scn->road->fMarkerScale = pSet->road_sphr;
		app->scn->road->UpdAllMarkers();  }
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

void CGui::listSurf(Li, size_t id)
{
	if (id == ITEM_NONE) {  id = 0;  surfList->setIndexSelected(0);  }
	if (id > 4) {  id = 4;  surfList->setIndexSelected(4);  }  ///TODO: temp 1 road mtr ...

	idSurf = id;  // help var
	TerLayer* l = idSurf < 4 ? &sc->td.layersAll[idSurf] : &sc->td.layerRoad;

	//  Particles
	edLDust->setCaption(toStr(l->dust));	edLDustS->setCaption(toStr(l->dustS));
	edLMud->setCaption(toStr(l->mud));	edLSmoke->setCaption(toStr(l->smoke));
	edLTrlClr->setCaption(toStr(l->tclr));  clrTrail->setColour(Colour(l->tclr.r,l->tclr.g,l->tclr.b));
	
	//  Surface
	cmbSurface->setIndexSelected( cmbSurface->findItemIndexWith(l->surfName));
	UpdSurfInfo();
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
}


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

void CGui::wheelTex(WP wp, int rel){  int r = rel < 0 ? 1 : -1;  listPickTex(liTex, liNext(liTex, r));  }
void CGui::wheelGrs(WP wp, int rel){  int r = rel < 0 ? 1 : -1;  listPickGrs(liGrs, liNext(liGrs, r));  }
void CGui::wheelVeg(WP wp, int rel){  int r = rel < 0 ? 1 : -1;  listPickVeg(liVeg, liNext(liVeg, r));  }
void CGui::wheelRd(WP wp, int rel){   int r = rel < 0 ? 1 : -1;  listPickRd(liRd, liNext(liRd, r));  }

void CGui::btnPickTex(WP){    PickShow(0);  }
void CGui::btnPickGrass(WP){  PickShow(1);  }
void CGui::btnPickVeget(WP){  PickShow(2);  }
void CGui::btnPickRoad(WP wp)
{
	if (!wp) {  PickShow(3, true);  return;  }
	String sn = wp->getName().substr(String("RdMtr").length(), wp->getName().length());
	int idRdOld = idRdPick;  idRdPick = atoi(sn.c_str())-1;
	PickShow(3, idRdOld==idRdPick);
}

//  show Pick window
void CGui::PickShow(int n, bool toggleVis)
{
	liTex->setVisible(n==0);
	liGrs->setVisible(n==1);
	liVeg->setVisible(n==2);
	liRd->setVisible(n==3);
	panPick->setPosition(liPickW[n], 36);  
	
	const int wx = pSet->windowx, wy = pSet->windowy;
	//if (pSet->pick_center
	switch (n)  ///pick dim
	{	case 0:  app->mWndPick->setCoord(wx*0.45f, 0.04f*wy, 420, 0.95f*wy);  break;
		case 1:  app->mWndPick->setCoord(wx*0.36f, 0.04f*wy, 380, 0.95f*wy);  break;
		case 2:  app->mWndPick->setCoord(wx*0.36f, 0.04f*wy, 420, 0.95f*wy);  break;
		case 3:  app->mWndPick->setCoord(wx*0.36f, 0.04f*wy, 420, 0.95f*wy);  break;
	}
	if (n==3)  // upd pick road
	{	UString s = btnRoad[idRdPick]->getCaption();
		for (int i=0; i < liRd->getItemCount(); ++i)
			if (liRd->getSubItemNameAt(1,i).substr(7) == s)
				liRd->setIndexSelected(i);
	}

	bool vis = app->mWndPick->getVisible();
	if (n != 3 || toggleVis || !vis)
		app->mWndPick->setVisible(!vis);
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
	//if (bTexNormAuto)
	{	String sNorm = StringUtil::replaceAll(s,"_d.","_n.");  //_T
		sNorm = p->texNorm+".jpg";

		//  preset
		if (pSet->pick_setpar)
		{	l.tiling = p->tiling;  svTerLScale.Upd();
			l.triplanar = p->triplanar;  ckTerLayTripl.Upd();
			//angMin angMax
			l.surfName = p->surfName;
			l.dust = p->dust;  l.dustS = p->dustS;
			l.mud = p->mud;  l.tclr = p->tclr;
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
	s = s.substr(7);
	const PGrass* p = data->pre->GetGrass(s);

	//  set
	SGrassLayer& l = sc->grLayersAll[idGrLay];
	l.material = s;
	
	//  preset
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
	s = s.substr(7);
	const PVeget* p = data->pre->GetVeget(s);

	//  upd
	btnVeget->setCaption(s);
	s += ".mesh";

	//  set
	PagedLayer& l = sc->pgLayersAll[idPgLay];
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

///  Road  -------------------------------------------------
void CGui::listPickRd(Mli2 li, size_t pos)
{
	if (pos==ITEM_NONE || pos > data->pre->rd.size())
	{	liRd->setIndexSelected(0);  pos = 0;  }
	
	string s = liRd->getSubItemNameAt(1,pos);
	s = s.substr(7);
	const PRoad* p = data->pre->GetRoad(s);

	//  set
	scn->road->sMtrRoad[idRdPick] = s;
	//  preset
	if (pSet->pick_setpar && p)
	{	TerLayer& l = scn->sc->td.layerRoad;  //[idRdPick]
		l.surfName = p->surfName;
		l.dust = p->dust;  l.dustS = p->dustS;
		l.mud = p->mud;  l.tclr = p->tclr;
		listSurf(surfList, idSurf);
	}
	//  upd
	btnRoad[idRdPick]->setCaption(s);
	app->scn->road->RebuildRoad(true);  scn->UpdPSSMMaterials();
	UpdSurfList();
}
