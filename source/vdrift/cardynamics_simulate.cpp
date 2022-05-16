#include "pch.h"
#include "par.h"
#include "cardynamics.h"
#include "tobullet.h"
#include "../ogre/common/Def_Str.h"
#include "isnan.h"


//  set the steering angle,  left -1..1 right
void CARDYNAMICS::SetSteering(const Dbl value1, const float range_mul)
{
	/// <><> damage reduce
	float dmg = fDamage >= 100.f ? 0.f : 1.f - 0.1f * fDamage*0.01f;
	Dbl value = value1 * dmg;
	
	steerValue = value;
	Dbl steerangle = value * maxangle * range_mul;  //steering angle in degrees
	if (numWheels == 2)  // bikes
	{	wheel[FRONT_LEFT].SetSteerAngle( steerangle );
		return;
	}
	
	// ackermann stuff
	int axles = numWheels / 2;  // even wheel count only
	for (int i=0; i < axles; ++i)
	{
		Dbl str = wheel[i*2].GetSteerMax();
		if (fabs(str) < 0.1)
			continue;

		int wl = FRONT_LEFT + i*2,
			wr = FRONT_RIGHT + i*2,
			rear = str > 0.0 ? wl + 2 : wl - 2;
		wl = std::max(int(FRONT_LEFT), std::min(int(REAR3_RIGHT), wl));
		wr = std::max(int(FRONT_LEFT), std::min(int(REAR3_RIGHT), wr));
		rear = std::max(int(FRONT_LEFT), std::min(int(REAR3_RIGHT), rear));

		Dbl alpha = std::abs( steerangle * PI_d/180.0 );  //outside wheel steering angle in radians
		Dbl dW = wheel[wl].GetExtendedPosition()[1] - wheel[wr].GetExtendedPosition()[1];  // distance between front wheels
		Dbl dL = wheel[wl].GetExtendedPosition()[0] - wheel[rear].GetExtendedPosition()[0];  // distance between front and rear wheels
		Dbl beta = atan2(1.0, 1.0 / tan(alpha) - dW / fabs(dL) );  //inside wheel steering angle in radians

		Dbl left = 0, right = 0;  // wheel angle
		if (value >= 0)	{	left = alpha;	right = beta;	}
		else			{	right = -alpha;	left = -beta;	}

		left *= 180.0/PI_d;  right *= 180.0/PI_d;
		if (i==1)  {  left *= -1.0;  right *= -1.0;  }  // inversed rear axle steer // par

		wheel[wl].SetSteerAngle( str * left );
		wheel[wr].SetSteerAngle( str * right );
	}
}


void CARDYNAMICS::ApplyEngineTorqueToBody()
{
	MATHVECTOR<Dbl,3> engine_torque(-engine.GetTorque(), 0, 0);
	Orientation().RotateVector(engine_torque);
	ApplyTorque(engine_torque*0.1);  // unwanted in jumps
}

void CARDYNAMICS::ApplyAerodynamicsToBody(Dbl dt)
{
	MATHVECTOR<Dbl,3> wind_force(0);
	MATHVECTOR<Dbl,3> wind_torque(0);
	MATHVECTOR<Dbl,3> air_velocity = -GetVelocity();
	(-Orientation()).RotateVector(air_velocity);

	for (auto i = aerodynamics.begin(); i != aerodynamics.end(); ++i)
	{
		MATHVECTOR<Dbl,3> force = i->GetForce(air_velocity);
		wind_force = wind_force + force;
		wind_torque = wind_torque + (i->GetPosition() - center_of_mass).cross(force);
	}
	Orientation().RotateVector(wind_force);
	Orientation().RotateVector(wind_torque);
	ApplyForce(wind_force);
	ApplyTorque(wind_torque);

	//  rotational damping/drag  ----------------
	if (rot_coef[0] > 0.0 && chassis)
	{
		MATHVECTOR<Dbl,3> rot_drag = -ToMathVector<Dbl>(chassis->getAngularVelocity());
		(-Orientation()).RotateVector(rot_drag);  // apply factors in car space
			// linear 50000 test big  10000 heavy  1000 light
			// sqare 2.5 ok, limits max rot vel-
			//LogO("a"+fToStr(rot_drag[2],3,6));
		rot_drag[0] *= rot_coef[0];  // roll
		rot_drag[1] *= rot_coef[1];  // pitch
		rot_drag[2] *= rot_coef[2] + rot_coef[3] * rot_drag[2];  // yaw
			//LogO("b"+fToStr(rot_drag[2],3,6));
		Orientation().RotateVector(rot_drag);
		ApplyTorque(rot_drag);
	}
}

