/*	   Filename:  BtOgrePG.h
 *	Description:  The part of BtOgre that handles information transfer from Bullet to 
 *				  Ogre (like updating graphics object positions).
 *		Version:  1.1  (1.0 modified by CrystalH)
 *		Created:  27/12/2008 03:40:56 AM
 *		 Author:  Nikhilesh (nikki)
 * =====================================================================================*/
#pragma once
//#include "btBulletDynamicsCommon.h"
#include "BtOgreDebug.h"

namespace Ogre {  class SceneNode;  };


namespace BtOgre {


// A MotionState is Bullet's way of informing you about updates to an object.
// Pass this MotionState to a btRigidBody to have your SceneNode updated automaticaly.

class RigidBodyState : public btMotionState 
{
	protected:
		btTransform mTransform;
		btTransform mCenterOfMassOffset;

		Ogre::SceneNode *mNode;

	public:
		RigidBodyState(Ogre::SceneNode *node, const btTransform &transform, const btTransform &offset = btTransform::getIdentity())
			: mNode(node), mTransform(transform), mCenterOfMassOffset(offset)
		{	}

		RigidBodyState(Ogre::SceneNode *node);

		virtual void getWorldTransform(btTransform &ret) const 
		{
			ret = mCenterOfMassOffset.inverse() * mTransform;
		}

		virtual void setWorldTransform(const btTransform &in);

		void setNode(Ogre::SceneNode *node) 
		{
			mNode = node;
		}
};

}
