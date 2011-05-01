#include "stdafx.h"

#include "cardynamics.h"

#include "configfile.h"
#include "tracksurface.h"
#include "coordinatesystems.h"
#include "collision_world.h"
#include "tobullet.h"
#include "model.h"
#include "../ogre/OgreGame.h"

typedef CARDYNAMICS::T T;


// executed as last function(after integration) in bullet singlestepsimulation
void CARDYNAMICS::updateAction(btCollisionWorld * collisionWorld, btScalar dt)
{
	SynchronizeBody();  // get velocity, position orientation after dt

	UpdateWheelContacts();  // update wheel contacts given new velocity, position

	Tick(dt);  // run internal simulation

	SynchronizeChassis();  // update velocity

	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	#if 0  // old not used
	if (GetAsyncKeyState(VK_F1))
	{
		///***  reset pos  ---------------------------------------------------------------------------------
		/*
		T chassisMass = body.GetMass();
		MATRIX3 <T> inertia = body.GetInertia();
		btVector3 chassisInertia(inertia[0], inertia[4], inertia[8]);
		*/
		btTransform transform;
		//transform.setOrigin(ToBulletVector(position));
		//transform.setRotation(ToBulletQuaternion(orientation));
		//btDefaultMotionState * chassisState = new btDefaultMotionState();
		chassis->getMotionState()->setWorldTransform(transform);

		MATHVECTOR <T, 3> zero(0, 10, 0);
		body.SetPosition( zero);
		//body.SetOrientation( );
		body.SetInitialForce(zero);
		body.SetInitialTorque(zero);
	}
	#endif
	#endif
}

void CARDYNAMICS::debugDraw(btIDebugDraw* debugDrawer)
{	}

void CARDYNAMICS::Update()
{
	if (!chassis)  return;//
	btTransform chassisTrans;
	chassis->getMotionState()->getWorldTransform(chassisTrans);
	chassisRotation = ToMathQuaternion<T>(chassisTrans.getRotation());
	chassisCenterOfMass = ToMathVector<T>(chassisTrans.getOrigin());
	MATHVECTOR <T, 3> com = center_of_mass;
	chassisRotation.RotateVector(com);
	chassisPosition = chassisCenterOfMass - com;
}

const MATHVECTOR <T, 3> & CARDYNAMICS::GetCenterOfMassPosition() const
{
	return chassisCenterOfMass;
}

const MATHVECTOR <T, 3> & CARDYNAMICS::GetPosition() const
{
	return chassisPosition;
}

const QUATERNION <T> & CARDYNAMICS::GetOrientation() const
{
	return chassisRotation;
}

MATHVECTOR <T, 3> CARDYNAMICS::GetWheelPosition(WHEEL_POSITION wp) const
{
	MATHVECTOR <T, 3> pos = GetLocalWheelPosition(wp, suspension[wp].GetDisplacementPercent());
	chassisRotation.RotateVector(pos);
	return pos + chassisPosition;

	//btTransform wheelTrans;
	//wheelBody[WHEEL_POSITION(wp)]->getMotionState()->getWorldTransform(wheelTrans);
	//return ToMathVector<T>(wheelTrans.getOrigin());
}

MATHVECTOR <T, 3> CARDYNAMICS::GetWheelPosition(WHEEL_POSITION wp, T displacement_percent) const
{
	MATHVECTOR <T, 3> pos = GetLocalWheelPosition(wp, displacement_percent);
	chassisRotation.RotateVector(pos);
	return pos + chassisPosition;
}

QUATERNION <T> CARDYNAMICS::GetWheelOrientation(WHEEL_POSITION wp) const
{
	QUATERNION <T> siderot;
	if(wp == FRONT_RIGHT || wp == REAR_RIGHT)
	{
		siderot.Rotate(3.141593, 0, 0, 1);
	}
	return chassisRotation * GetWheelSteeringAndSuspensionOrientation(wp) * wheel[wp].GetOrientation() * siderot;

	//btTransform wheelTrans;
	//wheelBody[WHEEL_POSITION(wp)]->getMotionState()->getWorldTransform(wheelTrans);
	//return ToMathQuaternion<T>(wheelTrans.getRotation()) * siderot;
}

QUATERNION <T> CARDYNAMICS::GetUprightOrientation(WHEEL_POSITION wp) const
{
	return chassisRotation * GetWheelSteeringAndSuspensionOrientation(wp);

	//btTransform wheelTrans;
	//wheelBody[WHEEL_POSITION(wp)]->getMotionState()->getWorldTransform(wheelTrans);
	//return ToMathQuaternion<T>(wheelTrans.getRotation());
}

/// worldspace wheel center position
MATHVECTOR <T, 3> CARDYNAMICS::GetWheelVelocity ( WHEEL_POSITION wp ) const
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

T CARDYNAMICS::GetSpeed() const
{
	return body.GetVelocity().Magnitude();
	//return chassis->getLinearVelocity().length();
}

MATHVECTOR <T, 3> CARDYNAMICS::GetVelocity() const
{
	return body.GetVelocity();
	//return ToMathVector<T>(chassis->getLinearVelocity());
}

