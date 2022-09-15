#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/data/CData.h"
#include "../ogre/common/ShapeData.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../road/PaceNotes.h"
#include "../paged-geom/PagedGeometry.h"
#include "../vdrift/pathmanager.h"
#include "../ogre/common/RenderConst.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "../shiny/Main/Factory.hpp"

#include <OgreTimer.h>
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreRenderWindow.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreMeshManager.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreManualObject.h>
#include <OgreViewport.h>
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreSceneNode.h>
#include "../ogre/common/MessageBox/MessageBox.h"
#include "../ogre/common/Instancing.h"
using namespace Ogre;
using namespace std;


//  Create Scene
//-------------------------------------------------------------------------------------
void App::createScene()  // once, init
{
	//  prv tex
	int k=1024;
	prvView.Create(k,k,"PrvView");
	prvRoad.Create(k,k,"PrvRoad");
	 prvTer.Create(k,k,"PrvTer");

	scn->roadDens.Create(k+1,k+1,"RoadDens");
	
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

	Ogre::Timer ti;


	//  data load xml
	scn->data->Load();
	scn->sc->pFluidsXml = scn->data->fluids;
	scn->sc->pReverbsXml = scn->data->reverbs;
	
	//  surfaces.cfg
	LoadAllSurfaces();
	
	LogO(String("::: Time load xmls: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");

	//  gui  * * *
	if (pSet->startInMain)
		pSet->bMain = true;
		
	bGuiFocus = false/*true*/;  bMoveCam = true;  //*--

	gui->InitGui();
	

	///__  All  #if 0  in Release !!!

	///  _Tool_ scene  ...................
	#if 0
	gui->ToolSceneXml();
	exit(0);
	#endif
	
	///  _Tool_	warnings  ................
	///  takes some time
	#if 0
	gui->ToolTracksWarnings();
	exit(0);
	#endif
	

	TerCircleInit();
	createBrushPrv();
	
	///  _Tool_ brushes prv  .............
	#if 0
	gui->ToolBrushesPrv();
	#endif
		

	//  load
	if (pSet->autostart)
		LoadTrack();

	if (!pSet->autostart)
	{	bGuiFocus = true;  UpdVisGui();	}

	iObjTNew = 0;
	//SetObjNewType(0);  //?white


	gui->chkInputBar(0);  // upd vis
	gui->chkCamPos(0);
	gui->chkFps(0);
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
		
	//if (inst) {  delete inst;  inst=0;  }

	scn->DestroyTrees();

	if (!onlyTerVeget)
		scn->DestroyWeather();

	mSceneMgr->destroyAllStaticGeometry();
	
	if (!onlyTerVeget)
	{
		DestroyObjects(true);
		scn->DestroyFluids();
		scn->DestroyEmitters(true);
	}
		
	scn->DestroyTerrain();
		
	//world.Clear();

	if (resTrk != "")  ResourceGroupManager::getSingleton().removeResourceLocation(resTrk);
	LogO("------  Loading track: "+pSet->gui.track);
	resTrk = gcom->TrkDir() + "objects";
	ResourceGroupManager::getSingleton().addResourceLocation(resTrk, "FileSystem");

	MeshManager::getSingleton().unloadUnreferencedResources();
	sh::Factory::getInstance().unloadUnreferencedMaterials();
	TextureManager::getSingleton().unloadUnreferencedResources();
}

///  Load
//---------------------------------------------------------------------------------------------------------------
void App::LoadTrack()
{
	eTrkEvent = TE_Load;
	gui->Status("#{Loading}...", 0.3,0.6,1.0);
}
void App::LoadTrackEv()
{
	Ogre::Timer ti;
	NewCommon(false);  // full destroy
	iObjCur = -1;  iEmtCur = -1;

	scn->DestroyRoads();
	scn->DestroyPace();
	

	// load scene
	scn->sc->LoadXml(gcom->TrkDir()+"scene.xml");
	
	//  water RTT recreate
	scn->UpdateWaterRTT(mCamera);
	
	BltWorldInit();

	UpdWndTitle();

	scn->CreateFluids();

	scn->CreateWeather();

	scn->CreateEmitters();


	//  set sky tex name for water
	sh::MaterialInstance* m = mFactory->getMaterialInstance(scn->sc->skyMtr);
	std::string skyTex = sh::retrieveValue<sh::StringValue>(m->getProperty("texture"), 0).get();
	sh::Factory::getInstance().setTextureAlias("SkyReflection", skyTex);
	sh::Factory::getInstance().setTextureAlias("CubeReflection", "ReflectionCube");


	bNewHmap = false;/**/
	scn->CreateTerrain(bNewHmap, scn->sc->ter);


	//  road ~
	CreateRoads();

	scn->UpdPSSMMaterials();
	
	//  pace ~ ~
	scn->pace = new PaceNotes(pSet);
	scn->pace->Setup(mSceneMgr, mCamera, scn->terrain, gui->mGui, mWindow);
	
	
	/// HW_Inst Test  * * *
	//inst = new Instanced();
	//inst->Create(mSceneMgr,"sphere_inst.mesh");
	
	
	CreateObjects();
	
	if (pSet->bTrees && scn->sc->ter)
		scn->CreateTrees();  // trees after objects so they aren't inside them


	//  updates after load
	//--------------------------
	gcom->ReadTrkStats();
	gui->SetGuiFromXmls();  ///
	
	Rnd2TexSetup();
	//UpdVisGui();
	//UpdStartPos();
	UpdEditWnds();  //

	try
	{	TexturePtr tex = TextureManager::getSingleton().getByName("waterDepth.png");
		if (tex)
			tex->reload();
	}catch(...)
	{	}

	gui->Status("#{Loaded}", 0.5,0.7,1.0);
	
	if (pSet->check_load)
		gui->WarningsCheck(scn->sc, scn->road);

	LogO(String("::: Time Load Track: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


void App::CreateRoads()
{
	strlist lr;  string path = gcom->TrkDir();
	PATHMANAGER::DirList(path, lr, "xml");
	
	for (auto fname:lr)
	if (StringUtil::startsWith(fname,"road"))
	{
		int id = scn->roads.size();
		LogO("~~~ Creating road " + toStr(id) + " from: " + fname);
		scn->road = new SplineRoad(this);
		scn->road->Setup("sphere.mesh", pSet->road_sphr, scn->terrain, mSceneMgr, mCamera, id);
		scn->road->LoadFile(path + fname);
		scn->roads.push_back(scn->road);
	}
	scn->rdCur = 0;
	scn->road = scn->roads[scn->rdCur];
}


///  Update
//---------------------------------------------------------------------------------------------------------------
void App::UpdateTrack()
{
	eTrkEvent = TE_Update;
	gui->Status("#{Updating}...", 0.2,1.0,0.5);
}
void App::UpdateTrackEv()
{
	if (!bNewHmap)
		scn->copyTerHmap();

	NewCommon(true);  // destroy only terrain and veget
	
	//CreateFluids();
	scn->CreateTerrain(bNewHmap,true,false);/**/

	//  road ~
	for (auto r : scn->roads)
	{
		r->mTerrain = scn->terrain;
		r->Rebuild(true);
	}
	scn->UpdPSSMMaterials();

	//CreateObjects();

	if (pSet->bTrees)
		scn->CreateTrees();

	Rnd2TexSetup();

	gui->Status("#{Updated}", 0.5,1.0,0.7);
}

//  Update btns  ---
void CGui::btnUpdateLayers(WP)
{
	if (!app->bNewHmap)
		app->scn->copyTerHmap();
	if (app->ndSky)
		app->mSceneMgr->destroySceneNode(app->ndSky);
	app->scn->DestroyTerrain();

	app->scn->CreateTerrain(app->bNewHmap,true,false);
	scn->road->mTerrain = scn->terrain;
	app->scn->updGrsTer();
}

void CGui::btnUpdateGrass(WP)  // TODO: grass only ...
{
	scn->DestroyTrees();
	if (pSet->bTrees)
		scn->CreateTrees();
}

void CGui::btnUpdateVeget(WP)
{
	scn->DestroyTrees();
	if (pSet->bTrees)
		scn->CreateTrees();
}


///  Save
//---------------------------------------------------------------------------------------------------------------
void App::SaveTrack()
{
	if (!pSet->allow_save)  // can force it when in writable location
	if (!pSet->gui.track_user)
	{	MyGUI::Message::createMessageBox(
			"Message", TR("#{Track} - #{RplSave}"), TR("#{CantSaveOrgTrack}"),
			MyGUI::MessageBoxStyle::IconWarning | MyGUI::MessageBoxStyle::Ok);
		return;
	}
	eTrkEvent = TE_Save;
	gui->Status("#{Saving}...", 1,0.4,0.1);

	if (pSet->check_save)
		gui->WarningsCheck(scn->sc, scn->road);
}
void App::SaveTrackEv()
{
	String dir = gcom->TrkDir();
	//  track dir in user
	gui->CreateDir(dir);
	gui->CreateDir(dir+"/objects");

	if (scn->terrain)
	{	float *fHmap = scn->terrain->getHeightData();
		int size = scn->sc->td.iVertsX * scn->sc->td.iVertsY * sizeof(float);

		String file = dir+"heightmap.f32";
		std::ofstream of;
		of.open(file.c_str(), std::ios_base::binary);
		of.write((const char*)fHmap, size);
		of.close();
	}

	int i = 0;  // all roads
	for (auto r : scn->roads)
	{
		auto si = i==0 ? "" : toStr(i+1);  ++i;
		r->SaveFile(dir+ "road"+si+".xml");
	}

	scn->sc->SaveXml(dir+"scene.xml");

	SaveGrassDens();
	SaveWaterDepth();

	gui->Delete(gui->getHMapNew());
	gui->Status("#{Saved}", 1,0.6,0.2);
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
		switch (edMode)
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
 