MATHVECTOR<Dbl,3> CARDYNAMICS::UpdateSuspension(int i, Dbl dt)
{
	// displacement
	const Dbl posx = wheel_contact[i].GetPosition()[0];
	const Dbl posz = wheel_contact[i].GetPosition()[2];
	const TRACKSURFACE & surface = wheel_contact[i].GetSurface();
	Dbl phase = 0;
	if (surface.bumpWaveLength > 0.0001)
		phase = 2 * PI_d * (posx + posz) / surface.bumpWaveLength;

	Dbl shift = 2.0 * sin (phase*1.414214);
	Dbl amplitude = 0.25 * surface.bumpAmplitude;
	Dbl bumpoffset = amplitude * (sin(phase + shift) + sin(phase*1.414214) - 2.0);
	
	Dbl displacement = 2.0 * wheel[i].GetRadius() - wheel_contact[i].GetDepth() + bumpoffset;

	// compute suspension force
	Dbl springdampforce = suspension[WHEEL_POSITION ( i ) ].Update ( dt , displacement );

	//do anti-roll
	int otheri = i;
	if (i%2 == 0)	++otheri;
	else			--otheri;

	Dbl antirollforce = suspension[WHEEL_POSITION(i)].GetAntiRollK() *
	                  ( suspension[WHEEL_POSITION(i)].GetDisplacement() -
	                    suspension[WHEEL_POSITION(otheri)].GetDisplacement() );

	//suspension[WHEEL_POSITION(i)].SetAntiRollInfo(antirollforce);
	if (isnan( antirollforce ))  antirollforce = 0.f;  //crash dyn obj-
	assert(!isnan( antirollforce ));

	//find the vector direction to apply the suspension force
#ifdef SUSPENSION_FORCE_DIRECTION
	const MATHVECTOR<Dbl,3> & wheelext = wheel[i].GetExtendedPosition();
	const MATHVECTOR<Dbl,3> & hinge = suspension[i].GetHinge();

	MATHVECTOR<Dbl,3> relwheelext = wheelext - hinge;
	MATHVECTOR<Dbl,3> up(0,0,1);
	MATHVECTOR<Dbl,3> rotaxis = up.cross( relwheelext.Normalize() );
	MATHVECTOR<Dbl,3> forcedirection = relwheelext.Normalize().cross(rotaxis);
	//std::cout << i << ". " << forcedirection << std::endl;

	MATHVECTOR<Dbl,3> susp_force = forcedirection * (antirollforce + springdampforce);
#else
	MATHVECTOR<Dbl,3> susp_force(0, 0, antirollforce + springdampforce);
#endif
	Orientation().RotateVector(susp_force);
	return susp_force;
}


//---------------------------------------------------------------------------------------------------------------------------
///  Wheels
//---------------------------------------------------------------------------------------------------------------------------

MATHVECTOR<Dbl,3> CARDYNAMICS::ApplyTireForce(int i, const Dbl normal_force, const QUATERNION<Dbl> & wheel_space)
{
	//  aplies tire friction  to car, returns friction in world space
	CARWHEEL& wheel = this->wheel[WHEEL_POSITION(i)];
	const COLLISION_CONTACT & wheel_contact = this->wheel_contact[WHEEL_POSITION(i)];
	const TRACKSURFACE & surface = wheel_contact.GetSurface();
	const CARTIRE* tire = surface.tire;  // this->tire[WHEEL_POSITION(i)];
	const MATHVECTOR<Dbl,3> surface_normal = wheel_contact.GetNormal();

	//  camber relative to surface(clockwise in wheel heading direction)
	MATHVECTOR<Dbl,3> wheel_axis(0,1,0);
	wheel_space.RotateVector(wheel_axis); // wheel axis in world space (wheel plane normal)
	Dbl camber_sin = wheel_axis.dot(surface_normal);
	Dbl camber_rad = asin(camber_sin);
	wheel.SetCamberDeg(camber_rad * 180.0/PI_d);

	//  tire space(SAE Tire Coordinate System)
	//  surface normal is z-axis
	//  wheel axis projected on surface plane is y-axis
	MATHVECTOR<Dbl,3> y_axis = wheel_axis - surface_normal * camber_sin;
	MATHVECTOR<Dbl,3> x_axis = y_axis.cross(surface_normal);

	//  wheel center velocity in tire space
	MATHVECTOR<Dbl,3> hub_velocity;
	hub_velocity[0] = x_axis.dot(wheel_velocity[WHEEL_POSITION(i)]);
	hub_velocity[1] = y_axis.dot(wheel_velocity[WHEEL_POSITION(i)]);
	hub_velocity[2] = 0; // unused

	//  rearward speed of the contact patch
	Dbl patch_speed = wheel.GetAngularVelocity() * wheel.GetRadius();

	//  friction force in tire space
	//Dbl friction_coeff = tire.GetTread() * surface.friction + (1.0 - tire.GetTread()) * surface.frictionNonTread;
	Dbl friction_coeff = surface.friction * wheel.GetFriction();
	//Dbl roll_friction_coeff = surface.rollResistanceCoefficient;
	MATHVECTOR<Dbl,3> friction_force(0);
	if (friction_coeff > 0)
		friction_force = tire->GetForce(
			normal_force /* wheel.GetFriction()*/, friction_coeff, //roll_friction_coeff,
			hub_velocity, patch_speed, camber_rad, &wheel.slips);

	///  multipliers x,y test
	friction_force[0] *= surface.frictionX;
	friction_force[1] *= surface.frictionY;
	
	//  set force feedback (aligning torque in tire space)
	wheel.SetFeedback(friction_force[2]);

	//  friction force in world space
	MATHVECTOR<Dbl,3> world_friction_force = x_axis * friction_force[0] + y_axis * friction_force[1];

	//  fake viscous friction (sand, gravel, mud)
	MATHVECTOR<Dbl,3> wheel_drag = - (x_axis * hub_velocity[0] + y_axis * hub_velocity[1]) * surface.rollingDrag;

	//  apply forces to body
	MATHVECTOR<Dbl,3> wheel_normal(0, 0, 1);
	wheel_space.RotateVector(wheel_normal);
	MATHVECTOR<Dbl,3> contactpos = wheel_position[WHEEL_POSITION(i)] + wheel_normal * wheel.GetRadius() * wheel.GetRollHeight();  ///+
	ApplyForce(world_friction_force + surface_normal * normal_force + wheel_drag, contactpos - Position());

	return world_friction_force;
}


