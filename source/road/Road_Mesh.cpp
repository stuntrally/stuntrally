#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "Road.h"
#include <OgreException.h>
#include <OgreSubMesh.h>
#include <OgreSceneManager.h>
#include <OgreMeshManager.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
using namespace Ogre;


//  Create Mesh
//---------------------------------------------------------

void SplineRoad::CreateMesh(SubMesh* mesh, AxisAlignedBox& aabox,
	const std::vector<Vector3>& pos, const std::vector<Vector3>& norm, const std::vector<Vector4>& clr,
	const std::vector<Vector2>& tcs, const std::vector<Ogre::uint16>& idx, String sMtrName)
{
	size_t i, si = pos.size();
	if (si == 0)  {
		LogO("!!Error:  Road CreateMesh 0 verts !");
		return;  }
	mesh->useSharedVertices = false;
	mesh->vertexData = new VertexData();
	mesh->vertexData->vertexStart = 0;
	mesh->vertexData->vertexCount = si;

	//  decl
	VertexDeclaration* decl = mesh->vertexData->vertexDeclaration;
	size_t offset = 0;
	offset += decl->addElement(0,offset,VET_FLOAT3,VES_POSITION).getSize();
	offset += decl->addElement(0,offset,VET_FLOAT3,VES_NORMAL).getSize();
	offset += decl->addElement(0,offset,VET_FLOAT2,VES_TEXTURE_COORDINATES).getSize();
	if (clr.size() > 0)
		offset += decl->addElement(0,offset,VET_FLOAT4,VES_DIFFUSE).getSize();

	//  vertex
	//-----------------------------------------
	HardwareVertexBufferSharedPtr vbuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
		decl->getVertexSize(0), si, HardwareBuffer::HBU_STATIC);
	float* vp = static_cast<float*>(vbuffer->lock(HardwareBuffer::HBL_DISCARD));
	
	//  fill vb, update aabb
	if (clr.size() > 0)
	for (i=0; i < si; ++i)
	{
		const Vector3& p = pos[i];
		*vp++ = p.x;  *vp++ = p.y;  *vp++ = p.z;	aabox.merge(p);
		const Vector3& n = norm[i];
		*vp++ = n.x;  *vp++ = n.y;  *vp++ = n.z;
		*vp++ = tcs[i].x;  *vp++ = tcs[i].y;
		const Vector4& c = clr[i];
		*vp++ = c.x;  *vp++ = c.y;  *vp++ = c.z;  *vp++ = c.w;
	}
	else
	for (i=0; i < si; ++i)
	{
		const Vector3& p = pos[i];
		*vp++ = p.x;  *vp++ = p.y;  *vp++ = p.z;	aabox.merge(p);
		const Vector3& n = norm[i];
		*vp++ = n.x;  *vp++ = n.y;  *vp++ = n.z;
		*vp++ = tcs[i].x;  *vp++ = tcs[i].y;
	}
	vbuffer->unlock();
	mesh->vertexData->vertexBufferBinding->setBinding(0,vbuffer);
	
	//  index
	//-----------------------------------------
	IndexData* id = mesh->indexData;
	id->indexCount = idx.size();  id->indexStart = 0;
	id->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
		HardwareIndexBuffer::IT_16BIT, id->indexCount, HardwareBuffer::HBU_STATIC);
	uint16* ip = static_cast<uint16*>(id->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

	//  fill ib
	for (size_t i=0; i < idx.size(); ++i)
		*ip++ = idx[i];
	
	id->indexBuffer->unlock();
	mesh->setMaterialName(sMtrName);
}


//  add mesh to scene
//---------------------------------------------------------

