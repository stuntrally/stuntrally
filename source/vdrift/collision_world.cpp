#include "stdafx.h"

#include "collision_world.h"

#include "tobullet.h"
#include "collision_contact.h"
#include "model.h"
#include "track.h"


///  ctor bullet world
///----------------------------------------------------------------------------
COLLISION_WORLD::COLLISION_WORLD() :
	collisiondispatcher(&collisionconfig),
	collisionbroadphase(btVector3(-5000, -5000, -5000), btVector3(5000, 5000, 5000)),
	world(&collisiondispatcher, &collisionbroadphase, &constraintsolver, &collisionconfig),
	track(NULL), trackObject(NULL), trackMesh(NULL),
	fixedTimestep(0.01f), maxSubsteps(7)  // default, set from settings
{
	world.setGravity(btVector3(0.0, 0.0, -9.81)); ///~
	//world.getSolverInfo().m_numIterations = 36;  //-
	//world.getSolverInfo().m_erp = 0.3;
	//world.getSolverInfo().m_erp2 = 0.2;
	world.getSolverInfo().m_restitution = 0.0f;
	//world.getDispatchInfo().m_enableSPU = true;
	world.setForceUpdateAllAabbs(false);  //+
}

COLLISION_WORLD::~COLLISION_WORLD()
{
	Clear();

	//delete m_DebugDrawer;
	//delete m_shapeDrawer;

	//delete world;
	//delete m_solver;
	//
	//delete collisionbroadphase;
	//delete collisiondispatcher;
	//delete collisionconfig;
}

void COLLISION_WORLD::DebugDrawScene()
{
}

btCollisionObject * COLLISION_WORLD::AddCollisionObject(const MODEL & model)
{
	btCollisionObject * col = new btCollisionObject();
	btCollisionShape * shape = AddMeshShape(model);
	col->setCollisionShape(shape);
	world.addCollisionObject(col);
	return col;
}

btRigidBody * COLLISION_WORLD::AddRigidBody(const btRigidBody::btRigidBodyConstructionInfo & info)
{
	btRigidBody * body = new btRigidBody(info);
	btCollisionShape * shape = body->getCollisionShape();
	//body->setActivationState(DISABLE_DEACTIVATION);  //!-for chassis only
	world.addRigidBody(body);
	shapes.push_back(shape);
	return body;
}

void COLLISION_WORLD::AddAction(btActionInterface * action)
{
	world.addAction(action);
	actions.push_back(action);
}

void COLLISION_WORLD::AddConstraint(btTypedConstraint * constraint, bool disableCollisionsBetweenLinked)
{
	world.addConstraint(constraint, disableCollisionsBetweenLinked);
	constraints.push_back(constraint);
}

void COLLISION_WORLD::SetTrack(TRACK * t)
{
	assert(t);
	
	// remove old track
	if(track)
	{
		world.removeCollisionObject(trackObject);
		
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
	for(std::list<TRACK_OBJECT>::const_iterator ob = objects.begin(); ob != objects.end(); ob++)
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
	
	world.addCollisionObject(trackObject);
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
	MyRayResultCallback(const btVector3 & rayFromWorld, const btVector3 & rayToWorld, const btCollisionObject * exclude)
	:m_rayFromWorld(rayFromWorld), m_rayToWorld(rayToWorld), m_exclude(exclude)
	{	}

	btVector3	m_rayFromWorld;//used to calculate hitPointWorld from hitFraction
	btVector3	m_rayToWorld;

	btVector3	m_hitNormalWorld;
	btVector3	m_hitPointWorld;
	
	int m_shapeId;
	const btCollisionObject * m_exclude;
		
	virtual	btScalar	addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		if (rayResult.m_collisionObject == m_exclude)
			return 1.0;
		
		//caller already does the filter on the m_closestHitFraction
		btAssert(rayResult.m_hitFraction <= m_closestHitFraction);
		
		m_closestHitFraction = rayResult.m_hitFraction;
		m_collisionObject = rayResult.m_collisionObject;

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

bool COLLISION_WORLD::CastRay(
	const MATHVECTOR <float, 3> & origin,
	const MATHVECTOR <float, 3> & direction,
	const float length,
	const btCollisionObject * caster,
	COLLISION_CONTACT & contact,
	int* pOnRoad) const
{
	btVector3 from = ToBulletVector(origin);
	btVector3 to = ToBulletVector(origin + direction * length);
	MyRayResultCallback rayCallback(from, to, caster);
	
	MATHVECTOR <float, 3> p;
	MATHVECTOR <float, 3> n;
	float d;
	const TRACKSURFACE * s = TRACKSURFACE::None();
	const BEZIER * b = NULL;
	btCollisionObject * c = NULL;
	
	// track geometry collision
	world.rayTest(from, to, rayCallback);
	bool geometryHit = rayCallback.hasHit();
	if (geometryHit)
	{
		p = ToMathVector<float>(rayCallback.m_hitPointWorld);
		n = ToMathVector<float>(rayCallback.m_hitNormalWorld);
		d = rayCallback.m_closestHitFraction * length;
		c = rayCallback.m_collisionObject;
		if (c->isStaticObject())
		{
			void * ptr = c->getCollisionShape()->getUserPointer();
			if (ptr == (void*)7777)
			{
				int a=0;
				s = trackSurface[0];
				*pOnRoad = 1;
			}
			else/**/
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
			MATHVECTOR <float, 3> colpoint;
			MATHVECTOR <float, 3> colnormal;
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

void COLLISION_WORLD::Update(float dt)
{
	//const int maxsubsteps = 7;  ///~  70  7*6+  o:7
	//const float fixedTimeStep = 1 / 60.0f;  ///~  320+  o:60.
	//world.stepSimulation(dt, maxsubsteps, fixedTimeStep);
	world.stepSimulation(dt, maxSubsteps, fixedTimestep);
	/*static int cc = 0;  cc++;  ///+ bullet profiling info
	if (cc > 60)
	{	cc = 0;
		CProfileManager::dumpAll();
	}/**/
}

void COLLISION_WORLD::DebugPrint(std::ostream & out)
{
	out << "Collision objects: " << world.getNumCollisionObjects() << std::endl;
}

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
		world.removeConstraint(constraints[i]);
	}
	constraints.resize(0);
	
	for(int i = world.getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world.getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		world.removeCollisionObject(obj);
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
		world.removeAction(actions[i]);
	}
	actions.resize(0);
}
