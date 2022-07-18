#include "pch.h"
#include "par.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/data/SceneXml.h"
#include "../ogre/common/ShapeData.h"
#include "../ogre/common/CScene.h"
#include "collision_world.h"
#include "tobullet.h"
#include "collision_contact.h"
#include "cardynamics.h"
//#include "car.h"//
#include "game.h"  //
#include "../ogre/CGame.h"  //
#include <iostream>
using namespace Ogre;


///  hit callback (accurate)
//-------------------------------------------------------------------------------------------------------------------------------
void IntTickCallback(btDynamicsWorld* world, btScalar timeStep)
{
	COLLISION_WORLD* cw = (COLLISION_WORLD*)world->getWorldUserInfo();

	int numManifolds = world->getDispatcher()->getNumManifolds();
	for (int i=0; i < numManifolds; ++i)  // pairs
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* bA = contactManifold->getBody0();
		const btCollisionObject* bB = contactManifold->getBody1();
	
		if (bA->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE ||
			bB->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE)  // ignore triggers
			continue;

		void* pA = bA->getUserPointer(), *pB = bB->getUserPointer();
		//if (pA && pB)
		{
			ShapeData* sdA = (ShapeData*)pA, *sdB = (ShapeData*)pB, *sdCar=0, *sdFluid=0, *sdWheel=0;
			if (sdA) {  if (sdA->type == ST_Car)  sdCar = sdA;  else if (sdA->type == ST_Fluid)  sdFluid = sdA;  else if (sdA->type == ST_Wheel)  sdWheel = sdA;  }
			if (sdB) {  if (sdB->type == ST_Car)  sdCar = sdB;  else if (sdB->type == ST_Fluid)  sdFluid = sdB;  else if (sdB->type == ST_Wheel)  sdWheel = sdB;  }
	
			if (sdFluid && sdFluid->pFluid)  // solid fluid hit
				if (sdFluid->pFluid->solid)  sdFluid = 0;
				
			if (sdCar &&/**/ !sdFluid && !sdWheel)
			{
				bool dyn = (sdCar == sdA && !bB->isStaticObject()) || (sdCar == sdB && !bA->isStaticObject());
				int num = contactManifold->getNumContacts();
				for (int j=0; j < num; ++j)
				{
					btManifoldPoint& pt = contactManifold->getContactPoint(j);
					btScalar f = pt.getAppliedImpulse() * timeStep;
					if (f > 0.f)
					{
						//LogO(toStr(i)+" "+toStr(j)+" "+fToStr(f,2,4));
						DynamicsWorld::Hit hit;
						hit.dyn = dyn ? 1 : 0;  /// todo: custom sound for obj type..
						hit.pos = pt.getPositionWorldOnA();  hit.norm = pt.m_normalWorldOnB;
						hit.force = f;  hit.sdCar = sdCar;
						//hit.force = std::max(0, 60 - pt.getLifeTime());
						//LogO(fToStr(hit.force,2,4));
						hit.vel = sdCar ? sdCar->pCarDyn->velPrev : (btVector3(1,1,1)*0.1f);
						cw->world->vHits.push_back(hit);
						//sdCar->pCarDyn->hitPnts.push_back(pt);  ///i
				}	}
		}	}
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
		const btCollisionObject* bA = contactManifold->getBody0();
		const btCollisionObject* bB = contactManifold->getBody1();
	
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
		Vector3 vel(vcar[0], vcar[2], -vcar[1]);
		Vector3 norm(hit.norm.getX(), hit.norm.getZ(), -hit.norm.getY());
		float vlen = vel.length(), normvel = abs(vel.dotProduct(norm));

		if (cd->vtype == V_Sphere)  // no damage when rolling
		{
			//LogO("vlen "+fToStr(vlen,2,4)+"  nv "+fToStr(normvel,2,4)+"  f "+fToStr(force,2,4));
			if (force > 11.f)  // hit
			{	vlen = normvel*0.2f;  force = normvel*0.2f;  normvel *= 0.2f;  }
			else  // quiet
			{	vlen *= 0.02f;  normvel *= 0.02f;  force *= 0.1f;  }
		}
		//  Sparks emit params
		cd->vHitPos = Vector3(hit.pos.getX(), hit.pos.getZ(), -hit.pos.getY());
		cd->vHitNorm = norm + vel * 0.1f;
		cd->fParVel = 3.0f + 0.4f * vlen;
		cd->fParIntens = 10.f + 30.f * vlen;
		
		///----  damage normal
		MATHVECTOR<Dbl,3> pos(hit.pos.getX(), hit.pos.getY(), hit.pos.getZ()),
			cN = pos - cd->GetPosition(),
			f = cN * force * 1000000.f / cd->body_mass;
			f[0] *= gPar.camBncFHit;  f[1] *= gPar.camBncFHit;  f[2] *= gPar.camBncFHitY;
			cd->cam_force = cd->cam_force + f;  // cam bounce hit ++

		(-cd->GetOrientation()).RotateVector(cN);
		cd->vHitCarN = Vector3(cN[0],cN[1],cN[2]);  cd->vHitCarN.normalise();
		//----  factors
		if (cd->vtype != V_Sphere)
		{
			float sx = cd->vHitCarN.x, sy = cd->vHitCarN.y, sz = cd->vHitCarN.z;
			float nx = fabs(sx), ny = fabs(sy), nz = fabs(sz);
			cd->vHitDmgN.x = nx * (1.f-ny) * (1.f-nz) * (sx > 0 ? 1.0f : 0.3f);  // front x+
			cd->vHitDmgN.y = (1.f-nx) * ny * (1.f-nz) * 0.3f;  // side y
			cd->vHitDmgN.z = (1.f-nx) * (1.f-ny) * nz * (sz > 0 ? 1.0f : 0.1f);  // top z+
			cd->fHitDmgA = cd->vHitDmgN.x + cd->vHitDmgN.y + cd->vHitDmgN.z;
		}else
			cd->fHitDmgA = 0.5f;
		//--------

		//  hit force for sound
		cd->fHitForce = normvel*0.02f /*+ 0.02f*vlen*/;  //+
		cd->fHitForce2 = force*0.1f;

		//  dyn obj hit  ***
		if (hit.dyn)
		{	cd->fHitForce *= 0.3f;  //par
			cd->fHitDmgA *= 0.4f;
		}
		
		float a = std::min(1.f, std::min(vlen, 2.2f)*0.1f*powf(force, 0.4f) );
		if (a > cd->fCarScrap)  cd->fCarScrap = a;

		float b = std::min(1.f, 0.2f*force);
		if (b > cd->fCarScreech)  cd->fCarScreech = b;
			
		//if (force > 0.5f)
			cd->fHitTime = 1.f;
		///LogO("upd sf " + toStr(cd->fHitForce) + " force " + toStr(hit.force) + " vel " + toStr(vlen) + " Nvel " + toStr(normvel));
	}
	else if (cdOld)
	{
		cdOld->fHitForce  = 0.f;
		cdOld->fHitForce2 = 0.f;
		//cdOld->fHitForce3 = cdOld->fHitTime;
		//cdOld = 0;
	}

	world->vHits.clear();//+
}
//-------------------------------------------------------------------------------------------------------------------------------


