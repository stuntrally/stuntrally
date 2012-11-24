#include "pch.h"
#include "cardynamics.h"
#include "tobullet.h"
#include "../ogre/common/Defines.h"


void CARDYNAMICS::debugDraw(btIDebugDraw* debugDrawer)
{	}

const MATHVECTOR <Dbl, 3> & CARDYNAMICS::GetCenterOfMassPosition() const
{
	return chassisCenterOfMass;
}

const MATHVECTOR <Dbl, 3> & CARDYNAMICS::GetPosition() const
{
	return chassisPosition;
}

const QUATERNION <Dbl> & CARDYNAMICS::GetOrientation() const
{
	return chassisRotation;
}

MATHVECTOR <Dbl, 3> CARDYNAMICS::GetWheelPosition(WHEEL_POSITION wp) const
{
	MATHVECTOR <Dbl, 3> pos = GetLocalWheelPosition(wp, suspension[wp].GetDisplacementPercent());
	chassisRotation.RotateVector(pos);
	return pos + chassisPosition;
}

MATHVECTOR <Dbl, 3> CARDYNAMICS::GetWheelPosition(WHEEL_POSITION wp, Dbl displacement_percent) const
{
	MATHVECTOR <Dbl, 3> pos = GetLocalWheelPosition(wp, displacement_percent);
	chassisRotation.RotateVector(pos);
	return pos + chassisPosition;
}

QUATERNION <Dbl> CARDYNAMICS::GetWheelOrientation(WHEEL_POSITION wp) const
{
	QUATERNION <Dbl> siderot;
	if(wp == FRONT_RIGHT || wp == REAR_RIGHT)
	{
		siderot.Rotate(PI_d, 0, 0, 1);
	}
	return chassisRotation * GetWheelSteeringAndSuspensionOrientation(wp) * wheel[wp].GetOrientation() * siderot;
}

QUATERNION <Dbl> CARDYNAMICS::GetUprightOrientation(WHEEL_POSITION wp) const
{
	return chassisRotation * GetWheelSteeringAndSuspensionOrientation(wp);
}

/// worldspace wheel center position
MATHVECTOR <Dbl, 3> CARDYNAMICS::GetWheelVelocity ( WHEEL_POSITION wp ) const
{
	return wheel_velocity[wp];
}

const COLLISION_CONTACT & CARDYNAMICS::GetWheelContact(WHEEL_POSITION wp) const
{
	return wheel_contact[wp];
}

COLLISION_CONTACT & CARDYNAMICS::GetWheelContact(WHEEL_POSITION wp)
{
	return wheel_contact[wp];
}

float CARDYNAMICS::GetMass() const
{
	return body.GetMass();
}

Dbl CARDYNAMICS::GetSpeed() const
{
	return body.GetVelocity().Magnitude();
	//return chassis->getLinearVelocity().length();
}

MATHVECTOR <Dbl, 3> CARDYNAMICS::GetVelocity() const
{
	return body.GetVelocity();
	//return ToMathVector<Dbl>(chassis->getLinearVelocity());
}

MATHVECTOR <Dbl, 3> CARDYNAMICS::GetAngularVelocity() const
{
	return body.GetAngularVelocity();
	//return ToMathVector<Dbl>(chassis->getAngularVelocity());
}

MATHVECTOR <Dbl, 3> CARDYNAMICS::GetEnginePosition() const
{
	MATHVECTOR <Dbl, 3> offset = engine.GetPosition();
	Orientation().RotateVector(offset);
	return offset + chassisPosition;
}

void CARDYNAMICS::StartEngine()
{
	engine.StartEngine();
}

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

void CARDYNAMICS::SetThrottle(float value)
{
	engine.SetThrottle(value);
}

void CARDYNAMICS::SetClutch(float value)
{
	clutch.SetClutch(value);
}

void CARDYNAMICS::SetBrake(float value)
{
	for(unsigned int i = 0; i < brake.size(); i++)
	{
		brake[i].SetBrakeFactor(value);
	}
}

void CARDYNAMICS::SetHandBrake(float value)
{
	for(unsigned int i = 0; i < brake.size(); i++)
	{
		brake[i].SetHandbrakeFactor(value);
	}
}

void CARDYNAMICS::SetAutoClutch(bool value)
{
	autoclutch = value;
}

void CARDYNAMICS::SetAutoShift(bool value)
{
	autoshift = value;
}

void CARDYNAMICS::SetAutoRear(bool value)
{
	autorear = value;
}

Dbl CARDYNAMICS::GetSpeedMPS() const
{
	Dbl left_front_wheel_speed = wheel[FRONT_LEFT].GetAngularVelocity();
	Dbl right_front_wheel_speed = wheel[FRONT_RIGHT].GetAngularVelocity();
	Dbl left_rear_wheel_speed = wheel[REAR_LEFT].GetAngularVelocity();
	Dbl right_rear_wheel_speed = wheel[REAR_RIGHT].GetAngularVelocity();
	for ( int i = 0; i < 4; i++ ) assert ( !isnan ( wheel[WHEEL_POSITION ( i ) ].GetAngularVelocity() ) );
	if ( drive == RWD )
	{
		return ( left_rear_wheel_speed+right_rear_wheel_speed ) * 0.5 * wheel[REAR_LEFT].GetRadius();
	}
	else if ( drive == FWD )
	{
		return ( left_front_wheel_speed+right_front_wheel_speed ) * 0.5 * wheel[FRONT_LEFT].GetRadius();
	}
	else if ( drive == AWD )
	{
		return ( ( left_rear_wheel_speed+right_rear_wheel_speed ) * 0.5 * wheel[REAR_LEFT].GetRadius() +
		         ( left_front_wheel_speed+right_front_wheel_speed ) * 0.5 * wheel[FRONT_LEFT].GetRadius() ) *0.5;
	}

	assert ( 0 );
	return 0;
}

