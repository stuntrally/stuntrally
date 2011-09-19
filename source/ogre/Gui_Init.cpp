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

	GuiInitLang();

	GuiInitGraphics();
	    
	//  view sizes
	Slv(SizeGaug,	(pSet->size_gauges-0.1f) /0.15f);
	Slv(SizeMinimap,(pSet->size_minimap-0.05f) /0.25f);
	Slv(ZoomMinimap,powf((pSet->zoom_minimap-1.0f) /9.f, 0.5f));
	
	//  particles/trails
	Slv(Particles,	powf(pSet->particles_len /4.f, 0.5f));
	Slv(Trails,		powf(pSet->trails_len /4.f, 0.5f));

	//  reflect
	Slv(ReflSkip,	powf(pSet->refl_skip /1000.f, 0.5f));
	Slv(ReflSize,	pSet->refl_size /res);
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
	if (bnQuit)  {  bnQuit->eventMouseButtonClick = newDelegate(this, &App::btnQuit);  bnQuit->setVisible(isFocGui);  }
	Chk("ReverseOn", chkReverse, pSet->trackreverse);
	Chk("ParticlesOn", chkParticles, pSet->particles);	Chk("TrailsOn", chkTrails, pSet->trails);

	Chk("Fps", chkFps, pSet->show_fps);	chFps = mGUI->findWidget<Button>("Fps");
	if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();

	Chk("Digits", chkDigits, pSet->show_digits);
	Chk("Gauges", chkGauges, pSet->show_gauges);  ShowHUD();//

	Chk("Minimap", chkMinimap, pSet->trackmap);	chMinimp = bchk;
	Chk("MiniZoom", chkMiniZoom, pSet->mini_zoomed);  Chk("MiniRot", chkMiniRot, pSet->mini_rotated);
	Chk("MiniTer", chkMiniTer, pSet->mini_terrain);
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

	//  startup
	Chk("OgreDialog", chkOgreDialog, pSet->ogre_dialog);
	Chk("AutoStart", chkAutoStart, pSet->autostart);
	Chk("EscQuits", chkEscQuits, pSet->escquit);
	Chk("BltLines", chkBltLines, pSet->bltLines);
	Chk("ShowPictures", chkLoadPics, pSet->loadingbackground);
	
	//  effects
	//TODO: Chk("AllEffects", load compositor shaders only if creating ...
	Chk("Bloom", chkVidBloom, pSet->bloom);
	Chk("HDR", chkVidHDR, pSet->hdr);
	Chk("MotionBlur", chkVidBlur, pSet->motionblur);

	Slv(BloomInt,	pSet->bloomintensity);
	Slv(BloomOrig,	pSet->bloomorig);
	Slv(BlurIntens, pSet->motionblurintensity);
	
	//todo: button_ramp, speed_sens..
	

	//  replays  ------------------------------------------------------------
	Btn("RplLoad", btnRplLoad);  Btn("RplSave", btnRplSave);
	Btn("RplDelete", btnRplDelete);  Btn("RplRename", btnRplRename);
	//  settings
	Chk("RplChkAutoRec", chkRplAutoRec, pSet->rpl_rec);
	Chk("RplChkGhost", chkRplChkGhost, pSet->rpl_ghost);
	Chk("RplChkBestOnly", chkRplChkBestOnly, pSet->rpl_bestonly);
	Chk("RplChkAlpha", chkRplChkAlpha, pSet->rpl_alpha);
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
	
	InitGuiScrenRes();
	
	
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

    imgCar = (StaticImagePtr)mLayout->findWidget("CarImg");
    listCarChng(carList,0);


    ///  tracks list, text, chg btn
    //------------------------------------------------------------------------

	//  track text, chg btn
	trkDesc = (EditPtr)mLayout->findWidget("TrackDesc");
    valTrk = (StaticTextPtr)mLayout->findWidget("TrackText");
    if (valTrk)
		valTrk->setCaption(TR("#{Track}: " + pSet->track));  sListTrack = pSet->track;

    GuiInitTrack();

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


void App::UpdCarClrSld(bool upd)
{
	HScrollPtr sl;  size_t v;
	bUpdCarClr = false;
	Slv(CarClrH, pSet->car_hue[iCurCar]);
	Slv(CarClrS, (pSet->car_sat[iCurCar] +1)*0.5f);  if (upd)  bUpdCarClr = true;
	Slv(CarClrV, (pSet->car_val[iCurCar] +1)*0.5f);  bUpdCarClr = true;
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
