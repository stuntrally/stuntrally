#include "stdafx.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "FollowCamera.h"
#include "../road/Road.h"

#include "../btOgre/BtOgrePG.h"
#include "../btOgre/BtOgreGP.h"

#include "boost/thread.hpp"


//  Create Scene
//-------------------------------------------------------------------------------------
void App::createScene()
{
	//  tex fil
	MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	MaterialManager::getSingleton().setDefaultAnisotropy(pSet->anisotropy);

	mRoot->addResourceLocation(pathTrk[1] + "_previews/", "FileSystem");  //prv user tracks
	
	if (!pSet->autostart)  isFocGui = true;
	InitGui();

    //  bullet Debug drawing
    //------------------------------------
    if (pSet->bltLines)
	{	dbgdraw = new BtOgre::DebugDrawer(
			mSceneMgr->getRootSceneNode(),
			&pGame->collision.world);
		pGame->collision.world.setDebugDrawer(dbgdraw);
		pGame->collision.world.getDebugDrawer()->setDebugMode(
			1 /*0xfe/*8+(1<<13)*/);
	}
	
	objs.LoadXml();
	Log(string("**** Loaded Vegetation objects: ") + toStr(objs.colsMap.size()));
	Log(string("**** ReplayFrame size: ") + toStr(sizeof(ReplayFrame)));	

	///  load replay
	if (pSet->rpl_play)
		pSet->rpl_play = 0;
	/*{
		string file = PATHMANAGER::GetReplayPath() + "/" + pSet->track + ".rpl";
		replay.LoadFile(file);
	}
	/**/

	if (pSet->autostart)
		NewGame();
}


//---------------------------------------------------------------------------------------------------------------
///  New Game
//---------------------------------------------------------------------------------------------------------------
void App::NewGame()
{
	// actual loading isn't done here
	bLoading = true;
	LoadingOn();
	// hide HUD
	ShowHUD(true);
	// hide FPS
	mFpsOverlay->hide();
	if (mWndRpl)  mWndRpl->setVisible(false);
	mGUI->setVisiblePointer(false);

	currentLoadingState = loadingStates.begin();
}

/* Loading steps (in this order) */
void App::LoadCleanUp()
{
	// rem old track
	if (resTrk != "")  Ogre::Root::getSingletonPtr()->removeResourceLocation(resTrk);
	resTrk = TrkDir() + "objects";
	
	// Delete all cars
	for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
	{
		delete (*it);
	}
	carModels.clear();
	newPosInfos.clear();
	
	//  hide trails
	///TODO
	/*for (int w=0; w<4; ++w)  if (whTrl[w])  {	wht[w] = 0.f;
		whTrl[w]->setVisible(false);	whTrl[w]->setInitialColour(0, 0.5,0.5,0.5, 0);	}*/
	
	if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }

	//  destroy all
	mSceneMgr->destroyAllManualObjects();
	mSceneMgr->destroyAllEntities();
	mSceneMgr->destroyAllStaticGeometry();

	//  rain/snow
	if (pr)  {  mSceneMgr->destroyParticleSystem(pr);   pr=0;  }
	if (pr2) {  mSceneMgr->destroyParticleSystem(pr2);  pr2=0;  }

	terrain = 0;
	if (mTerrainGroup)
		mTerrainGroup->removeAllTerrains();
	if (road)
	{	road->DestroyRoad();  delete road;  road = 0;  }
}
void App::LoadGame()
{
	//pGame->NewGame();  // ? timer 0 after ?
	pGame->NewGameDoCleanup();
	pGame->NewGameDoLoadTrack();
	/// init car models
	// will create vdrift cars
	// actual car loading will be done later in LoadCar()
	// this is just here because vdrift car has to be created first
	std::list<Camera*>::iterator camIt = mSplitMgr->mCameras.begin();
	for (int i=0; i<pSet->local_players; i++)
	{
		carModels.push_back( new CarModel(i, CarModel::CT_LOCAL, pSet->car/*sListCar*/, mSceneMgr, pSet, pGame, &sc, (*camIt) ) );
		camIt++;
	}
	
	pGame->NewGameDoLoadMisc();
	bGetStPos = true;
	
	bool ter = IsTerTrack();
	sc.ter = ter;
}

