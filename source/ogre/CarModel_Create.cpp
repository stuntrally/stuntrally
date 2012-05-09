#include "pch.h"
#include "common/Defines.h"
#include "CarModel.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/mathvector.h"
#include "../vdrift/track.h"
#include "../vdrift/game.h"
#include "OgreGame.h"
#include "SplitScreen.h"
#include "common/SceneXml.h"
#include "FollowCamera.h"
#include "CarReflection.h"
#include "../road/Road.h"
#include "common/RenderConst.h"
#include "common/MaterialGen/MaterialFactory.h"

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
	Ogre::SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* s,
	Ogre::Camera* cam, App* app, int startpos_index) :
	fCam(0), pMainNode(0), pCar(0), terrain(0), resCar(""),
	mCamera(0), pReflect(0), pApp(app), color(1,1,0),
	bLightMapEnabled(true), bBraking(false),
	hideTime(1.f), mbVisible(true),
	iCamNextOld(0), bLastChkOld(0), bWrongChk(0),
	angCarY(0), vStartPos(0,0,0), pNickTxt(0)
{
	iIndex = index;  sDirname = name;  mSceneMgr = sceneMgr;
	pSet = set;  pGame = game;  sc = s;  mCamera = cam;  eType = type;
	bGetStPos = true;  fChkTime = 0.f;  iChkWrong = -1;  iWonPlace = 0;  iWonPlaceOld = 0;
	iCurChk = -1;  iNumChks = 0;  iNextChk = 0;  //ResetChecks();  // road isnt yet
	distFirst = 1.f;  distLast = 1.f;  distTotal = 10.f;  trackPercent = 0.f;

	for (int w = 0; w < 4; ++w)
	{	ps[w] = 0;  pm[w] = 0;  pd[w] = 0;
		pflW[w] = 0;  pflM[w] = 0;  pflMs[w] = 0;
		ndWh[w] = 0;  ndWhE[w] = 0; whTrl[w] = 0;
		ndBrake[w] = 0;
		wht[w] = 0.f;  whTerMtr[w] = 0;  whRoadMtr[w] = 0;  }
	for (int i=0; i < 2; i++)
		pb[i] = 0;
	ph = 0;

	//  names for local play
	if (type == CT_GHOST)		sDispName = "Ghost";
	else if (type == CT_LOCAL)	sDispName = "Player"+toStr(iIndex+1);
	
	//  get car start pos from track  ------
	if (type != CT_GHOST)  // ghost has pCar, dont create
	{
		if (startpos_index == -1) startpos_index = iIndex;
		int i = set->game.collis_cars ? startpos_index : 0;  //  offset car start pos when cars collide
		MATHVECTOR<float, 3> pos(0,10,0);
		QUATERNION<float> rot;
		pos = pGame->track.GetStart(i).first;
		rot = pGame->track.GetStart(i).second;
		vStartPos = Vector3(pos[0], pos[2], -pos[1]); // save in ogre coords

		//  offset car start pos when cars collide
		MATHVECTOR<float, 3> offset(0,0,0);
		pCar = pGame->LoadCar(sDirname, pos, rot, true, false, type == CT_REMOTE, index);
		if (!pCar)  LogO("Error creating car " + sDirname);
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

	// destroy cloned materials
	for (int i=0; i<NumMaterials; i++)
		Ogre::MaterialManager::getSingleton().remove(sMtr[i]);
	
	// Destroy par sys
	for (int w=0; w < 4; ++w)  {
		if (ps[w]) {  mSceneMgr->destroyParticleSystem(ps[w]);   ps[w]=0;  }
		if (pm[w]) {  mSceneMgr->destroyParticleSystem(pm[w]);   pm[w]=0;  }
		if (pd[w]) {  mSceneMgr->destroyParticleSystem(pd[w]);   pd[w]=0;  }
		if (pflW[w]) {  mSceneMgr->destroyParticleSystem(pflW[w]);   pflW[w]=0;  }
		if (pflM[w]) {  mSceneMgr->destroyParticleSystem(pflM[w]);   pflM[w]=0;  }
		if (pflMs[w]) {  mSceneMgr->destroyParticleSystem(pflMs[w]);   pflMs[w]=0;  }  
		if (ndBrake[w]) mSceneMgr->destroySceneNode(ndBrake[w]);
	}
	for (int i=0; i < 2; i++)
		if (pb[i]) {  mSceneMgr->destroyParticleSystem(pb[i]);   pb[i]=0;  }
	if (ph)  {  mSceneMgr->destroyParticleSystem(ph);   ph=0;  }
						
	if (pMainNode) mSceneMgr->destroySceneNode(pMainNode);
	
	// Destroy resource group, will also destroy all resources in it
	if (Ogre::ResourceGroupManager::getSingleton().resourceGroupExists("Car" + toStr(iIndex)))
		Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup("Car" + toStr(iIndex));
}


//-------------------------------------------------------------------------------------------------------
//  Create
//-------------------------------------------------------------------------------------------------------
void CarModel::Create(int car)
{
	if (!pCar) return;
	String strI = toStr(iIndex);
	
	bool ghost = eType == CT_GHOST && pSet->rpl_alpha;  //1 || for ghost test
	
	//  Resource locations -----------------------------------------
	/// Add a resource group for this car
	Ogre::ResourceGroupManager::getSingleton().createResourceGroup("Car" + strI);
	resCar = PATHMANAGER::GetCarPath() + "/" + sDirname + "/textures";
	Ogre::Root::getSingletonPtr()->addResourceLocation(PATHMANAGER::GetCarPath() + "/" + sDirname, "FileSystem", "Car" + strI);
	Ogre::Root::getSingletonPtr()->addResourceLocation(PATHMANAGER::GetCarPath() + "/" + sDirname + "/textures", "FileSystem", "Car" + strI);
	
	String sCar = resCar + "/" + sDirname;
	
	pMainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

	//  --------  Follow Camera  --------
	if (mCamera)
	{
		fCam = new FollowCamera(mCamera, pSet);
		fCam->loadCameras();
		
		//  set in-car camera position to driver position
		Ogre::Vector3 driver_view_position = Vector3(pCar->driver_view_position[0], pCar->driver_view_position[2], -pCar->driver_view_position[1]);
		
		Ogre::Vector3 hood_view_position = Vector3(pCar->hood_view_position[0], pCar->hood_view_position[2], -pCar->hood_view_position[1]);
		
		for (std::vector<CameraAngle*>::iterator it=fCam->mCameraAngles.begin();
			it!=fCam->mCameraAngles.end(); ++it)
		{
			if ((*it)->mName == "Car driver")
				(*it)->mOffset = driver_view_position;
				
			if ((*it)->mName == "Car bonnet")
				(*it)->mOffset = hood_view_position;
		}
	}
	
	RecreateMaterials();
		
	//  reflection
	CreateReflection();

	//  car Models:  body, interior, glass  -------
	//vis flags:  2 not rendered in reflections  16 off by in-car camera
	SceneNode* ncart = pMainNode->createChildSceneNode();
	
	Ogre::Vector3 vPofs(0,0,0);
	AxisAlignedBox bodyBox;  Ogre::uint8 g = RQG_CarGhost;
	
	//  body  ----------------------

	sCar = resCar + "/" + sDirname;
	String sCar2 = PATHMANAGER::GetCarPath() + "/" + sDirname + "/" + sDirname;
	if (FileExists(sCar2 + "_body.mesh"))
	{
		Entity* eCar = mSceneMgr->createEntity("Car"+ strI, sDirname + "_" + "body.mesh", "Car" + strI);

		//eCar->setCastShadows(false);
		bodyBox = eCar->getBoundingBox();
		if (ghost)  {  eCar->setRenderQueueGroup(g);  eCar->setCastShadows(false);  }
		ncart->attachObject(eCar);  eCar->setVisibilityFlags(RV_Car);
	}else{
		ManualObject* mCar = CreateModel(mSceneMgr, sMtr[Mtr_CarBody], &pCar->bodymodel.mesh, vPofs, false, false, "Car"+strI);
		if (mCar){	bodyBox = mCar->getBoundingBox();
			if (ghost)  {  mCar->setRenderQueueGroup(g);  mCar->setCastShadows(false);  }
			ncart->attachObject(mCar);  mCar->setVisibilityFlags(RV_Car);  }
	}

	//  interior  ----------------------
	vPofs = Vector3(pCar->vInteriorOffset[0],pCar->vInteriorOffset[1],pCar->vInteriorOffset[2]);  //x+ back y+ down z+ right

	if (!ghost)
	if (FileExists(sCar2 + "_interior.mesh"))
	{
		Entity* eInter = mSceneMgr->createEntity("Car.interior"+ strI, sDirname + "_" + "interior.mesh", "Car" + strI);
		//eInter->setCastShadows(false);
		if (ghost)  {  eInter->setRenderQueueGroup(g);  eInter->setCastShadows(false);  }
		ncart->attachObject(eInter);  eInter->setVisibilityFlags(RV_Car);
	}else{
		ManualObject* mInter = CreateModel(mSceneMgr, sMtr[Mtr_CarInterior],&pCar->interiormodel.mesh, vPofs, false, false, "Car.interior"+strI);
		//mInter->setCastShadows(false);
		if (mInter){  if (ghost)  {  mInter->setRenderQueueGroup(g);  mInter->setCastShadows(false);  }
			ncart->attachObject(mInter);  mInter->setVisibilityFlags(RV_Car);  }
	}
	
	//  glass  ----------------------
	vPofs = Vector3(0,0,0);

	if (FileExists(sCar2 + "_glass.mesh"))
	{
		Entity* eGlass = mSceneMgr->createEntity("Car.glass"+ strI, sDirname + "_" + "glass.mesh", "Car" + strI);
		if (ghost)  {  eGlass->setRenderQueueGroup(g);  eGlass->setCastShadows(false);  }  else
			eGlass->setRenderQueueGroup(RQG_CarGlass);  eGlass->setVisibilityFlags(RV_CarGlass);
		ncart->attachObject(eGlass);
	}else{
		ManualObject* mGlass = CreateModel(mSceneMgr, sMtr[Mtr_CarGlass], &pCar->glassmodel.mesh, vPofs, false, false, "Car.glass"+strI);
		if (mGlass){  mGlass->setRenderQueueGroup(ghost ? g : RQG_CarGlass);  if (ghost)  mGlass->setCastShadows(false);
			ncart->attachObject(mGlass);  mGlass->setVisibilityFlags(RV_CarGlass);  }
	}
	
	///  save .mesh
	/**  MeshPtr mpCar = mCar->convertToMesh("MeshCar");
	Ogre::MeshSerializer* msr = new Ogre::MeshSerializer();
	msr->exportMesh(mpCar.getPointer(), "car.mesh");/**/


	//  wheels  ----------------------
	for (int w=0; w < 4; ++w)
	{
		// only 1 mesh for both?
		String siw = "Wheel"+ strI + "_" +toStr(w);
		if (FileExists(sCar2 + "_wheel.mesh"))
		{
			Entity* eWh = mSceneMgr->createEntity(siw, sDirname + "_wheel.mesh", "Car" + strI);
			if (ghost)  {  eWh->setRenderQueueGroup(g);  eWh->setCastShadows(false);  }
			ndWh[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
			ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(RV_Car);
		}else{
			if (w < 2 && FileExists(sCar2 + "_wheel_front.mesh"))
			{
				Entity* eWh = mSceneMgr->createEntity(siw, sDirname + "_" + "wheel_front.mesh", "Car" + strI);
				if (ghost)  {  eWh->setRenderQueueGroup(g);  eWh->setCastShadows(false);  }
				ndWh[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(RV_Car);
			}else
			if (FileExists(sCar2 + "_wheel_rear.mesh"))
			{
				Entity* eWh = mSceneMgr->createEntity(siw, sDirname + "_" + "wheel_rear.mesh", "Car" + strI);
				if (ghost)  {  eWh->setRenderQueueGroup(g);  eWh->setCastShadows(false);  }
				ndWh[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(RV_Car);
			}else{
				ManualObject* mWh;
				if (w < 2)	mWh = CreateModel(mSceneMgr, sMtr[Mtr_CarTireFront], &pCar->wheelmodelfront.mesh, vPofs, true, false, siw);
				else		mWh = CreateModel(mSceneMgr, sMtr[Mtr_CarTireRear],  &pCar->wheelmodelrear.mesh, vPofs, true, false, siw);
				if (mWh)  {
				if (ghost)  {  mWh->setRenderQueueGroup(g);  mWh->setCastShadows(false);  }
				ndWh[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				ndWh[w]->attachObject(mWh);  mWh->setVisibilityFlags(RV_Car);  }
			}
		}
		
		// brake mesh.. only on rear wheels
		//! todo: add a param to car file to control which wheels have brakes
		if (FileExists(sCar2 + "_brake.mesh"))
		{
			Entity* eBrake = mSceneMgr->createEntity(siw + "_brake", sDirname + "_brake.mesh", "Car" + strI);
			if (ghost)  {  eBrake->setRenderQueueGroup(g);  eBrake->setCastShadows(false);  }
			eBrake->setVisibilityFlags(RV_Car);
			ndBrake[w] = ndWh[w]->createChildSceneNode();
			ndBrake[w]->attachObject(eBrake);
		}
	}


	///  world hit sparks  ------------------------
	//if (!ghost)//-
	if (!ph)  {
		ph = mSceneMgr->createParticleSystem("Hit"+strI, "Sparks");
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
				SceneNode* nb = pMainNode->createChildSceneNode(bcenter+
					Vector3(bsize.x * 0.97, bsize.y * 0.65, bsize.z * 0.65 * (i==0 ? 1 : -1) ));
					//Vector3(1.9 /*back*/, 0.1 /*up*/, 0.6 * (i==0 ? 1 : -1)/*sides*/ ));
				nb->attachObject(pb[i]);
			}else{
				// use exhaust pos values from car file
				Vector3 pos;
				if (i==0)
					pos = Vector3(pCar->exhaustPosition[0], pCar->exhaustPosition[1], pCar->exhaustPosition[2]);
				else if (!pCar->has2exhausts)
					continue;
				else
					pos = Vector3(pCar->exhaustPosition[0], pCar->exhaustPosition[1], -1*pCar->exhaustPosition[2]);

				SceneNode* nb = pMainNode->createChildSceneNode(pos);
				nb->attachObject(pb[i]); 
			}
			pb[i]->getEmitter(0)->setEmissionRate(0);
		}
	}

	///  wheel emitters  ------------------------
	if (!ghost)
	for (int w=0; w < 4; ++w)
	{
		String siw = strI + "_" +toStr(w);
		if (!ps[w])  
		{
			ps[w] = mSceneMgr->createParticleSystem("Smoke"+siw, sc->sParSmoke);
			ps[w]->setVisibilityFlags(RV_Particles);
			mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ps[w]);		ps[w]->getEmitter(0)->setEmissionRate(0);  }
		if (!pm[w])  {
			pm[w] = mSceneMgr->createParticleSystem("Mud"+siw, sc->sParMud);
			pm[w]->setVisibilityFlags(RV_Particles);
			mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pm[w]);		pm[w]->getEmitter(0)->setEmissionRate(0);  }
		if (!pd[w])  {
			pd[w] = mSceneMgr->createParticleSystem("Dust"+siw, sc->sParDust);
			pd[w]->setVisibilityFlags(RV_Particles);
			mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pd[w]);		pd[w]->getEmitter(0)->setEmissionRate(0);  }

		if (!pflW[w])  {
			pflW[w] = mSceneMgr->createParticleSystem("FlWater"+siw, "FluidWater");
			pflW[w]->setVisibilityFlags(RV_Particles);
			mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pflW[w]);	pflW[w]->getEmitter(0)->setEmissionRate(0);  }
		if (!pflM[w])  {
			pflM[w] = mSceneMgr->createParticleSystem("FlMud"+siw, "FluidMud");
			pflM[w]->setVisibilityFlags(RV_Particles);
			mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pflM[w]);	pflM[w]->getEmitter(0)->setEmissionRate(0);  }
		if (!pflMs[w])  {
			pflMs[w] = mSceneMgr->createParticleSystem("FlMudS"+siw, "FluidMudSoft");
			pflMs[w]->setVisibilityFlags(RV_Particles);
			mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pflMs[w]);	pflMs[w]->getEmitter(0)->setEmissionRate(0);  }

		//  trails
		if (!ndWhE[w])
			ndWhE[w] = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		if (!whTrl[w])  {
			NameValuePairList params;
			params["numberOfChains"] = "1";
			params["maxElements"] = toStr(320 * pSet->trails_len);  //80

			whTrl[w] = (RibbonTrail*)mSceneMgr->createMovableObject("RibbonTrail", &params);
			whTrl[w]->setInitialColour(0, 0.1,0.1,0.1, 0);
			mSceneMgr->getRootSceneNode()->attachObject(whTrl[w]);
			whTrl[w]->setMaterialName("TireTrail");
			whTrl[w]->setCastShadows(false);
			whTrl[w]->addNode(ndWhE[w]);
		}
			whTrl[w]->setTrailLength(90 * pSet->trails_len);  //30
			whTrl[w]->setInitialColour(0, 0.1f,0.1f,0.1f, 0);
			whTrl[w]->setColourChange(0, 0.0,0.0,0.0, /*fade*/0.08f * 1.f / pSet->trails_len);
			whTrl[w]->setInitialWidth(0, 0.16f);  //0.18 0.2
	}

	UpdParsTrails();
			
	setMtrNames();
	
	//  this snippet makes sure the brake texture is pre-loaded.
	//  since it is not used until you actually brake, we have to explicitely declare it
	if (FileExists(sCar + "_body00_brake.png"))
		ResourceGroupManager::getSingleton().declareResource(sDirname + "_body00_brake.png", "Texture", "Car" + strI);
	if (FileExists(sCar + "_body00_add.png"))
		ResourceGroupManager::getSingleton().declareResource(sDirname + "_body00_add.png", "Texture", "Car" + strI);
	
	//  now just preload the whole resource group
	ResourceGroupManager::getSingleton().initialiseResourceGroup("Car" + strI);
	ResourceGroupManager::getSingleton().loadResourceGroup("Car" + strI);
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
	#define chooseMat(s) MaterialManager::getSingleton().resourceExists( "car" + String(s) + "_" + sDirname) ? "car"+String(s) + "_" + sDirname : "car"+String(s)
	
	//  ghost car has no interior, particles, trails and uses same material for all meshes
	if (!ghost)
	{	sMtr[Mtr_CarBody]     = chooseMat("_body");		sMtr[Mtr_CarTireFront]  = chooseMat("tire_front");
		sMtr[Mtr_CarInterior] = chooseMat("_interior");	sMtr[Mtr_CarTireRear]   = chooseMat("tire_rear");
		sMtr[Mtr_CarGlass]    = chooseMat("_glass");
	}else
	for (int i=0; i<NumMaterials; i++)
		sMtr[i] = "car_ghost";  //+s old mtr..

	// copy material to a new material with index
	Ogre::MaterialPtr mat;
	for (int i=0; i<NumMaterials; i++)
	{
		mat = Ogre::MaterialManager::getSingleton().getByName(sMtr[i]);
		if (Ogre::MaterialManager::getSingleton().resourceExists(sMtr[i] + strI))
			Ogre::MaterialManager::getSingleton().remove(sMtr[i] + strI);
		mat->clone(sMtr[i] + strI, false);
		sMtr[i] = sMtr[i] + strI;
	}
	
	if (!ghost)
	for (int i=0; i < NumMaterials; i++)
	{
		MaterialPtr mtr = MaterialManager::getSingleton().getByName(sMtr[i]);
		if (!mtr.isNull())
		{	Material::TechniqueIterator techIt = mtr->getTechniqueIterator();
			while (techIt.hasMoreElements())
			{	Technique* tech = techIt.getNext();
				Technique::PassIterator passIt = tech->getPassIterator();
				while (passIt.hasMoreElements())
				{	Pass* pass = passIt.getNext();
					Pass::TextureUnitStateIterator tusIt = pass->getTextureUnitStateIterator();
					while (tusIt.hasMoreElements())
					{
						TextureUnitState* tus = tusIt.getNext();
						String sTex = tus->getTextureName();  //!..
						
						// only 1 tire mesh?
						if ( (i == Mtr_CarTireFront || i == Mtr_CarTireRear) 
							&& FileExists(sCar + "_wheel.mesh") 
							&& (tus->getTextureName() == String("wheel_front.png") || tus->getTextureName() == String("wheel_rear.png")) )
						{
							// set same texture for both
							tus->setTextureName(String("wheel.png"));
						}
						
						if (!(StringUtil::startsWith(tus->getTextureName(), "ReflectionCube") ||
							tus->getTextureName() == "ReflectionCube" ||
							StringUtil::startsWith(tus->getName(), "shadowmap") ||
							StringUtil::startsWith(tus->getName(), "terrainlightmap") ||
							StringUtil::startsWith(tus->getTextureName(), "flat_n")))
						tus->setTextureName(sDirname + "_" + tus->getTextureName());
		}	}	}	}
		
		// set shader params of the cloned material
		MaterialFactory::getSingleton().setShaderParams(mtr);
	}
	
	ChangeClr(iIndex);
}

