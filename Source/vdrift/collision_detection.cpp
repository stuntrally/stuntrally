#include "stdafx.h"

#include "collision_detection.h"
#include "unittest.h"

#include <list>
using std::list;

namespace COLLISION_DETECTION
{
btVector3 ToBulletVector(const MATHVECTOR <float, 3> & v)
{
   return btVector3(v[0],v[1],v[2]);
}

MATHVECTOR <float, 3> ToMathVector(const btVector3 & v)
{
   return MATHVECTOR <float, 3> (v.x(),v.y(),v.z());
}

btQuaternion ToBulletQuaternion(const QUATERNION <float> & q)
{
   return btQuaternion(q.x(), q.y(), q.z(), q.w());
}

QUATERNION <float> ToMathQuaternion(const btQuaternion & q)
{
   return QUATERNION <float> (q.x(), q.y(), q.z(), q.w());
}
};

void COLLISION_WORLD::CollideRay(const MATHVECTOR <float, 3> & position, const MATHVECTOR <float, 3> & direction, const float length, std::list <COLLISION_CONTACT> & outputcontactlist, const COLLISION_SETTINGS & settings) const
{
   outputcontactlist.clear();
   
   //return;
   
   MATHVECTOR <float, 3> rpos = position;
   MATHVECTOR <float, 3> rdir = direction;
   MATHVECTOR <float, 3> rayend = position + direction * length;
   AABB <float> raybox;
   raybox.SetFromCorners(position,rayend);
   
   btVector3 rayFrom = COLLISION_DETECTION::ToBulletVector(position);
   btVector3 rayTo = COLLISION_DETECTION::ToBulletVector(rayend);
   MultipleRayResultCallback colresult(rayFrom, rayTo);
   colresult.m_collisionFilterGroup = 0xFF;
   colresult.m_collisionFilterMask = settings.GetRayMask();
   btTransform rayFromTrans,rayToTrans;
   rayFromTrans.setIdentity();
   rayFromTrans.setOrigin(rayFrom);
   rayToTrans.setIdentity();
   rayToTrans.setOrigin(rayTo);
   
   if (settings.GetDynamicCollide())
      id->rayTest(rayFrom, rayTo, colresult);
   
   if (settings.GetStaticCollide())
   {
      std::list <COLLISION_OBJECT *> candidates;
      colspeedup.Query(AABB<float>::RAY(rpos, rdir, length), candidates);
      
      //cout << "collision candidates: " << candidates.size() << endl;
      for (std::list <COLLISION_OBJECT *>::iterator i = candidates.begin(); i != candidates.end(); ++i)
      {
         if (settings.CanCollide(**i))
         {
            btCollisionObject * collisionObject = &((*i)->GetBulletObject());
            id->rayTestSingle(rayFromTrans,rayToTrans,
                  collisionObject,
                  collisionObject->getCollisionShape(),
                  collisionObject->getWorldTransform(),
                  colresult);
         }
      }
   }
   
   //id.rayTest(rayFrom, rayTo, colresult, raymask);
   
   //cout << "Ray collisions: " << colresult.results.size() << endl;
   
   //if (!colresult.results.empty())
   if (!colresult.results.size())
   {
      //HandleCollision(settings, &colresult, outputcontactlist);
      
      //for (list <btCollisionWorld::LocalRayResult>::iterator i = colresult.results.begin(); i != colresult.results.end(); ++i)
      for (int i = 0; i < colresult.results.size(); ++i)
      {
         //cout << "processing collision" << endl;
         
         //-assert (colresult.results.at(i)->m_collisionObject); //assert collision hit is tied to an object
         
         if (PassesFilter(settings, colresult.results.at(i).m_collisionObject->getUserPointer()))// && (!i->m_collisionObject->getBroadphaseHandle() || i->m_collisionObject->getBroadphaseHandle()->m_collisionFilterGroup & raymask))
         {
            COLLISION_CONTACT newcont;
            outputcontactlist.push_back(newcont);
            
            MATHVECTOR <float, 3> normal;
            normal.Set(colresult.results.at(i).m_hitNormalLocal.x(),
               colresult.results.at(i).m_hitNormalLocal.y(),
               colresult.results.at(i).m_hitNormalLocal.z());
            normal = normal.Normalize();
            
            btVector3 hp;
            hp.setInterpolate3(colresult.m_rayFromWorld,colresult.m_rayToWorld,colresult.results.at(i).m_hitFraction);
            MATHVECTOR <float, 3> hpv;
            hpv.Set(hp.x(),hp.y(),hp.z());
            outputcontactlist.back().Set(hpv,
               normal,
               (colresult.m_rayToWorld - colresult.m_rayFromWorld).length() * colresult.results.at(i).m_hitFraction,
               (COLLISION_OBJECT*)colresult.results.at(i).m_collisionObject->getUserPointer(),
               (COLLISION_OBJECT*)colresult.results.at(i).m_collisionObject->getUserPointer());
            
            //outputcontactlist.pop_back();
            
            //cout << "PHYSICS::COLLIDERAY Collision hit: raylength=" << (colresult.m_rayToWorld - colresult.m_rayFromWorld).length() << ", hitfraction=" << i->m_hitFraction << ", depth=" << (colresult.m_rayToWorld - colresult.m_rayFromWorld).length() * i->m_hitFraction << ", hitpoint=";VERTEX(hp.x(),hp.y(),hp.z()).DebugPrint();
            //cout << "Collision hit: raylength=" << (colresult.m_rayToWorld - colresult.m_rayFromWorld).length() << ", hitfraction=" << i->m_hitFraction << ", depth=" << (colresult.m_rayToWorld - colresult.m_rayFromWorld).length() * i->m_hitFraction << ", normal=";normal.DebugPrint();
         }
         /*else
         {
            std::cout << "discarded collision due to filtering" << std::endl;
            if (i->m_collisionObject->getBroadphaseHandle())
               std::cout << i->m_collisionObject->getBroadphaseHandle()->m_collisionFilterGroup << ", " << raymask << std::endl;
         }*/
      }
   }
}

