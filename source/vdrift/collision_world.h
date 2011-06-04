#ifndef _COLLISION_WORLD_H
#define _COLLISION_WORLD_H

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

template <class T, unsigned int dim> class MATHVECTOR;
class COLLISION_CONTACT;
class MODEL;
class TRACK;
class TRACKSURFACE;


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
		const MATHVECTOR <float, 3> & position,
		const MATHVECTOR <float, 3> & direction,
		const float length,
		const btCollisionObject * caster,
		COLLISION_CONTACT & contact,
		int* pOnRoad) const;
	
	// update world physics
	void Update(float dt, bool profiling);
	std::string bltProfiling;  // blt debug times info
	
	void Draw();
	void DebugPrint(std::ostream & out);
	void DebugDrawScene();
	
	void Clear();
	
//private:
public:
	float fixedTimestep;  int maxSubsteps;
	
	btDefaultCollisionConfiguration collisionconfig;
	btCollisionDispatcher collisiondispatcher;
	bt32BitAxisSweep3 collisionbroadphase;
	btSequentialImpulseConstraintSolver constraintsolver;
	btDiscreteDynamicsWorld world;
	
	//GL_ShapeDrawer*	m_shapeDrawer;
	//GLDebugDrawer* m_DebugDrawer;

	btAlignedObjectArray<btCollisionShape *> shapes;
	btAlignedObjectArray<btActionInterface *> actions;
	btAlignedObjectArray<btTypedConstraint *> constraints;
	btAlignedObjectArray<btTriangleIndexVertexArray *> meshes;
	
	//todo: cleanup here
	TRACK * track;
	btCollisionObject * trackObject;
	btTriangleIndexVertexArray * trackMesh;
	btAlignedObjectArray<const TRACKSURFACE *> trackSurface;
	
	btCollisionShape * AddMeshShape(const MODEL & model);
	btIndexedMesh GetIndexedMesh(const MODEL & model);
};

#endif // _COLLISION_WORLD_H
