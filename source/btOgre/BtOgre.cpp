#include "pch.h"
/*		   Filename:	BtOgre.cpp
 *		Description:	Bullet to Ogre implementation.
 *			Version:	1.1  (1.0 modified by CrystalH)
 *			Created:	27/12/2008 01:47:56 PM
 *			 Author:	Nikhilesh (nikki)
 * =============================================================================================*/
#include "BtOgrePG.h"
#include "BtOgreGP.h"
#include "BtOgreDebug.h"

#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <OgreSubMesh.h>
#include <OgreSubEntity.h>
#include <OgreLogManager.h>

using namespace Ogre;

namespace BtOgre {


//=============================================================================================

RigidBodyState::RigidBodyState(Ogre::SceneNode *node)
	: mNode(node),
	  mTransform(((node != NULL) ? BtOgre::Convert::toBullet(node->getOrientation()) : btQuaternion(0,0,0,1)), 
				 ((node != NULL) ? BtOgre::Convert::toBullet(node->getPosition())	: btVector3(0,0,0))),
	  mCenterOfMassOffset(btTransform::getIdentity())
{	}


void RigidBodyState::setWorldTransform(const btTransform &in) 
{
	if (mNode == NULL)
		return;

	mTransform = in;
	btTransform transform = in * mCenterOfMassOffset;

	btQuaternion rot = transform.getRotation();
	btVector3 pos = transform.getOrigin();
	mNode->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
	//mNode->setPosition(pos.x(), pos.y(), pos.z());
	mNode->setPosition(pos.x(), pos.z(), -pos.y());  ///!
}


//  BtOgre::VertexIndexToShape
//=============================================================================================

