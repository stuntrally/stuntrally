#include "pch.h"
#include "par.h"
#include "cardynamics.h"
#include "tobullet.h"
#include "../ogre/common/Def_Str.h"


MATHVECTOR<Dbl,3> CARDYNAMICS::GetWheelPosition(WHEEL_POSITION wp) const
{
	MATHVECTOR<Dbl,3> pos = GetLocalWheelPosition(wp, suspension[wp].GetDisplacementPercent());
	chassisRotation.RotateVector(pos);
	return pos + chassisPosition;
}

MATHVECTOR<Dbl,3> CARDYNAMICS::GetWheelPosition(WHEEL_POSITION wp, Dbl displacement_percent) const
{
	MATHVECTOR<Dbl,3> pos = GetLocalWheelPosition(wp, displacement_percent);
	chassisRotation.RotateVector(pos);
	return pos + chassisPosition;
}

QUATERNION<Dbl> CARDYNAMICS::GetWheelOrientation(WHEEL_POSITION wp) const
{
	QUATERNION<Dbl> siderot;
	if(wp == FRONT_RIGHT || wp == REAR_RIGHT || wp == REAR2_RIGHT || wp == REAR3_RIGHT)
	{
		siderot.Rotate(PI_d, 0, 0, 1);
	}
	return chassisRotation * GetWheelSteeringAndSuspensionOrientation(wp) * wheel[wp].GetOrientation() * siderot;
}

QUATERNION<Dbl> CARDYNAMICS::GetUprightOrientation(WHEEL_POSITION wp) const
{
	return chassisRotation * GetWheelSteeringAndSuspensionOrientation(wp);
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

Dbl CARDYNAMICS::GetSpeedDir() const
{
	MATHVECTOR<Dbl,3> v(1, 0, 0);
	Orientation().RotateVector(v);
	
	Dbl vel = body.GetVelocity().dot(v);  // car body vel in local car direction
	return sqrt(vel*vel);
}

MATHVECTOR<Dbl,3> CARDYNAMICS::GetVelocity() const
{
	return body.GetVelocity();
	//return ToMathVector<Dbl>(chassis->getLinearVelocity());
}

MATHVECTOR<Dbl,3> CARDYNAMICS::GetAngularVelocity() const
{
	return body.GetAngularVelocity();
	//return ToMathVector<Dbl>(chassis->getAngularVelocity());
}

MATHVECTOR<Dbl,3> CARDYNAMICS::GetEnginePosition() const
{
	MATHVECTOR<Dbl,3> offset = engine.GetPosition();
	Orientation().RotateVector(offset);
	return offset + chassisPosition;
}

void CARDYNAMICS::StartEngine()
{
	engine.StartEngine();
}
void CARDYNAMICS::SetClutch(float value)
{
	clutch.SetClutch(value);
}

float CARDYNAMICS::GetThrottle() const
{
	if (vtype == V_Spaceship)  return hov_throttle;  //
	return engine.GetThrottle();
}

void CARDYNAMICS::SetThrottle(float value)
{
	/// <><> damage reduce  from 50 %
	float dmg = fDamage >= 100.f ? 0.f : (1.f - 0.6f * std::max(0.f, fDamage-50.f)/50.f);

	if (vtype != V_Car)
		hov_throttle = value * dmg;
	else
		engine.SetThrottle(value * dmg);
}

void CARDYNAMICS::SetBrake(float value)
{
	/// <><> damage reduce  from 50 %
	float dmg = 1.f - std::max(0.f, fDamage-50.f)/50.f * 0.6f;

	for (size_t i = 0; i < brake.size(); ++i)
		brake[i].SetBrakeFactor(fDamage >= 100.f ? 0.1f : value * dmg);
}

void CARDYNAMICS::SetHandBrake(float value)
{
	/// <><> damage reduce  from 50 %
	float dmg = fDamage >= 100.f ? 0.f : (1.f - std::max(0.f, fDamage-50.f)/50.f * 0.6f);

	for (size_t i = 0; i < brake.size(); ++i)
		brake[i].SetHandbrakeFactor(value * dmg);
}

void CARDYNAMICS::SetAutoClutch(bool value)	{	autoclutch = value;	}
void CARDYNAMICS::SetAutoShift(bool value)	{	autoshift = value;	}
void CARDYNAMICS::SetAutoRear(bool value)	{	autorear = value;	}

Dbl CARDYNAMICS::GetTachoRPM() const	{	return tacho_rpm;	}

void CARDYNAMICS::SetABS(const bool newabs)	{	abs = newabs;	}
bool CARDYNAMICS::GetABSEnabled() const		{	return abs;		}
bool CARDYNAMICS::GetABSActive() const
{
	if (numWheels < 4)
		return abs && ( abs_active[0]||abs_active[1] );
	else
		return abs && ( abs_active[0]||abs_active[1]||abs_active[2]||abs_active[3] );
}
void CARDYNAMICS::SetTCS(const bool newtcs)	{	tcs = newtcs;	}
bool CARDYNAMICS::GetTCSEnabled() const		{	return tcs;		}
bool CARDYNAMICS::GetTCSActive() const
{
	if (numWheels < 4)
		return tcs && ( tcs_active[0]||tcs_active[1] );
	else
		return tcs && ( tcs_active[0]||tcs_active[1]||tcs_active[2]||tcs_active[3] );
}


void CARDYNAMICS::SetPosition(const MATHVECTOR<Dbl,3> & position)
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
	for (int i = 0; i < WHEEL_POSITION_SIZE; ++i)
		{
		Dbl height = wheel_contact[i].GetDepth() - 2 * tire[i].GetRadius();
		if (height < min_height || no_min_height)
			{
			min_height = height;
			no_min_height = false;
			}
	}/**/  //--
	
	//MATHVECTOR<Dbl,3> trimmed_position = Position() + GetDownVector() * min_height;
	//SetPosition(Position()/*trimmed_position*/);
}