///  ctor bullet world
COLLISION_WORLD::COLLISION_WORLD() : pApp(0),
	config(0), dispatcher(0), broadphase(0), solver(0), world(0), cdOld(0), 
	fixedTimestep(1.0/60.0), maxSubsteps(7)  // default, set from settings
{
	config = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(config);

	btScalar ws = 5000;  // world size
	broadphase = new bt32BitAxisSweep3(btVector3(-ws,-ws,-ws), btVector3(ws,ws,ws));
	solver = new btSequentialImpulseConstraintSolver();
	world = new DynamicsWorld(dispatcher, broadphase, solver, config);

	world->setGravity(btVector3(0.0, 0.0, -9.81)); ///~
	//world->getSolverInfo().m_numIterations = 36;  //-
	//world->getSolverInfo().m_splitImpulse = true;
	//world->getSolverInfo().m_splitImpulsePenetrationThreshold = -0.12f;
	//world->getSolverInfo().m_linearSlop = btScalar(0.05);
	//world->getSolverInfo().m_warmstartingFactor = btScalar(0.85);
	//world->getSolverInfo().m_restingContactRestitutionThreshold = 3;
	//world->getSolverInfo().m_erp = 0.3;
	//world->getSolverInfo().m_erp2 = 0.2;
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

btRigidBody* COLLISION_WORLD::AddRigidBody(const btRigidBody::btRigidBodyConstructionInfo & info,
	bool car, bool bCarsCollis)
{
	btRigidBody* body = new btRigidBody(info);
	btCollisionShape* shape = body->getCollisionShape();
	//body->setActivationState(DISABLE_DEACTIVATION);  //!-for chassis only
	//body->setCollisionFlags
	#define  COL_CAR  (1<<2)
	if (car)  world->addRigidBody(body, COL_CAR, 255 - (!bCarsCollis ? COL_CAR : 0));  // group, mask
	else	  world->addRigidBody(body);
	shapes.push_back(shape);
	return body;
}


struct MyRayResultCallback : public btCollisionWorld::RayResultCallback
{
	MyRayResultCallback(const btVector3 & rayFromWorld, const btVector3 & rayToWorld,
			const btCollisionObject* exclude, bool ignoreCars, bool camTilt, bool camDist)//, bool ignoreGlass)
		: m_rayFromWorld(rayFromWorld), m_rayToWorld(rayToWorld), m_exclude(exclude), m_shapeId(0)
		, bIgnoreCars(ignoreCars), bCamTilt(camTilt), bCamDist(camDist)
	{	}

	btVector3	m_rayFromWorld;//used to calculate hitPointWorld from hitFraction
	btVector3	m_rayToWorld;

	btVector3	m_hitNormalWorld;
	btVector3	m_hitPointWorld;
	
	int m_shapeId;
	const btCollisionObject* m_exclude;
	bool bIgnoreCars,bCamTilt,bCamDist;
		
	virtual	btScalar	addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		const btCollisionObject* obj = rayResult.m_collisionObject;
		if (obj == m_exclude)
			return 1.0;
					
		//  no other cars collision (for wheel raycasts)
		ShapeData* sd = (ShapeData*)obj->getUserPointer();
		if (sd)
		{
			if (bIgnoreCars && sd->type == ST_Car)
				return 1.0;
			
			//  car ignores fluids (camera not)
			//  hovercrafts treat fluids as solids
			if (!bCamTilt && sd->type == ST_Fluid && !sd->pFluid->solid)
				return 1.0;

			//  always ignore wheel triggers
			if (sd->type == ST_Wheel)  // && (obj->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE))
				return 1.0;
		}

		//  cam ingores dynamic objects (car not)
		if (bCamTilt && !obj->isStaticObject())
			return 1.0;

		//  cam dist ray only for terrain, // todo: would work for rest but loops and pipe glass ..
		if (bCamDist)
		{
			int su = (long)obj->getCollisionShape()->getUserPointer();
			if (su != SU_Terrain)
				return 1.0;
		}
				
		//caller already does the filter on the m_closestHitFraction
		btAssert(rayResult.m_hitFraction <= m_closestHitFraction);
		
		m_closestHitFraction = rayResult.m_hitFraction;
		m_collisionObject = obj;

		if (!rayResult.m_localShapeInfo)
			m_shapeId = 0;  //crash hf-
		else  // only for btTriangleMeshShape
			m_shapeId = rayResult.m_localShapeInfo->m_shapePart;

		if (normalInWorldSpace)
			m_hitNormalWorld = rayResult.m_hitNormalLocal;
		else  ///need to transform normal into worldspace
			m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;

		m_hitPointWorld.setInterpolate3(m_rayFromWorld,m_rayToWorld,rayResult.m_hitFraction);
		return rayResult.m_hitFraction;
	}
};


