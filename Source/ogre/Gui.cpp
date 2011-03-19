#include "stdafx.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "OgreGame.h"
using namespace MyGUI;


///  Gui Init
//-------------------------------------------------------------------------------------
#define res  1000000.f
#define Fmt  sprintf

void App::InitGui()
{
	//  change skin
	if (!mGUI)  return;
	LanguageManager::getInstance().loadUserTags("core_theme_black_blue_tag.xml");
	mGUI->load("core_skin.xml");

	//  load Options layout
	VectorWidgetPtr& rootV = LayoutManager::getInstance().load("Options.layout");
	mLayout = rootV.at(0);

	mWndOpts = mLayout->findWidget("OptionsWnd");
	if (mWndOpts)  {
		mWndOpts->setVisible(isFocGui);
		int sx = mWindow->getWidth(), sy = mWindow->getHeight();
		IntSize w = mWndOpts->getSize();  // center
		mWndOpts->setPosition((sx-w.width)*0.5f, (sy-w.height)*0.5f);  }
	mGUI->setVisiblePointer(isFocGui);
	mWndTabs = (TabPtr)mLayout->findWidget("TabWnd");
		
	//  center mouse pos
	int xm = mWindow->getWidth()/2, ym = mWindow->getHeight()/2;
	MyGUI::InputManager::getInstance().injectMouseMove(xm, ym, 0);
	OIS::MouseState &ms = const_cast<OIS::MouseState&>(mMouse->getMouseState());
	ms.X.abs = xm;  ms.Y.abs = ym;

	//  assign controls  ----------------------

	///  Sliders
    //------------------------------------
	HScrollPtr sl;  ComboBoxPtr combo;  size_t v;

	// get slider, assign event, get valtext, set value from settings
	#define Slv(name, vset)  \
		sl = (HScrollPtr)mLayout->findWidget(#name);  \
		if (sl)  sl->eventScrollChangePosition = newDelegate(this, &App::sl##name);  \
		val##name = (StaticTextPtr)(mLayout->findWidget(#name"Val"));  \
		v = vset*res;  if (sl)  sl->setScrollPosition(v);	sl##name(sl, v);

	//  detail
	Slv(TerDetail,	powf(pSet->terdetail /20.f, 0.5f));
	Slv(TerDist,	powf(pSet->terdist /1000.f, 0.5f));
	Slv(ViewDist,	powf((pSet->view_distance -50.f)/6950.f, 0.5f));
	Slv(RoadDist,	powf(pSet->road_dist /4.f, 0.5f));

	//  textures
	combo = (ComboBoxPtr)mLayout->findWidget("TexFiltering");
	if (combo)  combo->eventComboChangePosition = newDelegate(this, &App::comboTexFilter);
	Slv(Anisotropy,	pSet->anisotropy /res);
	Slv(Shaders,	pSet->shaders /res);
	
	//  particles/trails
	Slv(Particles,	powf(pSet->particles_len /4.f, 0.5f));
	Slv(Trails,		powf(pSet->trails_len /4.f, 0.5f));

	//  trees/grass
	Slv(Trees,		powf(pSet->trees /4.f, 0.5f));
	Slv(Grass,		powf(pSet->grass /4.f, 0.5f));
	Slv(TreesDist,	powf((pSet->trees_dist-0.5f) /6.5f, 0.5f));
	Slv(GrassDist,	powf((pSet->grass_dist-0.5f) /6.5f, 0.5f));

	//  view sizes
	Slv(SizeGaug,	(pSet->size_gauges-0.1f) /0.15f);
	Slv(SizeMinmap,	(pSet->size_minimap-0.05f) /0.25f);
	
	//  reflect
	Slv(ReflSkip,	powf(pSet->refl_skip /1000.f, 0.5f));
	Slv(ReflSize,	pSet->refl_size /res);
	Slv(ReflFaces,	pSet->refl_faces /res);
	Slv(ReflDist,	powf((pSet->refl_dist -20.f)/1480.f, 0.5f));

	//  shadows
	Slv(ShadowType,	pSet->shadow_type /res);
	Slv(ShadowCount,(pSet->shadow_count-2) /2.f);
	Slv(ShadowSize,	pSet->shadow_size /float(ciShadowNumSizes));
	Slv(ShadowDist,	powf((pSet->shadow_dist -50.f)/4750.f, 0.5f));

    ButtonPtr btn = (ButtonPtr)mLayout->findWidget("Apply");
    if (btn)  btn->eventMouseButtonClick = newDelegate(this, &App::btnShadows);
    
    //  sound
	Slv(VolMaster,	pSet->vol_master/1.6f);	 Slv(VolEngine,	pSet->vol_engine/1.4f);
	Slv(VolTires,	pSet->vol_tires/1.4f); 	 Slv(VolEnv,	pSet->vol_env/1.4f);
	
	// car color
	Slv(CarClrH, pSet->car_hue);
	Slv(CarClrS, (pSet->car_sat +1)*0.5f);  Slv(CarClrV, (pSet->car_val +1)*0.5f);


	///  Checkboxes
    //------------------------------------
	ButtonPtr bchk;
	#define Chk(name, event, var)  \
		bchk = mGUI->findWidget<Button>(name);  \
		if (bchk)  {  bchk->eventMouseButtonClick = newDelegate(this, &App::event);  \
			bchk->setStateCheck(pSet->var);  }

	Chk("ReverseOn", chkReverse, trackreverse);
	Chk("ParticlesOn", chkParticles, particles);	Chk("TrailsOn", chkTrails, trails);

	Chk("Fps", chkFps, show_fps);	chFps = mGUI->findWidget<Button>("Fps");
	if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();
	Chk("Gauges", chkGauges, show_gauges);  ShowHUD();//

	Chk("Minimap", chkMinimap, trackmap);	chMinimp = mGUI->findWidget<Button>("Minimap");
	Chk("Times", chkTimes, show_times);		chTimes  = mGUI->findWidget<Button>("Times");
	//Chk("Racingline", chkRacingLine, racingline);
	Chk("CamInfo", chkCamInfo, show_cam);

	Chk("CarDbgBars", chkCarDbgBars, car_dbgbars);	chDbgB = mGUI->findWidget<Button>("CarDbgBars");
	Chk("CarDbgTxt", chkCarDbgTxt, car_dbgtxt);		chDbgT = mGUI->findWidget<Button>("CarDbgTxt");
	Chk("BulletDebug", chkBltDebug, bltDebug);	chBlt = mGUI->findWidget<Button>("BulletDebug");
	
	//  abs, tcs
	Chk("CarABS",  chkAbs, abs);		Chk("CarTCS", chkTcs, tcs);
	Chk("CarGear", chkGear, autoshift);
	Chk("CarRear", chkRear, autorear);	Chk("CarClutch", chkClutch, autoclutch);

	//  kmh/mph radio
	bRkmh = mGUI->findWidget<Button>("kmh");
	bRmph = mGUI->findWidget<Button>("mph");
	if (bRkmh && bRmph)  {  bRkmh->setStateCheck(!pSet->show_mph);  bRmph->setStateCheck(pSet->show_mph);
		bRkmh->eventMouseButtonClick = newDelegate(this, &App::radKmh);
		bRmph->eventMouseButtonClick = newDelegate(this, &App::radMph);  }

	bchk = mGUI->findWidget<Button>("TrGrReset");
	if (bchk)  bchk->eventMouseButtonClick = newDelegate(this, &App::btnTrGrReset);

	//  startup
	Chk("OgreDialog", chkOgreDialog, ogre_dialog);
	Chk("AutoStart", chkAutoStart, autostart);
	Chk("EscQuits", chkEscQuits, escquit);
	Chk("BltLines", chkBltLines, bltLines);

	//button_ramp, speed_sens..

	
    ///  cars list
    //------------------------------------
    ListPtr carList = (ListPtr)mLayout->findWidget("CarList");
    if (carList)
    {	carList->removeAllItems();  int ii = 0;  bool bFound = false;

		std::list <std::string> li;
		pGame->pathmanager.GetFolderIndex(pGame->pathmanager.GetCarPath(), li);
		for (std::list <std::string>::iterator i = li.begin(); i != li.end(); ++i)
		{
			ifstream check((pGame->pathmanager.GetCarPath() + "/" + *i + "/about.txt").c_str());
			if (check)  {
				carList->addItem(*i);
				if (*i == pSet->car) {  carList->setIndexSelected(ii);  bFound = true;  }
				ii++;  }
		}
		if (!bFound)
			pSet->car = *li.begin();
		carList->eventListChangePosition = newDelegate(this, &App::listCarChng);
    }

	//  cars text, chg btn
    valCar = (StaticTextPtr)mLayout->findWidget("CarText");
	valCar->setCaption("Car: " + pSet->car);  sListCar = pSet->car;

    ButtonPtr btnCar = (ButtonPtr)mLayout->findWidget("ChangeCar");
    if (btnCar)  btnCar->eventMouseButtonClick = newDelegate(this, &App::btnChgCar);


    ///  tracks list, text, chg btn
    //------------------------------------
    ListPtr trkList = (ListPtr)mLayout->findWidget("TrackList");
    if (trkList)
    {	trkList->removeAllItems();
		int ii = 0, si = 0;  bool bFound = false;

		std::list <std::string> li;
		pGame->pathmanager.GetFolderIndex(pGame->pathmanager.GetTrackPath(), li);
		for (std::list <std::string>::iterator i = li.begin(); i != li.end(); ++i)
		{
			string s = pGame->pathmanager.GetTrackPath() + "/" + *i + "/track.txt";
			ifstream check(s.c_str());
			if (check)  {
				//string displayname;  getline(check, displayname);
				trkList->addItem(*i);
				if (*i == pSet->track)  {  si = ii;
					trkList->setIndexSelected(si);
					bFound = true;  }
				ii++;  }
		}
		//  not found last track, set 1st
		if (!bFound)
			pSet->track = *li.begin();
		trkList->beginToItemAt(max(0, si-11));  // center
		trkList->eventListChangePosition = newDelegate(this, &App::listTrackChng);
    }

	//  track text, chg btn
    valTrk = (StaticTextPtr)mLayout->findWidget("TrackText");
    if (valTrk)
		valTrk->setCaption("Track: " + pSet->track);  sListTrack = pSet->track;
	trkDesc = (EditPtr)mLayout->findWidget("TrackDesc");

	//  track stats
	for (int i=0; i < StTrk; ++i)
		stTrk[i] = (StaticTextPtr)mLayout->findWidget("iv"+toStr(i+1));

	//  preview images
    imgCar = (StaticImagePtr)mLayout->findWidget("CarImg");
    imgPrv = (StaticImagePtr)mLayout->findWidget("TrackImg");
    imgTer = (StaticImagePtr)mLayout->findWidget("TrkTerImg");
    imgMini = (StaticImagePtr)mLayout->findWidget("TrackMap");
    listCarChng(carList,0);  listTrackChng(trkList,0);

    ButtonPtr btnTrk = (ButtonPtr)mLayout->findWidget("ChangeTrack");
    if (btnTrk)  btnTrk->eventMouseButtonClick = newDelegate(this, &App::btnChgTrack);

    //  new game
    for (int i=1; i<=4; ++i)
    {	ButtonPtr btnNewG = (ButtonPtr)mLayout->findWidget("NewGame"+toStr(i));
		if (btnNewG)  btnNewG->eventMouseButtonClick = newDelegate(this, &App::btnNewGame);
	}
}


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
	int v = __max(0, __min(ciShadowNumSizes-1, ciShadowNumSizes * val/res));	pSet->refl_size = v;
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
	int v = __max(0, __min(ciShadowNumSizes-1, ciShadowNumSizes * val/res));	pSet->shadow_size = v;
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
	mGUI->setVisiblePointer(isFocGui);
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
//void OgreGame::chkFps(WP wp){		ChkEv(bltDebug);	}

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
void App::chkBltLines(WP wp){		ChkEv(bltLines);		}


///  . .  util tracks stats  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

void App::ReadTrkStats()
{
	string sTrk = "data/tracks/";
	String sRd = sTrk + sListTrack + "/road.xml";
	String sSc = sTrk + sListTrack + "/scene.xml";

	SplineRoad rd(pGame);  rd.LoadFile(sRd,false);  // load
	Scene sc;  sc.LoadXml(sSc);  // fails to defaults
	TIMER tim;  tim.Load(sTrk + sListTrack + "/records.txt", 0.f, pGame->error_output);
	tim.AddCar(sListCar);  tim.SetPlayerCarID(0);
	UpdGuiRdStats(&rd,sc, tim.GetBestLap(pSet->trackreverse));
}

void App::UpdGuiRdStats(const SplineRoad* rd, const Scene& sc, float time)
{
	Fmt(s, "%5.3f km", sc.td.fTerWorldSize / 1000.f);	if (stTrk[1])  stTrk[1]->setCaption(s);
	if (!rd)  return;
	Fmt(s, "%5.3f km", rd->st.Length / 1000.f);			if (stTrk[0])  stTrk[0]->setCaption(s);

	Fmt(s, "%4.2f m", rd->st.WidthAvg);		if (stTrk[2])  stTrk[2]->setCaption(s);
	Fmt(s, "%3.1f m", rd->st.HeightDiff);	if (stTrk[3])  stTrk[3]->setCaption(s);

	Fmt(s, "%3.1f%%", rd->st.OnTer);	if (stTrk[4])  stTrk[4]->setCaption(s);
	Fmt(s, "%3.1f%%", rd->st.Pipes);	if (stTrk[5])  stTrk[5]->setCaption(s);
					
	//Fmt(s, "%4.2f%%", rd->st.Yaw);	if (stTrk[6])  stTrk[6]->setCaption(s);
	//Fmt(s, "%4.2f%%", rd->st.Pitch);	if (stTrk[7])  stTrk[7]->setCaption(s);
	//Fmt(s, "%4.2f%%", rd->st.Roll);	if (stTrk[8])  stTrk[8]->setCaption(s);
	
	//  best time, avg vel,
	if (time < 0.1f)
	{	Fmt(s, "%s", GetTimeString(0.f).c_str());	if (stTrk[6])  stTrk[6]->setCaption(s);
		if (pSet->show_mph)	Fmt(s, "0 mph");
		else				Fmt(s, "0 km/h");		if (stTrk[7])  stTrk[7]->setCaption(s);
	}else
	{	Fmt(s, "%s", GetTimeString(time).c_str());	if (stTrk[6])  stTrk[6]->setCaption(s);
		if (pSet->show_mph)	Fmt(s, "%4.1f mph", rd->st.Length / time * 2.23693629f);
		else				Fmt(s, "%4.1f km/h", rd->st.Length / time * 3.6f);
		if (stTrk[7])  stTrk[7]->setCaption(s);
		//Fmt(s, "%4.2f%%", rd->st.Pitch);	if (stTrk[8])  stTrk[8]->setCaption(s);
	}
	if (trkDesc)  // desc
		trkDesc->setCaption(rd->sTxtDesc.c_str());
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

	   	case KC_TAB:	// on/off gui
	   	if (!alt)  {
	   		isFocGui = !isFocGui;
	   		if (mWndOpts)	mWndOpts->setVisible(isFocGui);
	   		if (mGUI)	mGUI->setVisiblePointer(isFocGui);
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
	}

	//  change gui tabs
	if (mWndTabs)
	{	int num = mWndTabs->getItemCount();
		if (isFocGui)  switch (arg.key)
		{
	   		case KC_1:  // prev tab
	   			mWndTabs->setIndexSelected( (mWndTabs->getIndexSelected() - 1 + num) % num );
	   			return true;
	   		case KC_2:  // next tab
	   			mWndTabs->setIndexSelected( (mWndTabs->getIndexSelected() + 1) % num );
	   			return true;
		}
		if (arg.key >= KC_F1 && arg.key <= KC_F6)
		{	int n = arg.key - KC_F1;
  			if (n < num)
  			{	mWndTabs->setIndexSelected(n);
  				isFocGui = true;  // on gui
	   			if (mWndOpts)	mWndOpts->setVisible(isFocGui);
	   			if (mGUI)	mGUI->setVisiblePointer(isFocGui);
	   			return true;
	   		}
	   	}
	}

	if (!BaseApp::keyPressed(arg))
		return true;

	return true;
}
