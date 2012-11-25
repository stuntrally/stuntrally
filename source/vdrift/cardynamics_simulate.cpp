#include "pch.h"
#include "cardynamics.h"
#include "tobullet.h"
#include "../ogre/common/Defines.h"


//  set the steering angle to "value", where 1.0 is maximum right lock and -1.0 is maximum left lock.
void CARDYNAMICS::SetSteering(const Dbl value)
{
	steerValue = value;
	Dbl steerangle = value * maxangle;  //steering angle in degrees

	//ackermann stuff
	Dbl alpha = std::abs( steerangle * PI_d/180.0 );  //outside wheel steering angle in radians
	Dbl B = wheel[FRONT_LEFT].GetExtendedPosition()[1]
		  - wheel[FRONT_RIGHT].GetExtendedPosition()[1];  //distance between front wheels
	Dbl L = wheel[FRONT_LEFT].GetExtendedPosition()[0]
		  - wheel[REAR_LEFT].GetExtendedPosition()[0];  //distance between front and rear wheels
	Dbl beta = atan2(1.0, 1.0 / tan(alpha) - B/L );  //inside wheel steering angle in radians

	Dbl left = 0, right = 0;  // wheel angle

	if (value >= 0)	{	left = alpha;	right = beta;	}
	else			{	right = -alpha;	left = -beta;	}

	left *= 180.0/PI_d;  right *= 180.0/PI_d;

	wheel[FRONT_LEFT].SetSteerAngle( left );
	wheel[FRONT_RIGHT].SetSteerAngle( right );
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

	for(std::vector <CARAERO>::iterator i = aerodynamics.begin(); i != aerodynamics.end(); ++i)
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
	if (i == 0 || i == 2)	++otheri;
	else					--otheri;

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
	CARWHEEL & wheel = this->wheel[WHEEL_POSITION(i)];
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
	//Dbl friction_coeff = tire.GetTread() * surface.frictionTread + (1.0 - tire.GetTread()) * surface.frictionNonTread;
	Dbl friction_coeff = surface.frictionTread;
	//Dbl roll_friction_coeff = surface.rollResistanceCoefficient;
	MATHVECTOR<Dbl,3> friction_force(0);
	if (friction_coeff > 0)
		friction_force = tire->GetForce(
			normal_force, friction_coeff, //roll_friction_coeff,
			hub_velocity, patch_speed, camber_rad, &wheel.slips);

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
	//CARTIRE* tire = this->tire[WHEEL_POSITION(i)];
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
		brake.WillLock(false);
		wheel_torque += brake_torque;   // brake torque has same direction as lock up torque
	}
	else if (lock_up_torque < 0 && lock_up_torque < -brake_torque)
	{
		brake.WillLock(false);
		wheel_torque -= brake_torque;
	}else
	{
		brake.WillLock(true);
		wheel_torque = wheel.GetLockUpTorque(dt);
	}

	wheel.SetTorque(wheel_torque);
	wheel.Integrate2(dt);

	//  apply torque to body  -not wanted in jumps
	///+  z yaw  y pitch
	//MATHVECTOR<Dbl,3> world_wheel_torque(0, -wheel_torque, 0);
	//wheel_space.RotateVector(world_wheel_torque);
	//ApplyTorque(world_wheel_torque);
}


