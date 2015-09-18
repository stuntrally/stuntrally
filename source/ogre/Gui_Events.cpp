#include "pch.h"
#include "common/Def_Str.h"
#include "common/Gui_Def.h"
#include "common/GuiCom.h"
#include "common/CScene.h"
#include "../vdrift/settings.h"
#include "../vdrift/game.h"
#include "../road/PaceNotes.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "SplitScreen.h"
#include "common/Slider.h"
#include "common/MultiList2.h"
#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include "../ogre/common/RenderBoxScene.h"
using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  Gui Events

//    [Car]
//---------------------------------------------------------------------
void CGui::chkAbs(WP wp)
{	if (pChall && !pChall->abs)  return;	ChkEv(abs[iTireSet]);	if (pGame)  pGame->ProcessNewSettings();	}
void CGui::chkTcs(WP wp)
{	if (pChall && !pChall->tcs)  return;	ChkEv(tcs[iTireSet]);	if (pGame)  pGame->ProcessNewSettings();	}

void CGui::chkGear(Ck*)
{
	if (pGame)  pGame->ProcessNewSettings();
}

//  reset to same as in default.cfg
void CGui::btnSSSReset(WP)
{
	pSet->sss_effect[0] = 0.574f;  pSet->sss_velfactor[0] = 0.626f;
	pSet->sss_effect[1] = 0.650f;  pSet->sss_velfactor[1] = 0.734f;
	SldUpd_TireSet();  slSSS(0);
}
void CGui::btnSteerReset(WP)
{
	pSet->steer_range[0] = 1.00f;  pSet->steer_sim[0] = 0.71f;
	pSet->steer_range[1] = 0.76f;  pSet->steer_sim[1] = 1.00f;
	SldUpd_TireSet();  slSSS(0);
}

//  sss sliders,  upd graph
void CGui::slSSS(SV* sv)
{
	///  upd sss graph
	float xs = 5.f, yo = 166.f, x2 = 500.f;
	const IntSize& wi = app->mWndOpts->getSize();
	const float sx = wi.width/1248.f, sy = wi.height/935.f;
	xs *= sx;  yo *= sy;  x2 *= sx;

	std::vector<FloatPoint> points,grid;
	float vmax = 300.f;  // kmh
	const int ii = 200;
	int i;
	for (i = 0; i <= ii; ++i)
	{	float v = float(i)/ii * vmax / 3.6f;  // to m/s
		float f = CARCONTROLMAP_LOCAL::GetSSScoeff(v, pSet->sss_velfactor[iTireSet], pSet->sss_effect[iTireSet]);
		points.push_back(FloatPoint(v * xs, yo - yo * f));
	}
	graphSSS->setPoints(points);

	//  grid lines
	const int y1 = yo +10, y2 = -10, x1 = -10;  //outside
	for (i = 0; i < 6; ++i)  // ||
	{	float v = i * 50.f / 3.6f;
		grid.push_back(FloatPoint(v * xs,  i%2==0 ? y1 : y2));
		grid.push_back(FloatPoint(v * xs,  i%2==0 ? y2 : y1));
	}
	grid.push_back(FloatPoint(x2, y1));
	grid.push_back(FloatPoint(x1, y1));
	for (i = 0; i < 4; ++i)  // ==
	{	grid.push_back(FloatPoint(i%2==0 ? x1 : x2,  yo - yo * i * 0.25f));
		grid.push_back(FloatPoint(i%2==0 ? x2 : x1,  yo - yo * i * 0.25f));
	}
	graphSGrid->setPoints(grid);
}

//  gravel/asphalt
void CGui::tabTireSet(Tab, size_t id)
{
	iTireSet = id;
	SldUpd_TireSet();
	bchAbs->setStateSelected(pSet->abs[id]);
	bchTcs->setStateSelected(pSet->tcs[id]);
	slSSS(0);
}

void CGui::SldUpd_TireSet()
{
	int i = iTireSet;
	svSSSEffect.UpdF(&pSet->sss_effect[i]);
	svSSSVelFactor.UpdF(&pSet->sss_velfactor[i]);
	svSteerRangeSurf.UpdF(&pSet->steer_range[i]);
	svSteerRangeSim.UpdF(&pSet->steer_sim[pSet->gui.sim_mode == "easy" ? 0 : 1]);
}

//  player
void CGui::tabPlayer(Tab, size_t id)
{
	iCurCar = id;
	//  update gui for this car (color h,s,v, name, img)
	bool plr = iCurCar < 4;
	if (plr)
	{
		string c = pSet->gui.car[iCurCar];
		for (size_t i=0; i < carList->getItemCount(); ++i)
		if (carList->getItemNameAt(i).substr(7) == c)
		{	carList->setIndexSelected(i);
			listCarChng(carList, i);
	}	}
	carList->setVisible(plr);

	UpdCarClrSld(false);  // no car color change
}