void SplineRoad::AddMesh(MeshPtr mesh, String sMesh, const AxisAlignedBox& aabox,
	Entity** pEnt, SceneNode** pNode, String sEnd)
{
	mesh->_setBounds(aabox);
	mesh->_setBoundingSphereRadius((aabox.getMaximum()-aabox.getMinimum()).length()/2.0);  
	mesh->load();
	unsigned short src, dest;
	if (!mesh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
		mesh->buildTangentVectors(VES_TANGENT, src, dest);

	*pEnt = mSceneMgr->createEntity("rd.ent"+sEnd, sMesh);
	*pNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("rd.node"+sEnd);
	(*pNode)->attachObject(*pEnt);
	(*pEnt)->setVisible(false);  (*pEnt)->setCastShadows(false);
	(*pEnt)->setVisibilityFlags(RV_Road);
}


//  add triangle to bullet
//---------------------------------------------------------
void SplineRoad::addTri(int f1, int f2, int f3, int i)
{
	/*bool ok = true;  const int fmax = 65530; //16bit
	if (f1 >= at_size || f1 > fmax)  {  LogO("idx too big: "+toStr(f1)+" >= "+toStr(at_size));  ok = 0;  }
	if (f2 >= at_size || f2 > fmax)  {  LogO("idx too big: "+toStr(f2)+" >= "+toStr(at_size));  ok = 0;  }
	if (f3 >= at_size || f3 > fmax)  {  LogO("idx too big: "+toStr(f3)+" >= "+toStr(at_size));  ok = 0;  }
	if (!ok)  return;/**/

	idx.push_back(f1);  idx.push_back(f2);  idx.push_back(f3);
	if (blendTri)
	{
		idxB.push_back(f1);  idxB.push_back(f2);  idxB.push_back(f3);
	}

	if (bltTri && i > 0 && i < at_ilBt)
	{
		posBt.push_back((*at_pos)[f1]);
		posBt.push_back((*at_pos)[f2]);
		posBt.push_back((*at_pos)[f3]);
	}
}


///  Destroy
//---------------------------------------------------------

void SplineRoad::Destroy()  // full
{
	if (ndSel)	mSceneMgr->destroySceneNode(ndSel);
	if (ndChosen)	mSceneMgr->destroySceneNode(ndChosen);
	if (ndRot)	mSceneMgr->destroySceneNode(ndRot);
	if (ndHit)	mSceneMgr->destroySceneNode(ndHit);
	if (ndChk)	mSceneMgr->destroySceneNode(ndChk);
	if (entSel)  mSceneMgr->destroyEntity(entSel);
	if (entChs)  mSceneMgr->destroyEntity(entChs);
	if (entRot)  mSceneMgr->destroyEntity(entRot);
	if (entHit)  mSceneMgr->destroyEntity(entHit);
	if (entChk)  mSceneMgr->destroyEntity(entChk);
	DestroyMarkers();
	DestroyRoad();
}

void SplineRoad::DestroySeg(int id)
{
	//LogO("DestroySeg" + toStr(id));
	RoadSeg& rs = vSegs[id];
	if (rs.empty)  return;
try
{
	for (int l=0; l < LODs; ++l)
	{
		if (rs.wall[l].ent)  // ] wall
		{
			rs.wall[l].node->detachAllObjects();
			#ifdef SR_EDITOR
			mSceneMgr->destroyEntity(rs.wall[l].ent);
			#endif
			//rs.wall[l].node->getParentSceneNode()->detachObject(0);
			mSceneMgr->destroySceneNode(rs.wall[l].node);
			Ogre::MeshManager::getSingleton().remove(rs.wall[l].smesh);
		}
		if (rs.blend[l].ent)  // > blend
		{
			rs.blend[l].node->detachAllObjects();
			#ifdef SR_EDITOR
			mSceneMgr->destroyEntity(rs.blend[l].ent);
			#endif
			mSceneMgr->destroySceneNode(rs.blend[l].node);
			Ogre::MeshManager::getSingleton().remove(rs.blend[l].smesh);
		}
	}
	if (rs.col.ent)  // | column
	{
		#ifdef SR_EDITOR
		rs.col.node->detachAllObjects();
		mSceneMgr->destroyEntity(rs.col.ent);
		#endif
		mSceneMgr->destroySceneNode(rs.col.node);
		Ogre::MeshManager::getSingleton().remove(rs.col.smesh);
	}
	for (int l=0; l < LODs; ++l)
	if (rs.road[l].ent)
	{
		rs.road[l].node->detachAllObjects();
		if (IsTrail())
			mSceneMgr->destroyEntity(rs.road[l].ent);
		#ifdef SR_EDITOR  //_crash in game (destroy all ents is before)
		mSceneMgr->destroyEntity(rs.road[l].ent);
		#endif
		mSceneMgr->destroySceneNode(rs.road[l].node);
		if (Ogre::MeshManager::getSingleton().resourceExists(rs.road[l].smesh))
			Ogre::MeshManager::getSingleton().remove(rs.road[l].smesh);
		//Resource* r = ResourceManae::getSingleton().remove(rs.road[l].smesh);
	}
}catch (Exception ex)
{
	LogO(String("# Error! road DestroySeg") + ex.what());
}
	//LogO("Destroyed.");
	rs.empty = true;
	rs.lpos.clear();
}


void SplineRoad::DestroyRoad()
{
	
#ifndef SR_EDITOR
	for (int i=0; i < vbtTriMesh.size(); ++i)
		delete vbtTriMesh[i];
	vbtTriMesh.clear();
#endif
	for (size_t seg=0; seg < vSegs.size(); ++seg)
		DestroySeg(seg);
	vSegs.clear();

	idStr = 0;
	st.Reset();
}