void CarModel::setMtrName(const String& entName, const String& mtrName)
{
	Ogre::Entity* ent; Ogre::ManualObject* manual;

	if (mSceneMgr->hasEntity(entName))
	{
		mSceneMgr->getEntity(entName)->setMaterialName(mtrName);
	}
	else if (mSceneMgr->hasManualObject(entName))
	{
		mSceneMgr->getManualObject(entName)->setMaterialName(0, mtrName);
	}
}

void CarModel::setMtrNames()
{
	if (FileExists(resCar + "/" + sDirname + "_body00_add.png")
	 || FileExists(resCar + "/" + sDirname + "_body00_red.png"))
		setMtrName("Car"+toStr(iIndex), sMtr[Mtr_CarBody]);
	setMtrName("Car.interior"+toStr(iIndex), sMtr[Mtr_CarInterior]);
	setMtrName("Car.glass"+toStr(iIndex), sMtr[Mtr_CarGlass]);
	for (int w=0; w<4; ++w)
	{
		setMtrName("Wheel"+toStr(iIndex)+"_"+toStr(w), w < 2 ? sMtr[Mtr_CarTireFront] : sMtr[Mtr_CarTireRear]);
		setMtrName("Wheel"+toStr(iIndex)+"_"+toStr(w)+"_brake", w < 2 ? sMtr[Mtr_CarTireFront] : sMtr[Mtr_CarTireRear]);
	}
}

