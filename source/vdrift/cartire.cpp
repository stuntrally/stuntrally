#include "pch.h"
#include "cartire.h"
#include "cardefs.h"
//#include "../ogre/common/Def_Str.h"
#include "isnan.h"


void CARTIRE::FindSigmaHatAlphaHat(Dbl load, Dbl & output_sigmahat, Dbl & output_alphahat, int iterations)
{
	Dbl x, y, ymax, junk, x4 = 4.0/iterations, x40 = 40.0/iterations;
	ymax = 0;
	for (x = -2; x < 2; x += x4)
	{
		y = Pacejka_Fx(x, load, 1.0, junk);
		if (y > ymax)
		{
			output_sigmahat = x;
			ymax = y;
		}
	}
	ymax = 0;
	for (x = -20; x < 20; x += x40)
	{
		y = Pacejka_Fy(x, load, 0, 1.0, junk);
		if (y > ymax)
		{
			output_alphahat = x;
			ymax = y;
		}
	}
}

void CARTIRE::LookupSigmaHatAlphaHat(Dbl normalforce, Dbl & sh, Dbl & ah) const
{
	assert(!sigma_hat.empty());
	assert(!alpha_hat.empty());
	assert(sigma_hat.size() == alpha_hat.size());

	int HAT_ITERATIONS = sigma_hat.size();

	Dbl HAT_LOAD = 0.5;
	Dbl nf = normalforce * 0.001;
	if (nf < HAT_LOAD)
	{
		sh = sigma_hat[0];
		ah = alpha_hat[0];
	}
	else if (nf >= HAT_LOAD*HAT_ITERATIONS)
	{
		sh = sigma_hat[HAT_ITERATIONS-1];
		ah = alpha_hat[HAT_ITERATIONS-1];
	}
	else
	{
		int lbound;
		Dbl blend;
		lbound = (int)(nf/HAT_LOAD);
		lbound--;
		if (lbound < 0)
			lbound = 0;
		blend = (nf-HAT_LOAD*(lbound+1))/HAT_LOAD;
		sh = sigma_hat[lbound]*(1.0-blend)+sigma_hat[lbound+1]*blend;
		ah = alpha_hat[lbound]*(1.0-blend)+alpha_hat[lbound+1]*blend;
	}
}


