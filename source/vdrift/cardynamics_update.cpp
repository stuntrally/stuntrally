#include "pch.h"
#include "cardynamics.h"
#include "collision_world.h"
#include "settings.h"
#include "tobullet.h"
#include "../ogre/OgreGame.h"
#include "Buoyancy.h"
#include "../ogre/common/Defines.h"
#include "../ogre/common/SceneXml.h"

typedef CARDYNAMICS::T T;


// executed as last function(after integration) in bullet singlestepsimulation
void CARDYNAMICS::updateAction(btCollisionWorld * collisionWorld, btScalar dt)
{
	SynchronizeBody();  // get velocity, position orientation after dt

	UpdateWheelContacts();  // update wheel contacts given new velocity, position

	Tick(dt);  // run internal simulation

	SynchronizeChassis();  // update velocity
}

void CARDYNAMICS::Update()
{
	if (!chassis)  return;//
	btTransform tr;
	chassis->getMotionState()->getWorldTransform(tr);
	chassisRotation = ToMathQuaternion<T>(tr.getRotation());
	chassisCenterOfMass = ToMathVector<T>(tr.getOrigin());
	MATHVECTOR <T, 3> com = center_of_mass;
	chassisRotation.RotateVector(com);
	chassisPosition = chassisCenterOfMass - com;
	
	UpdateBuoyancy();
}

///................................................ Buoyancy ................................................
void CARDYNAMICS::UpdateBuoyancy()
{
	if (!pScene || (pScene->fluids.size() == 0) || !poly || !pFluids)  return;

	//float bc = /*sinf(chassisPosition[0]*20.3f)*cosf(chassisPosition[1]*30.4f) +*/
	//	sinf(chassisPosition[0]*0.3f)*cosf(chassisPosition[1]*0.32f);
	//LogO("pos " + toStr((float)chassisPosition[0]) + " " + toStr((float)chassisPosition[1]) + "  b " + toStr(bc));

	for (std::list<FluidBox*>::const_iterator i = inFluids.begin();
		i != inFluids.end(); ++i)  // 0 or 1 is there
	{
		const FluidBox* fb = *i;
		if (fb->id >= 0)
		{
			const FluidParams& fp = pFluids->fls[fb->id];

			WaterVolume water;
			//float bump = 1.f + 0.7f * sinf(chassisPosition[0]*fp.bumpFqX)*cosf(chassisPosition[1]*fp.bumpFqY);
			water.density = fp.density /* (1.f + 0.7f * bc)*/;  water.angularDrag = fp.angularDrag;
			water.linearDrag = fp.linearDrag;  water.linearDrag2 = 0.f;  //1.4f;//fp.linearDrag2;
			water.velocity.SetZero();
			water.plane.offset = fb->pos.y;  water.plane.normal = Vec3(0,0,1);
			//todo: fluid boxes rotation yaw, pitch ?-

			RigidBody body;  body.mass = body_mass;
			body.inertia = Vec3(body_inertia.getX(),body_inertia.getY(),body_inertia.getZ());

			///  body initial conditions
			//  pos & rot
			body.x.x = chassisPosition[0];  body.x.y = chassisPosition[1];  body.x.z = chassisPosition[2];
			body.q.x = chassisRotation[0];  body.q.y = chassisRotation[1];  body.q.z = chassisRotation[2];  body.q.w = chassisRotation[3];
			body.q.Normalize();//
			//  vel, ang vel
			btVector3 v = chassis->getLinearVelocity();
			btVector3 a = chassis->getAngularVelocity();
			body.v.x = v.getX();  body.v.y = v.getY();  body.v.z = v.getZ();
			body.omega.x = a.getX();  body.omega.y = a.getY();  body.omega.z = a.getZ();
			body.F.SetZero();  body.T.SetZero();
			
			//  damp from height vel
			body.F.z += fp.heightVelRes * -1000.f * body.v.z;
			
			///  add buoyancy force
			if (ComputeBuoyancy(body, *poly, water, 9.8f))
			{
				chassis->applyCentralForce( btVector3(body.F.x,body.F.y,body.F.z) );
				chassis->applyTorque(       btVector3(body.T.x,body.T.y,body.T.z) );
			}	
		}
	}

	///  wheel spin force (for mud)
	//_______________________________________________________
	for (int w=0; w < 4; ++w)
	{
		if (inFluidsWh[w].size() > 0)  // 0 or 1 is there
		{
			MATHVECTOR <T, 3> up(0,0,1);
			Orientation().RotateVector(up);
			float upZ = std::max(0.f, (float)up[2]);
			
			const FluidBox* fb = *inFluidsWh[w].begin();
			if (fb->id >= 0)
			{
				const FluidParams& fp = pFluids->fls[fb->id];

				WHEEL_POSITION wp = WHEEL_POSITION(w);
				float whR = GetTire(wp).GetRadius() * 1.2f;  //bigger par
				MATHVECTOR <float, 3> wheelpos = GetWheelPosition(wp, 0);
				wheelpos[2] -= whR;
				whP[w] = fp.idParticles;
				
				//  height in fluid:  0 just touching surface, 1 fully in fluid
				//  wheel plane distance  water.plane.normal.z = 1  water.plane.offset = fl.pos.y;
				whH[w] = (wheelpos[2] - fb->pos.y) * -0.5f / whR;
				whH[w] = std::max(0.f, std::min(1.f, whH[w]));

				if (fp.bWhForce)
				{
					//bool inAir = GetWheelContact(wp).col == NULL;

					//  bump, adds some noise
					MATHVECTOR <T, 3> whPos = GetWheelPosition(wp) - chassisPosition;
					float bump = sinf(whPos[0]*fp.bumpFqX)*cosf(whPos[1]*fp.bumpFqY);
					
					float f = std::min(fp.whMaxAngVel, std::max(-fp.whMaxAngVel, (float)wheel[w].GetAngularVelocity() ));
					QUATERNION <T> steer;
					float angle = -wheel[wp].GetSteerAngle() * fp.whSteerMul  + bump * fp.bumpAng;
					steer.Rotate(angle * PI_d/180.f, 0, 0, 1);

					//  forwards, side, up
					MATHVECTOR <T, 3> force(whH[w] * fp.whForceLong * f, 0, /*^ 0*/100.f * whH[w] * fp.whForceUp * upZ);
					(Orientation()*steer).RotateVector(force);
					
					//  wheel spin resistance
					wheel[w].fluidRes = whH[w] * fp.whSpinDamp  * (1.f + bump * fp.bumpAmp);
					
					if (whH[w] > 0.01f /*&& inAir*/)
						chassis->applyForce( ToBulletVector(force), ToBulletVector(whPos) );
				}
			}
		}
		else
		{	whH[w] = 0.f;  wheel[w].fluidRes = 0.f;  whP[w] = -1;	}
	}

}