Dbl CARDYNAMICS::GetTachoRPM() const
{
	return tacho_rpm;
}

void CARDYNAMICS::SetABS ( const bool newabs )
{
	abs = newabs;
}

bool CARDYNAMICS::GetABSEnabled() const
{
	return abs;
}

bool CARDYNAMICS::GetABSActive() const
{
	return abs && ( abs_active[0]||abs_active[1]||abs_active[2]||abs_active[3] );
}

void CARDYNAMICS::SetTCS ( const bool newtcs )
{
	tcs = newtcs;
}

bool CARDYNAMICS::GetTCSEnabled() const
{
	return tcs;
}

bool CARDYNAMICS::GetTCSActive() const
{
	return tcs && ( tcs_active[0]||tcs_active[1]||tcs_active[2]||tcs_active[3] );
}

void CARDYNAMICS::SetPosition(const MATHVECTOR<Dbl, 3> & position)
{
	body.SetPosition(position);
	if (chassis)
		chassis->translate(ToBulletVector(position) - chassis->getCenterOfMassPosition());
}

//find the precise starting position for the car (trim out the extra space)
void CARDYNAMICS::AlignWithGround()
{
	UpdateWheelTransform();
	UpdateWheelContacts();
	
	/*Dbl min_height = 0;
	bool no_min_height = true;
	for (int i = 0; i < WHEEL_POSITION_SIZE; i++)
		{
		Dbl height = wheel_contact[i].GetDepth() - 2 * tire[i].GetRadius();
		if (height < min_height || no_min_height)
			{
			min_height = height;
			no_min_height = false;
			}
	}/**/  //--
	
	//MATHVECTOR <Dbl, 3> trimmed_position = Position() + GetDownVector() * min_height;
	//SetPosition(Position()/*trimmed_position*/);
}

//TODO: adjustable ackermann-like parameters
///set the steering angle to "value", where 1.0 is maximum right lock and -1.0 is maximum left lock.
void CARDYNAMICS::SetSteering ( const Dbl value )
{
	steerValue = value;
	Dbl steerangle = value * maxangle; //steering angle in degrees

	//ackermann stuff
	Dbl alpha = std::abs ( steerangle * PI_d/180.0 ); //outside wheel steering angle in radians
	Dbl B = wheel[FRONT_LEFT].GetExtendedPosition() [1]
		- wheel[FRONT_RIGHT].GetExtendedPosition() [1]; //distance between front wheels
	Dbl L = wheel[FRONT_LEFT].GetExtendedPosition() [0]
		- wheel[REAR_LEFT].GetExtendedPosition() [0]; //distance between front and rear wheels
	Dbl beta = atan2 ( 1.0, ( ( 1.0/ ( tan ( alpha ) ) )-B/L ) ); //inside wheel steering angle in radians

	Dbl left_wheel_angle = 0;
	Dbl right_wheel_angle = 0;

	if ( value >= 0 )
	{
		left_wheel_angle = alpha;
		right_wheel_angle = beta;
	}
	else
	{
		right_wheel_angle = -alpha;
		left_wheel_angle = -beta;
	}

	left_wheel_angle *= 180.0/PI_d;
	right_wheel_angle *= 180.0/PI_d;

	wheel[FRONT_LEFT].SetSteerAngle ( left_wheel_angle );
	wheel[FRONT_RIGHT].SetSteerAngle ( right_wheel_angle );
}

Dbl CARDYNAMICS::GetMaxSteeringAngle() const
{
	return maxangle;
}

MATHVECTOR <Dbl, 3> CARDYNAMICS::GetTotalAero() const
{
	MATHVECTOR <Dbl, 3> downforce = 0;
	for ( std::vector <CARAERO>::const_iterator i = aerodynamics.begin(); i != aerodynamics.end(); ++i )
	{
		downforce = downforce + i->GetLiftVector() +  i->GetDragVector();
	}
	return downforce;
}

Dbl CARDYNAMICS::GetAerodynamicDownforceCoefficient() const
{
	Dbl coeff = 0.0;
	for ( std::vector <CARAERO>::const_iterator i = aerodynamics.begin(); i != aerodynamics.end(); ++i )
		coeff += i->GetAerodynamicDownforceCoefficient();
	return coeff;
}

Dbl CARDYNAMICS::GetAeordynamicDragCoefficient() const
{
	Dbl coeff = 0.0;
	for ( std::vector <CARAERO>::const_iterator i = aerodynamics.begin(); i != aerodynamics.end(); ++i )
		coeff += i->GetAeordynamicDragCoefficient();
	return coeff;
}

MATHVECTOR< Dbl, 3 > CARDYNAMICS::GetLastBodyForce() const
{
	return lastbodyforce;
}

