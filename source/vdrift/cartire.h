#pragma once
#include <iostream>
#include <vector>
#include <cmath>

#include "dbl.h"
#include "mathvector.h"
#include "carwheel.h"


class CARTIRE	// loaded from .tire file
{
public:
	// constants
	std::vector <Dbl> longitudinal;	///< the parameters of the longitudinal pacejka equation.  this is series b
	std::vector <Dbl> lateral;		///< the parameters of the lateral pacejka equation.  this is series a
	std::vector <Dbl> aligning;		///< the parameters of the aligning moment pacejka equation.  this is series c

	std::vector <Dbl> sigma_hat;	///< maximum grip in the longitudinal direction
	std::vector <Dbl> alpha_hat;	///< maximum grip in the lateral direction

	// no variables - 1 tire for all wheels
	std::string name;  // .tire file for info
	int user;  // 1user/0orig file

	void FindSigmaHatAlphaHat(Dbl load, Dbl & output_sigmahat, Dbl & output_alphahat, int iterations = 400);

public:
	//  ctor
	CARTIRE()
	{
		longitudinal.resize(11, 0.0);
		lateral.resize(15, 0.0);
		aligning.resize(18, 0.0);
		user = 0;
	}
	
	void LookupSigmaHatAlphaHat(Dbl normalforce, Dbl & sh, Dbl & ah) const;
	void CalculateSigmaHatAlphaHat(int tablesize = 20);


	/// Return the friction vector calculated from the magic formula.
	MATHVECTOR<Dbl,3> GetForce (Dbl normal_force, Dbl friction_coeff, //Dbl roll_friction_coeff,
					const MATHVECTOR<Dbl,3> & hub_velocity, Dbl patch_speed, Dbl current_camber,
					CARWHEEL::SlideSlip* slips) const;  //out

	Dbl GetMaximumFx (Dbl load) const;
	Dbl GetMaximumFy (Dbl load, Dbl current_camber) const;
	Dbl GetMaximumMz (Dbl load, Dbl current_camber) const;

	Dbl Pacejka_Fx (Dbl sigma,				Dbl Fz,			   Dbl friction_coeff, Dbl & maxforce_output) const;
	Dbl Pacejka_Fy (Dbl alpha,				Dbl Fz,	Dbl gamma, Dbl friction_coeff, Dbl & maxforce_output) const;
	Dbl Pacejka_Mz (Dbl sigma, Dbl alpha,	Dbl Fz, Dbl gamma, Dbl friction_coeff, Dbl & maxforce_output) const;
	Dbl GetOptimumSteeringAngle(Dbl load) const;

	static /*const*/CARTIRE * None();  // default, inited, won't crash, not to be used in game
};