/// print debug info to the given ostream.  set p1, p2, etc if debug info part 1, and/or part 2, etc is desired
///..........................................................................................................
void CARDYNAMICS::DebugPrint ( std::ostream & out, bool p1, bool p2, bool p3, bool p4 )
{
	if (p1)
	{
		out.precision(5);
		out << std::endl;

	#if 0  //  bullet hit data-
		out << "hit S : " << fSndForce << std::endl;
		out << "hit P : " << fParIntens << std::endl;
		//out << "hit t : " << fHitTime << std::endl;
		out << "bHitS : " << (bHitSnd?1:0) << " id "<< sndHitN << std::endl;
		out << "N Vel : " << fNormVel << std::endl;
		out << "v Vel : " << GetSpeed() << std::endl;
	#endif

	#if 1	// body
		out << "---Body---" << std::endl;
		//out << "c of mass: " << center_of_mass << std::endl;
		out << "pos: " << chassisPosition << std::endl;
		//MATRIX3 <T> inertia = body.GetInertia();  //btVector3 chassisInertia(inertia[0], inertia[4], inertia[8]);
		//out << "inertia:  " << inertia[0] << "  " << inertia[4] << "  " << inertia[8] << "\n";
		out << "mass: " << body.GetMass() << std::endl << std::endl;
		
		//MATHVECTOR <T, 3> up(0,0,1);
		//Orientation().RotateVector(up);
		//out << "up: " << up << std::endl;
		
	#endif

	#if 1	// fluids
		out << "in fluids: " << inFluids.size() <<
				" wh: " << inFluidsWh[0].size() << inFluidsWh[1].size() << inFluidsWh[2].size() << inFluidsWh[3].size() << std::endl;
		out.precision(2);
		out << "wh fl H: " << whH[0] << " " << whH[1] << " " << whH[2] << " " << whH[3] << " " << std::endl;
		out.precision(4);
		out << std::endl;
	#endif

	#if 0
		engine.DebugPrint(out);  out << std::endl;
		//fuel_tank.DebugPrint(out);  out << std::endl;  //mass 8- for 3S,ES,FM
		clutch.DebugPrint(out);  out << std::endl;
		//transmission.DebugPrint(out);	out << std::endl;
	#endif

	#if 0
		if ( drive == RWD )  {
			out << "(rear)" << std::endl;		rear_differential.DebugPrint(out);	}
		else if ( drive == FWD )  {
			out << "(front)" << std::endl;		front_differential.DebugPrint(out);	}
		else if ( drive == AWD )  {
			out << "(center)" << std::endl;		center_differential.DebugPrint(out);
			out << "(front)" << std::endl;		front_differential.DebugPrint(out);
			out << "(rear)" << std::endl;		rear_differential.DebugPrint(out);	}
		out << std::endl;
	#endif
	}

	#if 0
	if (p2)
	{
		out << "(front left)" << std::endl;		suspension[FRONT_LEFT].DebugPrint(out);	out << std::endl;
		out << "(front right)" << std::endl;	suspension[FRONT_RIGHT].DebugPrint(out);	out << std::endl;
		out << "(rear left)" << std::endl;		suspension[REAR_LEFT].DebugPrint(out);	out << std::endl;
		out << "(rear right)" << std::endl;		suspension[REAR_RIGHT].DebugPrint(out);	out << std::endl;

		out << "(front left)" << std::endl;		brake[FRONT_LEFT].DebugPrint(out);	out << std::endl;
		out << "(front right)" << std::endl;	brake[FRONT_RIGHT].DebugPrint(out);	out << std::endl;
		out << "(rear left)" << std::endl;		brake[REAR_LEFT].DebugPrint(out);	out << std::endl;
		out << "(rear right)" << std::endl;		brake[REAR_RIGHT].DebugPrint(out);
	}
	#endif

	#if 0
	if (p3)
	{
		out << std::endl;
		out << "(front left)" << std::endl;		wheel[FRONT_LEFT].DebugPrint(out);	out << std::endl;
		out << "(front right)" << std::endl;	wheel[FRONT_RIGHT].DebugPrint(out);	out << std::endl;
		out << "(rear left)" << std::endl;		wheel[REAR_LEFT].DebugPrint(out);	out << std::endl;
		out << "(rear right)" << std::endl;		wheel[REAR_RIGHT].DebugPrint(out);	out << std::endl;

		out << "(front left)" << std::endl;		tire[FRONT_LEFT].DebugPrint(out);	out << std::endl;
		out << "(front right)" << std::endl;	tire[FRONT_RIGHT].DebugPrint(out);	out << std::endl;
		out << "(rear left)" << std::endl;		tire[REAR_LEFT].DebugPrint(out);		out << std::endl;
		out << "(rear right)" << std::endl;		tire[REAR_RIGHT].DebugPrint(out);
	}
	#endif

	#if 0
	if (p4)
	{
		for ( std::vector <CARAERO<T> >::iterator i = aerodynamics.begin(); i != aerodynamics.end(); ++i )
		{
			i->DebugPrint(out);	out << std::endl;
		}
	}
	#endif
}
///..........................................................................................................