void App::LoadScene()
{
	bool ter = IsTerTrack();
	if (ter)  // load scene
		sc.LoadXml(TrkDir()+"scene.xml");
	else
	{	sc.Default();  sc.td.hfData = NULL;  }	
	
	//  rain  -----
	if (!pr && sc.rainEmit > 0)  {
		pr = mSceneMgr->createParticleSystem("Rain", sc.rainName);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr);
		pr->setRenderQueueGroup(RENDER_QUEUE_9+5);
		pr->getEmitter(0)->setEmissionRate(0);  }
	//  rain2  =====
	if (!pr2 && sc.rain2Emit > 0)  {
		pr2 = mSceneMgr->createParticleSystem("Rain2", sc.rain2Name);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pr2);
		pr2->setRenderQueueGroup(RENDER_QUEUE_9+5);
		pr2->getEmitter(0)->setEmissionRate(0);  }
}
void App::LoadCar()
{
	// Create all cars
	for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
	{
		(*it)->Create();
		// Reserve an entry in newPosInfos
		PosInfo carPosInfo;
		newPosInfos.push_back(carPosInfo);
	}
	
	//  init replay  once  ---------------------------------
	float whR[4] = {0.3f,0.3f,0.3f,0.3f};
	if (pGame->cars.size() > 0)
	{
		CAR* pCar = &(*pGame->cars.begin());
		for (int w=0; w<4; ++w)
			whR[w] = pCar->GetTireRadius(WHEEL_POSITION(w));
	}
	replay.InitHeader(pSet->track.c_str(), pSet->track_user, pSet->car.c_str(), whR);
}
void App::LoadTerrain()
{
	bool ter = IsTerTrack();
	CreateTerrain(false,ter);  // common
	
	// Assign stuff to cars
	for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
	{
		(*it)->terrain = terrain;
		(*it)->blendMtr = blendMtr;
		(*it)->blendMapSize = blendMapSize;
	}
}
void App::LoadTrack()
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
		//CreateProps();  //-
		CreateRoad();
		CreateTrees();
	}
}
void App::LoadMisc()
{
	UpdGuiRdStats(road, sc, pGame->timer.GetBestLap(pSet->trackreverse));  // current
	CreateHUD();
	// immediately hide it
	ShowHUD(true);
	/*mFCam->first = true;  // no smooth	
	mFCam->mTerrain = mTerrainGroup; // assign terrain to cam*/
	
	// Camera settings
	for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
	{
		if ((*it)->fCam)
		{
			(*it)->fCam->first = true;
			(*it)->fCam->mTerrain = mTerrainGroup;
		}
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
	mLoadingBar.mLoadingCommentElement->setCaption( (*currentLoadingState).second );
	
	// Set %
	mLoadingBar.mLoadingBarElement->setWidth( mLoadingBar.mProgressBarMaxSize * (perc/100.0) );

	// Go to next loading step.
	currentLoadingState++;
}

//---------------------------------------------------------------------------------------------------------------
bool App::IsTerTrack()
{
	//  track: vdrift / terrain
	String sr = TrkDir()+"road.xml";
	ifstream fr(sr.c_str());
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
	///TODO road only has correct LOD for camera 1
	/// make separate road meshes for every camera???
	road->Setup("", 0.7,  terrain, mSceneMgr, *mSplitMgr->mCameras.begin());
	
	String sr = TrkDir()+"road.xml";
	road->LoadFile(TrkDir()+"road.xml");

	UpdPSSMMaterials();  ///+~-
}


///  props  ... .. . . .
//------------------------------------------------------------------------------

void App::CreateProps()
{
	/// . dyn objs +
	if (0)
	for (int j=-1; j<1; j++)
	for (int i=-1; i<1; i++)
	{
		btCollisionShape* shape;
		// switch(rand() % 5)
		switch( (50+i+j*3) % 6)
		{
		case 0:  shape = new btBoxShape(btVector3(0.4,0.3,0.5));  break;
		case 1:  shape = new btSphereShape(0.5);  break;
		case 2:  shape = new btCapsuleShapeZ(0.4,0.5);  break;
		case 3:  shape = new btCylinderShapeX(btVector3(0.5,0.7,0.4));  break;
		case 4:  shape = new btCylinderShapeZ(btVector3(0.5,0.6,0.7));  break;
		case 5:  shape = new btConeShapeX(0.4,0.6);  break;
		}

		btTransform tr(btQuaternion(0,0,0), btVector3(-5+i*2,5+j*2,3));
		btDefaultMotionState * ms = new btDefaultMotionState();
		ms->setWorldTransform(tr);

		btRigidBody::btRigidBodyConstructionInfo ci(320, ms, shape, btVector3(21,21,21));
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
			pGame->collision.world.addRigidBody(bdy);
		}
	}
}
