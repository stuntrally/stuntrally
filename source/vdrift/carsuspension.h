#ifndef _CARSUSPENSION_H
#define _CARSUSPENSION_H

#include <iostream>
#include <cmath>

#include "mathvector.h"
#include "linearframe.h"
#include "joeserialize.h"
#include "macros.h"
#include "linearinterp.h"
#include "../ogre/common/Defines.h"


template <typename T>
class CARSUSPENSION
{
friend class joeserialize::Serializer;
private:
	//constants (not actually declared as const because they can be changed after object creation)
	MATHVECTOR <T, 3> hinge; ///< the point that the wheels are rotated around as the suspension compresses
	T spring_constant; ///< the suspension spring constant
	T bounce; ///< suspension compression damping
	T rebound; ///< suspension decompression damping
	T travel; ///< how far the suspension can travel from the zero-g fully extended position around the hinge arc before wheel travel is stopped
	T anti_roll_k; ///< the spring constant for the anti-roll bar
	LINEARINTERP <T> damper_factors;
	LINEARINTERP <T> spring_factors;

	T camber; ///< camber angle in degrees. sign convention depends on the side
	T caster; ///< caster angle in degrees. sign convention depends on the side
	T toe; ///< toe angle in degrees. sign convention depends on the side

	//variables
	T overtravel; ///< the amount past the travel that the suspension was requested to compress
	T displacement; ///< a linear representation of the suspension displacement.  in actuality the displacement is about the arc formed by the hinge
	T velocity;
	T force;

public:
	//default constructor makes an S2000-like car
	CARSUSPENSION() : spring_constant(50000.0), bounce(2588), rebound(2600), travel(0.19),
			anti_roll_k(8000), damper_factors(1), spring_factors(1),
			camber(-0.5), caster(0.28), toe(0),
			overtravel(0), displacement(0), velocity(0), force(0) {}

	void DebugPrint(std::ostream & out)
	{
		//out << "---Suspension---" << std::endl;
		out << "Disp  " << fToStr(displacement, 2,4) << std::endl;
		out << "Vel  " << fToStr(velocity, 2,5) << std::endl;
		//always 1?..
		//out << "Spring " << fToStr(spring_factors.Interpolate(displacement), 2,4) << std::endl;
		//out << "Damp   " << fToStr(damper_factors.Interpolate(std::abs(velocity)), 2,4) << std::endl;
	}

	void SetHinge ( const MATHVECTOR< T, 3 >& value )
	{
		hinge = value;
	}

	const MATHVECTOR< T, 3 > & GetHinge() const
	{
		return hinge;
	}

	void SetBounce ( const T& value )
	{
		bounce = value;
	}

	T GetBounce() const
	{
		return bounce;
	}

	void SetRebound ( const T& value )
	{
		rebound = value;
	}

	T GetRebound() const
	{
		return rebound;
	}

	void SetTravel ( const T& value )
	{
		travel = value;
	}

	T GetTravel() const
	{
		return travel;
	}

	void SetAntiRollK ( const T& value )
	{
		anti_roll_k = value;
	}

	T GetAntiRollK() const
	{
		return anti_roll_k;
	}

	void SetCamber ( const T& value )
	{
		camber = value;
	}

	T GetCamber() const
	{
		return camber;
	}

	void SetCaster ( const T& value )
	{
		caster = value;
	}

	T GetCaster() const
	{
		return caster;
	}

	void SetToe ( const T& value )
	{
		toe = value;
	}

	T GetToe() const
	{
		return toe;
	}

	void SetSpringConstant ( const T& value )
	{
		spring_constant = value;
	}

	T GetSpringConstant() const
	{
		return spring_constant;
	}

	const T & GetDisplacement() const
	{
		return displacement;
	}

	///Return the displacement in percent of max travel where 0.0 is fully extended and 1.0 is fully compressed
	T GetDisplacementPercent() const
	{
		return displacement / travel;
	}

	///compute the suspension force for the given time interval and external displacement
	T Update(T dt, T ext_displacement)
	{
		// clamp external displacement
		overtravel = ext_displacement - travel;
		if (overtravel < 0)
			overtravel = 0;
		if (ext_displacement > travel)
			ext_displacement = travel;
		else if (ext_displacement < 0)
			ext_displacement = 0;

		T new_displacement;
/*
		const T inv_mass = 1/20.0;
		const T tire_stiffness = 250000;

		T ext_force = tire_stiffness * ext_displacement;

		// predict new displacement
		new_displacement = displacement + velocity * dt + 0.5 * force * inv_mass * dt * dt;

		// clamp new displacement
		if (new_displacement > travel)
			new_displacement = travel;
		else if (new_displacement < 0)
			new_displacement = 0;
		
		// calculate derivatives
		//if (new_displacement < ext_displacement)*/
			new_displacement = ext_displacement;

		velocity = (new_displacement - displacement) / dt;
		
		// clamp velocity (workaround for very high damping values)
		if (velocity > 5) velocity = 5;
		else if (velocity < -5) velocity = -5;

		displacement = new_displacement;
		force = GetForce(displacement, velocity);

		return -force;
	}

	const T GetForce(T displacement, T velocity)
	{
		T damping = bounce;
		if (velocity < 0) damping = rebound;

		//compute damper factor based on curve
		T dampfactor = damper_factors.Interpolate(std::abs(velocity));

		//compute spring factor based on curve
		T springfactor = spring_factors.Interpolate(displacement);

		T spring_force = -displacement * spring_constant * springfactor; //when compressed, the spring force will push the car in the positive z direction
		T damp_force = -velocity * damping * dampfactor; //when compression is increasing, the damp force will push the car in the positive z direction
		T force = spring_force + damp_force;

		return force;
	}

	const T & GetVelocity() const
	{
		return velocity;
	}

	//void SetAntiRollInfo(const T value)
	//{
	//	antiroll_force = value;
	//}

	bool Serialize(joeserialize::Serializer & s)
	{
		_SERIALIZE_(s,displacement);
		return true;
	}

	T GetOvertravel() const
	{
		return overtravel;
	}

	void SetDamperFactorPoints(std::vector <std::pair <T, T> > & curve)
	{
		//std::cout << "Damper factors: " << std::endl;
		for (typename std::vector <std::pair <T, T> >::iterator i = curve.begin(); i != curve.end(); i++)
		{
			//std::cout << i->first << ", " << i->second << std::endl;
			damper_factors.AddPoint(i->first, i->second);
		}
	}

	void SetSpringFactorPoints(std::vector <std::pair <T, T> > & curve)
	{
		//std::cout << "Spring factors: " << std::endl;
		for (typename std::vector <std::pair <T, T> >::iterator i = curve.begin(); i != curve.end(); i++)
		{
			//std::cout << i->first << ", " << i->second << std::endl;
			spring_factors.AddPoint(i->first, i->second);
		}
	}
};

#endif
