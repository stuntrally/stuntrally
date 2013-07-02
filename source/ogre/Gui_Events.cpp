#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/settings.h"
#include "../vdrift/game.h"
#include "OgreGame.h"
#include "SplitScreen.h"
#include "common/Gui_Def.h"
#include "common/Slider.h"
#include "common/MultiList2.h"
using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  Gui Events

//    [Car]
void App::chkAbs(WP wp){		ChkEv(abs[iTireSet]);	if (pGame)  pGame->ProcessNewSettings();	}
void App::chkTcs(WP wp){		ChkEv(tcs[iTireSet]);	if (pGame)  pGame->ProcessNewSettings();	}

void App::tabTireSet(MyGUI::TabPtr wp, size_t id)
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
void App::slSSSEffect(SL)
{
	Real v = 1.f * val;  if (bGI)  pSet->sss_effect[iTireSet] = v;
	if (valSSSEffect){	valSSSEffect->setCaption(fToStr(v,2,4));  }
}
void App::slSSSVelFactor(SL)
{
	Real v = 2.f * val;  if (bGI)  pSet->sss_velfactor[iTireSet] = v;
	if (valSSSVelFactor){	valSSSVelFactor->setCaption(fToStr(v,2,4));  }
}
void App::slSteerRangeSurf(SL)
{
	Real v = val +0.3f;  if (bGI)  pSet->steer_range[iTireSet] = v;
	if (valSteerRangeSurf){		valSteerRangeSurf->setCaption(fToStr(v,2,4));  }
}
void App::slSteerRangeSim(SL)
{
	Real v = val +0.3f;  if (bGI)  pSet->steer_range[iTireSet] = v;
	if (valSteerRangeSim){		valSteerRangeSim->setCaption(fToStr(v,2,4));  }
}

void App::chkGear(WP wp){		ChkEv(autoshift);	if (pGame)  pGame->ProcessNewSettings();	}
void App::chkRear(WP wp){		ChkEv(autorear);	if (pGame)  pGame->ProcessNewSettings();	}
void App::chkRearInv(WP wp){	ChkEv(rear_inv);	if (pGame)  pGame->ProcessNewSettings();	}


//    [Game]
void App::chkVegetCollis(WP wp){	ChkEv(gui.collis_veget);	}
void App::chkCarCollis(WP wp){		ChkEv(gui.collis_cars);		}
void App::chkRoadWCollis(WP wp){	ChkEv(gui.collis_roadw);	}
void App::chkDynObjects(WP wp){		ChkEv(gui.dyn_objects);		}

//  boost, flip
void App::comboBoost(CMB)
{
	pSet->gui.boost_type = val;  ShowHUD();
}
void App::comboFlip(CMB)
{
	pSet->gui.flip_type = val;
}
	
void App::btnNumPlayers(WP wp)
{
	if      (wp->getName() == "btnPlayers1")  pSet->gui.local_players = 1;
	else if (wp->getName() == "btnPlayers2")  pSet->gui.local_players = 2;
	else if (wp->getName() == "btnPlayers3")  pSet->gui.local_players = 3;
	else if (wp->getName() == "btnPlayers4")  pSet->gui.local_players = 4;
	if (valLocPlayers)  valLocPlayers->setCaption(toStr(pSet->gui.local_players));
}
void App::chkSplitVert(WP wp)
{
	ChkEv(split_vertically); 
}

void App::slNumLaps(SL)
{
	int v = 20.f * val + 1 +slHalf;  if (bGI)  pSet->gui.num_laps = v;
	if (valNumLaps){  valNumLaps->setCaption(toStr(v));  }
}

void App::tabPlayer(TabPtr wp, size_t id)
{
	iCurCar = id;
	//  update gui for this car (color h,s,v, name, img)
	string c = pSet->gui.car[iCurCar];
	for (size_t i=0; i < carList->getItemCount(); ++i)
	if (carList->getItemNameAt(i).substr(7) == c)
	{	carList->setIndexSelected(i);
		listCarChng(carList, i);
	}
	UpdCarClrSld(false);  // no car color change
}

