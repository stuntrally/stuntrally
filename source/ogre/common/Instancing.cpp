#pragma once
#include "pch.h"
#include "Def_Str.h"
#include "Instancing.h"
using namespace Ogre;


void Instanced::Create(SceneManager* mSceneMgr, String sMesh)
{
	Entity* ent = mSceneMgr->createEntity("aa1", sMesh);
	//ent->setCastShadows(false);
	int numSubs = ent->getMesh()->getNumSubMeshes();
	InstMesh imsh;
	int ii = 100;
	
	for (int s=0; s < numSubs; ++s)
	{
		InstSub isub;
		isub.instMgr = mSceneMgr->createInstanceManager(
			"Inst"+toStr(/**/1)+toStr(s), sMesh,
			ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
			InstanceManager::HWInstancingBasic,
			ii/*vPoses.size()*/, IM_USEALL, s);

		Ogre::String sMtr = ent->getMesh()->getSubMesh(s)->getMaterialName();

		///  add to instancing
		isub.ents.reserve(ii/*vPoses.size()*/);
		for (int i=0; i < ii/*vPoses.size()*/; ++i)
		{
			InstancedEntity* ent = isub.instMgr->createInstancedEntity(sMtr);
			
			//const SPos& p = vPoses[i];
			//imsh.treeId = p.id;
			ent->setPosition(Vector3(i%10,0,i/10));
			ent->setOrientation(Quaternion(Radian(i/40), Vector3::UNIT_Y));
			ent->setScale((1.f+i/ii) * Vector3::UNIT_SCALE);
				//ent->setPosition(p.pos);
				//ent->setOrientation(Quaternion(Radian(p.yaw), Vector3::UNIT_Y));
				//ent->setScale(p.sc * Vector3::UNIT_SCALE);
			//ent->setVisibilityFlags(0xFFF000+l);  //pages vis..?
			//ent->setVisible(true);
			
			isub.ents.push_back(ent);
		}
		imsh.subs.push_back(isub);

		//isub.instMgr->setBatchesAsStaticAndUpdate(true);  //does lower fps
	}
	inst.push_back(imsh);
}


#if 0
{
	static float instUpdTm = 0.f;
	instUpdTm += dt;  // interval [sec]
	if (instUpdTm > 0.1f && trees
	#ifndef ROAD_EDITOR
		&& mSplitMgr && mSplitMgr->mCameras.front()
	#endif
		)
	{
		instUpdTm = 0.f;
		#ifndef ROAD_EDITOR
		const Vector3& cp = mSplitMgr->mCameras.front()->getPosition();
		#else
		const Vector3& cp = mCamera->getPosition();
		#endif

		///  trees vis   // _par fit to impostors (temp?)
		const Real dist = 1.2f * sc->trDist * pSet->trees_dist, distSq = dist * dist;
		for (int l=0; l < inst.size(); ++l)
		{
			const InstMesh& imsh = inst[l];
			for (int s=0; s < imsh.subs.size(); ++s)
			{
				const InstSub& sub = imsh.subs[s];
				for (int e=0; e < sub.ents.size(); ++e)
				{
					const Vector3& p = sub.ents[e]->getPosition();
					bool b = (p-cp).squaredLength() < distSq;
					sub.ents[e]->setVisible(b);
				}
			}
		}
		//if (terrain)  //test
		//	terrain->setVisibilityFlags(0);
	}
}
#endif
