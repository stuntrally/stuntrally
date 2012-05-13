#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "OgreGame.h"
#include "FollowCamera.h"
#include "SplitScreen.h"
#include "common/Gui_Def.h"
#include "common/RenderConst.h"
#include "common/GraphView.h"
#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"

#include <MyGUI_PointerManager.h>
#include <OIS/OIS.h>
#include "../oisb/OISB.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <OgreRoot.h>
#include <OgreTerrain.h>
#include <OgreMaterialManager.h>
#include <OgreOverlay.h>
#include <OgreRenderWindow.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  Gui Events

//  [Setup]
//    [Car]
void App::chkAbs(WP wp){		ChkEv(abs);		if (pGame)  pGame->ProcessNewSettings();	}
void App::chkTcs(WP wp){		ChkEv(tcs);		if (pGame)  pGame->ProcessNewSettings();	}

void App::chkGear(WP wp){		ChkEv(autoshift);	if (pGame)  pGame->ProcessNewSettings();	}
void App::chkRear(WP wp){		ChkEv(autorear);	if (pGame)  pGame->ProcessNewSettings();	}
void App::chkRearInv(WP wp){	ChkEv(rear_inv);	if (pGame)  pGame->ProcessNewSettings();	}
//    [Game]
void App::chkVegetCollis(WP wp){	ChkEv(gui.collis_veget);	}
void App::chkCarCollis(WP wp){		ChkEv(gui.collis_cars);		}
void App::chkRoadWCollis(WP wp){	ChkEv(gui.collis_roadw);	}

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
	size_t i = carList->findItemIndexWith(pSet->gui.car[iCurCar]);
	if (i != ITEM_NONE)
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

