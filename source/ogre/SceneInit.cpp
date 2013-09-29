#include "pch.h"
#include "common/Def_Str.h"
#include "common/RenderConst.h"
#include "common/data/CData.h"
#include "common/data/SceneXml.h"
#include "common/GuiCom.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "LoadingBar.h"
#include "FollowCamera.h"
#include "SplitScreen.h"
#include "common/GraphView.h"

#include "../network/gameclient.hpp"
#include "../paged-geom/PagedGeometry.h"
#include "../shiny/Main/Factory.hpp"

#include <boost/thread.hpp>
#include <MyGUI_OgrePlatform.h>
#include "common/MyGUI_D3D11.h"
#include <MyGUI_PointerManager.h>
#include <OgreTerrainGroup.h>
using namespace MyGUI;
using namespace Ogre;


//  Create Scene
//-------------------------------------------------------------------------------------
void App::createScene()
{
	//  tex fil
	MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	MaterialManager::getSingleton().setDefaultAnisotropy(pSet->anisotropy);
	
	//  restore camNums
	for (int i=0; i<4; ++i)
		if (pSet->cam_view[i] >= 0)
			carsCamNum[i] = pSet->cam_view[i];

	QTimer ti;  ti.update();  /// time

	//  data xml
	data->Load();
	sc->pFluidsXml = data->fluids;

	//  championships.xml, progress.xml
	gui->Ch_XmlLoad();

	//  user.xml
	#if 0
	userXml.LoadXml(PATHMANAGER::UserConfigDir() + "/user.xml");
	for (int i=0; i < data->tracks->trks.size(); ++i)
	{
		const TrackInfo& ti = data->tracks->trks[i];
		if (userXml.trkmap[ti.name]==0)
		{	// not found, add
			UserTrkInfo tu;  tu.name = ti.name;

			userXml.trks.push_back(tu);
			userXml.trkmap[ti.name] = userXml.trks.size();
	}	}
	userXml.SaveXml(PATHMANAGER::UserConfigDir() + "/user.xml");
	#endif

	LogO(String("**** ReplayFrame size: ") + toStr(sizeof(ReplayFrame)));	
	LogO(String("**** ReplayHeader size: ") + toStr(sizeof(ReplayHeader)));	

	ti.update();  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time load xmls: ") + fToStr(dt,0,3) + " ms");


	///  _Tool_ ghosts times ...........................
	#if 0
	gui->ToolGhosts();
	//mShutDown = true;  return;
	exit(0);
	#endif

	///  _Tool_ convert to track's ghosts ..............
	#if 0
	gui->ToolGhostsConv();
	exit(0);
	#endif

	///  _Tool_ presets ..............
	#if 0
	gui->ToolPresets();
	exit(0);
	#endif


	//  gui  * * *
	if (pSet->startInMain)
		pSet->isMain = true;

	if (!pSet->autostart)
		isFocGui = true;

	pSet->gui.champ_num = -1;  //dont auto start old chs
	pSet->gui.chall_num = -1;

	bRplRec = pSet->rpl_rec;  // startup setting

	gui->InitGui();

    //  bullet Debug drawing
    //------------------------------------
    if (pSet->bltLines)
	{	dbgdraw = new BtOgre::DebugDrawer(
			mSceneMgr->getRootSceneNode(),
			pGame->collision.world);
		pGame->collision.world->setDebugDrawer(dbgdraw);
		pGame->collision.world->getDebugDrawer()->setDebugMode(
			1 /*0xfe/*8+(1<<13)*/);
	}
	
	//  load
	if (pSet->autostart)
		NewGame();
}


