#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/settings.h"
#include "../vdrift/game.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "SplitScreen.h"
#include "common/Gui_Def.h"
#include "common/Slider.h"
#include "common/MultiList2.h"
using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  Gui Events

//    [Car]
void CGui::chkAbs(WP wp){	if (pChall /*&& !pChall->abs*/)  return;
	ChkEv(abs[iTireSet]);	if (pGame)  pGame->ProcessNewSettings();	}
void CGui::chkTcs(WP wp){	if (pChall /*&& !pChall->tcs*/)  return;
	ChkEv(tcs[iTireSet]);	if (pGame)  pGame->ProcessNewSettings();	}

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

void CGui::chkGear(WP wp){		ChkEv(autoshift);	if (pGame)  pGame->ProcessNewSettings();	}
void CGui::chkRear(WP wp){		ChkEv(autorear);	if (pGame)  pGame->ProcessNewSettings();	}
void CGui::chkRearInv(WP wp){	ChkEv(rear_inv);	if (pGame)  pGame->ProcessNewSettings();	}


//    [Game]
void CGui::chkVegetCollis(WP wp){	ChkEv(gui.collis_veget);	}
void CGui::chkCarCollis(WP wp){		ChkEv(gui.collis_cars);		}
void CGui::chkRoadWCollis(WP wp){	ChkEv(gui.collis_roadw);	}
void CGui::chkDynObjects(WP wp){		ChkEv(gui.dyn_objects);		}

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
void CGui::chkSplitVert(WP wp)
{
	ChkEv(split_vertically); 
}

void CGui::chkStartOrd(WP wp)
{
	pSet->gui.start_order = pSet->gui.start_order==0 ? 1 : 0;
	ButtonPtr chk = wp->castType<MyGUI::Button>();
    chk->setStateSelected(pSet->gui.start_order > 0);
}