Dbl CARDYNAMICS::GetFeedback() const
{
	return feedback;
}

void CARDYNAMICS::UpdateTelemetry ( float dt )
{
	//telemetry.Update ( dt );
}

bool CARDYNAMICS::Serialize ( joeserialize::Serializer & s )
{
	_SERIALIZE_ ( s,body );
	_SERIALIZE_ ( s,engine );
	_SERIALIZE_ ( s,clutch );
	_SERIALIZE_ ( s,transmission );
	_SERIALIZE_ ( s,diff_front );
	_SERIALIZE_ ( s,diff_rear );
	_SERIALIZE_ ( s,diff_center );
	_SERIALIZE_ ( s,fuel_tank );
	_SERIALIZE_ ( s,suspension );
	_SERIALIZE_ ( s,wheel );
	_SERIALIZE_ ( s,brake );
	_SERIALIZE_ ( s,tire );
	_SERIALIZE_ ( s,aerodynamics );
	_SERIALIZE_ ( s,wheel_velocity );
	_SERIALIZE_ ( s,abs );
	_SERIALIZE_ ( s,abs_active );
	_SERIALIZE_ ( s,tcs );
	_SERIALIZE_ ( s,tcs_active );
	_SERIALIZE_ (s,last_auto_clutch);
	_SERIALIZE_ (s,remaining_shift_time);
	_SERIALIZE_ (s,shift_gear);
	_SERIALIZE_ (s,shifted);
	_SERIALIZE_ (s,autoshift);
	_SERIALIZE_ (s,autorear);
	return true;
}

MATHVECTOR <Dbl, 3> CARDYNAMICS::GetDownVector() const
{
	MATHVECTOR <Dbl, 3> v(0, 0, -1);
	Orientation().RotateVector(v);
	return v;
}

QUATERNION <Dbl> CARDYNAMICS::Orientation() const
{
	return body.GetOrientation();
	//return ToMathQuaternion<Dbl>(chassis->getOrientation());
}

MATHVECTOR <Dbl, 3> CARDYNAMICS::Position() const
{
	return body.GetPosition();
	//return ToMathVector<Dbl>(chassis->getCenterOfMassPosition());
}

MATHVECTOR <Dbl, 3> CARDYNAMICS::LocalToWorld(const MATHVECTOR <Dbl, 3> & local) const
{
	MATHVECTOR <Dbl,3> position = local - center_of_mass;
	body.GetOrientation().RotateVector(position);
	return position + body.GetPosition();
	//btVector3 position = chassis->getCenterOfMassTransform().getBasis() * ToBulletVector(local - center_of_mass);
	//position = position + chassis->getCenterOfMassTransform().getOrigin();
	//return ToMathVector <Dbl> (position);
}

//simple hinge (arc) suspension displacement
MATHVECTOR <Dbl, 3> CARDYNAMICS::GetLocalWheelPosition(WHEEL_POSITION wp, Dbl displacement_percent) const
{
	//const
	const MATHVECTOR <Dbl, 3> & wheelext = wheel[wp].GetExtendedPosition();
	const MATHVECTOR <Dbl, 3> & hinge = suspension[wp].GetHinge();
	MATHVECTOR <Dbl, 3> relwheelext = wheelext - hinge;
	MATHVECTOR <Dbl, 3> up (0, 0, 1);
	MATHVECTOR <Dbl, 3> rotaxis = up.cross ( relwheelext.Normalize() );
	Dbl hingeradius = relwheelext.Magnitude();
	Dbl travel = suspension[wp].GetTravel();
	//const

	Dbl displacement = displacement_percent * travel;
	Dbl displacementradians = displacement / hingeradius;
	QUATERNION <Dbl> hingerotate;
	hingerotate.Rotate ( -displacementradians, rotaxis[0], rotaxis[1], rotaxis[2] );
	MATHVECTOR <Dbl, 3> localwheelpos = relwheelext;
	hingerotate.RotateVector ( localwheelpos );
	return localwheelpos + hinge;
}

///returns the orientation of the wheel due only to steering and suspension
QUATERNION <Dbl> CARDYNAMICS::GetWheelSteeringAndSuspensionOrientation ( WHEEL_POSITION wp ) const
{
	QUATERNION <Dbl> steer;
	steer.Rotate ( -wheel[wp].GetSteerAngle() * PI_d/180.0, 0, 0, 1 );

	QUATERNION <Dbl> camber;
	Dbl camber_rotation = -suspension[wp].GetCamber() * PI_d/180.0;
	if ( wp == 1 || wp == 3 )
		camber_rotation = -camber_rotation;
	camber.Rotate ( camber_rotation, 1, 0, 0 );

	QUATERNION <Dbl> toe;
	Dbl toe_rotation = suspension[wp].GetToe() * PI_d/180.0;
	if ( wp == 0 || wp == 2 )
		toe_rotation = -toe_rotation;
	toe.Rotate ( toe_rotation, 0, 0, 1 );

	return camber * toe * steer;
}

/// worldspace position of the center of the wheel when the suspension is compressed
/// by the displacement_percent where 1.0 is fully compressed
MATHVECTOR <Dbl, 3> CARDYNAMICS::GetWheelPositionAtDisplacement(WHEEL_POSITION wp, Dbl displacement_percent) const
{
	return LocalToWorld(GetLocalWheelPosition(wp, displacement_percent));
}