AABB <float> COLLISION_OBJECT::GetBBOX() const
{
   if (settings.GetType() == COLLISION_OBJECT_SETTINGS::STATIC)
      return bbox;
   else
   {
      btVector3 aabbMin, aabbMax;
      shape->getAabb(id.getWorldTransform(), aabbMin, aabbMax);
      AABB <float> transformedbox;
      transformedbox.SetFromCorners(COLLISION_DETECTION::ToMathVector(aabbMin), COLLISION_DETECTION::ToMathVector(aabbMax));
      return transformedbox;
   }
}

void COLLISION_WORLD::CollideObject(COLLISION_OBJECT & object, std::list <COLLISION_CONTACT> & outputcontactlist, const COLLISION_SETTINGS & settings) const
{
   outputcontactlist.clear();
   
   //return;
   
   //determine collision candidates in a speedy way
   std::list <COLLISION_OBJECT *> candidates;
   colspeedup.Query(object.GetBBOX(), candidates);
   
   btCollisionAlgorithm * algo(NULL);
   
   int last_num_manifolds = collisiondispatcher->getNumManifolds();
   
   //std::cout << "Manifolds before: " << collisiondispatcher.getNumManifolds() << std::endl;
   
   for (std::list <COLLISION_OBJECT *>::iterator i = candidates.begin(); i != candidates.end(); ++i)
   {
      algo = collisiondispatcher->findAlgorithm(&object.GetBulletObject(), &((*i)->GetBulletObject()));
      
      assert (algo); //make sure we can find a collision algorithm
      
      btManifoldResult manifoldout(&object.GetBulletObject(), &((*i)->GetBulletObject()));
      algo->processCollision(&object.GetBulletObject(), &((*i)->GetBulletObject()), id->getDispatchInfo(), &manifoldout);
   }

   //find the number of hits
   int num_manifolds = collisiondispatcher->getNumManifolds();
   
   assert(num_manifolds >= last_num_manifolds);
   
   //if (num_manifolds > 0) std::cout << num_manifolds << " manifolds after" << std::endl;
   
   for (int i=0; i < num_manifolds; i++)
   {
      //find the number of contacts
      btPersistentManifold* manifold = collisiondispatcher->getManifoldByIndexInternal(i);
      btCollisionObject* obA = static_cast<btCollisionObject*>(manifold->getBody0());
      btCollisionObject* obB = static_cast<btCollisionObject*>(manifold->getBody1());
      manifold->refreshContactPoints(obA->getWorldTransform(),obB->getWorldTransform());
      
      btCollisionObject * otherobject(NULL);
      bool obAisbox(obA == &object.GetBulletObject());
      
      if (obAisbox)
         otherobject = obB;
      else
         otherobject = obA;
      
      int num_contacts = manifold->getNumContacts();
      //if (num_contacts > 0) cout << i << ". " << num_contacts << " contacts" << endl;
      for (int j=0; j < num_contacts; j++)
      {
         btManifoldPoint& pt = manifold->getContactPoint(j);
         btVector3 ptA = pt.getPositionWorldOnA();
         btVector3 ptB = pt.getPositionWorldOnB();
         MATHVECTOR <float, 3> boxpt, otherpt;
         
         if (obAisbox)
         {
            boxpt = COLLISION_DETECTION::ToMathVector(ptA);
            otherpt = COLLISION_DETECTION::ToMathVector(ptB);
         }
         else
         {
            boxpt = COLLISION_DETECTION::ToMathVector(ptB);
            otherpt = COLLISION_DETECTION::ToMathVector(ptA);
         }

         if (PassesFilter(settings, otherobject->getUserPointer()))
         {
            COLLISION_CONTACT newcont;
            outputcontactlist.push_back(newcont);
            
            MATHVECTOR <float, 3> ptv = COLLISION_DETECTION::ToMathVector(pt.m_normalWorldOnB);
            
            outputcontactlist.back().Set(otherpt,
                              ptv,
                              (otherpt-boxpt).Magnitude(),
                              &object,
                              (COLLISION_OBJECT*)otherobject->getUserPointer());
         }
      }
      
      manifold->clearManifold();
   }
   
   if (algo)
      collisiondispatcher->freeCollisionAlgorithm(algo);
   
   for (int i=0; i < num_manifolds-last_num_manifolds; i++)
      collisiondispatcher->releaseManifold(collisiondispatcher->getManifoldByIndexInternal(collisiondispatcher->getNumManifolds()-1));
}

