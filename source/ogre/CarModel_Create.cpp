#include "pch.h"
#include "common/Defines.h"
#include "CarModel.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/mathvector.h"
#include "../vdrift/track.h"
#include "../vdrift/game.h"
#include "../vdrift/performance_testing.h"
#include "OgreGame.h"
#include "SplitScreen.h"
#include "common/SceneXml.h"
#include "FollowCamera.h"
#include "CarReflection.h"
#include "../road/Road.h"
#include "common/RenderConst.h"
#include "../shiny/Main/Factory.hpp"

#include "boost/filesystem.hpp"
#define  FileExists(s)  boost::filesystem::exists(s)

#include <OgreRoot.h>
#include <OgreTerrain.h>
#include <OgreEntity.h>
#include <OgreManualObject.h>
#include <OgreMaterialManager.h>
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <OgreParticleAffector.h>
#include <OgreRibbonTrail.h>

using namespace Ogre;


//  Init  ---------------------------------------------------
CarModel::CarModel(unsigned int index, eCarType type, const std::string& name,
	SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* s,
	Camera* cam, App* app, int startpos_index) :
	fCam(0), pMainNode(0), pCar(0), terrain(0), resCar(""), ndSph(0),
	mCamera(0), pReflect(0), pApp(app), color(1,1,0),
	bLightMapEnabled(true), bBraking(false),
	hideTime(1.f), mbVisible(true),
	iCamNextOld(0), bLastChkOld(0), bWrongChk(0),  iFirst(0),
	angCarY(0), vStartPos(0,0,0), pNickTxt(0),
	all_subs(0), all_tris(0)  //stats
{
	iIndex = index;  sDirname = name;  mSceneMgr = sceneMgr;
	pSet = set;  pGame = game;  sc = s;  mCamera = cam;  eType = type;
	bGetStPos = true;  fChkTime = 0.f;  iChkWrong = -1;  iWonPlace = 0;  iWonPlaceOld = 0;
	iCurChk = -1;  iNumChks = 0;  iNextChk = 0;  //ResetChecks();  // road isnt yet
	distFirst = 1.f;  distLast = 1.f;  distTotal = 10.f;  trackPercent = 0.f;

	for (int w = 0; w < 4; ++w)
	{
		for (int p=0; p < PAR_ALL; ++p)
			par[p][w] = 0;

		ndWh[w] = 0;  ndWhE[w] = 0; whTrl[w] = 0;
		ndBrake[w] = 0;
		wht[w] = 0.f;
	}
	for (int i=0; i < 2; i++)
		pb[i] = 0;
	ph = 0;

	//  names for local play
	if (type == CT_GHOST)		sDispName = "Ghost";
	else if (type == CT_LOCAL)	sDispName = "Player"+toStr(iIndex+1);
	

	//  get car start pos from track  ------
	/***/if (type != CT_GHOST)  // ghost has pCar, dont create
	{
		if (startpos_index == -1) startpos_index = iIndex;
		int i = set->game.collis_cars ? startpos_index : 0;  // offset when cars collide
		MATHVECTOR<float,3> pos(0,10,0);
		QUATERNION<float> rot;
		pos = pGame->track.GetStart(i).first;
		rot = pGame->track.GetStart(i).second;
		vStartPos = Vector3(pos[0], pos[2], -pos[1]);  // save in ogre coords

		//  load car
		std::string pathCar;
		pApp->GetCarPath(&pathCar, 0, 0, sDirname, pApp->mClient);  // force orig for newtorked games
		
		pCar = pGame->LoadCar(pathCar, sDirname, pos, rot, true, false, type == CT_REMOTE, index);

		///  vdr car perf test  not working gears...
		#if 0
		QTimer ti;  ti.update();  /// time

		PERFORMANCE_TESTING perf;
		perf.Test(pathCar, pApp, pGame->info_output, pGame->error_output);

		ti.update();	/// time
		float dt = ti.dt * 1000.f;
		LogO(String("::: Time car perf test: ") + toStr(dt) + " ms");
		//exit(0);/*+*/
		#endif

		if (!pCar)  LogO("Error creating car " + sDirname + "  path: " + pathCar);
		else  pCar->pCarM = this;
	}
}

