#include "pch.h"
#include "Defines.h"
#include "CarModel.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/mathvector.h"
#include "../vdrift/track.h"
#include "../vdrift/game.h"
//#include "../ogre/OgreGame.h"
#include "SplitScreenManager.h"
#include "common/SceneXml.h"
#include "FollowCamera.h"
#include "CarReflection.h"
#include "../road/Road.h"

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


CarModel::CarModel(unsigned int index, eCarType type, const std::string name,
	Ogre::SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* s, Ogre::Camera* cam, App* app) :
	hue(0), sat(0), val(0), fCam(0), pMainNode(0), pCar(0), terrain(0), resCar(""), mCamera(0), pReflect(0), pApp(app)
{
	iIndex = index;  sDirname = name;  pSceneMgr = sceneMgr;
	pSet = set;  pGame = game;  sc = s;  mCamera = cam;  eType = type;
	bGetStPos = true;
	
	MATHVECTOR<float, 3> offset;
	offset.Set(5*iIndex,5*iIndex,0); // 5*sqrt(2) m distance between cars
	/// TODO: some quaternion magic to align the cars along track start orientation
	// 4 car positions from 1, step width,length ...
	
	if (type != CT_GHOST)  // ghost has pCar, dont create
	{
		MATHVECTOR<float, 3> pos(0,10,0);
		QUATERNION<float> rot;
		pos = pGame->track.GetStart(0/*iIndex*/).first;
		rot = pGame->track.GetStart(0/*iIndex*/).second;

		pCar = pGame->LoadCar(sDirname, pos + offset, rot, true, false);
		if (!pCar)  LogO("Error creating car " + sDirname);
	}
	
	for (int w = 0; w < 4; ++w)
	{	ps[w] = 0;  pm[w] = 0;  pd[w] = 0;
		ndWh[w] = 0;  ndWhE[w] = 0; whTrl[w] = 0;
		wht[w] = 0.f;  whTerMtr[w] = 0; }
	for (int i=0; i < 2; i++)
		pb[i] = 0;
}

CarModel::~CarModel()
{
	delete pReflect;  pReflect = 0;
	
	delete fCam;  fCam = 0;
	pSceneMgr->destroyCamera("CarCamera" + toStr(iIndex));
	
	//  hide trails
	for (int w=0; w<4; ++w)  if (whTrl[w])  {	wht[w] = 0.f;
		whTrl[w]->setVisible(false);	whTrl[w]->setInitialColour(0, 0.5,0.5,0.5, 0);	}

	// destroy cloned materials
	for (int i=0; i<NumMaterials; i++)
		Ogre::MaterialManager::getSingleton().remove(sMtr[i]);
	
	// Destroy par sys
	for (int w=0; w < 4; w++)  {
		if (ps[w]) {  pSceneMgr->destroyParticleSystem(ps[w]);   ps[w]=0;  }
		if (pm[w]) {  pSceneMgr->destroyParticleSystem(pm[w]);   pm[w]=0;  }
		if (pd[w]) {  pSceneMgr->destroyParticleSystem(pd[w]);   pd[w]=0;  }  }
	for (int i=0; i < 2; i++)
		if (pb[i]) {  pSceneMgr->destroyParticleSystem(pb[i]);   pb[i]=0;  }
						
	if (pMainNode) pSceneMgr->destroySceneNode(pMainNode);
	if (pSceneMgr->hasEntity("Car")) pSceneMgr->destroyEntity("Car");
	if (pSceneMgr->hasEntity("Car.interior")) pSceneMgr->destroyEntity("Car.interior");
	if (pSceneMgr->hasEntity("Car.glass")) pSceneMgr->destroyEntity("Car.glass");
	
	// Destroy resource group, will also destroy all resources in it
	Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup("Car" + toStr(iIndex));
}

void CarModel::setVisible(bool vis)
{
	//if (pMainNode->getVisible() == vis)  return;  //opt..
	pMainNode->setVisible(vis);
	for (int w=0; w < 4; ++w)
		ndWh[w]->setVisible(vis);
	UpdParsTrails(vis);
}