void COLLISION_WORLD::CollideDynamicObjects(std::map <COLLISION_OBJECT *, std::list <COLLISION_CONTACT> > & outputcontactlist) const
{
   outputcontactlist.clear();
   
   //return;
   
   id->performDiscreteCollisionDetection();
   
   int num_manifolds = collisiondispatcher->getNumManifolds();
   //std::cout << num_manifolds << std::endl;
   
   for (int i=0; i < num_manifolds; i++)
   {
      //find the number of contacts
      btPersistentManifold* manifold = collisiondispatcher->getManifoldByIndexInternal(i);
      btCollisionObject* obA = static_cast<btCollisionObject*>(manifold->getBody0());
      btCollisionObject* obB = static_cast<btCollisionObject*>(manifold->getBody1());
      manifold->refreshContactPoints(obA->getWorldTransform(),obB->getWorldTransform());
      
      int num_contacts = manifold->getNumContacts();
      //if (num_contacts > 0) cout << i << ". " << num_contacts << " contacts" << endl;
      for (int j=0; j < num_contacts; j++)
      {
         btManifoldPoint& pt = manifold->getContactPoint(j);
         
         MATHVECTOR <float, 3> ptA = COLLISION_DETECTION::ToMathVector(pt.getPositionWorldOnA());
         MATHVECTOR <float, 3> ptB = COLLISION_DETECTION::ToMathVector(pt.getPositionWorldOnB());

         //if (PassesFilter(settings, otherobject->getUserPointer()))
         {
            COLLISION_OBJECT* colobjA = (COLLISION_OBJECT*)obA->getUserPointer();
            COLLISION_OBJECT* colobjB = (COLLISION_OBJECT*)obB->getUserPointer();
            
            assert(colobjA);
            assert(colobjB);
            
            std::list <COLLISION_CONTACT> & outputlist = outputcontactlist[colobjA];
            outputlist.push_back(COLLISION_CONTACT());
            
            MATHVECTOR <float, 3> ptv = COLLISION_DETECTION::ToMathVector(pt.m_normalWorldOnB);
            
            outputlist.back().Set(ptB, ptv, (ptB-ptA).Magnitude(), colobjA, colobjB);
         }
      }
      
      manifold->clearManifold();
   }
   
   //for (int i=0; i < num_manifolds; i++) collisiondispatcher.releaseManifold(collisiondispatcher.getManifoldByIndexInternal(collisiondispatcher.getNumManifolds()-1));
}