void CARDYNAMICS::ApplyForce(const MATHVECTOR <Dbl, 3> & force)
{
	body.ApplyForce(force);
	//chassis->applyCentralForce(ToBulletVector(force));
}

void CARDYNAMICS::ApplyForce(const MATHVECTOR <Dbl, 3> & force, const MATHVECTOR <Dbl, 3> & offset)
{
	body.ApplyForce(force, offset);
	//chassis->applyForce(ToBulletVector(force), ToBulletVector(offset));
}

void CARDYNAMICS::ApplyTorque(const MATHVECTOR <Dbl, 3> & torque)
{
	body.ApplyTorque(torque);
	//if(torque.MagnitudeSquared() > 1E-6)
	//	chassis->applyTorque(ToBulletVector(torque));
}

void CARDYNAMICS::UpdateWheelVelocity()
{
	for(int i = 0; i < WHEEL_POSITION_SIZE; i++)
	{
		wheel_velocity[i] = body.GetVelocity(wheel_position[i] - body.GetPosition());
		//btVector3 offset = ToBulletVector(wheel_position[i]) - chassis->getCenterOfMassPosition();
		//wheel_velocity[i] = ToMathVector<Dbl>(chassis->getVelocityInLocalPoint(offset));
	}
}

void CARDYNAMICS::UpdateWheelTransform()
{
	for(int i = 0; i < WHEEL_POSITION_SIZE; i++)
	{
		wheel_position[i] = GetWheelPositionAtDisplacement(WHEEL_POSITION(i), suspension[i].GetDisplacementPercent());
		wheel_orientation[i] = Orientation() * GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION(i));
	}
}

void CARDYNAMICS::ApplyEngineTorqueToBody()
{
	MATHVECTOR <Dbl, 3> engine_torque(-engine.GetTorque(), 0, 0);
	Orientation().RotateVector(engine_torque);
	ApplyTorque(engine_torque*0.1);  // unwanted in jumps
}

