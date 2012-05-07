#include "pch.h"
#include "../common/RenderConst.h"
#include "../common/MaterialGen/MaterialFactory.h"

#ifdef ROAD_EDITOR
	#include "../common/Defines.h"
	#include "../../editor/OgreApp.h"
	#include "../../editor/settings.h"
	#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#else
	#include "../common/Defines.h"
	#include "../OgreGame.h"
	#include "../../vdrift/game.h"
	//#include "../../vdrift/settings.h"
#endif
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"

#include <OgreManualObject.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreEntity.h>
#include "../common/QTimer.h"

using namespace Ogre;



///  create Fluid areas  . . . . . . . 
//----------------------------------------------------------------------------------------------------------------------
void App::CreateFluids()
{
	#ifdef ROAD_EDITOR
	vFlNd.clear();  vFlEnt.clear();  vFlSMesh.clear();
	UpdFluidBox();
	#endif
	if (!mWaterRTT.mNdFluidsRoot)
		mWaterRTT.mNdFluidsRoot = mSceneMgr->getRootSceneNode()->createChildSceneNode("FluidsRootNode");
			
	for (int i=0; i < sc.fluids.size(); i++)
	{
		FluidBox& fb = sc.fluids[i];
		//  plane
		Plane p;  p.normal = Vector3::UNIT_Y;  p.d = 0;
		String smesh = "WaterMesh"+toStr(i);
		MeshPtr mesh = MeshManager::getSingleton().createPlane(smesh,
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			p, fb.size.x,fb.size.z, 6,6, true, 1, fb.tile.x*fb.size.x,fb.tile.y*fb.size.z, Vector3::UNIT_Z);

		Entity* efl = mSceneMgr->createEntity("WaterPlane"+toStr(i), "WaterMesh"+toStr(i));
		unsigned short src,dest;
		if (!mesh->suggestTangentVectorBuildParams(VES_TANGENT, src,dest))
			mesh->buildTangentVectors(VES_TANGENT, src,dest);

		String sMtr = fb.id == -1 ? "" : fluidsXml.fls[fb.id].material;  //"Water"+toStr(1+fb.type)
		MaterialPtr mtr = MaterialManager::getSingleton().getByName(sMtr);  //par,temp
		if (!mtr.isNull())
		{	//  set sky map
			MaterialPtr mtrSky = MaterialManager::getSingleton().getByName(sc.skyMtr);
			//if (!mtrSky.isNull())  {
			Pass* passSky = mtrSky->getTechnique(0)->getPass(0);
			TextureUnitState* tusSky = passSky->getTextureUnitState(0);

			Pass* pass = mtr->getTechnique(0)->getPass(0);
			TextureUnitState* tus = pass->getTextureUnitState(1);
			if (tus->getName() == "skyMap")  tus->setTextureName(tusSky->getTextureName());
		}
		efl->setMaterial(mtr);  efl->setCastShadows(false);
		efl->setRenderQueueGroup(RQG_Fluid);  efl->setVisibilityFlags(RV_Terrain);

		SceneNode* nfl = mWaterRTT.mNdFluidsRoot->createChildSceneNode(
			fb.pos/*, Quaternion(Degree(fb.rot.x),Vector3::UNIT_Y)*/);
		nfl->attachObject(efl);
		#ifdef ROAD_EDITOR
		vFlSMesh.push_back(smesh);  vFlEnt.push_back(efl);  vFlNd.push_back(nfl);
		#endif

		#ifndef ROAD_EDITOR  // game
		CreateBltFluids();
		#endif
	}		
}

void App::CreateBltFluids()
{
	for (int i=0; i < sc.fluids.size(); i++)
	{
		FluidBox& fb = sc.fluids[i];
		///  add bullet trigger box   . . . . . . . . .
		btVector3 pc(fb.pos.x, -fb.pos.z, fb.pos.y - fb.size.y);  // center
		btTransform tr;  tr.setIdentity();  tr.setOrigin(pc);
		//tr.setRotation(btQuaternion(0, 0, fb.rot.x*PI_d/180.f));

		btCollisionShape* bshp = 0;
		bshp = new btBoxShape(btVector3(fb.size.x/2,fb.size.z/2, fb.size.y));
		//shp->setUserPointer((void*)7777);

		btCollisionObject* bco = new btCollisionObject();
		bco->setActivationState(DISABLE_SIMULATION);
		bco->setCollisionShape(bshp);	bco->setWorldTransform(tr);
		//bco->setFriction(shp->friction);	bco->setRestitution(shp->restitution);
		bco->setCollisionFlags(bco->getCollisionFlags() |
			/*btCollisionObject::CF_STATIC_OBJECT |*/ btCollisionObject::CF_NO_CONTACT_RESPONSE/**/);
		
		bco->setUserPointer(new ShapeData(ST_Fluid, 0, &fb));  ///~~
		#ifndef ROAD_EDITOR
			pGame->collision.world->addCollisionObject(bco);
			pGame->collision.shapes.push_back(bshp);
			fb.cobj = bco;
		#else
			world->addCollisionObject(bco);
		#endif
	}
	#ifdef ROAD_EDITOR
	UpdObjPick();
	#endif
}

#ifdef ROAD_EDITOR
void App::DestroyFluids()
{
	for (int i=0; i < vFlSMesh.size(); ++i)
	{
		vFlNd[i]->detachAllObjects();
		mSceneMgr->destroyEntity(vFlEnt[i]);
		mSceneMgr->destroySceneNode(vFlNd[i]);
		Ogre::MeshManager::getSingleton().remove(vFlSMesh[i]);
	}
	vFlNd.clear();  vFlEnt.clear();  vFlSMesh.clear();
}

void App::UpdFluidBox()
{
	int fls = sc.fluids.size();
	bool bFluids = edMode == ED_Fluids && fls > 0 && !bMoveCam;
	if (fls > 0)
		iFlCur = std::min(iFlCur, fls-1);

	if (!ndFluidBox)  return;
	ndFluidBox->setVisible(bFluids);
	if (!bFluids)  return;
	
	FluidBox& fb = sc.fluids[iFlCur];
	Vector3 p = fb.pos;  p.y -= fb.size.y*0.5f;
	ndFluidBox->setPosition(p);
	ndFluidBox->setScale(fb.size);
}
#endif

//  water rtt
void App::UpdateWaterRTT(Ogre::Camera* cam)
{
	//  water RTT
	mWaterRTT.setViewerCamera(cam);
	mWaterRTT.setRTTSize(ciShadowSizesA[pSet->water_rttsize]);
	mWaterRTT.setReflect(MaterialFactory::getSingleton().getReflect());
	mWaterRTT.setRefract(MaterialFactory::getSingleton().getRefract());
	mWaterRTT.mSceneMgr = mSceneMgr;
	if (!sc.fluids.empty())
		mWaterRTT.setPlane(Plane(Vector3::UNIT_Y, sc.fluids.front().pos.y));
	mWaterRTT.recreate();
	mWaterRTT.setActive(!sc.fluids.empty());
}