/// Return the friction vector calculated from the magic formula.
/// HUB_VELOCITY is the velocity vector of the wheel's reference
/// frame.  PATCH_SPEED is the rearward speed of the contact patch
/// with respect to the wheel's frame.
/// current_camber is expected in radians.
/// normal_force is in units N.
//-------------------------------------------------------------------------------------------------------------------------------
MATHVECTOR<Dbl,3> CARTIRE::GetForce(
		Dbl normal_force,
		Dbl friction_coeff,
		//Dbl roll_friction_coeff,
		const MATHVECTOR<Dbl,3> & hub_velocity,
		Dbl patch_speed,
		Dbl current_camber,
		CARWHEEL::SlideSlip* slips) const
{
	assert(friction_coeff > 0);

	Dbl sigma_hat(0);
	Dbl alpha_hat(0);

	LookupSigmaHatAlphaHat(normal_force, sigma_hat, alpha_hat);

	//std::cout << hub_velocity << " -- " << patch_speed << std::endl;

	Dbl Fz = normal_force * 0.001;

	//cap Fz at a magic number to prevent explosions
	if (Fz > 30)
		Fz = 30;

	const Dbl EPSILON = 1e-6;
	if (Fz < EPSILON)
	{
		MATHVECTOR<Dbl,3> zero(0);
		return zero;
	}

	Dbl sigma = 0.0;
	Dbl tan_alpha = 0.0;
	Dbl alpha = 0.0;

	Dbl V = hub_velocity[0];
	Dbl denom = std::max(std::abs(V), 0.01);

	sigma = (patch_speed - V) / denom;
	tan_alpha = hub_velocity[1] / denom;
	alpha = -atan2(hub_velocity[1], denom) * 180.0/PI_d;

	/*crash dyn obj--*/
	if (isnan(alpha) || isnan(1.f/sigma_hat))
	{
		MATHVECTOR<Dbl,3> outvec(0, 0, 0);
		return outvec;
	}
	assert(!isnan(alpha));

	Dbl gamma = current_camber * 180.0/PI_d;

	//  beckman method for pre-combining longitudinal and lateral forces
	Dbl s = sigma / sigma_hat;  assert(!isnan(s));
	Dbl a = alpha / alpha_hat;  assert(!isnan(a));

	Dbl rho = std::max( sqrt( s*s+a*a ), 0.0001);  //avoid divide by zero
	assert(!isnan(rho));

	Dbl max_Fx(0), max_Fy(0), max_Mz(0);
	Dbl Fx = (s / rho) * Pacejka_Fx( rho*sigma_hat, Fz,        friction_coeff, max_Fx );  assert(!isnan(Fx));
	Dbl Fy = (a / rho) * Pacejka_Fy( rho*alpha_hat, Fz, gamma, friction_coeff, max_Fy );  assert(!isnan(Fy));
	Dbl Mz = Pacejka_Mz( sigma, alpha, Fz, gamma, friction_coeff, max_Mz );

	if (slips)  // out vis
	{	slips->preFx = Fx;
		slips->preFy = Fy;
	}
	//Dbl slip_x = -sigma / ( 1.0 + generic_abs ( sigma ) );
	//Dbl slip_y = tan_alpha / ( 1.0+generic_abs ( sigma-1.0 ) );
	//Dbl total_slip = std::sqrt ( slip_x * slip_x + slip_y * slip_y );
	//Dbl maxforce = longitudinal_parameters[2] * 7.0;

	//  combining method 0: no combining

	//  combining method 1: traction circle
	//  determine to what extent the tires are long (x) gripping vs lat (y) gripping
	float longfactor = 1.0;
	float combforce = std::abs(Fx)+std::abs(Fy);
	if (combforce > 1)  // avoid divide by zero (assume longfactor = 1 for this case)
		longfactor = std::abs(Fx)/combforce;  // 1.0 when Fy is zero, 0.0 when Fx is zero
	//  determine the maximum force for this amount of long vs lat grip
	float maxforce = std::abs(max_Fx)*longfactor + (1.0-longfactor)*std::abs(max_Fy); //linear interpolation
	if (combforce > maxforce)  // cap forces
	{
		//scale down forces to fit into the maximum
		Dbl sc = maxforce / combforce;
		Fx *= sc;  assert(!isnan(Fx));  max_Fx *= sc;  //vis only
		Fy *= sc;  assert(!isnan(Fy));	max_Fy *= sc;
		//std::cout << "Limiting " << combforce << " to " << maxforce << std::endl;
	}/**/

	//  combining method 2: traction ellipse (prioritize Fx)
	//std::cout << "Fy0=" << Fy << ", ";
	/*if (Fx >= max_Fx)
	{
		Fx = max_Fx;
		Fy = 0;
	}else
		Fy = Fy*sqrt(1.0-(Fx/max_Fx)*(Fx/max_Fx));/**/
	//std::cout << "Fy=" << Fy << ", Fx=Fx0=" << Fx << ", Fxmax=" << max_Fx << ", Fymax=" << max_Fy << std::endl;

	//  combining method 3: traction ellipse (prioritize Fy)
	/*if (Fy >= max_Fy)
	{
		Fy = max_Fy;
		Fx = 0;
	}else
	{	Dbl scale = sqrt(1.0-(Fy/max_Fy)*(Fy/max_Fy));
		if (std::isnan(scale))
			Fx = 0;
		else
			Fx = Fx*scale;
	}/**/

	assert(!isnan(Fx));
	assert(!isnan(Fy));

	/*if ( hub_velocity.Magnitude () < 0.1 )
	{
		slide = 0.0;
	}else
	{	slide = total_slip;
		if ( slide > 1.0 )
			slide = 1.0;
	}*/

	if (slips)  // out vis
	{
		slips->slide = sigma;  slips->slideratio = s;
		slips->slip  = alpha;  slips->slipratio  = a;
		slips->fx_sr = s / rho;  slips->fx_rsr = rho*sigma_hat;
		slips->fy_ar = a / rho;  slips->fy_rar = rho*alpha_hat;
		slips->frict = friction_coeff;
		//Dbl Fx = (s / rho) * Pacejka_Fx( rho*sigma_hat, Fz, friction_coeff, max_Fx );
		//Dbl Fy = (a / rho) * Pacejka_Fy( rho*alpha_hat, Fz, gamma, friction_coeff, max_Fy );
		slips->Fx = Fx;  slips->Fxm = max_Fx;
		slips->Fy = Fy;  slips->Fym = max_Fy;
		slips->Fz = Fz;
	}

	//std::cout << slide << ", " << slip << std::endl;

	MATHVECTOR<Dbl,3> outvec(Fx, Fy, Mz);
	return outvec;
}
//-------------------------------------------------------------------------------------------------------------------------------