	void VertexIndexToShape::addStaticVertexData(const VertexData *vertex_data)
	{
		if (!vertex_data) 
			return;

		const VertexData *data = vertex_data;

		const unsigned int prev_size = mVertexCount;
		mVertexCount += (unsigned int)data->vertexCount;

		Ogre::Vector3* tmp_vert = new Ogre::Vector3[mVertexCount];
		if (mVertexBuffer)
		{
			memcpy(tmp_vert, mVertexBuffer, sizeof(Vector3) * prev_size);
			delete[] mVertexBuffer;
		}
		mVertexBuffer = tmp_vert;

		// Get the positional buffer element
		{	
			const Ogre::VertexElement* posElem = data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);			
			Ogre::HardwareVertexBufferSharedPtr vbuf = data->vertexBufferBinding->getBuffer(posElem->getSource());
			const unsigned int vSize = (unsigned int)vbuf->getVertexSize();

			unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			float* pReal;
			Ogre::Vector3 * curVertices = &mVertexBuffer[prev_size];
			const unsigned int vertexCount = (unsigned int)data->vertexCount;
			for(unsigned int j = 0; j < vertexCount; ++j)
			{
				posElem->baseVertexPointerToElement(vertex, &pReal);
				vertex += vSize;

				curVertices->x = (*pReal++);
				curVertices->y = (*pReal++);
				curVertices->z = (*pReal++);

				*curVertices = mTransform * (*curVertices);
				curVertices++;
			}
			vbuf->unlock();
		}
	}
	//------------------------------------------------------------------------------------------------
	void VertexIndexToShape::addAnimatedVertexData(
		const Ogre::VertexData *vertex_data, const Ogre::VertexData *blend_data, const Ogre::Mesh::IndexMap *indexMap)
	{	
		// Get the bone index element
		assert(vertex_data);

		const VertexData *data = blend_data;
		const unsigned int prev_size = mVertexCount;
		mVertexCount += (unsigned int)data->vertexCount;
		Ogre::Vector3* tmp_vert = new Ogre::Vector3[mVertexCount];
		if (mVertexBuffer)
		{
			memcpy(tmp_vert, mVertexBuffer, sizeof(Vector3) * prev_size);
			delete[] mVertexBuffer;
		}
		mVertexBuffer = tmp_vert;

		// Get the positional buffer element
		{	
			const Ogre::VertexElement* posElem = data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);	
			assert (posElem);
			Ogre::HardwareVertexBufferSharedPtr vbuf = data->vertexBufferBinding->getBuffer(posElem->getSource());
			const unsigned int vSize = (unsigned int)vbuf->getVertexSize();

			unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			float* pReal;
			Ogre::Vector3 * curVertices = &mVertexBuffer[prev_size];
			const unsigned int vertexCount = (unsigned int)data->vertexCount;
			for(unsigned int j = 0; j < vertexCount; ++j)
			{
				posElem->baseVertexPointerToElement(vertex, &pReal);
				vertex += vSize;

				curVertices->x = (*pReal++);
				curVertices->y = (*pReal++);
				curVertices->z = (*pReal++);

				*curVertices = mTransform * (*curVertices);
				curVertices++;
			}
			vbuf->unlock();
		}{
			const Ogre::VertexElement* bneElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_BLEND_INDICES);
			assert (bneElem);
			
			Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(bneElem->getSource());
			const unsigned int vSize = (unsigned int)vbuf->getVertexSize();
			unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

			unsigned char* pBone;

			if (!mBoneIndex)
				mBoneIndex = new BoneIndex();	
			BoneIndex::iterator i;

			Ogre::Vector3 * curVertices = &mVertexBuffer[prev_size];

			const unsigned int vertexCount = (unsigned int)vertex_data->vertexCount;
			for(unsigned int j = 0; j < vertexCount; ++j)
			{
				bneElem->baseVertexPointerToElement(vertex, &pBone);
				vertex += vSize;

				const unsigned char currBone = (indexMap) ? (*indexMap)[*pBone] : *pBone;
				i = mBoneIndex->find (currBone);
				Vector3Array* l = 0;
				if (i == mBoneIndex->end())
				{
					l = new Vector3Array;
					mBoneIndex->insert(BoneKeyIndex(currBone, l));
				}else 
					l = i->second;

				l->push_back(*curVertices);
				curVertices++;
			}
			vbuf->unlock();	
		}
	}

	//------------------------------------------------------------------------------------------------
	void VertexIndexToShape::addIndexData(IndexData *data, const unsigned int offset)
	{
		const unsigned int prev_size = mIndexCount;
		mIndexCount += (unsigned int)data->indexCount;

		unsigned int* tmp_ind = new unsigned int[mIndexCount];
		if (mIndexBuffer)
		{
			memcpy (tmp_ind, mIndexBuffer, sizeof(unsigned int) * prev_size);
			delete[] mIndexBuffer;
		}
		mIndexBuffer = tmp_ind;

		const unsigned int numTris = (unsigned int) data->indexCount / 3;
		HardwareIndexBufferSharedPtr ibuf = data->indexBuffer;	
		const bool use32bitindexes = (ibuf->getType() == HardwareIndexBuffer::IT_32BIT);
		unsigned int index_offset = prev_size;

		if (use32bitindexes) 
		{
			const unsigned int* pInt = static_cast<unsigned int*>(ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
			for(unsigned int k = 0; k < numTris; ++k)
			{
				mIndexBuffer[index_offset ++] = offset + *pInt++;
				mIndexBuffer[index_offset ++] = offset + *pInt++;
				mIndexBuffer[index_offset ++] = offset + *pInt++;
			}
			ibuf->unlock();
		}else{
			const unsigned short* pShort = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
			for(unsigned int k = 0; k < numTris; ++k)
			{
				mIndexBuffer[index_offset ++] = offset + static_cast<unsigned int> (*pShort++);
				mIndexBuffer[index_offset ++] = offset + static_cast<unsigned int> (*pShort++);
				mIndexBuffer[index_offset ++] = offset + static_cast<unsigned int> (*pShort++);
			}
			ibuf->unlock();
		}
	}

	//------------------------------------------------------------------------------------------------
	Real VertexIndexToShape::getRadius()
	{
		if (mBoundRadius == -1)
		{
			getSize();
			mBoundRadius = (std::max(mBounds.x,std::max(mBounds.y,mBounds.z)) * 0.5);
		}
		return mBoundRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	Vector3 VertexIndexToShape::getSize()
	{
		const unsigned int vCount = getVertexCount();
		if (mBounds == Ogre::Vector3(-1,-1,-1) && vCount > 0)
		{
			const Ogre::Vector3 * const v = getVertices();

			Ogre::Vector3 vmin(v[0]);
			Ogre::Vector3 vmax(v[0]);

			for(unsigned int j = 1; j < vCount; j++)
			{
				vmin.x = std::min(vmin.x, v[j].x);
				vmin.y = std::min(vmin.y, v[j].y);
				vmin.z = std::min(vmin.z, v[j].z);

				vmax.x = std::max(vmax.x, v[j].x);
				vmax.y = std::max(vmax.y, v[j].y);
				vmax.z = std::max(vmax.z, v[j].z);
			}

			mBounds.x = vmax.x - vmin.x;
			mBounds.y = vmax.y - vmin.y;
			mBounds.z = vmax.z - vmin.z;
		}
		return mBounds;
	}
	
	//------------------------------------------------------------------------------------------------
	const Ogre::Vector3* VertexIndexToShape::getVertices() const
	{
		return mVertexBuffer;
	}
	unsigned int VertexIndexToShape::getVertexCount() const
	{
		return mVertexCount;
	}
	const unsigned int* VertexIndexToShape::getIndices() const
	{
		return mIndexBuffer;
	}
	unsigned int VertexIndexToShape::getIndexCount() const
	{
		return mIndexCount;
	}

	//------------------------------------------------------------------------------------------------
	btSphereShape* VertexIndexToShape::createSphere()
	{
		const Ogre::Real rad = getRadius();
		assert((rad > 0.0) && 
			("Sphere radius must be greater than zero"));
		btSphereShape* shape = new btSphereShape(rad);

		return shape;
	}

	btBoxShape* VertexIndexToShape::createBox()
	{
		const Ogre::Vector3 sz = getSize();

		assert((sz.x > 0.0) && (sz.y > 0.0) && (sz.y > 0.0) && 
			("Size of box must be greater than zero on all axes"));

		btBoxShape* shape = new btBoxShape(Convert::toBullet(sz * 0.5));
		return shape;
	}

	btCylinderShape* VertexIndexToShape::createCylinder()
	{
		const Ogre::Vector3 sz = getSize();

		assert((sz.x > 0.0) && (sz.y > 0.0) && (sz.y > 0.0) && 
			("Size of Cylinder must be greater than zero on all axes"));

		btCylinderShape* shape = new btCylinderShapeX(Convert::toBullet(sz * 0.5));
		return shape;
	}

	btConvexHullShape* VertexIndexToShape::createConvex()
	{
		assert(mVertexCount && (mIndexCount >= 6) && 
			("Mesh must have some vertices and at least 6 indices (2 triangles)"));

		return new btConvexHullShape((btScalar*) &mVertexBuffer[0].x, mVertexCount, sizeof(Vector3));
	}

	//------------------------------------------------------------------------------------------------
	btBvhTriangleMeshShape* VertexIndexToShape::createTrimesh()
	{
		assert(mVertexCount && (mIndexCount >= 6) && 
			("Mesh must have some vertices and at least 6 indices (2 triangles)"));

		unsigned int numFaces = mIndexCount / 3;

		btTriangleMesh *trimesh = new btTriangleMesh();
		unsigned int *indices = mIndexBuffer;
		Vector3 *vertices = mVertexBuffer;

		btVector3 vertexPos[3];
		for (unsigned int n = 0; n < numFaces; ++n)
		{	{
			const Vector3 &vec = vertices[*indices];
			vertexPos[0][0] = vec.x;
			vertexPos[0][1] =-vec.z;
			vertexPos[0][2] = vec.y;	}{
		
			const Vector3 &vec = vertices[*(indices + 1)];
			vertexPos[1][0] = vec.x;
			vertexPos[1][1] =-vec.z;
			vertexPos[1][2] = vec.y;	}{

			const Vector3 &vec = vertices[*(indices + 2)];
			vertexPos[2][0] = vec.x;
			vertexPos[2][1] =-vec.z;
			vertexPos[2][2] = vec.y;	}

			indices += 3;
			trimesh->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2]);
		}

		const bool useQuantizedAABB = true;
		btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(trimesh, useQuantizedAABB);

		return shape;
	}

	//------------------------------------------------------------------------------------------------
	VertexIndexToShape::~VertexIndexToShape()
	{
		delete[] mVertexBuffer;
		delete[] mIndexBuffer;

		if (mBoneIndex)
		{
			for(BoneIndex::iterator i = mBoneIndex->begin();
				i != mBoneIndex->end(); ++i)
			{
				delete i->second;
			}
			delete mBoneIndex;
		}
	}
	VertexIndexToShape::VertexIndexToShape(const Matrix4 &transform) :
		mVertexBuffer(0), mIndexBuffer(0),
		mVertexCount(0), mIndexCount(0),
		mBounds(Vector3(-1,-1,-1)),
		mBoundRadius(-1), mBoneIndex(0),
		mTransform(transform)
	{	}