void CARDYNAMICS::UpdateBody(T dt, T drive_torque[])
{
	body.Integrate1(dt);
	//chassis->clearForces();

	UpdateWheelVelocity();

	ApplyEngineTorqueToBody();

	ApplyAerodynamicsToBody(dt);
	

	///***  manual car flip over  ---------------------------------------
	if ((doFlip > 0.01f || doFlip < -0.01f) &&
		pSet->game.flip_type > 0)
	{
		MATRIX3 <T> inertia = body.GetInertia();
		btVector3 inrt(inertia[0], inertia[4], inertia[8]);
		float t = inrt[inrt.maxAxis()] * doFlip * 12.f;  // strength

		if (pSet->game.flip_type == 1)  // fuel dec
		{
			boostFuel -= doFlip > 0.f ? doFlip * dt : -doFlip * dt;
			if (boostFuel < 0.f)  boostFuel = 0.f;
			if (boostFuel <= 0.f)  t = 0.0;
		}
		MATHVECTOR <T, 3> v(t,0,0);
		Orientation().RotateVector(v);
		ApplyTorque(v);
	}

	///***  boost  ------------------------------------------------------
	if (doBoost > 0.01f	&& pSet->game.boost_type > 0)
	{
		boostVal = doBoost;
		if (pSet->game.boost_type == 1 || pSet->game.boost_type == 2)  // fuel dec
		{
			boostFuel -= doBoost * dt;
			if (boostFuel < 0.f)  boostFuel = 0.f;
			if (boostFuel <= 0.f)  boostVal = 0.f;
		}
		if (boostVal > 0.01f)
		{
			float f = body.GetMass() * boostVal * 16.f * pSet->game.boost_power;  // power
			MATHVECTOR <T, 3> v(f,0,0);
			Orientation().RotateVector(v);
			ApplyForce(v);
		}
	}else
		boostVal = 0.f;
		
	//  add fuel over time
	if (pSet->game.boost_type == 2)
	{
		boostFuel += dt * gfBoostFuelAddSec;
		if (boostFuel > gfBoostFuelMax)  boostFuel = gfBoostFuelMax;
	}
	//LogO(toStr(boostFuel));
	///***  -------------------------------------------------------------
	

	T normal_force[WHEEL_POSITION_SIZE];
	for(int i = 0; i < WHEEL_POSITION_SIZE; i++)
	{
		MATHVECTOR <T, 3> suspension_force = UpdateSuspension(i, dt);
		normal_force[i] = suspension_force.dot(wheel_contact[i].GetNormal());
		if (normal_force[i] < 0) normal_force[i] = 0;

		MATHVECTOR <T, 3> tire_friction = ApplyTireForce(i, normal_force[i], wheel_orientation[i]);
		ApplyWheelTorque(dt, drive_torque[i], i, tire_friction, wheel_orientation[i]);
	}

	body.Integrate2(dt);
	//chassis->integrateVelocities(dt);

	// update wheel state
	for(int i = 0; i < WHEEL_POSITION_SIZE; i++)
	{
		wheel_position[i] = GetWheelPositionAtDisplacement(WHEEL_POSITION(i), suspension[i].GetDisplacementPercent());
		wheel_orientation[i] = Orientation() * GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION(i));
	}
	InterpolateWheelContacts(dt);

	for(int i = 0; i < WHEEL_POSITION_SIZE; i++)
	{
		if (abs)  DoABS(i, normal_force[i]);
		if (tcs)  DoTCS(i, normal_force[i]);
	}
}