//  Destroy  ---------------------------------------------------
CarModel::~CarModel()
{
	delete pReflect;  pReflect = 0;
	
	delete fCam;  fCam = 0;
	mSceneMgr->destroyCamera("CarCamera" + toStr(iIndex));
	
	//  hide trails
	for (int w=0; w<4; ++w)  if (whTrl[w])  {	wht[w] = 0.f;
		whTrl[w]->setVisible(false);	whTrl[w]->setInitialColour(0, 0.5,0.5,0.5, 0);	}

	//  destroy cloned materials
	for (int i=0; i<NumMaterials; i++)
		MaterialManager::getSingleton().remove(sMtr[i]);
	
	//  destroy par sys
	for (int w=0; w < 4; ++w)
	{	for (int p=0; p < PAR_ALL; ++p)
			if (par[p][w]) {  mSceneMgr->destroyParticleSystem(par[p][w]);   par[p][w]=0;  }
		if (ndBrake[w]) mSceneMgr->destroySceneNode(ndBrake[w]);
	}
	for (int i=0; i < 2; i++)
		if (pb[i]) {  mSceneMgr->destroyParticleSystem(pb[i]);   pb[i]=0;  }
	if (ph)  {  mSceneMgr->destroyParticleSystem(ph);   ph=0;  }
						
	if (pMainNode) mSceneMgr->destroySceneNode(pMainNode);
	
	//  destroy resource group, will also destroy all resources in it
	if (ResourceGroupManager::getSingleton().resourceGroupExists("Car" + toStr(iIndex)))
		ResourceGroupManager::getSingleton().destroyResourceGroup("Car" + toStr(iIndex));
}

	
//  log mesh stats
void CarModel::LogMeshInfo(const Entity* ent, const String& name)
{
	//return;
	const MeshPtr& msh = ent->getMesh();
	int tris=0, subs = msh->getNumSubMeshes();
	for (int i=0; i < subs; ++i)
	{
		SubMesh* sm = msh->getSubMesh(i);
		tris += sm->indexData->indexCount;
	}
	all_tris += tris;
	all_subs += subs;
	LogO("MESH info:  "+name+"\t sub: "+toStr(subs)+"  tri: "+fToStr(tris/1000.f,1,4)+"k");
}

//---------------------------------------------------
void CarModel::CreatePart(SceneNode* ndCar, Vector3 vPofs,
	String sCar2, String sCarI, String sMesh, String sEnt,
	bool ghost, uint32 visFlags,
	AxisAlignedBox* bbox, String stMtr, VERTEXARRAY* var, bool bLogInfo)
{
	if (FileExists(sCar2 + sMesh))
	{
		Entity* ent = mSceneMgr->createEntity(sCarI + sEnt, sDirname + sMesh, sCarI);
		if (bbox)  *bbox = ent->getBoundingBox();
		if (ghost)  {  ent->setRenderQueueGroup(RQG_CarGhost);  ent->setCastShadows(false);  }
		else  if (visFlags == RV_CarGlass)  ent->setRenderQueueGroup(RQG_CarGlass);
		ndCar->attachObject(ent);  ent->setVisibilityFlags(visFlags);
		if (bLogInfo)  LogMeshInfo(ent, sDirname + sMesh);
	}
	else
	{	ManualObject* mo = pApp->CreateModel(mSceneMgr, stMtr, var, vPofs, false, false, sCarI+sEnt);
		if (!mo)  return;
		if (bbox)  *bbox = mo->getBoundingBox();
		if (ghost)  {  mo->setRenderQueueGroup(RQG_CarGhost);  mo->setCastShadows(false);  }
		else  if (visFlags == RV_CarGlass)  mo->setRenderQueueGroup(RQG_CarGlass);
		ndCar->attachObject(mo);  mo->setVisibilityFlags(visFlags);
	
		/** ///  save .mesh
		MeshPtr mpCar = mInter->convertToMesh("Mesh" + sEnt);
		MeshSerializer* msr = new MeshSerializer();
		msr->exportMesh(mpCar.getPointer(), sDirname + sMesh);/**/
	}
}