void CarModel::Update(PosInfo& posInfo, float time)
{	
	if (!posInfo.bNew)  return;  // new only
	posInfo.bNew = false;
	
	if (!pMainNode) return;
	//  car pos and rot
	pMainNode->setPosition(posInfo.pos);
	pMainNode->setOrientation(posInfo.rot);
	

	//  update particle emitters
	//-----------------------------------------------------------------------------

	//  boost
	if (pSet->particles)
	for (int i=0; i < 2; i++)
	if (pb[i])
	{
		float emitB = posInfo.fboost * 40.f;  // par
		ParticleEmitter* pe = pb[i]->getEmitter(0);
		pe->setEmissionRate(emitB);
	}
	
	//  wheels
	for (int w=0; w < 4; w++)
	{
		float wR = posInfo.whR[w];
		ndWh[w]->setPosition(posInfo.whPos[w]);
		ndWh[w]->setOrientation(posInfo.whRot[w]);

		int whMtr = posInfo.whMtr[w];  //whTerMtr[w];
		float whVel = posInfo.whVel[w] * 3.6f;  //kmh
		float slide = posInfo.whSlide[w], squeal = posInfo.whSqueal[w];
		float onGr = slide < 0.f ? 0.f : 1.f;

		//  wheel temp
		wht[w] += squeal * time * 7;
		wht[w] -= time*6;  if (wht[w] < 0.f)  wht[w] = 0.f;

		///  emit rates +
		Real emitS = 0.f, emitM = 0.f, emitD = 0.f;  //paused

		if (!pGame->pause)
		{
			 Real sq = squeal* std::min(1.f, wht[w]), l = pSet->particles_len * onGr;
			 emitS = sq * (whVel * 30) * l *0.3f;  //..
			 emitM = slide < 1.4f ? 0.f :  (8.f * sq * std::min(5.f, slide) * l);
			 emitD = (std::min(140.f, whVel) / 3.5f + slide * 1.f ) * l;  
			 //  resume
			 pd[w]->setSpeedFactor(1.f);  ps[w]->setSpeedFactor(1.f);  pm[w]->setSpeedFactor(1.f);
			 if (w < 2)  pb[w]->setSpeedFactor(1.f);
		}else{
			 //  stop par sys
			 pd[w]->setSpeedFactor(0.f);  ps[w]->setSpeedFactor(0.f);  pm[w]->setSpeedFactor(0.f);
			 if (w < 2)  pb[w]->setSpeedFactor(0.f);
		}
		Real sizeD = (0.3f + 1.1f * std::min(140.f, whVel) / 140.f) * (w < 2 ? 0.5f : 1.f);

		//  ter mtr factors
		int mtr = std::max(0, std::min(whMtr-1, (int)(sc->td.layers.size()-1)));
		TerLayer& lay = whMtr==0 ? sc->td.layerRoad : sc->td.layersAll[sc->td.layers[mtr]];
		emitD *= lay.dust;  emitM *= lay.mud;  sizeD *= lay.dustS;  emitS *= lay.smoke;

		//  par emit
		Vector3 vpos = posInfo.whPos[w];
		if (pSet->particles)
		{
			if (ps[w] && sc->td.layerRoad.smoke > 0.f/*&& !sc->ter*/)  // only at vdr road
			{
				ParticleEmitter* pe = ps[w]->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.7f); // 0.218
				/**/ps[w]->getAffector(0)->setParameter("alpha", toStr(-0.4f - 0.07f/2.4f * whVel));
				/**/pe->setTimeToLive( std::max(0.1, 2 - whVel/2.4f * 0.04) );  // fade,live
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitS);
			}
			if (pm[w])	//  mud
			{	ParticleEmitter* pe = pm[w]->getEmitter(0);
				//pe->setDimensions(sizeM,sizeM);
				pe->setPosition(vpos + posInfo.carY * wR*0.7f); // 0.218
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitM);
			}
			if (pd[w])	//  dust
			{	pd[w]->setDefaultDimensions(sizeD,sizeD);
				ParticleEmitter* pe = pd[w]->getEmitter(0);
				pe->setPosition(vpos + posInfo.carY * wR*0.51f ); // 0.16
				pe->setDirection(-posInfo.carY);	pe->setEmissionRate(emitD);
			}
		}

		//  update trails h+
		if (pSet->trails)  {
			if (ndWhE[w])
			{	Vector3 vp = vpos + posInfo.carY * wR*0.72f;  // 0.22
				if (terrain && whMtr > 0)
					vp.y = terrain->getHeightAtWorldPosition(vp) + 0.05f;
					//if (/*whOnRoad[w]*/whMtr > 0 && road)  // on road, add ofs
					//	vp.y += road->fHeight;	}/**/
				ndWhE[w]->setPosition(vp);
			}
			float al = 0.5f * /*squeal*/ std::min(1.f, 0.7f * wht[w]) * onGr;  // par+
			if (whTrl[w])	whTrl[w]->setInitialColour(0,
				lay.tclr.r,lay.tclr.g,lay.tclr.b, lay.tclr.a * al/**/);
		}
	}

	// Reflection
	pReflect->camPosition = pMainNode->getPosition();
	
	// blendmaps
	UpdWhTerMtr();
}