//  car color
//---------------------------------------------------------------------
//  3. apply new color to car/ghost
void CGui::SetCarClr()
{
	if (!bGI)  return;
	
	int s = app->carModels.size(), i;
	if (iCurCar == 4)  // ghost
	{
		for (i=0; i < s; ++i)
			if (app->carModels[i]->isGhost() && !app->carModels[i]->isGhostTrk())
				app->carModels[i]->ChangeClr();
	}
	else if (iCurCar == 5)  // track's ghost
	{
		for (i=0; i < s; ++i)
			if (app->carModels[i]->isGhostTrk())
				app->carModels[i]->ChangeClr();
	}else
		if (iCurCar < s)  // player
			app->carModels[iCurCar]->ChangeClr();
}

//  2. upd game set color and sliders
void CGui::UpdCarClrSld(bool upd)
{
	SldUpd_CarClr();
	int i = iCurCar;
	pSet->game.car_hue[i] = pSet->gui.car_hue[i];  // copy to apply
	pSet->game.car_sat[i] = pSet->gui.car_sat[i];
	pSet->game.car_val[i] = pSet->gui.car_val[i];
	pSet->game.car_gloss[i]= pSet->gui.car_gloss[i];
	pSet->game.car_refl[i] = pSet->gui.car_refl[i];
	if (upd)
		SetCarClr();
}

//  1. upd sld and pointers after tab change
void CGui::SldUpd_CarClr()
{
	int i = iCurCar;
	svCarClrH.UpdF(&pSet->gui.car_hue[i]);
	svCarClrS.UpdF(&pSet->gui.car_sat[i]);
	svCarClrV.UpdF(&pSet->gui.car_val[i]);
	svCarClrGloss.UpdF(&pSet->gui.car_gloss[i]);
	svCarClrRefl.UpdF(&pSet->gui.car_refl[i]);
}

void CGui::slCarClr(SV*)
{
	SetCarClr();
}

//  color buttons
void CGui::imgBtnCarClr(WP img)
{
	int i = iCurCar;
	pSet->gui.car_hue[i] = s2r(img->getUserString("h"));
	pSet->gui.car_sat[i] = s2r(img->getUserString("s"));
	pSet->gui.car_val[i] = s2r(img->getUserString("v"));
	pSet->gui.car_gloss[i]= s2r(img->getUserString("g"));
	pSet->gui.car_refl[i] = s2r(img->getUserString("r"));
	UpdCarClrSld();
}
void CGui::btnCarClrRandom(WP)
{
	int i = iCurCar;
	pSet->gui.car_hue[i] = Math::UnitRandom();
	pSet->gui.car_sat[i] = Math::UnitRandom();
	pSet->gui.car_val[i] = Math::UnitRandom();
	pSet->gui.car_gloss[i]= Math::UnitRandom();
	pSet->gui.car_refl[i] = Math::RangeRandom(0.3f,1.1f);
	UpdCarClrSld();
}


//  [Game]
//---------------------------------------------------------------------

void CGui::comboBoost(CMB)
{
	pSet->gui.boost_type = val;  app->hud->Show();
}
void CGui::comboFlip(CMB)
{
	pSet->gui.flip_type = val;
}
void CGui::comboDamage(CMB)
{
	pSet->gui.damage_type = val;
}
void CGui::comboRewind(CMB)
{
	pSet->gui.rewind_type = val;
}
	
void CGui::radKmh(WP wp){	Radio2(bRkmh, bRmph, true);   pSet->show_mph = false;  hud->Size();  if (app->scn->pace) app->scn->pace->UpdTxt();  }
void CGui::radMph(WP wp){	Radio2(bRkmh, bRmph, false);  pSet->show_mph = true;   hud->Size();  if (app->scn->pace) app->scn->pace->UpdTxt();  }

void CGui::setSimMode(std::string mode)
{
	pSet->gui.sim_mode = mode;  bReloadSim = true;
	bRsimEasy->setStateSelected(mode == "easy");
	bRsimNorm->setStateSelected(mode == "normal");
	if (bRsimHard)
		bRsimHard->setStateSelected(mode == "hard");
	tabTireSet(0,iTireSet);  listCarChng(carList,0);
}

void CGui::radSimEasy(WP){	setSimMode("easy");  }
void CGui::radSimNorm(WP){	setSimMode("normal");  }
void CGui::radSimHard(WP){	setSimMode("hard");  }