//  car color
void App::slCarClrH(SL)
{
	Real v = val;  if (bGI)  pSet->gui.car_hue[iCurCar] = v;
	if (valCarClrH){	valCarClrH->setCaption(fToStr(v,2,4));  }
	if (iCurCar < carModels.size() && bUpdCarClr && bGI)
		carModels[iCurCar]->ChangeClr(iCurCar);
}
void App::slCarClrS(SL)
{
	Real v = val;  if (bGI)  pSet->gui.car_sat[iCurCar] = v;
	if (valCarClrS){	valCarClrS->setCaption(fToStr(v,2,4));  }
	if (iCurCar < carModels.size() && bUpdCarClr && bGI)
		carModels[iCurCar]->ChangeClr(iCurCar);
}
void App::slCarClrV(SL)
{
	Real v = val;  if (bGI)  pSet->gui.car_val[iCurCar] = v;
	if (valCarClrV){	valCarClrV->setCaption(fToStr(v,2,4));  }
	if (iCurCar < carModels.size() && bUpdCarClr && bGI)
		carModels[iCurCar]->ChangeClr(iCurCar);
}
void App::slCarClrGloss(SL)
{
	Real v = powf(val, 1.6f);  if (bGI)  pSet->gui.car_gloss[iCurCar] = v;
	if (valCarClrGloss){	valCarClrGloss->setCaption(fToStr(v,2,4));  }
	if (iCurCar < carModels.size() && bUpdCarClr && bGI)
		carModels[iCurCar]->ChangeClr(iCurCar);
}
void App::slCarClrRefl(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->gui.car_refl[iCurCar] = v;
	if (valCarClrRefl){		valCarClrRefl->setCaption(fToStr(v,2,4));  }
	if (iCurCar < carModels.size() && bUpdCarClr && bGI)
		carModels[iCurCar]->ChangeClr(iCurCar);
}

void App::imgBtnCarClr(WP img)
{
	pSet->gui.car_hue[iCurCar] = s2r(img->getUserString("h"));
	pSet->gui.car_sat[iCurCar] = s2r(img->getUserString("s"));
	pSet->gui.car_val[iCurCar] = s2r(img->getUserString("v"));
	pSet->gui.car_gloss[iCurCar]= s2r(img->getUserString("g"));
	pSet->gui.car_refl[iCurCar] = s2r(img->getUserString("r"));
	UpdCarClrSld();
}
void App::btnCarClrRandom(WP)
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
void App::slParticles(SL)
{
	Real v = 4.f * powf(val, 2.f);  if (bGI)  pSet->particles_len = v;
	if (valParticles){	valParticles->setCaption(fToStr(v,2,4));  }
}
void App::slTrails(SL)
{
	Real v = 4.f * powf(val, 2.f);  if (bGI)  pSet->trails_len = v;
	if (valTrails){		valTrails->setCaption(fToStr(v,2,4));  }
}

