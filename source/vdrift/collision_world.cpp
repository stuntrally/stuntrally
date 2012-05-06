#include "pch.h"
#include "../ogre/common/Defines.h"
#include "collision_world.h"

#include "tobullet.h"
#include "collision_contact.h"
#include "model.h"
#include "track.h"
#include "cardynamics.h"
//#include "car.h"//
#include <OgreLogManager.h>


///  hit callback (accurate)
//-------------------------------------------------------------------------------------------------------------------------------
void IntTickCallback(btDynamicsWorld *world, btScalar timeStep)
{
	COLLISION_WORLD* cw = (COLLISION_WORLD*)world->getWorldUserInfo();

	int numManifolds = world->getDispatcher()->getNumManifolds();
	for (int i=0; i < numManifolds; ++i)  // pairs
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* bA = static_cast<btCollisionObject*>(contactManifold->getBody0());
		btCollisionObject* bB = static_cast<btCollisionObject*>(contactManifold->getBody1());
	
		if (bA->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE ||
			bB->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE)  // ignore triggers
			continue;

		void* pA = bA->getUserPointer(), *pB = bB->getUserPointer();
		//if (pA && pB)
		{
			ShapeData* sdA = (ShapeData*)pA, *sdB = (ShapeData*)pB, *sdCar=0, *sdFluid=0, *sdWheel=0;
			if (sdA) {  if (sdA->type == ST_Car)  sdCar = sdA;  else if (sdA->type == ST_Fluid)  sdFluid = sdA;  else if (sdA->type == ST_Wheel)  sdWheel = sdA;  }
			if (sdB) {  if (sdB->type == ST_Car)  sdCar = sdB;  else if (sdB->type == ST_Fluid)  sdFluid = sdB;  else if (sdB->type == ST_Wheel)  sdWheel = sdB;  }

			if (sdCar &&/**/ !sdFluid && !sdWheel)
			{
				int numContacts = contactManifold->getNumContacts();
				for (int j=0; j < numContacts; ++j)
				{
					btManifoldPoint& pt = contactManifold->getContactPoint(j);
					btScalar f = pt.getAppliedImpulse() * timeStep;
					if (f > 0.f)
					{
						//LogO(fToStr(bA->getInterpolationLinearVelocity().length(),3,5)+" "+fToStr(bB->getInterpolationLinearVelocity().length(),3,5));
						//LogO(toStr(i)+" "+toStr(j)+" "+fToStr(f,2,4));
						DynamicsWorld::Hit hit;
						hit.pos = pt.getPositionWorldOnA();  hit.norm = pt.m_normalWorldOnB;
						hit.force = f;  hit.sdCar = sdCar;
						//hit.force = std::max(0, 60 - pt.getLifeTime());
						//LogO(fToStr(hit.force,2,4));
						hit.vel = sdCar ? sdCar->pCarDyn->velPrev : (btVector3(1,1,1)*0.1f);
						cw->world->vHits.push_back(hit);
						//sdCar->pCarDyn->hitPnts.push_back(pt);  ///i
					}
				}
			}
		}
	}
}

///  collision callback - only for fluid triggers
//-------------------------------------------------------------------------------------------------------------------------------
void DynamicsWorld::solveConstraints(btContactSolverInfo& solverInfo)
{
	btDiscreteDynamicsWorld::solveConstraints(solverInfo);

	int numManifolds = getDispatcher()->getNumManifolds();
	for (int i=0; i < numManifolds; ++i)  // pairs
	{
		btPersistentManifold* contactManifold =  getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* bA = static_cast<btCollisionObject*>(contactManifold->getBody0());
		btCollisionObject* bB = static_cast<btCollisionObject*>(contactManifold->getBody1());
	
		void* pA = bA->getUserPointer(), *pB = bB->getUserPointer();
		//if (pA && pB)
		{
			ShapeData* sdA = (ShapeData*)pA, *sdB = (ShapeData*)pB, *sdCar=0, *sdFluid=0, *sdWheel=0;
			if (sdA) {  if (sdA->type == ST_Car)  sdCar = sdA;  else if (sdA->type == ST_Fluid)  sdFluid = sdA;  else if (sdA->type == ST_Wheel)  sdWheel = sdA;  }
			if (sdB) {  if (sdB->type == ST_Car)  sdCar = sdB;  else if (sdB->type == ST_Fluid)  sdFluid = sdB;  else if (sdB->type == ST_Wheel)  sdWheel = sdB;  }

			if (sdFluid)  /// wheel - fluid  -----~~~------~~~-----
				if (sdWheel)
				{
					std::list<FluidBox*>& fl = sdWheel->pCarDyn->inFluidsWh[sdWheel->whNum];
					if (fl.empty())
						fl.push_back(sdFluid->pFluid);  // add fluid to wheel (only 1)
					//LogO("Wheel+ Fluid " + toStr(sdWheel->whNum));
				}else
				if (sdCar)  /// car - fluid  -----~~~------~~~-----
				{
					if (sdCar->pCarDyn->inFluids.empty())
						sdCar->pCarDyn->inFluids.push_back(sdFluid->pFluid);  // add fluid to car (only 1)
				}
		}
	}
}