//--------------------------------------------------------------------------------------------------------------------------------
//  Ray
///-------------------------------------------------------------------------------------------------------------------------------
bool COLLISION_WORLD::CastRay(
	const MATHVECTOR<float,3> & origin,
	const MATHVECTOR<float,3> & direction, const float length,
	const btCollisionObject* caster,
	COLLISION_CONTACT& contact,  //out
	CARDYNAMICS* cd, int w, //out pCarDyn, nWheel
	bool ignoreCars, bool camTilt/*or treat fluids as solid*/, bool camDist) const
{
	btVector3 from = ToBulletVector(origin);
	btVector3 to = ToBulletVector(origin + direction* length);
	MyRayResultCallback res(from, to, caster, ignoreCars, camTilt, camDist);
	
	//  data to set
	MATHVECTOR<float,3> pos, norm;  float dist;
	const TRACKSURFACE* surf = TRACKSURFACE::None();
	const btCollisionObject* col = NULL;
	
	world->rayTest(from, to, res);

	bool geometryHit = res.hasHit();
	if (geometryHit)
	{
		pos = ToMathVector<float>(res.m_hitPointWorld);
		norm = ToMathVector<float>(res.m_hitNormalWorld);
		dist = res.m_closestHitFraction * length;
		col = res.m_collisionObject;
		const TerData& td = pApp->scn->sc->td;

		if (col->isStaticObject() /*&& (c->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE == 0)*/)
		{
			int ptrU = (long)col->getCollisionShape()->getUserPointer(),
				su = ptrU & 0xFF00, mtr = ptrU & 0xFF;  //void*

			///  set surface, basing on shape type  -----------------

			if (ptrU)
			switch (su)
			{
				case SU_Road:  // road
				{					 
					//LogO("RD "+toStr(mtr));
					int id = td.layerRoad[td.road1mtr ? 0 : mtr].surfId;
					surf = &pApp->pGame->surfaces[id];

					if (cd)
					{	cd->iWhOnRoad[w] = 1;   cd->whRoadMtr[w] = mtr;  cd->whTerMtr[w] = 0;  }
				}	break;

				case SU_Pipe:  // pipe
				{
					int id = td.layerRoad[0].surfId;
					surf = &pApp->pGame->surfaces[id];

					if (cd)
					{	cd->iWhOnRoad[w] = 2;   cd->whRoadMtr[w] = mtr+30;  cd->whTerMtr[w] = 0;  }
				}	break;

				case SU_Terrain:  // Terrain  get surface from blendmap mtr
				{
					int t = pApp->blendMapSize;
					float tws = td.fTerWorldSize;

					int mx = (pos[0] + 0.5*tws)/tws*t;  mx = std::max(0,std::min(t-1, mx));
					int my = (pos[1] + 0.5*tws)/tws*t;  my = std::max(0,std::min(t-1, t-1-my));

					int mtr = pApp->blendMtr[my*t + mx];
                    assert(mtr < td.layers.size());
					int id = td.layersAll[td.layers[mtr]].surfId;
					surf = &pApp->pGame->surfaces[id];

					if (cd)                                              // mtr 0 = not on terrain
					{	cd->iWhOnRoad[w] = 0;   cd->whRoadMtr[w] = 60;  cd->whTerMtr[w] = mtr + 1;  }
				}	break;
				
				//case SU_RoadWall: //case SU_RoadColumn:
				//case SU_Vegetation: case SU_Border:
				
				//case SU_ObjectStatic: //case SU_ObjectDynamic:

				case SU_Fluid:  //  solid fluids
				{
					int id = mtr;
					surf = &pApp->pGame->surfaces[id];

					if (cd)
					{	cd->iWhOnRoad[w] = 0;   cd->whRoadMtr[w] = 0;  cd->whTerMtr[w] = 1;  }
				}	break;
				
				default:
				{
					int id = td.layerRoad[0].surfId;
					surf = &pApp->pGame->surfaces[id];

					if (cd)
					{	cd->iWhOnRoad[w] = 0;   cd->whRoadMtr[w] = 80;  cd->whTerMtr[w] = 0;  }
				}	break;
			}
			else  //if (ptrU == 0)
			{
				int id = td.layersAll[0].surfId;  //0 only 1st
				surf = &pApp->pGame->surfaces[id];

				if (cd)
				{	cd->iWhOnRoad[w] = 0;   cd->whRoadMtr[w] = 0;  cd->whTerMtr[w] = 1;  }

			}
		}

		contact.Set(pos, norm, dist, surf, col);
		return true;
	}
	
	//  should only happen on vehicle rollover
	contact.Set(origin + direction * length, -direction, length, surf, col);
	return false;
}
///-------------------------------------------------------------------------------------------------------------------------------