void CARDYNAMICS::ApplyAerodynamicsToBody(Dbl dt)
{
	MATHVECTOR <Dbl, 3> wind_force(0);
	MATHVECTOR <Dbl, 3> wind_torque(0);
	MATHVECTOR <Dbl, 3> air_velocity = -GetVelocity();
	(-Orientation()).RotateVector(air_velocity);
	for(std::vector <CARAERO>::iterator i = aerodynamics.begin(); i != aerodynamics.end(); ++i)
	{
		MATHVECTOR <Dbl, 3> force = i->GetForce(air_velocity);
		wind_force = wind_force + force;
		wind_torque = wind_torque + (i->GetPosition() - center_of_mass).cross(force);
	}
	Orientation().RotateVector(wind_force);
	Orientation().RotateVector(wind_torque);
	ApplyForce(wind_force);
	ApplyTorque(wind_torque);

	// rotational damping/drag
	if (rot_coef[0] > 0.0 && chassis)
	{
		MATHVECTOR <Dbl, 3> rot_drag = -ToMathVector<Dbl>(chassis->getAngularVelocity());
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

MATHVECTOR <Dbl, 3> CARDYNAMICS::UpdateSuspension ( int i , Dbl dt )
{
	// displacement
	const Dbl posx = wheel_contact[i].GetPosition() [0];
	const Dbl posz = wheel_contact[i].GetPosition() [2];
	const TRACKSURFACE & surface = wheel_contact[i].GetSurface();
	Dbl phase = 0;
	if (surface.bumpWaveLength > 0.0001)
		phase = 2 * PI_d * ( posx+posz ) / surface.bumpWaveLength;
	Dbl shift = 2.0 * sin ( phase*1.414214 );
	Dbl amplitude = 0.25 * surface.bumpAmplitude;
	Dbl bumpoffset = amplitude * ( sin ( phase + shift ) + sin ( 1.414214*phase ) - 2.0 );
	//Dbl ray_offset = 0.2;  //*! wheel ray origin is offset by 1 meter relative to wheel extended position
	Dbl displacement = /*ray_offset +*/ 2.0*wheel[i].GetRadius() - wheel_contact[i].GetDepth() + bumpoffset;

	// compute suspension force
	Dbl springdampforce = suspension[WHEEL_POSITION ( i ) ].Update ( dt , displacement );

	//do anti-roll
	int otheri = i;
	if ( i == 0 || i == 2 )
		otheri++;
	else
		otheri--;
	Dbl antirollforce = suspension[WHEEL_POSITION ( i ) ].GetAntiRollK() *
	                  ( suspension[WHEEL_POSITION ( i ) ].GetDisplacement()-
	                    suspension[WHEEL_POSITION ( otheri ) ].GetDisplacement() );
	//suspension[WHEEL_POSITION(i)].SetAntiRollInfo(antirollforce);
	if (isnan ( antirollforce ))  antirollforce = 0.f;//crash dyn obj
	assert ( !isnan ( antirollforce ) );

	//find the vector direction to apply the suspension force
#ifdef SUSPENSION_FORCE_DIRECTION
	const MATHVECTOR <Dbl, 3> & wheelext = wheel[i].GetExtendedPosition();
	const MATHVECTOR <Dbl, 3> & hinge = suspension[i].GetHinge();
	MATHVECTOR <Dbl, 3> relwheelext = wheelext - hinge;
	MATHVECTOR <Dbl, 3> up ( 0,0,1 );
	MATHVECTOR <Dbl, 3> rotaxis = up.cross ( relwheelext.Normalize() );
	MATHVECTOR <Dbl, 3> forcedirection = relwheelext.Normalize().cross ( rotaxis );
	//std::cout << i << ". " << forcedirection << std::endl;
	MATHVECTOR <Dbl, 3> suspension_force = forcedirection * ( antirollforce+springdampforce );
#else
	MATHVECTOR <Dbl, 3> suspension_force(0, 0, antirollforce + springdampforce);
#endif
	Orientation().RotateVector(suspension_force);
	return suspension_force;
}


// aplies tire friction  to car, returns friction in world space
MATHVECTOR <Dbl, 3> CARDYNAMICS::ApplyTireForce(int i, const Dbl normal_force, const QUATERNION <Dbl> & wheel_space)
{
	CARWHEEL & wheel = this->wheel[WHEEL_POSITION(i)];
	CARTIRE & tire = this->tire[WHEEL_POSITION(i)];
	const COLLISION_CONTACT & wheel_contact = this->wheel_contact[WHEEL_POSITION(i)];
	const TRACKSURFACE & surface = wheel_contact.GetSurface();
	const MATHVECTOR <Dbl, 3> surface_normal = wheel_contact.GetNormal();

	// camber relative to surface(clockwise in wheel heading direction)
	MATHVECTOR <Dbl, 3> wheel_axis(0, 1, 0);
	wheel_space.RotateVector(wheel_axis); // wheel axis in world space (wheel plane normal)
	Dbl camber_sin = wheel_axis.dot(surface_normal);
	Dbl camber_rad = asin(camber_sin);
	wheel.SetCamberDeg(camber_rad * 180.0/PI_d);

	// tire space(SAE Tire Coordinate System)
	// surface normal is z-axis
	// wheel axis projected on surface plane is y-axis
	MATHVECTOR <Dbl, 3> y_axis = wheel_axis - surface_normal * camber_sin;
	MATHVECTOR <Dbl, 3> x_axis = y_axis.cross(surface_normal);

	// wheel center velocity in tire space
	MATHVECTOR <Dbl, 3> hub_velocity;
	hub_velocity[0] = x_axis.dot(wheel_velocity[WHEEL_POSITION(i)]);
	hub_velocity[1] = y_axis.dot(wheel_velocity[WHEEL_POSITION(i)]);
	hub_velocity[2] = 0; // unused

	// rearward speed of the contact patch
	Dbl patch_speed = wheel.GetAngularVelocity() * wheel.GetRadius();

	// friction force in tire space
	//Dbl friction_coeff = tire.GetTread() * surface.frictionTread + (1.0 - tire.GetTread()) * surface.frictionNonTread;
	Dbl friction_coeff = surface.frictionTread;
	Dbl roll_friction_coeff = surface.rollResistanceCoefficient;
	MATHVECTOR <Dbl, 3> friction_force(0);
	if(friction_coeff > 0)
		friction_force = tire.GetForce(normal_force, friction_coeff, roll_friction_coeff, hub_velocity, patch_speed, camber_rad);

	// set force feedback (aligning torque in tire space)
	tire.SetFeedback(friction_force[2]);

	// friction force in world space
	MATHVECTOR <Dbl, 3> world_friction_force = x_axis * friction_force[0] + y_axis * friction_force[1];

	// fake viscous friction (sand, gravel, mud)
	MATHVECTOR <Dbl, 3> wheel_drag = - (x_axis * hub_velocity[0] + y_axis * hub_velocity[1]) * surface.rollingDrag;

	// apply forces to body
	MATHVECTOR <Dbl, 3> wheel_normal(0, 0, 1);
	wheel_space.RotateVector(wheel_normal);
	MATHVECTOR <Dbl, 3> contactpos = wheel_position[WHEEL_POSITION(i)] + wheel_normal * wheel.GetRadius() * wheel.GetRollHeight();  ///+
	ApplyForce(world_friction_force + surface_normal * normal_force + wheel_drag, contactpos - Position());

	return world_friction_force;
}

void CARDYNAMICS::ApplyWheelTorque(Dbl dt, Dbl drive_torque, int i, MATHVECTOR <Dbl, 3> tire_friction, const QUATERNION <Dbl> & wheel_space)
{
	CARWHEEL & wheel = this->wheel[WHEEL_POSITION(i)];
	CARTIRE & tire = this->tire[WHEEL_POSITION(i)];
	CARBRAKE & brake = this->brake[WHEEL_POSITION(i)];

	// tire force / torque
	wheel.Integrate1(dt);

	(-wheel_space).RotateVector(tire_friction);

	// torques acting on wheel
	Dbl friction_torque = tire_friction[0] * wheel.GetRadius();
	Dbl wheel_torque = drive_torque - friction_torque;
	Dbl lock_up_torque = wheel.GetLockUpTorque(dt) - wheel_torque;	// torque needed to lock the wheel
	Dbl angVel = wheel.GetAngularVelocity();  if (angVel < 0.0)  angVel = -angVel; //
	Dbl brake_torque = brake.GetTorque()
		+ wheel.fluidRes * angVel;  /// fluid resistance

	// brake and rolling resistance torque should never exceed lock up torque
	if(lock_up_torque >= 0 && lock_up_torque > brake_torque)
	{
		brake.WillLock(false);
		wheel_torque += brake_torque;   // brake torque has same direction as lock up torque
	}
	else if(lock_up_torque < 0 && lock_up_torque < -brake_torque)
	{
		brake.WillLock(false);
		wheel_torque -= brake_torque;
	}
	else
	{
		brake.WillLock(true);
		wheel_torque = wheel.GetLockUpTorque(dt);
	}

	wheel.SetTorque(wheel_torque);
	wheel.Integrate2(dt);

	// apply torque to body
	///+  z yaw  y pitch
	//MATHVECTOR <Dbl, 3> world_wheel_torque(0, -wheel_torque, 0);
	//wheel_space.RotateVector(world_wheel_torque);
	//ApplyTorque(world_wheel_torque);
}


void CARDYNAMICS::InterpolateWheelContacts(Dbl dt)
{
	MATHVECTOR <float, 3> raydir = GetDownVector();
	for (int i = 0; i < WHEEL_POSITION_SIZE; i++)
	{
		MATHVECTOR <float, 3> raystart = LocalToWorld(wheel[i].GetExtendedPosition());
		raystart = raystart - raydir * wheel[i].GetRadius();  //*!
		float raylen = 1;  //!par
		GetWheelContact(WHEEL_POSITION(i)).CastRay(raystart, raydir, raylen);
	}
}

///calculate the center of mass, calculate the total mass of the body, calculate the inertia tensor
/// then store this information in the rigid body
void CARDYNAMICS::UpdateMass()
{
	typedef std::pair <Dbl, MATHVECTOR <Dbl, 3> > MASS_PAIR;

	Dbl total_mass ( 0 );

	center_of_mass.Set ( 0,0,0 );

	//calculate the total mass, and center of mass
	for ( std::list <MASS_PAIR>::iterator i = mass_only_particles.begin(); i != mass_only_particles.end(); ++i )
	{
		//add the current mass to the total mass
		total_mass += i->first;

		//incorporate the current mass into the center of mass
		center_of_mass = center_of_mass + i->second * i->first;
	}

	//account for fuel
	total_mass += fuel_tank.GetMass();
	center_of_mass =  center_of_mass + fuel_tank.GetPosition() * fuel_tank.GetMass();

	body.SetMass ( total_mass );

	center_of_mass = center_of_mass * ( 1.0 / total_mass );

	//calculate the inertia tensor
	MATRIX3 <Dbl> inertia;
	for ( int i = 0; i < 9; i++ )
		inertia[i] = 0;
	for ( std::list <MASS_PAIR>::iterator i = mass_only_particles.begin(); i != mass_only_particles.end(); ++i )
	{
		//transform into the rigid body coordinates
		MATHVECTOR <Dbl, 3> position = i->second - center_of_mass;
		Dbl mass = i->first;

		//add the current mass to the inertia tensor
		inertia[0] += mass * ( position[1] * position[1] + position[2] * position[2] );
		inertia[1] -= mass * ( position[0] * position[1] );
		inertia[2] -= mass * ( position[0] * position[2] );
		inertia[3] = inertia[1];
		inertia[4] += mass * ( position[2] * position[2] + position[0] * position[0] );
		inertia[5] -= mass * ( position[1] * position[2] );
		inertia[6] = inertia[2];
		inertia[7] = inertia[5];
		inertia[8] += mass * ( position[0] * position[0] + position[1] * position[1] );
	}
	//inertia.DebugPrint(std::cout);
	body.SetInertia ( inertia );
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

///apply forces on the engine due to drag from the clutch
void CARDYNAMICS::ApplyClutchTorque ( Dbl engine_drag, Dbl clutch_speed )
{
	if ( transmission.GetGear() == 0 )
	{
		engine.SetClutchTorque ( 0.0 );
	}
	else
	{
		engine.SetClutchTorque ( engine_drag );
	}
}

/// calculate the drive torque that the engine applies to each wheel,
/// and put the output into the supplied 4-element array
void CARDYNAMICS::CalculateDriveTorque ( Dbl * wheel_drive_torque, Dbl clutch_torque )
{
	Dbl driveshaft_torque = transmission.GetTorque ( clutch_torque );
	assert ( !isnan ( driveshaft_torque ) );

	for ( int i = 0; i < WHEEL_POSITION_SIZE; i++ )
		wheel_drive_torque[i] = 0;

	if ( drive == RWD )
	{
		diff_rear.ComputeWheelTorques ( driveshaft_torque );
		wheel_drive_torque[REAR_LEFT] = diff_rear.GetSide1Torque();
		wheel_drive_torque[REAR_RIGHT] = diff_rear.GetSide2Torque();
	}
	else if ( drive == FWD )
	{
		diff_front.ComputeWheelTorques ( driveshaft_torque );
		wheel_drive_torque[FRONT_LEFT] = diff_front.GetSide1Torque();
		wheel_drive_torque[FRONT_RIGHT] = diff_front.GetSide2Torque();
	}
	else if ( drive == AWD )
	{
		diff_center.ComputeWheelTorques ( driveshaft_torque );
		diff_front.ComputeWheelTorques ( diff_center.GetSide1Torque() );
		diff_rear.ComputeWheelTorques ( diff_center.GetSide2Torque() );
		wheel_drive_torque[FRONT_LEFT] = diff_front.GetSide1Torque();
		wheel_drive_torque[FRONT_RIGHT] = diff_front.GetSide2Torque();
		wheel_drive_torque[REAR_LEFT] = diff_rear.GetSide1Torque();
		wheel_drive_torque[REAR_RIGHT] = diff_rear.GetSide2Torque();
	}

	for ( int i = 0; i < WHEEL_POSITION_SIZE; i++ ) assert ( !isnan ( wheel_drive_torque[WHEEL_POSITION ( i ) ] ) );
}

Dbl CARDYNAMICS::CalculateDriveshaftSpeed()
{
	Dbl driveshaft_speed = 0.0;
	Dbl left_front_wheel_speed = wheel[FRONT_LEFT].GetAngularVelocity();
	Dbl right_front_wheel_speed = wheel[FRONT_RIGHT].GetAngularVelocity();
	Dbl left_rear_wheel_speed = wheel[REAR_LEFT].GetAngularVelocity();
	Dbl right_rear_wheel_speed = wheel[REAR_RIGHT].GetAngularVelocity();
	for ( int i = 0; i < 4; i++ ) assert ( !isnan ( wheel[WHEEL_POSITION ( i ) ].GetAngularVelocity() ) );
	if ( drive == RWD )
	{
		driveshaft_speed = diff_rear.CalculateDriveshaftSpeed ( left_rear_wheel_speed, right_rear_wheel_speed );
	}
	else if ( drive == FWD )
	{
		driveshaft_speed = diff_front.CalculateDriveshaftSpeed ( left_front_wheel_speed, right_front_wheel_speed );
	}
	else if ( drive == AWD )
	{
		driveshaft_speed = diff_center.CalculateDriveshaftSpeed (
		                       diff_front.CalculateDriveshaftSpeed ( left_front_wheel_speed, right_front_wheel_speed ),
		                       diff_rear.CalculateDriveshaftSpeed ( left_rear_wheel_speed, right_rear_wheel_speed ) );
	}

	return driveshaft_speed;
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
			const Dbl spdmarg = 1.0;
			if (gas <-0.5 && GetSpeedMPS() < spdmarg && gear == 1)  gear =-1;  else
			if (gas > 0.5 && GetSpeedMPS() >-spdmarg && gear ==-1)  gear = 1;
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

Dbl CARDYNAMICS::CalculateDriveshaftRPM() const
{
	Dbl driveshaft_speed = 0.0;
	Dbl left_front_wheel_speed = wheel[FRONT_LEFT].GetAngularVelocity();
	Dbl right_front_wheel_speed = wheel[FRONT_RIGHT].GetAngularVelocity();
	Dbl left_rear_wheel_speed = wheel[REAR_LEFT].GetAngularVelocity();
	Dbl right_rear_wheel_speed = wheel[REAR_RIGHT].GetAngularVelocity();
	for ( int i = 0; i < 4; i++ ) assert ( !isnan ( wheel[WHEEL_POSITION ( i ) ].GetAngularVelocity() ) );
	if ( drive == RWD )
	{
		driveshaft_speed = diff_rear.GetDriveshaftSpeed ( left_rear_wheel_speed, right_rear_wheel_speed );
	}
	else if ( drive == FWD )
	{
		driveshaft_speed = diff_front.GetDriveshaftSpeed ( left_front_wheel_speed, right_front_wheel_speed );
	}
	else if ( drive == AWD )
	{
		Dbl front_speed = diff_front.GetDriveshaftSpeed ( left_front_wheel_speed, right_front_wheel_speed );
		Dbl rear_speed = diff_rear.GetDriveshaftSpeed ( left_rear_wheel_speed, right_rear_wheel_speed );
		driveshaft_speed = diff_center.GetDriveshaftSpeed ( front_speed, rear_speed );
	}

	return transmission.GetClutchSpeed ( driveshaft_speed ) * 30.0 / PI_d;
}

bool CARDYNAMICS::WheelDriven(int i) const
{
	return (1 << i) & drive;
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
		{
            willlock = willlock && brake[i].WillLock();
		}
	}
	if (willlock) return 0;

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

	if (clutch < 0)
		clutch = 0;
	if (clutch > 1.0)
		clutch = 1.0;

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
	if(remaining_shift_time > 0.0)
	{
	    if(engine.GetRPM() < driveshaft_rpm && engine.GetRPM() < engine.GetRedline())
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

///return the gear change (0 for no change, -1 for shift down, 1 for shift up)
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
            if(driveshaft_rpm < DownshiftRPM(gear) && gear > 1)
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

///do traction control system (wheelspin prevention) calculations and modify the throttle position if necessary
void CARDYNAMICS::DoTCS ( int i, Dbl suspension_force )
{
	Dbl gasthresh = 0.1;
	Dbl gas = engine.GetThrottle();

	//only active if throttle commanded past threshold
	if ( gas > gasthresh )
	{
		//see if we're spinning faster than the rest of the wheels
		Dbl maxspindiff = 0;
		Dbl myrotationalspeed = wheel[WHEEL_POSITION ( i ) ].GetAngularVelocity();
		for ( int i2 = 0; i2 < WHEEL_POSITION_SIZE; i2++ )
		{
			Dbl spindiff = myrotationalspeed - wheel[WHEEL_POSITION ( i2 ) ].GetAngularVelocity();
			if ( spindiff < 0 )
				spindiff = -spindiff;
			if ( spindiff > maxspindiff )
				maxspindiff = spindiff;
		}

		//don't engage if all wheels are moving at the same rate
		if ( maxspindiff > 1.0 )
		{
			//sp is the ideal slip ratio given tire loading
			Dbl sp ( 0 ), ah ( 0 );
			tire[WHEEL_POSITION ( i ) ].LookupSigmaHatAlphaHat ( suspension_force, sp, ah );

			Dbl sense = 1.0;
			if ( transmission.GetGear() < 0 )
				sense = -1.0;

			Dbl error = tire[WHEEL_POSITION ( i ) ].GetSlide() * sense - sp;
			Dbl thresholdeng = 0.0;
			Dbl thresholddis = -sp/2.0;

			if ( error > thresholdeng && ! tcs_active[i] )
				tcs_active[i] = true;

			if ( error < thresholddis && tcs_active[i] )
				tcs_active[i] = false;

			if ( tcs_active[i] )
			{
				Dbl curclutch = clutch.GetClutch();
				if ( curclutch > 1 ) curclutch = 1;
				if ( curclutch < 0 ) curclutch = 0;

				gas = gas - error * 10.0 * curclutch;
				if ( gas < 0 ) gas = 0;
				if ( gas > 1 ) gas = 1;
				engine.SetThrottle ( gas );
			}
		}
		else
			tcs_active[i] = false;
	}
	else
		tcs_active[i] = false;
}

///do anti-lock brake system calculations and modify the brake force if necessary
void CARDYNAMICS::DoABS ( int i, Dbl suspension_force )
{
	Dbl braketresh = 0.1;
	Dbl brakesetting = brake[WHEEL_POSITION ( i ) ].GetBrakeFactor();

	//only active if brakes commanded past threshold
	if ( brakesetting > braketresh )
	{
		Dbl maxspeed = 0;
		for ( int i2 = 0; i2 < WHEEL_POSITION_SIZE; i2++ )
		{
			if ( wheel[WHEEL_POSITION ( i2 ) ].GetAngularVelocity() > maxspeed )
				maxspeed = wheel[WHEEL_POSITION ( i2 ) ].GetAngularVelocity();
		}

		//don't engage ABS if all wheels are moving slowly
		if ( maxspeed > 6.0 )
		{
			//sp is the ideal slip ratio given tire loading
			Dbl sp ( 0 ), ah ( 0 );
			tire[WHEEL_POSITION ( i ) ].LookupSigmaHatAlphaHat ( suspension_force, sp, ah );

			Dbl error = - tire[WHEEL_POSITION ( i ) ].GetSlide() - sp;
			Dbl thresholdeng = 0.0;
			Dbl thresholddis = -sp/2.0;

			if ( error > thresholdeng && ! abs_active[i] )
				abs_active[i] = true;

			if ( error < thresholddis && abs_active[i] )
				abs_active[i] = false;
		}
		else
			abs_active[i] = false;
	}
	else
		abs_active[i] = false;

	if ( abs_active[i] )
		brake[WHEEL_POSITION ( i ) ].SetBrakeFactor ( 0.0 );
}

///Set the maximum steering angle in degrees
void CARDYNAMICS::SetMaxSteeringAngle ( Dbl newangle )
{
	maxangle = newangle;
}
void CARDYNAMICS::SetAngDamp( Dbl newang )
{
	ang_damp = newang;
}

void CARDYNAMICS::SetDrive ( const std::string & newdrive )
{
	if ( newdrive == "RWD" )
		drive = RWD;
	else if ( newdrive == "FWD" )
		drive = FWD;
	else if ( newdrive == "AWD" )
		drive = AWD;
	else
		assert ( 0 ); //shouldn't ever happen unless there's an error in the code
}

void CARDYNAMICS::AddMassParticle ( Dbl newmass, MATHVECTOR <Dbl, 3> newpos )
{
	mass_only_particles.push_back ( std::pair <Dbl, MATHVECTOR <Dbl, 3> > ( newmass, newpos ) );
	//std::cout << "adding mass particle " << newmass << " at " << newpos << std::endl;
}

void CARDYNAMICS::AddAerodynamicDevice (
	const MATHVECTOR <Dbl, 3> & newpos,
	Dbl drag_frontal_area,
	Dbl drag_coefficient,
	Dbl lift_surface_area,
	Dbl lift_coefficient,
	Dbl lift_efficiency )
{
	aerodynamics.push_back ( CARAERO() );
	aerodynamics.back().Set ( newpos, drag_frontal_area, drag_coefficient, lift_surface_area,
	                          lift_coefficient, lift_efficiency );
}

char CARDYNAMICS::IsBraking() const
{
	//  true when any wheel is braking
	for (int w=0; w<4; ++w)
	{
		WHEEL_POSITION wp = (WHEEL_POSITION)w;
		if (GetBrake(wp).GetBrakeFactor() > 0
		 || GetBrake(wp).GetHandbrakeFactor() > 0)
			return 1;
	}
	return 0;
}
