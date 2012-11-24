#ifndef _CARTIRE_H
#define _CARTIRE_H

//#include <map>
#include <iostream>
#include <vector>
#include <cmath>

#include "dbl.h"
#include "joeserialize.h"
#include "macros.h"
#include "mathvector.h"
#include "../ogre/common/Defines.h"


#ifdef _WIN32
bool isnan(float number);
bool isnan(double number);
#endif


class TIRE_PARAMS		// loaded from .tire file
{
public:
	std::vector <Dbl> longitudinal; ///< the parameters of the longitudinal pacejka equation.  this is series b
	std::vector <Dbl> lateral; ///< the parameters of the lateral pacejka equation.  this is series a
	std::vector <Dbl> aligning; ///< the parameters of the aligning moment pacejka equation.  this is series c

	TIRE_PARAMS()
	{
		longitudinal.resize(11);
		lateral.resize(15);
		aligning.resize(18);
	}
};


class CARTIRE
{
friend class joeserialize::Serializer;
public://
	// constants
	Dbl rolling_resistance_linear;	///< linear rolling resistance on a hard surface
	Dbl rolling_resistance_quadratic;	///< quadratic rolling resistance on a hard surface
	TIRE_PARAMS* params;
	std::vector <Dbl> sigma_hat;	///< maximum grip in the longitudinal direction
	std::vector <Dbl> alpha_hat;	///< maximum grip in the lateral direction

	// variables
	Dbl feedback;	///< the force feedback effect value

	//for info only
	Dbl slide;	///< ratio of tire contact patch speed to road speed, minus one
	Dbl slip;	///< the angle (in degrees) between the wheel heading and the wheel's actual velocity
	Dbl slideratio;	///< ratio of the slide to the tire's optimim slide
	Dbl slipratio;	///< ratio of the slip to the tire's optimim slip

	void FindSigmaHatAlphaHat(Dbl load, Dbl & output_sigmahat, Dbl & output_alphahat, int iterations=400);

public:
	//  ctor
	CARTIRE()
		: slide(0), slip(0), params(0)
	{	}

	void DebugPrint(std::ostream & out);

	void LookupSigmaHatAlphaHat(Dbl normalforce, Dbl & sh, Dbl & ah) const;
	void CalculateSigmaHatAlphaHat(int tablesize=20);
	
	Dbl GetRollingResistance(const Dbl velocity, const Dbl normal_force, const Dbl rolling_resistance_factor) const;
	void SetRollingResistance(Dbl linear, Dbl quadratic)
	{
		rolling_resistance_linear = linear;
		rolling_resistance_quadratic = quadratic;
	}

	void SetPacejkaParameters(TIRE_PARAMS* params1)	{	params = params1;	}

	Dbl GetSlide() const	{	return slide;	}

	void SetFeedback(Dbl aligning_force)	{	feedback = aligning_force;	}
	Dbl GetFeedback() const					{	return feedback;	}


	/// Return the friction vector calculated from the magic formula.
	MATHVECTOR <Dbl, 3> GetForce (Dbl normal_force, Dbl friction_coeff, Dbl roll_friction_coeff,
					const MATHVECTOR <Dbl, 3> & hub_velocity, Dbl patch_speed, Dbl current_camber);

	bool Serialize(joeserialize::Serializer & s)
	{
		_SERIALIZE_(s,feedback);
		return true;
	}

	Dbl GetMaximumFx (Dbl load) const;
	Dbl GetMaximumFy (Dbl load, Dbl current_camber) const;
	Dbl GetMaximumMz (Dbl load, Dbl current_camber) const;

	Dbl Pacejka_Fx (Dbl sigma, Dbl Fz, Dbl friction_coeff, Dbl & maxforce_output);
	Dbl Pacejka_Fy (Dbl alpha, Dbl Fz, Dbl gamma, Dbl friction_coeff, Dbl & maxforce_output);
	Dbl Pacejka_Mz (Dbl sigma, Dbl alpha, Dbl Fz, Dbl gamma, Dbl friction_coeff, Dbl & maxforce_output);
	Dbl GetOptimumSteeringAngle(Dbl load) const;

	///  return the slide and slip ratios as a percentage of optimum
	std::pair <Dbl, Dbl> GetSlideSlipRatios() const
	{
		return std::make_pair(slideratio, slipratio);
	}
};

#endif
