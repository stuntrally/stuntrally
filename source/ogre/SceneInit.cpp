#include "pch.h"
#include "Defines.h"
#include "OgreGame.h"
#include "LoadingBar.h"
#include "../vdrift/game.h"
#include "FollowCamera.h"
#include "../road/Road.h"
#include "SplitScreen.h"
#include "common/RenderConst.h"
#include "common/MaterialFactory.h"

#include "../btOgre/BtOgrePG.h"
#include "../btOgre/BtOgreGP.h"

#include "boost/thread.hpp"
#include "../paged-geom/PagedGeometry.h"

#include <MyGUI_OgrePlatform.h>
#include <MyGUI_PointerManager.h>
using namespace MyGUI;
#include <OgreTerrainGroup.h>
using namespace Ogre;


//  Create Scene
//-------------------------------------------------------------------------------------
void App::createScene()
{
	//  tex fil
	MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	MaterialManager::getSingleton().setDefaultAnisotropy(pSet->anisotropy);

	mRoot->addResourceLocation(pathTrk[1] + "_previews/", "FileSystem");  //prv user tracks
	
	//  restore camNums
	for (int i=0; i<4; ++i)
		if (pSet->cam_view[i] >= 0)
			carsCamNum[i] = pSet->cam_view[i];

	QTimer ti;  ti.update();  /// time

	//  tracks.xml
	tracksXml.LoadXml(PATHMANAGER::GetGameConfigDir() + "/tracks.xml");
	//tracksXml.SaveXml(PATHMANAGER::GetGameConfigDir() + "/tracks2.xml");

	//  fluids.xml
	fluidsXml.LoadXml(PATHMANAGER::GetDataPath() + "/materials/fluids.xml");
	sc.pFluidsXml = &fluidsXml;
	LogO(String("**** Loaded fluids.xml: ") + toStr(fluidsXml.fls.size()));

	//  collisions.xml
	objs.LoadXml();
	LogO(String("**** Loaded Vegetation objects: ") + toStr(objs.colsMap.size()));
	LogO(String("**** ReplayFrame size: ") + toStr(sizeof(ReplayFrame)));	
	LogO(String("**** ReplayHeader size: ") + toStr(sizeof(ReplayHeader)));	

	ti.update();  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time load xmls: ") + toStr(dt) + " ms");


	//  gui
	if (!pSet->autostart)  isFocGui = true;
	InitGui();

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
	
	bRplRec = pSet->rpl_rec;  // startup setting

	//  load
	if (pSet->autostart)
		NewGame();
	
	#if 0  // autoload replay
		std::string file = PATHMANAGER::GetReplayPath() + "/S12-Infinity_good_x3.rpl"; //+ pSet->track + ".rpl";
		if (replay.LoadFile(file))
		{
			std::string car = replay.header.car, trk = replay.header.track;
			bool usr = replay.header.track_user == 1;

			pSet->car[0] = car;  pSet->track = trk;  pSet->track_user = usr;
			pSet->car_hue[0] = replay.header.hue[0];  pSet->car_sat[0] = replay.header.sat[0];  pSet->car_val[0] = replay.header.val[0];
			for (int p=1; p < replay.header.numPlayers; ++p)
			{	pSet->car[p] = replay.header.cars[p-1];
				pSet->car_hue[p] = replay.header.hue[p];  pSet->car_sat[p] = replay.header.sat[p];  pSet->car_val[p] = replay.header.val[p];
			}
			btnNewGame(0);
			bRplPlay = 1;
		}
	#endif
}


//---------------------------------------------------------------------------------------------------------------
///  New Game
//---------------------------------------------------------------------------------------------------------------
void App::NewGame()
{
	// actual loading isn't done here
	isFocGui = true;
	toggleGui();  // hide gui

	bLoading = true;
	carIdWin = 1;

	bRplPlay = 0;
	pSet->rpl_rec = bRplRec;  // changed only at new game
	if (mWndRpl)  mWndRpl->setVisible(false);

	LoadingOn();
	ShowHUD(true);  // hide HUD
	mFpsOverlay->hide();  // hide FPS
	PointerManager::getInstance().setVisible(false);

	currentLoadingState = loadingStates.begin();
}