void CARTIRE::CalculateSigmaHatAlphaHat(int tablesize)
{
	Dbl HAT_LOAD = 0.5;
	sigma_hat.resize(tablesize, 0);
	alpha_hat.resize(tablesize, 0);
	for (int i = 0; i < tablesize; ++i)
		FindSigmaHatAlphaHat((Dbl)(i+1)*HAT_LOAD, sigma_hat[i], alpha_hat[i]);
}

//-------------------------------------------------------------------------------------------------------------------------------


///  load is the normal force in newtons
Dbl CARTIRE::GetMaximumFx(Dbl load) const
{
	const std::vector <Dbl>& b = longitudinal;
	Dbl Fz = load * 0.001;
	return (b[1]*Fz + b[2]) * Fz;
}

Dbl CARTIRE::GetMaximumFy(Dbl load, Dbl current_camber) const
{
	const std::vector <Dbl>& a = lateral;
	Dbl Fz = load * 0.001;
	Dbl gamma = current_camber * 180.0/PI_d;

	Dbl D = (a[1]*Fz + a[2]) *Fz;
	Dbl Sv = ((a[11]*Fz + a[12]) * gamma + a[13]) *Fz + a[14];

	return D+Sv;
}

Dbl CARTIRE::GetMaximumMz(Dbl load, Dbl current_camber) const
{
	const std::vector <Dbl>& c = aligning;
	Dbl Fz = load * 0.001;
	Dbl gamma = current_camber * 180.0/PI_d;

	Dbl D = (c[1]*Fz + c[2]) *Fz;
	Dbl Sv = (c[14]*Fz*Fz + c[15]*Fz) * gamma + c[16]*Fz + c[17];

	return -(D+Sv);
}

///  pacejka magic formula function
///  longitudinal
Dbl CARTIRE::Pacejka_Fx (Dbl sigma, Dbl Fz, Dbl friction_coeff, Dbl & maxforce_output) const
{
	const std::vector <Dbl>& b = longitudinal;

	Dbl D = (b[1]*Fz + b[2]) *Fz *friction_coeff;
	assert(b[0]* (b[1]*Fz + b[2]) != 0);
	Dbl B = ( b[3]*Fz+b[4] ) *exp ( -b[5]*Fz ) / ( b[0]* ( b[1]*Fz+b[2] ) );
	Dbl E = ( b[6]*Fz*Fz+b[7]*Fz+b[8] );
	Dbl S = ( 100*sigma + b[9]*Fz+b[10] );
	Dbl Fx = D*sin ( b[0] * atan ( S*B+E* ( atan ( S*B )-S*B ) ) );

	maxforce_output = D;

	assert(!isnan(Fx));
	return Fx;
}

///  lateral
Dbl CARTIRE::Pacejka_Fy (Dbl alpha, Dbl Fz, Dbl gamma, Dbl friction_coeff, Dbl & maxforce_output) const
{
	const std::vector <Dbl>& a = lateral;

	Dbl D = ( a[1]*Fz+a[2] ) *Fz*friction_coeff;
	Dbl B = a[3]*sin ( 2.0*atan ( Fz/a[4] ) ) * ( 1.0-a[5]*std::abs ( gamma ) ) / ( a[0]* ( a[1]*Fz+a[2] ) *Fz );
	assert(!isnan(B));
	Dbl E = a[6]*Fz+a[7];
	Dbl S = alpha + a[8]*gamma+a[9]*Fz+a[10];
	Dbl Sv = ( ( a[11]*Fz+a[12] ) *gamma + a[13] ) *Fz+a[14];
	Dbl Fy = D*sin ( a[0]*atan ( S*B+E* ( atan ( S*B )-S*B ) ) ) +Sv;
	
	maxforce_output = D+Sv;

	//LogO("Fy: "+fToStr(alpha,4,6)+" "+fToStr(Fz,4,6)+" "+fToStr(gamma,4,6)+" "+fToStr(friction_coeff,4,6)+" "+fToStr(maxforce_output,4,6));

	assert(!isnan(Fy));
	return Fy;
}

