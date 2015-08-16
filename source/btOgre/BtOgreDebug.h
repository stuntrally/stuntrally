/*	   Filename:  BtOgreDebug.cpp
 *	Description:  Contains the Ogre Mesh to Bullet Shape converters.
 *		Version:  1.1  (1.0 modified by CrystalH)
 *		Created:  27/12/2008 01:45:56 PM
 *		 Author:  Nikhilesh (nikki)
 * =====================================================================================*/
#pragma once
#include "btBulletDynamicsCommon.h"
#include "DynamicLines.h"
#include <OgreVector3.h>

namespace Ogre {  class SceneNode;  }

namespace BtOgre  {


typedef std::vector<Ogre::Vector3> Vector3Array;

//  Converts between Bullet and Ogre
class Convert
{
public:
	static btQuaternion toBullet(const Ogre::Quaternion &q)
	{
		return btQuaternion(q.x, q.y, q.z, q.w);
	}
	static btVector3 toBullet(const Ogre::Vector3 &v)
	{
		return btVector3(v.x, v.z, -v.y);
	}

	static Ogre::Quaternion toOgre(const btQuaternion &q)
	{
		return Ogre::Quaternion(q.w(), q.x(), q.y(), q.z());
	}
	static Ogre::Vector3 toOgre(const btVector3 &v)
	{
		return Ogre::Vector3(v.x(), v.z(), -v.y());
	}
};


class DebugDrawer : public btIDebugDraw
{
protected:
	Ogre::SceneNode *mNode;
	btDynamicsWorld *mWorld;
	class DynamicLines *mLineDrawer;
	int m_debugMode;

public:

	DebugDrawer(Ogre::SceneNode *node, btDynamicsWorld *world);
	~DebugDrawer();

	void step();

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	
	void drawSphere(btScalar radius, const btTransform& transform, const btVector3& color/*, int half*/);

	void drawCircleZ(btScalar radius, const btVector3& p, const btVector3& x, const btVector3& y, const btVector3& color);

	void drawCapsule(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color);

	void drawCylinder(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color);

	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,
		btScalar distance, int lifeTime, const btVector3& color);

	void reportErrorWarning(const char* warningString);

	void draw3dText(const btVector3& location,const char* textString);
	
	void setDebugMode(int debugMode);
	int	getDebugMode() const;
};

}