void CGui::btnNumPlayers(WP wp)
{
	if      (wp->getName() == "btnPlayers1")  pSet->gui.local_players = 1;
	else if (wp->getName() == "btnPlayers2")  pSet->gui.local_players = 2;
	else if (wp->getName() == "btnPlayers3")  pSet->gui.local_players = 3;
	else if (wp->getName() == "btnPlayers4")  pSet->gui.local_players = 4;
	if (valLocPlayers)  valLocPlayers->setCaption(toStr(pSet->gui.local_players));
}

void CGui::chkStartOrd(WP wp)
{
	pSet->gui.start_order = pSet->gui.start_order==0 ? 1 : 0;
	Btn chk = wp->castType<Button>();
    chk->setStateSelected(pSet->gui.start_order > 0);
}


//  [Graphics]  options  (game only)
//---------------------------------------------------------------------

//  reflection
void CGui::slReflDist(SV*)
{
	app->recreateReflections();
}
void CGui::slReflMode(SV* sv)
{
	if (bGI)
	switch (pSet->refl_mode)
	{
		case 0: sv->setTextClr(0.0, 1.0, 0.0);  break;
		case 1: sv->setTextClr(1.0, 0.5, 0.0);  break;
		case 2: sv->setTextClr(1.0, 0.0, 0.0);  break;
	}
	app->recreateReflections();
}
void App::recreateReflections()
{
	for (std::vector<CarModel*>::iterator it = carModels.begin(); it!=carModels.end(); it++)
	{	
		delete (*it)->pReflect;
		(*it)->CreateReflection();
	}
}

void CGui::chkParTrl(Ck*)
{		
	for (std::vector<CarModel*>::iterator it=app->carModels.begin(); it!=app->carModels.end(); it++)
		(*it)->UpdParsTrails();
}


//  [View] size  . . . . . . . . . . . . . . . . . . . .
void CGui::slHudSize(SV*)
{
	hud->Size();
}
void CGui::slHudCreate(SV*)
{
	hud->Destroy();  hud->Create();
}
void CGui::chkHudCreate(Ck*)
{
	hud->Destroy();  hud->Create();
}

void CGui::slSizeArrow(SV*)
{
	float v = pSet->size_arrow * 0.5f;
	if (hud->arrow.nodeRot)
		hud->arrow.nodeRot->setScale(v * Vector3::UNIT_SCALE);
}
void CGui::slCountdownTime(SL)
{
	float v = (int)(val * 10.f +slHalf) * 0.5f;	if (bGI)  pSet->gui.pre_time = v;
	if (valCountdownTime){	valCountdownTime->setCaption(fToStr(v,1,4));  }
}


//  [View]  . . . . . . . . . . . . . . . . . . . .    ---- checks ----    . . . . . . . . . . . . . . . . . . . .

void CGui::chkWireframe(Ck*)
{
	bool b = app->mbWireFrame;
	
	///  Set for all cameras
	PolygonMode mode = b ? PM_WIREFRAME : PM_SOLID;
	
	app->refreshCompositor(b);  // disable effects

	if (app->mSplitMgr)
	for (std::list<Camera*>::iterator it=app->mSplitMgr->mCameras.begin(); it!=app->mSplitMgr->mCameras.end(); ++it)
		(*it)->setPolygonMode(mode);
	
	if (app->ndSky)
		app->ndSky->setVisible(!b);  // hide sky
}

//  Hud
void CGui::chkHudShow(Ck*)
{
	hud->Show();
}

void CGui::chkArrow(Ck*)
{
	if (hud->arrow.nodeRot)
		hud->arrow.nodeRot->setVisible(pSet->check_arrow && !app->bHideHudArr);
}
void CGui::chkBeam(Ck*)
{
	for (int i=0; i < app->carModels.size(); ++i)
		app->carModels[i]->ShowNextChk(pSet->check_beam && !app->bHideHudBeam);
}

//  hud minimap
void CGui::chkMinimap(Ck*)
{
	for (int c=0; c < hud->hud.size(); ++c)
		if (hud->hud[c].ndMap)
			hud->hud[c].ndMap->setVisible(pSet->trackmap);
}

void CGui::chkMiniUpd(Ck*)
{
	hud->UpdMiniTer();
}

//  pacenotes
void CGui::slUpd_Pace(SV*)
{
	app->scn->UpdPaceParams();
}

void CGui::chkReverse(Ck*){  gcom->ReadTrkStats();  }