bool COLLISION_WORLD::PassesFilter(const COLLISION_SETTINGS & settings, void * checkme) const
{
   bool exception = false;
   
   for (std::list <void *>::const_iterator i = settings.GetExceptionObjectIDs().begin(); i != settings.GetExceptionObjectIDs().end(); ++i)
   {
      exception = exception || (*i == checkme);
   }
   
   return !exception;
}

void COLLISION_OBJECT::InitTrimesh(const float * vertices, int vstride, int vcount, const int * faces, int fcount, int istride, const float * normals, const COLLISION_OBJECT_SETTINGS & objsettings)
{
   assert(!loaded); //Tried to double load a physics object
   
   if (shape != NULL || trimesh_varray != NULL)
      DeInit();
   
   assert(fcount % 3 == 0); //Face count is not a multiple of 3
   
   trimesh_varray = new btTriangleIndexVertexArray(fcount/3, (int*) faces, istride, vcount, (float*) vertices, vstride);
   shape = new btBvhTriangleMeshShape(trimesh_varray, true);
   //shape = new btGImpactMeshShape(trimesh_varray);
   
   id.getWorldTransform().setIdentity();
   id.setCollisionShape(shape);
   
   btVector3 AabbMin,AabbMax;
   id.getCollisionShape()->getAabb(id.getWorldTransform(),AabbMin,AabbMax);
   MATHVECTOR <float, 3> bboxmin;
   bboxmin.Set(AabbMin.x(),AabbMin.y(),AabbMin.z());
   MATHVECTOR <float, 3> bboxmax;
   bboxmax.Set(AabbMax.x(),AabbMax.y(),AabbMax.z());
   
   bbox.SetFromCorners(bboxmin, bboxmax);
   bbox.SetFromCorners(bboxmin, bboxmax);
   //std::cout << bboxmin << " -- " << bboxmax << std::endl;
   //id.setUserPointer(const_cast<void *>(objsettings.ObjID()));
   id.setUserPointer(this);
   settings = objsettings;
   
   loaded = true;
}

