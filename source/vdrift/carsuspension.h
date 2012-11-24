#ifndef _CARSUSPENSION_H
#define _CARSUSPENSION_H

#include <iostream>
#include <cmath>

#include "dbl.h"
#include "mathvector.h"
#include "linearframe.h"
#include "joeserialize.h"
#include "macros.h"
#include "linearinterp.h"
#include "../ogre/common/Defines.h"


class CARSUSPENSION
{
friend class joeserialize::Serializer;
private:
	//constants (not actually declared as const because they can be changed after object creation)
	MATHVECTOR <Dbl, 3> hinge; ///< the point that the wheels are rotated around as the suspension compresses
	Dbl spring_constant; ///< the suspension spring constant
	Dbl bounce; ///< suspension compression damping
	Dbl rebound; ///< suspension decompression damping
	Dbl travel; ///< how far the suspension can travel from the zero-g fully extended position around the hinge arc before wheel travel is stopped
	Dbl anti_roll_k; ///< the spring constant for the anti-roll bar
	LINEARINTERP <Dbl> damper_factors;
	LINEARINTERP <Dbl> spring_factors;

	Dbl camber; ///< camber angle in degrees. sign convention depends on the side
	Dbl caster; ///< caster angle in degrees. sign convention depends on the side
	Dbl toe; ///< toe angle in degrees. sign convention depends on the side

	//variables
	Dbl overtravel; ///< the amount past the travel that the suspension was requested to compress
	Dbl displacement; ///< a linear representation of the suspension displacement.  in actuality the displacement is about the arc formed by the hinge
	Dbl velocity;
	Dbl force;

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

	void SetHinge ( const MATHVECTOR< Dbl, 3 >& value )
	{
		hinge = value;
	}

	const MATHVECTOR< Dbl, 3 > & GetHinge() const
	{
		return hinge;
	}

	void SetBounce ( const Dbl& value )
	{
		bounce = value;
	}

	Dbl GetBounce() const
	{
		return bounce;
	}

	void SetRebound ( const Dbl& value )
	{
		rebound = value;
	}

	Dbl GetRebound() const
	{
		return rebound;
	}

	void SetTravel ( const Dbl& value )
	{
		travel = value;
	}

	Dbl GetTravel() const
	{
		return travel;
	}

	void SetAntiRollK ( const Dbl& value )
	{
		anti_roll_k = value;
	}

	Dbl GetAntiRollK() const
	{
		return anti_roll_k;
	}

	void SetCamber ( const Dbl& value )
	{
		camber = value;
	}

	Dbl GetCamber() const
	{
		return camber;
	}

	void SetCaster ( const Dbl& value )
	{
		caster = value;
	}

	Dbl GetCaster() const
	{
		return caster;
	}

	void SetToe ( const Dbl& value )
	{
		toe = value;
	}

	Dbl GetToe() const
	{
		return toe;
	}

	void SetSpringConstant ( const Dbl& value )
	{
		spring_constant = value;
	}

	Dbl GetSpringConstant() const
	{
		return spring_constant;
	}

	const Dbl & GetDisplacement() const
	{
		return displacement;
	}

	///Return the displacement in percent of max travel where 0.0 is fully extended and 1.0 is fully compressed
	Dbl GetDisplacementPercent() const
	{
		return displacement / travel;
	}

	///compute the suspension force for the given time interval and external displacement
	Dbl Update(Dbl dt, Dbl ext_displacement)
	{
		// clamp external displacement
		overtravel = ext_displacement - travel;
		if (overtravel < 0)
			overtravel = 0;
		if (ext_displacement > travel)
			ext_displacement = travel;
		else if (ext_displacement < 0)
			ext_displacement = 0;

		Dbl new_displacement;
/*
		const Dbl inv_mass = 1/20.0;
		const Dbl tire_stiffness = 250000;

		Dbl ext_force = tire_stiffness * ext_displacement;

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

	const Dbl GetForce(Dbl displacement, Dbl velocity)
	{
		Dbl damping = bounce;
		if (velocity < 0) damping = rebound;

		//compute damper factor based on curve
		Dbl dampfactor = damper_factors.Interpolate(std::abs(velocity));

		//compute spring factor based on curve
		Dbl springfactor = spring_factors.Interpolate(displacement);

		Dbl spring_force = -displacement * spring_constant * springfactor; //when compressed, the spring force will push the car in the positive z direction
		Dbl damp_force = -velocity * damping * dampfactor; //when compression is increasing, the damp force will push the car in the positive z direction
		Dbl force = spring_force + damp_force;

		return force;
	}

	const Dbl & GetVelocity() const
	{
		return velocity;
	}

	//void SetAntiRollInfo(const Dbl value)
	//{
	//	antiroll_force = value;
	//}

	bool Serialize(joeserialize::Serializer & s)
	{
		_SERIALIZE_(s,displacement);
		return true;
	}

	Dbl GetOvertravel() const
	{
		return overtravel;
	}

	void SetDamperFactorPoints(std::vector <std::pair <Dbl, Dbl> > & curve)
	{
		//std::cout << "Damper factors: " << std::endl;
		for (std::vector <std::pair <Dbl, Dbl> >::iterator i = curve.begin(); i != curve.end(); i++)
		{
			//std::cout << i->first << ", " << i->second << std::endl;
			damper_factors.AddPoint(i->first, i->second);
		}
	}

	void SetSpringFactorPoints(std::vector <std::pair <Dbl, Dbl> > & curve)
	{
		//std::cout << "Spring factors: " << std::endl;
		for (std::vector <std::pair <Dbl, Dbl> >::iterator i = curve.begin(); i != curve.end(); i++)
		{
			//std::cout << i->first << ", " << i->second << std::endl;
			spring_factors.AddPoint(i->first, i->second);
		}
	}
};

#endif