//  graphs
void CGui::chkGraphs(Ck*)
{
	bool te = pSet->graphs_type == Gh_TireEdit;
	for (int i=0; i < app->graphs.size(); ++i)
		app->graphs[i]->SetVisible(!te ? pSet->show_graphs :  // reference vis
			pSet->show_graphs && (i < 2*App::TireNG || i >= 4*App::TireNG || pSet->te_reference));
}
void CGui::comboGraphs(CMB)
{
	if (valGraphsType)
		valGraphsType->setCaption(toStr(val));
	if (bGI /*&& pSet->graphs_type != v*/)  {
		pSet->graphs_type = (eGraphType)val;
		app->DestroyGraphs();  app->CreateGraphs();  }
}

//  Startup
void CGui::chkMultiThread(WP wp)
{
	pSet->multi_thr = pSet->multi_thr ? 0 : 1;
	Btn chk = wp->castType<Button>();
	chk->setStateSelected(pSet->multi_thr > 0);
}


//  [Effects]  . . . . . . . . . . . . . . . . . . . .    ---- ------ ----    . . . . . . . . . . . . . . . . . . . .

void CGui::chkAllEffects(Ck*)
{
	app->recreateCompositor();  //app->refreshCompositor();
	app->scn->changeShadows();
}
void CGui::chkEffUpd(Ck*)
{		
	app->refreshCompositor();
}
void CGui::chkEffUpdShd(Ck*)
{
	app->refreshCompositor();
	app->scn->changeShadows();
}

void CGui::slEffUpd(SV*)
{
	if (bGI)  app->refreshCompositor();
}


//  [Sound]
void CGui::slVolMaster(SV*)
{
	pGame->ProcessNewSettings();
}

void CGui::slVolHud(SV*)
{
	pGame->UpdHudSndVol();
}


//  Hints, welcome screen
//---------------------------------------------------------------------
const static char hOrd[/*CGui::iHints*/17]={0,1,2,3,4,5,16,6,7,8,9,10,11,12,13,14, 15};
void CGui::UpdHint()
{
	if (!edHintTitle)  return;
	int h = hOrd[iHintCur];
	edHintTitle->setCaption(TR("#C0E0FF#{Hint}  #A0D0FF") +toStr(iHintCur+1)+"/"+toStr(iHints)+
					  ":   "+TR("#D0E8FF#{Hint-"+toStr(h)+"}"));
	edHintText->setCaption(TR("#{Hint-"+toStr(h)+"text}"));
}

void CGui::btnHintPrev(WP)
{
	iHintCur = (iHintCur-1+iHints) % iHints;  UpdHint();
}
void CGui::btnHintNext(WP)
{
	iHintCur = (iHintCur+1) % iHints;         UpdHint();
}

void CGui::btnHintScreen(WP)
{
	GuiShortcut(MNU_Options, TABo_Screen,0);  btnHintClose(0);
}
void CGui::btnHintInput(WP)
{
	GuiShortcut(MNU_Options, TABo_Input,0);  btnHintClose(0);
}

void CGui::btnHintClose(WP)
{
	app->mWndWelcome->setVisible(false);
}



///  3d car view  TODO ...
//--------------------------------------------

IntCoord CGui::GetViewSize()
{
	IntCoord ic = app->mWndGame->getClientCoord();
	return IntCoord(ic.width*0.56f, ic.height*0.38f, ic.width*0.43f, ic.height*0.57f);
}

void CGui::InitCarPrv()
{
	viewCanvas = app->mWndGame->createWidget<Canvas>("Canvas", GetViewSize(), Align::Stretch);
	viewCanvas->setInheritsAlpha(false);
	viewCanvas->setPointer("hand");
	viewCanvas->setVisible(true);
	viewBox->setCanvas(viewCanvas);
	viewBox->setBackgroundColour(Colour(0.32,0.35,0.37,1));
	//viewBox->setAutoRotation(true);
	viewBox->setMouseRotation(true);

	//viewBox->injectObject("sphere.mesh");
	viewCar = new CarModel(3, 0, CarModel::CT_GHOST, "XZ", viewBox->mScene, pSet, pGame, app->scn->sc, 0, app);
	viewCar->Load();
	viewCar->Create();
	viewCar->ChangeClr();

	PosInfo p;  p.bNew = true;
	p.pos = Vector3(0,0,0);
	p.rot = Quaternion(Degree(180),Vector3(1,0,0)) * Quaternion(Degree(50),Vector3(0,1,0));
	p.whPos[0] = Vector3(0,-1,-1);  p.whRot[0] = p.rot;
	
	viewCar->Update(p, p, 0.f);
	viewBox->mCamera->setPosition(Vector3(0,4,-7));
	viewBox->mCamera->setDirection(-Vector3(0,4,-7));
	//viewBox->mCameraNode->setPosition(Vector3(0,2,4));
	//viewBox->mCameraNode->lookAt(Vector3(0,0,0), Node::TS_WORLD);
	//viewBox->updateViewport();
}
