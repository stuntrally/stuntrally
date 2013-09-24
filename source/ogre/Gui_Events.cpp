#include "pch.h"
#include "common/Def_Str.h"
#include "common/Gui_Def.h"
#include "common/GuiCom.h"
#include "../vdrift/settings.h"
#include "../vdrift/game.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "SplitScreen.h"
#include "common/Slider.h"
#include "common/MultiList2.h"
#include <OgreCamera.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  Gui Events

//    [Car]
void CGui::chkAbs(WP wp)
{	if (pChall && !pChall->abs)  return;	ChkEv(abs[iTireSet]);	if (pGame)  pGame->ProcessNewSettings();	}
void CGui::chkTcs(WP wp)
{	if (pChall && !pChall->tcs)  return;	ChkEv(tcs[iTireSet]);	if (pGame)  pGame->ProcessNewSettings();	}

void CGui::chkGear(Ck*)
{
	if (pGame)  pGame->ProcessNewSettings();
}

void CGui::tabTireSet(MyGUI::TabPtr wp, size_t id)
{
	iTireSet = id;
	// UpdGuiTireSet
	bchAbs->setStateSelected(pSet->abs[iTireSet]);
	bchTcs->setStateSelected(pSet->tcs[iTireSet]);
	Real v = pSet->sss_effect[iTireSet];
	slSSSEff->setValue(v);  valSSSEffect->setCaption(fToStr(v,2,4));
	v = pSet->sss_velfactor[iTireSet];
	slSSSVel->setValue(v/2.f);  valSSSVelFactor->setCaption(fToStr(v,2,4));
	v = pSet->steer_range[iTireSet];
	slSteerRngSurf->setValue(v-0.3f);  valSteerRangeSurf->setCaption(fToStr(v,2,4));
	v = pSet->gui.sim_mode == "easy" ? pSet->steer_sim_easy : pSet->steer_sim_normal;
	slSteerRngSim->setValue(v-0.3f);  valSteerRangeSim->setCaption(fToStr(v,2,4));
}
void CGui::slSSSEffect(SL)
{
	Real v = 1.f * val;  if (bGI)  pSet->sss_effect[iTireSet] = v;
	if (valSSSEffect){	valSSSEffect->setCaption(fToStr(v,2,4));  }
}
void CGui::slSSSVelFactor(SL)
{
	Real v = 2.f * val;  if (bGI)  pSet->sss_velfactor[iTireSet] = v;
	if (valSSSVelFactor){	valSSSVelFactor->setCaption(fToStr(v,2,4));  }
}
void CGui::slSteerRangeSurf(SL)
{
	Real v = val +0.3f;  if (bGI)  pSet->steer_range[iTireSet] = v;
	if (valSteerRangeSurf){		valSteerRangeSurf->setCaption(fToStr(v,2,4));  }
}
void CGui::slSteerRangeSim(SL)
{
	Real v = val +0.3f;  if (bGI)  pSet->steer_range[iTireSet] = v;
	if (valSteerRangeSim){		valSteerRangeSim->setCaption(fToStr(v,2,4));  }
}


//  boost, flip
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
	ButtonPtr chk = wp->castType<MyGUI::Button>();
    chk->setStateSelected(pSet->gui.start_order > 0);
}

void CGui::tabPlayer(TabPtr wp, size_t id)
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
void CGui::UpdCarMClr()
{
	if (!bUpdCarClr || !bGI)  return;
	
	int s = app->carModels.size();
	if (iCurCar == 4)  // ghost
	{
		for (int i=0; i < s; ++i)
			if (app->carModels[i]->isGhost() && !app->carModels[i]->isGhostTrk())  app->carModels[i]->ChangeClr();
	}
	else if (iCurCar == 5)  // track's ghost
	{
		for (int i=0; i < s; ++i)
			if (app->carModels[i]->isGhostTrk())  app->carModels[i]->ChangeClr();
	}else
		if (iCurCar < s)  // player
			app->carModels[iCurCar]->ChangeClr();
}
/*void CGui::sldUpdCarClr()
{
	SV* sv;
	sv= &
}*/
void CGui::slCarClrH(SL)
{
	Real v = val;  if (bGI)  pSet->gui.car_hue[iCurCar] = v;
	if (valCarClrH){	valCarClrH->setCaption(fToStr(v,2,4));  }
	UpdCarMClr();
}
void CGui::slCarClrS(SL)
{
	Real v = val;  if (bGI)  pSet->gui.car_sat[iCurCar] = v;
	if (valCarClrS){	valCarClrS->setCaption(fToStr(v,2,4));  }
	UpdCarMClr();
}
void CGui::slCarClrV(SL)
{
	Real v = val;  if (bGI)  pSet->gui.car_val[iCurCar] = v;
	if (valCarClrV){	valCarClrV->setCaption(fToStr(v,2,4));  }
	UpdCarMClr();
}
void CGui::slCarClrGloss(SL)
{
	Real v = powf(val, 1.6f);  if (bGI)  pSet->gui.car_gloss[iCurCar] = v;
	if (valCarClrGloss){	valCarClrGloss->setCaption(fToStr(v,2,4));  }
	UpdCarMClr();
}
void CGui::slCarClrRefl(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->gui.car_refl[iCurCar] = v;
	if (valCarClrRefl){		valCarClrRefl->setCaption(fToStr(v,2,4));  }
	UpdCarMClr();
}