void CARDYNAMICS::ApplyWheelTorque(Dbl dt, Dbl drive_torque, int i, MATHVECTOR<Dbl,3> tire_friction, const QUATERNION<Dbl> & wheel_space)
{
	CARWHEEL & wheel = this->wheel[WHEEL_POSITION(i)];
	CARBRAKE & brake = this->brake[WHEEL_POSITION(i)];

	//  tire force / torque
	wheel.Integrate1(dt);

	(-wheel_space).RotateVector(tire_friction);

	//  torques acting on wheel
	Dbl friction_torque = tire_friction[0] * wheel.GetRadius();
	Dbl wheel_torque = drive_torque - friction_torque;
	Dbl lock_up_torque = wheel.GetLockUpTorque(dt) - wheel_torque;	// torque needed to lock the wheel
	Dbl angVel = wheel.GetAngularVelocity();  if (angVel < 0.0)  angVel = -angVel; //
	Dbl brake_torque = brake.GetTorque()
		+ wheel.fluidRes * angVel;  /// fluid resistance

	//  brake and rolling resistance torque should never exceed lock up torque
	if (lock_up_torque >= 0 && lock_up_torque > brake_torque)
	{
		brake.WillLock(false);  wheel_torque += brake_torque;   // brake torque has same direction as lock up torque
	}
	else if (lock_up_torque < 0 && lock_up_torque < -brake_torque)
	{
		brake.WillLock(false);  wheel_torque -= brake_torque;
	}else
	{
		brake.WillLock(true);   wheel_torque = wheel.GetLockUpTorque(dt);
	}

	//set wheel torque due to tire rolling resistance
	Dbl rolling_resistance = wheel.GetRollingResistance(wheel.GetAngularVelocity(), wheel_contact[i].GetSurface().rollingResist);
	Dbl tire_rolling_resistance_torque = - rolling_resistance * wheel.GetRadius();  //- tire_friction_torque;
	//assert(!isnan(tire_rolling_resistance_torque));
	
		//LogO(fToStr(wheel_torque, 4,9)+" r "+fToStr(tire_rolling_resistance_torque, 4,9));
	wheel.SetTorque(wheel_torque*0.5 + tire_rolling_resistance_torque);
	wheel.Integrate2(dt);

	//  apply torque to body  -not wanted in jumps
	///+  z yaw  y pitch
	//MATHVECTOR<Dbl,3> world_wheel_torque(0, -wheel_torque, 0);
	//wheel_space.RotateVector(world_wheel_torque);
	//ApplyTorque(world_wheel_torque);
	
	
	///: bike align straight torque from wheels
	if (numWheels == 2 && fDamage < 100.f)
	{
		float dmg = 1.f - 0.5f * fDamage*0.01f;
		Dbl v = GetSpeedDir() * 1./50.;
		v = 0.05 + 0.95 * std::min(1.0, v);  //par
		MATHVECTOR<float,3> dn = GetDownVector();

		for (int w=0; w < numWheels; ++w)
		if (wheel_contact[w].GetColObj())
		{
			MATHVECTOR <float,3> n = wheel_contact[w].GetNormal();
			MATHVECTOR <float,3> t = dn.cross(n);
			(-Orientation()).RotateVector(t);
			Dbl x = t[0] * -1000. * v * 22 * dmg;  ///par in .car ...
			MATHVECTOR<Dbl,3> v(x,0,0);
			Orientation().RotateVector(v);
			ApplyTorque(v);
		}
	}
}


void CARDYNAMICS::InterpolateWheelContacts(Dbl dt)
{
	MATHVECTOR<Dbl,3> raydir = GetDownVector();
	for (int i = 0; i < numWheels; ++i)
	{
		MATHVECTOR<Dbl,3> raystart = LocalToWorld(wheel[i].GetExtendedPosition());
		raystart = raystart - raydir * wheel[i].GetRadius();
		
		GetWheelContact(WHEEL_POSITION(i)).CastRay(raystart, raydir, wheel[i].GetRayLength());
	}
}

