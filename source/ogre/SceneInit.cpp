#include "stdafx.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "FollowCamera.h"
#include "../road/Road.h"

#include "../btOgre/BtOgrePG.h"
#include "../btOgre/BtOgreGP.h"


//  Create Scene
//-------------------------------------------------------------------------------------
void App::createScene()
{
	mCamera->setFarClipDistance(pSet->view_distance*1.1f);
	mCamera->setNearClipDistance(0.2f);

	//  tex fil
	MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	MaterialManager::getSingleton().setDefaultAnisotropy(pSet->anisotropy);

	//  --------  Follow Camera  --------
	mFCam = new FollowCamera(mCamera);  mFCam->loadCameras();
	
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

	createReflectCams();  ///*
	
	if (pSet->autostart)
		NewGame();
}


//---------------------------------------------------------------------------------------------------------------
///  New Game
//---------------------------------------------------------------------------------------------------------------
void App::NewGame()
{
	bLoading = true;

	//  hide trails
	for (int w=0; w<4; ++w)  if (whTrl[w])  {	wht[w] = 0.f;
		whTrl[w]->setVisible(false);	whTrl[w]->setInitialColour(0, 0.5,0.5,0.5, 0);	}
	
	if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }

	//  destroy all
	mSceneMgr->destroyAllManualObjects();
	mSceneMgr->destroyAllEntities();
	mSceneMgr->destroyAllStaticGeometry();

	//  par sys
	if (pr)  {  mSceneMgr->destroyParticleSystem(pr);   pr=0;  }
	if (pr2) {  mSceneMgr->destroyParticleSystem(pr2);  pr2=0;  }
	for (int w=0; w < 4; w++)  {
		if (ps[w]) {  mSceneMgr->destroyParticleSystem(ps[w]);   ps[w]=0;  }
		if (pm[w]) {  mSceneMgr->destroyParticleSystem(pm[w]);   pm[w]=0;  }
		if (pd[w]) {  mSceneMgr->destroyParticleSystem(pd[w]);   pd[w]=0;  }  }

	terrain = 0;
	if (mTerrainGroup)
		mTerrainGroup->removeAllTerrains();
	if (road)
	{	road->DestroyRoad();  delete road;  road = 0;  }


	///*
	pGame->NewGame();
	bGetStPos = true;
	
	bool ter = IsTerTrack();
	sc.ter = ter;

	if (ter)  // load scene
		sc.LoadXml(PATHMANAGER::GetTrackPath() + "/" + pSet->track + "/scene.xml");
	else
	{	sc.Default();  sc.td.hfData = NULL;  }

	CreateCar();  // par rain
	
	CreateTerrain(false,ter);  // common

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

	UpdGuiRdStats(road, sc, pGame->timer.GetBestLap(pSet->trackreverse));  // current
	CreateHUD();
	miReflectCntr = 5;  //.
	mReflAll1st = true;
	mFCam->first = true;  // no smooth
	
	bLoading = false;
}

//---------------------------------------------------------------------------------------------------------------
bool App::IsTerTrack()
{
	//  track: vdrift / terrain
	String sr = PATHMANAGER::GetTrackPath() + "/" + pSet->track + "/road.xml";
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
	road->Setup("", 0.7,  terrain, mSceneMgr, mCamera);

	String sr = PATHMANAGER::GetTrackPath() + "/" + pSet->track + "/road.xml";
	road->LoadFile(sr);

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
