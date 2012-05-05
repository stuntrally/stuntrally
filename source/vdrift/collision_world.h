#ifndef _COLLISION_WORLD_H
#define _COLLISION_WORLD_H

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
//#include "btAlignedObjectArray.h"

template <class T, unsigned int dim> class MATHVECTOR;
class COLLISION_CONTACT;
class MODEL;
class TRACK;
class TRACKSURFACE;


class DynamicsWorld : public btDiscreteDynamicsWorld
{
public:
	DynamicsWorld(
		btDispatcher* dispatcher,
		btBroadphaseInterface* broadphase,
		btConstraintSolver* constraintSolver,
		btCollisionConfiguration* collisionConfig)
	:	btDiscreteDynamicsWorld(dispatcher, broadphase, constraintSolver, collisionConfig)
	{  }

	~DynamicsWorld() {  }

	void solveConstraints(btContactSolverInfo& solverInfo);  // virtual

	struct Hit
	{
		btVector3 pos, norm, vel;  btScalar force;
		class ShapeData* sdCar;
	};
	btAlignedObjectArray<Hit> vHits;
};


// manages bodies / collision objects / collision shapes
class COLLISION_WORLD
{
public:
	COLLISION_WORLD();
	~COLLISION_WORLD();
	
	btCollisionObject * AddCollisionObject(const MODEL & model);
	
	btRigidBody * AddRigidBody(const btRigidBody::btRigidBodyConstructionInfo & info,
		bool car = false, bool bCarsCollis = false);

	void AddAction(btActionInterface * action);

	void AddConstraint(btTypedConstraint * constraint, bool disableCollisionsBetweenLinked=false);
	
	// add track to collision world (unloads previous track)
	void SetTrack(TRACK * t);
	
	// cast ray into collision world, returns first hit, caster is excluded fom hits
	bool CastRay(
		const MATHVECTOR <float,3> & position, const MATHVECTOR <float,3> & direction, const float length,
		const btCollisionObject * caster, COLLISION_CONTACT & contact,
		int* pOnRoad, bool ignoreCars, bool ignoreFluids) const;
	
	// update world physics
	void Update(double dt, bool profiling);
	class CARDYNAMICS* cdOld;  // for hit force setting back to 0
	std::string bltProfiling;  // blt debug times info
	
	void Draw();
	void DebugPrint(std::ostream & out);
	void DebugDrawScene();
	
	void Clear();
	
//private:
public:
	double fixedTimestep;  int maxSubsteps;
	
	// . . . bullet sim . . .
	btDefaultCollisionConfiguration* config;
	btCollisionDispatcher* dispatcher;
	bt32BitAxisSweep3* broadphase;
	btSequentialImpulseConstraintSolver* solver;
	DynamicsWorld* world;

	// . . . . . .
	btAlignedObjectArray<btCollisionShape *> shapes;
	btAlignedObjectArray<btActionInterface *> actions;
	btAlignedObjectArray<btTypedConstraint *> constraints;
	btAlignedObjectArray<btTriangleIndexVertexArray *> meshes;
	
	TRACK * track;
	btCollisionObject * trackObject;
	btTriangleIndexVertexArray * trackMesh;
	btAlignedObjectArray<const TRACKSURFACE *> trackSurface;
	
	btCollisionShape * AddMeshShape(const MODEL & model);
	btIndexedMesh GetIndexedMesh(const MODEL & model);
};

#endif // _COLLISION_WORLD_H
