#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/data/CData.h"
#include "../ogre/common/ShapeData.h"
#include "../ogre/common/QTimer.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../paged-geom/PagedGeometry.h"
#include "../vdrift/pathmanager.h"
#include "../ogre/common/RenderConst.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "../shiny/Main/Factory.hpp"

#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreRenderWindow.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreMeshManager.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreManualObject.h>
#include "../ogre/common/MessageBox/MessageBox.h"
using namespace Ogre;


//  Create Scene
//-------------------------------------------------------------------------------------
void App::createScene()  // once, init
{
	//  prv tex
	prvView.Create(1024,1024,"PrvView");
	prvRoad.Create(1024,1024,"PrvRoad");
	prvTer.Create(512,512,"PrvTer");

	scn->roadDens.Create(1025,1025,"RoadDens");
	
	///  ter lay tex
	for (int i=0; i < 6; ++i)
	{	String si = toStr(i);
		scn->texLayD[i].SetName("layD"+si);
		scn->texLayN[i].SetName("layN"+si);
	}


	//  camera
	asp = float(mWindow->getWidth())/float(mWindow->getHeight());
	mCamera->setFarClipDistance(pSet->view_distance*1.1f);
	mCamera->setNearClipDistance(0.1f);

	//  cam pos from last set
	mCamera->setPosition(Vector3(pSet->cam_x,pSet->cam_y,pSet->cam_z));
	mCamera->setDirection(Vector3(pSet->cam_dx,pSet->cam_dy,pSet->cam_dz).normalisedCopy());
	mViewport->setVisibilityMask(RV_MaskAll);  // hide prv cam rect

	//  tex fil
	MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	MaterialManager::getSingleton().setDefaultAnisotropy(pSet->anisotropy);

	QTimer ti;  ti.update();  /// time


	///  _Tool_ tex ..........................
	//  (remove alpha channel for ter tex prv img)
	#if 0
	gui->ToolTexAlpha();
	exit(0);
	#endif


	//  data load xml
	scn->data->Load();
	scn->sc->pFluidsXml = scn->data->fluids;
	
	//  surfaces.cfg
	LoadAllSurfaces();
	
	// TODO: ter/road layer  presets.xml

	ti.update();  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time load xmls: ") + fToStr(dt,0,3) + " ms");


	///  _Tool_ scene ...........................
	//  check/resave all tracks scene.xml 
	#if 0
	ToolSceneXml();
	exit(0);
	#endif
	

	postInit();  // material factory

	//  gui  * * *
	if (pSet->startInMain)
		pSet->isMain = true;
		
	bGuiFocus = false/*true*/;  bMoveCam = true;  //*--

	gui->InitGui();
	

	///  _Tool_ write all trks sceneryID .......
	#if 0
	ToolListSceneryID();
	exit(0);
	#endif

	///  _Tool_	Warnings ...........................
	//  check all tracks for warnings
	//  Warning: takes about 16 sec
	#if 0
	ToolTracksWarnings();
	exit(0);
	#endif
	

	TerCircleInit();
	createBrushPrv();
	
	///  _Tool_ brushes prv ...........................
	//  update all Brushes png
	#if 0  // 0 in release !!
	ToolBrushesPrv();
	#endif
	

	if (pSet->inputBar)  mDebugOverlay->show();
	if (!pSet->camPos)  ovPos->hide();

	//  load
	if (pSet->autostart)
		LoadTrack();

	if (!pSet->autostart)
	{	bGuiFocus = true;  UpdVisGui();	}

	iObjTNew = 0;
	//SetObjNewType(0);  //?white
}

void App::destroyScene()
{
	scn->destroyScene();
}


//---------------------------------------------------------------------------------------------------------------
///  Load Track
//---------------------------------------------------------------------------------------------------------------