void COLLISION_OBJECT::InitConvexHull(const std::vector <float> & varray, const COLLISION_OBJECT_SETTINGS & objsettings)
{
   /*const float * vertices;
   int vcount;
   varray.GetVertices(vertices, vcount);*/
   
   assert(!loaded); //Tried to double load a physics object
   
   if (shape != NULL)
      DeInit();
   
   //shape = new btConvexHullShape(&(varray[0]), varray.size());
   btConvexHullShape * hull = new btConvexHullShape();
   shape = hull;
   for (unsigned int i = 0; i < varray.size(); i+=3)
   {
      btVector3 vert(varray[i],varray[i+1],varray[i+2]);
      hull->addPoint(vert);
   }
   
   /*btConvexHullShape * hull = new btConvexHullShape();
   shape = hull;
   for (int i = 0; i < vcount; i+=3)
   {
      btVector3 vert(vertices[i],vertices[i+1],vertices[i+2]);
      if (!hull->isInside(vert, 0))
         hull->addPoint(vert);
   }*/
   
   id.getWorldTransform().setIdentity();
   id.setCollisionShape(shape);
   id.setUserPointer(this);
   settings = objsettings;
   
   loaded = true;
}

void COLLISION_OBJECT::InitBox(const MATHVECTOR <float, 3> & halfextents, const COLLISION_OBJECT_SETTINGS & objsettings)
{
   assert(!loaded); //Tried to double load a physics object
   
   if (shape != NULL)
      DeInit();
   
   shape = new btBoxShape(COLLISION_DETECTION::ToBulletVector(halfextents));
   id.getWorldTransform().setIdentity();
   id.setCollisionShape(shape);
   bbox.SetFromCorners(halfextents, -halfextents);
   //id.setUserPointer(const_cast<void *>(objsettings.ObjID()));
   id.setUserPointer(this);
   settings = objsettings;
   
   loaded = true;
}

void COLLISION_OBJECT::InitCylinderZ(const MATHVECTOR <float, 3> & halfextents, const COLLISION_OBJECT_SETTINGS & objsettings)
{
   assert(!loaded); //Tried to double load a physics object
   
   if (shape != NULL)
      DeInit();
   
   shape = new btCylinderShapeZ(COLLISION_DETECTION::ToBulletVector(halfextents));
   id.getWorldTransform().setIdentity();
   id.setCollisionShape(shape);
   bbox.SetFromCorners(halfextents, -halfextents);
   //id.setUserPointer(const_cast<void *>(objsettings.ObjID()));
   id.setUserPointer(this);
   settings = objsettings;
   
   loaded = true;
}

void COLLISION_OBJECT::InitTrimesh(const VERTEXARRAY & varray, const COLLISION_OBJECT_SETTINGS & objsettings)
{
   const float * vertices;
   int vcount;
   const int * faces;
   int fcount;
   float * normals(NULL);
   varray.GetVertices(vertices, vcount);
   varray.GetFaces(faces, fcount);
   //std::cout << "verts: " << vcount << ", faces: " << fcount << std::endl;
   InitTrimesh(vertices, sizeof(float)*3, vcount, faces, fcount, sizeof(int)*3, normals, objsettings);
}

void COLLISION_OBJECT::DeInit()
{
   if (trimesh_varray != NULL)
      delete trimesh_varray;
   trimesh_varray = NULL;
   
   if (shape != NULL)
      delete shape;
   shape = NULL;
}

void COLLISION_OBJECT::SetPosition(const MATHVECTOR <float, 3> & newpos)
{
   assert(loaded); //Physics object not loaded yet
   id.getWorldTransform().setOrigin(btVector3(newpos[0],newpos[1],newpos[2]));
}

MATHVECTOR <float, 3> COLLISION_OBJECT::GetPosition() const
{
   assert(loaded); //Physics object not loaded yet
   btVector3 newpos = id.getWorldTransform().getOrigin();
   return MATHVECTOR <float, 3> (newpos[0],newpos[1],newpos[2]);
}

void COLLISION_OBJECT::SetQuaternion(const QUATERNION <float> & newquat)
{
   assert(loaded); //Physics object not loaded yet
   id.getWorldTransform().setRotation(btQuaternion(newquat.x(),newquat.y(),newquat.z(),newquat.w()));
}

