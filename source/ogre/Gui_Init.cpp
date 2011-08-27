#include "pch.h"
#include "Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "OgreGame.h"
//#include "../oisb/OISB.h"

#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreOverlay.h>
#include "common/Gui_Def.h"
using namespace MyGUI;
using namespace Ogre;


///  Gui Init
//-------------------------------------------------------------------------------------

void App::InitGui()
{
	//  change skin
	if (!mGUI)  return;
	QTimer ti;  ti.update();  /// time
	LanguageManager::getInstance().loadUserTags("core_theme_black_blue_tag.xml");
	//mGUI->load("core_skin.xml");

	//  load Options layout
	vwGui = LayoutManager::getInstance().load("Options.layout");
	mLayout = vwGui.at(0);

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
	for (VectorWidgetPtr::iterator it = vwGui.begin(); it != vwGui.end(); ++it)
	{
		setToolTips((*it)->getEnumerator());
		//const std::string& name = (*it)->getName();
	}
	GuiInitTooltip();
		
	GuiCenterMouse();


	//  assign controls

	///  Sliders
    //------------------------------------------------------------------------
	ButtonPtr btn,bchk;  ComboBoxPtr combo;
	HScrollPtr sl;  size_t v;

	//  detail
	Slv(TerDetail,	powf(pSet->terdetail /20.f, 0.5f));
	Slv(TerDist,	powf(pSet->terdist /1000.f, 0.5f));
	Slv(ViewDist,	powf((pSet->view_distance -50.f)/6950.f, 0.5f));
	Slv(RoadDist,	powf(pSet->road_dist /4.f, 0.5f));

	//  textures
	combo = (ComboBoxPtr)mLayout->findWidget("TexFiltering");
	if (combo)  combo->eventComboChangePosition = newDelegate(this, &App::comboTexFilter);
	
	GuiInitLang();
	
	Slv(Anisotropy,	pSet->anisotropy /res);
	Slv(Shaders,	pSet->shaders /res);
	Slv(TexSize,	pSet->tex_size /res);
	Slv(TerMtr,		pSet->ter_mtr /res);
	
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
	Slv(SizeMinimap,(pSet->size_minimap-0.05f) /0.25f);
	Slv(ZoomMinimap,powf((pSet->zoom_minimap-1.0f) /9.f, 0.5f));
	
	//  reflect
	Slv(ReflSkip,	powf(pSet->refl_skip /1000.f, 0.5f));
	Slv(ReflSize,	pSet->refl_size /res);
	Slv(ReflFaces,	pSet->refl_faces /res);
	Slv(ReflDist,	powf((pSet->refl_dist -20.f)/1480.f, 0.5f));
	int value=0;  if (pSet->refl_mode == "static")  value = 0;
	else if (pSet->refl_mode == "single")  value = 1;
	else if (pSet->refl_mode == "full")  value = 2;
	Slv(ReflMode,   value /res);

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
	UpdCarClrSld();

	///  Checkboxes
    //------------------------------------------------------------------------
	bnQuit = mGUI->findWidget<Button>("Quit");
	if (bnQuit)  {  bnQuit->eventMouseButtonClick = newDelegate(this, &App::btnQuit);  bnQuit->setVisible(isFocGui);  }
	Chk("ReverseOn", chkReverse, pSet->trackreverse);
	Chk("ParticlesOn", chkParticles, pSet->particles);	Chk("TrailsOn", chkTrails, pSet->trails);

	Chk("Fps", chkFps, pSet->show_fps);	chFps = mGUI->findWidget<Button>("Fps");
	if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();

	Chk("Digits", chkDigits, pSet->show_digits);
	Chk("Gauges", chkGauges, pSet->show_gauges);  ShowHUD();//

	Chk("Minimap", chkMinimap, pSet->trackmap);	chMinimp = bchk;
	Chk("MiniZoom", chkMiniZoom, pSet->mini_zoomed);  Chk("MiniRot", chkMiniRot, pSet->mini_rotated);
	Chk("Times", chkTimes, pSet->show_times);	chTimes  = bchk;
	Chk("CamInfo", chkCamInfo, pSet->show_cam);

	Chk("CarDbgBars", chkCarDbgBars, pSet->car_dbgbars);	chDbgB = bchk;
	Chk("CarDbgTxt", chkCarDbgTxt, pSet->car_dbgtxt);		chDbgT = bchk;
	Chk("BulletDebug", chkBltDebug, pSet->bltDebug);	chBlt = bchk;
	Chk("BulletProfilerTxt", chkBltProfilerTxt, pSet->bltProfilerTxt);	chBltTxt = bchk;
	
	//  abs, tcs
	Chk("CarABS",  chkAbs, pSet->abs);		Chk("CarTCS", chkTcs, pSet->tcs);
	Chk("CarGear", chkGear, pSet->autoshift);
	Chk("CarRear", chkRear, pSet->autorear);	Chk("CarClutch", chkClutch, pSet->autoclutch);
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
	if (bRkmh && bRmph)  {  bRkmh->setStateCheck(!pSet->show_mph);  bRmph->setStateCheck(pSet->show_mph);
		bRkmh->eventMouseButtonClick = newDelegate(this, &App::radKmh);
		bRmph->eventMouseButtonClick = newDelegate(this, &App::radMph);  }

	Btn("TrGrReset", btnTrGrReset);

	//  startup
	Chk("OgreDialog", chkOgreDialog, pSet->ogre_dialog);
	Chk("AutoStart", chkAutoStart, pSet->autostart);
	Chk("EscQuits", chkEscQuits, pSet->escquit);
	Chk("BltLines", chkBltLines, pSet->bltLines);
	Chk("ShowPictures", chkLoadPics, pSet->loadingbackground);
	
	//  compositor, video
	Chk("Bloom", chkVidBloom, pSet->bloom);
	Chk("HDR", chkVidHDR, pSet->hdr);
	Chk("MotionBlur", chkVidBlur, pSet->motionblur);

	Slv(BloomInt,	pSet->bloomintensity);
	Slv(BloomOrig,	pSet->bloomorig);
	Slv(BlurIntens, pSet->motionblurintensity);
	
	Chk("FullScreen", chkVidFullscr, pSet->fullscreen);
	Chk("VSync", chkVidVSync, pSet->vsync);

	//button_ramp, speed_sens..
	

	//  replays  ------------------------------------------------------------
	Btn("RplLoad", btnRplLoad);  Btn("RplSave", btnRplSave);
	Btn("RplDelete", btnRplDelete);  Btn("RplRename", btnRplRename);
	//  settings
	Chk("RplChkAutoRec", chkRplAutoRec, pSet->rpl_rec);
	Chk("RplChkGhost", chkRplChkGhost, pSet->rpl_ghost);
	Chk("RplChkBestOnly", chkRplChkBestOnly, pSet->rpl_bestonly);
	//  radios
	Btn("RplBtnAll", btnRplAll);  rbRplAll = btn;
	Btn("RplBtnCur", btnRplCur);  rbRplCur = btn;
	Btn("RplBtnGhosts", btnRplGhosts);  rbRplGhosts = btn;  btn = 0;
	switch (pSet->rpl_listview)  // load from set
	{	case 0: btn = rbRplAll;  break;  case 1: btn = rbRplCur;  break;  case 2: btn = rbRplGhosts;  break;  }
	if (btn)  btn->setStateCheck(true);
	
    if (mWndRpl)
	{	//  replay controls
		Btn("RplToStart", btnRplToStart);  Btn("RplToEnd", btnRplToEnd)
		Btn("RplPlay", btnRplPlay);  btRplPl = btn;
		btn = (ButtonPtr)mWndRpl->findWidget("RplBack");	if (btn)  {		btn->eventMouseButtonPressed = newDelegate(this, &App::btnRplBackDn);  btn->eventMouseButtonReleased = newDelegate(this, &App::btnRplBackUp);  }
		btn = (ButtonPtr)mWndRpl->findWidget("RplForward");  if (btn)  {	btn->eventMouseButtonPressed = newDelegate(this, &App::btnRplFwdDn);  btn->eventMouseButtonReleased = newDelegate(this, &App::btnRplFwdUp);  }
		
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

	//  car color buttons . . . . .
	Real hsv[10][3] = {
		{0.43,-0.1,-0.2},	{0.90, 0.1, 0.1},	{0.00, 0.0,-0.1},	{0.28,-0.35,-0.66},	{0.75, 0.1,-0.1},
		{0.47, 0.1,-0.1},	{0.5,-0.15,0.16},	{0.86, 0.4,-0.0},	{0.8,-0.8,-0.18},	{0.7, 0.1,-0.15}};
	for (int i=0; i<10; i++)
	{
		StaticImagePtr img = (StaticImagePtr)mLayout->findWidget("carClr"+toStr(i));
		Real h = hsv[i][0], s = hsv[i][1], v = hsv[i][2];
		ColourValue c;  c.setHSB(1.f-h, (s+1.f)*0.5f, (v+1.f)*0.8f/**/);
		img->setColour(Colour(c.r,c.g,c.b));
		img->eventMouseButtonClick = newDelegate(this, &App::imgBtnCarClr);
		img->setUserString("s", toStr(s));  img->setUserString("h", toStr(h));
		img->setUserString("v", toStr(v));
	}
	Btn("CarClrRandom", btnCarClrRandom);
	Slv(NumLaps, (pSet->num_laps - 1) / 20.f);
	
	TabPtr tPlr = (TabPtr)mLayout->findWidget("tabPlayer");
	if (tPlr)  tPlr->eventTabChangeSelect = newDelegate(this, &App::tabPlayer);
	
	
	///  input tab
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
			std::ifstream check((PATHMANAGER::GetCarPath() + "/" + *i + "/about.txt").c_str());
			if (check)  {
				carList->addItem(*i);
				if (*i == pSet->car[0]) {  carList->setIndexSelected(ii);  bFound = true;  }
				ii++;  }
		}
		if (!bFound)
			pSet->car[0] = *li.begin();
		carList->eventListChangePosition = newDelegate(this, &App::listCarChng);
    }

	//  cars text, chg btn
    valCar = (StaticTextPtr)mLayout->findWidget("CarText");
	valCar->setCaption(TR("#{Car}: ") + pSet->car[0]);  sListCar = pSet->car[0];

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
			std::string s = pathTrk[0] + *i + "/track.txt";
			std::ifstream check(s.c_str());
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
			std::string s = pathTrk[1] + *i + "/track.txt";
			std::ifstream check(s.c_str());
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
		trkList->beginToItemAt(std::max(0, si-11));  // center
		trkList->eventListChangePosition = newDelegate(this, &App::listTrackChng);
		//?trkList->eventMouseButtonDoubleClick = newDelegate(this, &App::btnNewGameStart);
    }
    //------------------------------------------------------------------------

	//  track text, chg btn
    valTrk = (StaticTextPtr)mLayout->findWidget("TrackText");
    if (valTrk)
		valTrk->setCaption(TR("#{Track}: " + pSet->track));  sListTrack = pSet->track;
	trkDesc = (EditPtr)mLayout->findWidget("TrackDesc");

    imgCar = (StaticImagePtr)mLayout->findWidget("CarImg");
    listCarChng(carList,0);

	//  preview images
    imgPrv = (StaticImagePtr)mLayout->findWidget("TrackImg");
    imgTer = (StaticImagePtr)mLayout->findWidget("TrkTerImg");
    imgMini = (StaticImagePtr)mLayout->findWidget("TrackMap");
	//  track stats
	for (int i=0; i < StTrk; ++i)
		stTrk[i] = (StaticTextPtr)mLayout->findWidget("iv"+toStr(i+1));
    listTrackChng(trkList,0);

    ButtonPtr btnTrk = (ButtonPtr)mLayout->findWidget("ChangeTrack");
    if (btnTrk)  btnTrk->eventMouseButtonClick = newDelegate(this, &App::btnChgTrack);

    //  new game
    for (int i=1; i<=4; ++i)
    {	ButtonPtr btnNewG = (ButtonPtr)mLayout->findWidget("NewGame"+toStr(i));
		if (btnNewG)  btnNewG->eventMouseButtonClick = newDelegate(this, &App::btnNewGame);
	}

	bGI = true;  // gui inited, gui events can now save vals

	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Init Gui: ") + toStr(dt) + " ms");
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