//  reflect
void App::slReflSkip(SL)
{
	int v = 1000.f * powf(val, 2.f) +slHalf;	if (bGI)  pSet->refl_skip = v;
	if (valReflSkip)  valReflSkip->setCaption(toStr(v));
}
void App::slReflSize(SL)
{
	int v = std::max( 0.0f, std::min((float) ciShadowNumSizes-1, ciShadowNumSizes * val)) +slHalf;
	if (bGI)  pSet->refl_size = v;
	if (valReflSize)  valReflSize->setCaption(toStr(ciShadowSizesA[v]));
}
void App::slReflFaces(SL)
{
	int v = val * 6.f +slHalf;
	if (bGI)  pSet->refl_faces = v;
	if (valReflFaces)  valReflFaces->setCaption(toStr(v));
}
void App::slReflDist(SL)
{
	float v = 20.f + 1480.f * powf(val, 2.f);	if (bGI)  pSet->refl_dist = v;
	if (valReflDist){	valReflDist->setCaption(fToStr(v,0,4)+" m");  }
	
	recreateReflections();
}
void App::slReflMode(SL)
{
	int old = pSet->refl_mode;
	pSet->refl_mode = val * 2.f +slHalf;
	
	if (pSet->refl_mode != old)
		recreateReflections();
		
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
void App::slSizeGaug(SL)
{
	float v = 0.1f + 0.15f * val;	if (bGI)  {  pSet->size_gauges = v;  SizeHUD(true);  }
	if (valSizeGaug)	valSizeGaug->setCaption(fToStr(v,3,4));
}
void App::slTypeGaug(SL)
{
	int v = val * 5.f +slHalf;		if (bGI)  {  pSet->gauges_type = v;  CreateHUD(true);  }
	if (valTypeGaug)	valTypeGaug->setCaption(toStr(v));
}
void App::slSizeArrow(SL)
{
	float v = val;	if (bGI)  {  pSet->size_arrow = v;  }
	if (valSizeArrow)	valSizeArrow->setCaption(fToStr(v,3,4));
	if (arrowNode) arrowRotNode->setScale(v/2.f, v/2.f, v/2.f);
}
void App::slCountdownTime(SL)
{
	float v = (int)(val * 6.f +slHalf) * 0.5f;	if (bGI)  {  pSet->gui.pre_time = v;  }
	if (valCountdownTime){	valCountdownTime->setCaption(fToStr(v,1,4));  }
}

//  minimap
void App::slSizeMinimap(SL)
{
	float v = 0.05f + 0.25f * val;	if (bGI)  {  pSet->size_minimap = v;  SizeHUD(true);  }
	if (valSizeMinimap)  valSizeMinimap->setCaption(fToStr(v,3,4));
}
void App::slZoomMinimap(SL)
{
	float v = 1.f + 9.f * powf(val, 2.f);	if (bGI)  {  pSet->zoom_minimap = v;  SizeHUD(true);  }
	if (valZoomMinimap)  valZoomMinimap->setCaption(fToStr(v,3,4));
}


//  [Sound]
void App::slVolMaster(SL)
{
	Real v = 1.6f * val;	if (bGI)  {  pSet->vol_master = v;  pGame->ProcessNewSettings();  }
	if (valVolMaster)  valVolMaster->setCaption(fToStr(v,2,4));
}
void App::slVolEngine(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_engine = v;	if (valVolEngine)  valVolEngine->setCaption(fToStr(v,2,4));
}
void App::slVolTires(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_tires = v;	if (valVolTires)  valVolTires->setCaption(fToStr(v,2,4));
}
void App::slVolSusp(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_susp = v;		if (valVolSusp)  valVolSusp->setCaption(fToStr(v,2,4));
}
void App::slVolEnv(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_env = v;		if (valVolEnv)  valVolEnv->setCaption(fToStr(v,2,4));
}
void App::slVolFlSplash(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_fl_splash = v;	if (valVolFlSplash)  valVolFlSplash->setCaption(fToStr(v,2,4));
}
void App::slVolFlCont(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_fl_cont = v;		if (valVolFlCont)  valVolFlCont->setCaption(fToStr(v,2,4));
}
void App::slVolCarCrash(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_car_crash = v;	if (valVolCarCrash)  valVolCarCrash->setCaption(fToStr(v,2,4));
}
void App::slVolCarScrap(SL)
{
	Real v = 1.4f * val;  if (bGI)  pSet->vol_car_scrap = v;	if (valVolCarScrap)  valVolCarScrap->setCaption(fToStr(v,2,4));
}


//  [View]  . . . . . . . . . . . . . . . . . . . .    ---- checks ----    . . . . . . . . . . . . . . . . . . . .

void App::chkParticles(WP wp)
{		
	ChkEv(particles);
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		(*it)->UpdParsTrails();
}
void App::chkTrails(WP wp)
{			
	ChkEv(trails);		
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		(*it)->UpdParsTrails();
}
void App::toggleWireframe()
{
	mbWireFrame = !mbWireFrame;
	if (chWire)  chWire->setStateSelected(mbWireFrame);
	
	///  Set for all cameras
	PolygonMode mode = mbWireFrame ? PM_WIREFRAME : PM_SOLID;
	
	refreshCompositor(mode == PM_WIREFRAME);  // disable effects
	if (mSplitMgr)
	for (std::list<Camera*>::iterator it=mSplitMgr->mCameras.begin(); it!=mSplitMgr->mCameras.end(); ++it)
		(*it)->setPolygonMode(mode);
	
	if (ndSky)	ndSky->setVisible(!mbWireFrame);  // hide sky
}
//  hud
void App::chkDigits(WP wp){ 		ChkEv(show_digits); ShowHUD();   }
void App::chkGauges(WP wp){			ChkEv(show_gauges);	ShowHUD();	}

void App::radKmh(WP wp){	bRkmh->setStateSelected(true);  bRmph->setStateSelected(false);  pSet->show_mph = false;  ShowHUD();  }
void App::radMph(WP wp){	bRkmh->setStateSelected(false);  bRmph->setStateSelected(true);  pSet->show_mph = true;   ShowHUD();  }

void App::radSimEasy(WP){	bRsimEasy->setStateSelected(true);  bRsimNorm->setStateSelected(false);
	pSet->gui.sim_mode = "easy";	bReloadSim = true;
	tabTireSet(0,iTireSet);  listCarChng(carList,0);
}
void App::radSimNorm(WP){	bRsimEasy->setStateSelected(false);  bRsimNorm->setStateSelected(true);
	pSet->gui.sim_mode = "normal";	bReloadSim = true;
	tabTireSet(0,iTireSet);  listCarChng(carList,0);
}

void App::chkArrow(WP wp){			ChkEv(check_arrow);
	if (arrowRotNode) arrowRotNode->setVisible(pSet->check_arrow);
}
void App::chkBeam(WP wp){			ChkEv(check_beam);
	for (int i=0; i < carModels.size(); ++i)  carModels[i]->ShowNextChk(pSet->check_beam);
}

void App::chkMinimap(WP wp){		ChkEv(trackmap);
	for (int c=0; c < 4; ++c)
		if (ndMap[c])  ndMap[c]->setVisible(pSet->trackmap);
}
void App::chkMiniZoom(WP wp){		ChkEv(mini_zoomed);		UpdMiniTer();  }
void App::chkMiniRot(WP wp){		ChkEv(mini_rotated);	}
void App::chkMiniTer(WP wp){		ChkEv(mini_terrain);	UpdMiniTer();  }
void App::chkMiniBorder(WP wp){		ChkEv(mini_border);		UpdMiniTer();  }

void App::chkReverse(WP wp){		ChkEv(gui.trackreverse);	ReadTrkStats();  }

void App::chkTimes(WP wp){			ChkEv(show_times);	ShowHUD();	}
void App::chkOpponents(WP wp){		ChkEv(show_opponents);	ShowHUD();	}
void App::chkOpponentsSort(WP wp){	ChkEv(opplist_sort);	}

//void App::chkRacingLine(WP wp){		ChkEv(racingline);	if (ndLine)  ndLine->setVisible(pSet->racingline);	}
void App::chkCamInfo(WP wp){		ChkEv(show_cam);	ShowHUD();	}
void App::chkCamTilt(WP wp){		ChkEv(cam_tilt);	}

//  other
void App::chkFps(WP wp){			ChkEv(show_fps);	if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();	}
void App::chkWireframe(WP wp){		toggleWireframe();  }

void App::chkProfilerTxt(WP wp){	ChkEv(profilerTxt);	}
void App::chkBltDebug(WP wp){		ChkEv(bltDebug);	}
void App::chkBltProfilerTxt(WP wp){	ChkEv(bltProfilerTxt);	}

void App::chkCarDbgBars(WP wp){		ChkEv(car_dbgbars);	ShowHUD();	}
void App::chkCarDbgTxt(WP wp){		ChkEv(car_dbgtxt);	ShowHUD();	}
void App::chkCarDbgSurf(WP wp){		ChkEv(car_dbgsurf);	ShowHUD();	}

void App::chkGraphs(WP wp){			ChkEv(show_graphs);
	for (int i=0; i < graphs.size(); ++i)
		graphs[i]->SetVisible(pSet->show_graphs);
}
void App::comboGraphs(CMB)
{
	if (valGraphsType)	valGraphsType->setCaption(toStr(val));
	if (bGI /*&& pSet->graphs_type != v*/)
	{	pSet->graphs_type = (eGraphType)val;  DestroyGraphs();  CreateGraphs();  }
}

void App::slDbgTxtClr(SL)
{
	int v = val +slHalf;  if (bGI)  pSet->car_dbgtxtclr = v;
	if (valDbgTxtClr)	valDbgTxtClr->setCaption(toStr(v));
}
void App::slDbgTxtCnt(SL)
{
	int v = val*8 +slHalf;  if (bGI)  pSet->car_dbgtxtcnt = v;
	if (valDbgTxtCnt)	valDbgTxtCnt->setCaption(toStr(v));
}

//  Startup
void App::chkMouseCapture(WP wp){	ChkEv(capture_mouse);  }
void App::chkStartInMain(WP wp)	{	ChkEv(startInMain);    }

void App::chkAutoStart(WP wp){		ChkEv(autostart);	}
void App::chkEscQuits(WP wp){		ChkEv(escquit);		}
void App::chkOgreDialog(WP wp){		ChkEv(ogre_dialog);	}

void App::chkBltLines(WP wp){		ChkEv(bltLines);	}
void App::chkLoadPics(WP wp){		ChkEv(loadingbackground);	}
void App::chkMultiThread(WP wp){	pSet->multi_thr = pSet->multi_thr ? 0 : 1;  if (wp) {
	ButtonPtr chk = wp->castType<MyGUI::Button>();  chk->setStateSelected(pSet->multi_thr > 0);  }	}


//  [Video]  . . . . . . . . . . . . . . . . . . . .    ---- ------ ----    . . . . . . . . . . . . . . . . . . . .

void App::chkVidEffects(WP wp)
{
	ChkEv(all_effects);  recreateCompositor();  //refreshCompositor();
	changeShadows();
}
void App::chkVidBloom(WP wp)
{		
	ChkEv(bloom);  refreshCompositor();
}

void App::chkVidHDR(WP wp)
{			
	ChkEv(hdr);  refreshCompositor();
}
void App::slHDRParam1(SL)
{
	Real v = val;  if (bGI)  pSet->hdrParam1 = v;
	if (valHDRParam1)	valHDRParam1->setCaption(fToStr(v,2,4));
}
void App::slHDRParam2(SL)
{
	Real v = val;  if (bGI)  pSet->hdrParam2 = v;
	if (valHDRParam2)	valHDRParam2->setCaption(fToStr(v,2,4));
}
void App::slHDRParam3(SL)
{
	Real v = val;  if (bGI)  pSet->hdrParam3 = v;
	if (valHDRParam3)	valHDRParam3->setCaption(fToStr(v,2,4));
}
void App::slHDRAdaptationScale(SL)
{
	Real v = val;  if (bGI)  pSet->hdrAdaptationScale = v;
	if (valHDRAdaptationScale)	valHDRAdaptationScale->setCaption(fToStr(v,2,4));
}
void App::slHDRBloomInt(SL)
{
	Real v = val;  if (bGI)  pSet->hdrbloomint = v;
	if (valHDRBloomInt)  valHDRBloomInt->setCaption(fToStr(v,2,4));
}
void App::slHDRBloomOrig(SL)
{
	Real v = val;  if (bGI)  pSet->hdrbloomorig = v;
	if (valHDRBloomOrig)  valHDRBloomOrig->setCaption(fToStr(v,2,4));
}
void App::slHDRVignettingRadius(SL)
{
	Real v = 10 * val;  if (bGI)  pSet->vignettingRadius = v;
	if (valHDRVignettingRadius)  valHDRVignettingRadius->setCaption(fToStr(v,2,4));
}
void App::slHDRVignettingDarkness(SL)
{
	Real v = val;  if (bGI)  pSet->vignettingDarkness = v;
	if (valHDRVignettingDarkness)  valHDRVignettingDarkness->setCaption(fToStr(v,2,4));
}

void App::chkVidBlur(WP wp)
{		
	ChkEv(motionblur);
	refreshCompositor();  changeShadows();
}
void App::chkVidSSAO(WP wp)
{		
	ChkEv(ssao);
	refreshCompositor();  changeShadows();
}
void App::chkVidSoftParticles(WP wp)
{		
	ChkEv(softparticles);
	refreshCompositor();  changeShadows();
}
void App::chkVidDepthOfField(WP wp)
{		
	ChkEv(dof);
	refreshCompositor();  changeShadows();
}
void App::chkVidGodRays(WP wp)
{		
	ChkEv(godrays);
	refreshCompositor();  changeShadows();
}
void App::chkVidBoostFOV(WP wp)
{		
	ChkEv(boost_fov);
}
void App::slBloomInt(SL)
{
	Real v = val;  if (bGI)  pSet->bloomintensity = v;
	if (valBloomInt){	valBloomInt->setCaption(fToStr(v,2,4));  }
	if (bGI)  refreshCompositor();
}
void App::slBloomOrig(SL)
{
	Real v = val;  if (bGI)  pSet->bloomorig = v;
	if (valBloomOrig){	valBloomOrig->setCaption(fToStr(v,2,4));  }
	if (bGI)  refreshCompositor();
}
void App::slBlurIntens(SL)
{
	Real v = val;  if (bGI)  pSet->motionblurintensity = v;
	if (valBlurIntens){	valBlurIntens->setCaption(fToStr(v,2,4));  }
}
void App::slDepthOfFieldFocus(SL)
{
	Real v = 2000.f * powf(val, 2.f);  if (bGI)  pSet->depthOfFieldFocus = v;
	if (valDepthOfFieldFocus)	valDepthOfFieldFocus->setCaption(fToStr(v,0,4));
}
void App::slDepthOfFieldFar(SL)
{
	Real v = 2000.f * powf(val, 2.f);  if (bGI)  pSet->depthOfFieldFar = v;
	if (valDepthOfFieldFar)		valDepthOfFieldFar->setCaption(fToStr(v,0,4));
}