//-------------------------------------------------------------------------------------------------------
//  Create
//-------------------------------------------------------------------------------------------------------
void CarModel::Create()
{
	if (!pCar) return;
	
	//  Resource locations -----------------------------------------
	/// Add a resource group for this car
	Ogre::ResourceGroupManager::getSingleton().createResourceGroup("Car" + toStr(iIndex));
	Ogre::Root::getSingletonPtr()->addResourceLocation(PATHMANAGER::GetCacheDir(), "FileSystem");
	resCar = PATHMANAGER::GetCarPath() + "/" + sDirname + "/textures";
	Ogre::Root::getSingletonPtr()->addResourceLocation(resCar, "FileSystem", "Car" + toStr(iIndex));
	
	// Change color here - cache has to be created before loading model
	ChangeClr();
	
	pMainNode = pSceneMgr->getRootSceneNode()->createChildSceneNode();

	//  --------  Follow Camera  --------
	if (mCamera)
	{
		fCam = new FollowCamera(mCamera);
		fCam->mGoalNode = pMainNode;
		fCam->loadCameras();
	}

	// --------- Materials  -------------------
	String s = pSet->shaders == 0 ? "_old" : "";
	sMtr[Mtr_CarBody]		= "car_body"+s;			sMtr[Mtr_CarTireFront]	= "cartire_front"+s;
	sMtr[Mtr_CarInterior]	= "car_interior"+s;		sMtr[Mtr_CarTireRear]	= "cartire_rear"+s;
	sMtr[Mtr_CarGlass]		= "car_glass"+s;
	// copy material to a new material with index
	Ogre::MaterialPtr mat;
	for (int i=0; i<NumMaterials; i++)
	{
		mat = Ogre::MaterialManager::getSingleton().getByName(sMtr[i]);
		mat->clone(sMtr[i] + toStr(iIndex), false);
		// new mat name
		sMtr[i] = sMtr[i] + toStr(iIndex);
		LogO(" =============== New mat name: " + sMtr[i]);
	}
	// iterate through all materials and set body_dyn.png with correct index, add car prefix to other textures
	for (int i=0; i < NumMaterials; i++)
	{
		MaterialPtr mtr = (MaterialPtr)MaterialManager::getSingleton().getByName(sMtr[i]);
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
						// normal maps
						if (tus->getTextureName() == "flat_n.png")
						{
							// interior normal map
							if (i == Mtr_CarInterior 
								&& FileExists(resCar + "/" + sDirname + "_interior_normal.png") 
								&& tus->getTextureName() == "flat_n.png")
							{
								tus->setTextureName(sDirname + "_interior_normal.png");
								continue;
							}
							// glass normal map
							if (i == Mtr_CarGlass && FileExists(resCar + "/" + sDirname + "_glass_normal.png")
								&& tus->getTextureName() == "flat_n.png")
							{
								tus->setTextureName(sDirname + "_glass_normal.png");
								continue;
							}
							// tire normal map
							if ( (i == Mtr_CarTireFront || i == Mtr_CarTireRear) 
								&& FileExists(resCar + "/" + sDirname + "_wheel_normal.png") 
								&& tus->getTextureName() == "flat_n.png")
							{
								tus->setTextureName(sDirname + "_wheel_normal.png");
								continue;
							}
						}
						// only 1 tire mesh?
						if ( (i == Mtr_CarTireFront || i == Mtr_CarTireRear) 
							&& FileExists(resCar + "/" + sDirname + "_wheel.mesh") 
							&& (tus->getTextureName() == "wheel_front.png" || tus->getTextureName() == "wheel_rear.png") )
						{
							// set same texture for both
							tus->setTextureName("wheel.png");
						}
						
						if (tus->getTextureName() == "body_dyn.png")
							tus->setTextureName("body_dyn" + toStr(iIndex) + ".png");
						else if (!(StringUtil::startsWith(tus->getTextureName(), "ReflectionCube") || StringUtil::startsWith(tus->getTextureName(), "body_dyn") || tus->getTextureName() == "ReflectionCube"))
							tus->setTextureName(sDirname + "_" + tus->getTextureName());
	}	}	}	}	}
	
	// reflection
	CreateReflection();

	//  car Models:  body, interior, glass  -------
	//vis flags:  2 not rendered in reflections  16 off by in-car camera
	SceneNode* ncart = pMainNode->createChildSceneNode();
	
	//  body  ----------------------
	Ogre::Vector3 vPofs(0,0,0);
	AxisAlignedBox bodyBox;

	if (FileExists(resCar + "/" + sDirname + "_" + "body.mesh"))
	{
		Entity* eCar = pSceneMgr->createEntity("Car"+ toStr(iIndex), sDirname + "_" + "body.mesh", "Car" + toStr(iIndex));
		if (FileExists(resCar + "/" + sDirname + "_" + "body00_add.png") && FileExists(resCar + "/" + sDirname + "_" + "body00_red.png"))
			eCar->setMaterialName(sMtr[Mtr_CarBody]);
		bodyBox = eCar->getBoundingBox();
		ncart->attachObject(eCar);  eCar->setVisibilityFlags(2);
	}else{
		ManualObject* mCar = CreateModel(pSceneMgr, sMtr[Mtr_CarBody], &pCar->bodymodel.mesh, vPofs);
		bodyBox = mCar->getBoundingBox();
		if (mCar){	ncart->attachObject(mCar);  mCar->setVisibilityFlags(2);  }
	}

	//  interior  ----------------------
	vPofs = Vector3(pCar->vInteriorOffset[0],pCar->vInteriorOffset[1],pCar->vInteriorOffset[2]);  //x+ back y+ down z+ right

	if (FileExists(resCar + "/" + sDirname + "_" + "interior.mesh"))
	{
		Entity* eInter = pSceneMgr->createEntity("Car.interior"+ toStr(iIndex), sDirname + "_" + "interior.mesh", "Car" + toStr(iIndex));
		eInter->setMaterialName(sMtr[Mtr_CarInterior]);
		ncart->attachObject(eInter);  eInter->setVisibilityFlags(2);
	}else{
		ManualObject* mInter = CreateModel(pSceneMgr, sMtr[Mtr_CarInterior],&pCar->interiormodel.mesh, vPofs);
		if (mInter){  ncart->attachObject(mInter);  mInter->setVisibilityFlags(2);  }
	}
	
	//  glass  ----------------------
	vPofs = Vector3(0,0,0);

	if (FileExists(resCar + "/" + sDirname + "_" + "glass.mesh"))
	{
		Entity* eGlass = pSceneMgr->createEntity("Car.glass"+ toStr(iIndex), sDirname + "_" + "glass.mesh", "Car" + toStr(iIndex));
		eGlass->setMaterialName(sMtr[Mtr_CarGlass]);
		eGlass->setRenderQueueGroup(RENDER_QUEUE_8);  eGlass->setVisibilityFlags(16);
		ncart->attachObject(eGlass);
	}else{
		ManualObject* mGlass = CreateModel(pSceneMgr, sMtr[Mtr_CarGlass], &pCar->glassmodel.mesh, vPofs);
		if (mGlass){  ncart->attachObject(mGlass);	mGlass->setRenderQueueGroup(RENDER_QUEUE_8);  mGlass->setVisibilityFlags(16);  }
	}
	
	///  save .mesh
	/**  MeshPtr mpCar = mCar->convertToMesh("MeshCar");
	Ogre::MeshSerializer* msr = new Ogre::MeshSerializer();
	msr->exportMesh(mpCar.getPointer(), "car.mesh");/**/


	//  wheels  ----------------------
	for (int w=0; w < 4; w++)
	{
		// only 1 mesh for both?
		if (FileExists(resCar + "/" + sDirname + "_wheel.mesh"))
		{
			Entity* eWh = pSceneMgr->createEntity("Wheel"+ toStr(iIndex) + "_" +toStr(w), sDirname + "_wheel.mesh", "Car" + toStr(iIndex));
			eWh->setMaterialName(sMtr[Mtr_CarTireFront]);
			ndWh[w] = pSceneMgr->getRootSceneNode()->createChildSceneNode();
			ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(2);
		}
		else
		{
			if (w < 2 && FileExists(resCar + "/" + sDirname + "_wheel_front.mesh"))
			{
				Entity* eWh = pSceneMgr->createEntity("Wheel"+ toStr(iIndex) + "_" +toStr(w), sDirname + "_" + "wheel_front.mesh", "Car" + toStr(iIndex));
				eWh->setMaterialName(sMtr[Mtr_CarTireFront]);
				ndWh[w] = pSceneMgr->getRootSceneNode()->createChildSceneNode();
				ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(2);
			}else
			if (FileExists(resCar + "/" + sDirname + "_" + "wheel_rear.mesh"))
			{
				Entity* eWh = pSceneMgr->createEntity("Wheel"+ toStr(iIndex) + "_" +toStr(w), sDirname + "_" + "wheel_rear.mesh", "Car" + toStr(iIndex));
				eWh->setMaterialName(sMtr[Mtr_CarTireRear]);
				ndWh[w] = pSceneMgr->getRootSceneNode()->createChildSceneNode();
				ndWh[w]->attachObject(eWh);  eWh->setVisibilityFlags(2);
			}else{
				ManualObject* mWh;
				if (w < 2)	mWh = CreateModel(pSceneMgr, sMtr[Mtr_CarTireFront], &pCar->wheelmodelfront.mesh, vPofs, true);
				else		mWh = CreateModel(pSceneMgr, sMtr[Mtr_CarTireRear],  &pCar->wheelmodelrear.mesh, vPofs, true);
				if (mWh)  {
					ndWh[w] = pSceneMgr->getRootSceneNode()->createChildSceneNode();
					ndWh[w]->attachObject(mWh);  mWh->setVisibilityFlags(2);  }
			}
		}
	}


	///  boost emitters  ------------------------
	for (int i=0; i < 2; i++)
	{
		String si = toStr(iIndex) + "_" +toStr(i);
		if (!pb[i])  {
			pb[i] = pSceneMgr->createParticleSystem("Boost"+si, "Boost");
			Vector3 bsize = (bodyBox.getMaximum() - bodyBox.getMinimum())*0.5,
				bcenter = bodyBox.getMaximum() + bodyBox.getMinimum();
			LogO("Car body bbox :  size " + toStr(bsize) + ",  center " + toStr(bcenter));
			SceneNode* nb = pMainNode->createChildSceneNode(bcenter+
				Vector3(bsize.x * 0.97, bsize.y * 0.65, bsize.z * 0.65 * (i==0 ? 1 : -1) ));
				//Vector3(1.9 /*back*/, 0.1 /*up*/, 0.6 * (i==0 ? 1 : -1)/*sides*/ ));
			nb->attachObject(pb[i]);
			pb[i]->getEmitter(0)->setEmissionRate(0);  }
	}

	///  wheel emitters  ------------------------
	for (int w=0; w < 4; w++)
	{
		String siw = toStr(iIndex) + "_" +toStr(w);
		if (!ps[w])  {
			ps[w] = pSceneMgr->createParticleSystem("Smoke"+siw, sc->sParSmoke);
			pSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ps[w]);
			ps[w]->getEmitter(0)->setEmissionRate(0);  }
		if (!pm[w])  {
			pm[w] = pSceneMgr->createParticleSystem("Mud"+siw, sc->sParMud);
			pSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pm[w]);
			pm[w]->getEmitter(0)->setEmissionRate(0);  }
		if (!pd[w])  {
			pd[w] = pSceneMgr->createParticleSystem("Dust"+siw, sc->sParDust);
			pSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(pd[w]);
			pd[w]->getEmitter(0)->setEmissionRate(0);  }

		//  trails
		if (!ndWhE[w])
			ndWhE[w] = pSceneMgr->getRootSceneNode()->createChildSceneNode();

		if (!whTrl[w])  {
			NameValuePairList params;
			params["numberOfChains"] = "1";
			params["maxElements"] = toStr(320 * pSet->trails_len);  //80

			whTrl[w] = (RibbonTrail*)pSceneMgr->createMovableObject("RibbonTrail", &params);
			whTrl[w]->setInitialColour(0, 0.1,0.1,0.1, 0);
			pSceneMgr->getRootSceneNode()->attachObject(whTrl[w]);
			whTrl[w]->setMaterialName("TireTrail");
			whTrl[w]->setCastShadows(false);
			whTrl[w]->addNode(ndWhE[w]);
		}
			whTrl[w]->setTrailLength(90 * pSet->trails_len);  //30
			whTrl[w]->setInitialColour(0, 0.1,0.1,0.1, 0);
			whTrl[w]->setColourChange(0, 0.0,0.0,0.0, /*fade*/0.08 * 1.f / pSet->trails_len);
			whTrl[w]->setInitialWidth(0, 0.16);  //0.18 0.2
	}

	UpdParsTrails();
	
	//  reload car materials, omit car and road
	//int i0 = pApp->bRplPlay ? 0 : 1;
	for (int i = 0; i < NumMaterials; ++i)
		ReloadTex(sMtr[i]);
}