void CGui::imgBtnCarClr(WP img)
{
	pSet->gui.car_hue[iCurCar] = s2r(img->getUserString("h"));
	pSet->gui.car_sat[iCurCar] = s2r(img->getUserString("s"));
	pSet->gui.car_val[iCurCar] = s2r(img->getUserString("v"));
	pSet->gui.car_gloss[iCurCar]= s2r(img->getUserString("g"));
	pSet->gui.car_refl[iCurCar] = s2r(img->getUserString("r"));
	UpdCarClrSld();
}
void CGui::btnCarClrRandom(WP)
{
	pSet->gui.car_hue[iCurCar] = Math::UnitRandom();
	pSet->gui.car_sat[iCurCar] = Math::UnitRandom();
	pSet->gui.car_val[iCurCar] = Math::UnitRandom();
	pSet->gui.car_gloss[iCurCar] = Math::UnitRandom();
	pSet->gui.car_refl[iCurCar] = Math::RangeRandom(0.3f,1.1f);
	UpdCarClrSld();
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
	if (sv->text)
	switch (pSet->refl_mode)
	{
		case 0: sv->text->setTextColour(Colour(0.0, 1.0, 0.0));  break;
		case 1: sv->text->setTextColour(Colour(1.0, 0.5, 0.0));  break;
		case 2: sv->text->setTextColour(Colour(1.0, 0.0, 0.0));  break;
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
	hud->Size(true);
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
	float v = (int)(val * 6.f +slHalf) * 0.5f;	if (bGI)  pSet->gui.pre_time = v;
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

#define Radio2(bR1,bR2, b)  bR1->setStateSelected(b);  bR2->setStateSelected(!b);

void CGui::radKmh(WP wp){	Radio2(bRkmh, bRmph, true);   pSet->show_mph = false;  hud->Size(true);  }
void CGui::radMph(WP wp){	Radio2(bRkmh, bRmph, false);  pSet->show_mph = true;   hud->Size(true);  }

void CGui::radSimEasy(WP){	Radio2(bRsimEasy, bRsimNorm, true);
	pSet->gui.sim_mode = "easy";	bReloadSim = true;
	tabTireSet(0,iTireSet);  listCarChng(carList,0);
}
void CGui::radSimNorm(WP){	Radio2(bRsimEasy, bRsimNorm, false);
	pSet->gui.sim_mode = "normal";	bReloadSim = true;
	tabTireSet(0,iTireSet);  listCarChng(carList,0);
}

void CGui::chkArrow(Ck*)
{
	if (hud->arrow.nodeRot)
		hud->arrow.nodeRot->setVisible(pSet->check_arrow);
}
void CGui::chkBeam(Ck*)
{
	for (int i=0; i < app->carModels.size(); ++i)
		app->carModels[i]->ShowNextChk(pSet->check_beam);
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

void CGui::chkReverse(Ck*){  gcom->ReadTrkStats();  }

//  graphs
void CGui::chkGraphs(Ck*)
{
	for (int i=0; i < app->graphs.size(); ++i)
		app->graphs[i]->SetVisible(pSet->show_graphs);
}
void CGui::comboGraphs(CMB)
{
	if (valGraphsType)
		valGraphsType->setCaption(toStr(val));
	if (bGI /*&& pSet->graphs_type != v*/)  {
		pSet->graphs_type = (eGraphType)val;
		app->DestroyGraphs();  app->CreateGraphs(); }
}

//  Startup
void CGui::chkMultiThread(WP wp)
{	pSet->multi_thr = pSet->multi_thr ? 0 : 1;  if (wp) {
	ButtonPtr chk = wp->castType<MyGUI::Button>();  chk->setStateSelected(pSet->multi_thr > 0);  }
}


//  [Video]  . . . . . . . . . . . . . . . . . . . .    ---- ------ ----    . . . . . . . . . . . . . . . . . . . .

void CGui::chkVidEffects(WP wp)
{
	ChkEv(all_effects);
	app->recreateCompositor();  //refreshCompositor();
	app->changeShadows();
}
void CGui::chkVidBloom(WP wp)
{		
	ChkEv(bloom);  app->refreshCompositor();
}

void CGui::chkVidHDR(WP wp)
{			
	ChkEv(hdr);  app->refreshCompositor();
}

void CGui::chkVidBlur(WP wp)
{		
	ChkEv(blur);
	app->refreshCompositor();  app->changeShadows();
}
void CGui::chkVidSSAO(WP wp)
{		
	ChkEv(ssao);
	app->refreshCompositor();  app->changeShadows();
}
void CGui::chkVidSoftParticles(WP wp)
{		
	ChkEv(softparticles);
	app->refreshCompositor();  app->changeShadows();
}
void CGui::chkVidDepthOfField(WP wp)
{		
	ChkEv(dof);
	app->refreshCompositor();  app->changeShadows();
}
void CGui::chkVidGodRays(WP wp)
{		
	ChkEv(godrays);
	app->refreshCompositor();  app->changeShadows();
}
void CGui::chkVidBoostFOV(WP wp)
{		
	ChkEv(boost_fov);
}

void CGui::slBloom(SV*)
{
	if (bGI)  app->refreshCompositor();
}


//  [Sound]
void CGui::slVolMaster(SV*)
{
	pGame->ProcessNewSettings();
}