/* *  Loading steps (in this order)  * */

void App::LoadCleanUp()  // 1 first
{
	if (mGUI)	PointerManager::getInstance().setVisible(isFocGui);
	// rem old track
	if (resTrk != "")  Ogre::Root::getSingletonPtr()->removeResourceLocation(resTrk);
	resTrk = TrkDir() + "objects";
	
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
		delete c;
	}
	carModels.clear();  newPosInfos.clear();

	if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }

	///  destroy all  TODO ...
	///!  remove this crap and destroy everything with* manually  destroyCar, destroyScene
	///!  check if scene (track), car, color changed, omit creating the same if not
	//mSceneMgr->getRootSceneNode()->removeAndDestroyAllChildren();  // destroy all scenenodes
	mSceneMgr->destroyAllManualObjects();
	mSceneMgr->destroyAllEntities();
	mSceneMgr->destroyAllStaticGeometry();
	//mSceneMgr->destroyAllParticleSystems();
	mSceneMgr->destroyAllRibbonTrails();
	mSplitMgr->mHUDSceneMgr->destroyAllManualObjects();
	MeshManager::getSingleton().removeAll();  // destroy all meshes

	//  rain/snow
	if (pr)  {  mSceneMgr->destroyParticleSystem(pr);   pr=0;  }
	if (pr2) {  mSceneMgr->destroyParticleSystem(pr2);  pr2=0;  }

	terrain = 0;
	if (mTerrainGroup)
		mTerrainGroup->removeAllTerrains();
	if (road)
	{	road->DestroyRoad();  delete road;  road = 0;  }
}

void App::LoadGame()  // 2
{
	//  viewports
	mSplitMgr->mNumViewports = bRplPlay ? replay.header.numPlayers : pSet->local_players;  // set num players
	mSplitMgr->Align();
	mPlatform->getRenderManagerPtr()->setActiveViewport(mSplitMgr->mNumViewports*2);
	
	pGame->NewGameDoCleanup();
	pGame->NewGameDoLoadTrack();
	
	/// generate materials
	materialFactory->generate();
	
	/// init car models
	// will create vdrift cars, actual car loading will be done later in LoadCar()
	// this is just here because vdrift car has to be created first
	std::list<Camera*>::iterator camIt = mSplitMgr->mCameras.begin();
	int i;
	for (i=0; i < mSplitMgr->mNumViewports; i++,camIt++)
		carModels.push_back( new CarModel(i, CarModel::CT_LOCAL, pSet->car[i], mSceneMgr, pSet, pGame, &sc, (*camIt), this ) );

	/// ghost car  load if exists
	ghplay.Clear();
	if (!bRplPlay && pSet->rpl_ghost)  // load ghost play if exists
	{
		/*if (*/ghplay.LoadFile(GetGhostFile());
		//  always because ghplay can appear during play after best lap
		CarModel* c = new CarModel(i, CarModel::CT_GHOST, pSet->car[0], mSceneMgr, pSet, pGame, &sc, 0, this );
		c->pCar = (*carModels.begin())->pCar;  // based on 1st car
		carModels.push_back(c);
	}
	
	pGame->NewGameDoLoadMisc();
	bool ter = IsTerTrack();
	sc.ter = ter;
}