//---------------------------------------------------------------------------------------------------------------
///  New Game
//---------------------------------------------------------------------------------------------------------------
void App::NewGame()
{
	//  actual loading isn't done here
	isFocGui = false;
	gui->toggleGui(false);  // hide gui
	mWndNetEnd->setVisible(false);
 
	bLoading = true;  iLoad1stFrames = 0;
	carIdWin = 1;  iRplCarOfs = 0;

	//  wait until sim finishes
	while (bSimulating)
		boost::this_thread::sleep(boost::posix_time::milliseconds(pSet->thread_sleep));

	bRplPlay = 0;
	pSet->rpl_rec = bRplRec;  // changed only at new game
	gui->pChall = 0;
	
	if (!newGameRpl)  // if from replay, dont
	{
		pSet->game = pSet->gui;  // copy game config from gui
		Ch_NewGame();

		if (mClient && mLobbyState != HOSTING)  // all but host
			gui->updateGameSet();  // override gameset params for networked game (from host gameset)
		if (mClient)  // for all, including host
			pSet->game.local_players = 1;
	}
	newGameRpl = false;

	///  check if track exist ..
	if (!PATHMANAGER::FileExists(gcom->TrkDir()+"scene.xml"))
	{
		bLoading = false;  //iLoad1stFrames = -2;
		gui->BackFromChs();
		//toggleGui(true);  // show gui
		Message::createMessageBox("Message", TR("#{Track}"),
			TR("#{TrackNotFound}")+"\n"+pSet->game.track+(pSet->game.track_user?" *user*":"")+"\nPath: "+gcom->TrkDir(),
			MessageBoxStyle::IconError | MessageBoxStyle::Ok);
		//todo: gui is stuck..
		return;
	}	
	if (mWndRpl)  mWndRpl->setVisible(false);  // hide rpl ctrl

	LoadingOn();
	hud->Show(true);  // hide HUD
	//mFpsOverlay->hide();  // hide FPS
	hideMouse();

	curLoadState = 0;
}

/* *  Loading steps (in this order)  * */
//---------------------------------------------------------------------------------------------------------------

void App::LoadCleanUp()  // 1 first
{
	updMouse();
	
	DestroyFluids();  DestroyObjects(true);
	
	DestroyGraphs();  hud->Destroy();
	

	// rem old track
	if (resTrk != "")  Ogre::Root::getSingletonPtr()->removeResourceLocation(resTrk);
	resTrk = gcom->TrkDir() + "objects";
	mRoot->addResourceLocation(resTrk, "FileSystem");
	
	//  Delete all cars
	for (int i=0; i < carModels.size(); i++)
	{
		CarModel* c = carModels[i];
		if (c && c->fCam)
		{
			carsCamNum[i] = c->fCam->miCurrent +1;  // save which cam view
			if (i < 4)
				pSet->cam_view[i] = carsCamNum[i];
		}
		if (c->pNickTxt)  {  mGui->destroyWidget(c->pNickTxt);  c->pNickTxt = 0;  }
		delete c;
	}
	carModels.clear();  //carPoses.clear();

	if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }

	//  rain/snow
	if (pr)  {  mSceneMgr->destroyParticleSystem(pr);   pr=0;  }
	if (pr2) {  mSceneMgr->destroyParticleSystem(pr2);  pr2=0;  }

	terrain = 0;
	if (mTerrainGroup)
		mTerrainGroup->removeAllTerrains();
	if (road)
	{	road->DestroyRoad();  delete road;  road = 0;  }


	///  destroy all  TODO ...
	///!  remove this and destroy everything with* manually  destroyCar, destroyScene, destroyHud
	///!  check if scene (track), car changed, omit creating the same if not
	//mSceneMgr->getRootSceneNode()->removeAndDestroyAllChildren();  // destroy all scenenodes
	mSceneMgr->destroyAllManualObjects();
	mSceneMgr->destroyAllEntities();
	mSceneMgr->destroyAllStaticGeometry();
	mStaticGeom = 0;
	//mSceneMgr->destroyAllParticleSystems();
	mSceneMgr->destroyAllRibbonTrails();
	mSplitMgr->mGuiSceneMgr->destroyAllManualObjects(); // !?..

	// remove junk from previous tracks
	Ogre::MeshManager::getSingleton().unloadUnreferencedResources();
	sh::Factory::getInstance().unloadUnreferencedMaterials();
	Ogre::TextureManager::getSingleton().unloadUnreferencedResources();
}