void App::UpdCarClrSld(bool upd)
{
	HScrollPtr sl;  size_t v;
	bUpdCarClr = false;
	Slv(CarClrH, pSet->car_hue[iCurCar]);
	Slv(CarClrS, (pSet->car_sat[iCurCar] +1)*0.5f);  if (upd)  bUpdCarClr = true;
	Slv(CarClrV, (pSet->car_val[iCurCar] +1)*0.5f);  bUpdCarClr = true;
}


///  Gui ToolTips
//-----------------------------------------------------------------------------------------------------------

void App::setToolTips(EnumeratorWidgetPtr widgets)
{
    while (widgets.next())
    {
        WidgetPtr wp = widgets.current();
		wp->setAlign(Align::Relative);
        bool tip = wp->isUserString("tip");
		if (tip)  // if has tooltip string
		{	
			// needed for translation
			wp->setUserString("tip", LanguageManager::getInstance().replaceTags(wp->getUserString("tip")));
			wp->setNeedToolTip(true);
			wp->eventToolTip = newDelegate(this, &App::notifyToolTip);
		}
		//LogO(wp->getName() + (tip ? "  *" : ""));
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
	{	// TODO: isnt resizing properly ..
		mToolTip->setSize(320, 96);  // start size for wrap
		String s = TR(sender->getUserString("tip"));
		mToolTipTxt->setCaption(s);
		const IntSize &textsize = mToolTipTxt->getTextSize();
		mToolTip->setSize(textsize.width*1.5, textsize.height*1.5);
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


//  next/prev in list by key
int App::LNext(MyGUI::ListPtr lp, int rel)
{
	int i = std::max(0, std::min((int)lp->getItemCount()-1, (int)lp->getIndexSelected()+rel ));
	lp->setIndexSelected(i);
	lp->beginToItemAt(std::max(0, i-11));  // center
	return i;
}

void App::trkLNext(int rel)	{
	if (!(isFocGui && mWndTabs->getIndexSelected() == 0))  return;
	listTrackChng(trkList,LNext(trkList, rel));  }

void App::carLNext(int rel)	{
	if (!(isFocGui && mWndTabs->getIndexSelected() == 1))  return;
	listCarChng(carList,  LNext(carList, rel));  }

void App::rplLNext(int rel)	{
	if (!(isFocGui && mWndTabs->getIndexSelected() == 3))  return;
	listRplChng(rplList,  LNext(rplList, rel));  }