void CARDYNAMICS::UpdateDriveline(Dbl dt, Dbl drive_torque[])
{
	engine.Integrate1(dt);

	Dbl driveshaft_speed = CalculateDriveshaftSpeed();
	Dbl clutch_speed = transmission.CalculateClutchSpeed(driveshaft_speed);
	Dbl crankshaft_speed = engine.GetAngularVelocity();
	Dbl engine_drag = clutch.GetTorque(crankshaft_speed, clutch_speed);
	engine_drag += 0.1;  // fixes clutch stall bug when car vel = 0 and all wheels in air

	engine.ComputeForces();

	ApplyClutchTorque(engine_drag, clutch_speed);

	engine.ApplyForces();

	CalculateDriveTorque(drive_torque, engine_drag);

	engine.Integrate2(dt);
}


//  Shafts
//---------------------------------------------------------------------------------------------------------------------------

/// apply forces on the engine due to drag from the clutch
void CARDYNAMICS::ApplyClutchTorque(Dbl engine_drag, Dbl clutch_speed)
{
	engine.SetClutchTorque( transmission.GetGear() == 0 ? 0.0 : engine_drag);
}

/// calculate the drive torque that the engine applies to each wheel,
/// and put the output into the supplied 4-element array
void CARDYNAMICS::CalculateDriveTorque(Dbl * wh/*wheel_drive_torque*/, Dbl clutch_torque)
{
	Dbl driveshaft_torque = transmission.GetTorque( clutch_torque );
	assert( !isnan(driveshaft_torque) );

	for (int i = 0; i < numWheels; ++i)
		wh[i] = 0;

	if (drive == AWD)
	{
		diff_center.ComputeWheelTorques( driveshaft_torque );
		diff_front.ComputeWheelTorques( diff_center.GetSide1Torque() );
		diff_rear.ComputeWheelTorques( diff_center.GetSide2Torque() );
		wh[FRONT_LEFT] = diff_front.GetSide1Torque();
		wh[FRONT_RIGHT] = diff_front.GetSide2Torque();
		wh[REAR_LEFT] = diff_rear.GetSide1Torque();
		wh[REAR_RIGHT] = diff_rear.GetSide2Torque();
	}
	else if (drive == WD6)
	{
		diff_center2.ComputeWheelTorques( driveshaft_torque );
		diff_center.ComputeWheelTorques( driveshaft_torque );
		diff_front.ComputeWheelTorques( diff_center.GetSide1Torque() );
		diff_rear.ComputeWheelTorques( diff_center.GetSide2Torque() );
		diff_rear2.ComputeWheelTorques( diff_center2.GetSide2Torque() );
		wh[FRONT_LEFT]  = diff_front.GetSide1Torque();
		wh[FRONT_RIGHT] = diff_front.GetSide2Torque();
		wh[REAR_LEFT]   = diff_rear.GetSide1Torque();
		wh[REAR_RIGHT]  = diff_rear.GetSide2Torque();
		wh[REAR2_LEFT]  = diff_rear2.GetSide1Torque();
		wh[REAR2_RIGHT] = diff_rear2.GetSide2Torque();
	}
	else if (drive == WD8)
	{
		diff_center3.ComputeWheelTorques( driveshaft_torque );
		diff_center.ComputeWheelTorques( diff_center3.GetSide1Torque() );
		diff_center2.ComputeWheelTorques( diff_center3.GetSide2Torque() );

		diff_front.ComputeWheelTorques( diff_center.GetSide1Torque() );
		diff_rear.ComputeWheelTorques( diff_center.GetSide2Torque() );
		diff_rear2.ComputeWheelTorques( diff_center2.GetSide1Torque() );
		diff_rear3.ComputeWheelTorques( diff_center2.GetSide2Torque() );
		wh[FRONT_LEFT]  = diff_front.GetSide1Torque();
		wh[FRONT_RIGHT] = diff_front.GetSide2Torque();
		wh[REAR_LEFT]   = diff_rear.GetSide1Torque();
		wh[REAR_RIGHT]  = diff_rear.GetSide2Torque();
		wh[REAR2_LEFT]  = diff_rear2.GetSide1Torque();
		wh[REAR2_RIGHT] = diff_rear2.GetSide2Torque();
		wh[REAR3_LEFT]  = diff_rear3.GetSide1Torque();
		wh[REAR3_RIGHT] = diff_rear3.GetSide2Torque();
	}
	else if (drive == RWD)
	{
		diff_rear.ComputeWheelTorques( driveshaft_torque );
		wh[REAR_LEFT] = diff_rear.GetSide1Torque();
		wh[REAR_RIGHT] = diff_rear.GetSide2Torque();
	}
	else if (drive == FWD)
	{
		diff_front.ComputeWheelTorques( driveshaft_torque );
		wh[FRONT_LEFT] = diff_front.GetSide1Torque();
		wh[FRONT_RIGHT] = diff_front.GetSide2Torque();
	}

	for (int i = 0; i < numWheels; ++i)  assert(!isnan( wh[WHEEL_POSITION(i)] ));
}

