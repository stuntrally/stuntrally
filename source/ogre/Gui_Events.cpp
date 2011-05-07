#include "stdafx.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "OgreGame.h"
#include "MyGUI_PointerManager.h"
#include <boost/filesystem.hpp>
using namespace MyGUI;

#define res  1000000.f
#define Fmt  sprintf


///  Gui Events
//-----------------------------------------------------------------------------------------------------------


//  [Multiplayer]
//---------------------------------------------------------------------


void App::NetUpdPlayers()
{
	if (!listPlayers)  return;

	//  add players to list
	listPlayers->removeAllItems();
	for (int i=0; i < rand()%3+1/*count*/; ++i)
	{
		listPlayers->addItem("Eddy" + toStr(i));  int l = listPlayers->getItemCount()-1;
		listPlayers->setSubItemNameAt(1, l, "ES");
		listPlayers->setSubItemNameAt(2, l, toStr(rand()%200+10));
		listPlayers->setSubItemNameAt(3, l, "Yes");
	}
}

void App::evBtnNetRefresh(WP)
{
	mMasterClient.reset(new MasterClient(gameInfoListener.get()));
	mMasterClient->connect(pSet->master_server_address, pSet->master_server_port);
	// The actual refresh will be requested automatically when the connection is made
}

void App::evBtnNetJoin(WP)
{
	//  join selected game
	if (!listServers)  return;

	size_t i = listServers->getIndexSelected();  if (i==ITEM_NONE)  return;
	
	Message::createMessageBox(  // #{transl ..
		"Message", "Join game", "Game name: " + listServers->getItemNameAt(i),
		MessageBoxStyle::IconInfo | MessageBoxStyle::Ok);
	
	//.. get players list for current game
	NetUpdPlayers();
	
	//  update track info
	if (valNetTrack)
		valNetTrack->setCaption("Track: " + sListTrack);
	if (imgNetTrack)
		imgNetTrack->setImageTexture(sListTrack+".jpg");
	if (edNetTrackInfo)
		edNetTrackInfo->setCaption("eiuru shaf adlkj tqer agjkdfh lgeir gieroh gsdkdhrti");
}


void App::evBtnNetReady(WP)
{
	//  ready for game,  waiting for other players
	btnNetReady->setCaption("Waiting..");
}

void App::evBtnNetLeave(WP)
{
	//  leave current game
	//String s = btnNetLeave->getCaption();
	btnNetLeave->setCaption("Leaving..");
	//btnNetLeave->setCaption(s);
}

	// info texts
	//valNetGames
	//valNetChat

void App::evBtnNetSendMsg(WP)
{
	if (!edNetChatMsg || !listNetChat || !edNetNick)  return;

	String nick = edNetNick->getCaption();
	String msg = edNetChatMsg->getCaption();

	listNetChat->addItem(nick + ": " + msg);
	//btnNetSendMsg
}
    
//  net settings

void App::evEdNetNick(EditPtr ed)
{
	pSet->nickname = ed->getCaption();
}

void App::evEdNetServerIP(EditPtr ed)
{
	pSet->master_server_address = ed->getCaption();
}

void App::evEdNetServerPort(EditPtr ed)
{
	pSet->master_server_port = s2i(ed->getCaption());
}

void App::evEdNetLocalPort(EditPtr ed)
{
	pSet->local_port = s2i(ed->getCaption());
}


//  [Input]
//---------------------------------------------------------------------

void App::controlBtnClicked(Widget* sender)
{
	sender->setCaption( TR("#{InputAssignKey}"));
	// activate key capture mode
	bAssignKey = true;
	pressedKeySender = sender;
	// hide mouse
	MyGUI::PointerManager::getInstance().setVisible(false);
}