//  Sim Update
//-------------------------------------------------------------------------------------------------------------------------------
void COLLISION_WORLD::Update(double dt, bool profiling)
{
	///  Simulate
	world->stepSimulation(dt, maxSubsteps, fixedTimestep);
	
	//  use collision hit results, once a frame
	int n = world->vHits.size();
	if (n > 0)
	{
		//LogO(toStr(n));
		//  pick the one with biggest force
		DynamicsWorld::Hit& hit = world->vHits[0];
		float force = 0.f;//, vel = 0.f;
		for (int i=0; i < n; ++i)
			if (world->vHits[i].force > force)
			{
				force = world->vHits[i].force;
				hit = world->vHits[i];
			}

		CARDYNAMICS* cd = hit.sdCar->pCarDyn;
		cdOld = cd;
		btVector3 vcar = hit.vel;
		Ogre::Vector3 vel(vcar[0], vcar[2], -vcar[1]);
		Ogre::Vector3 norm(hit.norm.getX(), hit.norm.getZ(), -hit.norm.getY());
		float vlen = vel.length(), normvel = abs(vel.dotProduct(norm));

		//  Sparks emit params
		cd->vHitPos = Ogre::Vector3(hit.pos.getX(), hit.pos.getZ(), -hit.pos.getY());
		cd->vHitNorm = norm + vel * 0.1f;
		cd->fParVel = 3.0f + 0.4f * vlen;
		cd->fParIntens = 10.f + 30.f * vlen;

		//  hit force for sound
		cd->fHitForce = normvel*0.02f /*+ 0.02f*vlen*/;  //+
		cd->fHitForce2 = force*0.1f;
		
		//float a = (vlen*0.1f*powf(force, 0.2f) - 0*cd->fHitForce4);
		float a = std::min(1.f, std::min(vlen, 2.2f)*0.1f*powf(force, 0.4f) );
		if (a > cd->fCarScrap)  cd->fCarScrap = a;

		float b = std::min(1.f, 0.2f*force);
		if (b > cd->fCarScreech)  cd->fCarScreech = b;
			
		//if (cd->fHitForce4 > 0.f)
		//	cd->fHitForce4 -= 0.04f * dt;
		//if (force > 0.5f)
			cd->fHitTime = 1.f;
		///LogO("upd sf " + toStr(cd->fHitForce) + " force " + toStr(hit.force) + " vel " + toStr(vlen) + " Nvel " + toStr(normvel));
	}
	else if (cdOld)
	{
		cdOld->fHitForce  = 0.f;
		cdOld->fHitForce2 = 0.f;
		//cdOld->fHitForce4 = 0.f;
		//cdOld->fHitForce3 = cdOld->fHitTime;
		//cdOld = 0;
	}

	world->vHits.clear();//+
}
//-------------------------------------------------------------------------------------------------------------------------------