Dbl CARDYNAMICS::CalculateDriveshaftSpeed()
{
	Dbl whFL = wheel[FRONT_LEFT].GetAngularVelocity();
	Dbl whFR = wheel[FRONT_RIGHT].GetAngularVelocity();
	if (drive == FWD)
		return diff_front.CalcSpeed( whFL, whFR );

	Dbl whRL = wheel[REAR_LEFT].GetAngularVelocity();
	Dbl whRR = wheel[REAR_RIGHT].GetAngularVelocity();
	for (int i = 0; i < numWheels; ++i)  assert(!isnan( wheel[WHEEL_POSITION(i)].GetAngularVelocity() ));

	if (drive == AWD)
		return diff_center.CalcSpeed(
			diff_front.CalcSpeed( whFL, whFR ),
			diff_rear.CalcSpeed( whRL, whRR ) );
	else if (drive == RWD)
		return diff_rear.CalcSpeed( whRL, whRR );

	//  6, 8 wheels
	Dbl whRL2 = wheel[REAR2_LEFT].GetAngularVelocity();
	Dbl whRR2 = wheel[REAR2_RIGHT].GetAngularVelocity();

	if (drive == WD6)
	/*	[] front []  // todo: fix gear change..
			diff_center  \
		[] rear  []      diff_center2 - engine
			 			 /
		[] rear2 []
	*/	return diff_center2.CalcSpeed(
			diff_center.CalcSpeed(
		    	diff_front.CalcSpeed( whFL, whFR ),
		        diff_rear.CalcSpeed( whRL, whRR ) ),
			diff_rear2.CalcSpeed( whRL2, whRR2 ) );
	
	Dbl whRL3 = wheel[REAR3_LEFT].GetAngularVelocity();
	Dbl whRR3 = wheel[REAR3_RIGHT].GetAngularVelocity();

	//if (drive == WD8)  // todo: just locked diffs how?
	/*	[] front []
			diff_center  \
		[] rear  []       \ 
						diff_center3 - engine
		[] rear2 []       /
			diff_center2 /
		[] rear3 []
	*/	return diff_center3.CalcSpeed(
			diff_center.CalcSpeed(
		    	diff_front.CalcSpeed( whFL, whFR ),
		        diff_rear.CalcSpeed( whRL, whRR ) ),
			diff_center2.CalcSpeed(
		    	diff_rear2.CalcSpeed( whRL2, whRR2 ),
		        diff_rear3.CalcSpeed( whRL3, whRR3 ) ) );
}

//  rpm for gearbox
Dbl CARDYNAMICS::CalculateDriveshaftRPM() const
{
	for (int i = 0; i < numWheels; ++i)  assert(!isnan( wheel[WHEEL_POSITION(i)].GetAngularVelocity() ));
	Dbl speed = 0.0;
	Dbl whFL = wheel[FRONT_LEFT].GetAngularVelocity();
	Dbl whFR = wheel[FRONT_RIGHT].GetAngularVelocity();
	if (drive == FWD)
	{
		speed = diff_front.GetSpeed( whFL, whFR );
		return transmission.GetClutchSpeed( speed ) * 30.0 / PI_d;
	}
	Dbl whRL = wheel[REAR_LEFT].GetAngularVelocity();
	Dbl whRR = wheel[REAR_RIGHT].GetAngularVelocity();
	if (drive == RWD)
	{
		speed = diff_rear.GetSpeed( whRL, whRR );
		return transmission.GetClutchSpeed( speed ) * 30.0 / PI_d;
	}
	if (drive == AWD)
	{
		Dbl front = diff_front.GetSpeed( whFL, whFR );
		Dbl rear = diff_rear.GetSpeed( whRL, whRR );
		speed = diff_center.GetSpeed( front, rear );
		return transmission.GetClutchSpeed( speed ) * 30.0 / PI_d;
	}
	
	Dbl whRL2 = wheel[REAR2_LEFT].GetAngularVelocity();
	Dbl whRR2 = wheel[REAR2_RIGHT].GetAngularVelocity();
	if (drive == WD6)
	{
		Dbl front = diff_front.GetSpeed( whFL, whFR );
		Dbl rear = diff_rear.GetSpeed( whRL, whRR );
		Dbl center = diff_center.GetSpeed( front, rear );
		Dbl rear2 = diff_rear2.GetSpeed( whRL2, whRR2 );
		Dbl center2 = diff_center2.GetSpeed( center, rear2 );
		return transmission.GetClutchSpeed( speed ) * 30.0 / PI_d;
	}
	//if (drive == WD8)
	Dbl whRL3 = wheel[REAR3_LEFT].GetAngularVelocity();
	Dbl whRR3 = wheel[REAR3_RIGHT].GetAngularVelocity();
	{
		Dbl front = diff_front.GetSpeed( whFL, whFR );
		Dbl rear = diff_rear.GetSpeed( whRL, whRR );
		Dbl center = diff_center.GetSpeed( front, rear );
		Dbl rear2 = diff_rear2.GetSpeed( whRL2, whRR2 );
		Dbl rear3 = diff_rear3.GetSpeed( whRL3, whRR3 );
		Dbl center2 = diff_center2.GetSpeed( rear2, rear3 );
		speed = diff_center3.GetSpeed( center, center2 );
	}
	return transmission.GetClutchSpeed( speed ) * 30.0 / PI_d;
}

