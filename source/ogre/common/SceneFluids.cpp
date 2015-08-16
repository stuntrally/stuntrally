#include "pch.h"
#include "RenderConst.h"
#include "Def_Str.h"
#include "data/SceneXml.h"
#include "data/FluidsXml.h"
#include "data/CData.h"
#include "ShapeData.h"
#include "WaterRTT.h"
#include "CScene.h"
#ifdef SR_EDITOR
	#include "../../editor/CApp.h"
	#include "../../editor/settings.h"
	#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#else
	#include "../CGame.h"
	#include "../../vdrift/game.h"
	//#include "../../vdrift/settings.h"
#endif
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <OgreManualObject.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreMesh.h>
#include <OgreTimer.h>
#include "../shiny/Main/Factory.hpp"
using namespace Ogre;



///  create Fluid areas  . . . . . . . 
//----------------------------------------------------------------------------------------------------------------------
void CScene::CreateFluids()
{
	vFlNd.clear();  vFlEnt.clear();  vFlSMesh.clear();
	#ifdef SR_EDITOR
	app->UpdFluidBox();
	#endif
	if (!mWaterRTT->mNdFluidsRoot)
		mWaterRTT->mNdFluidsRoot = app->mSceneMgr->getRootSceneNode()->createChildSceneNode("FluidsRootNode");
			
	for (int i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		//  plane
		Plane p;  p.normal = Vector3::UNIT_Y;  p.d = 0;
		String smesh = "WaterMesh"+toStr(i);
		MeshPtr mesh = MeshManager::getSingleton().createPlane( smesh, rgDef,
			p, fb.size.x,fb.size.z, 6,6, true, 1, fb.tile.x*fb.size.x,fb.tile.y*fb.size.z, Vector3::UNIT_Z);

		Entity* efl = app->mSceneMgr->createEntity("WaterPlane"+toStr(i), "WaterMesh"+toStr(i));
		unsigned short src,dest;
		if (!mesh->suggestTangentVectorBuildParams(VES_TANGENT, src,dest))
			mesh->buildTangentVectors(VES_TANGENT, src,dest);

		String sMtr = fb.id == -1 ? "" : data->fluids->fls[fb.id].material;  //"Water"+toStr(1+fb.type)
		MaterialPtr mtr = MaterialManager::getSingleton().getByName(sMtr);  //temp

		efl->setMaterial(mtr);  efl->setCastShadows(false);
		efl->setRenderQueueGroup(RQG_Fluid);  efl->setVisibilityFlags(RV_Terrain);

		SceneNode* nfl = mWaterRTT->mNdFluidsRoot->createChildSceneNode(
			fb.pos/*, Quaternion(Degree(fb.rot.x),Vector3::UNIT_Y)*/);
		nfl->attachObject(efl);

		vFlSMesh.push_back(smesh);  vFlEnt.push_back(efl);  vFlNd.push_back(nfl);

		#ifndef SR_EDITOR  // game
		CreateBltFluids();
		#endif
	}		
}

void CScene::CreateBltFluids()
{
	for (int i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		const FluidParams& fp = sc->pFluidsXml->fls[fb.id];

		///  add bullet trigger box   . . . . . . . . .
		btVector3 pc(fb.pos.x, -fb.pos.z, fb.pos.y -fb.size.y/2);  // center
		btTransform tr;  tr.setIdentity();  tr.setOrigin(pc);
		//tr.setRotation(btQuaternion(0, 0, fb.rot.x*PI_d/180.f));

		btCollisionShape* bshp = 0;
		btScalar t = sc->td.fTerWorldSize*0.5f;  // not bigger than terrain
		bshp = new btBoxShape(btVector3(std::min(t, fb.size.x*0.5f), std::min(t, fb.size.z*0.5f), fb.size.y*0.5f));

		//  solid surf
		size_t id = SU_Fluid;  if (fp.solid)  id += fp.surf;
		bshp->setUserPointer((void*)id);

		btCollisionObject* bco = new btCollisionObject();
		bco->setActivationState(DISABLE_SIMULATION);
		bco->setCollisionShape(bshp);	bco->setWorldTransform(tr);

		if (!fp.solid)  // fluid
			bco->setCollisionFlags(bco->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_NO_CONTACT_RESPONSE/**/);
		else  // solid
		{	bco->setCollisionFlags(bco->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT);
			bco->setFriction(0.6f);  bco->setRestitution(0.f);  //par?..
		}
		
		bco->setUserPointer(new ShapeData(ST_Fluid, 0, &fb));  ///~~
		#ifndef SR_EDITOR
			app->pGame->collision.world->addCollisionObject(bco);
			app->pGame->collision.shapes.push_back(bshp);
			fb.cobj = bco;
		#else
			app->world->addCollisionObject(bco);
		#endif
	}
	#ifdef SR_EDITOR
	app->UpdObjPick();
	#endif
}

void CScene::DestroyFluids()
{
	for (int i=0; i < vFlSMesh.size(); ++i)
	{
		vFlNd[i]->detachAllObjects();
		app->mSceneMgr->destroyEntity(vFlEnt[i]);
		app->mSceneMgr->destroySceneNode(vFlNd[i]);
		Ogre::MeshManager::getSingleton().remove(vFlSMesh[i]);
	}
	vFlNd.clear();  vFlEnt.clear();  vFlSMesh.clear();
}

#ifdef SR_EDITOR
void App::UpdFluidBox()
{
	int fls = scn->sc->fluids.size();
	bool bFluids = edMode == ED_Fluids && fls > 0 && !bMoveCam;
	if (fls > 0)
		iFlCur = std::max(0, std::min(iFlCur, fls-1));

	if (!ndFluidBox)  return;
	ndFluidBox->setVisible(bFluids);
	if (!bFluids)  return;
	
	FluidBox& fb = scn->sc->fluids[iFlCur];
	ndFluidBox->setPosition(fb.pos);
	ndFluidBox->setScale(fb.size);
}

void App::UpdMtrWaterDepth()
{
	float fl = edMode == ED_Fluids ? 0.f : 1.f;
	sh::Factory::getInstance().setSharedParameter("waterDepth", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(fl)));
}
#endif


//  Water rtt, setup and recreate
void CScene::UpdateWaterRTT(Camera* cam)
{
	mWaterRTT->setRTTSize(ciShadowSizesA[app->pSet->water_rttsize]);
	mWaterRTT->setReflect(app->pSet->water_reflect);
	mWaterRTT->setRefract(app->pSet->water_refract);

	mWaterRTT->setViewerCamera(cam);
	mWaterRTT->mSceneMgr = app->mSceneMgr;

	if (!sc->fluids.empty())  // first fluid's plane
		mWaterRTT->setPlane(Plane(Vector3::UNIT_Y, sc->fluids.front().pos.y));
	mWaterRTT->recreate();
	mWaterRTT->setActive(!sc->fluids.empty());
}