void App::LoadGame()  // 2
{
	//  viewports
	int numRplViews = std::max(1, std::min( replay.header.numPlayers, pSet->rpl_numViews ));
	mSplitMgr->mNumViewports = bRplPlay ? numRplViews : pSet->game.local_players;  // set num players
	mSplitMgr->Align();
	mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumViewports);
	
	pGame->NewGameDoCleanup();

	if (gui->bReloadSim)
	{	gui->bReloadSim = false;
		pGame->ReloadSimData();
	}
	
	//  load scene.xml - default if not found
	//  need to know sc->asphalt before vdrift car load
	bool vdr = IsVdrTrack();
	sc->pGame = pGame;
	sc->LoadXml(gcom->TrkDir()+"scene.xml", !vdr/*for asphalt*/);
	sc->vdr = vdr;
	pGame->track.asphalt = sc->asphalt;  //*
	pGame->track.sDefaultTire = sc->asphalt ? "asphalt" : "gravel";  //*

	pGame->NewGameDoLoadTrack();

	if (!sc->ter)
	{	sc->td.hfHeight = sc->td.hfAngle = NULL;  }  // sc->td.layerRoad.smoke = 1.f;
	
	// upd car abs,tcs,sss
	pGame->ProcessNewSettings();

		
	///  init car models
	///--------------------------------------------
	//  will create vdrift cars, actual car loading will be done later in LoadCar()
	//  this is just here because vdrift car has to be created first
	std::list<Camera*>::iterator camIt = mSplitMgr->mCameras.begin();
	
	int numCars = mClient ? mClient->getPeerCount()+1 : pSet->game.local_players;  // networked or splitscreen
	int i;
	for (i = 0; i < numCars; ++i)
	{
		// TODO: This only handles one local player
		CarModel::eCarType et = CarModel::CT_LOCAL;
		int startId = i;
		std::string carName = pSet->game.car[i], nick = "";
		if (mClient)
		{
			// FIXME: Various places assume carModels[0] is local
			// so we swap 0 and local's id but preserve starting position
			if (i == 0)  startId = mClient->getId();
			else  et = CarModel::CT_REMOTE;

			if (i == mClient->getId())  startId = 0;
			if (i != 0)  carName = mClient->getPeer(startId).car;

			//  get nick name
			if (i == 0)  nick = pSet->nickname;
			else  nick = mClient->getPeer(startId).name;
		}
		Camera* cam = 0;
		if (et == CarModel::CT_LOCAL && camIt != mSplitMgr->mCameras.end())
		{	cam = *camIt;  ++camIt;  }
		
		CarModel* car = new CarModel(i, i, et, carName, mSceneMgr, pSet, pGame, sc, cam, this);
		car->Load(startId);
		carModels.push_back(car);
		
		if (nick != "")  // set remote nickname
		{	car->sDispName = nick;
			if (i != 0)  // not for local
				car->pNickTxt = hud->CreateNickText(i, car->sDispName);
		}
	}

	///  ghost car - last in carModels
	///--------------------------------------------
	ghplay.Clear();
	if (!bRplPlay/*|| pSet->rpl_show_ghost)*/ && pSet->rpl_ghost && !mClient)
	{
		std::string ghCar = pSet->game.car[0], orgCar = ghCar;
		ghplay.LoadFile(gui->GetGhostFile(pSet->rpl_ghostother ? &ghCar : 0));
		isGhost2nd = ghCar != orgCar;
		
		//  always because ghplay can appear during play after best lap
		// 1st ghost = orgCar
		CarModel* c = new CarModel(i, 4, CarModel::CT_GHOST, orgCar, mSceneMgr, pSet, pGame, sc, 0, this);
		c->Load();
		c->pCar = (*carModels.begin())->pCar;  // based on 1st car
		carModels.push_back(c);

		//  2st ghost - other car
		if (isGhost2nd)
		{
			CarModel* c = new CarModel(i, 4, CarModel::CT_GHOST2, ghCar, mSceneMgr, pSet, pGame, sc, 0, this);
			c->Load();
			c->pCar = (*carModels.begin())->pCar;
			carModels.push_back(c);
		}
	}
	///  track's ghost  . . .
	///--------------------------------------------
	ghtrk.Clear();  vTimeAtChks.clear();
	bool deny = gui->pChall && !gui->pChall->trk_ghost;
	if (!bRplPlay /*&& pSet->rpl_trackghost?*/ && !mClient && !pSet->game.track_user && !deny)
	if (!pSet->game.trackreverse)  // only not rev, todo..
	{
		std::string file = PATHMANAGER::TrkGhosts()+"/"+pSet->game.track+".gho";
		if (ghtrk.LoadFile(file))
		{
			CarModel* c = new CarModel(i, 5, CarModel::CT_TRACK, "ES", mSceneMgr, pSet, pGame, sc, 0, this);
			c->Load();
			c->pCar = (*carModels.begin())->pCar;  // based on 1st car
			carModels.push_back(c);
	}	}
	
	float pretime = mClient ? 2.0f : pSet->game.pre_time;  // same for all multi players
	if (bRplPlay)  pretime = 0.f;
	if (mClient)  pGame->timer.waiting = true;  //+
	
	pGame->NewGameDoLoadMisc(pretime);
}

