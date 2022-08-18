/*	   Filename:  BtOgreGP.h
 *	Description:  The part of BtOgre that handles information transfer from Ogre to 
 *				  Bullet (like mesh data for making trimeshes).
 *		Version:  1.1  (1.0 modified by CrystalH)
 *		Created:  27/12/2008 03:29:56 AM
 *		 Author:  Nikhilesh (nikki)
 * =====================================================================================*/
#pragma once
//#include "btBulletDynamicsCommon.h"
#include "BtOgreDebug.h"

#include <Ogre.h>
// #include <OgreCommon.h>
// #include <OgreVector3.h>
// #include <OgreMatrix4.h>
// #include <OgreMesh.h>
#include <vector>


namespace Ogre {  class Entity;  class SceneNode;  class Renderable;  class VertexData;  }


namespace BtOgre {


typedef std::map<unsigned short, Vector3Array*> BoneIndex;
typedef std::pair<unsigned short, Vector3Array*> BoneKeyIndex;


class VertexIndexToShape
{
public:
	VertexIndexToShape(const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
	~VertexIndexToShape();

	Ogre::Real getRadius();
	Ogre::Vector3 getSize();

	btSphereShape* createSphere();
	btBoxShape* createBox();
	btBvhTriangleMeshShape* createTrimesh();
	btCylinderShape* createCylinder();
	btConvexHullShape* createConvex();

	const Ogre::Vector3* getVertices() const;
	unsigned int getVertexCount() const;
	const unsigned int* getIndices() const;
	unsigned int getIndexCount() const;

protected:
	void addStaticVertexData(const Ogre::VertexData *vertex_data);

	void addAnimatedVertexData(const Ogre::VertexData *vertex_data, 
		const Ogre::VertexData *blended_data, 
		const Ogre::Mesh::IndexMap *indexMap);

	void addIndexData(Ogre::IndexData *data, const unsigned int offset = 0);

protected:
	Ogre::Vector3*	mVertexBuffer;
	unsigned int*	mIndexBuffer;
	unsigned int	mVertexCount;
	unsigned int	mIndexCount;

	Ogre::Matrix4	mTransform;
	Ogre::Real		mBoundRadius;
	Ogre::Vector3	mBounds;
	BoneIndex*		mBoneIndex;
};


///  For static (non-animated) meshes.
//
class StaticMeshToShapeConverter : public VertexIndexToShape
{
public:
	StaticMeshToShapeConverter(Ogre::Renderable *rend, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
	StaticMeshToShapeConverter(Ogre::Entity *entity,   const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
	StaticMeshToShapeConverter();
	~StaticMeshToShapeConverter();

	void addEntity(Ogre::Entity *entity,const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
	void addMesh(const Ogre::MeshPtr &mesh, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);

protected:
	Ogre::Entity*		mEntity;
	Ogre::SceneNode*	mNode;
};


///  For animated meshes.
//
class AnimatedMeshToShapeConverter : public VertexIndexToShape
{
public:
	AnimatedMeshToShapeConverter(Ogre::Entity *entity, const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
	AnimatedMeshToShapeConverter();
	~AnimatedMeshToShapeConverter();

	void addEntity(Ogre::Entity *entity,const Ogre::Matrix4 &transform = Ogre::Matrix4::IDENTITY);
	void addMesh(const Ogre::MeshPtr &mesh, const Ogre::Matrix4 &transform);

	btBoxShape* createAlignedBox(unsigned char bone, const Ogre::Vector3 &bonePosition, const Ogre::Quaternion &boneOrientation);
	btBoxShape* createOrientedBox(unsigned char bone, const Ogre::Vector3 &bonePosition, const Ogre::Quaternion &boneOrientation);

protected:
	bool getBoneVertices(unsigned char bone,
		unsigned int &vertex_count, Ogre::Vector3* &vertices, const Ogre::Vector3 &bonePosition);

	bool getOrientedBox(unsigned char bone, 
		const Ogre::Vector3 &bonePosition, const Ogre::Quaternion &boneOrientation,
		Ogre::Vector3 &extents, Ogre::Vector3* axis, Ogre::Vector3 &center);

	Ogre::Entity*	 mEntity;
	Ogre::SceneNode* mNode;

	Ogre::Vector3*	mTransformedVerticesTemp;
	size_t			mTransformedVerticesTempSize;
};

}