void App::LoadScene()  // 3
{
	bool ter = IsTerTrack();
	if (ter)  // load scene
	{
		sc.LoadXml(TrkDir()+"scene.xml");
		pSet->sceneryIdOld = sceneryId;
		sceneryId = sc.sceneryId;
	}
	else
	{	sc.Default();  sc.td.hfHeight = NULL;  sc.td.hfAngle = NULL;  }
	
	CreateFluids();

	//  rain  -----
	if (!pr && sc.rainEmit > 0)  {
		pr = mSceneMgr->createParticleSystem("Rain", sc.rainName);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr);
		pr->setRenderQueueGroup(RQG_Weather);
		pr->getEmitter(0)->setEmissionRate(0);  }
	//  rain2  =====
	if (!pr2 && sc.rain2Emit > 0)  {
		pr2 = mSceneMgr->createParticleSystem("Rain2", sc.rain2Name);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr2);
		pr2->setRenderQueueGroup(RQG_Weather);
		pr2->getEmitter(0)->setEmissionRate(0);  }
		
	//  checkpoint arrow
	if (/*pSet->check_arrow &&*/ !bRplPlay)  { //!
		if (!arrowNode) arrowNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		Ogre::Entity* arrowEnt = mSceneMgr->createEntity("CheckpointArrow", "arrow.mesh");
		arrowEnt->setRenderQueueGroup(RQG_Hud3);
		arrowEnt->setCastShadows(false);
		arrowRotNode = arrowNode->createChildSceneNode();
		arrowRotNode->attachObject(arrowEnt);
		arrowRotNode->setScale(pSet->size_arrow/2.f, pSet->size_arrow/2.f, pSet->size_arrow/2.f);
		arrowEnt->setVisibilityFlags(RV_Car); // hide in reflection
		arrowRotNode->setVisible(pSet->check_arrow); //!
	}
}

void App::LoadCar()  // 4
{
	//  Create all cars
	for (int i=0; i < carModels.size(); ++i)
	{
		CarModel* c = carModels[i];
		c->Create(i);

		//  restore which cam view
		if (c->fCam && carsCamNum[i] != 0)
			c->fCam->setCamera(carsCamNum[i] -1);

		//  Reserve an entry in newPosInfos
		PosInfo carPosInfo;  carPosInfo.bNew = false;  //-
		newPosInfos.push_back(carPosInfo);
	}
	
	///  Init Replay  once
	///=================----------------
	replay.InitHeader(pSet->track.c_str(), pSet->track_user, pSet->car[0].c_str(), !bRplPlay);
	replay.header.numPlayers = pSet->local_players;
	replay.header.hue[0] = pSet->car_hue[0];  replay.header.sat[0] = pSet->car_sat[0];  replay.header.val[0] = pSet->car_val[0];

	ghost.InitHeader(pSet->track.c_str(), pSet->track_user, pSet->car[0].c_str(), !bRplPlay);
	ghost.header.numPlayers = 1;  // ghost always 1 car
	ghost.header.hue[0] = pSet->car_hue[0];  ghost.header.sat[0] = pSet->car_sat[0];  ghost.header.val[0] = pSet->car_val[0];

	//if (pSet->local_players > 1)  // save other car names
	for (int p=1; p < pSet->local_players; ++p)
	{	strcpy(replay.header.cars[p-1], pSet->car[p].c_str());
		replay.header.hue[p] = pSet->car_hue[p];  replay.header.sat[p] = pSet->car_sat[p];
		replay.header.val[p] = pSet->car_val[p];  }
	
	int c = 0;  // copy wheels R
	for (std::list <CAR>::const_iterator it = pGame->cars.begin(); it != pGame->cars.end(); it++,c++)
		for (int w=0; w<4; ++w)
			replay.header.whR[c][w] = (*it).GetTireRadius(WHEEL_POSITION(w));
}

void App::LoadTerrain()  // 5
{
	bool ter = IsTerTrack();
	CreateTerrain(false,ter);  // common
	
	// Assign stuff to cars
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
	{
		(*it)->terrain = terrain;
		(*it)->blendMtr = blendMtr;
		(*it)->blendMapSize = blendMapSize;
	}
}