///  ctor bullet world
COLLISION_WORLD::COLLISION_WORLD() :
	config(0), dispatcher(0), broadphase(0), solver(0), world(0), cdOld(0),
	track(NULL), trackObject(NULL), trackMesh(NULL),
	fixedTimestep(1.0/60.0), maxSubsteps(7)  // default, set from settings
{
	config = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(config);

	broadphase = new bt32BitAxisSweep3(btVector3(-5000, -5000, -5000), btVector3(5000, 5000, 5000));
	solver = new btSequentialImpulseConstraintSolver();
	world = new DynamicsWorld(dispatcher, broadphase, solver, config);

	world->setGravity(btVector3(0.0, 0.0, -9.81)); ///~
	//world->getSolverInfo().m_numIterations = 36;  //-
	//world->getSolverInfo().m_erp = 0.3;
	//world->getSolverInfo().m_erp2 = 0.2;
	//world->getSolverInfo().m_splitImpulse = true;
	world->getSolverInfo().m_restitution = 0.0f;
	world->getDispatchInfo().m_enableSPU = true;
	world->setForceUpdateAllAabbs(false);  //+

	world->setInternalTickCallback(IntTickCallback,this,false);
}

COLLISION_WORLD::~COLLISION_WORLD()
{
	Clear();

	delete world;
	delete solver;
	delete broadphase;
	delete dispatcher;
	delete config;
}

void COLLISION_WORLD::DebugDrawScene()
{
}

btCollisionObject * COLLISION_WORLD::AddCollisionObject(const MODEL & model)
{
	btCollisionObject * col = new btCollisionObject();
	btCollisionShape * shape = AddMeshShape(model);
	col->setCollisionShape(shape);
	world->addCollisionObject(col);
	return col;
}

btRigidBody * COLLISION_WORLD::AddRigidBody(const btRigidBody::btRigidBodyConstructionInfo & info,
	bool car, bool bCarsCollis)
{
	btRigidBody * body = new btRigidBody(info);
	btCollisionShape * shape = body->getCollisionShape();
	//body->setActivationState(DISABLE_DEACTIVATION);  //!-for chassis only
	//body->setCollisionFlags
	#define  COL_CAR  (1<<2)
	if (car)  world->addRigidBody(body, COL_CAR, 255 - (!bCarsCollis ? COL_CAR : 0));  // group, mask
	else	  world->addRigidBody(body);
	shapes.push_back(shape);
	return body;
}

void COLLISION_WORLD::AddAction(btActionInterface * action)
{
	world->addAction(action);
	actions.push_back(action);
}

void COLLISION_WORLD::AddConstraint(btTypedConstraint * constraint, bool disableCollisionsBetweenLinked)
{
	world->addConstraint(constraint, disableCollisionsBetweenLinked);
	constraints.push_back(constraint);
}

void COLLISION_WORLD::SetTrack(TRACK * t)
{
	assert(t);
	
	// remove old track
	if(track)
	{
		world->removeCollisionObject(trackObject);
		
		delete trackObject->getCollisionShape();
		trackObject->setCollisionShape(NULL);
		
		delete trackObject;
		trackObject = NULL;
		
		delete trackMesh;
		trackMesh = NULL;
	}
	
	// setup new track
	track = t;
	trackMesh = new btTriangleIndexVertexArray();
	trackSurface.resize(0);
	const std::list<TRACK_OBJECT> & objects = track->GetTrackObjects();
	for(std::list<TRACK_OBJECT>::const_iterator ob = objects.begin(); ob != objects.end(); ++ob)
	{
		if(ob->GetSurface() != NULL)
		{
			MODEL & model = *ob->GetModel();
			btIndexedMesh mesh = GetIndexedMesh(model);
			trackMesh->addIndexedMesh(mesh);
			const TRACKSURFACE * surface = ob->GetSurface();
			trackSurface.push_back(surface);
		}
	}
	
	//  no objs track
	if (trackSurface.size()==0)
	{
		static TRACKSURFACE surface;
		trackSurface.push_back(&surface);
	}
	else  ///
	{
	// can not use QuantizedAabbCompression because of the track size
	btCollisionShape * trackShape = new btBvhTriangleMeshShape(trackMesh, false);
	trackObject = new btCollisionObject();
	trackObject->setCollisionShape(trackShape);
	trackObject->setUserPointer(NULL);
	
	world->addCollisionObject(trackObject);
	}
}

