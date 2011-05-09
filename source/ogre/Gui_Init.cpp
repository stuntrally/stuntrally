#include "stdafx.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "OgreGame.h"
#include "../oisb/OISB.h"
using namespace MyGUI;


///  Gui Init
//-------------------------------------------------------------------------------------
#define res  1000000.f
#define Fmt  sprintf


void GameInfoListener::listChanged(protocol::GameList list) {
	if (!mList) return;
	mList->removeAllItems();
	for (protocol::GameList::const_iterator it = list.begin(); it != list.end(); ++it) {
		mList->addItem(it->second.name);
		int l = mList->getItemCount()-1;
		mList->setSubItemNameAt(1, l, std::string(it->second.track));
		mList->setSubItemNameAt(2, l, boost::lexical_cast<std::string>((int)it->second.players));
		mList->setSubItemNameAt(3, l, net::IPv4(it->second.address));
		mList->setSubItemNameAt(4, l, boost::lexical_cast<std::string>((int)it->second.port));
	}
}



void App::InitGui()
{
	//  change skin
	if (!mGUI)  return;
	LanguageManager::getInstance().loadUserTags("core_theme_black_blue_tag.xml");
	mGUI->load("core_skin.xml");

	//  load Options layout
	VectorWidgetPtr rootV = LayoutManager::getInstance().load("Options.layout");
	mLayout = rootV.at(0);

	//  window
	mWndOpts = mLayout->findWidget("OptionsWnd");
	if (mWndOpts)  {
		mWndOpts->setVisible(isFocGui);
		int sx = mWindow->getWidth(), sy = mWindow->getHeight();
		IntSize w = mWndOpts->getSize();  // center
		mWndOpts->setPosition((sx-w.width)*0.5f, (sy-w.height)*0.5f);  }
	mGUI->setVisiblePointer(isFocGui);
	mWndTabs = (TabPtr)mLayout->findWidget("TabWnd");

	mWndRpl = mGUI->findWidget<Window>("RplWnd",false);
	if (mWndRpl)  mWndRpl->setVisible(false);


	//  tooltip  ------
	for (VectorWidgetPtr::iterator it = rootV.begin(); it != rootV.end(); ++it)
	{
		setToolTips((*it)->getEnumerator());
		const std::string& name = (*it)->getName();
		//if (name == "OptionsWnd")  mWndOpts = *it;
	}
	mToolTip = Gui::getInstance().findWidget<Widget>("ToolTip");
	mToolTip->setVisible(false);
	mToolTipTxt = mToolTip->getChildAt(0)->castType<Edit>();
	
		
	//  center mouse pos
	int xm = mWindow->getWidth()/2, ym = mWindow->getHeight()/2;
	MyGUI::InputManager::getInstance().injectMouseMove(xm, ym, 0);
	OIS::MouseState &ms = const_cast<OIS::MouseState&>(mMouse->getMouseState());
	ms.X.abs = xm;  ms.Y.abs = ym;


	//  assign controls

	///  Sliders
    //------------------------------------------------------------------------
	HScrollPtr sl;  ComboBoxPtr combo;  size_t v;  ButtonPtr btn;

	// get slider, assign event, get valtext, set value from settings
	#define Slv(name, vset)  \
		sl = (HScrollPtr)mLayout->findWidget(#name);  \
		if (sl)  sl->eventScrollChangePosition = newDelegate(this, &App::sl##name);  \
		val##name = (StaticTextPtr)(mLayout->findWidget(#name"Val"));  \
		v = vset*res;  if (sl)  sl->setScrollPosition(v);	sl##name(sl, v);

	#define Btn(name, event)   btn = mGUI->findWidget<Button>(name);  \
		if (btn)  btn->eventMouseButtonClick = newDelegate(this, &App::event);

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
    Btn("Apply", btnShadows);
    
    //  sound
	Slv(VolMaster,	pSet->vol_master/1.6f);	 Slv(VolEngine,	pSet->vol_engine/1.4f);
	Slv(VolTires,	pSet->vol_tires/1.4f); 	 Slv(VolEnv,	pSet->vol_env/1.4f);
	
	// car color
	Slv(CarClrH, pSet->car_hue);
	Slv(CarClrS, (pSet->car_sat +1)*0.5f);  Slv(CarClrV, (pSet->car_val +1)*0.5f);


	///  Checkboxes
    //------------------------------------------------------------------------
	ButtonPtr bchk;
	#define Chk(name, event, var)  \
		bchk = mGUI->findWidget<Button>(name);  \
		if (bchk)  {  bchk->eventMouseButtonClick = newDelegate(this, &App::event);  \
			bchk->setStateCheck(pSet->var);  }

	bnQuit = mGUI->findWidget<Button>("Quit");
	if (bnQuit)  {  bnQuit->eventMouseButtonClick = newDelegate(this, &App::btnQuit);  bnQuit->setVisible(isFocGui);  }
	Chk("ReverseOn", chkReverse, trackreverse);
	Chk("ParticlesOn", chkParticles, particles);	Chk("TrailsOn", chkTrails, trails);

	Chk("Fps", chkFps, show_fps);	chFps = mGUI->findWidget<Button>("Fps");
	if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();

	Chk("Digits", chkDigits, show_digits);
	Chk("Gauges", chkGauges, show_gauges);  ShowHUD();//

	Chk("Minimap", chkMinimap, trackmap);	chMinimp = mGUI->findWidget<Button>("Minimap");
	Chk("Times", chkTimes, show_times);		chTimes  = mGUI->findWidget<Button>("Times");
	//Chk("Racingline", chkRacingLine, racingline);
	Chk("CamInfo", chkCamInfo, show_cam);

	Chk("CarDbgBars", chkCarDbgBars, car_dbgbars);	chDbgB = mGUI->findWidget<Button>("CarDbgBars");
	Chk("CarDbgTxt", chkCarDbgTxt, car_dbgtxt);		chDbgT = mGUI->findWidget<Button>("CarDbgTxt");
	Chk("BulletDebug", chkBltDebug, bltDebug);	chBlt = mGUI->findWidget<Button>("BulletDebug");
	Chk("BulletProfilerTxt", chkBltProfilerTxt, bltProfilerTxt);	chBltTxt = mGUI->findWidget<Button>("BulletDebug");
	
	//  abs, tcs
	Chk("CarABS",  chkAbs, abs);		Chk("CarTCS", chkTcs, tcs);
	Chk("CarGear", chkGear, autoshift);
	Chk("CarRear", chkRear, autorear);	Chk("CarClutch", chkClutch, autoclutch);
	Chk("VegetCollis", chkVegetCollis, veget_collis);
	
	//  kmh/mph radio
	bRkmh = mGUI->findWidget<Button>("kmh");
	bRmph = mGUI->findWidget<Button>("mph");
	if (bRkmh && bRmph)  {  bRkmh->setStateCheck(!pSet->show_mph);  bRmph->setStateCheck(pSet->show_mph);
		bRkmh->eventMouseButtonClick = newDelegate(this, &App::radKmh);
		bRmph->eventMouseButtonClick = newDelegate(this, &App::radMph);  }

	Btn("TrGrReset", btnTrGrReset);

	//  startup
	Chk("OgreDialog", chkOgreDialog, ogre_dialog);
	Chk("AutoStart", chkAutoStart, autostart);
	Chk("EscQuits", chkEscQuits, escquit);
	Chk("BltLines", chkBltLines, bltLines);
	Chk("ShowPictures", chkLoadPics, loadingbackground);
	
	//  compositor, video
	Chk("Bloom", chkVidBloom, bloom);
	Chk("HDR", chkVidHDR, hdr);
	Chk("MotionBlur", chkVidBlur, motionblur);

	Slv(BloomInt,	pSet->bloomintensity);
	Slv(BloomOrig,	pSet->bloomorig);
	Slv(BlurIntens, pSet->motionblurintensity);
	
	Chk("FullScreen", chkVidFullscr, fullscreen);
	Chk("VSync", chkVidVSync, vsync);

	//button_ramp, speed_sens..
	

	//  replays  ------------------------------------------------------------
	Btn("RplLoad", btnRplLoad);  Btn("RplSave", btnRplSave);  Btn("RplDelete", btnRplDelete);
	Chk("RplChkAutoRec", chkRplAutoRec, rpl_rec);
	//Chk("RplChkGhost", chkRplChkGhost, rpl_play);
	Btn("RplBtnCur", btnRplCur)  Btn("RplBtnAll", btnRplAll);  // radio
    if (mWndRpl)
	{	//  replay controls
		Btn("RplToStart", btnRplToStart);  Btn("RplToEnd", btnRplToEnd)
		Btn("RplBack", btnRplBack);  Btn("RplForward", btnRplForward);
		Btn("RplPlay", btnRplPlay);  btRplPl = btn;

		//  info
		slRplPos = (HScrollPtr)mWndRpl->findWidget("RplSlider");
		if (slRplPos)  slRplPos->eventScrollChangePosition = newDelegate(this, &App::slRplPosEv);

		valRplPerc = (StaticTextPtr)mWndRpl->findWidget("RplPercent");
    	valRplCur = (StaticTextPtr)mWndRpl->findWidget("RplTimeCur");
    	valRplLen = (StaticTextPtr)mWndRpl->findWidget("RplTimeLen");
	}
	//  text desc
	valRplName = mGUI->findWidget<StaticText>("RplName");  valRplName2 = mGUI->findWidget<StaticText>("RplName2");
	valRplInfo = mGUI->findWidget<StaticText>("RplInfo");  valRplInfo2 = mGUI->findWidget<StaticText>("RplInfo2");
	edRplName = mGUI->findWidget<Edit>("RplNameEdit");
	edRplDesc = mGUI->findWidget<Edit>("RplDesc");

	rplList = mGUI->findWidget<List>("RplList");
	if (rplList)  rplList->eventListChangePosition = newDelegate(this, &App::listRplChng);
	updReplaysList();
	
	Btn("btnPlayers1", btnNumPlayers);
	Btn("btnPlayers2", btnNumPlayers);
	Btn("btnPlayers3", btnNumPlayers);
	Btn("btnPlayers4", btnNumPlayers);
	Chk("chkSplitVertically", chkSplitVert, split_vertically);


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
		listServers->addColumn("Track", 160);
		listServers->addColumn("Players", 80);
		listServers->addColumn("Host", 120);
		listServers->addColumn("Port", 100);
	}
	gameInfoListener.reset(new GameInfoListener(listServers));
	Btn("btnNetRefresh", evBtnNetRefresh);  btnNetRefresh = btn;
	Btn("btnNetJoin", evBtnNetJoin);  btnNetJoin = btn;
	Btn("btnNetCreate", evBtnNetCreate);  btnNetCreate = btn;

	//  game, players
	valNetGameName = mGUI->findWidget<StaticText>("valNetGameName");
	edNetGameName = mGUI->findWidget<Edit>("edNetGameName");
	
	listPlayers = mGUI->findWidget<MultiList>("MListPlayers");
	if (listPlayers)
	{	listPlayers->addColumn("Player", 140);
		listPlayers->addColumn("Car", 70);
		listPlayers->addColumn("Ping", 80);
		listPlayers->addColumn("Ready", 70);
	}
	Btn("btnNetReady", evBtnNetReady);  btnNetReady = btn;
	Btn("btnNetLeave", evBtnNetLeave);	btnNetLeave = btn;
	btnNetLeave->setCaption(getCreateGameButtonCaption());

    //  chat
    valNetChat = mGUI->findWidget<StaticText>("valNetChat");
    edNetChat = mGUI->findWidget<Edit>("edNetChat");  // chat area
    edNetChatMsg = mGUI->findWidget<Edit>("edNetChatMsg");  // user text
    Btn("btnNetSendMsg", evBtnNetSendMsg);  btnNetSendMsg = btn;
    
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
		edNetNick->eventEditTextChange = newDelegate(this, &App::evEdNetNick);	}
	if (edNetServerIP)	{	edNetServerIP->setCaption(pSet->master_server_address);
		edNetServerIP->eventEditTextChange = newDelegate(this, &App::evEdNetServerIP);	}
	if (edNetServerPort){	edNetServerPort->setCaption(toStr(pSet->master_server_port));
		edNetServerPort->eventEditTextChange = newDelegate(this, &App::evEdNetServerPort);	}
	if (edNetLocalPort)	{	edNetLocalPort->setCaption(toStr(pSet->local_port));
		edNetLocalPort->eventEditTextChange = newDelegate(this, &App::evEdNetLocalPort);	}


	///  input tab  -------
	InitInputGui();
	
	
	///  video resolutions combobox
    //------------------------------------------------------------------------
	resList = mGUI->findWidget<List>("ResList");
	if (resList)
	{
		//  fill video resolution list
		const StringVector& videoModes = Root::getSingleton().getRenderSystem()->getConfigOptions()["Video Mode"].possibleValues;
		String modeSel = "";
		for (int i=0; i < videoModes.size(); i++)
		{
			String mode = videoModes[i];
			StringUtil::trim(mode);
			if (StringUtil::match(mode, "*16-bit*"))  continue;  //skip ?DX

			StringVector vmopts = StringUtil::split(mode, " x");  // only resolution
			int w = StringConverter::parseUnsignedInt(vmopts[0]);
			int h = StringConverter::parseUnsignedInt(vmopts[1]);
			if (w >= 800 && h >= 600)  // min res
			{
				mode = toStr(w) + " x " + toStr(h);
				resList->addItem(mode);
				int ww = w - mWindow->getWidth(), hh = h - mWindow->getHeight();
				if (abs(ww) < 30 && abs(hh) < 50)
					modeSel = mode;
			}
		}
		// todo.. sort w,h asc.
		//  sel current mode
		if (modeSel != "")
			resList->setIndexSelected(resList->findItemIndexWith(modeSel));
	}
	ButtonPtr btnRes = mGUI->findWidget<Button>("ResChange");
	if (btnRes)  {  btnRes->eventMouseButtonClick = newDelegate(this, &App::btnResChng);  }
	
	
	///  cars list
    //------------------------------------------------------------------------
    carList = (ListPtr)mLayout->findWidget("CarList");
    if (carList)
    {	carList->removeAllItems();  int ii = 0;  bool bFound = false;

		strlist li;
		PATHMANAGER::GetFolderIndex(PATHMANAGER::GetCarPath(), li);
		for (strlist::iterator i = li.begin(); i != li.end(); ++i)
		{
			ifstream check((PATHMANAGER::GetCarPath() + "/" + *i + "/about.txt").c_str());
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
	valCar->setCaption(TR("#{Car}: ") + pSet->car);  sListCar = pSet->car;

    ButtonPtr btnCar = (ButtonPtr)mLayout->findWidget("ChangeCar");
    if (btnCar)  btnCar->eventMouseButtonClick = newDelegate(this, &App::btnChgCar);


    ///  tracks list, text, chg btn
    //------------------------------------------------------------------------
    trkList = (ListPtr)mLayout->findWidget("TrackList");
    if (trkList)
    {	trkList->removeAllItems();
		int ii = 0, si = 0;  bool bFound = false;

		strlist li, lu;
		PATHMANAGER::GetFolderIndex(pathTrk[0], li);
		PATHMANAGER::GetFolderIndex(pathTrk[1], lu);
		//  original
		for (strlist::iterator i = li.begin(); i != li.end(); ++i)
		{

			string s = pathTrk[0] + *i + "/track.txt";
			ifstream check(s.c_str());
			if (check)  {
				trkList->addItem(*i, 0);
				if (!pSet->track_user && *i == pSet->track)  {  si = ii;
					trkList->setIndexSelected(si);
					bFound = true;  bListTrackU = 0;  }
				ii++;  }
		}
		//  user
		for (strlist::iterator i = lu.begin(); i != lu.end(); ++i)
		{

			string s = pathTrk[1] + *i + "/track.txt";
			ifstream check(s.c_str());
			if (check)  {
				trkList->addItem("*" + (*i) + "*", 1);
				if (pSet->track_user && *i == pSet->track)  {  si = ii;
					trkList->setIndexSelected(si);
					bFound = true;  bListTrackU = 1;  }
				ii++;  }
		}
		//  not found last track, set 1st
		if (!bFound)
		{	pSet->track = *li.begin();  pSet->track_user = 0;  }
		trkList->beginToItemAt(max(0, si-11));  // center
		trkList->eventListChangePosition = newDelegate(this, &App::listTrackChng);
		//?trkList->eventMouseButtonDoubleClick = newDelegate(this, &App::btnNewGameStart);
    }
    //------------------------------------------------------------------------

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


///  . .  util tracks stats  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

void App::ReadTrkStats()
{
	String sRd = PathListTrk() + "/road.xml";
	String sSc = PathListTrk() + "/scene.xml";

	SplineRoad rd(pGame);  rd.LoadFile(sRd,false);  // load
	Scene sc;  sc.LoadXml(sSc);  // fails to defaults
	TIMER tim;  tim.Load(PATHMANAGER::GetTrackRecordsPath()+"/"+sListTrack+".txt", 0.f, pGame->error_output);
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


///  Gui ToolTips
//-----------------------------------------------------------------------------------------------------------

void App::setToolTips(EnumeratorWidgetPtr widgets)
{
    while (widgets.next())
    {
        WidgetPtr wp = widgets.current();
        bool tip = wp->isUserString("tip");
		if (tip)  // if has tooltip string
		{	
			// needed for translation
			wp->setUserString("tip", LanguageManager::getInstance().replaceTags(wp->getUserString("tip")));
			wp->setNeedToolTip(true);
			wp->eventToolTip = newDelegate(this, &App::notifyToolTip);
		}
		//Log(wp->getName() + (tip ? "  *" : ""));
        setToolTips(wp->getEnumerator());
    }
}

void App::notifyToolTip(Widget *sender, const ToolTipInfo &info)
{
	if (!mToolTip)  return;

	if (!isFocGui)
	{	mToolTip->setVisible(false);
		return;  }

	if (info.type == ToolTipInfo::Show)
	{
		mToolTip->setSize(320, 96);  // start size for wrap
		mToolTipTxt->setCaption(sender->getUserString("tip"));
		const IntSize &textsize = mToolTipTxt->getTextSize();
		mToolTip->setSize(textsize.width +  6, textsize.height + 6);
		mToolTip->setVisible(true);
		boundedMove(mToolTip, info.point);
	}
	else if (info.type == ToolTipInfo::Hide)
		mToolTip->setVisible(false);
}

//  Move a widget to a point while making it stay in the viewport.
void App::boundedMove(Widget* moving, const IntPoint& point)
{
	const IntPoint offset(20, 20);  // mouse cursor
	IntPoint p = point + offset;

	const IntSize& size = moving->getSize();
	
	unsigned int vpw = mWindow->getWidth();
	unsigned int vph = mWindow->getHeight();
	
	if (p.left + size.width > vpw)
		p.left = vpw - size.width;
	if (p.top + size.height > vph)
		p.top = vph - size.height;
			
	moving->setPosition(p);
}


//  next/prev track in list
void App::trkListNext(int rel)
{
	if (!(isFocGui && mWndTabs->getIndexSelected() == 0))  return;
	int i = max(0, min((int)trkList->getItemCount()-1, (int)trkList->getIndexSelected()+rel ));
	trkList->setIndexSelected(i);
	trkList->beginToItemAt(max(0, i-11));  // center
	listTrackChng(trkList,i);
}

void App::carListNext(int rel)
{
	if (!(isFocGui && mWndTabs->getIndexSelected() == 1))  return;
	int i = max(0, min((int)carList->getItemCount()-1, (int)carList->getIndexSelected()+rel ));
	carList->setIndexSelected(i);
	carList->beginToItemAt(max(0, i-11));  // center
	listCarChng(carList,i);
}


///  input tab
//----------------------------------------------------------------------------------------------------------------------------------
void App::InitInputGui()
{
	//  Log all devices (and keys) to log.txt
	pGame->info_output << " --------------------------------------  Input devices  BEGIN" << std::endl;
	//OISB::System::getSingleton().dumpActionSchemas(pGame->info_output);
	OISB::System::getSingleton().dumpDevices(pGame->info_output);
	pGame->info_output << " --------------------------------------  Input devices  END" << std::endl;

	MyGUI::TabPtr inputTab = mGUI->findWidget<Tab>("InputTab");
	if (!inputTab)  return;

	//  insert a tab item for every schema (4players,global)
	std::map<OISB::String, OISB::ActionSchema*> schemas = OISB::System::getSingleton().mActionSchemas;
	for (std::map<OISB::String, OISB::ActionSchema*>::const_iterator it = schemas.begin(); it != schemas.end(); it++)
	{
		MyGUI::TabItemPtr tabitem = inputTab->addItem( TR("#{InputMap" + (*it).first + "}") );
		
		//-scroll view.... doesnt work???
		//MyGUI::ScrollViewPtr sv = tabitem->createWidget<ScrollView>("ScrollView", 0, 0, 700, 580, MyGUI::Align::Default, "scrollView_" + (*it).first );
		
		///  Headers
		MyGUI::StaticTextPtr headkb = tabitem->createWidget<StaticText>(
			"StaticText", 220, 10, 200, 24, MyGUI::Align::Default, "staticText_" + (*it).first );
		headkb->setCaption(TR("#88AAFF#{InputKey1}"));
		MyGUI::StaticTextPtr headkb2 = tabitem->createWidget<StaticText>(
			"StaticText", 360, 10, 200, 24, MyGUI::Align::Default, "staticText_" + (*it).first );
		headkb2->setCaption(TR("#88AAFF#{InputKey2}"));
		
		//  custom y pos of each row, to sort by function
		int i = 0, y = 0;
		std::map <std::string, int> yRow;
		yRow["Throttle"] = y;	y+=2;
		yRow["Brake"] = y;		y+=2;
		yRow["Steering"] = y;	y+=2+1;
		yRow["HandBrake"] = y;	y+=2;
		yRow["Boost"] = y;		y+=2;
		yRow["Flip"] = y;		y+=2+1;
		yRow["ShiftDown"] = y;	y+=2;
		yRow["ShiftUp"] = y;	y+=2;
		
		///  Actions
		for (std::map<OISB::String, OISB::Action*>::const_iterator
			ait = (*it).second->mActions.begin();
			ait != (*it).second->mActions.end(); ait++,i++)
		{
			// macro to strip away the Keyboard/
			#define stripk(s) Ogre::StringUtil::split(s, "/")[1]
			
			OISB::Action* act = (*ait).second;
			const int sx = 130, sy = 24, x0 = 20, x1 = 180, x2 = 320;  // button size and columns positon

			// description label
			const String& name = (*ait).second->getName();
			y = 40 + 26 * yRow[name] / 2;
			MyGUI::StaticTextPtr desc = tabitem->createWidget<StaticText>("StaticText", x0, y, sx+70, sy, MyGUI::Align::Default, "staticText_" + (*ait).first );
			desc->setCaption( TR("#{InputMap" + name + "}") );
			
			// bound key(s)
			if (act->getActionType() == OISB::AT_TRIGGER)
			{
				if (act->mBindings.size() > 0 && act->mBindings.front()->getNumBindables() > 0)
				{
					// first key
					MyGUI::ButtonPtr key1 = tabitem->createWidget<Button>("Button", x1, y, sx, sy, MyGUI::Align::Default, "inputbutton_" + (*ait).first + "_" + (*it).first + "_1" );
					key1->setCaption( stripk(act->mBindings.front()->getBindable(0)->getBindableName()) );
					key1->eventMouseButtonClick = MyGUI::newDelegate(this, &App::controlBtnClicked);
					// alternate key
					MyGUI::ButtonPtr key2 = tabitem->createWidget<Button>("Button", x2, y, sx, sy, MyGUI::Align::Default, "inputbutton_" + (*ait).first + "_" + (*it).first + "_2");
					if (act->mBindings.front()->getNumBindables() > 1)
						key2->setCaption( stripk(act->mBindings.front()->getBindable(1)->getBindableName()) );
					else
						key2->setCaption( TR("#{InputKeyUnassigned}"));
					key2->eventMouseButtonClick = MyGUI::newDelegate(this, &App::controlBtnClicked);
				}
			}
			else if (act->getActionType() == OISB::AT_ANALOG_AXIS)
			{
				if (act->mBindings.size() > 0 && act->mBindings.front()->getNumBindables() > 0)
				{
					// look for increase/decrease binds
					OISB::Bindable* increase = NULL, *decrease = NULL;
					for (std::vector<std::pair<String, OISB::Bindable*> >::const_iterator
						bnit = act->mBindings.front()->mBindables.begin();
						bnit != act->mBindings.front()->mBindables.end(); bnit++)
					{
						if ((*bnit).first == "increase")		increase = (*bnit).second;
						else if ((*bnit).first == "decrease")	decrease = (*bnit).second;
					}
					if (increase)
					{
						MyGUI::ButtonPtr key1 = tabitem->createWidget<Button>("Button", x1, y, sx, sy, MyGUI::Align::Default,
							"inputbutton_" + (*ait).first + "_" + (*it).first + "_1");
						key1->setCaption( stripk(increase->getBindableName()) );
						key1->eventMouseButtonClick = MyGUI::newDelegate(this, &App::controlBtnClicked);
					}
					if (decrease)
					{
						MyGUI::ButtonPtr key2 = tabitem->createWidget<Button>("Button", x2, y, sx, sy, MyGUI::Align::Default,
							"inputbutton_" + (*ait).first + "_" + (*it).first + "_2");
						key2->setCaption( stripk(decrease->getBindableName()) );
						key2->eventMouseButtonClick = MyGUI::newDelegate(this, &App::controlBtnClicked);
					}
					else
					{
						MyGUI::ButtonPtr key2 = tabitem->createWidget<Button>("Button", x2, y, sx, sy, MyGUI::Align::Default,
							"inputbutton_" + (*ait).first + "_" + (*it).first + "_2");
						key2->setCaption( TR("#{InputKeyUnassigned}"));
						key2->eventMouseButtonClick = MyGUI::newDelegate(this, &App::controlBtnClicked);
					}
				}
			}
			///-AT_SEQUENCE: not used
		}
		
		// joystick selection menu
		MyGUI::ComboBoxPtr joysticks = tabitem->createWidget<ComboBox>("ComboBox", 540, 10, 150, 24, MyGUI::Align::Default, "joystickSel_" + (*it).first );
		joysticks->addItem(TR("#{InputNoJS}"));
		joysticks->setIndexSelected(0);
		for (std::vector<OISB::JoyStick*>::const_iterator it=OISB::System::getSingleton().mJoysticks.begin();
				it!=OISB::System::getSingleton().mJoysticks.end();
				it++)
		{
			joysticks->addItem( (*it)->getName() );
		}
				
	}
}



/// Misc

String App::getCreateGameButtonCaption() const
{
	// FIXME: i18n
	switch (mLobbyState) {
	case DISCONNECTED:
		return "Create game";
	case HOSTING:
	case JOINED:
		return "Leave game";
	}
	return "This should not display";
}
