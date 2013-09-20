#include "pch.h"
#include "../common/RenderConst.h"
#include "../common/Def_Str.h"
#include "../common/SceneXml.h"
#include "../common/FluidsXml.h"
#include "../common/CData.h"
#include "../common/ShapeData.h"
#include "../common/WaterRTT.h"
#ifdef SR_EDITOR
	#include "../../editor/CApp.h"
	#include "../../editor/settings.h"
	#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#else
	#include "../CGame.h"
	#include "../../vdrift/game.h"
	//#include "../../vdrift/settings.h"
#endif
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include <OgreManualObject.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include "../common/QTimer.h"
#include "../shiny/Main/Factory.hpp"
using namespace Ogre;



///  create Fluid areas  . . . . . . . 
//----------------------------------------------------------------------------------------------------------------------
void App::CreateFluids()
{
	vFlNd.clear();  vFlEnt.clear();  vFlSMesh.clear();
	#ifdef SR_EDITOR
	UpdFluidBox();
	#endif
	if (!mWaterRTT->mNdFluidsRoot)
		mWaterRTT->mNdFluidsRoot = mSceneMgr->getRootSceneNode()->createChildSceneNode("FluidsRootNode");
			
	for (int i=0; i < sc->fluids.size(); i++)
	{
		FluidBox& fb = sc->fluids[i];
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

		String sMtr = fb.id == -1 ? "" : data->fluids->fls[fb.id].material;  //"Water"+toStr(1+fb.type)
		MaterialPtr mtr = MaterialManager::getSingleton().getByName(sMtr);  //par,temp

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

void App::CreateBltFluids()
{
	for (int i=0; i < sc->fluids.size(); i++)
	{
		FluidBox& fb = sc->fluids[i];
		///  add bullet trigger box   . . . . . . . . .
		btVector3 pc(fb.pos.x, -fb.pos.z, fb.pos.y -fb.size.y/2);  // center
		btTransform tr;  tr.setIdentity();  tr.setOrigin(pc);
		//tr.setRotation(btQuaternion(0, 0, fb.rot.x*PI_d/180.f));

		btCollisionShape* bshp = 0;
		btScalar t = sc->td.fTerWorldSize*0.5f;  // not bigger than terrain
		bshp = new btBoxShape(btVector3(std::min(t, fb.size.x*0.5f), std::min(t, fb.size.z*0.5f), fb.size.y*0.5f));
		//shp->setUserPointer((void*)SU_Fluid);

		btCollisionObject* bco = new btCollisionObject();
		bco->setActivationState(DISABLE_SIMULATION);
		bco->setCollisionShape(bshp);	bco->setWorldTransform(tr);
		//bco->setFriction(shp->friction);	bco->setRestitution(shp->restitution);
		bco->setCollisionFlags(bco->getCollisionFlags() |
			/*btCollisionObject::CF_STATIC_OBJECT |*/ btCollisionObject::CF_NO_CONTACT_RESPONSE/**/);
		
		bco->setUserPointer(new ShapeData(ST_Fluid, 0, &fb));  ///~~
		#ifndef SR_EDITOR
			pGame->collision.world->addCollisionObject(bco);
			pGame->collision.shapes.push_back(bshp);
			fb.cobj = bco;
		#else
			world->addCollisionObject(bco);
		#endif
	}
	#ifdef SR_EDITOR
	UpdObjPick();
	#endif
}

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

#ifdef SR_EDITOR
void App::UpdFluidBox()
{
	int fls = sc->fluids.size();
	bool bFluids = edMode == ED_Fluids && fls > 0 && !bMoveCam;
	if (fls > 0)
		iFlCur = std::max(0, std::min(iFlCur, fls-1));

	if (!ndFluidBox)  return;
	ndFluidBox->setVisible(bFluids);
	if (!bFluids)  return;
	
	FluidBox& fb = sc->fluids[iFlCur];
	ndFluidBox->setPosition(fb.pos);
	ndFluidBox->setScale(fb.size);
}

void App::UpdMtrWaterDepth()
{
	float fl = edMode == ED_Fluids ? 0.f : 1.f;
	sh::Factory::getInstance().setSharedParameter("waterDepth", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(fl)));
}
#endif

//  water rtt
void App::UpdateWaterRTT(Ogre::Camera* cam)
{
	//  water RTT
	mWaterRTT->setViewerCamera(cam);
	mWaterRTT->setRTTSize(ciShadowSizesA[pSet->water_rttsize]);
	mWaterRTT->setReflect(pSet->water_reflect);
	mWaterRTT->setRefract(pSet->water_refract);
	mWaterRTT->mSceneMgr = mSceneMgr;
	if (!sc->fluids.empty())
		mWaterRTT->setPlane(Plane(Vector3::UNIT_Y, sc->fluids.front().pos.y));
	mWaterRTT->recreate();
	mWaterRTT->setActive(!sc->fluids.empty());
}