MATHVECTOR<Dbl,3> CARDYNAMICS::GetTotalAero() const
{
	MATHVECTOR<Dbl,3> downforce = 0;
	for (auto a : aerodynamics)
		downforce = downforce + a.GetLiftVector() + a.GetDragVector();
	return downforce;
}

Dbl CARDYNAMICS::GetAerodynamicDownforceCoefficient() const
{
	Dbl coeff = 0.0;
	for (auto a : aerodynamics)
		coeff += a.GetAerodynamicDownforceCoefficient();
	return coeff;
}

Dbl CARDYNAMICS::GetAeordynamicDragCoefficient() const
{
	Dbl coeff = 0.0;
	for (auto a : aerodynamics)
		coeff += a.GetAeordynamicDragCoefficient();
	return coeff;
}


MATHVECTOR<Dbl,3> CARDYNAMICS::GetDownVector() const
{
	MATHVECTOR<Dbl,3> v(0, 0, -1);
	Orientation().RotateVector(v);
	return v;
}

QUATERNION<Dbl> CARDYNAMICS::Orientation() const
{
	return body.GetOrientation();
	//return ToMathQuaternion<Dbl>(chassis->getOrientation());
}

MATHVECTOR<Dbl,3> CARDYNAMICS::Position() const
{
	return body.GetPosition();
	//return ToMathVector<Dbl>(chassis->getCenterOfMassPosition());
}

MATHVECTOR<Dbl,3> CARDYNAMICS::LocalToWorld(const MATHVECTOR<Dbl,3> & local) const
{
	MATHVECTOR<Dbl,3> position = local - center_of_mass;
	body.GetOrientation().RotateVector(position);
	return position + body.GetPosition();
	//btVector3 position = chassis->getCenterOfMassTransform().getBasis() * ToBulletVector(local - center_of_mass);
	//position = position + chassis->getCenterOfMassTransform().getOrigin();
	//return ToMathVector <Dbl> (position);
}

//  simple hinge (arc) suspension displacement
MATHVECTOR<Dbl,3> CARDYNAMICS::GetLocalWheelPosition(WHEEL_POSITION wp, Dbl displacement_percent) const
{
	// const
	const MATHVECTOR<Dbl,3> & wheelext = wheel[wp].GetExtendedPosition();
	const MATHVECTOR<Dbl,3> & hinge = suspension[wp].GetHinge();
	MATHVECTOR<Dbl,3> relwheelext = wheelext - hinge;
	MATHVECTOR<Dbl,3> up(0,0,1);
	MATHVECTOR<Dbl,3> rotaxis = up.cross ( relwheelext.Normalize() );
	Dbl hingeradius = relwheelext.Magnitude();
	Dbl travel = suspension[wp].GetTravel();
	// const

	Dbl displacement = displacement_percent * travel;
	Dbl displacementradians = displacement / hingeradius;
	QUATERNION<Dbl> hingerotate;
	hingerotate.Rotate ( -displacementradians, rotaxis[0], rotaxis[1], rotaxis[2] );
	MATHVECTOR<Dbl,3> localwheelpos = relwheelext;
	hingerotate.RotateVector ( localwheelpos );
	return localwheelpos + hinge;
}

