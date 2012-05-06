#include "pch.h"
#include "../common/RenderConst.h"
#include "../common/Defines.h"

#ifdef ROAD_EDITOR
	#include "../../editor/OgreApp.h"
#else
	#include "../OgreGame.h"
	#include "../../vdrift/game.h"
	#include "../../btOgre/BtOgreGP.h"
#endif
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "LinearMath/btSerializer.h"
#include "Serialize/BulletFileLoader/btBulletFile.h"
#include "Serialize/BulletWorldImporter/btBulletWorldImporter.h"
#include <boost/filesystem.hpp>

#include <OgreManualObject.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreEntity.h>
using namespace Ogre;



///  Objects  ... .. . . .
//----------------------------------------------------------------------------------------------------------------------
#ifndef ROAD_EDITOR
class BulletWorldOffset : public btBulletWorldImporter
{
public:
	btTransform mTrOfs;  // in offset
	btDefaultMotionState* ms;  // out
	
	BulletWorldOffset(btDynamicsWorld* world=0)
		: btBulletWorldImporter(world), ms(0)
	{
		mTrOfs.setIdentity();
	}
	
	btCollisionObject* createCollisionObject(const btTransform& startTransform,btCollisionShape* shape, const char* bodyName)
	{
		return createRigidBody(false,0,startTransform,shape,bodyName);
	}

	btRigidBody*  createRigidBody(bool isDynamic, btScalar mass, const btTransform& startTransform,btCollisionShape* shape,const char* bodyName)
	{
		btVector3 localInertia;
		localInertia.setZero();

		if (mass)
			shape->calculateLocalInertia(mass,localInertia);
		
		ms = new btDefaultMotionState();
		ms->setWorldTransform(startTransform * mTrOfs);
		btRigidBody* body = new btRigidBody(mass,ms,shape,localInertia);	
		//body->setWorldTransform(startTransform * mTrOfs);
		body->setDamping(0.1f, 0.3f);
		//body->setFriction(0.5f);

		if (m_dynamicsWorld)
			m_dynamicsWorld->addRigidBody(body);
		
		if (bodyName)
		{
			char* newname = duplicateName(bodyName);
			m_objectNameMap.insert(body,newname);
			m_nameBodyMap.insert(newname,body);
		}
		m_allocatedRigidBodies.push_back(body);
		return body;
	}
};
#endif

void App::CreateObjects()
{
	for (int i=0; i < sc.objects.size(); ++i)
	{
		Object& o = sc.objects[i];
		String s = toStr(i);  // counter for names

		//  add to ogre
		o.ent = mSceneMgr->createEntity("oE"+s, o.name + ".mesh");
		o.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode("oN"+s);
		o.SetFromBlt();
		o.nd->setScale(o.scale);
		o.nd->attachObject(o.ent);  o.ent->setVisibilityFlags(RV_Vegetation);


		#ifndef ROAD_EDITOR
		//  add to bullet world (in game)
		std::string file = PATHMANAGER::GetDataPath()+"/objects/"+o.name+".bullet";
		///  use some map ! dont check for every object ...
		if (!boost::filesystem::exists(file))
		{
			///  static
			Vector3 posO = Vector3(o.pos[0],o.pos[2],-o.pos[1]);
			Quaternion q(o.rot[0],o.rot[1],o.rot[2],o.rot[3]), q1;
			Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);
			q1.FromAngleAxis(-rad,Vector3(axi.z,-axi.x,-axi.y));
			Quaternion rotO = q1 * Object::qrFix;

			Matrix4 tre;  tre.makeTransform(posO,o.scale,rotO);
			BtOgre::StaticMeshToShapeConverter converter(o.ent, tre);
			btCollisionShape* shape = converter.createTrimesh();  //createBox();
			shape->setUserPointer((void*)0);  // mark

			btCollisionObject* bco = new btCollisionObject();
			btTransform tr;  tr.setIdentity();  //tr.setOrigin(btVector3(pos.x,-pos.z,pos.y));
			bco->setActivationState(DISABLE_SIMULATION);  // ISLAND_SLEEPING  WANTS_DEACTIVATION
			bco->setCollisionShape(shape);	bco->setWorldTransform(tr);
			bco->setFriction(0.7f);  bco->setRestitution(0.f);
			bco->setCollisionFlags(bco->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
			pGame->collision.world->addCollisionObject(bco);
			pGame->collision.shapes.push_back(shape);
		}
		else  ///  dynamic
		{
			// .bullet load
			BulletWorldOffset* fileLoader = new BulletWorldOffset(pGame->collision.world);
			fileLoader->mTrOfs.setOrigin(btVector3(o.pos[0],o.pos[1],o.pos[2]+0.5f));
			///+  why is this z ofs needed ? 1st sim dt ??...
			fileLoader->mTrOfs.setRotation(btQuaternion(o.rot[0],o.rot[1],o.rot[2],o.rot[3]));
			//fileLoader->setVerboseMode(true);//

			//LogO(".bullet: "+file);
			if (fileLoader->loadFile(file.c_str()))
			{
				o.ms = fileLoader->ms;  // 1 only
				//LogO(".bullet: "+toStr(fileLoader->getNumCollisionShapes()));
			}
		}
		#endif
	}
}

void App::DestroyObjects()
{
	///  props
	for (int i=0; i < sc.objects.size(); ++i)
	{
		Object& o = sc.objects[i];
		if (o.nd)  mSceneMgr->destroySceneNode(o.nd);  o.nd = 0;
		#ifdef ROAD_EDITOR  // game has destroyAll
		if (o.ent)  mSceneMgr->destroyEntity(o.ent);  o.ent = 0;
		#endif
		//delete o.ms;//?
		o.ms = 0;
	}
	sc.objects.clear();
}

#ifdef ROAD_EDITOR
void App::UpdObjPick()
{
	if (ndStBox)
		ndStBox->setVisible(edMode == ED_Start && !bMoveCam);  //

	int objs = sc.objects.size();
	bool bObjects = edMode == ED_Objects && !bMoveCam && objs > 0 && iObjCur >= 0;
	if (objs > 0)
		iObjCur = std::min(iObjCur, objs-1);

	if (!ndObjBox)  return;
	ndObjBox->setVisible(bObjects);
	if (!bObjects)  return;
	
	const Object& o = sc.objects[iObjCur];
	const AxisAlignedBox& ab = o.nd->getAttachedObject(0)->getBoundingBox();
	Vector3 s = o.scale * ab.getSize();  // * sel obj's node aabb

		Vector3 posO = Vector3(o.pos[0],o.pos[2],-o.pos[1]);
		Quaternion q(o.rot[0],o.rot[1],o.rot[2],o.rot[3]), q1;
		Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);
		q1.FromAngleAxis(-rad,Vector3(axi.z,-axi.x,-axi.y));
		Quaternion rotO = q1 * Object::qrFix;

	ndObjBox->setPosition(posO);
	ndObjBox->setOrientation(rotO);
	ndObjBox->setScale(s);
}
#endif
