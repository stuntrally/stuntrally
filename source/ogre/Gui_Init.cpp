#include "pch.h"
#include "Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "OgreGame.h"

#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreOverlay.h>
#include "common/Gui_Def.h"
#include "common/MultiList2.h"
using namespace MyGUI;
using namespace Ogre;


///  Gui Init
//-------------------------------------------------------------------------------------


void App::InitGui()
{
	//  change skin
	if (!mGUI)  return;
	QTimer ti;  ti.update();  /// time

	//  load Options layout
	vwGui = LayoutManager::getInstance().loadLayout("Options.layout");
	mLayout = vwGui.at(0);

	//  window
	mWndOpts = mLayout->findWidget("OptionsWnd");
	if (mWndOpts)  {
		mWndOpts->setVisible(isFocGui);
		int sx = mWindow->getWidth(), sy = mWindow->getHeight();
		IntSize w = mWndOpts->getSize();  // center
		mWndOpts->setPosition((sx-w.width)*0.5f, (sy-w.height)*0.5f);  }
	PointerManager::getInstance().setVisible(isFocGui);
	mWndTabs = mGUI->findWidget<Tab>("TabWnd");
	
	//  tooltip  ------
	for (VectorWidgetPtr::iterator it = vwGui.begin(); it != vwGui.end(); ++it)
	{
		setToolTips((*it)->getEnumerator());
		//const std::string& name = (*it)->getName();
	}

	mWndRpl = mGUI->findWidget<Window>("RplWnd",false);
	if (mWndRpl)  mWndRpl->setVisible(false);

	GuiInitTooltip();
		
	GuiCenterMouse();


	//  assign controls

	///  Sliders
    //------------------------------------------------------------------------
	ButtonPtr btn,bchk;  ComboBoxPtr combo;
	ScrollBar* sl;  size_t v;

	GuiInitLang();

	GuiInitGraphics();
	    
	//  view sizes
	Slv(SizeGaug,	(pSet->size_gauges-0.1f) /0.15f);
	Slv(SizeMinimap,(pSet->size_minimap-0.05f) /0.25f);
	Slv(SizeArrow,  (pSet->size_arrow));
	Slv(ZoomMinimap,powf((pSet->zoom_minimap-1.0f) /9.f, 0.5f));
	
	//  particles/trails
	Slv(Particles,	powf(pSet->particles_len /4.f, 0.5f));
	Slv(Trails,		powf(pSet->trails_len /4.f, 0.5f));

	//  reflect
	Slv(ReflSkip,	powf(pSet->refl_skip /1000.f, 0.5f));
	Slv(ReflSize,	pSet->refl_size /float(ciShadowNumSizes));
	Slv(ReflFaces,	pSet->refl_faces /res);
	Slv(ReflDist,	powf((pSet->refl_dist -20.f)/1480.f, 0.5f));
	int value=0;  if (pSet->refl_mode == "static")  value = 0;
	else if (pSet->refl_mode == "single")  value = 1;
	else if (pSet->refl_mode == "full")  value = 2;
	Slv(ReflMode,   value /res);

    //  sound
	Slv(VolMaster,	pSet->vol_master/1.6f);	 Slv(VolEngine,	pSet->vol_engine/1.4f);
	Slv(VolTires,	pSet->vol_tires/1.4f); 	 Slv(VolEnv,	pSet->vol_env/1.4f);
	
	// car color
	UpdCarClrSld();

	///  Checkboxes
    //------------------------------------------------------------------------
	bnQuit = mGUI->findWidget<Button>("Quit");
	if (bnQuit)  {  bnQuit->eventMouseButtonClick += newDelegate(this, &App::btnQuit);  bnQuit->setVisible(isFocGui);  }
	Chk("SSAA", chkVidSSAA, pSet->ssaa);
	Chk("ReverseOn", chkReverse, pSet->trackreverse);
	Chk("ParticlesOn", chkParticles, pSet->particles);	Chk("TrailsOn", chkTrails, pSet->trails);

	Chk("Fps", chkFps, pSet->show_fps);	chFps = mGUI->findWidget<Button>("Fps");
	if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();

	Chk("Digits", chkDigits, pSet->show_digits);
	Chk("Gauges", chkGauges, pSet->show_gauges);  ShowHUD();//
	Chk("Arrow", chkArrow, pSet->check_arrow);

	Chk("Minimap", chkMinimap, pSet->trackmap);	chMinimp = bchk;
	Chk("MiniZoom", chkMiniZoom, pSet->mini_zoomed);  Chk("MiniRot", chkMiniRot, pSet->mini_rotated);
	Chk("MiniTer", chkMiniTer, pSet->mini_terrain);
	Chk("Times", chkTimes, pSet->show_times);	chTimes  = bchk;
	Chk("CamInfo", chkCamInfo, pSet->show_cam);

	Chk("CarDbgBars", chkCarDbgBars, pSet->car_dbgbars);	chDbgB = bchk;
	Chk("CarDbgTxt", chkCarDbgTxt, pSet->car_dbgtxt);		chDbgT = bchk;
	Chk("BulletDebug", chkBltDebug, pSet->bltDebug);	chBlt = bchk;
	Chk("BulletProfilerTxt", chkBltProfilerTxt, pSet->bltProfilerTxt);	chBltTxt = bchk;
	
	//  car setup  todo: for each player ..
	Chk("CarABS",  chkAbs, pSet->abs);			Chk("CarTCS", chkTcs, pSet->tcs);
	Chk("CarGear", chkGear, pSet->autoshift);	Chk("CarRear", chkRear, pSet->autorear);
	Chk("CarRearThrInv", chkRearInv, pSet->rear_inv);
	//  game
	Chk("VegetCollis", chkVegetCollis, pSet->veget_collis);
	Chk("CarCollis", chkCarCollis, pSet->car_collis);
	Btn("btnPlayers1", btnNumPlayers);	Btn("btnPlayers2", btnNumPlayers);
	Btn("btnPlayers3", btnNumPlayers);	Btn("btnPlayers4", btnNumPlayers);
	Chk("chkSplitVertically", chkSplitVert, pSet->split_vertically);
	valLocPlayers = mGUI->findWidget<StaticText>("valLocPlayers");
	if (valLocPlayers)  valLocPlayers->setCaption(toStr(pSet->local_players));
	
	//  kmh/mph radio
	bRkmh = mGUI->findWidget<Button>("kmh");
	bRmph = mGUI->findWidget<Button>("mph");
	if (bRkmh && bRmph)  {  bRkmh->setStateSelected(!pSet->show_mph);  bRmph->setStateSelected(pSet->show_mph);
		bRkmh->eventMouseButtonClick += newDelegate(this, &App::radKmh);
		bRmph->eventMouseButtonClick += newDelegate(this, &App::radMph);  }

	//  startup
	Chk("OgreDialog", chkOgreDialog, pSet->ogre_dialog);
	Chk("AutoStart", chkAutoStart, pSet->autostart);
	Chk("EscQuits", chkEscQuits, pSet->escquit);
	Chk("BltLines", chkBltLines, pSet->bltLines);
	Chk("ShowPictures", chkLoadPics, pSet->loadingbackground);
	
	//  effects
	Chk("AllEffects", chkVidEffects, pSet->all_effects);
	Chk("Bloom", chkVidBloom, pSet->bloom);
	//Chk("HDR", chkVidHDR, pSet->hdr);
	Chk("MotionBlur", chkVidBlur, pSet->motionblur);
	Chk("ssao", chkVidSSAO, pSet->ssao);

	Slv(BloomInt,	pSet->bloomintensity);
	Slv(BloomOrig,	pSet->bloomorig);
	Slv(BlurIntens, pSet->motionblurintensity);
	
	Chk("FullScreen", chkVidFullscr, pSet->fullscreen);
	Chk("VSync", chkVidVSync, pSet->vsync);

	//todo: button_ramp, speed_sens..

	//  replays  ------------------------------------------------------------
	Btn("RplLoad", btnRplLoad);  Btn("RplSave", btnRplSave);
	Btn("RplDelete", btnRplDelete);  Btn("RplRename", btnRplRename);
	//  settings
	Chk("RplChkAutoRec", chkRplAutoRec, pSet->rpl_rec);
	Chk("RplChkGhost", chkRplChkGhost, pSet->rpl_ghost);
	Chk("RplChkBestOnly", chkRplChkBestOnly, pSet->rpl_bestonly);
	Chk("RplChkAlpha", chkRplChkAlpha, pSet->rpl_alpha);
	Chk("RplChkParticles", chkRplChkPar, pSet->rpl_ghostpar);
	//  radios
	Btn("RplBtnAll", btnRplAll);  rbRplAll = btn;
	Btn("RplBtnCur", btnRplCur);  rbRplCur = btn;
	Btn("RplBtnGhosts", btnRplGhosts);  rbRplGhosts = btn;  btn = 0;
	switch (pSet->rpl_listview)  // load from set
	{	case 0: btn = rbRplAll;  break;  case 1: btn = rbRplCur;  break;  case 2: btn = rbRplGhosts;  break;  }
	if (btn)  btn->setStateSelected(true);
	
    if (mWndRpl)
	{	//  replay controls
		Btn("RplToStart", btnRplToStart);  Btn("RplToEnd", btnRplToEnd)
		Btn("RplPlay", btnRplPlay);  btRplPl = btn;
		btn = mGUI->findWidget<Button>("RplBack");	if (btn)  {		btn->eventMouseButtonPressed += newDelegate(this, &App::btnRplBackDn);  btn->eventMouseButtonReleased += newDelegate(this, &App::btnRplBackUp);  }
		btn = mGUI->findWidget<Button>("RplForward");  if (btn)  {	btn->eventMouseButtonPressed += newDelegate(this, &App::btnRplFwdDn);  btn->eventMouseButtonReleased += newDelegate(this, &App::btnRplFwdUp);  }
		
		//  info
		slRplPos = (ScrollBar*)mWndRpl->findWidget("RplSlider");
		if (slRplPos)  slRplPos->eventScrollChangePosition += newDelegate(this, &App::slRplPosEv);

		valRplPerc = mGUI->findWidget<StaticText>("RplPercent");
    	valRplCur = mGUI->findWidget<StaticText>("RplTimeCur");
    	valRplLen = mGUI->findWidget<StaticText>("RplTimeLen");
	}
	//  text desc
	valRplName = mGUI->findWidget<StaticText>("RplName");  valRplName2 = mGUI->findWidget<StaticText>("RplName2");
	valRplInfo = mGUI->findWidget<StaticText>("RplInfo");  valRplInfo2 = mGUI->findWidget<StaticText>("RplInfo2");
	edRplName = mGUI->findWidget<Edit>("RplNameEdit");
	//edRplDesc = mGUI->findWidget<Edit>("RplDesc");

	rplList = mGUI->findWidget<List>("RplList");
	if (rplList)  rplList->eventListChangePosition += newDelegate(this, &App::listRplChng);
	updReplaysList();


	//  car color buttons . . . . .
	Real hsv[10][3] = {
		{0.43,0.86,0.73}, {0.90,1.00,0.80}, {0.00,1.00,0.66}, {0.28,0.57,0.17}, {0.75,0.90,0.55},
		{0.47,0.90,0.80}, {0.50,0.33,0.80}, {0.86,1.00,0.87}, {0.83,0.10,0.58}, {0.70,0.38,0.74}};
	for (int i=0; i<10; i++)
	{
		StaticImagePtr img = mGUI->findWidget<StaticImage>("carClr"+toStr(i));
		Real h = hsv[i][0], s = hsv[i][1], v = hsv[i][2];
		ColourValue c;  c.setHSB(1.f-h, s, v);
		img->setColour(Colour(c.r,c.g,c.b));
		img->eventMouseButtonClick += newDelegate(this, &App::imgBtnCarClr);
		img->setUserString("s", toStr(s));  img->setUserString("h", toStr(h));
		img->setUserString("v", toStr(v));
	}
	Btn("CarClrRandom", btnCarClrRandom);
	Slv(NumLaps, (pSet->num_laps - 1) / 20.f);
	
	TabPtr tPlr = mGUI->findWidget<Tab>("tabPlayer");
	if (tPlr)  tPlr->eventTabChangeSelect += newDelegate(this, &App::tabPlayer);
	
	Btn("btnPlayers1", btnNumPlayers);
	Btn("btnPlayers2", btnNumPlayers);
	Btn("btnPlayers3", btnNumPlayers);
	Btn("btnPlayers4", btnNumPlayers);
	Chk("chkSplitVertically", chkSplitVert, pSet->split_vertically);

	///  Multiplayer
    //------------------------------------------------------------------------
    mWndTabs->setIndexSelected(2);  //- auto switch at start
	tabsNet = mGUI->findWidget<Tab>("tabsNet");
		//TabItem* t1 = tabsNet->getItemAt(0);
		//t1->setEnabled(0);
	//int num = tabsNet ? tabsNet->getItemCount() : 0;
	//tabsNet->setIndexSelected( (tabsNet->getIndexSelected() - 1 + num) % num );
	
    //  server, games
    valNetGames = mGUI->findWidget<StaticText>("valNetGames");
	listServers = mGUI->findWidget<MultiList>("MListServers");
	if (listServers)
	{	listServers->addColumn("Game name", 200);
		listServers->addColumn("Track", 140);
		listServers->addColumn("Players", 80);
		listServers->addColumn("Password", 80);
		listServers->addColumn("Host", 110);
		listServers->addColumn("Port", 80);
	}
	Btn("btnNetRefresh", evBtnNetRefresh);  btnNetRefresh = btn;
	Btn("btnNetJoin", evBtnNetJoin);  btnNetJoin = btn;
	Btn("btnNetCreate", evBtnNetCreate);  btnNetCreate = btn;
	Btn("btnNetDirect", evBtnNetDirect);  btnNetDirect = btn;

	//  game, players
	valNetGameName = mGUI->findWidget<StaticText>("valNetGameName");
	edNetGameName = mGUI->findWidget<Edit>("edNetGameName");
	if (edNetGameName)
		edNetGameName->eventEditTextChange += newDelegate(this, &App::evEdNetGameName);
	
	listPlayers = mGUI->findWidget<MultiList>("MListPlayers");
	if (listPlayers)
	{	listPlayers->addColumn("Player", 140);
		listPlayers->addColumn("Car", 60);
		listPlayers->addColumn("Peers", 60);
		listPlayers->addColumn("Ping", 60);
		listPlayers->addColumn("Ready", 60);
	}
	Btn("btnNetReady", evBtnNetReady);  btnNetReady = btn;
	Btn("btnNetLeave", evBtnNetLeave);	btnNetLeave = btn;

	//  panels to hide tabs
	panelNetServer = mGUI->findWidget<Widget>("panelNetServer");
	panelNetGame = mGUI->findWidget<Widget>("panelNetGame");
	panelNetServer->setVisible(false);
	panelNetGame->setVisible(true);

    //  chat
    valNetChat = mGUI->findWidget<StaticText>("valNetChat");
    edNetChat = mGUI->findWidget<Edit>("edNetChat");  // chat area
    edNetChatMsg = mGUI->findWidget<Edit>("edNetChatMsg");  // user text
    
    //  track
    imgNetTrack = mGUI->findWidget<StaticImage>("imgNetTrack");
    valNetTrack = mGUI->findWidget<StaticText>("valNetTrack");
    edNetTrackInfo = mGUI->findWidget<Edit>("edNetTrackInfo");

	//  settings
	edNetNick = mGUI->findWidget<Edit>("edNetNick");
	edNetServerIP = mGUI->findWidget<Edit>("edNetServerIP");
	edNetServerPort = mGUI->findWidget<Edit>("edNetServerPort");
	edNetLocalPort = mGUI->findWidget<Edit>("edNetLocalPort");
	if (edNetNick)		{	edNetNick->setCaption(pSet->nickname);						
		edNetNick->eventEditTextChange += newDelegate(this, &App::evEdNetNick);	}
	if (edNetServerIP)	{	edNetServerIP->setCaption(pSet->master_server_address);
		edNetServerIP->eventEditTextChange += newDelegate(this, &App::evEdNetServerIP);	}
	if (edNetServerPort){	edNetServerPort->setCaption(toStr(pSet->master_server_port));
		edNetServerPort->eventEditTextChange += newDelegate(this, &App::evEdNetServerPort);	}
	if (edNetLocalPort)	{	edNetLocalPort->setCaption(toStr(pSet->local_port));
		edNetLocalPort->eventEditTextChange += newDelegate(this, &App::evEdNetLocalPort);	}


	///  input tab  -------
	InitInputGui();
	
	InitGuiScrenRes();
	
	
	///  cars list
    //------------------------------------------------------------------------
    carList = mGUI->findWidget<List>("CarList");
    if (carList)
    {	carList->removeAllItems();  int ii = 0;  bool bFound = false;
		strlist li;
		PATHMANAGER::GetFolderIndex(PATHMANAGER::GetCarPath(), li);
		for (strlist::iterator i = li.begin(); i != li.end(); ++i)
		{
			std::ifstream check((PATHMANAGER::GetCarPath() + "/" + *i + "/about.txt").c_str());
			if (check)  {
				carList->addItem(*i);
				if (*i == pSet->car[0]) {  carList->setIndexSelected(ii);  bFound = true;  }
				ii++;  }
		}
		if (!bFound)
			pSet->car[0] = *li.begin();
		carList->eventListChangePosition += newDelegate(this, &App::listCarChng);
    }

	//  cars text, chg btn
    valCar = mGUI->findWidget<StaticText>("CarText");
	valCar->setCaption(TR("#{Car}: ") + pSet->car[0]);  sListCar = pSet->car[0];

    ButtonPtr btnCar = mGUI->findWidget<Button>("ChangeCar");
    if (btnCar)  btnCar->eventMouseButtonClick += newDelegate(this, &App::btnChgCar);

    imgCar = mGUI->findWidget<StaticImage>("CarImg");
    listCarChng(carList,0);


    ///  tracks list, text, chg btn
    //------------------------------------------------------------------------

	//  track text, chg btn
	trkDesc = mGUI->findWidget<Edit>("TrackDesc");
    valTrk = mGUI->findWidget<StaticText>("TrackText");
    if (valTrk)
		valTrk->setCaption(TR("#{Track}: " + pSet->track));  sListTrack = pSet->track;

    GuiInitTrack();

    ButtonPtr btnTrk = mGUI->findWidget<Button>("ChangeTrack");
    if (btnTrk)  btnTrk->eventMouseButtonClick += newDelegate(this, &App::btnChgTrack);

    //  new game
    for (int i=1; i<=3; ++i)
    {	ButtonPtr btnNewG = mGUI->findWidget<Button>("NewGame"+toStr(i));
		if (btnNewG)  btnNewG->eventMouseButtonClick += newDelegate(this, &App::btnNewGame);
	}
	
	bGI = true;  // gui inited, gui events can now save vals

	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Init Gui: ") + toStr(dt) + " ms");
}