void CGui::slNumLaps(SL)
{
	int v = 20.f * val + 1 +slHalf;  if (bGI)  pSet->gui.num_laps = v;
	if (valNumLaps){  valNumLaps->setCaption(toStr(v));  }
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


//  [Graphics]
//---------------------------------------------------------------------

//  particles/trails
void CGui::slParticles(SL)
{
	Real v = 4.f * powf(val, 2.f);  if (bGI)  pSet->particles_len = v;
	if (valParticles){	valParticles->setCaption(fToStr(v,2,4));  }
}
void CGui::slTrails(SL)
{
	Real v = 4.f * powf(val, 2.f);  if (bGI)  pSet->trails_len = v;
	if (valTrails){		valTrails->setCaption(fToStr(v,2,4));  }
}

//  reflect
void CGui::slReflSkip(SL)
{
	int v = 1000.f * powf(val, 2.f) +slHalf;	if (bGI)  pSet->refl_skip = v;
	if (valReflSkip)  valReflSkip->setCaption(toStr(v));
}
void CGui::slReflSize(SL)
{
	int v = std::max( 0.0f, std::min((float) ciShadowNumSizes-1, ciShadowNumSizes * val)) +slHalf;
	if (bGI)  pSet->refl_size = v;
	if (valReflSize)  valReflSize->setCaption(toStr(ciShadowSizesA[v]));
}
void CGui::slReflFaces(SL)
{
	int v = val * 6.f +slHalf;
	if (bGI)  pSet->refl_faces = v;
	if (valReflFaces)  valReflFaces->setCaption(toStr(v));
}
void CGui::slReflDist(SL)
{
	float v = 20.f + 1480.f * powf(val, 2.f);	if (bGI)  pSet->refl_dist = v;
	if (valReflDist){	valReflDist->setCaption(fToStr(v,0,4)+" m");  }
	
	app->recreateReflections();
}
void CGui::slReflMode(SL)
{
	int old = pSet->refl_mode;
	pSet->refl_mode = val * 2.f +slHalf;
	
	if (pSet->refl_mode != old)
		app->recreateReflections();
		
	if (valReflMode)
	{
		switch (pSet->refl_mode)
		{
		case 0: valReflMode->setCaption( TR("#{ReflMode_static}") );  valReflMode->setTextColour(MyGUI::Colour(0.0, 1.0, 0.0));  break;
		case 1: valReflMode->setCaption( TR("#{ReflMode_single}") );  valReflMode->setTextColour(MyGUI::Colour(1.0, 0.5, 0.0));  break;
		case 2: valReflMode->setCaption( TR("#{ReflMode_full}") );  valReflMode->setTextColour(MyGUI::Colour(1.0, 0.0, 0.0));  break;
		}
	}
}
void App::recreateReflections()
{
	for (std::vector<CarModel*>::iterator it = carModels.begin(); it!=carModels.end(); it++)
	{	
		delete (*it)->pReflect;
		(*it)->CreateReflection();
	}
}


//  [View] size
void CGui::slSizeGaug(SL)
{
	float v = 0.1f + 0.15f * val;	if (bGI)  {  pSet->size_gauges = v;  hud->Size(true);  }
	if (valSizeGaug)	valSizeGaug->setCaption(fToStr(v,3,4));
}
void CGui::slTypeGaug(SL)
{	int old = pSet->gauges_type;
	int v = val * 5.f +slHalf;		if (bGI && v != old)  {  pSet->gauges_type = v;  hud->Destroy();  hud->Create();  }
	if (valTypeGaug)	valTypeGaug->setCaption(toStr(v));
}
void CGui::slLayoutGaug(SL)
{	int old = pSet->gauges_layout;
	int v = val * 2.0f +slHalf;		if (bGI && v != old)  {  pSet->gauges_layout = v;  hud->Destroy();  hud->Create();  }
	if (valLayoutGaug)	valLayoutGaug->setCaption(toStr(v));
}
void CGui::slSizeArrow(SL)
{
	float v = val;	if (bGI)  {  pSet->size_arrow = v;  }
	if (valSizeArrow)	valSizeArrow->setCaption(fToStr(v,3,4));
	if (hud->arrow.nodeRot)  hud->arrow.nodeRot->setScale(v/2.f, v/2.f, v/2.f);
}
void CGui::slCountdownTime(SL)
{
	float v = (int)(val * 6.f +slHalf) * 0.5f;	if (bGI)  {  pSet->gui.pre_time = v;  }
	if (valCountdownTime){	valCountdownTime->setCaption(fToStr(v,1,4));  }
}

//  minimap
void CGui::slSizeMinimap(SL)
{
	float v = 0.05f + 0.25f * val;	if (bGI)  {  pSet->size_minimap = v;  hud->Size(true);  }
	if (valSizeMinimap)  valSizeMinimap->setCaption(fToStr(v,3,4));
}
void CGui::slZoomMinimap(SL)
{
	float v = 1.f + 9.f * powf(val, 2.f);	if (bGI)  {  pSet->zoom_minimap = v;  hud->Size(true);  }
	if (valZoomMinimap)  valZoomMinimap->setCaption(fToStr(v,3,4));
}


//  [Sound]
void CGui::slVolMaster(SL)
{
	Real v = 1.6f * val;	if (bGI)  {  pSet->vol_master = v;  pGame->ProcessNewSettings();  }
	if (valVolMaster)  valVolMaster->setCaption(fToStr(v,2,4));
}
void CGui::slVolEngine(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_engine = v;	if (valVolEngine)  valVolEngine->setCaption(fToStr(v,2,4));
}
void CGui::slVolTires(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_tires = v;	if (valVolTires)  valVolTires->setCaption(fToStr(v,2,4));
}
void CGui::slVolSusp(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_susp = v;		if (valVolSusp)  valVolSusp->setCaption(fToStr(v,2,4));
}
void CGui::slVolEnv(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_env = v;		if (valVolEnv)  valVolEnv->setCaption(fToStr(v,2,4));
}
void CGui::slVolFlSplash(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_fl_splash = v;	if (valVolFlSplash)  valVolFlSplash->setCaption(fToStr(v,2,4));
}
void CGui::slVolFlCont(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_fl_cont = v;		if (valVolFlCont)  valVolFlCont->setCaption(fToStr(v,2,4));
}
void CGui::slVolCarCrash(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_car_crash = v;	if (valVolCarCrash)  valVolCarCrash->setCaption(fToStr(v,2,4));
}
void CGui::slVolCarScrap(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_car_scrap = v;	if (valVolCarScrap)  valVolCarScrap->setCaption(fToStr(v,2,4));
}


//  [View]  . . . . . . . . . . . . . . . . . . . .    ---- checks ----    . . . . . . . . . . . . . . . . . . . .

void CGui::chkParticles(WP wp)
{		
	ChkEv(particles);
	for (std::vector<CarModel*>::iterator it=app->carModels.begin(); it!=app->carModels.end(); it++)
		(*it)->UpdParsTrails();
}
void CGui::chkTrails(WP wp)
{			
	ChkEv(trails);		
	for (std::vector<CarModel*>::iterator it=app->carModels.begin(); it!=app->carModels.end(); it++)
		(*it)->UpdParsTrails();
}
void CGui::toggleWireframe()
{
	bool& b = app->mbWireFrame;  b = !b;
	if (chWire)  chWire->setStateSelected(b);
	
	///  Set for all cameras
	PolygonMode mode = b ? PM_WIREFRAME : PM_SOLID;
	
	app->refreshCompositor(mode == PM_WIREFRAME);  // disable effects
	if (app->mSplitMgr)
	for (std::list<Camera*>::iterator it=app->mSplitMgr->mCameras.begin(); it!=app->mSplitMgr->mCameras.end(); ++it)
		(*it)->setPolygonMode(mode);
	
	if (app->ndSky)	app->ndSky->setVisible(!b);  // hide sky
}
//  hud
void CGui::chkDigits(WP wp){ 		ChkEv(show_digits);  hud->Show();  }
void CGui::chkGauges(WP wp){			ChkEv(show_gauges);	 hud->Show();  }

void CGui::radKmh(WP wp){	bRkmh->setStateSelected(true);  bRmph->setStateSelected(false);  pSet->show_mph = false;  hud->Size(true);  }
void CGui::radMph(WP wp){	bRkmh->setStateSelected(false);  bRmph->setStateSelected(true);  pSet->show_mph = true;   hud->Size(true);  }

void CGui::radSimEasy(WP){	bRsimEasy->setStateSelected(true);  bRsimNorm->setStateSelected(false);
	pSet->gui.sim_mode = "easy";	bReloadSim = true;
	tabTireSet(0,iTireSet);  listCarChng(carList,0);
}
void CGui::radSimNorm(WP){	bRsimEasy->setStateSelected(false);  bRsimNorm->setStateSelected(true);
	pSet->gui.sim_mode = "normal";	bReloadSim = true;
	tabTireSet(0,iTireSet);  listCarChng(carList,0);
}

void CGui::chkArrow(WP wp){			ChkEv(check_arrow);
	if (hud->arrow.nodeRot)  hud->arrow.nodeRot->setVisible(pSet->check_arrow);
}
void CGui::chkBeam(WP wp){			ChkEv(check_beam);
	for (int i=0; i < app->carModels.size(); ++i)  app->carModels[i]->ShowNextChk(pSet->check_beam);
}

void CGui::chkMinimap(WP wp){		ChkEv(trackmap);
	for (int c=0; c < hud->hud.size(); ++c)
		if (hud->hud[c].ndMap)  hud->hud[c].ndMap->setVisible(pSet->trackmap);
}
void CGui::chkMiniZoom(WP wp){		ChkEv(mini_zoomed);		hud->UpdMiniTer();  }
void CGui::chkMiniRot(WP wp){		ChkEv(mini_rotated);	}
void CGui::chkMiniTer(WP wp){		ChkEv(mini_terrain);	hud->UpdMiniTer();  }
void CGui::chkMiniBorder(WP wp){		ChkEv(mini_border);		hud->UpdMiniTer();  }

void CGui::chkReverse(WP wp){		ChkEv(gui.trackreverse);	ReadTrkStats();  }

void CGui::chkTimes(WP wp){			ChkEv(show_times);		hud->Show();	}
void CGui::chkOpponents(WP wp){		ChkEv(show_opponents);	hud->Show();	}
void CGui::chkOpponentsSort(WP wp){	ChkEv(opplist_sort);	}

//void CGui::chkRacingLine(WP wp){		ChkEv(racingline);	if (ndLine)  ndLine->setVisible(pSet->racingline);	}
void CGui::chkCamInfo(WP wp){		ChkEv(show_cam);	hud->Show();	}
void CGui::chkCamTilt(WP wp){		ChkEv(cam_tilt);	}

//  other
void CGui::chkFps(WP wp){			ChkEv(show_fps);	}
void CGui::chkWireframe(WP wp){		toggleWireframe();  }

void CGui::chkProfilerTxt(WP wp){	ChkEv(profilerTxt);	}
void CGui::chkBltDebug(WP wp){		ChkEv(bltDebug);	}
void CGui::chkBltProfilerTxt(WP wp){	ChkEv(bltProfilerTxt);	}

void CGui::chkCarDbgBars(WP wp){		ChkEv(car_dbgbars);  hud->Show();  }
void CGui::chkCarDbgTxt(WP wp){		ChkEv(car_dbgtxt);   hud->Show();  }
void CGui::chkCarDbgSurf(WP wp){		ChkEv(car_dbgsurf);  hud->Show();  }
void CGui::chkCarTireVis(WP wp){		ChkEv(car_tirevis);  hud->Destroy();  hud->Create();  }

void CGui::chkGraphs(WP wp){			ChkEv(show_graphs);
	for (int i=0; i < app->graphs.size(); ++i)
		app->graphs[i]->SetVisible(pSet->show_graphs);
}
void CGui::comboGraphs(CMB)
{
	if (valGraphsType)	valGraphsType->setCaption(toStr(val));
	if (bGI /*&& pSet->graphs_type != v*/)
	{	pSet->graphs_type = (eGraphType)val;  app->DestroyGraphs();  app->CreateGraphs();  }
}

void CGui::slDbgTxtClr(SL)
{
	int v = val +slHalf;  if (bGI)  pSet->car_dbgtxtclr = v;
	if (valDbgTxtClr)	valDbgTxtClr->setCaption(toStr(v));
}
void CGui::slDbgTxtCnt(SL)
{
	int v = val*8 +slHalf;  if (bGI)  pSet->car_dbgtxtcnt = v;
	if (valDbgTxtCnt)	valDbgTxtCnt->setCaption(toStr(v));
}

//  Startup
void CGui::chkStartInMain(WP wp)	{	ChkEv(startInMain);    }
void CGui::chkAutoStart(WP wp){		ChkEv(autostart);	}
void CGui::chkEscQuits(WP wp){		ChkEv(escquit);		}
void CGui::chkOgreDialog(WP wp){		ChkEv(ogre_dialog);	}

void CGui::chkBltLines(WP wp){		ChkEv(bltLines);	}
void CGui::chkLoadPics(WP wp){		ChkEv(loadingbackground);	}
void CGui::chkMultiThread(WP wp){	pSet->multi_thr = pSet->multi_thr ? 0 : 1;  if (wp) {
	ButtonPtr chk = wp->castType<MyGUI::Button>();  chk->setStateSelected(pSet->multi_thr > 0);  }	}


//  [Video]  . . . . . . . . . . . . . . . . . . . .    ---- ------ ----    . . . . . . . . . . . . . . . . . . . .

void CGui::chkVidEffects(WP wp)
{
	ChkEv(all_effects);  app->recreateCompositor();  //refreshCompositor();
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
void CGui::slHDRParam1(SL)
{
	Real v = val;  if (bGI)  pSet->hdrParam1 = v;
	if (valHDRParam1)	valHDRParam1->setCaption(fToStr(v,2,4));
}
void CGui::slHDRParam2(SL)
{
	Real v = val;  if (bGI)  pSet->hdrParam2 = v;
	if (valHDRParam2)	valHDRParam2->setCaption(fToStr(v,2,4));
}
void CGui::slHDRParam3(SL)
{
	Real v = val;  if (bGI)  pSet->hdrParam3 = v;
	if (valHDRParam3)	valHDRParam3->setCaption(fToStr(v,2,4));
}
void CGui::slHDRAdaptationScale(SL)
{
	Real v = val;  if (bGI)  pSet->hdrAdaptationScale = v;
	if (valHDRAdaptationScale)	valHDRAdaptationScale->setCaption(fToStr(v,2,4));
}
void CGui::slHDRBloomInt(SL)
{
	Real v = val;  if (bGI)  pSet->hdrbloomint = v;
	if (valHDRBloomInt)  valHDRBloomInt->setCaption(fToStr(v,2,4));
}
void CGui::slHDRBloomOrig(SL)
{
	Real v = val;  if (bGI)  pSet->hdrbloomorig = v;
	if (valHDRBloomOrig)  valHDRBloomOrig->setCaption(fToStr(v,2,4));
}
void CGui::slHDRVignettingRadius(SL)
{
	Real v = 10 * val;  if (bGI)  pSet->vignettingRadius = v;
	if (valHDRVignettingRadius)  valHDRVignettingRadius->setCaption(fToStr(v,2,4));
}
void CGui::slHDRVignettingDarkness(SL)
{
	Real v = val;  if (bGI)  pSet->vignettingDarkness = v;
	if (valHDRVignettingDarkness)  valHDRVignettingDarkness->setCaption(fToStr(v,2,4));
}

void CGui::chkVidBlur(WP wp)
{		
	ChkEv(motionblur);
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
void CGui::slBloomInt(SL)
{
	Real v = val;  if (bGI)  pSet->bloomintensity = v;
	if (valBloomInt)  valBloomInt->setCaption(fToStr(v,2,4));
	if (bGI)  app->refreshCompositor();
}
void CGui::slBloomOrig(SL)
{
	Real v = val;  if (bGI)  pSet->bloomorig = v;
	if (valBloomOrig)  valBloomOrig->setCaption(fToStr(v,2,4));
	if (bGI)  app->refreshCompositor();
}
void CGui::slBlurIntens(SL)
{
	Real v = val;  if (bGI)  pSet->motionblurintensity = v;
	if (valBlurIntens)  valBlurIntens->setCaption(fToStr(v,2,4));
}
void CGui::slDepthOfFieldFocus(SL)
{
	Real v = 2000.f * powf(val, 2.f);  if (bGI)  pSet->depthOfFieldFocus = v;
	if (valDepthOfFieldFocus)  valDepthOfFieldFocus->setCaption(fToStr(v,0,4));
}
void CGui::slDepthOfFieldFar(SL)
{
	Real v = 2000.f * powf(val, 2.f);  if (bGI)  pSet->depthOfFieldFar = v;
	if (valDepthOfFieldFar)  valDepthOfFieldFar->setCaption(fToStr(v,0,4));
}