//  ----------------- Reflection ------------------------
void CarModel::CreateReflection()
{
	pReflect = new CarReflection(pSet, pApp, mSceneMgr, iIndex);
	for (int i=0; i<NumMaterials; i++)
		pReflect->sMtr[i] = sMtr[i];

	pReflect->Create();
}


//  utility - create VDrift model in Ogre
//-------------------------------------------------------------------------------------------------------
ManualObject* CarModel::CreateModel(SceneManager* sceneMgr, const String& mat,
	class VERTEXARRAY* a, Vector3 vPofs, bool flip, bool track, const String& name)
{
	int verts = a->vertices.size();
	if (verts == 0)  return NULL;
	int tcs   = a->texcoords[0].size(); //-
	int norms = a->normals.size();
	int faces = a->faces.size();
	// norms = verts, verts % 3 == 0

	ManualObject* m;
	if (name == "")
		m = sceneMgr->createManualObject();
	else
		m = sceneMgr->createManualObject(name);
	m->begin(mat, RenderOperation::OT_TRIANGLE_LIST);

	int t = 0;
	if (track)
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(a->vertices[v+0], a->vertices[v+2], -a->vertices[v+1]);
			if (norms)
			m->normal(	a->normals [v+0], a->normals [v+2], -a->normals [v+1]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; ++f)
			m->index(a->faces[f]);
	}else
	if (flip)
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(a->vertices[v], a->vertices[v+1], a->vertices[v+2]);
			if (norms)
			m->normal(  a->normals [v], a->normals [v+1], a->normals [v+2]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; f += 3)
		{	m->index(a->faces[f+2]);  m->index(a->faces[f+1]);  m->index(a->faces[f]);	}
	}else
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(-a->vertices[v+1]+vPofs.x, -a->vertices[v+2]+vPofs.y, a->vertices[v]+vPofs.z);
			if (norms)
			m->normal(	-a->normals [v+1], -a->normals [v+2], a->normals [v]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; f += 3)
		{	m->index(a->faces[f+2]);  m->index(a->faces[f+1]);  m->index(a->faces[f]);	}
	}
	m->end();
	return m;
}

