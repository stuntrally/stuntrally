#ifndef _CARWHEEL_H
#define _CARWHEEL_H

#include "dbl.h"
#include <iostream>
#include "mathvector.h"
#include "rotationalframe.h"
#include "matrix3.h"
#include "joeserialize.h"
#include "macros.h"
#include "../ogre/common/Defines.h"


class CARWHEEL
{
friend class joeserialize::Serializer;
public:
	//default constructor makes an S2000-like car
	CARWHEEL() : roll_height(0.29), mass(18.14), inertia_cache(10.0), steer_angle(0), fluidRes(0.f)
	{	SetInertia(10.0);	}
	
	void DebugPrint(std::ostream & out)
	{
		//out << "---Wheel---" << std::endl;
		//out << "Speed: " << fToStr(GetRPM(), 1,4) << std::endl;
		out << "Steer  " << fToStr(steer_angle, 1,5) << std::endl;
		out << "Camber " << fToStr(camber_deg, 1,5)  << std::endl;
	}

	void SetExtendedPosition ( const MATHVECTOR<Dbl,3>& value )
	{
		extended_position = value;
	}
	
	Dbl GetRPM() const
	{
		return rotation.GetAngularVelocity()[0] * 30.0 / PI_d;
	}
	
	//used for telemetry only
	const Dbl & GetAngVelInfo()
	{
		return angvel;
	}
	
	Dbl GetAngularVelocity() const
	{
		return rotation.GetAngularVelocity()[1];
	}
	
	void SetAngularVelocity(Dbl angvel)
	{
		MATHVECTOR <Dbl, 3> v(0, angvel, 0);
		return rotation.SetAngularVelocity(v);
	}

	MATHVECTOR< Dbl, 3 > GetExtendedPosition() const
	{
		return extended_position;
	}

	void SetRollHeight ( const Dbl& value )
	{
		roll_height = value;
	}

	Dbl GetRollHeight() const
	{
		return roll_height;
	}

	void SetMass ( const Dbl& value )
	{
		mass = value;
	}

	Dbl GetMass() const
	{
		return mass;
	}
	
	void SetInertia(Dbl new_inertia)
	{
		inertia_cache = new_inertia;
		MATRIX3 <Dbl> inertia;
		inertia.Scale(new_inertia);
		rotation.SetInertia(inertia);
	}
	
	Dbl GetInertia() const
	{
		return inertia_cache;
	}
	
	void SetInitialConditions()
	{
		MATHVECTOR <Dbl, 3> v;
		rotation.SetInitialTorque(v);
	}
	
	void Integrate1(const Dbl dt)
	{
		rotation.Integrate1(dt);
	}
		
	void Integrate2(const Dbl dt)
	{
		rotation.Integrate2(dt);
	}
	
	void SetTorque(const Dbl torque)
	{
		MATHVECTOR <Dbl, 3> v(0, torque, 0);
		rotation.SetTorque(v);
		angvel = GetAngularVelocity();
	}
	
	Dbl GetTorque()
	{
		return rotation.GetTorque()[1];
	}
	
	Dbl GetLockUpTorque(const Dbl dt) const
	{
	    return rotation.GetLockUpTorque(dt)[1];
	}
	
	void ZeroForces()
	{
		MATHVECTOR <Dbl, 3> v;
		rotation.SetTorque(v);
	}
	
	const QUATERNION <Dbl> & GetOrientation() const
	{
		return rotation.GetOrientation();
	}

	Dbl GetSteerAngle() const
	{
		return steer_angle;
	}

	void SetSteerAngle ( const Dbl& value )
	{
		steer_angle = value;
	}
	
	bool Serialize(joeserialize::Serializer & s)
	{
		_SERIALIZE_(s,inertia_cache);
		_SERIALIZE_(s,steer_angle);
		return true;
	}

	void SetAdditionalInertia ( const Dbl& value )
	{
		additional_inertia = value;
		
		MATRIX3 <Dbl> inertia;
		inertia.Scale(inertia_cache + additional_inertia);
		rotation.SetInertia(inertia);
		
		//std::cout << inertia_cache << " + " << additional_inertia << " = " << inertia_cache + additional_inertia << std::endl;
	}

	void SetCamberDeg ( const Dbl& value )
	{
		camber_deg = value;
	}
		
	Dbl fluidRes;  /// new: fluid resistance

	void SetRadius ( const Dbl& value )
	{
		radius = value;
	}

	Dbl GetRadius() const
	{
		return radius;
	}
	
private:
	//constants (not actually declared as const because they can be changed after object creation)
	MATHVECTOR <Dbl, 3> extended_position; ///< the position of the wheel when the suspension is fully extended (zero g)
	Dbl roll_height; ///< how far off the road lateral forces are applied to the chassis
	Dbl mass; ///< the mass of the wheel
	ROTATIONALFRAME rotation; ///< a simulation of wheel rotation.  this contains the wheel orientation, angular velocity, angular acceleration, and inertia tensor
	
	//variables
	Dbl additional_inertia;
	Dbl inertia_cache;
	Dbl steer_angle; ///<negative values cause steering to the left
	Dbl radius;  ///< the total radius of the tire
	
	//for info only
	Dbl angvel;
	Dbl camber_deg;
};

#endif