//  returns the orientation of the wheel due only to steering and suspension
QUATERNION<Dbl> CARDYNAMICS::GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION wp) const
{
	QUATERNION<Dbl> steer;
	steer.Rotate( -wheel[wp].GetSteerAngle() * PI_d/180.0, 0,0,1);

	QUATERNION<Dbl> camber;
	Dbl camber_rotation = -suspension[wp].GetCamber() * PI_d/180.0;
	if (wp%2 == 1)
		camber_rotation = -camber_rotation;
	camber.Rotate( camber_rotation, 1,0,0);

	QUATERNION<Dbl> toe;
	Dbl toe_rotation = suspension[wp].GetToe() * PI_d/180.0;
	if (wp%2 == 0)
		toe_rotation = -toe_rotation;
	toe.Rotate( toe_rotation, 0,0,1);

	return camber * toe * steer;
}

//  worldspace position of the center of the wheel when the suspension is compressed
//  by the displacement_percent where 1.0 is fully compressed
MATHVECTOR<Dbl,3> CARDYNAMICS::GetWheelPositionAtDisplacement(WHEEL_POSITION wp, Dbl displacement_percent) const
{
	return LocalToWorld(GetLocalWheelPosition(wp, displacement_percent));
}

void CARDYNAMICS::ApplyForce(const MATHVECTOR<Dbl,3> & force)
{
	body.ApplyForce(force);
	cam_body.ApplyForce(force / GetMass() * gPar.camBncF * fBncMass);
	//chassis->applyCentralForce(ToBulletVector(force));
}

void CARDYNAMICS::ApplyForce(const MATHVECTOR<Dbl,3> & force, const MATHVECTOR<Dbl,3> & offset)
{
	body.ApplyForce(force, offset);
	MATHVECTOR<Dbl,3> fm = force / GetMass();
	MATHVECTOR<Dbl,3> fo = offset * fm.Magnitude();
	cam_body.ApplyForce((fm * gPar.camBncFo + fo * gPar.camBncFof) * fBncMass);
	//chassis->applyForce(ToBulletVector(force), ToBulletVector(offset));
}

void CARDYNAMICS::ApplyTorque(const MATHVECTOR<Dbl,3> & torque)
{
	body.ApplyTorque(torque);
	//if(torque.MagnitudeSquared() > 1E-6)
	//	chassis->applyTorque(ToBulletVector(torque));
}

void CARDYNAMICS::UpdateWheelVelocity()
{
	for(int i = 0; i < numWheels; ++i)
	{
		wheel_velocity[i] = body.GetVelocity(wheel_position[i] - body.GetPosition());
		//btVector3 offset = ToBulletVector(wheel_position[i]) - chassis->getCenterOfMassPosition();
		//wheel_velocity[i] = ToMathVector<Dbl>(chassis->getVelocityInLocalPoint(offset));
	}
}

void CARDYNAMICS::UpdateWheelTransform()
{
	for(int i = 0; i < numWheels; ++i)
	{
		wheel_position[i] = GetWheelPositionAtDisplacement(WHEEL_POSITION(i), suspension[i].GetDisplacementPercent());
		wheel_orientation[i] = Orientation() * GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION(i));
	}
}

void CARDYNAMICS::SetDrive(const std::string & newdrive)
{
		 if (newdrive == "RWD")  drive = RWD;
	else if (newdrive == "FWD")  drive = FWD;
	else if (newdrive == "AWD")  drive = AWD;
	else if (newdrive == "6WD")  drive = WD6;
	else if (newdrive == "8WD")  drive = WD8;
	else  assert(0);
}

void CARDYNAMICS::AddMassParticle(Dbl newmass, MATHVECTOR<Dbl,3> newpos)
{
	newpos[0] += com_ofs_L;  //|
	newpos[2] += com_ofs_H;  //|
	mass_only_particles.push_back( std::pair <Dbl, MATHVECTOR<Dbl,3> > (newmass, newpos) );
	//std::cout << "adding mass particle " << newmass << " at " << newpos << std::endl;
}

void CARDYNAMICS::AddAerodynamicDevice(const MATHVECTOR<Dbl,3> & newpos,
	Dbl drag_frontal_area, Dbl drag_coefficient,
	Dbl lift_surface_area, Dbl lift_coefficient, Dbl lift_efficiency )
{
	aerodynamics.push_back( CARAERO() );
	aerodynamics.back().Set( newpos, drag_frontal_area, drag_coefficient, lift_surface_area,
	                          lift_coefficient, lift_efficiency );
}

char CARDYNAMICS::IsBraking() const
{
	//  true when any wheel is braking
	if (fDamage < 100.f)
	for (int w=0; w < numWheels; ++w)
	{
		WHEEL_POSITION wp = (WHEEL_POSITION)w;
		if (GetBrake(wp).GetBrakeFactor() > 0 ||
			GetBrake(wp).GetHandbrakeFactor() > 0)
			return 1;
	}
	return 0;
}
