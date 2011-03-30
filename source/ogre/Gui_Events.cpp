#include "stdafx.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "OgreGame.h"
using namespace MyGUI;

#define res  1000000.f
#define Fmt  sprintf

///  Gui Events
//-----------------------------------------------------------------------------------------------------------

//  [Graphics]

//  textures
void App::comboTexFilter(SL)
{
	TextureFilterOptions tfo;							
	switch (val)  {
		case 0:	 tfo = TFO_BILINEAR;	break;
		case 1:	 tfo = TFO_TRILINEAR;	break;
		case 2:	 tfo = TFO_ANISOTROPIC;	break;	}
	MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
}

void App::slAnisotropy(SL)
{
	MaterialManager::getSingleton().setDefaultAnisotropy(val);	pSet->anisotropy = val;
	if (valAnisotropy)	valAnisotropy->setCaption(toStr(val));
}

//  view dist
void App::slViewDist(SL)
{
	Real v = 50.f + 6950.f * powf(val/res, 2.f);
	mCamera->setFarClipDistance(v*1.1f);
	Vector3 sc = v*Vector3::UNIT_SCALE;

	SceneNode* nskb = mSceneMgr->getSkyBoxNode();
	if (nskb)  nskb->setScale(sc*0.58);
	else  if (ndSky)  ndSky->setScale(sc);

	pSet->view_distance = v;
	if (valViewDist){	Fmt(s, "%4.1f km", v*0.001f);	valViewDist->setCaption(s);  }
}

//  ter detail
void App::slTerDetail(SL)
{
	Real v = 20.f * powf(val/res, 2.f);  pSet->terdetail = v;
	if (mTerrainGlobals)
		mTerrainGlobals->setMaxPixelError(v);
	if (valTerDetail){	Fmt(s, "%4.1f %%", v);	valTerDetail->setCaption(s);  }
}

//  ter dist
void App::slTerDist(SL)
{
	Real v = 1000.f * powf(val/res, 2.f);  pSet->terdist = v;
	if (mTerrainGlobals)
		mTerrainGlobals->setCompositeMapDistance(v);
	if (valTerDist){	Fmt(s, "%4.0f m", v);	valTerDist->setCaption(s);  }
}

//  road dist
void App::slRoadDist(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  pSet->road_dist = v;
	if (valRoadDist){	Fmt(s, "%5.2f", v);	valRoadDist->setCaption(s);  }
}


//  trees/grass
void App::slTrees(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  pSet->trees = v;
	if (valTrees){	Fmt(s, "%4.2f", v);	valTrees->setCaption(s);  }
}
void App::slGrass(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  pSet->grass = v;
	if (valGrass){	Fmt(s, "%4.2f", v);	valGrass->setCaption(s);  }
}

void App::slTreesDist(SL)
{
	Real v = 0.5f + 6.5f * powf(val/res, 2.f);  pSet->trees_dist = v;
	if (valTreesDist){	Fmt(s, "%4.2f", v);	valTreesDist->setCaption(s);  }
}
void App::slGrassDist(SL)
{
	Real v = 0.5f + 6.5f * powf(val/res, 2.f);  pSet->grass_dist = v;
	if (valGrassDist){	Fmt(s, "%4.2f", v);	valGrassDist->setCaption(s);  }
}