void App::LoadTrack()  // 6
{
	mRoot->addResourceLocation(resTrk, "FileSystem");

	bool ter = IsTerTrack();
	if (!ter)	//  track
	{
		CreateTrack();
		CreateMinimap();
		//CreateRacingLine();  //?-
		//CreateRoadBezier();  //-
	}
	if (ter)	//  Terrain
	{
		CreateBltTerrain();
		CreateProps();  //-
		CreateRoad();
		CreateTrees();
	}
}

void App::LoadMisc()  // 7 last
{
	if (pGame && pGame->cars.size() > 0)
		UpdGuiRdStats(road, sc, pGame->timer.GetBestLap(pSet->trackreverse));  // current

	CreateHUD();
	// immediately hide it
	ShowHUD(true);
	
	// Camera settings
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		if ((*it)->fCam)
		{	(*it)->fCam->first = true;
			(*it)->fCam->mTerrain = mTerrainGroup;
			#if 0
			(*it)->fCam->mWorld = &(pGame->collision);
			#endif
		}
}

/* Actual loading procedure that gets called every frame during load. Performs a single loading step. */
void App::NewGameDoLoad()
{	
	if (currentLoadingState == loadingStates.end())
	{
		// Loading finished.
		bLoading = false;
		LoadingOff();
		ShowHUD();
		if (pSet->show_fps)
			mFpsOverlay->show();
		mSplitMgr->mGuiViewport->setClearEveryFrame(true, FBT_DEPTH);
		return;
	}
	// Do the next loading step.
	unsigned int perc = 0;
	switch ( (*currentLoadingState).first )
	{
		case LS_CLEANUP:	LoadCleanUp();	perc = 3;	break;
		case LS_GAME:		LoadGame();		perc = 10;	break;
		case LS_SCENE:		LoadScene();	perc = 20;	break;
		case LS_CAR:		LoadCar();		perc = 45;	break;
		case LS_TER:		LoadTerrain();	perc = 70;	break;
		case LS_TRACK:		LoadTrack();	perc = 75;	break;
		case LS_MISC:		LoadMisc();		perc = 80;	break;
	}

	// Update label.
	mLoadingBar->mLoadingCommentElement->setCaption( (*currentLoadingState).second );
	
	// Set %
	mLoadingBar->mLoadingBarElement->setWidth( mLoadingBar->mProgressBarMaxSize * (perc/100.0) );

	// Go to next loading step.
	currentLoadingState++;
}

//---------------------------------------------------------------------------------------------------------------
bool App::IsTerTrack()
{
	//  track: vdrift / terrain
	String sr = TrkDir()+"road.xml";
	std::ifstream fr(sr.c_str());
	bool ter = fr.good(); //!fail()
	if (ter)  fr.close();
	return ter;
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
	road->iTexSize = pSet->tex_size;
	road->bForceShadowCaster = (pSet->shadow_type == 3);
	
	String sr = TrkDir()+"road.xml";
	road->LoadFile(TrkDir()+"road.xml");
	
	//  after road load we have iChk1 so set it for carModels
	for (int i=0; i < carModels.size(); ++i)
		carModels[i]->ResetChecks();

	UpdPSSMMaterials();  ///+~-
}


///  props  ... .. . . .
//------------------------------------------------------------------------------

