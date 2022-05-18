#include "pch.h"
#include "par.h"
#include "cardynamics.h"
#include "collision_world.h"
#include "settings.h"
#include "game.h"
#include "tobullet.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/data/CData.h"
#include "../ogre/common/data/SceneXml.h"
#include "../ogre/common/data/FluidsXml.h"
#include "../ogre/common/ShapeData.h"
#include "../ogre/CGame.h"
#include "Buoyancy.h"


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
	chassisRotation = ToMathQuaternion<Dbl>(tr.getRotation());
	chassisCenterOfMass = ToMathVector<Dbl>(tr.getOrigin());
	MATHVECTOR<Dbl,3> com = center_of_mass;
	chassisRotation.RotateVector(com);
	chassisPosition = chassisCenterOfMass - com;
	
	UpdateBuoyancy();
}

///................................................ Buoyancy ................................................
void CARDYNAMICS::UpdateBuoyancy()
{
	if (!pScene || (pScene->fluids.size() == 0) || !pFluids)  return;

	//float bc = /*sinf(chassisPosition[0]*20.3f)*cosf(chassisPosition[1]*30.4f) +*/
	//	sinf(chassisPosition[0]*0.3f)*cosf(chassisPosition[1]*0.32f);
	//LogO("pos " + toStr((float)chassisPosition[0]) + " " + toStr((float)chassisPosition[1]) + "  b " + toStr(bc));

	for (auto fb : inFluids)
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
		if (vtype == V_Sphere)
		{	body.q.x = 0.f;  body.q.y = 0.f;  body.q.z = 0.f;  body.q.w = 1.f;  // no rot
		}else
		{	body.q.x = chassisRotation[0];  body.q.y = chassisRotation[1];  body.q.z = chassisRotation[2];  body.q.w = chassisRotation[3];
			body.q.Normalize();//
		}
		//LogO(fToStr(body.q.x,2,4)+" "+fToStr(body.q.y,2,4)+" "+fToStr(body.q.z,2,4)+" "+fToStr(body.q.w,2,4));

		//  vel, ang vel
		btVector3 v = chassis->getLinearVelocity();
		btVector3 a = chassis->getAngularVelocity();
		body.v.x = v.getX();  body.v.y = v.getY();  body.v.z = v.getZ();
		if (vtype == V_Sphere)
		{	body.omega.SetZero();  // no ang vel
		}else
		{	body.omega.x = a.getX();  body.omega.y = a.getY();  body.omega.z = a.getZ();
		}
		body.F.SetZero();  body.T.SetZero();
		
		//  damp from height vel
		body.F.z += fp.heightVelRes * -1000.f * body.v.z;
		
		///  add buoyancy force
		if (ComputeBuoyancy(body, poly, water, 9.8f))
		{
			if (vtype != V_Car)
			{	body.F.x *= 0.15f;  body.F.y *= 0.15f;  }
			chassis->applyCentralForce( btVector3(body.F.x,body.F.y,body.F.z) );
			chassis->applyTorque(       btVector3(body.T.x,body.T.y,body.T.z) );
		}	
	}

	///  wheel spin force (for mud)
	//_______________________________________________________
	for (int w=0; w < numWheels; ++w)
	{
		if (inFluidsWh[w].size() > 0)  // 0 or 1 is there
		{
			MATHVECTOR<Dbl,3> up(0,0,1);
			Orientation().RotateVector(up);
			float upZ = std::max(0.f, (float)up[2]);
			
			const FluidBox* fb = *inFluidsWh[w].begin();
			if (fb->id >= 0)
			{
				const FluidParams& fp = pFluids->fls[fb->id];

				WHEEL_POSITION wp = WHEEL_POSITION(w);
				float whR = GetWheel(wp).GetRadius() * 1.2f;  //bigger par
				MATHVECTOR<float,3> wheelpos = GetWheelPosition(wp, 0);
				wheelpos[2] -= whR;
				whP[w] = fp.idParticles;
				whDmg[w] = fp.fDamage;
				
				//  height in fluid:  0 just touching surface, 1 fully in fluid
				//  wheel plane distance  water.plane.normal.z = 1  water.plane.offset = fl.pos.y;
				whH[w] = (wheelpos[2] - fb->pos.y) * -0.5f / whR;
				whH[w] = std::max(0.f, std::min(1.f, whH[w]));

				if (fp.bWhForce)
				{
					//bool inAir = GetWheelContact(wp).col == NULL;

					//  bump, adds some noise
					MATHVECTOR<Dbl,3> whPos = GetWheelPosition(wp) - chassisPosition;
					float bump = sinf(whPos[0]*fp.bumpFqX)*cosf(whPos[1]*fp.bumpFqY);
					
					float f = std::min(fp.whMaxAngVel, std::max(-fp.whMaxAngVel, (float)wheel[w].GetAngularVelocity() ));
					QUATERNION<Dbl> steer;
					float ba = numWheels==2 && w==0 ? 2.f : 1.f;  //bike
					float angle = -wheel[wp].GetSteerAngle() * fp.whSteerMul * ba  + bump * fp.bumpAng;
					steer.Rotate(angle * PI_d/180.f, 0, 0, 1);

					//  forwards, side, up
					MATHVECTOR<Dbl,3> force(whH[w] * fp.whForceLong * f, 0, /*^ 0*/100.f * whH[w] * fp.whForceUp * upZ);
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
void CARDYNAMICS::DebugPrint( std::ostream & out, bool p1, bool p2, bool p3, bool p4 )
{
	using namespace std;
	out.precision(2);  out.width(6);  out << fixed;
	int cnt = pSet->car_dbgtxtcnt, w;

	if (p1)
	{
		#if 0  //  bullet hit data-
			out << "hit P : " << fParIntens << endl;
			//out << "hit t : " << fHitTime << endl;
			out << "v Vel : " << GetSpeed() << endl;
		#endif
		#if 0
			out << "Damage : " << fToStr(fDamage,0,3) << "  "
				//<< vHitCarN.x << ", " << vHitCarN.y << ", " << vHitCarN.z
				//<< "  x " << fToStr(vHitDmgN.x ,2,4)
				//<< "  y " << fToStr(vHitDmgN.y ,2,4)
				//<< "  z " << fToStr(vHitDmgN.z ,2,4)
				<< "  a " << fToStr(fHitDmgA ,2,4)
				<< endl;
			return;
		#endif

		//  body
		{
			//out << "___ Body ___" << endl;  // L| front+back-  W_ left-right+  H/ up+down-
			out << "com: W right+ " << -center_of_mass[1] << " L front+ " << center_of_mass[0] << " H up+ " << center_of_mass[2] << endl;
			out.precision(0);
			out << "mass: " << body.GetMass();

			/*if (hover)			
			{	MATHVECTOR<Dbl,3> sv = -GetVelocity();
				(-Orientation()).RotateVector(sv);
				out << " vel: " << sv << endl << "  au: " << al << endl;
			}/**/
			//  wheel pos, com ratio
			Dbl whf = wheel[0].GetExtendedPosition()[0], whr = wheel[numWheels==2?1:2].GetExtendedPosition()[0];
			out.precision(2);
			out << "  wh fr " << whf << "  rr " << whr;
			out.precision(1);
			out << "  fr% " << (center_of_mass[0]+whf)/(whf-whr)*100 << endl;
			out.precision(0);
			MATRIX3 <Dbl> inertia = body.GetInertiaConst();
			
			out << "inertia: roll " << inertia[0] << " pitch " << inertia[4] << " yaw " << inertia[8] << endl;

			//out << "inertia: " << inertia[0] <<" "<< inertia[4] <<" "<< inertia[8] <<" < "<< inertia[1] <<" "<< inertia[2] <<" "<< inertia[3] <<" "<< inertia[5] <<" "<< inertia[6] <<" "<< inertia[7] << endl;
			//MATHVECTOR<Dbl,3> av = GetAngularVelocity();  Orientation().RotateVector(av);
			//out << "ang vel: " << fToStr(av[0],2,5) <<" "<< fToStr(av[1],2,5) <<" "<< fToStr(av[2],2,5) << endl;
			//out << "pos: " << chassisPosition << endl;
			out.precision(2);
			//MATHVECTOR<Dbl,3> up(0,0,1);  Orientation().RotateVector(up);
			//out << "up: " << up << endl;
			out << endl;
			//out << sHov.c_str() << endl;
		}

		//  fluids
		if (cnt > 1)
		{	out << "in fluids: " << inFluids.size() << " wh:";
			for (w=0; w < numWheels; ++w)  out << " " << inFluidsWh[w].size();
			out << endl;
			out << "wh fl H:";
			for (w=0; w < numWheels; ++w)  out << " " << fToStr(whH[w],1,3);
			out << " \n\n";
		}

		if (cnt > 3)
		{
			engine.DebugPrint(out);  out << endl;
			//fuel_tank.DebugPrint(out);  out << endl;  //mass 8- for ES,
			clutch.DebugPrint(out);  out << endl;
			transmission.DebugPrint(out);	out << endl;
		}

		if (cnt > 5)
		{
			out << "___ Differential ___\n";
			if (drive == RWD)  {
				out << " rear\n";		diff_rear.DebugPrint(out);	}
			else if (drive == FWD)  {
				out << " front\n";		diff_front.DebugPrint(out);	}
			else if (drive == AWD) {
				out << " center\n";		diff_center.DebugPrint(out);
				out << " front\n";		diff_front.DebugPrint(out);
				out << " rear\n";		diff_rear.DebugPrint(out);	}
			else if (drive == WD6) {
				out << " 1 front\n";	diff_front.DebugPrint(out);
				out << " 2 rear\n";		diff_rear.DebugPrint(out);
				out << " 3 rear2\n";	diff_rear2.DebugPrint(out);
				out << " 12 center\n";	diff_center.DebugPrint(out);
				out << " center2\n";	diff_center2.DebugPrint(out);  }
			else if (drive == WD8) {
				out << " 1 front\n";	diff_front.DebugPrint(out);
				out << " 2 rear\n";		diff_rear.DebugPrint(out);
				out << " 3 rear2\n";	diff_rear2.DebugPrint(out);
				out << " 4 rear3\n";	diff_rear3.DebugPrint(out);
				out << " 12 center\n";	diff_center.DebugPrint(out);
				out << " 34 center2\n";	diff_center2.DebugPrint(out);
				out << " center3\n";	diff_center3.DebugPrint(out);  }
			out << endl;
		}
	}

	const static char sWh[MAX_WHEELS][8] = {" FL [^", " FR ^]", " RL [_", " RR _]", " RL2[_", " RR2_]"};
	if (p2)
	{
		out << "\n\n\n\n";
		if (cnt > 4)
		{
			out << "___ Brake ___n";
			for (w=0; w < numWheels; ++w)
			{	out << sWh[w] << endl;	brake[w].DebugPrint(out);  }
		}
		if (cnt > 7)
		{
			out << "\n___ Suspension ___\n";
			for (w=0; w < numWheels; ++w)
			{	out << sWh[w] << endl;	suspension[w].DebugPrint(out);  }
		}
	}

	if (p3)
		if (cnt > 6)
		{
			out << "___Wheel___\n";
			for (w=0; w < numWheels; ++w)
			{	out << sWh[w] << endl;	wheel[w].DebugPrint(out);  }
		}

	if (p4)
		if (cnt > 0)
		{
			out << "___Aerodynamic___\n";
			Dbl down = GetAerodynamicDownforceCoefficient();
			Dbl drag = GetAeordynamicDragCoefficient();
			out << "down: " << fToStr(down,2,5) << "  drag: " << fToStr(drag,2,4) << endl;

			if (cnt > 2)
			{
			MATHVECTOR<Dbl,3> aero = GetTotalAero();
			out << "total: " << endl;
			out << fToStr(aero[0],0,5) << " " << fToStr(aero[1],0,4) << " " << fToStr(aero[2],0,6) << endl;

			for (auto a : aerodynamics)
				a.DebugPrint(out);
			}

			//if (cnt > 1)
			{
			// get force and torque at 160kmh  from ApplyAerodynamicsToBody
			out << "--at 160 kmh--" << endl;
			MATHVECTOR<Dbl,3> wind_force(0), wind_torque(0), air_velocity(0);
			air_velocity[0] = -160/3.6;

			for (auto a : aerodynamics)
			{
				MATHVECTOR<Dbl,3> force = a.GetForce(air_velocity, false);
				wind_force = wind_force + force;
				wind_torque = wind_torque + (a.GetPosition() - center_of_mass).cross(force);
			}
			out << "F: " << wind_force << endl << "Tq: " << wind_torque << endl;
			}

			//---
			/*out << "__Tires__" << endl;
			for (int i=0; i < numWheels; ++i)
			{
				CARWHEEL::SlideSlip& sl = wheel[i].slips;
				out << "Fx " << fToStr(sl.Fx,0,6) << "  FxM " << fToStr(sl.Fxm,0,6) << "   Fy " << fToStr(sl.Fy,0,6) << "  FyM " << fToStr(sl.Fym,0,6) << endl;
			}*/
		}
}
///..........................................................................................................


void CARDYNAMICS::UpdateBody(Dbl dt, Dbl drive_torque[])
{
	body.Integrate1(dt);
	cam_body.Integrate1(dt);
	//chassis->clearForces();


	//  camera bounce sim - spring and damper
	MATHVECTOR<Dbl,3> p = cam_body.GetPosition(), v = cam_body.GetVelocity();
	MATHVECTOR<Dbl,3> f = p * gPar.camBncSpring + v * gPar.camBncDamp;
	cam_body.ApplyForce(f);
	cam_body.ApplyForce(cam_force);
	cam_force[0]=0.0;  cam_force[1]=0.0;  cam_force[2]=0.0;


	bool car = vtype == V_Car;
	if (car)
	{
		UpdateWheelVelocity();

		ApplyEngineTorqueToBody();

		ApplyAerodynamicsToBody(dt);
	}
	

	//  extra damage from scene <><>
	if (pScene && pSet->game.damage_type > 0)
	{
		float fRed = pSet->game.damage_type==1 ? 0.5f : 1.f;

		/// <><> terrain layer damage _
		int w;
		for (w=0; w < numWheels; ++w)
		if (!iWhOnRoad[w])
		{
			float d = 0.5f * wheel_contact[w].GetDepth() / wheel[w].GetRadius();
			int mtr = whTerMtr[w]-1;
			if (d < 1.f && mtr >= 0 && mtr < pScene->td.layers.size())
			{
				const TerLayer& lay = pScene->td.layersAll[pScene->td.layers[mtr]];
				if (lay.fDamage > 0.f)
					fDamage += lay.fDamage * fRed * dt;
		}	}

		/// <><> height fog damage _
		if (pScene->fHDamage > 0.f && chassisPosition[2] < pScene->fogHeight)
		{
			float h = (pScene->fogHeight - chassisPosition[2]) / pScene->fogHDensity;
			if (h > 0.2f)  //par
				fDamage += pScene->fHDamage * h * fRed * dt;
		}

		/// <><> fluid damage _
		for (w=0; w < numWheels; ++w)
		if (whH[w] > 0.01f)
			fDamage += whDmg[w] * whH[w] * fRed * dt;
	}
	

	///***  wind ~->
	if (pScene && pScene->windAmt > 0.01f)
	{
		float f = body.GetMass()*pScene->windAmt;
			// simple modulation
			float n = 1.f + 0.3f * sin(time*4.3f)*cosf(time*7.74f);
			time += dt;
		//LogO(fToStr(n,4,6));
		MATHVECTOR<Dbl,3> v(-f*n,0,0);  // todo yaw, dir
		ApplyForce(v);
	}

	///***  manual car flip over  ----------------------------
	if ((doFlip > 0.01f || doFlip < -0.01f) &&
		pSet->game.flip_type > 0 && fDamage < 100.f)
	{
		MATHVECTOR<Dbl,3> av = GetAngularVelocity();  Orientation().RotateVector(av);
		Dbl angvel = fabs(av[0]);
		if (angvel < 2.0)  // max rot vel allowed
		{
		float t = 20000.f * doFlip * flip_mul;  // strength

		if (pSet->game.flip_type == 1)  // fuel dec
		{
			boostFuel -= doFlip > 0.f ? doFlip * dt : -doFlip * dt;
			if (boostFuel < 0.f)  boostFuel = 0.f;
			if (boostFuel <= 0.f)  t = 0.0;
		}
		MATHVECTOR<Dbl,3> v(t,0,0);
		Orientation().RotateVector(v);
		ApplyTorque(v);
		}
	}

	///***  boost  -------------------------------------------
	if (vtype != V_Sphere &&
		doBoost > 0.01f && pSet->game.boost_type > 0)
	{
		/// <><> damage reduce
		float dmg = fDamage >= 80.f ? 0.f : (130.f - fDamage)*0.01f;
		boostVal = doBoost * dmg;
		if (pSet->game.boost_type == 1 || pSet->game.boost_type == 2)  // fuel dec
		{
			boostFuel -= doBoost * dt;
			if (boostFuel < 0.f)  boostFuel = 0.f;
			if (boostFuel <= 0.f)  boostVal = 0.f;
		}
		if (boostVal > 0.01f)
		{
			float f = body.GetMass() * boostVal * 12.f * pSet->game.boost_power;  // power
			MATHVECTOR<Dbl,3> v(f,0,0);
			Orientation().RotateVector(v);
			ApplyForce(v);
		}
	}else
		boostVal = 0.f;
	
	fBoostFov += (boostVal - fBoostFov) * pSet->fov_smooth * 0.0001f;
		
	//  add fuel over time
	if (pSet->game.boost_type == 2 && pGame->timer.pretime < 0.001f && fDamage < 100.f)
	{
		boostFuel += dt * pSet->game.boost_add_sec;
		if (boostFuel > pSet->game.boost_max)  boostFuel = pSet->game.boost_max;
	}
	///***  --------------------------------------------------
	
	
	///  hover
	if (vtype == V_Spaceship)
		SimulateSpaceship(dt);
	else
	///  sphere
	if (vtype == V_Sphere)
		SimulateSphere(dt);
	

	int i;
	Dbl normal_force[MAX_WHEELS];
	if (car)
	{
		for (i = 0; i < numWheels; ++i)
		{
			MATHVECTOR<Dbl,3> suspension_force = UpdateSuspension(i, dt);
			normal_force[i] = suspension_force.dot(wheel_contact[i].GetNormal());
			if (normal_force[i] < 0) normal_force[i] = 0;

			MATHVECTOR<Dbl,3> tire_friction = ApplyTireForce(i, normal_force[i], wheel_orientation[i]);
			ApplyWheelTorque(dt, drive_torque[i], i, tire_friction, wheel_orientation[i]);
		}
	}

	body.Integrate2(dt);
	cam_body.Integrate2(dt);
	//chassis->integrateVelocities(dt);

	// update wheel state
	if (car)
	{
		for (i = 0; i < numWheels; ++i)
		{
			wheel_position[i] = GetWheelPositionAtDisplacement(WHEEL_POSITION(i), suspension[i].GetDisplacementPercent());
			wheel_orientation[i] = Orientation() * GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION(i));
		}
		InterpolateWheelContacts(dt);

		for (i = 0; i < numWheels; ++i)
		{
			if (abs)  DoABS(i, normal_force[i]);
			if (tcs)  DoTCS(i, normal_force[i]);
		}
	}
}


///  Tick  (one Simulation step)
//---------------------------------------------------------------------------------
void CARDYNAMICS::Tick(Dbl dt)
{
	// has to happen before UpdateDriveline, overrides clutch, throttle
	UpdateTransmission(dt);

	const int num_repeats = pSet->dyn_iter;  ///~ 30+  o:10
	const float internal_dt = dt / num_repeats;
	for(int i = 0; i < num_repeats; ++i)
	{
		Dbl drive_torque[MAX_WHEELS];

		UpdateDriveline(internal_dt, drive_torque);

		UpdateBody(internal_dt, drive_torque);

		feedback += 0.5 * (wheel[FRONT_LEFT].GetFeedback() + wheel[FRONT_RIGHT].GetFeedback());
	}

	feedback /= (num_repeats + 1);

	fuel_tank.Consume(engine.FuelRate() * dt);
	//engine.SetOutOfGas(fuel_tank.Empty());
	
	if (fHitTime > 0.f)
		fHitTime -= dt * 2.f;

	const float tacho_factor = 0.1;
	tacho_rpm = engine.GetRPM() * tacho_factor + tacho_rpm * (1.0 - tacho_factor);
}
//---------------------------------------------------------------------------------


void CARDYNAMICS::SynchronizeBody()
{
	MATHVECTOR<Dbl,3> v = ToMathVector<Dbl>(chassis->getLinearVelocity());
	MATHVECTOR<Dbl,3> w = ToMathVector<Dbl>(chassis->getAngularVelocity());
	MATHVECTOR<Dbl,3> p = ToMathVector<Dbl>(chassis->getCenterOfMassPosition());
	QUATERNION<Dbl> q = ToMathQuaternion<Dbl>(chassis->getOrientation());
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
	MATHVECTOR<float,3> raydir = GetDownVector();
	for (int i = 0; i < numWheels; ++i)
	{
		COLLISION_CONTACT & wheelContact = wheel_contact[WHEEL_POSITION(i)];
		MATHVECTOR<float,3> raystart = LocalToWorld(wheel[i].GetExtendedPosition());
		raystart = raystart - raydir * wheel[i].GetRadius();// *0.5;  //*?!
		float raylen = wheel[i].GetRayLength();
		
		world->CastRay( raystart, raydir, raylen, chassis, wheelContact, this,i, !pSet->game.collis_cars, false );
	}
}


/// calculate the center of mass, calculate the total mass of the body, calculate the inertia tensor
/// then store this information in the rigid body
void CARDYNAMICS::UpdateMass()
{
	typedef std::pair <Dbl, MATHVECTOR<Dbl,3> > MASS_PAIR;

	Dbl total_mass(0);
	center_of_mass.Set(0,0,0);

	// calculate the total mass, and center of mass
	for (auto i : mass_only_particles)
	{
		// add the current mass to the total mass
		total_mass += i.first;

		// incorporate the current mass into the center of mass
		center_of_mass = center_of_mass + i.second * i.first;
	}

	// account for fuel
	total_mass += fuel_tank.GetMass();
	center_of_mass = center_of_mass + fuel_tank.GetPosition() * fuel_tank.GetMass();

	body.SetMass(total_mass);
	cam_body.SetMass(1350/*total_mass/**/ * gPar.camBncMass);
	fBncMass = 1350;//1.0; //1350.0 / total_mass;

	center_of_mass = center_of_mass * (1.0 / total_mass);
	
	// calculate the inertia tensor
	MATRIX3 <Dbl> inertia;
	for (int i = 0; i < 9; ++i)
		inertia[i] = 0;

	for (auto i : mass_only_particles)
	{
		// transform into the rigid body coordinates
		MATHVECTOR<Dbl,3> pos = i.second - center_of_mass;
		Dbl mass = i.first;

		// add the current mass to the inertia tensor
		inertia[0] += mass * ( pos[1] * pos[1] + pos[2] * pos[2] );
		inertia[1] -= mass * ( pos[0] * pos[1] );
		inertia[2] -= mass * ( pos[0] * pos[2] );
		inertia[3] = inertia[1];
		inertia[4] += mass * ( pos[2] * pos[2] + pos[0] * pos[0] );
		inertia[5] -= mass * ( pos[1] * pos[2] );
		inertia[6] = inertia[2];
		inertia[7] = inertia[5];
		inertia[8] += mass * ( pos[0] * pos[0] + pos[1] * pos[1] );
	}
	// inertia.DebugPrint(std::cout);
	body.SetInertia( inertia );
}


///  HOVER Spaceship
///..........................................................................................................
void CARDYNAMICS::SimulateSpaceship(Dbl dt)
{
	//sHov = "";

	//  destroyed  damping
	if (fDamage >= 100.f)
	{
		btVector3 v = chassis->getLinearVelocity();
		v[2] *= 0.1;
		chassis->applyCentralForce(v * -20);

		btVector3 av = chassis->getAngularVelocity();
		chassis->applyTorque(av * -40);
		hov_roll = 0.f;  //-
		return;
	}

	float dmg = fDamage > 50.f ? 1.f - (fDamage-50.f)*0.02f : 1.f;
	float dmgE = 1.f - 0.2 * dmg;

	//  cast ray down .
	COLLISION_CONTACT ct,ct2;
	MATHVECTOR<Dbl,3> dn = GetDownVector();
	Dbl ups = dn[2] < 0.0 ? 1.0 : -1.0;
		//sHov += " dn "+fToStr(dn[2],2,5)+"\n";

	/*const */Dbl len = hov.hAbove, rlen = hov.hRayLen;
	MATHVECTOR<Dbl,3> p = GetPosition();  // - dn * 0.1;  // v fluids as solids
	world->CastRay(p, dn, rlen, chassis, ct,  0,0, false, true);
	float d = ct.GetDepth();

	//  2nd in front for pitch
	MATHVECTOR<Dbl,3> dx(3.4, 0, 0);  //par
	Orientation().RotateVector(dx);
	
	world->CastRay(p+dx, dn, rlen, chassis, ct2,  0,0, false, true);
	float d2 = ct2.GetDepth();

	//  pipe..?
	bool pipe = false;
	if (ct.GetColObj())
	{
		int su = (long)ct.GetColObj()->getCollisionShape()->getUserPointer();
		if (su >= SU_Pipe && su < SU_RoadWall)
			pipe = true;
	}
	if (pipe)
	{	len *= 1.2;  rlen *= 0.9;  }  ///!par
	
	//  vel
	MATHVECTOR<Dbl,3> sv = -GetVelocity();
	(-Orientation()).RotateVector(sv);
	MATHVECTOR<Dbl,3> av = GetAngularVelocity();

	//  roll /  vis only
	hov_roll = sv[1] * hov.roll;  // vis degrees
	hov_roll = std::max(-90.f, std::min(90.f, hov_roll));


	//  steer  < >
	bool rear = sv[0] > 0.0;
	Dbl rr = std::max(-1.0, std::min(1.0, -sv[0] * 0.4));  //par
		//sHov += " rr "+fToStr(rr,2,5)+"\n";

	MATHVECTOR<Dbl,3> t(0,0, -1000.0 * rr * ups * hov.steerForce * steerValue * dmgE);
	Orientation().RotateVector(t);
	Dbl damp = pipe ? hov.steerDamp : hov.steerDamp;  //damp *= 1 - fabs(steerValue);
	ApplyTorque(t - av * damp * 1000.0);  // rotation damping


	//  handbrake damping
	btVector3 v = chassis->getLinearVelocity();
	Dbl h = brake[0].GetHandbrakeFactor();
	if (h > 0.01f)
	{
		chassis->applyCentralForce(v * h * -20);  //par
		btVector3 av = chassis->getAngularVelocity();
		chassis->applyTorque(av * h * -20);
	}
	
	//  engine  ^
	float vel = sv.Magnitude(),  //  decrease power with velocity
		velMul = 1.f - std::min(1.f, hov.engineVelDec * vel),
		velMulR = 1.f - std::min(1.f, hov.engineVelDecR * vel);
		//sHov += " m  "+fToStr(velMul,2,5)+"\n";

	Dbl brk = brake[0].GetBrakeFactor() * (1.0 - h);
	float f = hov.engineForce * velMul * hov_throttle * dmgE
			- hov.brakeForce * (rear ? velMulR : 1.f) * brk * dmgE;

	MATHVECTOR<Dbl,3> vf(body.GetMass() * f, 0,0);
	Orientation().RotateVector(vf);
	ApplyForce(vf);


	//  side, vel damping  --
	sv[0] *= hov.dampAirRes;
	sv[1] *= hov.dampSide;
	sv[2] *= sv[2] > 0.0 ? hov.dampUp : hov.dampDn;
	Orientation().RotateVector(sv);
	Dbl ss = pipe ? hov.dampPmul : 1;
	ApplyForce(sv * ss);

	
	//  align straight torque
	MATHVECTOR <float,3> n = ct.GetNormal();  // ground
	MATHVECTOR <float,3> n2 = ct2.GetNormal();
	if (!(d > 0.f && d < rlen))  n = MATHVECTOR <float,3>(0,0,1);  // in air
	MATHVECTOR<Dbl,3> al = dn.cross(n), ay = al;
	MATHVECTOR<Dbl,3> ay2 = dn.cross(n2);
	if (pipe) {  al[0] *= hov.alp[0];  al[1] *= hov.alp[1];  al[2] *= hov.alp[2];  }
	else      {  al[0] *= hov.alt[0];  al[1] *= hov.alt[1];  al[2] *= hov.alt[2];  }
	ApplyTorque(al * -1000.0);

		//sHov += " a1 "+fToStr(ay[0],2,5)+" "+fToStr(ay[1],2,5)+" "+fToStr(ay[2],2,5) +"\n";
		//sHov += " a2 "+fToStr(ay2[0],2,5)+" "+fToStr(ay2[1],2,5)+" "+fToStr(ay2[2],2,5) +"\n";
		//sHov += " 12 "+fToStr(ay[0]-ay2[0],2,5)+" "+fToStr(ay[1]-ay2[1],2,5)+" "+fToStr(ay[2]-ay2[2],2,5) +"\n";

	//  pitch torque )
	Dbl pitch = (d < len && d2 < len) ? (d2 - d) * hov.pitchTq * 1000.f : 0.f;
	Dbl roll = sv[1] * hov.rollTq * -1000.f;
	Dbl yawP = !pipe ? 0.0 : 600.0 * (ay[0]-ay2[0]);
	Dbl spiP = !pipe ? 0.0 : 600.0 * (ay[1]-ay2[1]);
		//sHov += " yp "+fToStr(yawP,0,5)+" "+fToStr(spiP,0,5) +"\n";
	MATHVECTOR<Dbl,3> tq(roll, pitch + spiP * ups, ups * yawP);
	Orientation().RotateVector(tq);
	ApplyTorque(tq);


	///  heavy landing damp  __
	Dbl vz = chassis->getLinearVelocity().dot( pipe ? ToBulletVector(-dn) : ToBulletVector(n) );
		suspension[1].velocity = vz * hov.hov_vz;  // for graphs
		suspension[1].displacement = d / len;
		suspension[0].displacement = d / rlen;
	
	float aa = std::min(1.0, vz * hov.hov_vz);
	float df = vz > 0 ? (1.0 - hov.hov_vsat * aa) : 1.0;
          df *= pipe ? hov.hov_dampP : hov.hov_damp;

	float dlen = len * hov.hov_dsat;
	float dm = d > dlen ? 0 : (0.5+0.5*d/len) * df;
		suspension[2].displacement = dm;
		suspension[3].displacement = (d < dlen ? (len-d) * 1.f : 0.f);
	float fn =
		(d > dlen ? -hov.hov_fall : 0.f) +  // fall down force v
		//  anti grav force  TODO: goes crazy in pipes
		(d < dlen ? (dlen-d) * (pipe ? hov.hov_riseP : hov.hov_rise) : 0.f) +
		dm * -1000.f * vz;
	chassis->applyCentralForce(ToBulletVector(-dn * fn * dmgE));
}


///  SPHERE
///..........................................................................................................
void CARDYNAMICS::SimulateSphere(Dbl dt)
{
	//  destroyed  damping
	if (fDamage >= 100.f)
	{
		btVector3 v = chassis->getLinearVelocity();
		v[2] *= 0.1;
		chassis->applyCentralForce(v * -10);

		btVector3 av = chassis->getAngularVelocity();
		chassis->applyTorque(av * -10);
		return;
	}

	float dmg = fDamage > 50.f ? 1.f - (fDamage-50.f)*0.02f : 1.f;
	float dmgE = 1.f - 0.1 * dmg;

	//  ray,  only to check if in pipe
	/*const */Dbl len = hov.hAbove, rlen = hov.hRayLen;
	COLLISION_CONTACT ct;
	MATHVECTOR<Dbl,3> p = GetPosition();  // - dn * 0.1;  // v fluids as solids
	MATHVECTOR<float,3> dn(0,0,-1);
	world->CastRay(p, dn, rlen, chassis, ct,  0,0, false, true);
	float d = ct.GetDepth();

	//  pipe
	bool pipe = false;
	if (ct.GetColObj())
	{
		int su = (long)ct.GetColObj()->getCollisionShape()->getUserPointer();
		if (su >= SU_Pipe && su < SU_RoadWall)
			pipe = true;
	}

	//  engine
	//bool rear = false;  //transmission.GetGear() < 0;
	MATHVECTOR<Dbl,3> sv = -GetVelocity();
	//(-Orientation()).RotateVector(sv);
	
	float vel = sv.Magnitude(),  //  decrease power with velocity
		velMul = 1.f - std::min(1.f, hov.engineVelDec * vel);
	float f = hov.engineForce * velMul * hov_throttle * dmgE
			- hov.brakeForce * velMul * brake[0].GetBrakeFactor() * dmgE;
	//if (rear)  f *= -1.f;

	float h = brake[0].GetHandbrakeFactor();

	//  steer <> rotate dir
	float pst = hov.steerForce;  //if (rear)  pst = -pst;
	if (pipe)  pst *= hov.steerDampP;

	float hh = 1.f + 1.0f * sqrt(h);  // factor, faster steer with handbrake
	sphereYaw += steerValue * hh * dt * pst * PI_d/180.f;
	MATHVECTOR<Dbl,3> dir(cosf(sphereYaw), -sinf(sphereYaw), 0);

	f *= body.GetMass() * -1.0;
	btVector3 fc = ToBulletVector(dir * f);
		if (!pipe)  fc += btVector3(0,0,-hov.hov_fall);
	chassis->applyCentralForce(fc);

	//  handbrake damping
	btVector3 v = chassis->getLinearVelocity();
	if (h > 0.01f)
	{
		chassis->applyCentralForce(v * h * -10.f);

		btVector3 av = chassis->getAngularVelocity();
		chassis->applyTorque(av * h * -10.f);
	}

	//  side damp --
	btVector3 vv(dir[1], -dir[0], 0.f);
	float pmul = hov.dampSide;
		if (pipe)  pmul *= hov.dampPmul;
	float dot = v.getX()*vv.getX() + v.getY()*vv.getY();
	chassis->applyCentralForce(vv * dot * -pmul
		/*- btVector3(0,0, v.getZ()*100)*/ );
}