//  speed for gearbox and HUD
Dbl CARDYNAMICS::GetSpeedMPS() const
{
	Dbl whFL = wheel[FRONT_LEFT].GetAngularVelocity();
	Dbl whFR = wheel[FRONT_RIGHT].GetAngularVelocity();
	if (drive == FWD)
		return (whFL+whFR) * 0.5 * wheel[FRONT_LEFT].GetRadius();

	Dbl whRL = wheel[REAR_LEFT].GetAngularVelocity();
	Dbl whRR = wheel[REAR_RIGHT].GetAngularVelocity();
	for (int i = 0; i < numWheels; ++i)  assert (!isnan(wheel[WHEEL_POSITION(i)].GetAngularVelocity()));

	if (drive == RWD)
		return (whRL+whRR) * 0.5 * wheel[REAR_LEFT].GetRadius();
	else if (drive == AWD)
		return ( (whRL+whRR) * 0.5 * wheel[REAR_LEFT].GetRadius() +
		         (whFL+whFR) * 0.5 * wheel[FRONT_LEFT].GetRadius() ) *0.5;

	Dbl whRL2 = wheel[REAR2_LEFT].GetAngularVelocity();
	Dbl whRR2 = wheel[REAR2_RIGHT].GetAngularVelocity();
	if (drive == WD6)
		return ( (whRL2+whRR2) * 0.5 * wheel[REAR2_LEFT].GetRadius() +
		         (whRL+whRR) * 0.5 * wheel[REAR_LEFT].GetRadius() +
		         (whFL+whFR) * 0.5 * wheel[FRONT_LEFT].GetRadius() ) *0.33333333;

	Dbl whRL3 = wheel[REAR2_LEFT].GetAngularVelocity();
	Dbl whRR3 = wheel[REAR2_RIGHT].GetAngularVelocity();
	if (drive == WD8)
		return (((whRL3+whRR3) * 0.5 * wheel[REAR3_LEFT].GetRadius() +
		         (whRL2+whRR2) * 0.5 * wheel[REAR2_LEFT].GetRadius() ) *0.5+
		        ((whRL+whRR) * 0.5 * wheel[REAR_LEFT].GetRadius() +
		         (whFL+whFR) * 0.5 * wheel[FRONT_LEFT].GetRadius() ) *0.5) *0.5;

	assert(0);
	return 0;
}


//---------------------------------------------------------------------------------------------------------------------------
//  Gearbox, Clutch
//---------------------------------------------------------------------------------------------------------------------------

void CARDYNAMICS::ShiftGear(int value)  // Gift Share
{
	if (transmission.GetGear() != value && shifted)
	{
		if (value <= transmission.GetForwardGears() && value >= -transmission.GetReverseGears())
		{
			rem_shift_time = shift_time;
			shift_gear = value;
			shifted = false;
		}
	}
}

void CARDYNAMICS::UpdateTransmission(Dbl dt)
{
	driveshaft_rpm = CalculateDriveshaftRPM();

	if (vtype != V_Car)  // no gearbox
		ShiftGear(1);
	else
	if (autoshift)
	{
		bool dmg = fDamage >= 100.f;
		int gear = dmg ? 1 : NextGear();  /// <><> damage shift to N
		if (!dmg)
		{
			Dbl spm = GetSpeedMPS();  // opposite and N gear fix
			if (gear <= 0 && spm > 3)  gear = 1;  else
			if (gear >= 0 && spm <-3)  gear =-1;
			else		
			///  auto Rear gear
			if (autorear && shifted && fDamage < 100.f && rem_shift_time <= 0)
			{
				Dbl gas = engine.GetThrottle()*0.8;
				gas -= brake[0].GetBrakeFactor();
				Dbl g = gas;
				if (transmission.GetGear() == -1)  gas *= -1;
				const Dbl spdmarg = 2.0, spd = GetSpeed();
				if (g <-0.5 && spd < spdmarg && gear == 1)  gear =-1;  else
				if (g <-0.5 && spd < spdmarg && gear ==-1)  gear = 1;
		}	}
		ShiftGear(gear);
	}

	rem_shift_time -= dt;
	if (rem_shift_time < 0)  rem_shift_time = 0;

	if (rem_shift_time <= shift_time * 0.5 && !shifted)
	{
		shifted = true;
		transmission.Shift(shift_gear);
	}

	if (autoclutch)
	{
		if (!engine.GetCombustion())
		{
		    engine.StartEngine();
		    //std::cout << "start engine" << std::endl;
		}

		/*int inAir = 0;
		for (int w=0; w<4; ++w)
		{
			WHEEL_POSITION wp = WHEEL_POSITION(w);
			float d = GetWheelContact(wp).GetDepth() - 2*wheel[w].GetRadius();
			if (d > 0.f)  ++inAir;  // in air
		}/**/

		Dbl throttle = engine.GetThrottle();  
		//throttle = shifted ? ShiftAutoClutchThrottle(throttle, dt) : (inAir >= 2 ? 0.0 : 0.5);  //half full gas during shifting
		throttle = ShiftAutoClutchThrottle(throttle, dt);
		engine.SetThrottle(throttle);

		Dbl new_clutch = AutoClutch(last_auto_clutch, dt);
		clutch.SetClutch(new_clutch);
		last_auto_clutch = new_clutch;
	}
}


