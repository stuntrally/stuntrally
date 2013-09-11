#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "OgreGame.h"
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
	loadReadme = true;


	//  new widgets
	MyGUI::FactoryManager::getInstance().registerFactory<MultiList2>("Widget");
	MyGUI::FactoryManager::getInstance().registerFactory<Slider>("Widget");
	

	//  load Options layout
	vwGui = LayoutManager::getInstance().loadLayout("Game.layout");
	//mLayout = vwGui.at(0);

	//  window
	mWndMain = mGUI->findWidget<Window>("MainMenuWnd");
	mWndGame = mGUI->findWidget<Window>("GameWnd");
	mWndReplays = mGUI->findWidget<Window>("ReplaysWnd");
	mWndHelp = mGUI->findWidget<Window>("HelpWnd");
	mWndOpts = mGUI->findWidget<Window>("OptionsWnd");

	mWndChampStage = mGUI->findWidget<Window>("WndChampStage");  mWndChampStage->setVisible(false);
	mWndChampEnd   = mGUI->findWidget<Window>("WndChampEnd");    mWndChampEnd->setVisible(false);
	mWndChallStage = mGUI->findWidget<Window>("WndChallStage");  mWndChallStage->setVisible(false);
	mWndChallEnd   = mGUI->findWidget<Window>("WndChallEnd");    mWndChallEnd->setVisible(false);

	mWndNetEnd = mGUI->findWidget<Window>("WndNetEnd");  mWndNetEnd->setVisible(false);
	mWndTweak = mGUI->findWidget<Window>("WndTweak");  mWndTweak->setVisible(false);
	mWndTweak->setPosition(0,40);
	
	//  main menu
	for (int i=0; i < ciMainBtns; ++i)
	{
		const String s = toStr(i);
		mWndMainPanels[i] = mWndMain->findWidget("PanMenu"+s);
		mWndMainBtns[i] = (ButtonPtr)mWndMain->findWidget("BtnMenu"+s);
		mWndMainBtns[i]->eventMouseButtonClick += newDelegate(this, &App::MainMenuBtn);
	}
		
	updMouse();
	
	//  center
	IntSize w = mWndMain->getSize();
	int wx = mWindow->getWidth(), wy = mWindow->getHeight();
	mWndMain->setPosition((wx-w.width)*0.5f, (wy-w.height)*0.5f);

	TabPtr tab;
	tab = mGUI->findWidget<Tab>("TabWndGame");    tab->setIndexSelected(1); tab->setSmoothShow(false);	mWndTabsGame = tab;		tab->eventTabChangeSelect += newDelegate(this, &App::MenuTabChg);
	tab = mGUI->findWidget<Tab>("TabWndReplays"); tab->setIndexSelected(1);	tab->setSmoothShow(false);	mWndTabsRpl = tab;		tab->eventTabChangeSelect += newDelegate(this, &App::MenuTabChg);
	tab = mGUI->findWidget<Tab>("TabWndHelp");    tab->setIndexSelected(1);	tab->setSmoothShow(false);	mWndTabsHelp = tab;		tab->eventTabChangeSelect += newDelegate(this, &App::MenuTabChg);
	tab = mGUI->findWidget<Tab>("TabWndOptions"); tab->setIndexSelected(1); tab->setSmoothShow(false);	mWndTabsOpts = tab;		tab->eventTabChangeSelect += newDelegate(this, &App::MenuTabChg);

	if (pSet->inMenu > MNU_Single && pSet->inMenu <= MNU_Challenge)  mWndTabsGame->setIndexSelected(TAB_Champs);

	//  get sub tabs
	vSubTabsGame.clear();
	for (size_t i=0; i < mWndTabsGame->getItemCount(); ++i)
	{	// todo: startsWith("SubTab")..
		MyGUI::TabPtr sub = (TabPtr)mWndTabsGame->getItemAt(i)->findWidget(
			i==TAB_Champs ? "ChampType" : (i==TAB_Multi ? "tabsNet" : "tabPlayer") );
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

	valGraphsType = mGUI->findWidget<StaticText>("GraphsTypeVal",false);
	Cmb(combo, "CmbGraphsType", comboGraphs);  cmbGraphs = combo;
	if (combo)
	{	combo->removeAllItems();
		for (int i=0; i < Gh_ALL; ++i)
			combo->addItem(csGraphNames[i]);
		combo->setIndexSelected(pSet->graphs_type);
	}
	valGraphsType->setCaption(toStr(pSet->graphs_type));
	
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
	//Chk("SMAA", chkVidSSAA, pSet->ssaa);
	Chk("ReverseOn", chkReverse, pSet->gui.trackreverse);
	Chk("ParticlesOn", chkParticles, pSet->particles);	Chk("TrailsOn", chkTrails, pSet->trails);

	//  hud
	Chk("Digits", chkDigits, pSet->show_digits);
	Chk("Gauges", chkGauges, pSet->show_gauges);  ShowHUD();//
	Chk("Arrow", chkArrow, pSet->check_arrow);
	Chk("ChkBeam", chkBeam, pSet->check_beam);
	
	Chk("Minimap", chkMinimap, pSet->trackmap);	chMinimp = bchk;
	Chk("MiniZoom", chkMiniZoom, pSet->mini_zoomed);  Chk("MiniRot", chkMiniRot, pSet->mini_rotated);
	Chk("MiniTer", chkMiniTer, pSet->mini_terrain);   Chk("MiniBorder", chkMiniBorder, pSet->mini_border);

	Chk("CamInfo", chkCamInfo, pSet->show_cam);
	Chk("CamTilt", chkCamTilt, pSet->cam_tilt);

	Chk("Times", chkTimes, pSet->show_times);	chTimes  = bchk;
	Chk("Opponents", chkOpponents, pSet->show_opponents);  chOpponents = bchk;
	Chk("OpponentsSort", chkOpponentsSort, pSet->opplist_sort);


	//  other
	Chk("Fps", chkFps, pSet->show_fps);  chFps = bchk;
	bckFps->setVisible(pSet->show_fps);
	Chk("Wireframe", chkWireframe, mbWireFrame);  chWire = bchk;

	Chk("ProfilerTxt", chkProfilerTxt, pSet->profilerTxt);	chProfTxt = bchk;
	Chk("BulletDebug", chkBltDebug, pSet->bltDebug);		chBlt = bchk;
	Chk("BulletProfilerTxt", chkBltProfilerTxt, pSet->bltProfilerTxt);	chBltTxt = bchk;

	Chk("CarDbgBars", chkCarDbgBars, pSet->car_dbgbars);	chDbgB = bchk;
	Chk("CarDbgTxt", chkCarDbgTxt, pSet->car_dbgtxt);		chDbgT = bchk;
	Chk("CarDbgSurf", chkCarDbgSurf, pSet->car_dbgsurf);	chDbgS = bchk;
	Chk("CarTireVis", chkCarTireVis, pSet->car_tirevis);	chTireVis = bchk;
	Chk("Graphs", chkGraphs, pSet->show_graphs);		chGraphs = bchk;
	Slv(DbgTxtClr, pSet->car_dbgtxtclr /1.f);
	Slv(DbgTxtCnt, pSet->car_dbgtxtcnt /8.f);


	//  car setup  todo: for each player ..
	Chk("CarABS",  chkAbs, pSet->abs[0]);  bchAbs = bchk;
	Chk("CarTCS", chkTcs, pSet->tcs[0]);  bchTcs = bchk;
	Chk("CarGear", chkGear, pSet->autoshift);	Chk("CarRear", chkRear, pSet->autorear);
	Chk("CarRearThrInv", chkRearInv, pSet->rear_inv);

	TabPtr tTires = mGUI->findWidget<Tab>("tabCarTires");
	if (tTires)  tTires->eventTabChangeSelect += newDelegate(this, &App::tabTireSet);
	Slv(SSSEffect,	pSet->sss_effect[0]);  slSSSEff = sl;
	Slv(SSSVelFactor, pSet->sss_velfactor[0]/2.f);  slSSSVel = sl;
	Slv(SteerRangeSurf, pSet->steer_range[0]-0.3f);  slSteerRngSurf = sl;
	Slv(SteerRangeSim, (pSet->gui.sim_mode == "easy" ? pSet->steer_sim_easy : pSet->steer_sim_normal)-0.3f);  slSteerRngSim = sl;


	//  game  ------------------------------------------------------------
	Chk("VegetCollis", chkVegetCollis, pSet->gui.collis_veget);
	Chk("CarCollis", chkCarCollis, pSet->gui.collis_cars);
	Chk("RoadWCollis", chkRoadWCollis, pSet->gui.collis_roadw);
	Chk("DynamicObjects", chkDynObjects, pSet->gui.dyn_objects);

	Cmb(combo, "CmbBoost", comboBoost);		cmbBoost = combo;	if (combo)
	{	combo->removeAllItems();
		combo->addItem(TR("#{Never}"));		combo->addItem(TR("#{FuelLap}"));
		combo->addItem(TR("#{FuelTime}"));	combo->addItem(TR("#{Always}"));
		combo->setIndexSelected(pSet->gui.boost_type);
	}
	Cmb(combo, "CmbFlip", comboFlip);		cmbFlip = combo;	if (combo)
	{	combo->removeAllItems();
		combo->addItem(TR("#{Never}"));		combo->addItem(TR("#{FuelBoost}"));
		combo->addItem(TR("#{Always}"));
		combo->setIndexSelected(pSet->gui.flip_type);
	}
	Cmb(combo, "CmbDamage", comboDamage);	cmbDamage = combo;	if (combo)
	{	combo->removeAllItems();
		combo->addItem(TR("#{None}"));		combo->addItem(TR("#{Reduced}"));
		combo->addItem(TR("#{Normal}"));
		combo->setIndexSelected(pSet->gui.damage_type);
	}
	Cmb(combo, "CmbRewind", comboRewind);	cmbRewind = combo;	if (combo)
	{	combo->removeAllItems();
		combo->addItem(TR("#{None}"));		combo->addItem(TR("#{Always}"));
		combo->addItem(TR("#{FuelLap}"));	combo->addItem(TR("#{FuelTime}"));
		combo->setIndexSelected(pSet->gui.rewind_type);
	}

	//  split
	Btn("btnPlayers1", btnNumPlayers);	Btn("btnPlayers2", btnNumPlayers);
	Btn("btnPlayers3", btnNumPlayers);	Btn("btnPlayers4", btnNumPlayers);
	Chk("chkSplitVertically", chkSplitVert, pSet->split_vertically);
	Chk("chkStartOrderRev", chkStartOrd, pSet->gui.start_order);
	valLocPlayers = mGUI->findWidget<StaticText>("valLocPlayers");
	if (valLocPlayers)  valLocPlayers->setCaption(toStr(pSet->gui.local_players));


	//  sim mode radio
	bRsimEasy = mGUI->findWidget<Button>("SimModeEasy");
	bRsimNorm = mGUI->findWidget<Button>("SimModeNorm");
	bool bNorm = pSet->gui.sim_mode == "normal";
	bool bEasy = pSet->gui.sim_mode == "easy";
	bRsimEasy->setStateSelected(bEasy);  bRsimEasy->eventMouseButtonClick += newDelegate(this, &App::radSimEasy);
	bRsimNorm->setStateSelected(bNorm);  bRsimNorm->eventMouseButtonClick += newDelegate(this, &App::radSimNorm);

	//  kmh/mph radio
	bRkmh = mGUI->findWidget<Button>("kmh");
	bRmph = mGUI->findWidget<Button>("mph");
	if (bRkmh && bRmph)  {  bRkmh->setStateSelected(!pSet->show_mph);  bRmph->setStateSelected(pSet->show_mph);
		bRkmh->eventMouseButtonClick += newDelegate(this, &App::radKmh);
		bRmph->eventMouseButtonClick += newDelegate(this, &App::radMph);  }


	//  startup
	Chk("StartInMain", chkStartInMain, pSet->startInMain);
	Chk("AutoStart", chkAutoStart, pSet->autostart);
	Chk("EscQuits", chkEscQuits, pSet->escquit);
	Chk("OgreDialog", chkOgreDialog, pSet->ogre_dialog);

	Chk("BltLines", chkBltLines, pSet->bltLines);
	Chk("ShowPictures", chkLoadPics, pSet->loadingbackground);
	Chk("MultiThread", chkMultiThread, pSet->multi_thr > 0);

	
	//  effects
	Chk("AllEffects", chkVidEffects, pSet->all_effects);
	Chk("Bloom", chkVidBloom, pSet->bloom);
	Slv(BloomInt,	pSet->bloomintensity);
	Slv(BloomOrig,	pSet->bloomorig);

	Chk("HDR", chkVidHDR, pSet->hdr);
	Slv(HDRParam1, pSet->hdrParam1);
	Slv(HDRParam2, pSet->hdrParam2);
	Slv(HDRParam3, pSet->hdrParam3);
	Slv(HDRAdaptationScale, pSet->hdrAdaptationScale);
	Slv(HDRBloomInt,  pSet->hdrbloomint);
	Slv(HDRBloomOrig, pSet->hdrbloomorig);
	Slv(HDRVignettingRadius, pSet->vignettingRadius/10);
	Slv(HDRVignettingDarkness, pSet->vignettingDarkness);
	
	Chk("MotionBlur", chkVidBlur, pSet->motionblur);
	Chk("ssao", chkVidSSAO, pSet->ssao);

	Chk("softparticles", chkVidSoftParticles, pSet->softparticles);
	Chk("DepthOfField", chkVidDepthOfField, pSet->dof);
	Chk("godrays", chkVidGodRays, pSet->godrays);
	Chk("BoostFOV", chkVidBoostFOV, pSet->boost_fov);

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
	Chk("RplChkBestOnly", chkRplChkBestOnly, pSet->rpl_bestonly);
	Chk("RplChkGhost", chkRplChkGhost, pSet->rpl_ghost);
	Chk("RplChkParticles", chkRplChkPar, pSet->rpl_ghostpar);

	Chk("RplChkRewind", chkRplChkRewind, pSet->rpl_ghostrewind);
	Chk("RplChkGhostOther", chkRplChkGhostOther, pSet->rpl_ghostother);
	Chk("RplChkTrackGhost", chkRplChkTrackGhost, pSet->rpl_trackghost);
	Slv(RplNumViewports, (pSet->rpl_numViews-1) / 3.f);

	//  radios
	Btn("RplBtnAll", btnRplAll);  rbRplAll = btn;
	Btn("RplBtnCur", btnRplCur);  rbRplCur = btn;
	Chk("RplBtnGhosts",chkRplGhosts, pSet->rpl_listghosts);
	btn = pSet->rpl_listview == 0 ? rbRplAll : rbRplCur;
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


	///  Car
	//------------------------------------------------------------
	const int clrBtn = 30;
	Real hsv[clrBtn][5] = {  // color buttons  hue,sat,val, gloss,refl
	{0.05,0.64,0.27, 0.10,0.9}, {0.00,0.97,0.90, 0.3, 1.2},  // cherry, red
	{0.91,1.00,1.00, 0.5, 1.0}, {0.86,1.00,0.97, 0.8, 0.6},  // orange, yellow
	{0.75,0.95,0.90, 1.0, 0.4}, {0.70,1.00,0.70, 0.03,1.3}, // lime, green
	{0.54,0.88,0.60, 0.7, 0.85},{0.51,0.90,0.50, 0.1, 0.7},  // cyan
	{0.41,0.34,0.30, 0.01,0.3}, {0.43,0.58,0.23, 0.1, 1.0},  // dark-cyan  //{0.45,0.54,0.37, 0.5,1.0},
	{0.37,0.78,0.21, 0.34,0.5}, {0.35,0.70,0.40, 0.5, 1.0}, {0.38,0.97,0.52, 0.5, 1.0},  // dark-blue
	{0.44,0.90,0.71, 1.0, 1.1}, {0.47,0.80,0.80, 0.2, 0.9}, // sky-blue
	{0.50,0.33,0.90, 0.9, 1.0}, {0.42,0.20,0.94, 0.5, 0.4}, // sky-white
	{0.63,0.21,0.62, 0.1, 1.2}, {0.80,0.52,0.32, 0.1, 0.6}, {0.62,0.74,0.12, 0.8, 0.7},  // olive-
	{0.28,0.00,0.12, 0.09,0.0}, {0.28,0.00,0.07, 0.14,0.84},  // black
	{0.83,0.00,0.20, 0.0, 0.8}, {0.41,0.00,0.86, 0.15,0.37}, // silver,white
	{0.83,0.31,0.31, 0.0, 0.6}, {0.91,0.40,0.37, 0.0, 1.0}, {0.20,0.40,0.37, 0.05,1.0},  // orng-white-
	{0.24,0.90,0.26, 0.04,0.8}, {0.28,0.57,0.17, 0.3, 1.0}, {0.27,0.38,0.23, 0.03,0.6},  // dark violet
	};
	for (int i=0; i < clrBtn; ++i)
	{
		StaticImagePtr img = mGUI->findWidget<StaticImage>("carClr"+toStr(i));
		Real h = hsv[i][0], s = hsv[i][1], v = hsv[i][2], g = hsv[i][3], r = hsv[i][4];
		ColourValue c;  c.setHSB(1.f-h, s, v);
		img->setColour(Colour(c.r,c.g,c.b));
		img->eventMouseButtonClick += newDelegate(this, &App::imgBtnCarClr);
		img->setUserString("s", toStr(s));  img->setUserString("h", toStr(h));
		img->setUserString("v", toStr(v));  img->setUserString("g", toStr(g));  img->setUserString("r", toStr(r));
	}
	Btn("CarClrRandom", btnCarClrRandom);
	Slv(NumLaps, (pSet->gui.num_laps - 1) / 20.f);

	txCarStatsTxt = mGUI->findWidget<StaticText>("CarStatsTxt");
	txCarStatsVals = mGUI->findWidget<StaticText>("CarStatsVals");

    txCarSpeed = mGUI->findWidget<StaticText>("CarSpeed");
    txCarType = mGUI->findWidget<StaticText>("CarType");

    txCarAuthor = mGUI->findWidget<StaticText>("CarAuthor");
    txTrackAuthor = mGUI->findWidget<StaticText>("TrackAuthor");
	
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
	listServers = mGUI->findWidget<MultiList>("MListServers");  int c=0;
	if (listServers)
	{	listServers->addColumn("#C0FFC0"+TR("#{Game name}"), 160);  ++c;
		listServers->addColumn("#50FF50"+TR("#{Track}"), 120);  ++c;
		listServers->addColumn("#80FFC0"+TR("#{Laps}"), 60);  ++c;
		listServers->addColumn("#FFFF00"+TR("#{Players}"), 60);  ++c;
		listServers->addColumn("#80FFFF"+TR("#{Collis.}"), 70);  ++c;
		listServers->addColumn("#A0D0FF"+TR("#{Simulation}"), 80);  ++c;
		listServers->addColumn("#A0D0FF"+TR("#{Boost}"), 90);  ++c;
		listServers->addColumn("#FF6060"+TR("#{Locked}"), 60);  iColLock = c;  ++c;
		listServers->addColumn("#FF9000"+TR("#{NetHost}"), 140);  iColHost = c;  ++c;
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
	{	listPlayers->addColumn("#80C0FF"+TR("#{Player}"), 140);
		listPlayers->addColumn("#F08080"+TR("#{Car}"), 60);
		listPlayers->addColumn("#C0C060"+TR("#{Peers}"), 60);
		listPlayers->addColumn("#60F0F0"+TR("#{Ping}"), 60);
		listPlayers->addColumn("#40F040"+TR("#{NetReady}"), 60);
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
    //  track,game text
    valNetGameInfo = mGUI->findWidget<StaticText>("valNetGameInfo");

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
	//  user dir
    MyGUI::EditPtr edUserDir = mGUI->findWidget<Edit>("EdUserDir");
	edUserDir->setCaption(PATHMANAGER::UserConfigDir());

	
	///  tweak
	for (int i=0; i < ciEdCar; ++i)
		edCar[i] = mGUI->findWidget<Edit>("EdCar"+toStr(i),false);
	edTweakCol = mGUI->findWidget<Edit>("TweakEditCol");
	edPerfTest = mGUI->findWidget<Edit>("TweakPerfTest");
	tabEdCar = mGUI->findWidget<Tab>("TabEdCar");
	tabEdCar->setIndexSelected(pSet->car_ed_tab);
	tabEdCar->eventTabChangeSelect += newDelegate(this, &App::tabCarEdChng);

	tabTweak = mGUI->findWidget<Tab>("TabTweak");
	txtTweakPath = mGUI->findWidget<StaticText>("TweakPath");
	txtTweakPathCol = mGUI->findWidget<StaticText>("TweakPathCol");
	txtTweakTire = mGUI->findWidget<StaticText>("TweakTireSaved");

	Btn("TweakCarSave", btnTweakCarSave);  Btn("TweakTireSave", btnTweakTireSave);
	Btn("TweakColSave", btnTweakColSave);
	Cmb(cmbTweakTireSet,"TweakTireSet",CmbTweakTireSet);  cmbTweakTireSet->eventEditTextChange+= newDelegate(this, &App::CmbEdTweakTireSet);


	///  input tab  -------
	InitInputGui();
	
	InitGuiScreenRes();
	
	
	///  cars list
    //------------------------------------------------------------------------
	TabItem* cartab = (TabItem*)mWndGame->findWidget("TabCar");
	carList = cartab->createWidget<MultiList2>("MultiListBox",16,48,200,110, Align::Left | Align::VStretch);
	carList->setColour(Colour(0.7,0.85,1.0));
	carList->removeAllColumns();  int n=0;
	carList->addColumn("#FF8888"+TR("#{Car}"), colCar[n++]);
	carList->addColumn("#FFC080"+TR("#{CarSpeed}"), colCar[n++]);
	carList->addColumn("#B0B8C0"+TR("#{CarYear}"), colCar[n++]);
	carList->addColumn("#C0C0E0"+TR("#{CarType}"), colCar[n++]);
	carList->addColumn(" ", colCar[n++]);

	FillCarList();  //once

	carList->mSortColumnIndex = pSet->cars_sort;
	carList->mSortUp = pSet->cars_sortup;
   	carList->eventListChangePosition += newDelegate(this, &App::listCarChng);

   	CarListUpd(false);  //upd

    sListCar = pSet->gui.car[0];
    imgCar = mGUI->findWidget<StaticImage>("CarImg");
    carDesc = mGUI->findWidget<Edit>("CarDesc",false);
    listCarChng(carList,0);


    ///  tracks list, text, chg btn
    //------------------------------------------------------------------------

	trkDesc[0] = mGUI->findWidget<Edit>("TrackDesc");
	sListTrack = pSet->gui.track;

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
    valTrkNet = mGUI->findWidget<StaticText>("TrackText");
	//  preview images
	imgPrv[1] = mGUI->findWidget<StaticImage>("TrackImg2");
	imgTer[1] = mGUI->findWidget<StaticImage>("TrkTerImg2");
	imgMini[1] = mGUI->findWidget<StaticImage>("TrackMap2");
	//  track stats text
	for (int i=0; i < StTrk; ++i)
		stTrk[1][i] = mGUI->findWidget<StaticText>("2iv"+toStr(i+1), false);
	for (int i=0; i < InfTrk; ++i)
		infTrk[1][i] = mGUI->findWidget<StaticText>("2ti"+toStr(i+1), false);

	edChInfo = mGUI->findWidget<EditBox>("ChampInfo");
	if (edChInfo)  edChInfo->setVisible(pSet->champ_info);
	Btn("btnChampInfo",btnChampInfo);

	panCh = mWndGame->findWidget("panCh");
	txtCh = (TextBox*)mWndGame->findWidget("txtChDetail");
	valCh = (TextBox*)mWndGame->findWidget("valChDetail");
	for (int i=0; i<3; ++i) {  String s = toStr(i);
		txtChP[i] = (TextBox*)mWndGame->findWidget("txtChP"+s);
		valChP[i] = (TextBox*)mWndGame->findWidget("valChP"+s);  }
	edChDesc = mGUI->findWidget<EditBox>("ChampDescr");

	//  Champs list  -------------
	MyGUI::MultiList2* li;
	TabItem* trktab = (TabItem*)mWndGame->findWidget("TabChamps");
	li = trktab->createWidget<MultiList2>("MultiListBox",0,0,400,300, Align::Left | Align::VStretch);
	li->eventListChangePosition += newDelegate(this, &App::listChampChng);
   	li->setVisible(false);
	
	li->removeAllColumns();  c=0;
	li->addColumn("#80A080N", colCh[c++]);
	li->addColumn(TR("#40F040#{Name}"), colCh[c++]);		li->addColumn(TR("#F0F040#{Difficulty}"), colCh[c++]);
	li->addColumn(TR("#80F0C0#{Stages}"), colCh[c++]);		li->addColumn(TR("#80E0FF#{Time} m:s"), colCh[c++]);
	li->addColumn(TR("#D0C0FF#{Progress}"), colCh[c++]);	li->addColumn(TR("#F0E0F0#{Score}"), colCh[c++]);
	li->addColumn(" ", colCh[c++]);
	liChamps = li;

	//  Challs list  -------------
	li = trktab->createWidget<MultiList2>("MultiListBox",0,0,400,300, Align::Left | Align::VStretch);
	li->eventListChangePosition += newDelegate(this, &App::listChallChng);
   	li->setVisible(false);
	
	li->removeAllColumns();  c=0;
	li->addColumn("#80A080N", colChL[c++]);
	li->addColumn(TR("#60F060#{Name}"), colChL[c++]);		li->addColumn(TR("#F0D040#{Difficulty}"), colChL[c++]);
	li->addColumn(TR("#F09090#{Cars}"), colChL[c++]);
	li->addColumn(TR("#80F0C0#{Stages}"), colChL[c++]);		li->addColumn(TR("#80E0FF#{Time} m"), colChL[c++]);
	li->addColumn(TR("#D0C0FF#{Progress}"), colChL[c++]);
	li->addColumn(TR("#F0F8FF#{Prize}"), colChL[c++]);		li->addColumn(TR("#F0D0F0#{Score}"), colChL[c++]);
	li->addColumn(" ", colChL[c++]);
	liChalls = li;

	//  Stages list  -------------
	trktab = (TabItem*)mWndGame->findWidget("TabStages");
	li = trktab->createWidget<MultiList2>("MultiListBox",0,0,400,300, Align::Left | Align::VStretch);
	li->setColour(Colour(0.7,0.73,0.76));
	li->eventListChangePosition += newDelegate(this, &App::listStageChng);
   	li->setVisible(false);
	
	li->removeAllColumns();  c=0;
	li->addColumn("#80A080N", colSt[c++]);
	li->addColumn(TR("#50F050#{Track}"), colSt[c++]);		li->addColumn(TR("#80FF80#{Scenery}"), colSt[c++]);
	li->addColumn(TR("#F0F040#{Difficulty}"), colSt[c++]);	li->addColumn(TR("#60E0A0#{Laps}"), colSt[c++]);
	li->addColumn(TR("#80E0FF#{Time} m:s"), colSt[c++]);	li->addColumn(TR("#F0E0F0#{Score}"), colSt[c++]);
	li->addColumn(" ", colSt[c++]);
	liStages = li;

	updChampListDim();
	ChampsListUpdate();  listChampChng(liChamps, liChamps->getIndexSelected());
	ChallsListUpdate();  listChallChng(liChalls, liChalls->getIndexSelected());


	//  tabs
	tabTut = mGUI->findWidget<Tab>("TutType",false);
	if (tabTut)
	{	tabTut->setIndexSelected(pSet->tut_type);
		tabTut->eventTabChangeSelect += newDelegate(this, &App::tabTutType);
	}
	tabChamp = mGUI->findWidget<Tab>("ChampType",false);
	if (tabChamp)
	{	tabChamp->setIndexSelected(pSet->champ_type);
		tabChamp->eventTabChangeSelect += newDelegate(this, &App::tabChampType);
	}
	tabChall = mGUI->findWidget<Tab>("ChallType",false);
	if (tabChall)
	{	tabChall->setIndexSelected(pSet->chall_type);
		tabChall->eventTabChangeSelect += newDelegate(this, &App::tabChallType);
	}
	imgTut = mGUI->findWidget<StaticImage>("imgTut",false);
	imgChamp = mGUI->findWidget<StaticImage>("imgChamp",false);
	imgChall = mGUI->findWidget<StaticImage>("imgChall",false);

	Btn("btnTutStart", btnChampStart);    btStTut = btn;
	Btn("btnChampStart", btnChampStart);  btStChamp = btn;
	Btn("btnChallStart", btnChallStart);  btStChall = btn;
	Btn("btnChRestart", btnChRestart);  btChRestart = btn;


	//  ch other
	Chk("ChampRev", chkChampRev, pSet->gui.champ_rev);
	
	Btn("btnChampStageBack", btnChampStageBack);
	Btn("btnChampStageStart", btnChampStageStart);  btChampStage = btn;
	Btn("btnChampEndClose", btnChampEndClose);

	Btn("btnChallStageBack", btnChallStageBack);
	Btn("btnChallStageStart", btnChallStageStart);  btChallStage = btn;
	Btn("btnChallEndClose", btnChallEndClose);

	Btn("btnStageNext", btnStageNext);
	Btn("btnStagePrev", btnStagePrev);
    valStageNum = mGUI->findWidget<StaticText>("StageNum");

	edChampStage = (EditBox*)mWndChampStage->findWidget("ChampStageText");
	edChallStage = (EditBox*)mWndChallStage->findWidget("ChallStageText");
	edChampEnd   = (EditBox*)mWndChampEnd->findWidget("ChampEndText");
	edChallEnd   = (EditBox*)mWndChallEnd->findWidget("ChallEndText");
	imgChampStage = (ImageBox*)mWndChampStage->findWidget("ChampStageImg");
	imgChallStage = (ImageBox*)mWndChallStage->findWidget("ChallStageImg");

	imgChampEndCup  = (ImageBox*)mWndChampEnd->findWidget("ChampEndImgCup");
	imgChallFail = (ImageBox*)mWndChallEnd->findWidget("ChallEndImgFail");
	imgChallCup  = (ImageBox*)mWndChallEnd->findWidget("ChallEndImgCup");
	txChallEndC = (TextBox*)mWndChallEnd->findWidget("ChallEndCongrats");
	txChallEndF = (TextBox*)mWndChallEnd->findWidget("ChallEndFinished");

	UpdChampTabVis();


	//  netw end list  ------
	Btn("btnNetEndClose", btnNetEndClose);
	li = mWndNetEnd->createWidget<MultiList2>("MultiListBox",4,42,632,360, Align::Left | Align::VStretch);
	li->setInheritsAlpha(false);  li->setColour(Colour(0.8,0.9,1,1));
	li->removeAllColumns();
	li->addColumn("", 40);  //N
	li->addColumn(TR("#{TBPlace}"), 60);	li->addColumn(TR("#{NetNickname}"), 180);
	li->addColumn(TR("#{TBTime}"), 120);	li->addColumn(TR("#{TBBest}"), 120);
	li->addColumn(TR("#{TBLap}"), 60);
	liNetEnd = li;
	

	bGI = true;  // gui inited, gui events can now save vals
	
	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Init Gui: ") + fToStr(dt,0,3) + " ms");
}