void App::btnTrGrReset(WP wp)
{
	HScrollPtr sl;  size_t v;
	#define setSld(name)  sl##name(0,v);  \
		sl = (HScrollPtr)mLayout->findWidget(#name);  if (sl)  sl->setScrollPosition(v);
	v = res*powf(1.f /4.f, 0.5f);
	setSld(Trees);
	setSld(Grass);
	v = res*powf((1.f-0.5f) /6.5f, 0.5f);
	setSld(TreesDist);
	setSld(GrassDist);
}


//  particles/trails
void App::slParticles(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  pSet->particles_len = v;
	if (valParticles){	Fmt(s, "%4.2f", v);	valParticles->setCaption(s);  }
}
void App::slTrails(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  pSet->trails_len = v;
	if (valTrails){	Fmt(s, "%4.2f", v);	valTrails->setCaption(s);  }
}


//  view size
void App::slSizeGaug(SL)
{
	float v = 0.1f + 0.15f * val/res;	pSet->size_gauges = v;  SizeHUD(true);
	if (valSizeGaug){	Fmt(s, "%4.3f", v);	valSizeGaug->setCaption(s);  }
}
//  size minimap
void App::slSizeMinmap(SL)
{
	float v = 0.05f + 0.25f * val/res;	pSet->size_minimap = v;  SizeHUD(true);
	if (valSizeMinmap){	Fmt(s, "%4.3f", v);	valSizeMinmap->setCaption(s);  }
}


//  reflect
void App::slReflSkip(SL)
{
	int v = 1000.f * powf(val/res, 2.f);	pSet->refl_skip = v;
	if (valReflSkip)  valReflSkip->setCaption(toStr(v));
}
void App::slReflSize(SL)
{
	int v = max( 0.0f, min((float) ciShadowNumSizes-1, ciShadowNumSizes * val/res));	pSet->refl_size = v;
	if (valReflSize)  valReflSize->setCaption(toStr(ciShadowSizesA[v]));
}
void App::slReflFaces(SL)
{
	pSet->refl_faces = val;
	if (valReflFaces)  valReflFaces->setCaption(toStr(val));
}
void App::slReflDist(SL)
{
	float v = 20.f + 1480.f * powf(val/res, 2.f);	pSet->refl_dist = v;
	if (valReflDist){	Fmt(s, "%4.0f m", v);	valReflDist->setCaption(s);  }
}


void App::slShaders(SL)
{
	int v = val;  pSet->shaders = v;
	if (valShaders)
	{	if (v == 0)  valShaders->setCaption("Vertex");  else
		if (v == 1)  valShaders->setCaption("Pixel");  else
		if (v == 2)  valShaders->setCaption("Metal");  }
}

//  shadows
void App::btnShadows(WP){	changeShadows();	}

void App::slShadowType(SL)
{
	int v = val;	pSet->shadow_type = v;
	if (valShadowType)
	{	if (v == 0)  valShadowType->setCaption("None");  else
		if (v == 1)  valShadowType->setCaption("Old");  else
		if (v == 2)  valShadowType->setCaption("Normal");  else
		if (v == 3)  valShadowType->setCaption("Depth");  }
}

void App::slShadowCount(SL)
{
	int v = 2 + 2.f * val/res;	pSet->shadow_count = v;
	if (valShadowCount)  valShadowCount->setCaption(toStr(v));
}

void App::slShadowSize(SL)
{
	int v = max( 0.0f, min((float) ciShadowNumSizes-1, ciShadowNumSizes * val/res));	pSet->shadow_size = v;
	if (valShadowSize)  valShadowSize->setCaption(toStr(ciShadowSizesA[v]));
}

void App::slShadowDist(SL)
{
	Real v = 50.f + 4750.f * powf(val/res, 2.f);	pSet->shadow_dist = v;
	if (valShadowDist){  Fmt(s, "%4.1f km", v*0.001f);	valShadowDist->setCaption(s);  }
}


//  sound
void App::slVolMaster(SL)
{
	Real v = 1.6f * val/res;	pSet->vol_master = v;	pGame->ProcessNewSettings();
	if (valVolMaster){  Fmt(s, "%4.2f", v);	valVolMaster->setCaption(s);  }
}
void App::slVolEngine(SL)
{
	Real v = 1.4f * val/res;	pSet->vol_engine = v;
	if (valVolEngine){  Fmt(s, "%4.2f", v);	valVolEngine->setCaption(s);  }
}
void App::slVolTires(SL)
{
	Real v = 1.4f * val/res;	pSet->vol_tires = v;
	if (valVolTires){  Fmt(s, "%4.2f", v);	valVolTires->setCaption(s);  }
}
void App::slVolEnv(SL)
{
	Real v = 1.4f * val/res;	pSet->vol_env = v;
	if (valVolEnv){  Fmt(s, "%4.2f", v);		valVolEnv->setCaption(s);  }
}


//  car color
void App::slCarClrH(SL)
{
	Real v = val/res;  pSet->car_hue = v;
	if (valCarClrH){	Fmt(s, "%4.2f", v);	valCarClrH->setCaption(s);  }
	CarChangeClr();
}
void App::slCarClrS(SL)
{
	Real v = -1.f + 2.f * val/res;  pSet->car_sat = v;
	if (valCarClrS){	Fmt(s, "%4.2f", v);	valCarClrS->setCaption(s);  }
	CarChangeClr();
}
void App::slCarClrV(SL)
{
	Real v = -1.f + 2.f * val/res;  pSet->car_val = v;
	if (valCarClrV){	Fmt(s, "%4.2f", v);	valCarClrV->setCaption(s);  }
	CarChangeClr();
}


//  [Game] 	. . . . . . . . . . . . . . . . . . . .    --- lists ----    . . . . . . . . . . . . . . . . . . . .

//  car
void App::listCarChng(List* li, size_t pos)
{
	size_t i = li->getIndexSelected();  if (i==ITEM_NONE)  return;
	const UString& sl = li->getItemNameAt(i);	sListCar = sl;
	
	if (imgCar)  imgCar->setImageTexture(sListCar+".jpg");
}
void App::btnChgCar(WP)
{
	if (valCar){  valCar->setCaption("Car: " + sListCar);	pSet->car = sListCar;  }
}

//  track
void App::listTrackChng(List* li, size_t pos)
{
	size_t i = li->getIndexSelected();  if (i==ITEM_NONE)  return;
	const UString& sl = li->getItemNameAt(i);	sListTrack = sl;
	
	if (imgPrv)  imgPrv->setImageTexture(sListTrack+".jpg");
	if (imgTer)  imgTer->setImageTexture(sListTrack+"_ter.jpg");
	if (imgMini)  imgMini->setImageTexture(sListTrack+"_mini.png");
	ReadTrkStats();
}
void App::btnChgTrack(WP)
{
	if (valTrk){  valTrk->setCaption("Track: " + sListTrack);	pSet->track = sListTrack;  }
}

//  new game
void App::btnNewGame(WP)
{
	NewGame();  isFocGui = false;  // off gui
	if (mWndOpts)  mWndOpts->setVisible(isFocGui);
	if (bnQuit)  bnQuit->setVisible(isFocGui);
	mGUI->setVisiblePointer(isFocGui);
	mToolTip->setVisible(false);
}
void App::btnNewGameStart(WP wp)
{
	btnChgTrack(wp);
	btnNewGame(wp);
}

void App::btnQuit(WP)
{
	mShutDown = true;
}


//  [View]  . . . . . . . . . . . . . . . . . . . .    ---- checks ----    . . . . . . . . . . . . . . . . . . . .

#define ChkEv(var)  \
	pSet->var = !pSet->var;  if (wp) {  \
	ButtonPtr chk = wp->castType<MyGUI::Button>(); \
    chk->setStateCheck(pSet->var);  }

void App::chkReverse(WP wp){		ChkEv(trackreverse);	ReadTrkStats();  }

void App::chkParticles(WP wp){		ChkEv(particles);	UpdParsTrails();	}
void App::chkTrails(WP wp){			ChkEv(trails);		UpdParsTrails();	}
void App::chkFps(WP wp){			ChkEv(show_fps);	if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();	}

void App::chkGauges(WP wp){			ChkEv(show_gauges);	ShowHUD();	}
void App::chkMinimap(WP wp){		ChkEv(trackmap);	if (ndMap)  ndMap->setVisible(pSet->trackmap);	}
void App::chkTimes(WP wp){			ChkEv(show_times);	ShowHUD();	}

//void App::chkRacingLine(WP wp){		ChkEv(racingline);	if (ndLine)  ndLine->setVisible(pSet->racingline);	}
void App::chkCamInfo(WP wp){		ChkEv(show_cam);	ShowHUD();	}

void App::chkCarDbgBars(WP wp){		ChkEv(car_dbgbars);	ShowHUD();	}
void App::chkCarDbgTxt(WP wp){		ChkEv(car_dbgtxt);	ShowHUD();	}
void App::chkBltDebug(WP wp){		ChkEv(bltDebug);	}

//  [Car]
void App::chkAbs(WP wp){		ChkEv(abs);		if (pGame)  pGame->ProcessNewSettings();	}
void App::chkTcs(WP wp){		ChkEv(tcs);		if (pGame)  pGame->ProcessNewSettings();	}

void App::chkGear(WP wp){		ChkEv(autoshift);	if (pGame)  pGame->ProcessNewSettings();	}
void App::chkRear(WP wp){		ChkEv(autorear);	if (pGame)  pGame->ProcessNewSettings();	}
void App::chkClutch(WP wp){		ChkEv(autoclutch);	if (pGame)  pGame->ProcessNewSettings();	}

void App::radKmh(WP wp){	bRkmh->setStateCheck(true);  bRmph->setStateCheck(false);  pSet->show_mph = false;  ShowHUD();  }
void App::radMph(WP wp){	bRkmh->setStateCheck(false);  bRmph->setStateCheck(true);  pSet->show_mph = true;   ShowHUD();  }

//  Startup
void App::chkOgreDialog(WP wp){		ChkEv(ogre_dialog);	}
void App::chkAutoStart(WP wp){		ChkEv(autostart);	}
void App::chkEscQuits(WP wp){		ChkEv(escquit);		}
void App::chkBltLines(WP wp){		ChkEv(bltLines);	}

void App::chkLoadPics(WP wp){		ChkEv(loadingbackground);	}
void App::chkVegetCollis(WP wp){	ChkEv(veget_collis);	}



//  [Video]  . . . . . . . . . . . . . . . . . . . .    ---- ------ ----    . . . . . . . . . . . . . . . . . . . .

void App::btnResChng(WP)
{
	if (!resList)  return;

	String mode = resList->getItem(resList->getIndexSelected());
	pSet->windowx = Ogre::StringConverter::parseInt(Ogre::StringUtil::split(mode, "x")[0]);
	pSet->windowy = Ogre::StringConverter::parseInt(Ogre::StringUtil::split(mode, "x")[1]);
	
	mWindow->resize(pSet->windowx, pSet->windowy);
	
	if (pSet->fullscreen)
		mWindow->setFullscreen(true, pSet->windowx, pSet->windowy);
	else
	{
	#ifdef _WIN32
		int sx = GetSystemMetrics(SM_CXSCREEN), sy = GetSystemMetrics(SM_CYSCREEN);
		int cx = max(0,(sx - pSet->windowx) / 2), cy = max(0,(sy - pSet->windowy) / 2);
		mWindow->reposition(cx,cy);
	#else
		mWindow->reposition(0,0);  // center ?..
	#endif
	}
		
	bSizeHUD = true;  // resize HUD
	if (bnQuit)  // reposition Quit btn
	{
		//bnQuit->setRealCoord(0.922,0,0.08,0.03);
		bnQuit->setCoord(pSet->windowx - 0.09*pSet->windowx, 0, 0.09*pSet->windowx, 0.03*pSet->windowy);
	}
}

void App::chkVidBloom(WP wp){		ChkEv(bloom);		Ogre::CompositorManager::getSingleton().setCompositorEnabled(mViewport, "Bloom", pSet->bloom);		}
void App::chkVidHDR(WP wp){			ChkEv(hdr);			Ogre::CompositorManager::getSingleton().setCompositorEnabled(mViewport, "HDR", pSet->hdr);	}
void App::chkVidBlur(WP wp){		ChkEv(motionblur);	Ogre::CompositorManager::getSingleton().setCompositorEnabled(mViewport, "Motion Blur", pSet->motionblur);	}

void App::chkVidFullscr(WP wp){		ChkEv(fullscreen);
	mWindow->setFullscreen(pSet->fullscreen, pSet->windowx, pSet->windowy); mWindow->resize(pSet->windowx, pSet->windowy);
}
void App::chkVidVSync(WP wp)
{		
	ChkEv(vsync); 
	Ogre::Root::getSingleton().getRenderSystem()->setWaitForVerticalBlank(pSet->vsync);
}

void App::slBloomInt(SL)
{
	Real v = val/res;  pSet->bloomintensity = v;
	if (valBloomInt){	Fmt(s, "%4.2f", v);	valBloomInt->setCaption(s);  }
	refreshCompositor();
}
void App::slBloomOrig(SL)
{
	Real v = val/res;  pSet->bloomorig = v;
	if (valBloomOrig){	Fmt(s, "%4.2f", v);	valBloomOrig->setCaption(s);  }
	refreshCompositor();
}
void App::slBlurIntens(SL)
{
	Real v = val/res;  pSet->motionblurintensity = v;
	if (valBlurIntens){	Fmt(s, "%4.2f", v);	valBlurIntens->setCaption(s);  }
	refreshCompositor();
}

//-----------------------------------------------------------------------------------------------------------
//  Key pressed
//-----------------------------------------------------------------------------------------------------------
bool App::keyPressed( const OIS::KeyEvent &arg )
{
	using namespace OIS;
	switch (arg.key)
	{
		case KC_ESCAPE:		// quit
		if (pSet->escquit)  {
			mShutDown = true;	return true;  }

	   	case KC_F1:
	   	case KC_TAB:	// on/off gui
	   	if (!alt)  {
	   		isFocGui = !isFocGui;
	   		if (mWndOpts)	mWndOpts->setVisible(isFocGui);
			if (bnQuit)  bnQuit->setVisible(isFocGui);
	   		if (mGUI)	mGUI->setVisiblePointer(isFocGui);
	   		if (!isFocGui)  mToolTip->setVisible(false);
	   	}	return true;

	   				   		
		case KC_F9:		// car debug text/bars
			if (shift)	{	WP wp = chDbgT;  ChkEv(car_dbgtxt);  ShowHUD();  }
			else		{	WP wp = chDbgB;  ChkEv(car_dbgbars);   ShowHUD();  }
			return true;

		case KC_F11:	//  fps
		if (!shift)
		{	WP wp = chFps;  ChkEv(show_fps); 
			if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();
			return false;
		}	break;

		case KC_F10:	//  blt debug
		if (shift)
		{	WP wp = chBlt;  ChkEv(bltDebug);  return false;
		}	break;


		case KC_F7:		// Times
		{	WP wp = chTimes;  ChkEv(show_times);  ShowHUD();  }
			return false;
			
		case KC_F8:		// Minimap
		{	WP wp = chMinimp;  ChkEv(trackmap);  if (ndMap)  ndMap->setVisible(pSet->trackmap);
		}	return false;
		
		case KC_F5:		//  new game
		//if (ctrl)
		{	NewGame();  return false;
		}	break;
		
		case KC_RETURN:	//  chng trk + new game  after pg up/dn
		if (isFocGui)
		if (mWndTabs->getIndexSelected() == 0)
		{	btnChgTrack(0);
			btnNewGame(0);
		}else if (mWndTabs->getIndexSelected() == 1)
		{	btnChgCar(0);
			btnNewGame(0);
		}
		return false;
	}

	//  change gui tabs
	if (mWndTabs)
	{	int num = mWndTabs->getItemCount();
		if (isFocGui)  switch (arg.key)
		{
	   		case KC_F2:  // prev tab
	   			mWndTabs->setIndexSelected( (mWndTabs->getIndexSelected() - 1 + num) % num );
	   			return true;
	   		case KC_F3:  // next tab
	   			mWndTabs->setIndexSelected( (mWndTabs->getIndexSelected() + 1) % num );
	   			return true;
		}
		/*if (arg.key == KC_F1 && arg.key <= KC_F6)
		{	int n = arg.key - KC_F1;
  			if (n < num)
  			{	mWndTabs->setIndexSelected(n);
  				isFocGui = true;  // on gui
	   			if (mWndOpts)	mWndOpts->setVisible(isFocGui);
	   			if (mGUI)	mGUI->setVisiblePointer(isFocGui);
	   			return true;
	   		}
	   	}/**/
	}

	if (!BaseApp::keyPressed(arg))
		return true;

	return true;
}