//  [Graphics]
//---------------------------------------------------------------------

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
	Vector3 sc = v*Vector3::UNIT_SCALE;

	SceneNode* nskb = mSceneMgr->getSkyBoxNode();
	if (nskb)  nskb->setScale(sc*0.58);
	else  if (ndSky)  ndSky->setScale(sc);

	pSet->view_distance = v;
	if (valViewDist){	Fmt(s, "%4.1f km", v*0.001f);	valViewDist->setCaption(s);  }
	// Set new far clip distance for all cams
	mSplitMgr->UpdateCamDist();
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
	if (valVolEnv){  Fmt(s, "%4.2f", v);	valVolEnv->setCaption(s);  }
}


//  car color
void App::slCarClrH(SL)
{
	Real v = val/res;  pSet->car_hue = v;
	if (valCarClrH){	Fmt(s, "%4.2f", v);	valCarClrH->setCaption(s);  }
	for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		(*it)->ChangeClr();
}
void App::slCarClrS(SL)
{
	Real v = -1.f + 2.f * val/res;  pSet->car_sat = v;
	if (valCarClrS){	Fmt(s, "%4.2f", v);	valCarClrS->setCaption(s);  }
	for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		(*it)->ChangeClr();
}
void App::slCarClrV(SL)
{
	Real v = -1.f + 2.f * val/res;  pSet->car_val = v;
	if (valCarClrV){	Fmt(s, "%4.2f", v);	valCarClrV->setCaption(s);  }
	for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		(*it)->ChangeClr();
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
	if (valCar){  valCar->setCaption(TR("#{Car}: ") + sListCar);	pSet->car = sListCar;  }
}

//  track
void App::listTrackChng(List* li, size_t pos)
{
	if (!li)  return;
	size_t i = li->getIndexSelected();  if (i==ITEM_NONE)  return;

	const UString& sl = li->getItemNameAt(i);  String s = sl;
	s = StringUtil::replaceAll(s, "*", "");
	sListTrack = s;

	int u = *li->getItemDataAt<int>(i,false);
	bListTrackU = u;
	
	//  won't refresh if same-...  road dissapears if not found...
	if (imgPrv)  imgPrv->setImageTexture(sListTrack+".jpg");
	if (imgTer)  imgTer->setImageTexture(sListTrack+"_ter.jpg");
	if (imgMini)  imgMini->setImageTexture(sListTrack+"_mini.png");
	ReadTrkStats();
}

void App::btnChgTrack(WP)
{
	pSet->track = sListTrack;
	pSet->track_user = bListTrackU;
	if (valTrk)  valTrk->setCaption("Track: " + sListTrack);
}

//  new game
void App::btnNewGame(WP)
{
	NewGame();  isFocGui = false;  // off gui
	if (mWndOpts)  mWndOpts->setVisible(isFocGui);
	if (mWndRpl)  mWndRpl->setVisible(false);//
	if (bnQuit)  bnQuit->setVisible(isFocGui);
	mGUI->setVisiblePointer(isFocGuiOrRpl());
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

void App::chkDigits(WP wp){ 		ChkEv(show_digits); ShowHUD();   }

void App::chkReverse(WP wp){		ChkEv(trackreverse);	ReadTrkStats();  }

void App::chkParticles(WP wp)
{		
	ChkEv(particles);
	for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		(*it)->UpdParsTrails();
}
void App::chkTrails(WP wp)
{			
	ChkEv(trails);		
	for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		(*it)->UpdParsTrails();
}
void App::chkFps(WP wp){			ChkEv(show_fps);	if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();	}

void App::chkGauges(WP wp){			ChkEv(show_gauges);	ShowHUD();	}
void App::chkMinimap(WP wp){		ChkEv(trackmap);	if (ndMap)  ndMap->setVisible(pSet->trackmap);	}
void App::chkTimes(WP wp){			ChkEv(show_times);	ShowHUD();	}

//void App::chkRacingLine(WP wp){		ChkEv(racingline);	if (ndLine)  ndLine->setVisible(pSet->racingline);	}
void App::chkCamInfo(WP wp){		ChkEv(show_cam);	ShowHUD();	}

void App::chkCarDbgBars(WP wp){		ChkEv(car_dbgbars);	ShowHUD();	}
void App::chkCarDbgTxt(WP wp){		ChkEv(car_dbgtxt);	ShowHUD();	}
void App::chkBltDebug(WP wp){		ChkEv(bltDebug);	}
void App::chkBltProfilerTxt(WP wp){	ChkEv(bltProfilerTxt);	}

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
	if (resList->getIndexSelected() == MyGUI::ITEM_NONE) return;
	String mode = resList->getItem(resList->getIndexSelected());

	pSet->windowx = StringConverter::parseInt(StringUtil::split(mode, "x")[0]);
	pSet->windowy = StringConverter::parseInt(StringUtil::split(mode, "x")[1]);
	
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
		//mWindow->reposition(0,0);  // center ?..
	#endif
	}
	bWindowResized = true;
}