void App::CreateProps()
{
	/// . dyn objs +
	if (0)
	for (int j=-2; j<1; j++)
	for (int i=-2; i<1; i++)
	{
		btCollisionShape* shape;
		btScalar s = Ogre::Math::RangeRandom(1,3);
		// switch(rand() % 5)
		switch( (50+i+j*3) % 6)
		{
		case 0:  shape = new btBoxShape(s*btVector3(0.4,0.3,0.5));  break;
		case 1:  shape = new btSphereShape(s*0.5);  break;
		case 2:  shape = new btCapsuleShapeZ(s*0.4,s*0.5);  break;
		case 3:  shape = new btCylinderShapeX(s*btVector3(0.5,0.7,0.4));  break;
		case 4:  shape = new btCylinderShapeZ(s*btVector3(0.5,0.6,0.7));  break;
		case 5:  shape = new btConeShapeX(s*0.4,s*0.6);  break;
		}

		btTransform tr(btQuaternion(0,0,0), btVector3(-5+i*5 -20, 5+j*5 -25,0));
		btDefaultMotionState * ms = new btDefaultMotionState();
		ms->setWorldTransform(tr);

		btRigidBody::btRigidBodyConstructionInfo ci(220*s+rand()%500, ms, shape, s*21*btVector3(1,1,1));
		ci.m_restitution = 0.9;
		ci.m_friction = 0.9;
		ci.m_linearDamping = 0.4;
		pGame->collision.AddRigidBody(ci);
	}
	
	//.  props
	if (0)  {
		String sn[9] = {"garage_stand", "indicator", "stop_sign",
			"Cone1", "Barrel1", "2x4_1", "concrete1", "crate1", "Dumpster1"}; //plywood1
			// "cube.1m.smooth.mesh", "capsule.50cmx1m.mesh", "sphere.mesh"
		int a = 0;
		for (int j=-1; j<1; j++)
		for (int i=-1; i<1; i++)
		{
			Vector3 pos = Vector3(-2+i*2,-1,-2+j*2);
			Quaternion rot = Quaternion::IDENTITY;
			//int a = (100 + j*10 + i) % 9; //rand() % 9;

			// Ogre stuff
			String s = StringConverter::toString(j*10+i);
			Entity* ent = mSceneMgr->createEntity(
				"Ent"+s, sn[a % 9] + ".mesh");  a++;
			SceneNode* nod = mSceneMgr->getRootSceneNode()->createChildSceneNode(
				"Node"+s, pos, rot);
			nod->attachObject(ent);

			// Shape
			BtOgre::StaticMeshToShapeConverter converter(ent);
			btCollisionShape* shp = converter.createBox();  // createSphere();

			// calculate inertia
			btScalar mass = 5;  btVector3 inertia;
			shp->calculateLocalInertia(mass, inertia);

			// BtOgre MotionState (connects Ogre and Bullet)
			BtOgre::RigidBodyState *stt = new BtOgre::RigidBodyState(nod);

			// Body
			btRigidBody* bdy = new btRigidBody(mass, stt, shp, inertia);
			pGame->collision.world->addRigidBody(bdy);
		}
	}
}

/*void App::ReloadCar()
{
	// Delete all cars
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		delete (*it);

	carModels.clear();
	newPosInfos.clear();

	/// init car models
	// will create vdrift cars, actual car loading will be done later in LoadCar()
	// this is just here because vdrift car has to be created first
	std::list<Camera*>::iterator camIt = mSplitMgr->mCameras.begin();
	int i;
	for (i=0; i < mSplitMgr->mNumViewports; i++,camIt++)
		carModels.push_back( new CarModel(i, CarModel::CT_LOCAL, pSet->car[i], mSceneMgr, pSet, pGame, &sc, (*camIt), this ) );

	//  Create all cars
	for (int i=0; i < carModels.size(); ++i)
	{
		CarModel* c = carModels[i];
		c->Create(i);

		//  restore which cam view
		if (c->fCam && carsCamNum[i] != 0)
			c->fCam->setCamera(carsCamNum[i] -1);

		//  Reserve an entry in newPosInfos
		PosInfo carPosInfo;  carPosInfo.bNew = false;  //-
		newPosInfos.push_back(carPosInfo);
	}

	// Assign stuff to cars
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
	{
		(*it)->terrain = terrain;
		(*it)->blendMtr = blendMtr;
		(*it)->blendMapSize = blendMapSize;
	}

	// Camera settings
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		if ((*it)->fCam)
		{	(*it)->fCam->first = true;
			(*it)->fCam->mTerrain = mTerrainGroup;
			#if 0
			(*it)->fCam->mWorld = &(pGame->collision);
			#endif
		}
}*/