btIndexedMesh COLLISION_WORLD::GetIndexedMesh(const MODEL & model)
{
	const float * vertices;  int vcount;
	const int * faces;  int fcount;
	model.GetVertexArray().GetVertices(vertices, vcount);
	model.GetVertexArray().GetFaces(faces, fcount);
	
	assert(fcount % 3 == 0); //Face count is not a multiple of 3
	
	btIndexedMesh mesh;
	mesh.m_numTriangles = fcount / 3;
	mesh.m_triangleIndexBase = (const unsigned char *)faces;
	mesh.m_triangleIndexStride = sizeof(int) * 3;
	mesh.m_numVertices = vcount;
	mesh.m_vertexBase = (const unsigned char *)vertices;
	mesh.m_vertexStride = sizeof(float) * 3;
	mesh.m_vertexType = PHY_FLOAT;
	return mesh;
}

btCollisionShape * COLLISION_WORLD::AddMeshShape(const MODEL & model)
{
	btTriangleIndexVertexArray * mesh = new btTriangleIndexVertexArray();
	mesh->addIndexedMesh(GetIndexedMesh(model));
	btCollisionShape * shape = new btBvhTriangleMeshShape(mesh, true);
	
	meshes.push_back(mesh);
	shapes.push_back(shape);
	
	return shape;
}


struct MyRayResultCallback : public btCollisionWorld::RayResultCallback
{
	MyRayResultCallback(const btVector3 & rayFromWorld, const btVector3 & rayToWorld, const btCollisionObject * exclude, bool ignoreCars, bool ignoreFluids)
		: m_rayFromWorld(rayFromWorld), m_rayToWorld(rayToWorld), m_exclude(exclude), m_shapeId(0), bIgnoreCars(ignoreCars), bIgnoreFluids(ignoreFluids)
	{	}

	btVector3	m_rayFromWorld;//used to calculate hitPointWorld from hitFraction
	btVector3	m_rayToWorld;

	btVector3	m_hitNormalWorld;
	btVector3	m_hitPointWorld;
	
	int m_shapeId;
	const btCollisionObject * m_exclude;
	bool bIgnoreCars,bIgnoreFluids;
		
	virtual	btScalar	addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		btCollisionObject* obj = rayResult.m_collisionObject;
		if (obj == m_exclude)
			return 1.0;
			
		//  no other cars collision (for wheel raycasts)
		if (bIgnoreCars)
		{
			ShapeData* sd = (ShapeData*)obj->getUserPointer();
			if (sd && sd->type == ST_Car)
				return 1.0;
		}
			
		//  fluid triggers - no collision (but collide for camera rays)
		if (bIgnoreFluids && (obj->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE))
			return 1.0;
		
		//caller already does the filter on the m_closestHitFraction
		btAssert(rayResult.m_hitFraction <= m_closestHitFraction);
		
		m_closestHitFraction = rayResult.m_hitFraction;
		m_collisionObject = obj;

		if (!rayResult.m_localShapeInfo)
			m_shapeId = 0;  //crash hf-
		else  // only for btTriangleMeshShape
			m_shapeId = rayResult.m_localShapeInfo->m_shapePart;

		if (normalInWorldSpace)
		{
			m_hitNormalWorld = rayResult.m_hitNormalLocal;
		}
		else
		{
			///need to transform normal into worldspace
			m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;
		}
		m_hitPointWorld.setInterpolate3(m_rayFromWorld,m_rayToWorld,rayResult.m_hitFraction);
		return rayResult.m_hitFraction;
	}
};