void App::UpdCarClrSld(bool upd)
{
	ScrollBar* sl;  size_t v;
	
	// this causes problems (color not applied)
	//bUpdCarClr = false;
	// car color update is instant now anyway.
	
	Slv(CarClrH, pSet->car_hue[iCurCar]);
	Slv(CarClrS, pSet->car_sat[iCurCar]);  if (upd)  bUpdCarClr = true;
	Slv(CarClrV, pSet->car_val[iCurCar]);  bUpdCarClr = true;
}


//  next/prev in list by key
int App::LNext(MyGUI::MultiList2* lp, int rel)
{
	int i = std::max(0, std::min((int)lp->getItemCount()-1, (int)lp->getIndexSelected()+rel ));
	lp->setIndexSelected(i);  //not sorted !..
	lp->beginToItemAt(std::max(0, i-11));  // center
	return i;
}
int App::LNext(MyGUI::ListPtr lp, int rel)
{
	int i = std::max(0, std::min((int)lp->getItemCount()-1, (int)lp->getIndexSelected()+rel ));
	lp->setIndexSelected(i);
	lp->beginToItemAt(std::max(0, i-11));  // center
	return i;
}

void App::trkLNext(int rel)	{
	if (!(isFocGui && mWndTabs->getIndexSelected() == 0))  return;
	listTrackChng(trkMList,LNext(trkMList, rel));  }

void App::carLNext(int rel)	{
	if (!(isFocGui && mWndTabs->getIndexSelected() == 1))  return;
	listCarChng(carList,  LNext(carList, rel));  }

void App::rplLNext(int rel)	{
	if (!(isFocGui && mWndTabs->getIndexSelected() == 3))  return;
	listRplChng(rplList,  LNext(rplList, rel));  }