void App::LoadScene()  // 3
{

	//  water RTT
	UpdateWaterRTT(mSplitMgr->mCameras.front());

	/// generate materials
	refreshCompositor();

	//  fluids
	CreateFluids();


	//  set sky tex name for water
	sh::MaterialInstance* m = mFactory->getMaterialInstance(sc->skyMtr);
	std::string skyTex = sh::retrieveValue<sh::StringValue>(m->getProperty("texture"), 0).get();
	sh::Factory::getInstance().setTextureAlias("SkyReflection", skyTex);
	

	//  weather rain,snow  -----
	if (!pr && sc->rainEmit > 0)
	{	pr = mSceneMgr->createParticleSystem("Rain", sc->rainName);
		pr->setVisibilityFlags(RV_Particles);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr);
		pr->setRenderQueueGroup(RQG_Weather);
		pr->getEmitter(0)->setEmissionRate(0);
	}
	if (!pr2 && sc->rain2Emit > 0)
	{	pr2 = mSceneMgr->createParticleSystem("Rain2", sc->rain2Name);
		pr2->setVisibilityFlags(RV_Particles);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr2);
		pr2->setRenderQueueGroup(RQG_Weather);
		pr2->getEmitter(0)->setEmissionRate(0);
	}
		
	//  checkpoint arrow
	bool deny = gui->pChall && !gui->pChall->chk_arr;
	if (!bRplPlay && !deny)
		hud->CreateArrow();
}