void CARDYNAMICS::InterpolateWheelContacts(Dbl dt)
{
	MATHVECTOR<float,3> raydir = GetDownVector();
	for (int i = 0; i < WHEEL_POSITION_SIZE; ++i)
	{
		MATHVECTOR<float,3> raystart = LocalToWorld(wheel[i].GetExtendedPosition());
		raystart = raystart - raydir * wheel[i].GetRadius();  //*!
		float raylen = 1;  //!par
		GetWheelContact(WHEEL_POSITION(i)).CastRay(raystart, raydir, raylen);
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
void CARDYNAMICS::CalculateDriveTorque(Dbl * wheel_drive_torque, Dbl clutch_torque)
{
	Dbl driveshaft_torque = transmission.GetTorque( clutch_torque );
	assert( !isnan(driveshaft_torque) );

	for (int i = 0; i < WHEEL_POSITION_SIZE; ++i)
		wheel_drive_torque[i] = 0;

	if (drive == RWD)
	{
		diff_rear.ComputeWheelTorques( driveshaft_torque );
		wheel_drive_torque[REAR_LEFT] = diff_rear.GetSide1Torque();
		wheel_drive_torque[REAR_RIGHT] = diff_rear.GetSide2Torque();
	}
	else if (drive == FWD)
	{
		diff_front.ComputeWheelTorques( driveshaft_torque );
		wheel_drive_torque[FRONT_LEFT] = diff_front.GetSide1Torque();
		wheel_drive_torque[FRONT_RIGHT] = diff_front.GetSide2Torque();
	}
	else if (drive == AWD)
	{
		diff_center.ComputeWheelTorques( driveshaft_torque );
		diff_front.ComputeWheelTorques( diff_center.GetSide1Torque() );
		diff_rear.ComputeWheelTorques( diff_center.GetSide2Torque() );
		wheel_drive_torque[FRONT_LEFT] = diff_front.GetSide1Torque();
		wheel_drive_torque[FRONT_RIGHT] = diff_front.GetSide2Torque();
		wheel_drive_torque[REAR_LEFT] = diff_rear.GetSide1Torque();
		wheel_drive_torque[REAR_RIGHT] = diff_rear.GetSide2Torque();
	}

	for (int i = 0; i < WHEEL_POSITION_SIZE; ++i)  assert(!isnan( wheel_drive_torque[WHEEL_POSITION(i)] ));
}

Dbl CARDYNAMICS::CalculateDriveshaftSpeed()
{
	Dbl driveshaft_speed = 0.0;
	Dbl whFL = wheel[FRONT_LEFT].GetAngularVelocity();
	Dbl whFR = wheel[FRONT_RIGHT].GetAngularVelocity();
	Dbl whRL = wheel[REAR_LEFT].GetAngularVelocity();
	Dbl whRR = wheel[REAR_RIGHT].GetAngularVelocity();
	for (int i = 0; i < 4; ++i)  assert(!isnan( wheel[WHEEL_POSITION(i)].GetAngularVelocity() ));

	if (drive == RWD)
		driveshaft_speed = diff_rear.CalculateDriveshaftSpeed( whRL, whRR );
	else if (drive == FWD)
		driveshaft_speed = diff_front.CalculateDriveshaftSpeed( whFL, whFR );
	else if (drive == AWD)
		driveshaft_speed = diff_center.CalculateDriveshaftSpeed(
		                   diff_front.CalculateDriveshaftSpeed( whFL, whFR ),
		                   diff_rear.CalculateDriveshaftSpeed( whRL, whRR ) );
	return driveshaft_speed;
}

Dbl CARDYNAMICS::CalculateDriveshaftRPM() const
{
	Dbl driveshaft_speed = 0.0;
	Dbl whFL = wheel[FRONT_LEFT].GetAngularVelocity();
	Dbl whFR = wheel[FRONT_RIGHT].GetAngularVelocity();
	Dbl whRL = wheel[REAR_LEFT].GetAngularVelocity();
	Dbl whRR = wheel[REAR_RIGHT].GetAngularVelocity();
	for (int i = 0; i < 4; ++i)  assert(!isnan( wheel[WHEEL_POSITION(i)].GetAngularVelocity() ));
	if (drive == RWD)
		driveshaft_speed = diff_rear.GetDriveshaftSpeed( whRL, whRR );
	else if (drive == FWD)
		driveshaft_speed = diff_front.GetDriveshaftSpeed( whFL, whFR );
	else if (drive == AWD)
	{
		Dbl front = diff_front.GetDriveshaftSpeed( whFL, whFR );
		Dbl rear = diff_rear.GetDriveshaftSpeed( whRL, whRR );
		driveshaft_speed = diff_center.GetDriveshaftSpeed( front, rear );
	}
	return transmission.GetClutchSpeed( driveshaft_speed ) * 30.0 / PI_d;
}

Dbl CARDYNAMICS::GetSpeedMPS() const
{
	Dbl whFL = wheel[FRONT_LEFT].GetAngularVelocity();
	Dbl whFR = wheel[FRONT_RIGHT].GetAngularVelocity();
	Dbl whRL = wheel[REAR_LEFT].GetAngularVelocity();
	Dbl whRR = wheel[REAR_RIGHT].GetAngularVelocity();
	for (int i = 0; i < 4; ++i)  assert (!isnan(wheel[WHEEL_POSITION(i)].GetAngularVelocity()));

	if (drive == RWD)
		return (whRL+whRR) * 0.5 * wheel[REAR_LEFT].GetRadius();
	else if (drive == FWD)
		return (whFL+whFR) * 0.5 * wheel[FRONT_LEFT].GetRadius();
	else if (drive == AWD)
		return ( (whRL+whRR) * 0.5 * wheel[REAR_LEFT].GetRadius() +
		         (whFL+whFR) * 0.5 * wheel[FRONT_LEFT].GetRadius() ) *0.5;
	assert(0);
	return 0;
}


//---------------------------------------------------------------------------------------------------------------------------
//  Gearbox, Clutch
//---------------------------------------------------------------------------------------------------------------------------

void CARDYNAMICS::ShiftGear(int value)
{
	if (transmission.GetGear() != value && shifted)
	{
		if (value <= transmission.GetForwardGears() && value >= -transmission.GetReverseGears())
		{
			remaining_shift_time = shift_time;
			shift_gear = value;
			shifted = false;
		}
	}
}

void CARDYNAMICS::UpdateTransmission(Dbl dt)
{
	driveshaft_rpm = CalculateDriveshaftRPM();

	if (autoshift)
	{
		int gear = NextGear();
		
		///  auto Rear gear
		if (autorear)
		{
			Dbl gas = engine.GetThrottle();
			gas -= brake[0].GetBrakeFactor();
			if (transmission.GetGear() == -1)  gas *= -1;
			const Dbl spdmarg = 2.0;
			if (gas <-0.2 && GetSpeedMPS() < spdmarg && gear == 1)  gear =-1;  else
			if (gas > 0.2 && GetSpeedMPS() >-spdmarg && gear ==-1)  gear = 1;
		}
		ShiftGear(gear);
	}

	remaining_shift_time -= dt;
	if (remaining_shift_time < 0) remaining_shift_time = 0;

	if (remaining_shift_time <= shift_time * 0.5 && !shifted)
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
	const Dbl threshold = 1000.0;
	const Dbl margin = 100.0;
	const Dbl geareffect = 1.0; //zero to 1, defines special consideration of first/reverse gear

	//take into account locked brakes
	bool willlock(true);
	for (int i = 0; i < WHEEL_POSITION_SIZE; i++)
	{
		if (WheelDriven(WHEEL_POSITION(i)))
            willlock = willlock && brake[i].WillLock();
	}
	if (willlock)  return 0;

	const Dbl rpm = engine.GetRPM();
	const Dbl maxrpm = engine.GetRPMLimit();
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
}


Dbl CARDYNAMICS::ShiftAutoClutch() const
{
	Dbl shift_clutch = 1.0;
	if (remaining_shift_time > shift_time * 0.5)
	    shift_clutch = 0.0;
	else if (remaining_shift_time > 0.0)
	    shift_clutch = 1.0 - remaining_shift_time / (shift_time * 0.5);
	return shift_clutch;
}

Dbl CARDYNAMICS::ShiftAutoClutchThrottle(Dbl throttle, Dbl dt)
{
	if (remaining_shift_time > 0.0)
	{
	    if (engine.GetRPM() < driveshaft_rpm && engine.GetRPM() < engine.GetRedline())
	    {
	        remaining_shift_time += dt;
            return 1.0;
	    }
	    else
	    {
	        return 0.5 * throttle;
	    }
	}
	return throttle;
}

/// return the gear change (0 for no change, -1 for shift down, 1 for shift up)
int CARDYNAMICS::NextGear() const
{
	int gear = transmission.GetGear();

	// only autoshift if a shift is not in progress
	if (shifted /*&& remaining_shift_time < 0.01f*/)
	{
        if (clutch.GetClutch() == 1.0)
        {
            // shift up when driveshaft speed exceeds engine redline
            // we do not shift up from neutral/reverse
            if (driveshaft_rpm > engine.GetRedline() && gear > 0)
            {
                return gear + 1;
            }
            // shift down when driveshaft speed below shift_down_point
            // we do not auto shift down from 1st gear to neutral
            if (driveshaft_rpm < DownshiftRPM(gear) && gear > 1)
            {
                return gear - 1;
            }
        }
    }
	return gear;
}

Dbl CARDYNAMICS::DownshiftRPM(int gear) const
{
	Dbl shift_down_point = 0.0;
	if (gear > 1)
	{
        Dbl current_gear_ratio = transmission.GetGearRatio(gear);
        Dbl lower_gear_ratio = transmission.GetGearRatio(gear - 1);
		Dbl peak_engine_speed = engine.GetRedline();
		shift_down_point = 0.9 * peak_engine_speed / lower_gear_ratio * current_gear_ratio;
	}					  // 0.5 def-
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
		for ( int i2 = 0; i2 < WHEEL_POSITION_SIZE; ++i2)
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
		for (int i2 = 0; i2 < WHEEL_POSITION_SIZE; ++i2)
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
