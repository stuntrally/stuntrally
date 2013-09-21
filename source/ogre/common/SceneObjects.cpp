#include "pch.h"
#include "RenderConst.h"
#include "Def_Str.h"
#include "../common/data/SceneXml.h"
#include "../../vdrift/pathmanager.h"
#include "../../btOgre/BtOgreGP.h"
#include "../../road/Road.h"
#include "../common/ShapeData.h"

#ifdef SR_EDITOR
	#include "../../editor/CApp.h"
	#include "../../editor/CGui.h"
#else
	#include "../CGame.h"
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
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
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
	
	//todo: shape->setUserPointer((void*)SU_ObjectDynamic);  // mark shapes..
	
	btCollisionObject* createCollisionObject(const btTransform& startTransform,btCollisionShape* shape, const char* bodyName)
	{
		return createRigidBody(false,0,startTransform,shape,bodyName);
	}

	btRigidBody* createRigidBody(bool isDynamic, btScalar mass, const btTransform& startTransform,
								btCollisionShape* shape, const char* bodyName)
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

		#ifdef SR_EDITOR
		//body->setActivationState(DISABLE_DEACTIVATION);
		#else
		body->setActivationState(WANTS_DEACTIVATION);  // game creates deactivated (sleeping)
		#endif

		if (m_dynamicsWorld)
			m_dynamicsWorld->addRigidBody(body);
		
		/*if (bodyName)
		{
			char* newname = duplicateName(bodyName);
			m_objectNameMap.insert(body,newname);
			m_nameBodyMap.insert(newname,body);
		}*/
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
	
	for (int i=0; i < sc->objects.size(); ++i)
	{
		const string& s = sc->objects[i].name;
		objExists[s] = false;  objHasBlt[s] = false;
	}
	for (map<string,bool>::iterator it = objExists.begin(); it != objExists.end(); ++it)
	{
		bool ex = boost::filesystem::exists(PATHMANAGER::Data()+"/objects/"+ (*it).first + ".mesh");
		(*it).second = ex;
		if (!ex)  LogO("CreateObjects mesh doesn't exist: " + (*it).first + ".mesh");
	}
	for (map<string,bool>::iterator it = objHasBlt.begin(); it != objHasBlt.end(); ++it)
		(*it).second = boost::filesystem::exists(PATHMANAGER::Data()+"/objects/"+ (*it).first + ".bullet");

	//  loader
	#ifndef SR_EDITOR
	btDiscreteDynamicsWorld* world = pGame->collision.world;
	#endif
	BulletWorldOffset* fileLoader = new BulletWorldOffset(world);

	///  create  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
	for (int i=0; i < sc->objects.size(); ++i)
	{
		Object& o = sc->objects[i];
		String s = toStr(i);  // counter for names
		o.dyn = objHasBlt[o.name];
		#ifndef SR_EDITOR
		if (o.dyn && !pSet->game.dyn_objects)  continue;
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
		if (!o.dyn)
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
			shape->setUserPointer((void*)SU_ObjectStatic);  // mark

			btCollisionObject* bco = new btCollisionObject();
			btTransform tr;  tr.setIdentity();  //tr.setOrigin(btVector3(pos.x,-pos.z,pos.y));
			bco->setActivationState(DISABLE_SIMULATION);  // WANTS_DEACTIVATION
			bco->setCollisionShape(shape);	bco->setWorldTransform(tr);
			bco->setFriction(0.7f);   //+
			bco->setRestitution(0.f);
			bco->setCollisionFlags(bco->getCollisionFlags() |
				btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
			world->addCollisionObject(bco);
			#ifndef SR_EDITOR
			o.co = bco;  o.ms = 0;  o.rb = 0;
			pGame->collision.shapes.push_back(shape);
			#endif
		}
		else  ///  dynamic  . . . . . . . . . . . . 
		{
			// .bullet load
			fileLoader->mTrOfs.setOrigin(btVector3(o.pos[0],o.pos[1],o.pos[2]));
			fileLoader->mTrOfs.setRotation(btQuaternion(o.rot[0],o.rot[1],o.rot[2],o.rot[3]));
			//fileLoader->setVerboseMode(true);//
			std::string file = PATHMANAGER::Data()+"/objects/"+o.name+".bullet";

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

	#ifdef SR_EDITOR
	gui->iObjLast = sc->objects.size();
	#endif
}

///  destroy
void App::DestroyObjects(bool clear)
{
	for (int i=0; i < sc->objects.size(); ++i)
	{
		Object& o = sc->objects[i];
		// ogre
		if (o.nd)  mSceneMgr->destroySceneNode(o.nd);  o.nd = 0;
		#ifdef SR_EDITOR  // game has destroyAll
		if (o.ent)  mSceneMgr->destroyEntity(o.ent);  o.ent = 0;
		#endif

		#ifdef SR_EDITOR
		// bullet
		if (o.co)
		{	delete o.co->getCollisionShape();
			#ifdef SR_EDITOR
			world->removeCollisionObject(o.co);
			#else
			pGame->collision.world->removeCollisionObject(o.co);
			#endif
			delete o.co;  o.co = 0;
		}
		if (o.rb)
		{	delete o.rb->getCollisionShape();
			delete o.ms;  o.ms = 0;
			#ifdef SR_EDITOR
			world->removeRigidBody(o.rb);
			#else
			pGame->collision.world->removeCollisionObject(o.rb);
			#endif
			delete o.rb;  o.rb = 0;
		}
		#endif
	}
	if (clear)
		sc->objects.clear();
}


//  Pick
//-------------------------------------------------------------------------------------------------------

#ifdef SR_EDITOR
void App::UpdObjPick()
{
	if (ndStBox)
		ndStBox->setVisible(edMode == ED_Start && !bMoveCam);

	int objs = sc->objects.size();
	bool bObjects = edMode == ED_Objects && !bMoveCam && objs > 0 && gui->iObjCur >= 0;
	if (objs > 0)
		gui->iObjCur = std::min(gui->iObjCur, objs-1);

	if (!ndObjBox)  return;
	ndObjBox->setVisible(bObjects);
	if (!bObjects)  return;
	
	const Object& o = sc->objects[gui->iObjCur];
	const AxisAlignedBox& ab = o.nd->getAttachedObject(0)->getBoundingBox();
	//Vector3 p = ab.getCenter();
	Vector3 s = o.scale * ab.getSize();  // * sel obj's node aabb

		//Vector3 posO = Vector3(o.pos[0]+p.x,o.pos[2]+p.z,-o.pos[1]-p.y);
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
	if (sc->objects.empty())  return;

	gui->iObjCur = -1;
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
			for (int o=0; o < sc->objects.size(); ++o)
				if (s == sc->objects[o].ent->getName())
				{	i = o;  break;  }

			//  pick if closer
			if (i != -1 && (*it).distance < dist)
			{
				gui->iObjCur = i;
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
	if (gui->objPan)  gui->objPan->setVisible(gui->objSim);
	
	DestroyObjects(false);

	if (!gui->objSim)  // off sim
	{
		// Destroy blt world
		for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject* obj = world->getCollisionObjectArray()[i];
			delete obj->getCollisionShape();
			
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
				delete body->getMotionState();

			if (obj->getUserPointer() != (void*)111)
			{
				ShapeData* sd = static_cast<ShapeData*>(obj->getUserPointer());
				delete sd;
			}
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
void CGui::AddNewObj()  //App..
{
	::Object o = objNew;
	o.name = vObjNames[iObjTNew];
	++iObjLast;
	String s = toStr(iObjLast);  // counter for names
	///TODO: ?dyn objs size, !?get center,size, rmb height..

	//  pos, rot
	const Ogre::Vector3& v = app->road->posHit;
	o.pos[0] = v.x;  o.pos[1] =-v.z;  o.pos[2] = v.y + objNew.pos[2];

	//  create object
	o.ent = app->mSceneMgr->createEntity("oE"+s, o.name + ".mesh");
	o.nd = app->mSceneMgr->getRootSceneNode()->createChildSceneNode("oN"+s);
	o.SetFromBlt();
	o.nd->setScale(o.scale);
	o.nd->attachObject(o.ent);  o.ent->setVisibilityFlags(RV_Vegetation);

	o.dyn = boost::filesystem::exists(PATHMANAGER::Data()+"/objects/"+ o.name + ".bullet");
	sc->objects.push_back(o);
}

//  change obj to insert
void CGui::listObjsChng(MyGUI::List* l, size_t t)
{
	std::string s = l->getItemNameAt(t).substr(7);
	for (int i=0; i < vObjNames.size(); ++i)
		if (s == vObjNames[i])
		{
			SetObjNewType(i);
			Upd3DView(s+".mesh");
			return;
		}
}

//  preview model for insert
void CGui::SetObjNewType(int tnew)
{
	iObjTNew = tnew;
	if (objNew.nd)	{	app->mSceneMgr->destroySceneNode(objNew.nd);  objNew.nd = 0;  }
	if (objNew.ent)	{	app->mSceneMgr->destroyEntity(objNew.ent);  objNew.ent = 0;  }
	
	String name = vObjNames[iObjTNew];
	objNew.dyn = boost::filesystem::exists(PATHMANAGER::Data()+"/objects/"+ name + ".bullet");
	if (objNew.dyn)  objNew.scale = Vector3::UNIT_SCALE;  // dyn no scale
	objNew.ent = app->mSceneMgr->createEntity("-oE", name + ".mesh");
	objNew.nd = app->mSceneMgr->getRootSceneNode()->createChildSceneNode("-oN");
	objNew.nd->attachObject(objNew.ent);  objNew.ent->setVisibilityFlags(RV_Vegetation);
	UpdObjNewNode();

	if (objListSt)  objListSt->setIndexSelected(-1);  // unselect
	if (objListDyn) objListDyn->setIndexSelected(-1);
	if (objListBld) objListBld->setIndexSelected(-1);
}

void CGui::UpdObjNewNode()
{
	if (!app->road || !objNew.nd)  return;

	bool vis = app->road->bHitTer && app->bEdit() && iObjCur == -1 && app->edMode == ED_Objects;
	objNew.nd->setVisible(vis);
	if (!vis)  return;
	
	Vector3 p = app->road->posHit;  p.y += objNew.pos[2];
	objNew.SetFromBlt();
	objNew.nd->setPosition(p);
	objNew.nd->setScale(objNew.scale);
}

#endif