//  ----------------- Reflection ------------------------
void CarModel::CreateReflection()
{
	pReflect = new CarReflection(pSet, pSceneMgr, iIndex);
	for (int i=0; i<NumMaterials; i++)
		pReflect->sMtr[i] = sMtr[i];

	pReflect->Create();
}


void CarModel::UpdParsTrails(bool visible)
{
	bool vis = visible && pSet->particles;
	for (int w=0; w < 4; w++)
	{
		Ogre::uint8 grp = RENDER_QUEUE_9;  //9=road  after glass
		if (w < 2 &&
			pb[w])	{	pb[w]->setVisible(vis);  pb[w]->setRenderQueueGroup(grp);  }
		if (whTrl[w]){  whTrl[w]->setVisible(visible && pSet->trails);  whTrl[w]->setRenderQueueGroup(grp);  }  grp += 2;
		if (ps[w])	{	ps[w]->setVisible(vis);  ps[w]->setRenderQueueGroup(grp);  }  // vdr only && !sc.ter
		if (pm[w])	{	pm[w]->setVisible(vis);  pm[w]->setRenderQueueGroup(grp);  }
		if (pd[w])	{	pd[w]->setVisible(vis);  pd[w]->setRenderQueueGroup(grp);  }
	}
}


///  terrain mtr from blend maps
//-------------------------------------------------------------------------------------------------------
void CarModel::UpdWhTerMtr()
{
	if (!pCar || !ndWh[0])  return;
	if (!terrain || !blendMtr)	// vdr trk
	{
		for (int i=0; i<4; ++i)  // for particles/trails only
			whTerMtr[i] = pCar->dynamics.bWhOnRoad[i] ? 0 : 1;
		return;
	}

	int t = blendMapSize;
	Real tws = sc->td.fTerWorldSize;

	//  wheels
	for (int i=0; i<4; ++i)
	{
		Vector3 w = ndWh[i]->getPosition();
		int mx = (w.x + 0.5*tws)/tws*t, my = (w.z + 0.5*tws)/tws*t;
		mx = std::max(0,std::min(t-1, mx)), my = std::max(0,std::min(t-1, my));

		int mtr = blendMtr[my*t + mx];
		if (pCar->dynamics.bWhOnRoad[i])
			mtr = 0;
		whTerMtr[i] = mtr;

		///  vdr set surface for wheel
		TRACKSURFACE* tsu = &pGame->track.tracksurfaces[mtr];
		pCar->dynamics.terSurf[i] = tsu;
		pCar->dynamics.bTerrain = true;
	}
}