void App::LoadCar()  // 4
{
	//  Create all cars
	for (int i=0; i < carModels.size(); ++i)
	{
		CarModel* c = carModels[i];
		c->Create(i);

		///  challenge off abs,tcs
		if (gui->pChall && c->pCar)
		{
			if (!gui->pChall->abs)  c->pCar->dynamics.SetABS(false);
			if (!gui->pChall->tcs)  c->pCar->dynamics.SetTCS(false);
		}

		//  restore which cam view
		if (c->fCam && carsCamNum[i] != 0)
		{
			c->fCam->setCamera(carsCamNum[i] -1);
			
			int visMask = 255;
			visMask = c->fCam->ca->mHideGlass ? RV_MaskAll-RV_CarGlass : RV_MaskAll;
			for (std::list<Viewport*>::iterator it = mSplitMgr->mViewports.begin();
				it != mSplitMgr->mViewports.end(); ++it)
				(*it)->setVisibilityMask(visMask);
		}
		iCurPoses[i] = 0;
	}
	
	
	///  Init Replay  header, once
	///=================----------------
	if (!bRplPlay)
	{
	replay.InitHeader(pSet->game.track.c_str(), pSet->game.track_user, pSet->game.car[0].c_str(), !bRplPlay);
	replay.header.numPlayers = mClient ? std::min(4, (int)mClient->getPeerCount()+1) : pSet->game.local_players;  // networked or splitscreen
	replay.header.hue[0] = pSet->game.car_hue[0];  replay.header.sat[0] = pSet->game.car_sat[0];  replay.header.val[0] = pSet->game.car_val[0];
	strcpy(replay.header.nicks[0], carModels[0]->sDispName.c_str());  // player's nick
	replay.header.trees = pSet->game.trees;
	replay.header.networked = mClient ? 1 : 0;
	replay.header.num_laps = pSet->game.num_laps;
	strcpy(replay.header.sim_mode, pSet->game.sim_mode.c_str());
	}
	rewind.Clear();

	ghost.InitHeader(pSet->game.track.c_str(), pSet->game.track_user, pSet->game.car[0].c_str(), !bRplPlay);
	ghost.header.numPlayers = 1;  // ghost always 1 car
	ghost.header.hue[0] = pSet->game.car_hue[0];  ghost.header.sat[0] = pSet->game.car_sat[0];  ghost.header.val[0] = pSet->game.car_val[0];
	ghost.header.trees = pSet->game.trees;

	//  fill other cars (names, nicks, colors)
	if (mClient)  // networked
	{
		int cars = std::min(4, (int)mClient->getPeerCount()+1);  // replay has max 4
		for (int p = 1; p < cars; ++p)  // 0 is local car
		{
			CarModel* cm = carModels[p];
			strcpy(replay.header.cars[p-1], cm->sDirname.c_str());
			strcpy(replay.header.nicks[p], cm->sDispName.c_str());
			replay.header.hue[p] = pSet->game.car_hue[p];  replay.header.sat[p] = pSet->game.car_sat[p];  replay.header.val[p] = pSet->game.car_val[p];
		}
	}
	else  // splitscreen
	if (!bRplPlay)
	for (int p = 1; p < pSet->game.local_players; ++p)
	{
		strcpy(replay.header.cars[p-1], pSet->game.car[p].c_str());
		strcpy(replay.header.nicks[p], carModels[p]->sDispName.c_str());
		replay.header.hue[p] = pSet->game.car_hue[p];  replay.header.sat[p] = pSet->game.car_sat[p];  replay.header.val[p] = pSet->game.car_val[p];
	}
	//  set carModel nicks from networked replay
	if (bRplPlay && replay.header.networked)
	{
		for (int p = 0; p < pSet->game.local_players; ++p)
		{
			CarModel* cm = carModels[p];
			cm->sDispName = String(replay.header.nicks[p]);
			cm->pNickTxt = hud->CreateNickText(p, cm->sDispName);
		}
	}

	int c = 0;  // copy wheels R
	for (std::list <CAR>::const_iterator it = pGame->cars.begin(); it != pGame->cars.end(); ++it,++c)
		for (int w=0; w<4; ++w)
			replay.header.whR[c][w] = (*it).GetTireRadius(WHEEL_POSITION(w));
}

void App::LoadTerrain()  // 5
{
	CreateTerrain(false,sc->ter);  // common
	if (sc->ter)
		CreateBltTerrain();
	

	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		(*it)->terrain = terrain;
	
	sh::Factory::getInstance().setTextureAlias("CubeReflection", "ReflectionCube");


	if (sc->vdr)  // vdrift track
	{
		CreateVdrTrack(pSet->game.track, &pGame->track);
		hud->CreateVdrMinimap();
		//CreateRacingLine();  //?-
		//CreateRoadBezier();  //-
	}
}