//  BtOgre::StaticMeshToShapeConverter
//=============================================================================================

	StaticMeshToShapeConverter::StaticMeshToShapeConverter() :
		VertexIndexToShape(),
		mEntity(0), mNode(0)
	{	}

	StaticMeshToShapeConverter::~StaticMeshToShapeConverter()
	{	}

	StaticMeshToShapeConverter::StaticMeshToShapeConverter(Entity *entity,	const Matrix4 &transform) :
		VertexIndexToShape(transform),
		mEntity(0), mNode(0)
	{
		addEntity(entity, transform);	
	}

	StaticMeshToShapeConverter::StaticMeshToShapeConverter(Renderable *rend, const Matrix4 &transform) :
		VertexIndexToShape(transform),
		mEntity(0), mNode(0)
	{
		RenderOperation op;
		rend->getRenderOperation(op);
		VertexIndexToShape::addStaticVertexData(op.vertexData);
		if(op.useIndexes)
			VertexIndexToShape::addIndexData(op.indexData);

	}
	
	//------------------------------------------------------------------------------------------------
	void StaticMeshToShapeConverter::addEntity(Entity *entity,const Matrix4 &transform)
	{
		// Each entity added need to reset size and radius
		// next time getRadius and getSize are asked, they're computed.
		mBounds	= Ogre::Vector3(-1,-1,-1);
		mBoundRadius = -1;

		mEntity = entity;
		mNode = (SceneNode*)(mEntity->getParentNode());
		mTransform = transform;

		if (mEntity->getMesh()->sharedVertexData)
		{
			VertexIndexToShape::addStaticVertexData (mEntity->getMesh()->sharedVertexData);
		}

		for (unsigned int i = 0;i < mEntity->getNumSubEntities();++i)
		{
			SubMesh *sub_mesh = mEntity->getSubEntity(i)->getSubMesh();

			if (!sub_mesh->useSharedVertices)
			{
				VertexIndexToShape::addIndexData(sub_mesh->indexData, mVertexCount);
				VertexIndexToShape::addStaticVertexData (sub_mesh->vertexData);
			}else 
				VertexIndexToShape::addIndexData (sub_mesh->indexData);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StaticMeshToShapeConverter::addMesh(const MeshPtr &mesh, const Matrix4 &transform)
	{
		// Each entity added need to reset size and radius
		// next time getRadius and getSize are asked, they're computed.
		mBounds	= Ogre::Vector3(-1,-1,-1);
		mBoundRadius = -1;

		//_entity = entity;
		//_node = (SceneNode*)(_entity->getParentNode());
		mTransform = transform;

		if (mesh->hasSkeleton ())
			Ogre::LogManager::getSingleton().logMessage("MeshToShapeConverter::addMesh : Mesh " +
				mesh->getName () + " as skeleton but added to trimesh non animated");

		if (mesh->sharedVertexData)
		{
			VertexIndexToShape::addStaticVertexData (mesh->sharedVertexData);
		}

		for(unsigned int i = 0;i < mesh->getNumSubMeshes();++i)
		{
			SubMesh *sub_mesh = mesh->getSubMesh(i);

			if (!sub_mesh->useSharedVertices)
			{
				VertexIndexToShape::addIndexData(sub_mesh->indexData, mVertexCount);
				VertexIndexToShape::addStaticVertexData (sub_mesh->vertexData);
			}else 
				VertexIndexToShape::addIndexData (sub_mesh->indexData);

		}
	}

}