//  Tick
//---------------------------------------------------------------------------------
void CARDYNAMICS::Tick(T dt)
{
	// has to happen before UpdateDriveline, overrides clutch, throttle
	UpdateTransmission(dt);

	const int num_repeats = pSet->dyn_iter;  ///~ 30+  o:10
	const float internal_dt = dt / num_repeats;
	for(int i = 0; i < num_repeats; ++i)
	{
		T drive_torque[WHEEL_POSITION_SIZE];

		UpdateDriveline(internal_dt, drive_torque);

		UpdateBody(internal_dt, drive_torque);

		feedback += 0.5 * (tire[FRONT_LEFT].GetFeedback() + tire[FRONT_RIGHT].GetFeedback());
	}

	feedback /= (num_repeats + 1);

	fuel_tank.Consume(engine.FuelRate() * dt);
	//engine.SetOutOfGas(fuel_tank.Empty());
	
	if (fHitTime > 0.f)
		fHitTime -= dt * 2.f;

	const float tacho_factor = 0.1;
	tacho_rpm = engine.GetRPM() * tacho_factor + tacho_rpm * (1.0 - tacho_factor);

	UpdateTelemetry(dt);
}

void CARDYNAMICS::SynchronizeBody()
{
	MATHVECTOR<T, 3> v = ToMathVector<T>(chassis->getLinearVelocity());
	MATHVECTOR<T, 3> w = ToMathVector<T>(chassis->getAngularVelocity());
	MATHVECTOR<T, 3> p = ToMathVector<T>(chassis->getCenterOfMassPosition());
	QUATERNION<T> q = ToMathQuaternion<T>(chassis->getOrientation());
	body.SetPosition(p);
	body.SetOrientation(q);
	body.SetVelocity(v);
	body.SetAngularVelocity(w);
}

void CARDYNAMICS::SynchronizeChassis()
{
	chassis->setLinearVelocity(ToBulletVector(body.GetVelocity()));
	chassis->setAngularVelocity(ToBulletVector(body.GetAngularVelocity()));
}

void CARDYNAMICS::UpdateWheelContacts()
{
	MATHVECTOR <float, 3> raydir = GetDownVector();
	for (int i = 0; i < WHEEL_POSITION_SIZE; i++)
	{
		COLLISION_CONTACT & wheelContact = wheel_contact[WHEEL_POSITION(i)];
		MATHVECTOR <float, 3> raystart = LocalToWorld(wheel[i].GetExtendedPosition());
		raystart = raystart - raydir * tire[i].GetRadius();  //*!
		float raylen = 1;  // !par
		
		//vRayStarts[i] = raystart;  // info
		//vRayDirs[i] = raystart + raydir * raylen;
		
		world->CastRay(raystart, raydir, raylen, chassis, wheelContact, /*R+*/&bWhOnRoad[i], !pSet->game.collis_cars, true);
		if (bTerrain)  ///  terrain surf from blendmap
			wheelContact.SetSurface(terSurf[i]);
	}
}