//-------------------------------------------------------------------------------------------------------
//  utils
//-------------------------------------------------------------------------------------------------------

void CarModel::ChangeClr()
{
	bool add = 1;
	Image ima;	try{
		ima.load(sDirname + "_body00_add.png", "Car" + toStr(iIndex));  // add, not colored
	}catch(...){  add = 0;  }
	uchar* da = 0;  size_t incRow,incRowA=0, inc1=0,inc1A=0;
	if (add)
	{	PixelBox pba = ima.getPixelBox();
		da = (uchar*)pba.data;  incRowA = pba.rowPitch;
		inc1A = PixelUtil::getNumElemBytes(pba.format);
	}
	String svName = PATHMANAGER::GetCacheDir() + "/body_dyn" + toStr(iIndex) + ".png";  // dynamic
	Image im;  try{
		im.load(sDirname + "_body00_red.png", "Car" + toStr(iIndex));  // original red diffuse
	}catch(...){  return;  }
	if (im.getWidth())
	{
		PixelBox pb = im.getPixelBox();
		size_t xw = pb.getWidth(), yw = pb.getHeight();

		uchar* d = (uchar*)pb.data;  incRow = pb.rowPitch;
		inc1 = PixelUtil::getNumElemBytes(pb.format);

		//Ogre::LogManager::getSingleton().logMessage(
			//"img clr +++  w "+toStr(xw)+"  h "+toStr(yw)+"  pf "+toStr(pb.format)+"  iA "+toStr(inc1A));

		size_t x,y,a,aa;
		for (y = 0; y < yw; ++y)
		{	a = y*incRow*inc1, aa = y*incRowA*inc1A;
		for (x = 0; x < xw; ++x)
		{
			uchar r,g,b;
			if (da && da[aa+3] > 60)  // adding area (not transparent)
			{	r = da[aa];  g = da[aa+1];  b = da[aa+2];	}
			else
			{	r = d[a], g = d[a+1], b = d[a+2];  // get
				ColourValue c(r/255.f,g/255.f,b/255.f);  //

				Real h,s,v;  // hue shift
				c.getHSB(&h,&s,&v);
				h += pSet->car_hue;  if (h>1.f) h-=1.f;  // 0..1
				s += pSet->car_sat;  // -1..1
				v += pSet->car_val;
				c.setHSB(h,s,v);

				r = c.r*255;  g = c.g*255;  b = c.b*255;  // set
			}
			d[a] = r;  d[a+1] = g;  d[a+2] = b;	 // write back
			a += inc1;  aa += inc1A;  // next pixel
		}	}
	}
	im.save(svName);
	ReloadTex(sMtr[Mtr_CarBody]);
}