MATHVECTOR <T, 3> CARDYNAMICS::GetEnginePosition() const
{
	MATHVECTOR <T, 3> offset = engine.GetPosition();
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

T CARDYNAMICS::GetSpeedMPS() const
{
	T left_front_wheel_speed = wheel[FRONT_LEFT].GetAngularVelocity();
	T right_front_wheel_speed = wheel[FRONT_RIGHT].GetAngularVelocity();
	T left_rear_wheel_speed = wheel[REAR_LEFT].GetAngularVelocity();
	T right_rear_wheel_speed = wheel[REAR_RIGHT].GetAngularVelocity();
	for ( int i = 0; i < 4; i++ ) assert ( !isnan ( wheel[WHEEL_POSITION ( i ) ].GetAngularVelocity() ) );
	if ( drive == RWD )
	{
		return ( left_rear_wheel_speed+right_rear_wheel_speed ) * 0.5 * tire[REAR_LEFT].GetRadius();
	}
	else if ( drive == FWD )
	{
		return ( left_front_wheel_speed+right_front_wheel_speed ) * 0.5 * tire[FRONT_LEFT].GetRadius();
	}
	else if ( drive == AWD )
	{
		return ( ( left_rear_wheel_speed+right_rear_wheel_speed ) * 0.5 * tire[REAR_LEFT].GetRadius() +
		         ( left_front_wheel_speed+right_front_wheel_speed ) * 0.5 * tire[FRONT_LEFT].GetRadius() ) *0.5;
	}

	assert ( 0 );
	return 0;
}

T CARDYNAMICS::GetTachoRPM() const
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

void CARDYNAMICS::SetPosition(const MATHVECTOR<T, 3> & position)
{
	body.SetPosition(position);
	chassis->translate(ToBulletVector(position) - chassis->getCenterOfMassPosition());
}

//find the precise starting position for the car (trim out the extra space)
void CARDYNAMICS::AlignWithGround()
{
	UpdateWheelTransform();
	UpdateWheelContacts();
	
	/*T min_height = 0;
	bool no_min_height = true;
	for (int i = 0; i < WHEEL_POSITION_SIZE; i++)
		{
		T height = wheel_contact[i].GetDepth() - 2 * tire[i].GetRadius();
		if (height < min_height || no_min_height)
			{
			min_height = height;
			no_min_height = false;
			}
	}/**/  //--
	
	//MATHVECTOR <T, 3> trimmed_position = Position() + GetDownVector() * min_height;
	//SetPosition(Position()/*trimmed_position*/);
}

//TODO: adjustable ackermann-like parameters
///set the steering angle to "value", where 1.0 is maximum right lock and -1.0 is maximum left lock.
void CARDYNAMICS::SetSteering ( const T value )
{
	T steerangle = value * maxangle; //steering angle in degrees

	//ackermann stuff
	T alpha = std::abs ( steerangle * 3.141593/180.0 ); //outside wheel steering angle in radians
	T B = wheel[FRONT_LEFT].GetExtendedPosition() [1]
		- wheel[FRONT_RIGHT].GetExtendedPosition() [1]; //distance between front wheels
	T L = wheel[FRONT_LEFT].GetExtendedPosition() [0]
		- wheel[REAR_LEFT].GetExtendedPosition() [0]; //distance between front and rear wheels
	T beta = atan2 ( 1.0, ( ( 1.0/ ( tan ( alpha ) ) )-B/L ) ); //inside wheel steering angle in radians

	T left_wheel_angle = 0;
	T right_wheel_angle = 0;

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

	left_wheel_angle *= 180.0/3.141593;
	right_wheel_angle *= 180.0/3.141593;

	wheel[FRONT_LEFT].SetSteerAngle ( left_wheel_angle );
	wheel[FRONT_RIGHT].SetSteerAngle ( right_wheel_angle );
}

T CARDYNAMICS::GetMaxSteeringAngle() const
{
	return maxangle;
}

MATHVECTOR <T, 3> CARDYNAMICS::GetTotalAero() const
{
	MATHVECTOR <T, 3> downforce = 0;
	for ( std::vector <CARAERO<T> >::const_iterator i = aerodynamics.begin(); i != aerodynamics.end(); ++i )
	{
		downforce = downforce + i->GetLiftVector() +  i->GetDragVector();
	}
	return downforce;
}

T CARDYNAMICS::GetAerodynamicDownforceCoefficient() const
{
	T coeff = 0.0;
	for ( std::vector <CARAERO <T> >::const_iterator i = aerodynamics.begin(); i != aerodynamics.end(); ++i )
		coeff += i->GetAerodynamicDownforceCoefficient();
	return coeff;
}

T CARDYNAMICS::GetAeordynamicDragCoefficient() const
{
	T coeff = 0.0;
	for ( std::vector <CARAERO <T> >::const_iterator i = aerodynamics.begin(); i != aerodynamics.end(); ++i )
		coeff += i->GetAeordynamicDragCoefficient();
	return coeff;
}

MATHVECTOR< T, 3 > CARDYNAMICS::GetLastBodyForce() const
{
	return lastbodyforce;
}

T CARDYNAMICS::GetFeedback() const
{
	return feedback;
}

void CARDYNAMICS::UpdateTelemetry ( float dt )
{
	//telemetry.Update ( dt );
}

/// print debug info to the given ostream.  set p1, p2, etc if debug info part 1, and/or part 2, etc is desired
void CARDYNAMICS::DebugPrint ( std::ostream & out, bool p1, bool p2, bool p3, bool p4 )
{
	if ( p1 )
	{
		out.precision(4);
		out << "---Body---" << std::endl;
		out << "c of mass: " << center_of_mass << std::endl;
		MATRIX3 <T> inertia = body.GetInertia();
		//btVector3 chassisInertia(inertia[0], inertia[4], inertia[8]);
		out << "inertia:  " << inertia[0] << "  " << inertia[4] << "  " << inertia[8] << "\n";
		out.precision(6);
		out << "mass: " << body.GetMass() << std::endl;	out << std::endl;
		engine.DebugPrint ( out );	out << std::endl;
	return;//
		fuel_tank.DebugPrint ( out );	out << std::endl;
		clutch.DebugPrint ( out );	out << std::endl;
		transmission.DebugPrint ( out );	out << std::endl;
		if ( drive == RWD )  {
			out << "(rear)" << std::endl;		rear_differential.DebugPrint ( out );	}
		else if ( drive == FWD )  {
			out << "(front)" << std::endl;		front_differential.DebugPrint ( out );	}
		else if ( drive == AWD )  {
			out << "(center)" << std::endl;		center_differential.DebugPrint ( out );
			out << "(front)" << std::endl;		front_differential.DebugPrint ( out );
			out << "(rear)" << std::endl;		rear_differential.DebugPrint ( out );	}
		out << std::endl;
	}
	return;//

	if ( p2 )
	{
		out << "(front left)" << std::endl;		suspension[FRONT_LEFT].DebugPrint ( out );	out << std::endl;
		out << "(front right)" << std::endl;	suspension[FRONT_RIGHT].DebugPrint ( out );	out << std::endl;
		out << "(rear left)" << std::endl;		suspension[REAR_LEFT].DebugPrint ( out );	out << std::endl;
		out << "(rear right)" << std::endl;		suspension[REAR_RIGHT].DebugPrint ( out );	out << std::endl;

		out << "(front left)" << std::endl;		brake[FRONT_LEFT].DebugPrint ( out );	out << std::endl;
		out << "(front right)" << std::endl;	brake[FRONT_RIGHT].DebugPrint ( out );	out << std::endl;
		out << "(rear left)" << std::endl;		brake[REAR_LEFT].DebugPrint ( out );	out << std::endl;
		out << "(rear right)" << std::endl;		brake[REAR_RIGHT].DebugPrint ( out );
	}

	if ( p3 )
	{
		out << std::endl;
		out << "(front left)" << std::endl;		wheel[FRONT_LEFT].DebugPrint ( out );	out << std::endl;
		out << "(front right)" << std::endl;	wheel[FRONT_RIGHT].DebugPrint ( out );	out << std::endl;
		out << "(rear left)" << std::endl;		wheel[REAR_LEFT].DebugPrint ( out );	out << std::endl;
		out << "(rear right)" << std::endl;		wheel[REAR_RIGHT].DebugPrint ( out );	out << std::endl;

		out << "(front left)" << std::endl;		tire[FRONT_LEFT].DebugPrint ( out );	out << std::endl;
		out << "(front right)" << std::endl;	tire[FRONT_RIGHT].DebugPrint ( out );	out << std::endl;
		out << "(rear left)" << std::endl;		tire[REAR_LEFT].DebugPrint ( out );		out << std::endl;
		out << "(rear right)" << std::endl;		tire[REAR_RIGHT].DebugPrint ( out );
	}

	if ( p4 )
	{
		for ( std::vector <CARAERO<T> >::iterator i = aerodynamics.begin(); i != aerodynamics.end(); ++i )
		{
			i->DebugPrint ( out );	out << std::endl;
		}
	}
}

bool CARDYNAMICS::Serialize ( joeserialize::Serializer & s )
{
	_SERIALIZE_ ( s,body );
	_SERIALIZE_ ( s,engine );
	_SERIALIZE_ ( s,clutch );
	_SERIALIZE_ ( s,transmission );
	_SERIALIZE_ ( s,front_differential );
	_SERIALIZE_ ( s,rear_differential );
	_SERIALIZE_ ( s,center_differential );
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

MATHVECTOR <T, 3> CARDYNAMICS::GetDownVector() const
{
	MATHVECTOR <T, 3> v(0, 0, -1);
	Orientation().RotateVector(v);
	return v;
}

QUATERNION <T> CARDYNAMICS::Orientation() const
{
	return body.GetOrientation();
	//return ToMathQuaternion<T>(chassis->getOrientation());
}

MATHVECTOR <T, 3> CARDYNAMICS::Position() const
{
	return body.GetPosition();
	//return ToMathVector<T>(chassis->getCenterOfMassPosition());
}

MATHVECTOR <T, 3> CARDYNAMICS::LocalToWorld(const MATHVECTOR <T, 3> & local) const
{
	MATHVECTOR <T,3> position = local - center_of_mass;
	body.GetOrientation().RotateVector(position);
	return position + body.GetPosition();
	//btVector3 position = chassis->getCenterOfMassTransform().getBasis() * ToBulletVector(local - center_of_mass);
	//position = position + chassis->getCenterOfMassTransform().getOrigin();
	//return ToMathVector <T> (position);
}

//simple hinge (arc) suspension displacement
MATHVECTOR <T, 3> CARDYNAMICS::GetLocalWheelPosition(WHEEL_POSITION wp, T displacement_percent) const
{
	//const
	const MATHVECTOR <T, 3> & wheelext = wheel[wp].GetExtendedPosition();
	const MATHVECTOR <T, 3> & hinge = suspension[wp].GetHinge();
	MATHVECTOR <T, 3> relwheelext = wheelext - hinge;
	MATHVECTOR <T, 3> up (0, 0, 1);
	MATHVECTOR <T, 3> rotaxis = up.cross ( relwheelext.Normalize() );
	T hingeradius = relwheelext.Magnitude();
	T travel = suspension[wp].GetTravel();
	//const

	T displacement = displacement_percent * travel;
	T displacementradians = displacement / hingeradius;
	QUATERNION <T> hingerotate;
	hingerotate.Rotate ( -displacementradians, rotaxis[0], rotaxis[1], rotaxis[2] );
	MATHVECTOR <T, 3> localwheelpos = relwheelext;
	hingerotate.RotateVector ( localwheelpos );
	return localwheelpos + hinge;
}

///returns the orientation of the wheel due only to steering and suspension
QUATERNION <T> CARDYNAMICS::GetWheelSteeringAndSuspensionOrientation ( WHEEL_POSITION wp ) const
{
	QUATERNION <T> steer;
	steer.Rotate ( -wheel[wp].GetSteerAngle() * 3.141593/180.0, 0, 0, 1 );

	QUATERNION <T> camber;
	T camber_rotation = -suspension[wp].GetCamber() * 3.141593/180.0;
	if ( wp == 1 || wp == 3 )
		camber_rotation = -camber_rotation;
	camber.Rotate ( camber_rotation, 1, 0, 0 );

	QUATERNION <T> toe;
	T toe_rotation = suspension[wp].GetToe() * 3.141593/180.0;
	if ( wp == 0 || wp == 2 )
		toe_rotation = -toe_rotation;
	toe.Rotate ( toe_rotation, 0, 0, 1 );

	return camber * toe * steer;
}

/// worldspace position of the center of the wheel when the suspension is compressed
/// by the displacement_percent where 1.0 is fully compressed
MATHVECTOR <T, 3> CARDYNAMICS::GetWheelPositionAtDisplacement(WHEEL_POSITION wp, T displacement_percent) const
{
	return LocalToWorld(GetLocalWheelPosition(wp, displacement_percent));
}

void CARDYNAMICS::ApplyForce(const MATHVECTOR <T, 3> & force)
{
	body.ApplyForce(force);
	//chassis->applyCentralForce(ToBulletVector(force));
}

void CARDYNAMICS::ApplyForce(const MATHVECTOR <T, 3> & force, const MATHVECTOR <T, 3> & offset)
{
	body.ApplyForce(force, offset);
	//chassis->applyForce(ToBulletVector(force), ToBulletVector(offset));
}

void CARDYNAMICS::ApplyTorque(const MATHVECTOR <T, 3> & torque)
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
		//wheel_velocity[i] = ToMathVector<T>(chassis->getVelocityInLocalPoint(offset));
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
	MATHVECTOR <T, 3> engine_torque(-engine.GetTorque(), 0, 0);
	Orientation().RotateVector(engine_torque);
	ApplyTorque(engine_torque);
}

void CARDYNAMICS::ApplyAerodynamicsToBody(T dt)
{
	MATHVECTOR <T, 3> wind_force(0);
	MATHVECTOR <T, 3> wind_torque(0);
	MATHVECTOR <T, 3> air_velocity = -GetVelocity();
	(-Orientation()).RotateVector(air_velocity);
	for(std::vector <CARAERO <T> >::iterator i = aerodynamics.begin(); i != aerodynamics.end(); ++i)
	{
		MATHVECTOR <T, 3> force = i->GetForce(air_velocity);
		wind_force = wind_force + force;
		wind_torque = wind_torque + (i->GetPosition() - center_of_mass).cross(force);
	}
	Orientation().RotateVector(wind_force);
	Orientation().RotateVector(wind_torque);
	ApplyForce(wind_force);
	ApplyTorque(wind_torque);

	//apply rotational damping/drag (hide oscillation around z-axis) ?
	//MATHVECTOR <T, 3> rotational_aero_drag = - ToMathVector<T>(chassis->getAngularVelocity()) * 1000.0f;
	//ApplyTorque(rotational_aero_drag);
}

MATHVECTOR <T, 3> CARDYNAMICS::UpdateSuspension ( int i , T dt )
{
	// displacement
	const T posx = wheel_contact[i].GetPosition() [0];
	const T posz = wheel_contact[i].GetPosition() [2];
	const TRACKSURFACE & surface = wheel_contact[i].GetSurface();
	T phase = 0;
	if (surface.bumpWaveLength > 0.0001)
		phase = 2 * 3.141593 * ( posx+posz ) / surface.bumpWaveLength;
	T shift = 2.0 * sin ( phase*1.414214 );
	T amplitude = 0.25 * surface.bumpAmplitude;
	T bumpoffset = amplitude * ( sin ( phase + shift ) + sin ( 1.414214*phase ) - 2.0 );
	//T ray_offset = 0.2;  //*! wheel ray origin is offset by 1 meter relative to wheel extended position
	T displacement = /*ray_offset +*/ 2.0*tire[i].GetRadius() - wheel_contact[i].GetDepth() + bumpoffset;

	// compute suspension force
	T springdampforce = suspension[WHEEL_POSITION ( i ) ].Update ( dt , displacement );

	//do anti-roll
	int otheri = i;
	if ( i == 0 || i == 2 )
		otheri++;
	else
		otheri--;
	T antirollforce = suspension[WHEEL_POSITION ( i ) ].GetAntiRollK() *
	                  ( suspension[WHEEL_POSITION ( i ) ].GetDisplacement()-
	                    suspension[WHEEL_POSITION ( otheri ) ].GetDisplacement() );
	//suspension[WHEEL_POSITION(i)].SetAntiRollInfo(antirollforce);
	if (isnan ( antirollforce ))  antirollforce = 0.f;//crash dyn obj
	assert ( !isnan ( antirollforce ) );

	//find the vector direction to apply the suspension force
#ifdef SUSPENSION_FORCE_DIRECTION
	const MATHVECTOR <T, 3> & wheelext = wheel[i].GetExtendedPosition();
	const MATHVECTOR <T, 3> & hinge = suspension[i].GetHinge();
	MATHVECTOR <T, 3> relwheelext = wheelext - hinge;
	MATHVECTOR <T, 3> up ( 0,0,1 );
	MATHVECTOR <T, 3> rotaxis = up.cross ( relwheelext.Normalize() );
	MATHVECTOR <T, 3> forcedirection = relwheelext.Normalize().cross ( rotaxis );
	//std::cout << i << ". " << forcedirection << std::endl;
	MATHVECTOR <T, 3> suspension_force = forcedirection * ( antirollforce+springdampforce );
#else
	MATHVECTOR <T, 3> suspension_force(0, 0, antirollforce + springdampforce);
#endif
	Orientation().RotateVector(suspension_force);
	return suspension_force;
}

// aplies tire friction  to car, returns friction in world space
MATHVECTOR <T, 3> CARDYNAMICS::ApplyTireForce(int i, const T normal_force, const QUATERNION <T> & wheel_space)
{
	CARWHEEL<T> & wheel = this->wheel[WHEEL_POSITION(i)];
	CARTIRE<T> & tire = this->tire[WHEEL_POSITION(i)];
	const COLLISION_CONTACT & wheel_contact = this->wheel_contact[WHEEL_POSITION(i)];
	const TRACKSURFACE & surface = wheel_contact.GetSurface();
	const MATHVECTOR <T, 3> surface_normal = wheel_contact.GetNormal();

	// camber relative to surface(clockwise in wheel heading direction)
	MATHVECTOR <T, 3> wheel_axis(0, 1, 0);
	wheel_space.RotateVector(wheel_axis); // wheel axis in world space (wheel plane normal)
	T camber_sin = wheel_axis.dot(surface_normal);
	T camber_rad = asin(camber_sin);
	wheel.SetCamberDeg(camber_rad * 180.0/3.141593);

	// tire space(SAE Tire Coordinate System)
	// surface normal is z-axis
	// wheel axis projected on surface plane is y-axis
	MATHVECTOR <T, 3> y_axis = wheel_axis - surface_normal * camber_sin;
	MATHVECTOR <T, 3> x_axis = y_axis.cross(surface_normal);

	// wheel center velocity in tire space
	MATHVECTOR <T, 3> hub_velocity;
	hub_velocity[0] = x_axis.dot(wheel_velocity[WHEEL_POSITION(i)]);
	hub_velocity[1] = y_axis.dot(wheel_velocity[WHEEL_POSITION(i)]);
	hub_velocity[2] = 0; // unused

	// rearward speed of the contact patch
	T patch_speed = wheel.GetAngularVelocity() * tire.GetRadius();

	// friction force in tire space
	T friction_coeff = tire.GetTread() * surface.frictionTread + (1.0 - tire.GetTread()) * surface.frictionNonTread;
	T roll_friction_coeff = surface.rollResistanceCoefficient;
	MATHVECTOR <T, 3> friction_force(0);
	if(friction_coeff > 0)
		friction_force = tire.GetForce(normal_force, friction_coeff, roll_friction_coeff, hub_velocity, patch_speed, camber_rad);

	// set force feedback (aligning torque in tire space)
	tire.SetFeedback(friction_force[2]);

	// friction force in world space
	MATHVECTOR <T, 3> world_friction_force = x_axis * friction_force[0] + y_axis * friction_force[1];

	// fake viscous friction (sand, gravel, mud)
	MATHVECTOR <T, 3> wheel_drag = - (x_axis * hub_velocity[0] + y_axis * hub_velocity[1]) * surface.rollingDrag;

	// apply forces to body
	MATHVECTOR <T, 3> wheel_normal(0, 0, 1);
	wheel_space.RotateVector(wheel_normal);
	MATHVECTOR <T, 3> contactpos = wheel_position[WHEEL_POSITION(i)] - wheel_normal * tire.GetRadius();
	ApplyForce(world_friction_force + surface_normal * normal_force + wheel_drag, contactpos - Position());

	return world_friction_force;
}

void CARDYNAMICS::ApplyWheelTorque(T dt, T drive_torque, int i, MATHVECTOR <T, 3> tire_friction, const QUATERNION <T> & wheel_space)
{
	CARWHEEL<T> & wheel = this->wheel[WHEEL_POSITION(i)];
	CARTIRE<T> & tire = this->tire[WHEEL_POSITION(i)];
	CARBRAKE<T> & brake = this->brake[WHEEL_POSITION(i)];

	// tire force / torque
	wheel.Integrate1(dt);

	(-wheel_space).RotateVector(tire_friction);

	// torques acting on wheel
	T friction_torque = tire_friction[0] * tire.GetRadius();
	T wheel_torque = drive_torque - friction_torque;
	T lock_up_torque = wheel.GetLockUpTorque(dt) - wheel_torque;	// torque needed to lock the wheel
	T brake_torque = brake.GetTorque();

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
	MATHVECTOR <T, 3> world_wheel_torque(0, -wheel_torque, 0);
	wheel_space.RotateVector(world_wheel_torque);
	ApplyTorque(world_wheel_torque);
}

void CARDYNAMICS::UpdateBody(T dt, T drive_torque[])
{
	body.Integrate1(dt);
	//chassis->clearForces();

	UpdateWheelVelocity();

	ApplyEngineTorqueToBody();

	ApplyAerodynamicsToBody(dt);
	

	///***  manual car flip over  ---------------------------------------------------------------------------------
	bool flipLeft = doFlipLeft;
	bool flipRight = doFlipRight;
	int flip = (flipLeft ? -1 : 0) + (flipRight ? 1 : 0);
	if (flip)
	{
		MATRIX3 <T> inertia = body.GetInertia();
		btVector3 inrt(inertia[0], inertia[4], inertia[8]);
		//  strength_
		float t = 12.f * flip * inrt[inrt.maxAxis()];
		MATHVECTOR <T, 3> v(t,0,0);
		Orientation().RotateVector(v);
		ApplyTorque(v);
	}
	///***  boost
	bool ctrl = doBoost;
	if (ctrl)
	{
		T f = body.GetMass() * 16.f;
		MATHVECTOR <T, 3> v(f,0,0), ofs(0,0,0);
		Orientation().RotateVector(v);
		ApplyForce(v, ofs);
	}
	///***
	

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

void CARDYNAMICS::Tick ( T dt )
{
	// has to happen before UpdateDriveline, overrides clutch, throttle
	UpdateTransmission(dt);

	const int num_repeats = 30;  ///~ 30+  o:10
	const float internal_dt = dt / num_repeats;
	for(int i = 0; i < num_repeats; ++i)
	{
		T drive_torque[WHEEL_POSITION_SIZE];

		UpdateDriveline(internal_dt, drive_torque);

		UpdateBody(internal_dt, drive_torque);

		feedback += 0.5 * (tire[FRONT_LEFT].GetFeedback() + tire[FRONT_RIGHT].GetFeedback());
	}

	feedback /= (num_repeats + 1);

	fuel_tank.Consume ( engine.FuelRate() * dt );
	engine.SetOutOfGas ( fuel_tank.Empty() );

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
		
		world->CastRay(raystart, raydir, raylen, chassis, wheelContact, /*R+*/&bWhOnRoad[i]);
		if (bTerrain)  ///  terrain surf from blendmap
			wheelContact.SetSurface(terSurf[i]);
	}
}

void CARDYNAMICS::InterpolateWheelContacts(T dt)
{
	MATHVECTOR <float, 3> raydir = GetDownVector();
	for (int i = 0; i < WHEEL_POSITION_SIZE; i++)
	{
		MATHVECTOR <float, 3> raystart = LocalToWorld(wheel[i].GetExtendedPosition());
		raystart = raystart - raydir * tire[i].GetRadius();  //*!
		float raylen = 1;  //!par
		GetWheelContact(WHEEL_POSITION(i)).CastRay(raystart, raydir, raylen);
	}
}

///calculate the center of mass, calculate the total mass of the body, calculate the inertia tensor
/// then store this information in the rigid body
void CARDYNAMICS::UpdateMass()
{
	typedef std::pair <T, MATHVECTOR <T, 3> > MASS_PAIR;

	T total_mass ( 0 );

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
	MATRIX3 <T> inertia;
	for ( int i = 0; i < 9; i++ )
		inertia[i] = 0;
	for ( std::list <MASS_PAIR>::iterator i = mass_only_particles.begin(); i != mass_only_particles.end(); ++i )
	{
		//transform into the rigid body coordinates
		MATHVECTOR <T, 3> position = i->second - center_of_mass;
		T mass = i->first;

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

void CARDYNAMICS::UpdateDriveline(T dt, T drive_torque[])
{
	engine.Integrate1(dt);

	T driveshaft_speed = CalculateDriveshaftSpeed();
	T clutch_speed = transmission.CalculateClutchSpeed(driveshaft_speed);
	T crankshaft_speed = engine.GetAngularVelocity();
	T engine_drag = clutch.GetTorque(crankshaft_speed, clutch_speed);

	engine.ComputeForces();

	ApplyClutchTorque(engine_drag, clutch_speed);

	engine.ApplyForces();

	CalculateDriveTorque(drive_torque, engine_drag);

	engine.Integrate2(dt);
}

///apply forces on the engine due to drag from the clutch
void CARDYNAMICS::ApplyClutchTorque ( T engine_drag, T clutch_speed )
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
void CARDYNAMICS::CalculateDriveTorque ( T * wheel_drive_torque, T clutch_torque )
{
	T driveshaft_torque = transmission.GetTorque ( clutch_torque );
	assert ( !isnan ( driveshaft_torque ) );

	for ( int i = 0; i < WHEEL_POSITION_SIZE; i++ )
		wheel_drive_torque[i] = 0;

	if ( drive == RWD )
	{
		rear_differential.ComputeWheelTorques ( driveshaft_torque );
		wheel_drive_torque[REAR_LEFT] = rear_differential.GetSide1Torque();
		wheel_drive_torque[REAR_RIGHT] = rear_differential.GetSide2Torque();
	}
	else if ( drive == FWD )
	{
		front_differential.ComputeWheelTorques ( driveshaft_torque );
		wheel_drive_torque[FRONT_LEFT] = front_differential.GetSide1Torque();
		wheel_drive_torque[FRONT_RIGHT] = front_differential.GetSide2Torque();
	}
	else if ( drive == AWD )
	{
		center_differential.ComputeWheelTorques ( driveshaft_torque );
		front_differential.ComputeWheelTorques ( center_differential.GetSide1Torque() );
		rear_differential.ComputeWheelTorques ( center_differential.GetSide2Torque() );
		wheel_drive_torque[FRONT_LEFT] = front_differential.GetSide1Torque();
		wheel_drive_torque[FRONT_RIGHT] = front_differential.GetSide2Torque();
		wheel_drive_torque[REAR_LEFT] = rear_differential.GetSide1Torque();
		wheel_drive_torque[REAR_RIGHT] = rear_differential.GetSide2Torque();
	}

	for ( int i = 0; i < WHEEL_POSITION_SIZE; i++ ) assert ( !isnan ( wheel_drive_torque[WHEEL_POSITION ( i ) ] ) );
}

T CARDYNAMICS::CalculateDriveshaftSpeed()
{
	T driveshaft_speed = 0.0;
	T left_front_wheel_speed = wheel[FRONT_LEFT].GetAngularVelocity();
	T right_front_wheel_speed = wheel[FRONT_RIGHT].GetAngularVelocity();
	T left_rear_wheel_speed = wheel[REAR_LEFT].GetAngularVelocity();
	T right_rear_wheel_speed = wheel[REAR_RIGHT].GetAngularVelocity();
	for ( int i = 0; i < 4; i++ ) assert ( !isnan ( wheel[WHEEL_POSITION ( i ) ].GetAngularVelocity() ) );
	if ( drive == RWD )
	{
		driveshaft_speed = rear_differential.CalculateDriveshaftSpeed ( left_rear_wheel_speed, right_rear_wheel_speed );
	}
	else if ( drive == FWD )
	{
		driveshaft_speed = front_differential.CalculateDriveshaftSpeed ( left_front_wheel_speed, right_front_wheel_speed );
	}
	else if ( drive == AWD )
	{
		driveshaft_speed = center_differential.CalculateDriveshaftSpeed (
		                       front_differential.CalculateDriveshaftSpeed ( left_front_wheel_speed, right_front_wheel_speed ),
		                       rear_differential.CalculateDriveshaftSpeed ( left_rear_wheel_speed, right_rear_wheel_speed ) );
	}

	return driveshaft_speed;
}

void CARDYNAMICS::UpdateTransmission(T dt)
{
	driveshaft_rpm = CalculateDriveshaftRPM();

	if (autoshift)
	{
		int gear = NextGear();
		
		//  auto Rear gear
		if (autorear)
		{
			T gas = engine.GetThrottle();
			gas -= brake[0].GetBrakeFactor();
			if (transmission.GetGear() == -1)  gas *= -1;
			const T spdmarg = 1.0;
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
		    std::cout << "start engine" << std::endl;
		}

		T throttle = engine.GetThrottle();
		throttle = ShiftAutoClutchThrottle(throttle, dt);
		engine.SetThrottle(throttle);

		T new_clutch = AutoClutch(last_auto_clutch, dt);
		clutch.SetClutch(new_clutch);
		last_auto_clutch = new_clutch;
	}
}

T CARDYNAMICS::CalculateDriveshaftRPM() const
{
	T driveshaft_speed = 0.0;
	T left_front_wheel_speed = wheel[FRONT_LEFT].GetAngularVelocity();
	T right_front_wheel_speed = wheel[FRONT_RIGHT].GetAngularVelocity();
	T left_rear_wheel_speed = wheel[REAR_LEFT].GetAngularVelocity();
	T right_rear_wheel_speed = wheel[REAR_RIGHT].GetAngularVelocity();
	for ( int i = 0; i < 4; i++ ) assert ( !isnan ( wheel[WHEEL_POSITION ( i ) ].GetAngularVelocity() ) );
	if ( drive == RWD )
	{
		driveshaft_speed = rear_differential.GetDriveshaftSpeed ( left_rear_wheel_speed, right_rear_wheel_speed );
	}
	else if ( drive == FWD )
	{
		driveshaft_speed = front_differential.GetDriveshaftSpeed ( left_front_wheel_speed, right_front_wheel_speed );
	}
	else if ( drive == AWD )
	{
		T front_speed = front_differential.GetDriveshaftSpeed ( left_front_wheel_speed, right_front_wheel_speed );
		T rear_speed = rear_differential.GetDriveshaftSpeed ( left_rear_wheel_speed, right_rear_wheel_speed );
		driveshaft_speed = center_differential.GetDriveshaftSpeed ( front_speed, rear_speed );
	}

	return transmission.GetClutchSpeed ( driveshaft_speed ) * 30.0 / 3.141593;
}

bool CARDYNAMICS::WheelDriven(int i) const
{
	return (1 << i) & drive;
}

T CARDYNAMICS::AutoClutch(T last_clutch, T dt) const
{
	const T threshold = 1000.0;
	const T margin = 100.0;
	const T geareffect = 1.0; //zero to 1, defines special consideration of first/reverse gear

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

	const T rpm = engine.GetRPM();
	const T maxrpm = engine.GetRPMLimit();
	const T stallrpm = engine.GetStallRPM() + margin * (maxrpm / 2000.0);
	const int gear = transmission.GetGear();

	T gearfactor = 1.0;
	if (gear <= 1)
		gearfactor = 2.0;
	T thresh = threshold * (maxrpm/7000.0) * ((1.0-geareffect)+gearfactor*geareffect) + stallrpm;
	if (clutch.IsLocked())
		thresh *= 0.5;
	T clutch = (rpm-stallrpm) / (thresh-stallrpm);

	//std::cout << rpm << ", " << stallrpm << ", " << threshold << ", " << clutch << std::endl;

	if (clutch < 0)
		clutch = 0;
	if (clutch > 1.0)
		clutch = 1.0;

	T newauto = clutch * ShiftAutoClutch();

	//rate limit the autoclutch
	const T min_engage_time = 0.05; //the fastest time in seconds for auto-clutch engagement
	const T engage_rate_limit = 1.0/min_engage_time;
	const T rate = (last_clutch - newauto)/dt; //engagement rate in clutch units per second
	if (rate > engage_rate_limit)
		newauto = last_clutch - engage_rate_limit*dt;

    return newauto;
}

T CARDYNAMICS::ShiftAutoClutch() const
{
	T shift_clutch = 1.0;
	if (remaining_shift_time > shift_time * 0.5)
	    shift_clutch = 0.0;
	else if (remaining_shift_time > 0.0)
	    shift_clutch = 1.0 - remaining_shift_time / (shift_time * 0.5);
	return shift_clutch;
}

T CARDYNAMICS::ShiftAutoClutchThrottle(T throttle, T dt)
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
	if (shifted)
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

T CARDYNAMICS::DownshiftRPM(int gear) const
{
	T shift_down_point = 0.0;
	if (gear > 1)
	{
        T current_gear_ratio = transmission.GetGearRatio(gear);
        T lower_gear_ratio = transmission.GetGearRatio(gear - 1);
		T peak_engine_speed = engine.GetRedline();
		shift_down_point = 0.9 * peak_engine_speed / lower_gear_ratio * current_gear_ratio;
	}					  // 0.5 def-
	return shift_down_point;
}

///do traction control system (wheelspin prevention) calculations and modify the throttle position if necessary
void CARDYNAMICS::DoTCS ( int i, T suspension_force )
{
	T gasthresh = 0.1;
	T gas = engine.GetThrottle();

	//only active if throttle commanded past threshold
	if ( gas > gasthresh )
	{
		//see if we're spinning faster than the rest of the wheels
		T maxspindiff = 0;
		T myrotationalspeed = wheel[WHEEL_POSITION ( i ) ].GetAngularVelocity();
		for ( int i2 = 0; i2 < WHEEL_POSITION_SIZE; i2++ )
		{
			T spindiff = myrotationalspeed - wheel[WHEEL_POSITION ( i2 ) ].GetAngularVelocity();
			if ( spindiff < 0 )
				spindiff = -spindiff;
			if ( spindiff > maxspindiff )
				maxspindiff = spindiff;
		}

		//don't engage if all wheels are moving at the same rate
		if ( maxspindiff > 1.0 )
		{
			//sp is the ideal slip ratio given tire loading
			T sp ( 0 ), ah ( 0 );
			tire[WHEEL_POSITION ( i ) ].LookupSigmaHatAlphaHat ( suspension_force, sp, ah );

			T sense = 1.0;
			if ( transmission.GetGear() < 0 )
				sense = -1.0;

			T error = tire[WHEEL_POSITION ( i ) ].GetSlide() * sense - sp;
			T thresholdeng = 0.0;
			T thresholddis = -sp/2.0;

			if ( error > thresholdeng && ! tcs_active[i] )
				tcs_active[i] = true;

			if ( error < thresholddis && tcs_active[i] )
				tcs_active[i] = false;

			if ( tcs_active[i] )
			{
				T curclutch = clutch.GetClutch();
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
void CARDYNAMICS::DoABS ( int i, T suspension_force )
{
	T braketresh = 0.1;
	T brakesetting = brake[WHEEL_POSITION ( i ) ].GetBrakeFactor();

	//only active if brakes commanded past threshold
	if ( brakesetting > braketresh )
	{
		T maxspeed = 0;
		for ( int i2 = 0; i2 < WHEEL_POSITION_SIZE; i2++ )
		{
			if ( wheel[WHEEL_POSITION ( i2 ) ].GetAngularVelocity() > maxspeed )
				maxspeed = wheel[WHEEL_POSITION ( i2 ) ].GetAngularVelocity();
		}

		//don't engage ABS if all wheels are moving slowly
		if ( maxspeed > 6.0 )
		{
			//sp is the ideal slip ratio given tire loading
			T sp ( 0 ), ah ( 0 );
			tire[WHEEL_POSITION ( i ) ].LookupSigmaHatAlphaHat ( suspension_force, sp, ah );

			T error = - tire[WHEEL_POSITION ( i ) ].GetSlide() - sp;
			T thresholdeng = 0.0;
			T thresholddis = -sp/2.0;

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
void CARDYNAMICS::SetMaxSteeringAngle ( T newangle )
{
	maxangle = newangle;
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

void CARDYNAMICS::AddMassParticle ( T newmass, MATHVECTOR <T, 3> newpos )
{
	mass_only_particles.push_back ( std::pair <T, MATHVECTOR <T, 3> > ( newmass, newpos ) );
	//std::cout << "adding mass particle " << newmass << " at " << newpos << std::endl;
}

void CARDYNAMICS::AddAerodynamicDevice (
	const MATHVECTOR <T, 3> & newpos,
	T drag_frontal_area,
	T drag_coefficient,
	T lift_surface_area,
	T lift_coefficient,
	T lift_efficiency )
{
	aerodynamics.push_back ( CARAERO<T>() );
	aerodynamics.back().Set ( newpos, drag_frontal_area, drag_coefficient, lift_surface_area,
	                          lift_coefficient, lift_efficiency );
}