Dbl CARDYNAMICS::AutoClutch(Dbl last_clutch, Dbl dt) const
{
#if 1
	const Dbl threshold = 1000.0;
	const Dbl margin = 100.0;
	const Dbl geareffect = 1.0; //zero to 1, defines special consideration of first/reverse gear

	//take into account locked brakes
	bool willlock(true);
	for (int i = 0; i < numWheels; ++i)
	{
		if (WheelDriven(WHEEL_POSITION(i)))
            willlock = willlock && brake[i].WillLock();
	}
	if (willlock)  return 0;

	const Dbl rpm = engine.GetRPM();
	const Dbl maxrpm = engine.GetRpmMax();  //GetRPMLimit();
	const Dbl stallrpm = engine.GetStallRPM() + margin * (maxrpm / 2000.0);
	const int gear = transmission.GetGear();

	Dbl gearfactor = 1.0;
	if (gear <= 1)
		gearfactor = 2.0;
	Dbl thresh = threshold * (maxrpm/7000.0) * ((1.0-geareffect)+gearfactor*geareffect) + stallrpm;
	if (clutch.IsLocked())
		thresh *= 0.5;
	Dbl clutch = (rpm-stallrpm) / (thresh-stallrpm);

	//std::cout << rpm << ", " << stallrpm << ", " << threshold << ", " << clutch << std::endl;

	if (clutch < 0)		clutch = 0;
	if (clutch > 1.0)	clutch = 1.0;

	Dbl newauto = clutch * ShiftAutoClutch();

	//rate limit the autoclutch
	const Dbl min_engage_time = 0.05; //the fastest time in seconds for auto-clutch engagement
	const Dbl engage_rate_limit = 1.0/min_engage_time;
	const Dbl rate = (last_clutch - newauto)/dt; //engagement rate in clutch units per second
	if (rate > engage_rate_limit)
		newauto = last_clutch - engage_rate_limit*dt;

    return newauto;
#else
	btScalar rpm = engine.GetRPM();
	btScalar stallrpm = engine.GetStallRPM();
	btScalar clutchrpm = driveshaft_rpm; //clutch rpm on driveshaft/transmission side

	// clutch slip
	btScalar clutch = (5.0 * rpm + clutchrpm) / (9.0 * stallrpm) - 1.5;
	if (clutch < 0.0) clutch = 0.0;
	else if (clutch > 1.0) clutch = 1.0;

	// shift time
	clutch *= ShiftAutoClutch();

	// rate limit the autoclutch
	btScalar min_engage_time = 0.05;
	btScalar engage_limit = dt / min_engage_time;
	if (last_clutch - clutch > engage_limit)
	{
		clutch = last_clutch - engage_limit;
	}

	return clutch;
#endif
}


Dbl CARDYNAMICS::ShiftAutoClutch() const
{
	Dbl shift_clutch = 1.0;
	if (rem_shift_time > shift_time * 0.5)
	    shift_clutch = 0.0;
	else if (rem_shift_time > 0.0)
	    shift_clutch = 1.0 - rem_shift_time / (shift_time * 0.5);
	
	return shift_clutch;
}

Dbl CARDYNAMICS::ShiftAutoClutchThrottle(Dbl throttle, Dbl dt)
{
	if (rem_shift_time > 0.0)
	{
	    if (engine.GetRPM() < driveshaft_rpm && engine.GetRPM() < engine.GetRpmMax())
	    {
	        rem_shift_time += dt;
            return 1.0;
	    }else
	        return 0.5 * throttle;
	}
	return throttle;
}

/// return the gear change (0 for no change, -1 for shift down, 1 for shift up)
int CARDYNAMICS::NextGear() const
{
	int gear = transmission.GetGear();

	Dbl avg_slide = 0.;  float avg_whH = 0.f;
	for (int i=0; i < numWheels; ++i)
	{
		Dbl sl = fabs(wheel[i].slips.slide);
		avg_slide += sl;
		avg_whH += whH[i];
	}
	bool allow = true;
	float nw = 1.f / numWheels;
	avg_slide *= nw;  avg_whH *= nw;
	if (avg_slide > 1.0)  //par?
		allow = false;

	// only autoshift if a shift is not in progress
	if (allow)
	if (shifted /*&& remaining_shift_time < 0.01f*/ &&
        clutch.GetClutch() == 1.0)
    {
        // shift up when driveshaft speed exceeds engine redline
        if (driveshaft_rpm > engine.GetRpmMax() && gear > 0)
			// car vel < wheel vel	slip < 1.5 ?
            return gear + 1;

        // shift down when driveshaft speed below shift_down_point
        if (driveshaft_rpm < DownshiftRPM(gear, avg_whH) && gear > 1)
            return gear - 1;
    }
	return gear;
}