//  Ray
//-------------------------------------------------------------------------------------------------------------------------------
bool COLLISION_WORLD::CastRay(
	const MATHVECTOR <float, 3> & origin,
	const MATHVECTOR <float, 3> & direction,
	const float length,
	const btCollisionObject * caster,
	COLLISION_CONTACT & contact,
	int* pOnRoad, bool ignoreCars, bool ignoreFluids) const
{
	btVector3 from = ToBulletVector(origin);
	btVector3 to = ToBulletVector(origin + direction * length);
	MyRayResultCallback rayCallback(from, to, caster, ignoreCars, ignoreFluids);
	
	MATHVECTOR <float, 3> p, n;  float d;
	const TRACKSURFACE * s = TRACKSURFACE::None();
	const BEZIER * b = NULL;
	btCollisionObject * c = NULL;
	
	// track geometry collision
	world->rayTest(from, to, rayCallback);
	bool geometryHit = rayCallback.hasHit();
	if (geometryHit)
	{
		p = ToMathVector<float>(rayCallback.m_hitPointWorld);
		n = ToMathVector<float>(rayCallback.m_hitNormalWorld);
		d = rayCallback.m_closestHitFraction * length;
		c = rayCallback.m_collisionObject;
		if (c->isStaticObject() /*&& (c->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE == 0)*/)
		{
			void * ptr = c->getCollisionShape()->getUserPointer();
			if (ptr == (void*)7777)  // road
			{
				s = trackSurface[0];
				*pOnRoad = 1;
			}
			if (ptr == (void*)7788)  // pipe
			{
				s = trackSurface[0];
				*pOnRoad = 2;
			}
			else if (ptr == 0)
			{
				*pOnRoad = 0;
				void * ptr = c->getUserPointer();
				if (ptr != NULL)
				{
					const TRACK_OBJECT * const obj = reinterpret_cast <const TRACK_OBJECT * const> (ptr);
					assert(obj);
					s = obj->GetSurface();
				}
				else //track geometry
				{
					int shapeId = rayCallback.m_shapeId;
					//assert(shapeId >= 0 && shapeId < trackSurface.size());
					if (shapeId >= trackSurface.size() || shapeId < 0)  shapeId = 0;  //crash hf-
					//if (trackSurface.size() > 0)
						s = trackSurface[shapeId];
				}
			}
		}
		
		// track bezierpatch collision
		if (track != NULL)
		{
			MATHVECTOR <float, 3> bezierspace_raystart(origin[1], origin[2], origin[0]);
			MATHVECTOR <float, 3> bezierspace_dir(direction[1], direction[2], direction[0]);
			MATHVECTOR <float, 3> colpoint, colnormal;
			const BEZIER * colpatch = NULL;
			bool bezierHit = track->CastRay(bezierspace_raystart, bezierspace_dir, length, colpoint, colpatch, colnormal);
			if (bezierHit)
			{
				p = MATHVECTOR <float, 3> (colpoint[2], colpoint[0], colpoint[1]);
				n = MATHVECTOR <float, 3> (colnormal[2], colnormal[0], colnormal[1]);
				d = (colpoint - bezierspace_raystart).Magnitude();
				s = track->GetRoadSurface();
				b = colpatch;
				c = NULL;
				*pOnRoad = 1;
			}
		}

		contact.Set(p, n, d, s, b, c);
		return true;
	}
	
	// should only happen on vehicle rollover
	contact.Set(origin + direction * length, -direction, length, s, b, c);
	return false;
}


void COLLISION_WORLD::DebugPrint(std::ostream & out)
{
	out << "Collision objects: " << world->getNumCollisionObjects() << std::endl;
}

//  Clear - delete bullet pointers
void COLLISION_WORLD::Clear()
{
	track = NULL;
	if(trackObject)
	{
		delete trackObject->getCollisionShape();
		trackObject = NULL;
	}
	if(trackMesh)
	{
		delete trackMesh;
		trackMesh = NULL;
	}
	trackSurface.resize(0);

	// remove constraint before deleting rigid body
	for(int i = 0; i < constraints.size(); i++)
	{
		world->removeConstraint(constraints[i]);
		delete constraints[i];
	}
	constraints.resize(0);
	
	for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		world->removeCollisionObject(obj);

		ShapeData* sd = (ShapeData*)obj->getUserPointer();
		delete sd;
		delete obj;
	}
	
	for(int i = 0; i < shapes.size(); i++)
	{
		btCollisionShape * shape = shapes[i];
		if (shape->isCompound())
		{
			btCompoundShape * cs = (btCompoundShape *)shape;
			for (int i = 0; i < cs->getNumChildShapes(); i++)
			{
				delete cs->getChildShape(i);
			}
		}
		delete shape;
	}
	shapes.resize(0);
	
	for(int i = 0; i < meshes.size(); i++)
	{
		delete meshes[i];
	}
	meshes.resize(0);
	
	for(int i = 0; i < actions.size(); i++)
	{
		world->removeAction(actions[i]);
	}
	actions.resize(0);
}