void App::imgBtnCarClr(WP img)
{
	pSet->gui.car_hue[iCurCar] = s2r(img->getUserString("h"));
	pSet->gui.car_sat[iCurCar] = s2r(img->getUserString("s"));
	pSet->gui.car_val[iCurCar] = s2r(img->getUserString("v"));
	UpdCarClrSld();
}
void App::btnCarClrRandom(WP)
{
	pSet->gui.car_hue[iCurCar] = Math::UnitRandom();
	pSet->gui.car_sat[iCurCar] = Math::UnitRandom();
	pSet->gui.car_val[iCurCar] = Math::UnitRandom();
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
void App::slGraphsType(SL)
{
	int v = val * graph_types +slHalf;	if (valGraphsType)	valGraphsType->setCaption(toStr(v));
	if (bGI /*&& pSet->graphs_type != v*/)
	{	pSet->graphs_type = v;  DestroyGraphs();  CreateGraphs();  }
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


//  [Game] 	. . . . . . . . . . . . . . . . . . . .    --- lists ----    . . . . . . . . . . . . . . . . . . . .

//  car
void App::listCarChng(List* li, size_t pos)
{
	size_t i = li->getIndexSelected();  if (i==ITEM_NONE)  return;
	const UString& sl = li->getItemNameAt(i);	sListCar = sl;

	if (imgCar)  imgCar->setImageTexture(sListCar+".jpg");
	if (mClient) mClient->updatePlayerInfo(pSet->nickname, sListCar);
}
void App::btnChgCar(WP)
{
	if (valCar){  valCar->setCaption(TR("#{Car}: ") + sListCar);	pSet->gui.car[iCurCar] = sListCar;  }
}

//  track
void App::changeTrack()
{
	pSet->gui.track = sListTrack;
	pSet->gui.track_user = bListTrackU;
	if (valTrk[0])  valTrk[0]->setCaption(TR("#{Track}: ") + sListTrack);

	if (mMasterClient) {
		uploadGameInfo();
		updateGameInfoGUI();
	}
}

//  new game
void App::btnNewGame(WP)
{
	if (mWndGame->getVisible() && mWndTabsGame->getIndexSelected() < 5  || mClient)
		pSet->gui.champ_num = -1;  /// champ, back to single race
	
	NewGame();  isFocGui = false;  // off gui
	if (mWndOpts)  mWndOpts->setVisible(isFocGui);
	if (mWndRpl)  mWndRpl->setVisible(false);//
	if (bnQuit)  bnQuit->setVisible(isFocGui);
	
	updMouse();
	
	mToolTip->setVisible(false);
}
void App::btnNewGameStart(WP wp)
{
	changeTrack();
	btnNewGame(wp);
}


//  [View]  . . . . . . . . . . . . . . . . . . . .    ---- checks ----    . . . . . . . . . . . . . . . . . . . .

void App::chkParticles(WP wp)
{		
	ChkEv(particles);
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
	//? if ((*it)->eType != CarModel::CT_GHOST)
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

void App::chkArrow(WP wp){			ChkEv(check_arrow); if (arrowRotNode) arrowRotNode->setVisible(pSet->check_arrow);  }
void App::chkMinimap(WP wp){		ChkEv(trackmap);
	for (int c=0; c < 4; ++c)
		if (ndMap[c])  ndMap[c]->setVisible(pSet->trackmap);
}
void App::chkMiniZoom(WP wp){		ChkEv(mini_zoomed);		}
void App::chkMiniRot(WP wp){		ChkEv(mini_rotated);	}
void App::chkMiniTer(WP wp){		ChkEv(mini_terrain);	UpdMiniTer();  }

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

void App::chkGraphs(WP wp){			ChkEv(show_graphs);
	for (int i=0; i < graphs.size(); ++i)
		graphs[i]->SetVisible(pSet->show_graphs);
}

//  Startup
void App::chkMouseCapture(WP wp){	ChkEv(x11_capture_mouse);	}
void App::chkOgreDialog(WP wp){		ChkEv(ogre_dialog);	}
void App::chkAutoStart(WP wp){		ChkEv(autostart);	}
void App::chkEscQuits(WP wp){		ChkEv(escquit);		}
void App::chkBltLines(WP wp){		ChkEv(bltLines);	}
void App::chkLoadPics(WP wp){		ChkEv(loadingbackground);	}
void App::chkMultiThread(WP wp){	pSet->multi_thr = pSet->multi_thr ? 0 : 1;  if (wp) {
	ButtonPtr chk = wp->castType<MyGUI::Button>();  chk->setStateSelected(pSet->multi_thr > 0);  }	}


//  [Video]  . . . . . . . . . . . . . . . . . . . .    ---- ------ ----    . . . . . . . . . . . . . . . . . . . .

void App::chkVidEffects(WP wp)
{
	ChkEv(all_effects);  recreateCompositor();  //refreshCompositor();
}
void App::chkVidBloom(WP wp)
{		
	ChkEv(bloom);  refreshCompositor();
}
void App::chkVidHDR(WP wp)
{			
	ChkEv(hdr);  refreshCompositor();
}
void App::chkVidBlur(WP wp)
{		
	ChkEv(motionblur);  refreshCompositor();
}
void App::chkVidSSAA(WP wp)
{
	ChkEv(ssaa);  refreshCompositor();
}
void App::chkVidSSAO(WP wp)
{		
	ChkEv(ssao);  refreshCompositor();
}
void App::chkVidSoftParticles(WP wp)
{		
	ChkEv(softparticles);  refreshCompositor();
}
void App::chkVidDepthOfField(WP wp)
{		
	ChkEv(dof);  refreshCompositor();
}
void App::chkVidGodRays(WP wp)
{		
	ChkEv(godrays);  refreshCompositor();
}
void App::chkVidFilmGrain(WP wp)
{		
	ChkEv(filmgrain);  refreshCompositor();
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
	// if (bGI)  refreshCompositor();   // intensity is set every frame in UpdateHUD
}
void App::slDepthOfFieldFocus(SL)
{
	Real v = 2000.f * powf(val, 2.f);  if (bGI)  pSet->depthOfFieldFocus = v;
	if (valDepthOfFieldFocus)	valDepthOfFieldFocus->setCaption(fToStr(v,0,4));
	// if (bGI)  refreshCompositor();   // intensity is set every frame in UpdateHUD
}
void App::slDepthOfFieldFar(SL)
{
	Real v = 2000.f * powf(val, 2.f);  if (bGI)  pSet->depthOfFieldFar = v;
	if (valDepthOfFieldFar)		valDepthOfFieldFar->setCaption(fToStr(v,0,4));
	// if (bGI)  refreshCompositor();   // intensity is set every frame in UpdateHUD
}


//  close netw end
void App::btnNetEndClose(WP)
{
	mWndNetEnd->setVisible(false);
	isFocGui = true;  // show back gui
	toggleGui(false);
}


//  Menu
//---------------------------------------------------------------------

void App::toggleGui(bool toggle)
{
	if (toggle)
		isFocGui = !isFocGui;

	bool notMain = isFocGui && !pSet->isMain;
	if (mWndMain)	mWndMain->setVisible(isFocGui && pSet->isMain);
	if (mWndReplays) mWndReplays->setVisible(notMain && pSet->inMenu == WND_Replays);
	if (mWndHelp)	mWndHelp->setVisible(notMain && pSet->inMenu == WND_Help);
	if (mWndOpts)	mWndOpts->setVisible(notMain && pSet->inMenu == WND_Options);
	
	//  fill Readme editbox from file
	static bool first = true;
	if (mWndHelp && mWndHelp->getVisible() && first)
	{
		first = false;
		EditBox* edit = mGUI->findWidget<EditBox>("Readme");
		if (edit)
		{	std::string path = PATHMANAGER::GetDataPath()+"/../Readme.txt";
			std::ifstream fi(path.c_str());

			static char buf[2*4096];
			fi.read(buf,sizeof(buf));
			String text = buf;

			text = StringUtil::replaceAll(text, "#", "##");
			edit->setCaption(UString(text));
			edit->setVScrollPosition(0);
	}	}

	///  update track tab, for champs wnd
	bool game = pSet->inMenu == WND_Game, champ = pSet->inMenu == WND_Champ, gc = game || champ;
	if (mWndGame)
	{	mWndGame->setVisible(notMain  && gc);
		if (mWndGame->getVisible())
			mWndGame->setCaption(champ ? TR("#{Championship}") : TR("#{SingleRace}"));
	}
	if (notMain && gc)  // show hide champs,stages
	{
		size_t id = mWndTabsGame->getIndexSelected();
		mWndTabsGame->setButtonWidthAt(1,champ ? 1 :-1);  if (id == 1 && champ)  mWndTabsGame->setIndexSelected(5);
		mWndTabsGame->setButtonWidthAt(4,champ ? 1 :-1);  if (id == 4 && champ)  mWndTabsGame->setIndexSelected(5);
		mWndTabsGame->setButtonWidthAt(5,champ ?-1 : 1);  if (id == 5 && !champ)  mWndTabsGame->setIndexSelected(1);
		mWndTabsGame->setButtonWidthAt(6,champ ?-1 : 1);  if (id == 6 && !champ)  mWndTabsGame->setIndexSelected(1);
		mWndTabsGame->setButtonWidthAt(7,champ ?-1 : 1);  if (id == 7 && !champ)  mWndTabsGame->setIndexSelected(1);
	}

	if (bnQuit)  bnQuit->setVisible(isFocGui);
	updMouse();
	if (!isFocGui)  mToolTip->setVisible(false);

	for (int i=0; i < WND_ALL; ++i)
		mWndMainPanels[i]->setVisible(pSet->inMenu == i);
}

void App::MainMenuBtn(MyGUI::WidgetPtr wp)
{
	for (int i=0; i < WND_ALL; ++i)
		if (wp == mWndMainBtns[i])
		{
			pSet->isMain = false;
			pSet->inMenu = i;
			toggleGui(false);
			return;
		}
}

void App::MenuTabChg(MyGUI::TabPtr tab, size_t id)
{
	if (id != 0)  return;
	tab->setIndexSelected(1);  // dont switch to 0
	pSet->isMain = true;
	toggleGui(false);  // back to main
}

void App::GuiShortcut(WND_Types wnd, int tab, int subtab)
{
	isFocGui = true;
	pSet->isMain = false;  pSet->inMenu = wnd;
	
	MyGUI::TabPtr mWndTabs = 0;
	std::vector<MyGUI::TabControl*>* subt = 0;
	
	switch (wnd)
	{	case WND_Champ:
		case WND_Game:		mWndTabs = mWndTabsGame;  subt = &vSubTabsGame;  break;
		case WND_Replays:	mWndTabs = mWndTabsRpl;  break;
		case WND_Help:		mWndTabs = mWndTabsHelp;  break;
		case WND_Options:	mWndTabs = mWndTabsOpts;  subt = &vSubTabsOpts;  break;
	}
	toggleGui(false);


	size_t t = mWndTabs->getIndexSelected();
	mWndTabs->setIndexSelected(tab);

	if (!subt)  return;
	MyGUI::TabControl* tc = (*subt)[t];  if (!tc)  return;
	int  cnt = tc->getItemCount();

	if (t == tab && subtab == -1)  // cycle subpages if same tab
	{	if (shift)
			tc->setIndexSelected( (tc->getIndexSelected()-1+cnt) % cnt );
		else
			tc->setIndexSelected( (tc->getIndexSelected()+1) % cnt );
	}else
	if (subtab > -1)
		tc->setIndexSelected( std::min(cnt-1, subtab) );
}