void App::LoadRoad()  // 6
{
	CreateRoad();
		
	if (road && road->getNumPoints() == 0 && hud->arrow.nodeRot)
		hud->arrow.nodeRot->setVisible(false);  // hide when no road


	///  Run track's ghost
	// to get times at checkpoints
	if (!road || ghtrk.GetTimeLength() < 1.f)  return;
	int ncs = road->mChks.size();
	if (ncs == 0)  return;

	vTimeAtChks.resize(ncs);
	int i,c;  // clear
	for (c=0; c < ncs; ++c)
		vTimeAtChks[c] = 0.f;

	int si = ghtrk.frames.size();
	for (i=0; i < si; ++i)
	{
		const TrackFrame& tf = ghtrk.frames[i];  // car
		for (c=0; c < ncs; ++c)  // test if in any checkpoint
		{
			const CheckSphere& cs = road->mChks[c];
			Vector3 pos(tf.pos[0],tf.pos[2],-tf.pos[1]);
			Real d2 = cs.pos.squaredDistance(pos);
			if (d2 < cs.r2)  // inside
			if (vTimeAtChks[c] == 0.f)
			{	vTimeAtChks[c] = tf.time;
				LogO("Chk "+toStr(c)+" ti "+fToStr(tf.time,1,4));
		}	}
	}
}

void App::LoadObjects()  // 7
{
	CreateObjects();
}

void App::LoadTrees()  // 8
{
	if (sc->ter)
		CreateTrees();
	
		
	//  check for cars inside terrain ___
	if (terrain)
	for (int i=0; i < carModels.size(); ++i)
	{
		CAR* car = carModels[i]->pCar;
		if (car)
		{
			MATHVECTOR<float,3> pos = car->posAtStart;
			Vector3 stPos(pos[0],pos[2],-pos[1]);
			float yt = terrain->getHeightAtWorldPosition(stPos), yd = stPos.y - yt - 0.5f;
			//todo: either sweep test car body, or world->CastRay x4 at wheels -for bridges, pipes
			//pGame->collision.world->;  //car->dynamics.chassis
			if (yd < 0.f)
				pos[2] += -yd + 0.9f;
			car->SetPosition1(pos);
	}	}
}


void App::LoadMisc()  // 9 last
{
	if (pGame && pGame->cars.size() > 0)  //todo: move this into gui track tab chg evt, for cur game type
		gcom->UpdGuiRdStats(road, sc, gcom->sListTrack, pGame->timer.GetBestLap(0, pSet->game.trackreverse));  // current


	hud->Create();
	hud->Show(true);  // hide
	
	// Camera settings
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); ++it)
	{	(*it)->First();
		if ((*it)->fCam)
		{	(*it)->fCam->mTerrain = mTerrainGroup;
			//(*it)->fCam->mWorld = &(pGame->collision);
	}	}
	
	try {
	TexturePtr tex = Ogre::TextureManager::getSingleton().getByName("waterDepth.png");
	if (!tex.isNull())
		tex->reload();
	} catch(...) {  }

	
	/// rendertextures debug
	#if 0
	// init overlay elements
	OverlayManager& mgr = OverlayManager::getSingleton();
	Overlay* overlay;
	// destroy if already exists
	if (overlay = mgr.getByName("DebugOverlay"))
		mgr.destroy(overlay);
	overlay = mgr.create("DebugOverlay");
	//Ogre::CompositorInstance  *compositor= CompositorManager::getSingleton().getCompositorChain(mSplitMgr->mViewports.front())->getCompositor("HDR");
	for (int i=0; i<3; ++i)
	{
		// Set up a debug panel
		if (MaterialManager::getSingleton().resourceExists("Ogre/DebugTexture" + toStr(i)))
			MaterialManager::getSingleton().remove("Ogre/DebugTexture" + toStr(i));
		MaterialPtr debugMat = MaterialManager::getSingleton().create(
			"Ogre/DebugTexture" + toStr(i), 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		debugMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
		//TexturePtr depthTexture = compositor->getTextureInstance("mrt_output",i);
		//TexturePtr depthTexture = compositor->getTextureInstance("rt_bloom0",0);
		TexturePtr depthTexture = mSceneMgr->getShadowTexture(i);
		if(!depthTexture.isNull())
		{
			TextureUnitState *t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState(depthTexture->getName());
			t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
		}
		OverlayContainer* debugPanel;
		// destroy container if exists
		try
		{
			if (debugPanel = 
				static_cast<OverlayContainer*>(
					mgr.getOverlayElement("Ogre/DebugTexPanel" + toStr(i)
				)))
				mgr.destroyOverlayElement(debugPanel);
		}
		catch (Ogre::Exception&) {}
		debugPanel = (OverlayContainer*)
			(OverlayManager::getSingleton().createOverlayElement("Panel", "Ogre/DebugTexPanel" + StringConverter::toString(i)));
		debugPanel->_setPosition(0.67, i*0.33);
		debugPanel->_setDimensions(0.33, 0.33);
		debugPanel->setMaterialName(debugMat->getName());
		debugPanel->show();
		overlay->add2D(debugPanel);
		overlay->show();
	}
	#endif
}


