#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "OgreGame.h"
#include "common/MaterialGen/MaterialGenerator.h"

#include <boost/filesystem.hpp>

#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreOverlay.h>
#include "common/Gui_Def.h"
#include "common/MultiList2.h"
#include "common/Slider.h"

using namespace MyGUI;
using namespace Ogre;


///  Gui Init
//---------------------------------------------------------------------------------------------------------------------

void App::InitGui()
{
	//  change skin
	if (!mGUI)  return;
	popup.mGUI = mGUI;
	popup.mPlatform = mPlatform;
	QTimer ti;  ti.update();  /// time

	//  new widgets
	MyGUI::FactoryManager::getInstance().registerFactory<MultiList2>("Widget");
	MyGUI::FactoryManager::getInstance().registerFactory<Slider>("Widget");
	

	//  load Options layout
	vwGui = LayoutManager::getInstance().loadLayout("Options.layout");
	//mLayout = vwGui.at(0);

	//  window
	mWndMain = mGUI->findWidget<Window>("MainMenuWnd",false);
	mWndGame = mGUI->findWidget<Window>("GameWnd",false);
	mWndReplays = mGUI->findWidget<Window>("ReplaysWnd",false);
	mWndHelp = mGUI->findWidget<Window>("HelpWnd",false);
	mWndOpts = mGUI->findWidget<Window>("OptionsWnd",false);
	mWndChampStage = mGUI->findWidget<Window>("WndChampStage",false);  mWndChampStage->setVisible(false);
	mWndChampEnd = mGUI->findWidget<Window>("WndChampEnd",false);  mWndChampEnd->setVisible(false);
	mWndNetEnd = mGUI->findWidget<Window>("WndNetEnd",false);  mWndNetEnd->setVisible(false);
	
	//  main menu
	for (int i=0; i < WND_ALL; ++i)
	{
		const String s = toStr(i);
		mWndMainPanels[i] = mWndMain->findWidget("PanMenu"+s);
		mWndMainBtns[i] = (ButtonPtr)mWndMain->findWidget("BtnMenu"+s);
		mWndMainBtns[i]->eventMouseButtonClick += newDelegate(this, &App::MainMenuBtn);
	}
		
	updMouse();
	
	//  center
	//mWndOpts->setVisible(isFocGui);
	int sx = mWindow->getWidth(), sy = mWindow->getHeight();
	IntSize w = mWndMain->getSize();
	mWndMain->setPosition((sx-w.width)*0.5f, (sy-w.height)*0.5f);

	TabPtr tab;
	tab = mGUI->findWidget<Tab>("TabWndGame");    tab->setIndexSelected(1); tab->setSmoothShow(false);	mWndTabsGame = tab;		tab->eventTabChangeSelect += newDelegate(this, &App::MenuTabChg);
	tab = mGUI->findWidget<Tab>("TabWndReplays"); tab->setIndexSelected(1);	tab->setSmoothShow(false);	mWndTabsRpl = tab;		tab->eventTabChangeSelect += newDelegate(this, &App::MenuTabChg);
	tab = mGUI->findWidget<Tab>("TabWndHelp");    tab->setIndexSelected(1);	tab->setSmoothShow(false);	mWndTabsHelp = tab;		tab->eventTabChangeSelect += newDelegate(this, &App::MenuTabChg);
	tab = mGUI->findWidget<Tab>("TabWndOptions"); tab->setIndexSelected(1); tab->setSmoothShow(false);	mWndTabsOpts = tab;		tab->eventTabChangeSelect += newDelegate(this, &App::MenuTabChg);
	if (pSet->inMenu == WND_Champ)  mWndTabsGame->setIndexSelected(5);

	//  get sub tabs
	vSubTabsGame.clear();
	for (size_t i=0; i < mWndTabsGame->getItemCount(); ++i)
	{
		MyGUI::TabPtr sub = (TabPtr)mWndTabsGame->getItemAt(i)->findWidget(i==4 ? "tabsNet" : "tabPlayer!");//car tab wrong-
		vSubTabsGame.push_back(sub);  // 0 for not found
	}
	vSubTabsOpts.clear();
	for (size_t i=0; i < mWndTabsOpts->getItemCount(); ++i)
	{
		MyGUI::TabPtr sub = (TabPtr)mWndTabsOpts->getItemAt(i)->findWidget(i==4 ? "InputTab" : "SubTab");
		vSubTabsOpts.push_back(sub);
	}

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

	toggleGui(false);


	//  assign controls

	///  Sliders
    //------------------------------------------------------------------------
	ButtonPtr btn,bchk;  ComboBoxPtr combo;  Slider* sl;

	GuiInitLang();

	GuiInitGraphics();
	    
	//  view sizes
	Slv(SizeGaug,	(pSet->size_gauges-0.1f) /0.15f);
	Slv(TypeGaug,	pSet->gauges_type /5.f);
	Slv(SizeMinimap,(pSet->size_minimap-0.05f) /0.25f);
	Slv(SizeArrow,  (pSet->size_arrow));
	Slv(ZoomMinimap,powf((pSet->zoom_minimap-1.0f) /9.f, 0.5f));
	Slv(CountdownTime,  pSet->gui.pre_time / 0.5f /6.f);
	Slv(GraphsType,	float(pSet->graphs_type) /graph_types);  slGraphT = sl;
	
	//  particles/trails
	Slv(Particles,	powf(pSet->particles_len /4.f, 0.5f));
	Slv(Trails,		powf(pSet->trails_len /4.f, 0.5f));

	//  reflect
	Slv(ReflSkip,	powf(pSet->refl_skip /1000.f, 0.5f));
	Slv(ReflSize,	pSet->refl_size /float(ciShadowNumSizes));
	Slv(ReflFaces,	pSet->refl_faces /6.f);
	Slv(ReflDist,	powf((pSet->refl_dist -20.f)/1480.f, 0.5f));
	Slv(ReflMode,   pSet->refl_mode /2.f);

    //  sound
	Slv(VolMaster,	pSet->vol_master/1.6f);
	Slv(VolEngine,	pSet->vol_engine/1.4f);		 Slv(VolTires, pSet->vol_tires/1.4f);
	Slv(VolSusp,	pSet->vol_susp/1.4f);		 Slv(VolEnv,   pSet->vol_env/1.4f);
	Slv(VolFlSplash, pSet->vol_fl_splash/1.4f);  Slv(VolFlCont,	  pSet->vol_fl_cont/1.4f);
	Slv(VolCarCrash, pSet->vol_car_crash/1.4f);  Slv(VolCarScrap, pSet->vol_car_scrap/1.4f);
	
	// car color
	UpdCarClrSld();

	///  Checkboxes
    //------------------------------------------------------------------------
	bnQuit = mGUI->findWidget<Button>("Quit");
	if (bnQuit)  {  bnQuit->eventMouseButtonClick += newDelegate(this, &App::btnQuit);  bnQuit->setVisible(isFocGui);  }
	Chk("SSAA", chkVidSSAA, pSet->ssaa);
	Chk("ReverseOn", chkReverse, pSet->gui.trackreverse);
	Chk("ParticlesOn", chkParticles, pSet->particles);	Chk("TrailsOn", chkTrails, pSet->trails);

	//  hud
	Chk("Digits", chkDigits, pSet->show_digits);
	Chk("Gauges", chkGauges, pSet->show_gauges);  ShowHUD();//
	Chk("Arrow", chkArrow, pSet->check_arrow);
	
	Chk("Minimap", chkMinimap, pSet->trackmap);	chMinimp = bchk;
	Chk("MiniZoom", chkMiniZoom, pSet->mini_zoomed);  Chk("MiniRot", chkMiniRot, pSet->mini_rotated);
	Chk("MiniTer", chkMiniTer, pSet->mini_terrain);

	Chk("CamInfo", chkCamInfo, pSet->show_cam);
	Chk("CamTilt", chkCamTilt, pSet->cam_tilt);

	Chk("Times", chkTimes, pSet->show_times);	chTimes  = bchk;
	Chk("Opponents", chkOpponents, pSet->show_opponents);  chOpponents = bchk;
	Chk("OpponentsSort", chkOpponentsSort, pSet->opplist_sort);

	//  other
	Chk("Fps", chkFps, pSet->show_fps);  chFps = bchk;
	if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();
	Chk("Wireframe", chkWireframe, mbWireFrame);  chWire = bchk;

	Chk("ProfilerTxt", chkProfilerTxt, pSet->profilerTxt);	chProfTxt = bchk;
	Chk("BulletDebug", chkBltDebug, pSet->bltDebug);		chBlt = bchk;
	Chk("BulletProfilerTxt", chkBltProfilerTxt, pSet->bltProfilerTxt);	chBltTxt = bchk;

	Chk("CarDbgBars", chkCarDbgBars, pSet->car_dbgbars);	chDbgB = bchk;
	Chk("CarDbgTxt", chkCarDbgTxt, pSet->car_dbgtxt);		chDbgT = bchk;
	Chk("Graphs", chkGraphs, pSet->show_graphs);		chGraphs = bchk;

	//  car setup  todo: for each player ..
	Chk("CarABS",  chkAbs, pSet->abs);			Chk("CarTCS", chkTcs, pSet->tcs);
	Chk("CarGear", chkGear, pSet->autoshift);	Chk("CarRear", chkRear, pSet->autorear);
	Chk("CarRearThrInv", chkRearInv, pSet->rear_inv);
	//  game
	Chk("VegetCollis", chkVegetCollis, pSet->gui.collis_veget);
	Chk("CarCollis", chkCarCollis, pSet->gui.collis_cars);
	Chk("RoadWCollis", chkRoadWCollis, pSet->gui.collis_roadw);
	//  boost, flip combos
	Cmb(combo, "CmbBoost", comboBoost);
	if (combo)
	{	combo->removeAllItems();
		combo->addItem(TR("#{Never}"));
		combo->addItem(TR("#{FuelLap}"));
		combo->addItem(TR("#{FuelTime}"));
		combo->addItem(TR("#{Always}"));
		combo->setIndexSelected(pSet->gui.boost_type);
	}
	Cmb(combo, "CmbFlip", comboFlip);
	if (combo)
	{	combo->removeAllItems();
		combo->addItem(TR("#{Never}"));
		combo->addItem(TR("#{FuelBoost}"));
		combo->addItem(TR("#{Always}"));
		combo->setIndexSelected(pSet->gui.flip_type);
	}

	Btn("btnPlayers1", btnNumPlayers);	Btn("btnPlayers2", btnNumPlayers);
	Btn("btnPlayers3", btnNumPlayers);	Btn("btnPlayers4", btnNumPlayers);
	Chk("chkSplitVertically", chkSplitVert, pSet->split_vertically);
	valLocPlayers = mGUI->findWidget<StaticText>("valLocPlayers");
	if (valLocPlayers)  valLocPlayers->setCaption(toStr(pSet->gui.local_players));

	//  kmh/mph radio
	bRkmh = mGUI->findWidget<Button>("kmh");
	bRmph = mGUI->findWidget<Button>("mph");
	if (bRkmh && bRmph)  {  bRkmh->setStateSelected(!pSet->show_mph);  bRmph->setStateSelected(pSet->show_mph);
		bRkmh->eventMouseButtonClick += newDelegate(this, &App::radKmh);
		bRmph->eventMouseButtonClick += newDelegate(this, &App::radMph);  }

	//  startup
	Chk("MouseCapture", chkMouseCapture, pSet->x11_capture_mouse);
	Chk("OgreDialog", chkOgreDialog, pSet->ogre_dialog);
	Chk("AutoStart", chkAutoStart, pSet->autostart);
	Chk("EscQuits", chkEscQuits, pSet->escquit);
	Chk("BltLines", chkBltLines, pSet->bltLines);
	Chk("ShowPictures", chkLoadPics, pSet->loadingbackground);
	Chk("MultiThread", chkMultiThread, pSet->multi_thr > 0);
	
	//  effects
	Chk("AllEffects", chkVidEffects, pSet->all_effects);
	Chk("Bloom", chkVidBloom, pSet->bloom);
	//Chk("HDR", chkVidHDR, pSet->hdr);
	Chk("MotionBlur", chkVidBlur, pSet->motionblur);
	Chk("ssao", chkVidSSAO, pSet->ssao);
	Chk("softparticles", chkVidSoftParticles, pSet->softparticles);
	if (!MaterialGenerator::MRTSupported())
		mGUI->findWidget<Button>("softparticles")->setEnabled(false);
	Chk("DepthOfField", chkVidDepthOfField, pSet->dof);
	if (!MaterialGenerator::MRTSupported())
		mGUI->findWidget<Button>("DepthOfField")->setEnabled(false);
	Chk("godrays", chkVidGodRays, pSet->godrays);
	if (!MaterialGenerator::MRTSupported())
		mGUI->findWidget<Button>("godrays")->setEnabled(false);
	Chk("filmgrain", chkVidFilmGrain, pSet->filmgrain);
	
	Slv(BloomInt,	pSet->bloomintensity);
	Slv(BloomOrig,	pSet->bloomorig);
	Slv(BlurIntens, pSet->motionblurintensity);
	Slv(DepthOfFieldFocus, powf(pSet->depthOfFieldFocus/2000.f, 0.5f));
	Slv(DepthOfFieldFar,   powf(pSet->depthOfFieldFar/2000.f, 0.5f));
	
	Chk("FullScreen", chkVidFullscr, pSet->fullscreen);
	Chk("VSync", chkVidVSync, pSet->vsync);

	
	//  replays  ------------------------------------------------------------
	Btn("RplLoad", btnRplLoad);  Btn("RplSave", btnRplSave);
	Btn("RplDelete", btnRplDelete);  Btn("RplRename", btnRplRename);
	//  settings
	Chk("RplChkAutoRec", chkRplAutoRec, pSet->rpl_rec);
	Chk("RplChkGhost", chkRplChkGhost, pSet->rpl_ghost);
	Chk("RplChkBestOnly", chkRplChkBestOnly, pSet->rpl_bestonly);
	Chk("RplChkAlpha", chkRplChkAlpha, pSet->rpl_alpha);
	Chk("RplChkParticles", chkRplChkPar, pSet->rpl_ghostpar);
	Slv(RplNumViewports, (pSet->rpl_numViews-1) / 3.f);
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
		slRplPos = (Slider*)mWndRpl->findWidget("RplSlider");
		if (slRplPos)  slRplPos->eventValueChanged += newDelegate(this, &App::slRplPosEv);

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
	Real hsv[15][3] = {
		{0.43,0.80,0.80}, {0.47,0.90,0.85}, {0.50,0.33,0.90}, {0.56,0.80,0.80}, {0.55,0.50,0.32}, 
		{0.75,0.90,0.90}, {0.70,0.80,0.80}, {0.86,1.00,0.97}, {0.91,1.00,1.00}, {0.00,0.97,0.93},
		{0.35,0.70,0.40}, {0.28,0.57,0.17}, {0.28,0.00,0.10}, {0.83,0.00,0.58}, {0.41,0.00,0.88}, };
	for (int i=0; i<15; i++)
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
	Slv(NumLaps, (pSet->gui.num_laps - 1) / 20.f);
	
	TabPtr tPlr = mGUI->findWidget<Tab>("tabPlayer");
	if (tPlr)  tPlr->eventTabChangeSelect += newDelegate(this, &App::tabPlayer);
	
	Btn("btnPlayers1", btnNumPlayers);	Btn("btnPlayers2", btnNumPlayers);
	Btn("btnPlayers3", btnNumPlayers);	Btn("btnPlayers4", btnNumPlayers);
	Chk("chkSplitVertically", chkSplitVert, pSet->split_vertically);


	///  Multiplayer
	//------------------------------------------------------------------------
	tabsNet = mGUI->findWidget<Tab>("tabsNet");
		//TabItem* t1 = tabsNet->getItemAt(0);
		//t1->setEnabled(0);
	//int num = tabsNet ? tabsNet->getItemCount() : 0;
	//tabsNet->setIndexSelected( (tabsNet->getIndexSelected() - 1 + num) % num );
	
	//  server, games
	valNetGames = mGUI->findWidget<StaticText>("valNetGames");
	listServers = mGUI->findWidget<MultiList>("MListServers");  int c=0;
	if (listServers)
	{	listServers->addColumn("#C0FFC0"+TR("#{Game name}"), 180);  ++c;
		listServers->addColumn("#50FF50"+TR("#{Track}"), 140);  ++c;
		listServers->addColumn("#80FFC0"+TR("#{Laps}"), 70);  ++c;
		listServers->addColumn("#FFFF00"+TR("#{Players}"), 70);  ++c;
		listServers->addColumn("#80FFFF"+TR("#{Collis.}"), 80);  ++c;
		listServers->addColumn("#A0D0FF"+TR("#{Boost}"), 100);  ++c;
		listServers->addColumn("#FF6060"+TR("#{Locked}"), 70);  iColLock = c;  ++c;
		listServers->addColumn("#FF9000"+TR("#{NetHost}"), 130);  iColHost = c;  ++c;
		listServers->addColumn("#FFB000"+TR("#{NetPort}"), 80);  iColPort = c;  ++c;
	}
	Btn("btnNetRefresh", evBtnNetRefresh);  btnNetRefresh = btn;
	Btn("btnNetJoin", evBtnNetJoin);  btnNetJoin = btn;
	Btn("btnNetCreate", evBtnNetCreate);  btnNetCreate = btn;
	Btn("btnNetDirect", evBtnNetDirect);  btnNetDirect = btn;

	//  game, players
	valNetGameName = mGUI->findWidget<StaticText>("valNetGameName");
	edNetGameName = mGUI->findWidget<Edit>("edNetGameName");
	if (edNetGameName)
	{	edNetGameName->setCaption(pSet->netGameName);
		edNetGameName->eventEditTextChange += newDelegate(this, &App::evEdNetGameName);
	}
	//  password
	valNetPassword = mGUI->findWidget<StaticText>("valNetPassword");
	edNetPassword = mGUI->findWidget<Edit>("edNetPassword");
	if (edNetPassword)
		edNetPassword->eventEditTextChange += newDelegate(this, &App::evEdNetPassword);

	listPlayers = mGUI->findWidget<MultiList>("MListPlayers");
	if (listPlayers)
	{	listPlayers->addColumn("#C0E0FF"+TR("#{Player}"), 140);
		listPlayers->addColumn("#80FFFF"+TR("#{Car}"), 60);
		listPlayers->addColumn("#C0C0FF"+TR("#{Peers}"), 60);
		listPlayers->addColumn("#C0FFFF"+TR("#{Ping}"), 60);
		listPlayers->addColumn("#C0FF80"+TR("#{NetReady}"), 60);
	}
	Btn("btnNetReady", evBtnNetReady);  btnNetReady = btn;
	Btn("btnNetLeave", evBtnNetLeave);	btnNetLeave = btn;

	//  panels to hide tabs
	panelNetServer = mGUI->findWidget<Widget>("panelNetServer");
	panelNetGame = mGUI->findWidget<Widget>("panelNetGame");
	//panelNetTrack = mGUI->findWidget<Widget>("panelNetTrack",false);
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

	
	//  quick help text
	MyGUI::EditPtr edHelp = mGUI->findWidget<Edit>("QuickHelpText");
	String s = TR("#{QuickHelpText}");
	s = StringUtil::replaceAll(s, "@", "\n");
	edHelp->setCaption(s);


	///  input tab  -------
	InitInputGui();
	panInputDetail = mGUI->findWidget<Widget>("PanInputDetail");
	
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
			if (boost::filesystem::exists(PATHMANAGER::GetCarPath() + "/" + *i + "/about.txt"))  {
				carList->addItem(*i);
				if (*i == pSet->gui.car[0]) {  carList->setIndexSelected(ii);  bFound = true;  }
				ii++;  }
		}
		if (!bFound)
			pSet->gui.car[0] = *li.begin();
		carList->eventListChangePosition += newDelegate(this, &App::listCarChng);
    }

	//  cars text, chg btn
    valCar = mGUI->findWidget<StaticText>("CarText");
	valCar->setCaption(TR("#{Car}: ") + pSet->gui.car[0]);  sListCar = pSet->gui.car[0];

    ButtonPtr btnCar = mGUI->findWidget<Button>("ChangeCar");
    if (btnCar)  btnCar->eventMouseButtonClick += newDelegate(this, &App::btnChgCar);

    imgCar = mGUI->findWidget<StaticImage>("CarImg");
    listCarChng(carList,0);


    ///  tracks list, text, chg btn
    //------------------------------------------------------------------------

	//  track text, chg btn
	trkDesc[0] = mGUI->findWidget<Edit>("TrackDesc");
    valTrk[0] = mGUI->findWidget<StaticText>("TrackText");
    if (valTrk[0])
		valTrk[0]->setCaption(TR("#{Track}: " + pSet->gui.track));  sListTrack = pSet->gui.track;

    GuiInitTrack();

	//if (!panelNetTrack)
	{
		TabItem* trkTab = mGUI->findWidget<TabItem>("TabTrack");
		trkTab->setColour(Colour(0.8f,0.96f,1.f));
		const IntCoord& tc = trkTab->getCoord();

		panelNetTrack = trkTab->createWidget<Widget>(
			"PanelSkin", 0,0,tc.width*0.66f,tc.height, Align::Default/*, "Popup", "panelNetTrack"*/);
		panelNetTrack->setColour(Colour(0.8f,0.96f,1.f));
		panelNetTrack->setAlpha(0.8f);
		panelNetTrack->setVisible(false);
		//<UserString key="RelativeTo" value="OptionsWnd"/>
	}

    //  new game
    for (int i=1; i<=3; ++i)
    {	ButtonPtr btnNewG = mGUI->findWidget<Button>("NewGame"+toStr(i));
		if (btnNewG)  btnNewG->eventMouseButtonClick += newDelegate(this, &App::btnNewGame);
	}
	

	//  championships
	//------------------------------------------------------------------------
	//  track stats 2nd set
	trkDesc[1] = mGUI->findWidget<Edit>("TrackDesc2");
    valTrk[1] = mGUI->findWidget<StaticText>("TrackText2");
	//  preview images
	imgPrv[1] = mGUI->findWidget<StaticImage>("TrackImg2");
	imgTer[1] = mGUI->findWidget<StaticImage>("TrkTerImg2");
	imgMini[1] = mGUI->findWidget<StaticImage>("TrackMap2");
	//  stats text
	for (int i=0; i < StTrk; ++i)
		stTrk[1][i] = mGUI->findWidget<StaticText>("2iv"+toStr(i+1), false);
	for (int i=0; i < InfTrk; ++i)
		infTrk[1][i] = mGUI->findWidget<StaticText>("2ti"+toStr(i+1), false);


	//  champs list
	MyGUI::MultiList2* li;
	TabItem* trktab = (TabItem*)mWndGame->findWidget("TabChamps");
	li = trktab->createWidget<MultiList2>("MultiListBox",0,0,400,300, Align::Left | Align::VStretch);
	li->eventListChangePosition += newDelegate(this, &App::listChampChng);
   	li->setVisible(false);
	
	li->removeAllColumns();  c=0;
	li->addColumn("N", ChColW[c++]);
	li->addColumn(TR("#{Name}"), ChColW[c++]);
	li->addColumn(TR("#{Difficulty}"), ChColW[c++]);
	li->addColumn(TR("#{Stages}"), ChColW[c++]);
	li->addColumn(TR("#{Progress}"), ChColW[c++]);
	li->addColumn(TR("#{Score}"), ChColW[c++]);
	li->addColumn(" ", ChColW[c++]);
	liChamps = li;

	//  stages list
	trktab = (TabItem*)mWndGame->findWidget("TabStages");
	li = trktab->createWidget<MultiList2>("MultiListBox",0,0,400,300, Align::Left | Align::VStretch);
	li->eventListChangePosition += newDelegate(this, &App::listStageChng);
   	li->setVisible(false);
	
	li->removeAllColumns();  c=0;
	li->addColumn("N", StColW[c++]);
	li->addColumn(TR("#{Track}"), StColW[c++]);
	li->addColumn(TR("#{Scenery}"), StColW[c++]);
	li->addColumn(TR("#{Difficulty}"), StColW[c++]);
	li->addColumn(TR("#{Time}"), StColW[c++]);
	li->addColumn(TR("#{Score}"), StColW[c++]);
	li->addColumn(" ", StColW[c++]);
	liStages = li;

	updChampListDim();
	ChampsListUpdate();
	listChampChng(liChamps, liChamps->getIndexSelected());


	Btn("btnChampStart", btnChampStart);
	Btn("btnChampStageBack", btnChampStageBack);
	Btn("btnChampStageStart", btnChampStageStart);
	Btn("btnChampEndClose", btnChampEndClose);

	edChampStage = (EditBox*)mWndChampStage->findWidget("ChampStageText");
	edChampEnd = (EditBox*)mWndChampEnd->findWidget("ChampEndText");
	imgChampStage = (ImageBox*)mWndChampStage->findWidget("ChampStageImg");


	//  netw end list
	Btn("btnNetEndClose", btnNetEndClose);
	li = mWndNetEnd->createWidget<MultiList2>("MultiListBox",4,42,632,360, Align::Left | Align::VStretch);
	li->removeAllColumns();
	li->addColumn("", 40);  //N
	li->addColumn(TR("#{TBPlace}"), 60);
	li->addColumn(TR("#{NetNickname}"), 180);
	li->addColumn(TR("#{TBTime}"), 120);
	li->addColumn(TR("#{TBBest}"), 120);
	li->addColumn(TR("#{TBLap}"), 60);
	liNetEnd = li;
	

	bGI = true;  // gui inited, gui events can now save vals
	
	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Init Gui: ") + toStr(dt) + " ms");
}


