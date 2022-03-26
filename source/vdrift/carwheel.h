#pragma once
#include "dbl.h"
#include <iostream>
#include "mathvector.h"
#include "rotationalframe.h"
#include "matrix3.h"


class CARWHEEL
{
private:
	// constants
	MATHVECTOR<Dbl,3> extended_position;	///< the position of the wheel when the suspension is fully extended (zero g)
	Dbl roll_height;	///< how far off the road lateral forces are applied to the chassis
	Dbl steer_max;      ///< max steering angle, for > 4 wheels
	Dbl mass;			///< the mass of the wheel
	ROTATIONALFRAME rotation;	///< a simulation of wheel rotation.  this contains the wheel orientation, angular velocity, angular acceleration, and inertia tensor

	Dbl rolling_res_lin;	///< linear rolling resistance on a hard surface
	Dbl rolling_res_quad;	///< quadratic rolling resistance on a hard surface
	Dbl friction;  //<* extra coeff for bigger tires
	
	// variables
	//Dbl additional_inertia;
	Dbl inertia_cache;
	Dbl steer_angle;	///<negative values cause steering to the left
	Dbl radius;		///< the total radius of the tire
	Dbl ray_len;	///< distance to cast ray
	Dbl feedback;	///< the force feedback effect value

	//for info only
	Dbl angvel;
	Dbl camber_deg;

public:
	//for info only, vis
	struct SlideSlip
	{
		Dbl slide;	///< ratio of tire contact patch speed to road speed, minus one
		Dbl slip;	///< the angle (in degrees) between the wheel heading and the wheel's actual velocity
		Dbl slideratio;	///< ratio of the slide to the tire's optimim slide
		Dbl slipratio;	///< ratio of the slip to the tire's optimim slip
		Dbl fx_sr, fx_rsr;
		Dbl fy_ar, fy_rar;
		Dbl frict,gamma;
		Dbl Fx,Fxm,preFx, Fy,Fym,preFy, Fz;  // tire vis circle

		SlideSlip()
			:slide(0),slip(0), slideratio(0),slipratio(0)
			,Fx(0),Fxm(0),preFx(0), Fy(0),Fym(0),preFy(0), Fz(0)
			,fx_sr(0), fx_rsr(0)
			,fy_ar(0), fy_rar(0)
			,frict(0),gamma(0)
		{	}
	} slips;

	Dbl fluidRes;  /// new: fluid resistance
	
public:
	//default constructor makes an S2000-like car
	CARWHEEL()
		:roll_height(0.9), mass(18.1), inertia_cache(10.0)
		,steer_angle(0.), steer_max(0.), fluidRes(0.)
		,radius(0.3), ray_len(1.5), friction(1.0)
		,feedback(0.), camber_deg(0.), angvel(0.)
		,rolling_res_lin(1.3e-2), rolling_res_quad(6.5e-6)
	{	SetInertia(10.0);	}
	
	void DebugPrint(std::ostream & out);


	Dbl GetRollingResistance(const Dbl velocity, const Dbl rolling_resistance_factor) const;
	void SetRollingResistance(Dbl linear, Dbl quadratic)
	{
		rolling_res_lin = linear;  rolling_res_quad = quadratic;
	}


	void SetExtendedPosition (const MATHVECTOR<Dbl,3>& value)	{	extended_position = value;	}
	MATHVECTOR<Dbl,3> GetExtendedPosition() const				{	return extended_position;	}
	
	Dbl GetRPM() const		{	return rotation.GetAngularVelocity()[0] * 30.0 / PI_d;	}
	
	//used for telemetry only
	const Dbl & GetAngVelInfo()		{	return angvel;	}
	
	Dbl GetAngularVelocity() const	{	return rotation.GetAngularVelocity()[1];	}
	void SetAngularVelocity(Dbl angvel)
	{
		MATHVECTOR<Dbl,3> v(0, angvel, 0);
		return rotation.SetAngularVelocity(v);
	}

	Dbl GetSteerAngle() const				{	return steer_angle;		}
	void SetSteerAngle (const Dbl& value)	{	steer_angle = value;	}

	void SetRadius (const Dbl& value)	{	radius = value;		}
	Dbl GetRadius() const				{	return radius;		}

	void SetRayLength (const Dbl& value)	{	ray_len = value;	}
	Dbl GetRayLength() const				{	return ray_len;		}

	void SetFriction (const Dbl& value)	{	friction = value;	}
	Dbl GetFriction() const				{	return friction;		}

	void SetRollHeight (const Dbl& value)	{	roll_height = value;	}
	Dbl GetRollHeight() const				{	return roll_height;		}

	void SetSteerMax (const Dbl& value)	{	steer_max = value;	}
	Dbl GetSteerMax() const				{	return steer_max;	}

	void SetMass (const Dbl& value)	{	mass = value;	}
	Dbl GetMass() const				{	return mass;	}
	
	void SetInertia(Dbl new_inertia);
	Dbl GetInertia() const			{	return inertia_cache;	}

	void SetFeedback(Dbl aligning_force)	{	feedback = aligning_force;	}
	Dbl GetFeedback() const					{	return feedback;	}

	
	void SetInitialConditions()
	{
		MATHVECTOR<Dbl,3> v;
		rotation.SetInitialTorque(v);
	}
	void ZeroForces()
	{
		MATHVECTOR<Dbl,3> v;
		rotation.SetTorque(v);
	}
	
	void Integrate1(const Dbl dt)	{	rotation.Integrate1(dt);	}
	void Integrate2(const Dbl dt)	{	rotation.Integrate2(dt);	}
	
	void SetTorque(const Dbl torque);
	Dbl GetTorque()		{	return rotation.GetTorque()[1];		}
	
	Dbl GetLockUpTorque(const Dbl dt) const	{	return rotation.GetLockUpTorque(dt)[1];		}
		
	const QUATERNION<Dbl> & GetOrientation() const	{	return rotation.GetOrientation();	}
	
	//void SetAdditionalInertia (const Dbl& value);

	void SetCamberDeg (const Dbl& value)	{	camber_deg = value;		}
};
