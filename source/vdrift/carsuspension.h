#pragma once
#include <iostream>
#include <cmath>

#include "dbl.h"
#include "mathvector.h"
#include "linearframe.h"
#include "linearinterp.h"


class CARSUSPENSION
{
private:
	// constants
	MATHVECTOR<Dbl,3> hinge;	///< the point that the wheels are rotated around as the suspension compresses
	Dbl spring_constant;	///< the suspension spring constant
	Dbl bounce;		///< suspension compression damping
	Dbl rebound;	///< suspension decompression damping
	Dbl travel;		///< how far the suspension can travel from the zero-g fully extended position around the hinge arc before wheel travel is stopped
	Dbl anti_roll_k;	///< the spring constant for the anti-roll bar
	LINEARINTERP <Dbl> damper_factors;
	LINEARINTERP <Dbl> spring_factors;

	Dbl camber;		///< camber angle in degrees. sign convention depends on the side
	Dbl caster;		///< caster angle in degrees. sign convention depends on the side
	Dbl toe;		///< toe angle in degrees. sign convention depends on the side

public:
	// variables
	Dbl overtravel;		///< the amount past the travel that the suspension was requested to compress
	Dbl displacement;	///< a linear representation of the suspension displacement.  in actuality the displacement is about the arc formed by the hinge
	Dbl velocity;
	Dbl force;

public:
	//default constructor makes an S2000-like car
	CARSUSPENSION() : spring_constant(50000.0), bounce(2588), rebound(2600), travel(0.19),
			anti_roll_k(8000), damper_factors(1), spring_factors(1),
			camber(-0.5), caster(0.28), toe(0),
			overtravel(0), displacement(0), velocity(0), force(0) {}

	void DebugPrint(std::ostream & out);

	void SetHinge ( const MATHVECTOR<Dbl,3>& value )	{	hinge = value;	}
	const MATHVECTOR<Dbl,3> & GetHinge() const		{	return hinge;	}

	void SetBounce (const Dbl& value)	{	bounce = value;		}
	Dbl GetBounce() const				{	return bounce;		}

	void SetRebound (const Dbl& value)	{	rebound = value;	}
	Dbl GetRebound() const				{	return rebound;		}

	void SetTravel (const Dbl& value)	{	travel = value;		}
	Dbl GetTravel() const				{	return travel;		}

	void SetAntiRollK (const Dbl& value){	anti_roll_k = value;	}
	Dbl GetAntiRollK() const			{	return anti_roll_k;		}

	void SetCamber (const Dbl& value)	{	camber = value;		}
	Dbl GetCamber() const				{	return camber;		}

	void SetCaster (const Dbl& value)	{	caster = value;		}
	Dbl GetCaster() const				{	return caster;		}

	void SetToe (const Dbl& value)		{	toe = value;	}
	Dbl GetToe() const					{	return toe;		}

	void SetSpringConstant (const Dbl& value)	{	spring_constant = value;	}
	Dbl GetSpringConstant() const				{	return spring_constant;		}

	const Dbl & GetDisplacement() const	{	return displacement;	}
	Dbl GetOvertravel() const			{	return overtravel;	}

	///Return the displacement in percent of max travel where 0.0 is fully extended and 1.0 is fully compressed
	Dbl GetDisplacementPercent() const
	{
		return displacement / travel;
	}

	///compute the suspension force for the given time interval and external displacement
	Dbl Update(Dbl dt, Dbl ext_displacement);

	const Dbl GetForce(Dbl displacement, Dbl velocity);

	const Dbl & GetVelocity() const	{	return velocity;	}

	void SetDamperFactorPoints(std::vector <std::pair <Dbl, Dbl> > & curve);
	void SetSpringFactorPoints(std::vector <std::pair <Dbl, Dbl> > & curve);
};