//  destroy
void App::NewCommon(bool onlyTerVeget)
{
	//  destroy all
	if (ndSky)
		mSceneMgr->destroySceneNode(ndSky);

	scn->DestroyTrees();

	if (!onlyTerVeget)
		scn->DestroyWeather();

	mSceneMgr->destroyAllStaticGeometry();
	mStaticGeom = 0;
	DestroyVdrTrackBlt();
	
	if (!onlyTerVeget)
	{
		DestroyObjects(true);
		scn->DestroyFluids();
	}
		
	scn->DestroyTerrain();
		
	//world.Clear();
	track->Clear();

	if (resTrk != "")  mRoot->removeResourceLocation(resTrk);
	resTrk = gcom->TrkDir() + "objects";
	mRoot->addResourceLocation(resTrk, "FileSystem");

	MeshManager::getSingleton().unloadUnreferencedResources();
	sh::Factory::getInstance().unloadUnreferencedMaterials();
	TextureManager::getSingleton().unloadUnreferencedResources();
}

//  create
//---------------------------------------------------------------------------------------------------------------
void App::LoadTrack()
{
	eTrkEvent = TE_Load;
	gui->Status("Loading...", 0.3,0.6,1.0);
}
void App::LoadTrackEv()
{
	QTimer ti;  ti.update();  /// time
	NewCommon(false);  // full destroy

	if (scn->road)
	{	scn->road->Destroy();  delete scn->road;  scn->road = 0;  }

	// load scene
	scn->sc->LoadXml(gcom->TrkDir()+"scene.xml");
	scn->sc->vdr = IsVdrTrack();
	if (scn->sc->vdr)  scn->sc->ter = false;
	
	//  water RTT recreate
	scn->UpdateWaterRTT(mCamera);
	
	BltWorldInit();

	UpdWndTitle();

	scn->CreateFluids();

	scn->CreateWeather();


	//  set sky tex name for water
	sh::MaterialInstance* m = mFactory->getMaterialInstance(scn->sc->skyMtr);
	std::string skyTex = sh::retrieveValue<sh::StringValue>(m->getProperty("texture"), 0).get();
	sh::Factory::getInstance().setTextureAlias("SkyReflection", skyTex);
	sh::Factory::getInstance().setTextureAlias("CubeReflection", "ReflectionCube");


	bNewHmap = false;/**/
	scn->CreateTerrain(bNewHmap, scn->sc->ter);

	if (scn->sc->vdr)  // vdrift track
	{
		if (!LoadTrackVdr(pSet->gui.track))
			LogO("Error during track loading: " + pSet->gui.track);
		
		CreateVdrTrack(pSet->gui.track, track);
		CreateVdrTrackBlt();
	}


	//  road ~
	scn->road = new SplineRoad(this);
	scn->road->Setup("sphere.mesh", 1.4f*pSet->road_sphr, scn->terrain, mSceneMgr, mCamera);
	scn->road->LoadFile(gcom->TrkDir()+"road.xml");
	scn->UpdPSSMMaterials();
	
	CreateObjects();

	if (pSet->bTrees && scn->sc->ter)
		scn->CreateTrees();  // trees after objects so they aren't inside them


	//  updates after load
	//--------------------------
	gcom->ReadTrkStats();
	gui->SetGuiFromXmls();  ///
	
	Rnd2TexSetup();
	UpdVisGui();
	LoadStartPos(gcom->TrkDir());

	try {
	TexturePtr tex = TextureManager::getSingleton().getByName("waterDepth.png");
	if (!tex.isNull())
		tex->reload();
	} catch(...) {  }

	gui->Status("Loaded", 0.5,0.7,1.0);
	
	if (pSet->check_load)
		gui->WarningsCheck(scn->sc, scn->road);

	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Load Track: ") + fToStr(dt,0,3) + " ms");
}


///  Update Track
//---------------------------------------------------------------------------------------------------------------
void App::UpdateTrack()
{
	eTrkEvent = TE_Update;
	gui->Status("Updating...",0.2,1.0,0.5);
}
void App::UpdateTrackEv()
{
	NewCommon(true);  // destroy only terrain and veget
	
	//CreateFluids();
	scn->CreateTerrain(bNewHmap,true);/**/

	//  road ~
	scn->road->mTerrain = scn->terrain;
	scn->road->RebuildRoad(true);
	scn->UpdPSSMMaterials();

	//CreateObjects();

	if (pSet->bTrees)
		scn->CreateTrees();

	Rnd2TexSetup();

	gui->Status("Updated",0.5,1.0,0.7);
}


