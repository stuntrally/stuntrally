#include "pch.h"
/*		   Filename:	BtOgreAnim.cpp
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
#include <OgreMesh.h>
#include <OgreSubMesh.h>
#include <OgreSubEntity.h>

using namespace Ogre;

namespace BtOgre {


//  BtOgre::AnimatedMeshToShapeConverter
//=============================================================================================

	AnimatedMeshToShapeConverter::AnimatedMeshToShapeConverter(Entity *entity,const Matrix4 &transform) :
		VertexIndexToShape(transform), mEntity(0), mNode(0),
		mTransformedVerticesTemp(0), mTransformedVerticesTempSize(0)
	{
		addEntity(entity, transform);	
	}

	//------------------------------------------------------------------------------------------------
	AnimatedMeshToShapeConverter::AnimatedMeshToShapeConverter() :
		VertexIndexToShape(), mEntity(0), mNode(0),
		mTransformedVerticesTemp(0), mTransformedVerticesTempSize(0)
	{
	}
	AnimatedMeshToShapeConverter::~AnimatedMeshToShapeConverter()
	{
		delete[] mTransformedVerticesTemp;
	}

	//------------------------------------------------------------------------------------------------
	void AnimatedMeshToShapeConverter::addEntity(Entity *entity,const Matrix4 &transform)
	{
		// Each entity added need to reset size and radius
		// next time getRadius and getSize are asked, they're computed.
		mBounds	= Ogre::Vector3(-1,-1,-1);
		mBoundRadius = -1;

		mEntity = entity;
		mNode = (SceneNode*)(mEntity->getParentNode());
		mTransform = transform;

		assert (entity->getMesh()->hasSkeleton ());

		mEntity->addSoftwareAnimationRequest(false);
		mEntity->_updateAnimation();

		if (mEntity->getMesh()->sharedVertexData)
		{
			VertexIndexToShape::addAnimatedVertexData (mEntity->getMesh()->sharedVertexData, 
				mEntity->_getSkelAnimVertexData(),
				&mEntity->getMesh()->sharedBlendIndexToBoneIndexMap); 
		}

		for (unsigned int i = 0;i < mEntity->getNumSubEntities();++i)
		{
			SubMesh *sub_mesh = mEntity->getSubEntity(i)->getSubMesh();

			if (!sub_mesh->useSharedVertices)
			{
				VertexIndexToShape::addIndexData(sub_mesh->indexData, mVertexCount);

				VertexIndexToShape::addAnimatedVertexData (sub_mesh->vertexData, 
					mEntity->getSubEntity(i)->_getSkelAnimVertexData(),
					&sub_mesh->blendIndexToBoneIndexMap); 
			}else 
				VertexIndexToShape::addIndexData(sub_mesh->indexData);
		}
		mEntity->removeSoftwareAnimationRequest(false);
	}

	//------------------------------------------------------------------------------------------------
	void AnimatedMeshToShapeConverter::addMesh(const MeshPtr &mesh, const Matrix4 &transform)
	{
		// Each entity added need to reset size and radius
		// next time getRadius and getSize are asked, they're computed.
		mBounds	= Ogre::Vector3(-1,-1,-1);
		mBoundRadius = -1;

		//_entity = entity;
		//_node = (SceneNode*)(_entity->getParentNode());
		mTransform = transform;

		assert (mesh->hasSkeleton());

		if (mesh->sharedVertexData)
		{
			VertexIndexToShape::addAnimatedVertexData (mesh->sharedVertexData, 
				0, &mesh->sharedBlendIndexToBoneIndexMap); 
		}

		for(unsigned int i = 0;i < mesh->getNumSubMeshes();++i)
		{
			SubMesh *sub_mesh = mesh->getSubMesh(i);

			if (!sub_mesh->useSharedVertices)
			{
				VertexIndexToShape::addIndexData(sub_mesh->indexData, mVertexCount);

				VertexIndexToShape::addAnimatedVertexData (sub_mesh->vertexData, 
					0, &sub_mesh->blendIndexToBoneIndexMap); 
			}else 
				VertexIndexToShape::addIndexData (sub_mesh->indexData);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool AnimatedMeshToShapeConverter::getBoneVertices(
		unsigned char bone, unsigned int &vertex_count, Ogre::Vector3* &vertices, const Vector3 &bonePosition)
	{
		BoneIndex::iterator i = mBoneIndex->find(bone);

		if (i == mBoneIndex->end()) 
			return false;

		if (i->second->empty()) 
			return false;

		vertex_count = (unsigned int) i->second->size() + 1;
		if (vertex_count > mTransformedVerticesTempSize)
		{	
			if (mTransformedVerticesTemp)
				delete[] mTransformedVerticesTemp;

			mTransformedVerticesTemp = new Ogre::Vector3[vertex_count];
		}

		vertices = mTransformedVerticesTemp;
		vertices[0] = bonePosition;
		//mEntity->_getParentNodeFullTransform() * 
		//	mEntity->getSkeleton()->getBone(bone)->_getDerivedPosition();

		//mEntity->getSkeleton()->getBone(bone)->_getDerivedOrientation()
		unsigned int currBoneVertex = 1;
		Vector3Array::iterator j = i->second->begin();
		while(j != i->second->end())
		{
			vertices[currBoneVertex] = (*j);  ++j;
			++currBoneVertex; 
		}			 
		return true;
	}

	//------------------------------------------------------------------------------------------------
	btBoxShape* AnimatedMeshToShapeConverter::createAlignedBox(
		unsigned char bone, const Vector3 &bonePosition, const Quaternion &boneOrientation)
	{
		unsigned int vertex_count;
		Vector3* vertices;

		if (!getBoneVertices(bone, vertex_count, vertices, bonePosition)) 
			return 0;

		Vector3 min_vec(vertices[0]);
		Vector3 max_vec(vertices[0]);

		for(unsigned int j = 1; j < vertex_count ;j++)
		{
			min_vec.x = std::min(min_vec.x,vertices[j].x);
			min_vec.y = std::min(min_vec.y,vertices[j].y);
			min_vec.z = std::min(min_vec.z,vertices[j].z);

			max_vec.x = std::max(max_vec.x,vertices[j].x);
			max_vec.y = std::max(max_vec.y,vertices[j].y);
			max_vec.z = std::max(max_vec.z,vertices[j].z);
		}
		const Ogre::Vector3 maxMinusMin(max_vec - min_vec);
		btBoxShape* box = new btBoxShape(Convert::toBullet(maxMinusMin));

		/*const Ogre::Vector3 pos
			(min_vec.x + (maxMinusMin.x * 0.5),
			min_vec.y + (maxMinusMin.y * 0.5),
			min_vec.z + (maxMinusMin.z * 0.5));*/

		//box->setPosition(pos);

		return box;
	}

	//------------------------------------------------------------------------------------------------
	bool AnimatedMeshToShapeConverter::getOrientedBox(
		unsigned char bone, const Vector3 &bonePosition, const Quaternion &boneOrientation,
		Vector3 &box_afExtent, Vector3 *box_akAxis, Vector3 &box_kCenter)
	{
		unsigned int vertex_count;
		Vector3* vertices;

		if (!getBoneVertices(bone, vertex_count, vertices, bonePosition))
			return false;

		box_kCenter = Vector3::ZERO;
		{
			for(unsigned int c = 0 ;c < vertex_count;c++)
				box_kCenter += vertices[c];

			const Ogre::Real invVertexCount = 1.0 / vertex_count;
			box_kCenter *= invVertexCount;
		}
		Quaternion orient = boneOrientation;
		orient.ToAxes(box_akAxis);

		// Let C be the box center and let U0, U1, and U2 be the box axes.	Each
		// input point is of the form X = C + y0*U0 + y1*U1 + y2*U2.	The
		// following code computes min(y0), max(y0), min(y1), max(y1), min(y2),
		// and max(y2).	The box center is then adjusted to be
		//	 C' = C + 0.5*(min(y0)+max(y0))*U0 + 0.5*(min(y1)+max(y1))*U1 +
		//				0.5*(min(y2)+max(y2))*U2

		Ogre::Vector3 kDiff (vertices[1] - box_kCenter);
		Ogre::Real fY0Min = kDiff.dotProduct(box_akAxis[0]), fY0Max = fY0Min;
		Ogre::Real fY1Min = kDiff.dotProduct(box_akAxis[1]), fY1Max = fY1Min;
		Ogre::Real fY2Min = kDiff.dotProduct(box_akAxis[2]), fY2Max = fY2Min;

		for (unsigned int i = 2; i < vertex_count; ++i)
		{
			kDiff = vertices[i] - box_kCenter;

			const Ogre::Real fY0 = kDiff.dotProduct(box_akAxis[0]);
			if (fY0 < fY0Min)		fY0Min = fY0;
			else if (fY0 > fY0Max)	fY0Max = fY0;

			const Ogre::Real fY1 = kDiff.dotProduct(box_akAxis[1]);
			if (fY1 < fY1Min)		fY1Min = fY1;
			else if (fY1 > fY1Max)	fY1Max = fY1;

			const Ogre::Real fY2 = kDiff.dotProduct(box_akAxis[2]);
			if (fY2 < fY2Min)		fY2Min = fY2;
			else if (fY2 > fY2Max)	fY2Max = fY2;
		}

		box_afExtent.x = ((Real)0.5)*(fY0Max - fY0Min);
		box_afExtent.y = ((Real)0.5)*(fY1Max - fY1Min);
		box_afExtent.z = ((Real)0.5)*(fY2Max - fY2Min);

		box_kCenter += (0.5*(fY0Max+fY0Min))*box_akAxis[0] +
			(0.5*(fY1Max+fY1Min))*box_akAxis[1] +
			(0.5*(fY2Max+fY2Min))*box_akAxis[2];

		box_afExtent *= 2.0;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	btBoxShape *AnimatedMeshToShapeConverter::createOrientedBox(
		unsigned char bone, const Vector3 &bonePosition, const Quaternion &boneOrientation)
	{
		Ogre::Vector3 box_akAxis[3];
		Ogre::Vector3 box_afExtent;
		Ogre::Vector3 box_afCenter;

		if (!getOrientedBox(bone, bonePosition, boneOrientation,
							box_afExtent, box_akAxis, box_afCenter))
			return 0;

		btBoxShape *geom = new btBoxShape(Convert::toBullet(box_afExtent));
		//geom->setOrientation(Quaternion(box_akAxis[0],box_akAxis[1],box_akAxis[2]));
		//geom->setPosition(box_afCenter);
		return geom; 
	}
	
}