//-------------------------------------------------------------------------------------------------------
//  Create
//-------------------------------------------------------------------------------------------------------
void CarModel::Create(int car)
{
	if (!pCar)  return;

	String strI = toStr(iIndex), sCarI = "Car" + strI;
	String sCars = PATHMANAGER::Cars() + "/" + sDirname;
	resCar = sCars + "/textures";
	String rCar = resCar + "/" + sDirname;
	String sCar = sCars + "/" + sDirname;
	
	bool ghost = eType == CT_GHOST && pSet->rpl_alpha;  //1 || for ghost test
	bool bLogInfo = eType != CT_GHOST;  // log mesh info
	
	//  Resource locations -----------------------------------------
	/// Add a resource group for this car
	ResourceGroupManager::getSingleton().createResourceGroup(sCarI);
	Ogre::Root::getSingletonPtr()->addResourceLocation(sCars, "FileSystem", sCarI);
	Ogre::Root::getSingletonPtr()->addResourceLocation(sCars + "/textures", "FileSystem", sCarI);
		
	pMainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	SceneNode* ndCar = pMainNode->createChildSceneNode();

	//  --------  Follow Camera  --------
	if (mCamera)
	{
		fCam = new FollowCamera(mCamera, pSet);
		fCam->chassis = pCar->dynamics.chassis;
		fCam->loadCameras();
		
		//  set in-car camera position to driver position
		for (std::vector<CameraAngle*>::iterator it=fCam->mCameraAngles.begin();
			it!=fCam->mCameraAngles.end(); ++it)
		{
			if ((*it)->mName == "Car driver")
				(*it)->mOffset = Vector3(pCar->driver_view_position[0], pCar->driver_view_position[2], -pCar->driver_view_position[1]);
			else if ((*it)->mName == "Car bonnet")
				(*it)->mOffset = Vector3(pCar->hood_view_position[0], pCar->hood_view_position[2], -pCar->hood_view_position[1]);
		}
	}
	
	RecreateMaterials();
		
	CreateReflection();


	///()  grass sphere test
	#if 0
	Entity* es = mSceneMgr->createEntity(sCarI+"s", "sphere.mesh", sCarI);
	es->setRenderQueueGroup(RQG_CarGhost);
	MaterialPtr mtr = MaterialManager::getSingleton().getByName("pipeGlass");
	es->setMaterial(mtr);
	ndSph = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	ndSph->attachObject(es);
	#endif


	///  Create Models:  body, interior, glass
	//-------------------------------------------------
	Vector3 vPofs(0,0,0);
	AxisAlignedBox bodyBox;  uint8 g = RQG_CarGhost;
	all_subs=0;  all_tris=0;  //stats
	
	if (pCar->bRotFix)
		ndCar->setOrientation(Quaternion(Degree(90),Vector3::UNIT_Y)*Quaternion(Degree(180),Vector3::UNIT_X));


	CreatePart(ndCar, vPofs, sCar, sCarI, "_body.mesh",     "",  ghost, RV_Car,  &bodyBox,  sMtr[Mtr_CarBody], &pCar->bodymodel.mesh,     bLogInfo);

	vPofs = Vector3(pCar->interiorOffset[0],pCar->interiorOffset[1],pCar->interiorOffset[2]);  //x+ back y+ down z+ right
	if (!ghost)
	CreatePart(ndCar, vPofs, sCar, sCarI, "_interior.mesh", "i", ghost, RV_Car,      0, sMtr[Mtr_CarBody]+"i", &pCar->interiormodel.mesh, bLogInfo);

	vPofs = Vector3::ZERO;
	CreatePart(ndCar, vPofs, sCar, sCarI, "_glass.mesh",    "g", ghost, RV_CarGlass, 0, sMtr[Mtr_CarBody]+"g", &pCar->glassmodel.mesh,    bLogInfo);
	

	//  wheels  ----------------------
	for (int w=0; w < 4; ++w)
	{
		String siw = "Wheel" + strI + "_" + toStr(w);
		ndWh[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		if (FileExists(sCar + "_wheel.mesh"))
		{
			String name = sDirname + "_wheel.mesh";
			Entity* eWh = mSceneMgr->createEntity(siw, sDirname + "_wheel.mesh", sCarI);
			if (ghost)  {  eWh->setRenderQueueGroup(g);  eWh->setCastShadows(false);  }
			ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(RV_Car);
			if (bLogInfo && w==0)  LogMeshInfo(eWh, name);
		}else
		{	ManualObject* mWh = pApp->CreateModel(mSceneMgr, sMtr[Mtr_CarBody]+siw, &pCar->wheelmodelfront.mesh, vPofs, true, false, siw);
			if (mWh)  {
			if (ghost)  {  mWh->setRenderQueueGroup(g);  mWh->setCastShadows(false);  }
			ndWh[w]->attachObject(mWh);  mWh->setVisibilityFlags(RV_Car);  }
		}
		
		if (FileExists(sCar + "_brake.mesh"))
		{
			String name = sDirname + "_brake.mesh";
			Entity* eBrake = mSceneMgr->createEntity(siw + "_brake", name, sCarI);
			if (ghost)  {  eBrake->setRenderQueueGroup(g);  eBrake->setCastShadows(false);  }
			ndBrake[w] = ndWh[w]->createChildSceneNode();
			ndBrake[w]->attachObject(eBrake);  eBrake->setVisibilityFlags(RV_Car);
			if (bLogInfo && w==0)  LogMeshInfo(eBrake, name);
		}
	}
	if (bLogInfo)  // all
		LogO("MESH info:  "+sDirname+"\t ALL sub: "+toStr(all_subs)+"  tri: "+fToStr(all_tris/1000.f,1,4)+"k");


	///  world hit sparks  ------------------------
	//if (!ghost)//-
	if (!ph)  {
		ph = mSceneMgr->createParticleSystem("Hit" + strI, "Sparks");
		ph->setVisibilityFlags(RV_Particles);
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ph);
		ph->getEmitter(0)->setEmissionRate(0);  }

	///  boost emitters  ------------------------
	for (int i=0; i < 2; i++)
	{
		String si = strI + "_" +toStr(i);
		if (!pb[i])  {
			pb[i] = mSceneMgr->createParticleSystem("Boost"+si, "Boost");
			pb[i]->setVisibilityFlags(RV_Particles);
			if (!pSet->boostFromExhaust || !pCar->manualExhaustPos)
			{
				// no exhaust pos in car file, guess from bounding box
				Vector3 bsize = (bodyBox.getMaximum() - bodyBox.getMinimum())*0.5,
					bcenter = bodyBox.getMaximum() + bodyBox.getMinimum();
				//LogO("Car body bbox :  size " + toStr(bsize) + ",  center " + toStr(bcenter));
				Vector3 vp = pCar->bRotFix ?
					Vector3(bsize.z * 0.97, bsize.y * 0.65, bsize.x * 0.65 * (i==0 ? 1 : -1)) :
					Vector3(bsize.x * 0.97, bsize.y * 0.65, bsize.z * 0.65 * (i==0 ? 1 : -1));
					//Vector3(1.9 /*back*/, 0.1 /*up*/, 0.6 * (i==0 ? 1 : -1)/*sides*/
				vp += Vector3(pCar->boostOffset[0],pCar->boostOffset[1],pCar->boostOffset[2]);
				SceneNode* nb = pMainNode->createChildSceneNode(bcenter+vp);
				nb->attachObject(pb[i]);
			}else{
				// use exhaust pos values from car file
				Vector3 pos;
				if (i==0)
					pos = Vector3(pCar->exhaustPosition[0], pCar->exhaustPosition[1], pCar->exhaustPosition[2]);
				else if (!pCar->has2exhausts)
					continue;
				else
					pos = Vector3(pCar->exhaustPosition[0], pCar->exhaustPosition[1], -pCar->exhaustPosition[2]);

				SceneNode* nb = pMainNode->createChildSceneNode(pos);
				nb->attachObject(pb[i]); 
			}
			pb[i]->getEmitter(0)->setEmissionRate(0);
		}
	}

	///  wheel emitters  ------------------------
	if (!ghost)
	{
		const static String sPar[PAR_ALL] = {"Smoke","Mud","Dust","FlWater","FlMud","FlMudS"};  // for ogre name
		//  particle type names
		const String sName[PAR_ALL] = {sc->sParSmoke, sc->sParMud, sc->sParDust, "FluidWater", "FluidMud", "FluidMudSoft"};
		for (int w=0; w < 4; ++w)
		{
			String siw = strI + "_" +toStr(w);
			//  particles
			for (int p=0; p < PAR_ALL; ++p)
			if (!par[p][w])
			{
				par[p][w] = mSceneMgr->createParticleSystem(sPar[p]+siw, sName[p]);
				par[p][w]->setVisibilityFlags(RV_Particles);
				mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(par[p][w]);
				par[p][w]->getEmitter(0)->setEmissionRate(0.f);
			}
			//  trails
			if (!ndWhE[w])
				ndWhE[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();

			if (!whTrl[w])
			{	NameValuePairList params;
				params["numberOfChains"] = "1";
				params["maxElements"] = toStr(320 * pSet->trails_len);

				whTrl[w] = (RibbonTrail*)mSceneMgr->createMovableObject("RibbonTrail", &params);
				whTrl[w]->setInitialColour(0, 0.1,0.1,0.1, 0);
				whTrl[w]->setFaceCamera(false,Vector3::UNIT_Y);
				mSceneMgr->getRootSceneNode()->attachObject(whTrl[w]);
				whTrl[w]->setMaterialName("TireTrail");
				whTrl[w]->setCastShadows(false);
				whTrl[w]->addNode(ndWhE[w]);
			}
			whTrl[w]->setTrailLength(90 * pSet->trails_len);  //30
			whTrl[w]->setInitialColour(0, 0.1f,0.1f,0.1f, 0);
			whTrl[w]->setColourChange(0, 0.0,0.0,0.0, /*fade*/0.08f * 1.f / pSet->trails_len);
			whTrl[w]->setInitialWidth(0, 0.f);
		}
	}


	UpdParsTrails();
			
	setMtrNames();
	
	//  this snippet makes sure the brake texture is pre-loaded.
	//  since it is not used until you actually brake, we have to explicitely declare it
	if (FileExists(rCar + "_body00_brake.png")) ResourceGroupManager::getSingleton().declareResource(sDirname + "_body00_brake.png", "Texture", sCarI);
	if (FileExists(rCar + "_body00_add.png"))   ResourceGroupManager::getSingleton().declareResource(sDirname + "_body00_add.png", "Texture", sCarI);
	
	//  now just preload the whole resource group
	ResourceGroupManager::getSingleton().initialiseResourceGroup(sCarI);
	ResourceGroupManager::getSingleton().loadResourceGroup(sCarI);
}


//-------------------------------------------------------------------------------------------------------
//  materials
//-------------------------------------------------------------------------------------------------------
void CarModel::RecreateMaterials()
{
	String strI = toStr(iIndex);
	String sCar = resCar + "/" + sDirname;
	bool ghost = eType == CT_GHOST && pSet->rpl_alpha;  //1 || for ghost test
	
	// --------- Materials  -------------------
	
	// if specialised car material (e.g. car_body_FM) exists, use this one instead of e.g. car_body
	// useful macro for choosing between these 2 variants
	#define chooseMat(s)  MaterialManager::getSingleton().resourceExists("car"+String(s) + "_"+sDirname) ? "car"+String(s) + "_"+sDirname : "car"+String(s)

	//  ghost car has no interior, particles, trails and uses same material for all meshes
	if (!ghost)
	{	sMtr[Mtr_CarBody]     = chooseMat("_body");
		sMtr[Mtr_CarBrake]    = chooseMat("_glass");
	}else
	for (int i=0; i < NumMaterials; ++i)
		sMtr[i] = "car_ghost";

	//  copy material to a new material with index
	MaterialPtr mat;
	for (int i=0; i < 1/*NumMaterials*/; ++i)
	{
		sh::Factory::getInstance().destroyMaterialInstance(sMtr[i] + strI);
		sh::MaterialInstance* m = sh::Factory::getInstance().createMaterialInstance(sMtr[i] + strI, sMtr[i]);

		m->setListener(this);

		// change textures for the car
		if (m->hasProperty("diffuseMap"))
		{
			std::string v = sh::retrieveValue<sh::StringValue>(m->getProperty("diffuseMap"), 0).get();
			m->setProperty("diffuseMap", sh::makeProperty<sh::StringValue>(new sh::StringValue(sDirname + "_" + v)));
		}
		if (m->hasProperty("carPaintMap"))
		{
			std::string v = sh::retrieveValue<sh::StringValue>(m->getProperty("carPaintMap"), 0).get();
			m->setProperty("carPaintMap", sh::makeProperty<sh::StringValue>(new sh::StringValue(sDirname + "_" + v)));
		}
		if (m->hasProperty("reflMap"))
		{
			std::string v = sh::retrieveValue<sh::StringValue>(m->getProperty("reflMap"), 0).get();
			m->setProperty("reflMap", sh::makeProperty<sh::StringValue>(new sh::StringValue(sDirname + "_" + v)));
		}
		sMtr[i] = sMtr[i] + strI;
	}

	//ChangeClr(iIndex);

	UpdateLightMap();
}

void CarModel::setMtrName(const String& entName, const String& mtrName)
{
	if (mSceneMgr->hasEntity(entName))
		mSceneMgr->getEntity(entName)->setMaterialName(mtrName);
	else
	if (mSceneMgr->hasManualObject(entName))
		mSceneMgr->getManualObject(entName)->setMaterialName(0, mtrName);
}

void CarModel::setMtrNames()
{
	String strI = toStr(iIndex);

	if (FileExists(resCar + "/" + sDirname + "_body00_add.png") ||
		FileExists(resCar + "/" + sDirname + "_body00_red.png"))
		setMtrName("Car"+strI, sMtr[Mtr_CarBody]);

	if (pCar && pCar->bRotFix)
		return;
}

//  ----------------- Reflection ------------------------
void CarModel::CreateReflection()
{
	pReflect = new CarReflection(pSet, pApp, mSceneMgr, iIndex);
	for (int i=0; i < NumMaterials; i++)
		pReflect->sMtr[i] = sMtr[i];

	pReflect->Create();
}

void CarModel::requestedConfiguration(sh::MaterialInstance* m, const std::string& configuration)
{
}

void CarModel::createdConfiguration(sh::MaterialInstance* m, const std::string& configuration)
{
	UpdateLightMap();
	ChangeClr(iIndex);
}