///  Save Terrain
//---------------------------------------------------------------------------------------------------------------
void App::SaveTrack()
{
	if (!pSet->allow_save)  // could force it when in writable location
	if (!pSet->gui.track_user)
	{	MyGUI::Message::createMessageBox(
			"Message", "Save Track", "Can't save original track. Duplicate it first.",
			MyGUI::MessageBoxStyle::IconWarning | MyGUI::MessageBoxStyle::Ok);
		return;
	}
	eTrkEvent = TE_Save;
	gui->Status("Saving...", 1,0.4,0.1);

	if (pSet->check_save)
		gui->WarningsCheck(scn->sc, scn->road);
}
void App::SaveTrackEv()
{
	String dir = gcom->TrkDir();
	//  track dir in user
	gui->CreateDir(dir);
	gui->CreateDir(dir+"/objects");
	//  check if succeded ...

	if (scn->terrain)
	{	float *fHmap = scn->terrain->getHeightData();
		int size = scn->sc->td.iVertsX * scn->sc->td.iVertsY * sizeof(float);

		String file = dir+"heightmap.f32";
		std::ofstream of;
		of.open(file.c_str(), std::ios_base::binary);
		of.write((const char*)fHmap, size);
		of.close();
	}
	if (scn->road)
		scn->road->SaveFile(dir+"road.xml");

	scn->sc->SaveXml(dir+"scene.xml");

	bool vdr = IsVdrTrack();
	/*if (!vdr)*/  SaveGrassDens();
	if (!vdr)  SaveWaterDepth();  //?-

	SaveStartPos(dir+"track.txt");  //..load/save inside
	
	gui->Delete(gui->getHMapNew());
	gui->Status("Saved", 1,0.6,0.2);
}


///  Ter Circle mesh   o
//-------------------------------------------------------------------------------------
const int divs = 90;
const Real aAdd = 2 * 2*PI_d / divs, dTc = 2.f/(divs+1) *4;
static Real fTcos[divs+4], fTsin[divs+4];


void App::TerCircleInit()
{
	moTerC = mSceneMgr->createManualObject();
	moTerC->setDynamic(true);
	moTerC->setCastShadows(false);

	moTerC->estimateVertexCount(divs+2);
	moTerC->estimateIndexCount(divs+2);
	moTerC->begin("circle_deform", RenderOperation::OT_TRIANGLE_STRIP);

	for (int d = 0; d < divs+2; ++d)
	{
		Real a = d/2 * aAdd;	fTcos[d] = cosf(a);  fTsin[d] = sinf(a);
		Real r = (d % 2 == 0) ? 1.f : 0.9f;
		Real x = r * fTcos[d], z = r * fTsin[d];
		moTerC->position(x,0,z);  //moTerC->normal(0,1,0);
		moTerC->textureCoord(d/2*dTc, d%2);
	}
	moTerC->end();
 
	AxisAlignedBox aab;  aab.setInfinite();
	moTerC->setBoundingBox(aab);  // always visible
	moTerC->setRenderQueueGroup(RQG_Hud2);
	moTerC->setVisibilityFlags(RV_Hud);
	ndTerC = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,0,0));
	ndTerC->attachObject(moTerC);  ndTerC->setVisible(false);
}


void App::TerCircleUpd()
{
	if (!moTerC || !scn->terrain || !scn->road)  return;

	bool edTer = bEdit() && (edMode < ED_Road) && scn->road->bHitTer;
	ndTerC->setVisible(edTer);
	if (!edTer)  return;
	
	Real rbr = mBrSize[curBr] * 0.5f * scn->sc->td.fTriangleSize * 0.8f/*?par*/;

	static ED_MODE edOld = ED_ALL;
	if (edOld != edMode)
	{	edOld = edMode;
		switch(edMode)
		{
		case ED_Deform: moTerC->setMaterialName(0, "circle_deform");  break;
		case ED_Filter: moTerC->setMaterialName(0, "circle_filter");  break;
		case ED_Smooth: moTerC->setMaterialName(0, "circle_smooth");  break;
		case ED_Height: moTerC->setMaterialName(0, "circle_height");  break;
		}
	}
	moTerC->beginUpdate(0);
	for (int d = 0; d < divs+2; ++d)
	{
		Real a = d/2 * aAdd;
		Real r = ((d % 2 == 0) ? 1.0f : 0.95f) * rbr;
		Real x = r * fTcos[d], z = r * fTsin[d];
		Vector3 p(x,0,z);  p += scn->road->posHit;
		p.y = scn->terrain->getHeightAtWorldPosition(p) + 0.3f;
		moTerC->position(p);  //moTerC->normal(0,1,0);
		moTerC->textureCoord(d/2*dTc, d%2);
	}
	moTerC->end();
}


