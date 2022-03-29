#pragma once
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
//#include "btAlignedObjectArray.h"

template <class T, unsigned int dim> class MATHVECTOR;
class COLLISION_CONTACT;
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
		class ShapeData* sdCar;  int dyn;
	};
	btAlignedObjectArray<Hit> vHits;
};


// manages bodies / collision objects / collision shapes
class COLLISION_WORLD
{
public:
	COLLISION_WORLD();
	~COLLISION_WORLD();

	class App* pApp;  //for blend mtr
	
	btRigidBody* AddRigidBody(const btRigidBody::btRigidBodyConstructionInfo & info,
		bool car = false, bool bCarsCollis = false);

	// cast ray into collision world, returns first hit, caster is excluded fom hits
	bool CastRay(
		const MATHVECTOR<float,3> & position, const MATHVECTOR<float,3> & direction, const float length,
		const btCollisionObject* caster, COLLISION_CONTACT & contact,
		class CARDYNAMICS* pCarDyn, int nWheel, //
		bool ignoreCars, bool camTilt/*or treat fluids as solid*/, bool camDist=false) const;
	
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
	btAlignedObjectArray<btCollisionShape*> shapes;
	btAlignedObjectArray<btActionInterface*> actions;
	btAlignedObjectArray<btTypedConstraint*> constraints;
	btAlignedObjectArray<btTriangleIndexVertexArray*> meshes;
};