//  Performs a single loading step.  Actual loading procedure that gets called every frame during load.
//---------------------------------------------------------------------------------------------------------------
String App::cStrLoad[LS_ALL+1] = 
	{"LS_CLEANUP","LS_GAME","LS_SCENE","LS_CAR","LS_TER","LS_ROAD","LS_OBJS","LS_TREES","LS_MISC","LS_ALL"};

void App::NewGameDoLoad()
{
	if (curLoadState == LS_ALL)
	{
		// Loading finished
		bLoading = false;
		#ifdef DEBUG  //todo: doesnt hide later, why?
		LoadingOff();
		#endif
		mLoadingBar->SetWidth(100.f);
				
		hud->Show();
		//if (pSet->show_fps)
		//	mFpsOverlay->show();
		//.mSplitMgr->mGuiViewport->setClearEveryFrame(true, FBT_DEPTH);

		//.ChampLoadEnd();
		//boost::this_thread::sleep(boost::posix_time::milliseconds(6000 * mClient->getId())); // Test loading synchronization
		//.bLoadingEnd = true;
		return;
	}
	//  Do the next loading step
	int perc = 0;
	switch (curLoadState)
	{
		case LS_CLEANUP:	LoadCleanUp();	perc = 3;	break;
		case LS_GAME:		LoadGame();		perc = 10;	break;
		case LS_SCENE:		LoadScene();	perc = 20;	break;
		case LS_CAR:		LoadCar();		perc = 30;	break;

		case LS_TERRAIN:	LoadTerrain();	perc = 40;	break;
		case LS_ROAD:		LoadRoad();		perc = 50;	break;
		case LS_OBJECTS:	LoadObjects();	perc = 60;	break;
		case LS_TREES:		LoadTrees();	perc = 70;	break;

		case LS_MISC:		LoadMisc();		perc = 80;	break;
	}

	//  Update bar,txt
	txLoad->setCaption(TR("#{"+cStrLoad[curLoadState]+"}"));
	mLoadingBar->SetWidth(perc);

	//  next loading step
	++curLoadState;
}


//---------------------------------------------------------------------------------------------------------------
///  Road  * * * * * * * 
//---------------------------------------------------------------------------------------------------------------

void App::CreateRoad()
{
	///  road  ~ ~ ~
	if (road)
	{	road->DestroyRoad();  delete road;  road = 0;  }

	road = new SplineRoad(pGame);  // sphere.mesh
	road->Setup("", 0.7,  terrain, mSceneMgr, *mSplitMgr->mCameras.begin());
	
	String sr = gcom->TrkDir()+"road.xml";
	road->LoadFile(gcom->TrkDir()+"road.xml");
	
	//  after road load we have iChk1 so set it for carModels
	for (int i=0; i < carModels.size(); ++i)
		carModels[i]->ResetChecks(true);

	UpdPSSMMaterials();  ///+~-

	road->bCastShadow = pSet->shadow_type >= Sh_Depth;
	road->bRoadWFullCol = pSet->gui.collis_roadw;
	road->RebuildRoadInt();
}
