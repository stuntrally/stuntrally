#include "pch.h"
#include "RenderConst.h"
#include "Defines.h"
#include "../../vdrift/pathmanager.h"
#include "../../btOgre/BtOgreGP.h"
#include "../../road/Road.h"

#ifdef ROAD_EDITOR
	#include "../../editor/OgreApp.h"
#else
	#include "../OgreGame.h"
	#include "../../vdrift/game.h"
#endif
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "LinearMath/btDefaultMotionState.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
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
class BulletWorldOffset : public btBulletWorldImporter
{
public:
	btTransform mTrOfs;  // in offset
	btDefaultMotionState* ms;  // out
	btRigidBody* rb;  // out
	
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
		ms->setWorldTransform(mTrOfs);
		btRigidBody* body = new btRigidBody(mass,ms,shape,localInertia);	
		body->setDamping(0.1f, 0.3f);
		//body->setFriction(0.5f);
		rb = body;

		#ifdef ROAD_EDITOR
		//body->setActivationState(DISABLE_DEACTIVATION);
		#else
		body->setActivationState(WANTS_DEACTIVATION);  // game creates deactivated (sleeping)
		#endif

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


//  Create
//-------------------------------------------------------------------------------------------------------
void App::CreateObjects()
{
	//  maps for file exist (optimize)
	using std::map;  using std::string;
	map<string,bool> objExists, objHasBlt;
	
	for (int i=0; i < sc.objects.size(); ++i)
	{
		const string& s = sc.objects[i].name;
		objExists[s] = false;  objHasBlt[s] = false;
	}
	for (map<string,bool>::iterator it = objExists.begin(); it != objExists.end(); ++it)
	{
		bool ex = boost::filesystem::exists(PATHMANAGER::GetDataPath()+"/objects/"+ (*it).first + ".mesh");
		(*it).second = ex;
		if (!ex)  LogO("CreateObjects mesh doesn't exist: " + (*it).first + ".mesh");
	}
	for (map<string,bool>::iterator it = objHasBlt.begin(); it != objHasBlt.end(); ++it)
		(*it).second = boost::filesystem::exists(PATHMANAGER::GetDataPath()+"/objects/"+ (*it).first + ".bullet");

	//  loader
	#ifndef ROAD_EDITOR
	btDiscreteDynamicsWorld* world = pGame->collision.world;
	#endif
	BulletWorldOffset* fileLoader = new BulletWorldOffset(world);

	///  create  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
	for (int i=0; i < sc.objects.size(); ++i)
	{
		Object& o = sc.objects[i];
		String s = toStr(i);  // counter for names
		#ifndef ROAD_EDITOR
		if (objHasBlt[o.name] && !pSet->game.dyn_objects)  continue;
		#endif

		//  add to ogre
		bool no = !objExists[o.name];
		o.ent = mSceneMgr->createEntity("oE"+s, (no ? "sphere" : o.name) + ".mesh");
		o.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode("oN"+s);
		o.SetFromBlt();
		o.nd->attachObject(o.ent);  o.ent->setVisibilityFlags(RV_Objects);
		o.nd->setScale(o.scale);
		if (no)  continue;

		//  add to bullet world (in game)
		if (!objHasBlt[o.name])
		{
			///  static  . . . . . . . . . . . . 
			Vector3 posO = Vector3(o.pos[0],o.pos[2],-o.pos[1]);
			Quaternion q(o.rot[0],o.rot[1],o.rot[2],o.rot[3]), q1;
			Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);
			q1.FromAngleAxis(-rad,Vector3(axi.z,-axi.x,-axi.y));
			Quaternion rotO = q1 * Object::qrFix;

			Matrix4 tre;  tre.makeTransform(posO,o.scale,rotO);
			BtOgre::StaticMeshToShapeConverter converter(o.ent, tre);
			btCollisionShape* shape = converter.createTrimesh();  //=new x2 todo:del?...
			shape->setUserPointer((void*)0);  // mark

			btCollisionObject* bco = new btCollisionObject();
			btTransform tr;  tr.setIdentity();  //tr.setOrigin(btVector3(pos.x,-pos.z,pos.y));
			bco->setActivationState(DISABLE_SIMULATION);  // WANTS_DEACTIVATION
			bco->setCollisionShape(shape);	bco->setWorldTransform(tr);
			bco->setFriction(0.7f);  bco->setRestitution(0.f);
			bco->setCollisionFlags(bco->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
			world->addCollisionObject(bco);
			#ifndef ROAD_EDITOR
			o.co = bco;
			pGame->collision.shapes.push_back(shape);
			#endif
		}
		else  ///  dynamic  . . . . . . . . . . . . 
		{
			// .bullet load
			fileLoader->mTrOfs.setOrigin(btVector3(o.pos[0],o.pos[1],o.pos[2]));
			fileLoader->mTrOfs.setRotation(btQuaternion(o.rot[0],o.rot[1],o.rot[2],o.rot[3]));
			//fileLoader->setVerboseMode(true);//
			std::string file = PATHMANAGER::GetDataPath()+"/objects/"+o.name+".bullet";

			if (fileLoader->loadFile(file.c_str()))
			{
				o.ms = fileLoader->ms;  // 1 only
				o.rb = fileLoader->rb;  // 1 only
				#if 0
				LogO(".bullet: "+o.name+
					"  shapes:"+toStr(fileLoader->getNumCollisionShapes())+
					"  bodies:"+toStr(fileLoader->getNumRigidBodies())+
					"  constr:"+toStr(fileLoader->getNumConstraints())); /**/
				#endif
			}else
				LogO(".bullet: Load Error: "+o.name);
		}
	}
	delete fileLoader;

	#ifdef ROAD_EDITOR
	iObjLast = sc.objects.size();
	#endif
}

///  destroy
void App::DestroyObjects(bool clear)
{
	for (int i=0; i < sc.objects.size(); ++i)
	{
		Object& o = sc.objects[i];
		// ogre
		if (o.nd)  mSceneMgr->destroySceneNode(o.nd);  o.nd = 0;
		#ifdef ROAD_EDITOR  // game has destroyAll
		if (o.ent)  mSceneMgr->destroyEntity(o.ent);  o.ent = 0;

		// bullet
		if (o.co)
		{	delete o.co->getCollisionShape();
			world->removeCollisionObject(o.co);
			delete o.co;  o.co = 0;
		}
		if (o.rb)
		{	delete o.rb->getCollisionShape();
			delete o.ms;  o.ms = 0;
			world->removeRigidBody(o.rb);
			delete o.rb;  o.rb = 0;
		}
		#endif
	}
	if (clear)
		sc.objects.clear();
}


//  Pick
//-------------------------------------------------------------------------------------------------------

#ifdef ROAD_EDITOR
void App::UpdObjPick()
{
	if (ndStBox)
		ndStBox->setVisible(edMode == ED_Start && !bMoveCam);

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

void App::PickObject()
{
	if (sc.objects.empty())  return;

	iObjCur = -1;
	const MyGUI::IntPoint& mp = MyGUI::InputManager::getInstance().getMousePosition();
	Real mx = Real(mp.left)/mWindow->getWidth(), my = Real(mp.top)/mWindow->getHeight();
	Ray ray = mCamera->getCameraToViewportRay(mx,my);  // 0..1

	//  query scene (aabbs are enough)
	RaySceneQuery* rq = mSceneMgr->createRayQuery(ray);
	rq->setSortByDistance(true);
	RaySceneQueryResult& res = rq->execute();

	Real dist = 100000.f;
	for (RaySceneQueryResult::iterator it = res.begin(); it != res.end(); ++it)
	{
		const String& s = (*it).movable->getName();
		//LogO("RAY "+s+" "+fToStr((*it).distance,2,4));

		if (StringUtil::startsWith(s,"oE",false))
		{
			int i = -1;
			//  find obj with same ent name
			for (int o=0; o < sc.objects.size(); ++o)
				if (s == sc.objects[o].ent->getName())
				{	i = o;  break;  }

			//  pick if closer
			if (i != -1 && (*it).distance < dist)
			{
				iObjCur = i;
				dist = (*it).distance;
			}
		}
	}
	//rq->clearResults();
	mSceneMgr->destroyQuery(rq);
}


///  toggle objects simulation (bullet world)
//-------------------------------------------------------------------------------------------------------
void App::ToggleObjSim()
{
	if (objPan)  objPan->setVisible(objSim);
	
	DestroyObjects(false);

	if (!objSim)  // off sim
	{
		// Destroy blt world
		for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject* obj = world->getCollisionObjectArray()[i];
			delete obj->getCollisionShape();
			
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
				delete body->getMotionState();

			ShapeData* sd = static_cast<ShapeData*>(obj->getUserPointer());
			delete sd;

			world->removeCollisionObject(obj);
			delete obj;
		}
	}
	else  // on sim
	{
		// Create blt world
		CreateBltTerrain();  road->RebuildRoadInt(false,true);
	}
	CreateObjects();
	UpdObjPick();
}

///  add new object
void App::AddNewObj()
{
	::Object o;  o.name = vObjNames[iObjTNew];
	++iObjLast;
	String s = toStr(iObjLast);  // counter for names
	///TODO: ?dyn objs size, !?get center,size, rmb height..

	//  pos, rot
	const Ogre::Vector3& v = road->posHit;
	o.pos[0] = v.x;  o.pos[1] =-v.z;  o.pos[2] = v.y + objNewH;
	QUATERNION<float> q,q1;
	q.SetAxisAngle(objNewYaw+PI_d*0.5f,0,0,-1);  q1.SetAxisAngle(PI_d,0,1,0);
	o.rot = q1*q;

	//  create object
	o.ent = mSceneMgr->createEntity("oE"+s, o.name + ".mesh");
	o.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode("oN"+s);
	o.SetFromBlt();
	o.nd->setScale(o.scale);
	o.nd->attachObject(o.ent);  o.ent->setVisibilityFlags(RV_Vegetation);

	sc.objects.push_back(o);
}

//  change obj to insert
void App::listObjsChngSt(MyGUI::List* l, size_t t)
{
	std::string s = objListSt->getItemNameAt(t).substr(7);
	for (int i=0; i < vObjNames.size(); ++i)
		if (s == vObjNames[i])
		{	SetObjNewType(i);  return;	}
}
void App::listObjsChngDyn(MyGUI::List* l, size_t t)
{
	std::string s = objListDyn->getItemNameAt(t).substr(7);
	for (int i=0; i < vObjNames.size(); ++i)
		if (s == vObjNames[i])
		{	SetObjNewType(i);  return;	}
}

//  preview model for insert
void App::SetObjNewType(int tnew)
{
	iObjTNew = tnew;
	//if (objList)  objList->setIndexSelected(iObjTNew);
	if (objNewNd)	{	mSceneMgr->destroySceneNode(objNewNd);  objNewNd = 0;  }
	if (objNewEnt)	{	mSceneMgr->destroyEntity(objNewEnt);  objNewEnt = 0;  }
	
	String name = vObjNames[iObjTNew];
	objNewEnt = mSceneMgr->createEntity("-oE", name + ".mesh");
	objNewNd = mSceneMgr->getRootSceneNode()->createChildSceneNode("-oN");
	objNewNd->attachObject(objNewEnt);  objNewEnt->setVisibilityFlags(RV_Vegetation);
	UpdObjNewNode();
}

void App::UpdObjNewNode()
{
	if (!road || !objNewNd)  return;

	objNewNd->setVisible(road->bHitTer && bEdit() && iObjCur == -1 && edMode == ED_Objects);
	Vector3 p = road->posHit;  p.y += objNewH;
	Quaternion q;  q.FromAngleAxis(Radian(objNewYaw), Vector3::UNIT_Y);
	
	objNewNd->setPosition(p);
	objNewNd->setOrientation(q);
}

#endif