//  utility
//---------------------------------------------------------------------------------------------------------------------

void App::UpdCarClrSld(bool upd)
{
	Slider* sl;
	Slv(CarClrH, pSet->gui.car_hue[iCurCar]);
	Slv(CarClrS, pSet->gui.car_sat[iCurCar]);
	Slv(CarClrV, pSet->gui.car_val[iCurCar]);
	pSet->game.car_hue[iCurCar] = pSet->gui.car_hue[iCurCar];  // copy to apply
	pSet->game.car_sat[iCurCar] = pSet->gui.car_sat[iCurCar];
	pSet->game.car_val[iCurCar] = pSet->gui.car_val[iCurCar];
	bUpdCarClr = true;
}


//  next/prev in list by key
int App::LNext(MyGUI::MultiList2* lp, int rel)
{
	int i = std::max(0, std::min((int)lp->getItemCount()-1, (int)lp->getIndexSelected()+rel ));
	lp->setIndexSelected(i);
	lp->beginToItemAt(std::max(0, i-11));  // center
	return i;
}
int App::LNext(MyGUI::MultiList* lp, int rel)
{
	int i = std::max(0, std::min((int)lp->getItemCount()-1, (int)lp->getIndexSelected()+rel ));
	lp->setIndexSelected(i);
	//lp->beginToItemAt(std::max(0, i-11));  // center
	return i;
}
int App::LNext(MyGUI::ListPtr lp, int rel)
{
	int i = std::max(0, std::min((int)lp->getItemCount()-1, (int)lp->getIndexSelected()+rel ));
	lp->setIndexSelected(i);
	lp->beginToItemAt(std::max(0, i-11));  // center
	return i;
}

void App::LNext(int rel)
{
	//if (!isFocGui || pSet->isMain)  return;
	switch (pSet->inMenu)
	{
	case WND_Game: case WND_Champ:
		switch (mWndTabsGame->getIndexSelected())	{
			case 1:  listTrackChng(trkMList,LNext(trkMList, rel));  return;
			case 2:	 listCarChng(carList,   LNext(carList, rel));  return;
			case 5:  listChampChng(liChamps,LNext(liChamps, rel));  return;
			case 6:	 listStageChng(liStages, LNext(liStages, rel));  return;	}
		break;
	case WND_Replays:
		listRplChng(rplList,  LNext(rplList, rel));
		break;
	}
}