void App::chkVidBloom(WP wp)
{		
	ChkEv(bloom);		
	refreshCompositor();		
}
void App::chkVidHDR(WP wp)
{			
	ChkEv(hdr);	
	refreshCompositor();
}
void App::chkVidBlur(WP wp)
{		
	ChkEv(motionblur);
	refreshCompositor();
}

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


///  [Replay]  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void App::slRplPosEv(SL)  // change play pos
{
	if (!bRplPlay)  return;
	double oldt = pGame->timer.GetReplayTime();
	double v = val/res;  v = max(0.0, min(1.0, v));  v *= replay.GetTimeLength();
	pGame->timer.SetReplayTime(v);

	FollowCamera* fCam = (*carModels.begin())->fCam;
	fCam->first = true;  // instant change
	for (int i=0; i < 10; ++i)
		fCam->update(abs(v-oldt)/10.f);  //..?
}

void App::btnRplLoad(WP)  // Load
{
	//  from list
	int i = rplList->getIndexSelected();
	if (i == MyGUI::ITEM_NONE)  return;

	String name = rplList->getItemNameAt(i);
	string file = PATHMANAGER::GetReplayPath() + "/" + name + ".rpl";

	if (!replay.LoadFile(file))
	{
		Message::createMessageBox(  // #{transl ..
			"Message", "Load Replay", "Error: Can't load file.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
	}
	else  // car, track change
	{
		string car = replay.header.car, trk = replay.header.track;
		bool usr = replay.header.track_user == 1;

		pSet->car = car;
		pSet->track = trk;  pSet->track_user = usr;
		//bRplPlay = 1;
		btnNewGame(0);
		bRplPlay = 1;
	}
}

void App::btnRplSave(WP)  // Save
{
	String edit = edRplName->getCaption();
	String file = PATHMANAGER::GetReplayPath() + "/" + pSet->track + edit + ".rpl";
	///  save
	if (!replay.SaveFile(file.c_str()))
	{
		Message::createMessageBox(  // #{..
			"Message", "Save Replay", "Error: Can't save file.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
	}
	updReplaysList();
}

void App::btnRplDelete(WP)  // Delete
{
}

//  list change
void App::listRplChng(List* li, size_t pos)
{
	size_t i = li->getIndexSelected();  if (i == ITEM_NONE)  return;
	String name = li->getItemNameAt(i);
	string file = PATHMANAGER::GetReplayPath() + "/" + name + ".rpl";
	if (!valRplName)  return;  valRplName->setCaption(name);
	if (!valRplInfo)  return;
	
	//  load replay header upd text descr
	Replay rpl;
	if (rpl.LoadFile(file,true))
	{
		String ss = String("Track: ") + rpl.header.track + (rpl.header.track_user ? "  *user*" : "");
		valRplName->setCaption(ss);

		ss = String("Car: ") + rpl.header.car + "\n" +
			"Time: " + GetTimeString(rpl.GetTimeLength());
		valRplInfo->setCaption(ss);

		int size = boost::filesystem::file_size(file);
		sprintf(s, "%5.2f", float(size)/1000000.f);
		ss = String("File size:") + s + " MB\n" +
			"Version: " + toStr(rpl.header.ver) + "     " + toStr(rpl.header.frameSize) + "B";
		if (valRplInfo2)  valRplInfo2->setCaption(ss);
	}
	//edRplDesc  edRplName
}


void App::chkRplAutoRec(WP wp){		ChkEv(rpl_rec);		}

void App::chkRplChkGhost(WP wp){	/*ChkEv(rpl_play);*/	}


void App::btnRplCur(WP)
{
}

void App::btnRplAll(WP)
{
}

//  replay controls

void App::btnRplToStart(WP)
{
	pGame->timer.RestartReplay();
}

void App::btnRplToEnd(WP)
{
}

void App::btnRplBack(WP)
{
}

void App::btnRplForward(WP)
{
}

void App::btnRplPlay(WP)  // play / pause
{
	bRplPause = !bRplPause;
	UpdRplPlayBtn();
}

void App::UpdRplPlayBtn()
{
	String sign = bRplPause ? "|>" : "||";
	if (btRplPl)
		btRplPl->setCaption(sign);
}


void App::updReplaysList()
{
	if (!rplList)  return;
	rplList->removeAllItems();  int ii = 0;  bool bFound = false;

	strlist li;
	PATHMANAGER::GetFolderIndex(PATHMANAGER::GetReplayPath(), li, "rpl");
	for (strlist::iterator i = li.begin(); i != li.end(); ++i)
	if (StringUtil::endsWith(*i, ".rpl"))
	{
		String s = *i;
		s = StringUtil::replaceAll(s,".rpl","");
		rplList->addItem(s);
	}
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

		#if 0
		case KC_1:
		if (mSplitMgr)
		{	Ogre::Viewport* vp = *mSplitMgr->mViewports.begin();
			vp->setAutoUpdated(shift);
			vp->setVisibilityMask(shift ? 255 : 0);
		}	return true;
		case KC_2:
		if (mSplitMgr)
		{	Ogre::Viewport* vp = *(++mSplitMgr->mViewports.begin());
			vp->setAutoUpdated(shift);
			vp->setVisibilityMask(shift ? 255 : 0);
		}	return true;
		case KC_3:
		if (mSplitMgr)
		{	Ogre::Viewport* vp = *(--mSplitMgr->mViewports.end());
			vp->setAutoUpdated(shift);
			vp->setVisibilityMask(shift ? 255 : 0);
		}	return true;
		#endif
	   	
	   	case KC_F1:
	   	case KC_TAB:	// on/off gui
	   	if (!alt)  {
	   		isFocGui = !isFocGui;
	   		if (mWndOpts)	mWndOpts->setVisible(isFocGui);
			if (bnQuit)  bnQuit->setVisible(isFocGui);
	   		if (mGUI)	mGUI->setVisiblePointer(isFocGuiOrRpl());
	   		if (!isFocGui)  mToolTip->setVisible(false);
	   	}	return true;


		case KC_BACK:	// replay controls
			if (mWndRpl && !isFocGui)
			{	mWndRpl->setVisible(!mWndRpl->isVisible());
				return true;  }
			break;

		case KC_P:		// replay play/pause
			if (bRplPlay)
			{	bRplPause = !bRplPause;  UpdRplPlayBtn();  }
			return true;


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

		case KC_F10:	//  blt debug, txt
		if (shift)
		{	WP wp = chBltTxt;  ChkEv(bltProfilerTxt);  return false;  }
		else if (ctrl)
		{	WP wp = chBlt;  ChkEv(bltDebug);  return false;  }
		break;


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
	}

	if (!BaseApp::keyPressed(arg))
		return true;

	return true;
}