Dbl CARDYNAMICS::DownshiftRPM(int gear, float avg_whH) const
{
	Dbl shift_down_point = 0.0;
	if (gear > 1)
	{
        Dbl current_gear_ratio = transmission.GetGearRatio(gear);
        Dbl lower_gear_ratio = transmission.GetGearRatio(gear - 1);
		Dbl peak_engine_speed = engine.GetRpmMax();
		Dbl thr = engine.GetThrottle();
		shift_down_point = peak_engine_speed / lower_gear_ratio * current_gear_ratio
			//* (thr > 0.5 ? 0.6 : 0.9);  // par
			* (thr > 0.5 ? (avg_whH > 0.5f ?
						  0.4 :  // in mud, shift down only at very low rpm
						  0.7)   // throttle spin wheels, dont shift down much
						: 0.9);  // without throttle, shift down easily
	}
	return shift_down_point;
}


//  TCS, ABS
/// do traction control system (wheelspin prevention) calculations and modify the throttle position if necessary
//---------------------------------------------------------------------------------------------------------------------------
void CARDYNAMICS::DoTCS(int i, Dbl suspension_force)
{
	Dbl gasthresh = 0.1;
	Dbl gas = engine.GetThrottle();

	//only active if throttle commanded past threshold
	if ( gas > gasthresh )
	{
		//see if we're spinning faster than the rest of the wheels
		Dbl maxspindiff = 0;
		Dbl myrotationalspeed = wheel[WHEEL_POSITION(i)].GetAngularVelocity();
		for ( int i2 = 0; i2 < numWheels; ++i2)
		{
			Dbl spindiff = myrotationalspeed - wheel[WHEEL_POSITION(i2)].GetAngularVelocity();
			if (spindiff < 0)  spindiff = -spindiff;
			if (spindiff > maxspindiff)  maxspindiff = spindiff;
		}

		//don't engage if all wheels are moving at the same rate
		if (maxspindiff > 1.0)
		{
			//sp is the ideal slip ratio given tire loading
			Dbl sp(0), ah(0);
			GetTire(WHEEL_POSITION(i))->LookupSigmaHatAlphaHat( suspension_force, sp, ah );

			Dbl sense = 1.0;
			if (transmission.GetGear() < 0)
				sense = -1.0;

			Dbl error = wheel[WHEEL_POSITION(i)].slips.slide * sense - sp;
			Dbl thresholdeng = 0.0;
			Dbl thresholddis = -sp/2.0;

			if (error > thresholdeng && ! tcs_active[i])
				tcs_active[i] = true;

			if (error < thresholddis && tcs_active[i])
				tcs_active[i] = false;

			if (tcs_active[i])
			{
				Dbl curclutch = clutch.GetClutch();
				if (curclutch > 1)  curclutch = 1;
				if (curclutch < 0)  curclutch = 0;

				gas = gas - error * 10.0 * curclutch;
				if (gas < 0)  gas = 0;
				if (gas > 1)  gas = 1;
				engine.SetThrottle(gas);
			}
		}else
			tcs_active[i] = false;
	}else
		tcs_active[i] = false;
}

/// do anti-lock brake system calculations and modify the brake force if necessary
void CARDYNAMICS::DoABS(int i, Dbl suspension_force)
{
	Dbl braketresh = 0.1;
	Dbl brakesetting = brake[WHEEL_POSITION(i)].GetBrakeFactor();

	//only active if brakes commanded past threshold
	if (brakesetting > braketresh)
	{
		Dbl maxspeed = 0;
		for (int i2 = 0; i2 < numWheels; ++i2)
		{
			if (wheel[WHEEL_POSITION(i2)].GetAngularVelocity() > maxspeed)
				maxspeed = wheel[WHEEL_POSITION(i2)].GetAngularVelocity();
		}

		//don't engage ABS if all wheels are moving slowly
		if (maxspeed > 6.0)
		{
			//sp is the ideal slip ratio given tire loading
			Dbl sp(0), ah(0);
			GetTire(WHEEL_POSITION(i))->LookupSigmaHatAlphaHat( suspension_force, sp, ah );

			Dbl error = -wheel[WHEEL_POSITION(i)].slips.slide - sp;
			Dbl thresholdeng = 0.0;
			Dbl thresholddis = -sp/2.0;

			if (error > thresholdeng && ! abs_active[i])
				abs_active[i] = true;

			if (error < thresholddis && abs_active[i])
				abs_active[i] = false;
		}else
			abs_active[i] = false;
	}else
		abs_active[i] = false;

	if (abs_active[i])
		brake[WHEEL_POSITION(i)].SetBrakeFactor(0.0);
}