///  aligning
Dbl CARTIRE::Pacejka_Mz (Dbl sigma, Dbl alpha, Dbl Fz, Dbl gamma, Dbl friction_coeff, Dbl & maxforce_output) const
{
	const std::vector <Dbl>& c = aligning;

	Dbl D = ( c[1]*Fz+c[2] ) *Fz*friction_coeff;
	Dbl B = ( c[3]*Fz*Fz+c[4]*Fz ) * ( 1.0-c[6]*std::abs ( gamma ) ) *exp ( -c[5]*Fz ) / ( c[0]*D );
	Dbl E = ( c[7]*Fz*Fz+c[8]*Fz+c[9] ) * ( 1.0-c[10]*std::abs ( gamma ) );
	Dbl S = alpha + c[11]*gamma+c[12]*Fz+c[13];
	Dbl Sv = ( c[14]*Fz*Fz+c[15]*Fz ) *gamma+c[16]*Fz + c[17];
	Dbl Mz = D*sin ( c[0]*atan ( S*B+E* ( atan ( S*B )-S*B ) ) ) +Sv;

	maxforce_output = D+Sv;

	assert(!isnan(Mz));
	return Mz;
}


///  optimum steering angle in degrees given load in newtons
Dbl CARTIRE::GetOptimumSteeringAngle(Dbl load) const
{
	Dbl sigma_hat(0);
	Dbl alpha_hat(0);

	LookupSigmaHatAlphaHat(load, sigma_hat, alpha_hat);

	return alpha_hat;
}


//-------------------------------------------------------------------------------------------------------------------------------
/*const*/CARTIRE * CARTIRE::None()  // default, inited, won't crash, not to be used in game
{
	static CARTIRE s;
	static bool init = true;
	if (init)
	{	init = false;
		int i=0;
		s.lateral[i++] = 1.61;
		s.lateral[i++] = -0;
		s.lateral[i++] = 2775;
		s.lateral[i++] = 2220;
		s.lateral[i++] = 19.6;
		s.lateral[i++] = 0.013;
		s.lateral[i++] = -0.14;
		s.lateral[i++] = 0.14;
		s.lateral[i++] = 0.019;
		s.lateral[i++] = -0.019;
		s.lateral[i++] = -0.18;
		s.lateral[i++] = 0;
		s.lateral[i++] = 0;
		s.lateral[i++] = 0;
		s.lateral[i++] = 0;
		i = 0;
		s.longitudinal[i++] = 1.73;
		s.longitudinal[i++] = -0.49;
		s.longitudinal[i++] = 3439;
		s.longitudinal[i++] = 279;
		s.longitudinal[i++] = 470;
		s.longitudinal[i++] = 0;
		s.longitudinal[i++] = 0.0008;
		s.longitudinal[i++] = 0.005;
		s.longitudinal[i++] = -0.024;
		s.longitudinal[i++] = 0;
		s.longitudinal[i++] = 0;
		i = 0;
		s.aligning[i++] = 2.10;
		s.aligning[i++] = -3.9;
		s.aligning[i++] = -3.9;
		s.aligning[i++] = -1.26;
		s.aligning[i++] = -8.20;
		s.aligning[i++] = 0.025;
		s.aligning[i++] = 0;
		s.aligning[i++] = 0.044;
		s.aligning[i++] = -0.58;
		s.aligning[i++] = 0.18;
		s.aligning[i++] = 0.043;
		s.aligning[i++] = 0.048;
		s.aligning[i++] = -0.0035;
		s.aligning[i++] = -0.18;
		s.aligning[i++] = 0.14;
		s.aligning[i++] = -1.029;
		s.aligning[i++] = 0.27;
		s.aligning[i++] = -1.1;
		s.name = "None";
		s.user = 0;

		s.CalculateSigmaHatAlphaHat();
	}
	return &s;
}