bool COLLISION_CONTACT::CollideRay(const MATHVECTOR <float, 3> & origin, const MATHVECTOR <float, 3> & direction, const float length, COLLISION_CONTACT & output_contact) const
{
   /*//simple approximation
   output_contact.Set(position, normal, depth, col1, col2);
   return true;*/

   //enhanced plane-based approximation
   float D = - normal.dot ( position );
   float Pn_dot_Rd = normal.dot ( direction );
   bool newcon ( true );
   if ( Pn_dot_Rd != 0 )
   {
      float t = - ( normal.dot ( origin ) + D ) / ( Pn_dot_Rd );
      if ( t >= 0 )
      {
         MATHVECTOR <float, 3> newpos = origin + direction*t;

         float newdepth = t;

         output_contact.Set(newpos, normal, newdepth, col1, col2);
      }
      else
         newcon = false;
   }
   else
      newcon = false;
   
   return newcon;
}

void COLLISION_WORLD::CollideBox(const MATHVECTOR <float, 3> & position, const QUATERNION <float> & orientation, const MATHVECTOR <float, 3> & dimensions, std::list <COLLISION_CONTACT> & outputcontactlist, const COLLISION_SETTINGS & settings) const
{
   COLLISION_OBJECT_SETTINGS boxsettings;
   boxsettings.SetDynamicObject();
   COLLISION_OBJECT mybox;
   mybox.InitBox(dimensions, boxsettings);
   mybox.SetPosition(position);
   mybox.SetQuaternion(orientation);
   
   CollideObject(mybox, outputcontactlist, settings);
}

void COLLISION_WORLD::CollideMovingBox(const MATHVECTOR <float, 3> & position, const MATHVECTOR <float, 3> & velocity, const QUATERNION <float> & orientation, const MATHVECTOR <float, 3> & half_dimensions, std::list <COLLISION_CONTACT> & outputcontactlist, const COLLISION_SETTINGS & settings, float dt) const
{
   //CollideBox(position, orientation, half_dimensions, outputcontactlist, settings);
   
   //extend our box back in the direction of negative velocity
   MATHVECTOR <float, 3> poschange = velocity*dt;
   MATHVECTOR <float, 3> velocity_local = poschange;
   (-orientation).RotateVector(velocity_local);
   MATHVECTOR <float, 3> newposition = position-poschange*0.5;
   MATHVECTOR <float, 3> absvel = velocity_local;
   absvel.absify();
   MATHVECTOR <float, 3> newdimensions = half_dimensions+absvel*0.5;
   
   /*std::cout << poschange << std::endl;
   std::cout << velocity_local << std::endl;
   std::cout << position << " to " << newposition << std::endl;
   std::cout << half_dimensions << " to " << newdimensions << std::endl;*/
   
   CollideBox(newposition, orientation, newdimensions, outputcontactlist, settings);
   
   for (std::list <COLLISION_CONTACT>::iterator i = outputcontactlist.begin(); i != outputcontactlist.end(); ++i)
   {
      
   }
}

QT_TEST(collision_test)
{
   {
      COLLISION_OBJECT_SETTINGS settings;
      QT_CHECK_EQUAL(settings.GetMask(),1);
      QT_CHECK_EQUAL(settings.GetGroup(),1);
      settings.SetDynamicObjectMask(0,false);
      QT_CHECK_EQUAL(settings.GetMask(),0);
      settings.SetDynamicObjectGroup(2,true);
      QT_CHECK_EQUAL(settings.GetGroup(),5);
      settings.SetDynamicObjectGroup(0,false);
      QT_CHECK_EQUAL(settings.GetGroup(),4);
   }
   {
      COLLISION_OBJECT_SETTINGS settings;
      QT_CHECK_EQUAL(settings.GetGroup(),1);
      settings.SetDynamicObjectGroup(0,false);
      settings.SetDynamicObjectGroup(1,true);
      QT_CHECK_EQUAL(settings.GetGroup(),2);
   }
}
//and of course, youhave to remove #include <stl/_auto_ptr.h> from the stdafx.h
//and the stlportxxx.lib library link reference.