ManualObject* CarModel::CreateModel(SceneManager* sceneMgr, const String& mat, class VERTEXARRAY* a, Vector3 vPofs, bool flip, bool track)
{
	int verts = a->vertices.size();
	if (verts == 0)  return NULL;
	int tcs   = a->texcoords[0].size(); //-
	int norms = a->normals.size();
	int faces = a->faces.size();
	// norms = verts, verts % 3 == 0

	ManualObject* m = sceneMgr->createManualObject();
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


void CarModel::ReloadTex(String mtrName)
{
	MaterialPtr mtr = (MaterialPtr)MaterialManager::getSingleton().getByName(mtrName);
	if (!mtr.isNull())
	{	Material::TechniqueIterator techIt = mtr->getTechniqueIterator();
		while (techIt.hasMoreElements())
		{	Technique* tech = techIt.getNext();
			Technique::PassIterator passIt = tech->getPassIterator();
			while (passIt.hasMoreElements())
			{	Pass* pass = passIt.getNext();
				Pass::TextureUnitStateIterator tusIt = pass->getTextureUnitStateIterator();
				while (tusIt.hasMoreElements())
				{	TextureUnitState* tus = tusIt.getNext();  String name = tus->getTextureName();
					if (! (Ogre::StringUtil::startsWith(name, "ReflectionCube", false) || name == "ReflectionCube") )
					{
						Ogre::LogManager::getSingletonPtr()->logMessage( "Tex Reload: " + name );
						TexturePtr tex = (TexturePtr)Ogre::TextureManager::getSingleton().getByName( name );
						if (!tex.isNull())
						{							
							tex->reload();
						}
					}
				}
	}	}	}	
}