//  vdrift track load
//-------------------------------------------------------------------------------------
bool App::LoadTrackVdr(const std::string & trackname)
{
	if (!track->DeferredLoad(
		(pSet->gui.track_user ? PATHMANAGER::TracksUser() : PATHMANAGER::Tracks()) + "/" + trackname,
		false/*trackreverse*/,
		/**/0, "large", true, false))
	{
		LogO("Error loading vdrift track: "+trackname);
		return false;
	}
	bool success = true;
	while (!track->Loaded() && success)
	{
		success = track->ContinueDeferredLoad();
	}

	if (!success)
	{
		LogO("Error loading vdrift track: "+trackname);
		return false;
	}
	return true;
}


//---------------------------------------------------------------------------------------------------------------
///  Bullet world
//---------------------------------------------------------------------------------------------------------------
void App::BltWorldInit()
{
	if (world)  // create world once
		return;
		//BltWorldDestroy();

	//  setup bullet world
	config = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(config);

	btScalar ws = 5000;  // world size
	broadphase = new bt32BitAxisSweep3(btVector3(-ws,-ws,-ws), btVector3(ws,ws,ws));
	solver = new btSequentialImpulseConstraintSolver();
	world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config);

	world->setGravity(btVector3(0.0, 0.0, -9.81)); ///~
	world->getSolverInfo().m_restitution = 0.0f;
	world->getDispatchInfo().m_enableSPU = true;
	world->setForceUpdateAllAabbs(false);  //+
}

void App::BltWorldDestroy()
{
	BltClear();

	delete world;  delete solver;
	delete broadphase;  delete dispatcher;  delete config;
}

//  Clear - delete bullet pointers
void App::BltClear()
{
	track = NULL;
	DestroyVdrTrackBlt();
	
	if (world)
	for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		world->removeCollisionObject(obj);

		ShapeData* sd = (ShapeData*)obj->getUserPointer();
		delete sd;  delete obj;
	}
	
	/*for (int i = 0; i < shapes.size(); i++)
	{
		btCollisionShape * shape = shapes[i];
		if (shape->isCompound())
		{
			btCompoundShape * cs = (btCompoundShape *)shape;
			for (int i = 0; i < cs->getNumChildShapes(); i++)
			{
				delete cs->getChildShape(i);
			}
		}
		delete shape;
	}
	shapes.resize(0);
	
	for(int i = 0; i < meshes.size(); i++)
	{
		delete meshes[i];
	}
	meshes.resize(0);
	
	for(int i = 0; i < actions.size(); i++)
	{
		world->removeAction(actions[i]);
	}
	actions.resize(0);*/
}


//  update (simulate)
//-------------------------------------------------------------------------------------
void App::BltUpdate(float dt)
{
	if (!world)  return;
	
	///  Simulate
	//double fixedTimestep(1.0/60.0);  int maxSubsteps(7);
	double fixedTimestep(1.0/160.0);  int maxSubsteps(24);  // same in game
	world->stepSimulation(dt, maxSubsteps, fixedTimestep);
	
	///  objects - dynamic (props)  -------------------------------------------------------------
	for (int i=0; i < scn->sc->objects.size(); ++i)
	{
		Object& o = scn->sc->objects[i];
		if (o.ms)
		{
			btTransform tr, ofs;
			o.ms->getWorldTransform(tr);
			/*const*/ btVector3& p = tr.getOrigin();
			const btQuaternion& q = tr.getRotation();
			o.pos[0] = p.x();  o.pos[1] = p.y();  o.pos[2] = p.z();
			o.rot[0] = q.x();  o.rot[1] = q.y();  o.rot[2] = q.z();  o.rot[3] = q.w();
			o.SetFromBlt();

			//p.setX(p.getX()+0.01f);  // move test-
			//tr.setOrigin(p);
			//o.ms->setWorldTransform(tr);
		}
	}
 }
 