void COLLISION_WORLD::DebugPrint(std::ostream & out)
{
	out << "Collision objects: " << world->getNumCollisionObjects() << std::endl;
}

//  Clear - delete bullet pointers
//-------------------------------------------------------------	
void COLLISION_WORLD::Clear()
{
	cdOld = NULL;
	//trackSurface.resize(0);

	// remove constraint before deleting rigid body
	int i,c;
	for (i = 0; i < constraints.size(); ++i)
	{
		world->removeConstraint(constraints[i]);
		delete constraints[i];
	}
	constraints.resize(0);
	
	for (i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
			delete body->getMotionState();

		world->removeCollisionObject(obj);

		ShapeData* sd = (ShapeData*)obj->getUserPointer();
		delete sd;
		delete obj;
	}
	
	for (i = 0; i < shapes.size(); ++i)
	{
		btCollisionShape* shape = shapes[i];
		if (shape->isCompound())
		{
			btCompoundShape* cs = (btCompoundShape*)shape;
			for (c = 0; c < cs->getNumChildShapes(); ++c)
				delete cs->getChildShape(c);
		}
		delete shape;
	}
	shapes.resize(0);
	
	for (i = 0; i < meshes.size(); ++i)
		delete meshes[i];

	meshes.resize(0);
	
	for (i = 0; i < actions.size(); ++i)
		world->removeAction(actions[i]);

	actions.resize(0);
}
