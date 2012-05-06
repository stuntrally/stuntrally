#ifndef _CARTIRE_H
#define _CARTIRE_H

#include "joeserialize.h"
#include "macros.h"

#include <map>
#include <iostream>
#include <vector>

#include <cmath>

#ifdef _WIN32
bool isnan(float number);
bool isnan(double number);
#endif

//#include "../ogre/common/Defines.h"


template <typename T>
class CARTIRE
{
friend class joeserialize::Serializer;
private:
public://
	//constants (not actually declared as const because they can be changed after object creation)
	T radius; ///< the total radius of the tire
	T tread; ///< 1.0 means a pure off-road tire, 0.0 is a pure road tire
	T rolling_resistance_linear; ///< linear rolling resistance on a hard surface
	T rolling_resistance_quadratic; ///< quadratic rolling resistance on a hard surface
	std::vector <T> longitudinal_parameters; ///< the parameters of the longitudinal pacejka equation.  this is series b
	std::vector <T> transverse_parameters; ///< the parameters of the lateral pacejka equation.  this is series a
	std::vector <T> aligning_parameters; ///< the parameters of the aligning moment pacejka equation.  this is series c
	std::vector <T> sigma_hat; ///< maximum grip in the longitudinal direction
	std::vector <T> alpha_hat; ///< maximum grip in the lateral direction

	//variables
	T feedback; ///< the force feedback effect value

	//for info only
	T slide; ///< ratio of tire contact patch speed to road speed, minus one
	T slip; ///< the angle (in degrees) between the wheel heading and the wheel's actual velocity
	T slideratio; ///< ratio of the slide to the tire's optimim slide
	T slipratio; ///< ratio of the slip to the tire's optimim slip

	void FindSigmaHatAlphaHat(T load, T & output_sigmahat, T & output_alphahat, int iterations=400)
	{
		T x, y, ymax, junk;
		ymax = 0;
		for (x = -2; x < 2; x += 4.0/iterations)
		{
			y = Pacejka_Fx(x, load, 1.0, junk);
			if (y > ymax)
			{
				output_sigmahat = x;
				ymax = y;
			}
		}

		ymax = 0;
		for (x = -20; x < 20; x += 40.0/iterations)
		{
			y = Pacejka_Fy(x, load, 0, 1.0, junk);
			if (y > ymax)
			{
				output_alphahat = x;
				ymax = y;
			}
		}
	}

public:
	//default constructor makes an S2000-like car
	CARTIRE() : slide(0),slip(0) {longitudinal_parameters.resize(11);transverse_parameters.resize(15);aligning_parameters.resize(18);}

	void DebugPrint(std::ostream & out)
	{
		out << "---Tire---" << std::endl;
		out << "Slide ratio: " << slide << std::endl;
		out << "Slip angle: " << slip << std::endl;
	}

	void LookupSigmaHatAlphaHat(T normalforce, T & sh, T & ah) const
	{
		assert(!sigma_hat.empty());
		assert(!alpha_hat.empty());
		assert(sigma_hat.size() == alpha_hat.size());

		int HAT_ITERATIONS = sigma_hat.size();

		T HAT_LOAD = 0.5;
		T nf = normalforce * 0.001;
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
			T blend;
			lbound = (int)(nf/HAT_LOAD);
			lbound--;
			if (lbound < 0)
				lbound = 0;
			blend = (nf-HAT_LOAD*(lbound+1))/HAT_LOAD;
			sh = sigma_hat[lbound]*(1.0-blend)+sigma_hat[lbound+1]*blend;
			ah = alpha_hat[lbound]*(1.0-blend)+alpha_hat[lbound+1]*blend;
		}
	}

	void SetRadius ( const T& value )
	{
		radius = value;
	}


	T GetRadius() const
	{
		return radius;
	}

	void SetTread ( const T& value )
	{
		tread = value;
	}


	T GetTread() const
	{
		return tread;
	}

	void SetRollingResistance(T linear, T quadratic)
	{
		rolling_resistance_linear = linear;
		rolling_resistance_quadratic = quadratic;
	}

	void SetPacejkaParameters(const std::vector <T> & longitudinal, const std::vector <T> & lateral, const std::vector <T> & aligning)
	{
		assert(longitudinal.size() == 11);
		assert(lateral.size() == 15);
		assert(aligning.size() == 18);
		assert(longitudinal_parameters.size() == 11);
		assert(transverse_parameters.size() == 15);
		assert(aligning_parameters.size() == 18);

		longitudinal_parameters = longitudinal;
		transverse_parameters = lateral;
		aligning_parameters = aligning;
	}

	void SetSlide ( const T& value )
	{
		slide = value;
	}


	T GetSlide() const
	{
		return slide;
	}

	void SetSlip ( const T& value )
	{
		slip = value;
	}


	T GetSlip() const
	{
		return slip;
	}

	/// Return the friction vector calculated from the magic formula.
	/// HUB_VELOCITY is the velocity vector of the wheel's reference
	/// frame.  PATCH_SPEED is the rearward speed of the contact patch
	/// with respect to the wheel's frame.
	/// current_camber is expected in radians.
	/// normal_force is in units N.
	MATHVECTOR <T, 3> GetForce(
					T normal_force,
					T friction_coeff,
					T roll_friction_coeff,
					const MATHVECTOR <T, 3> & hub_velocity,
					T patch_speed,
					T current_camber)

	{
		assert(friction_coeff > 0);

		T sigma_hat(0);
		T alpha_hat(0);

		LookupSigmaHatAlphaHat(normal_force, sigma_hat, alpha_hat);

		//std::cout << hub_velocity << " -- " << patch_speed << std::endl;

		T Fz = normal_force * 0.001;

		//cap Fz at a magic number to prevent explosions
		if (Fz > 30)
			Fz = 30;

		//std::cout << normal_force << std::endl;
		const T EPSILON = 1e-6;
		if (Fz < EPSILON)
		{
			MATHVECTOR <T, 3> zero(0);
			//std::cout << "Tire off ground detected: " << normal_force << ", " << Fz << std::endl;
			return zero;
		}

		T sigma = 0.0;
		T tan_alpha = 0.0;
		T alpha = 0.0;

		T V = hub_velocity[0];

		T denom = std::max ( std::abs ( V ), 0.1 );

		sigma = ( patch_speed - V ) /denom;

		tan_alpha = hub_velocity [1] / denom;

		alpha = - ( atan2 ( hub_velocity[1],denom ) ) * 180.0/3.141593;

		/*crash dyn obj--*/
		if (isnan(alpha) || isnan(1.f/sigma_hat))
		{
			MATHVECTOR <T, 3> outvec(0, 0, 0);
			return outvec;	}
		
		assert(!isnan(alpha));

		T gamma = ( current_camber ) * 180.0/3.141593;

		//beckman method for pre-combining longitudinal and lateral forces
		T s = sigma / sigma_hat;
		assert(!isnan(s));
		T a = alpha / alpha_hat;
		assert(!isnan(a));
		T rho = std::max ( sqrt ( s*s+a*a ), 0.0001); //the constant is arbitrary; just trying to avoid divide-by-zero
		assert(!isnan(rho));

		T max_Fx(0);
		T Fx = ( s / rho ) *Pacejka_Fx ( rho*sigma_hat, Fz, friction_coeff, max_Fx );
		//std::cout << "s=" << s << ", rho=" << rho << ", sigma_hat=" << sigma_hat << ", Fz=" << Fz << ", friction_coeff=" << friction_coeff << ", Fx=" << Fx << std::endl;
		assert(!isnan(Fx));
		T max_Fy(0);
		T Fy = ( a / rho ) *Pacejka_Fy ( rho*alpha_hat, Fz, gamma, friction_coeff, max_Fy );
		//std::cout << "s=" << s << ", a=" << a << ", rho=" << rho << ", Fy=" << Fy << std::endl;
		assert(!isnan(Fy));
		T max_Mz(0);
		T Mz = Pacejka_Mz ( sigma, alpha, Fz, gamma, friction_coeff, max_Mz );

		//T slip_x = -sigma / ( 1.0 + generic_abs ( sigma ) );
		//T slip_y = tan_alpha / ( 1.0+generic_abs ( sigma-1.0 ) );
		//T total_slip = std::sqrt ( slip_x * slip_x + slip_y * slip_y );

		//T maxforce = longitudinal_parameters[2] * 7.0;
		//std::cout << maxforce << ", " << max_Fx << ", " << max_Fy << ", " << Fx << ", " << Fy << std::endl;

		//combining method 0: no combining! :-)

		//combining method 1: traction circle
		//determine to what extent the tires are long (x) gripping vs lat (y) gripping
		float longfactor = 1.0;
		float combforce = std::abs(Fx)+std::abs(Fy);
		if (combforce > 1) //avoid divide by zero (assume longfactor = 1 for this case)
			longfactor = std::abs(Fx)/combforce; //1.0 when Fy is zero, 0.0 when Fx is zero
		//determine the maximum force for this amount of long vs lat grip
		float maxforce = std::abs(max_Fx)*longfactor + (1.0-longfactor)*std::abs(max_Fy); //linear interpolation
		if (combforce > maxforce) //cap forces
		{
			//scale down forces to fit into the maximum
			Fx *= maxforce / combforce;
			Fy *= maxforce / combforce;
			assert(!isnan(Fx));
			assert(!isnan(Fy));
			//std::cout << "Limiting " << combforce << " to " << maxforce << std::endl;
		}/**/

		//combining method 2: traction ellipse (prioritize Fx)
		//std::cout << "Fy0=" << Fy << ", ";
		/*if (Fx >= max_Fx)
		{
			Fx = max_Fx;
			Fy = 0;
		}
		else
			Fy = Fy*sqrt(1.0-(Fx/max_Fx)*(Fx/max_Fx));/**/
		//std::cout << "Fy=" << Fy << ", Fx=Fx0=" << Fx << ", Fxmax=" << max_Fx << ", Fymax=" << max_Fy << std::endl;

		//combining method 3: traction ellipse (prioritize Fy)
		/*if (Fy >= max_Fy)
		{
			Fy = max_Fy;
			Fx = 0;
		}
		else
		{
			T scale = sqrt(1.0-(Fy/max_Fy)*(Fy/max_Fy));
			if (isnan(scale))
				Fx = 0;
			else
				Fx = Fx*scale;
		}/**/

		// rolling resistance (broken)  /// todo: is it fixed ?--
		//Fx += GetRollingResistance(hub_velocity[0] / radius, normal_force, roll_friction_coeff);

		assert(!isnan(Fx));
		assert(!isnan(Fy));

		/*if ( hub_velocity.Magnitude () < 0.1 )
		{
			slide = 0.0;
		}
		else
		{
			slide = total_slip;
			if ( slide > 1.0 )
				slide = 1.0;
		}*/

		slide = sigma;
		slip = alpha;
		slideratio = s;
		slipratio = a;

		//std::cout << slide << ", " << slip << std::endl;

		MATHVECTOR <T, 3> outvec(Fx, Fy, Mz);
		return outvec;
	}

	void SetFeedback(T aligning_force)
	{
		feedback = aligning_force;
	}

	T GetRollingResistance(const T velocity, const T normal_force, const T rolling_resistance_factor) const
	{
		// surface influence on rolling resistance
		T rolling_resistance = rolling_resistance_linear * rolling_resistance_factor;
		
		// heat due to tire deformation increases rolling resistance
		// approximate by quadratic function
		rolling_resistance += velocity * velocity * rolling_resistance_quadratic;
		
		// rolling resistance magnitude
		T resistance = -normal_force * rolling_resistance;
		if (velocity < 0) resistance = -resistance;
		
		return resistance;
	}

	void CalculateSigmaHatAlphaHat(int tablesize=20)
	{
		T HAT_LOAD = 0.5;
		sigma_hat.resize(tablesize, 0);
		alpha_hat.resize(tablesize, 0);
		for (int i = 0; i < tablesize; i++)
		{
			FindSigmaHatAlphaHat((T)(i+1)*HAT_LOAD, sigma_hat[i], alpha_hat[i]);
		}
	}

	bool Serialize(joeserialize::Serializer & s)
	{
		_SERIALIZE_(s,feedback);
		return true;
	}

	T GetFeedback() const
	{
		return feedback;
	}

	///load is the normal force in newtons.
	T GetMaximumFx(T load) const
	{
		const std::vector <T>& b = longitudinal_parameters;
		T Fz = load * 0.001;
		return ( b[1]*Fz + b[2] ) *Fz;
	}

	///load is the normal force in newtons.
	T GetMaximumFy(T load, T current_camber) const
	{
		const std::vector <T>& a = transverse_parameters;
		T Fz = load * 0.001;
		T gamma = ( current_camber ) * 180.0/3.141593;

		T D = ( a[1]*Fz+a[2] ) *Fz;
		T Sv = ( ( a[11]*Fz+a[12] ) *gamma + a[13] ) *Fz+a[14];

		return D+Sv;
	}

	///load is the normal force in newtons.
	T GetMaximumMz(T load, T current_camber) const
	{
		const std::vector <T>& c = aligning_parameters;
		T Fz = load * 0.001;
		T gamma = ( current_camber ) * 180.0/3.141593;

		T D = ( c[1]*Fz+c[2] ) *Fz;
		T Sv = ( c[14]*Fz*Fz+c[15]*Fz ) *gamma+c[16]*Fz + c[17];

		return -(D+Sv);
	}

	/// pacejka magic formula function, longitudinal
	T Pacejka_Fx ( T sigma, T Fz, T friction_coeff, T & maxforce_output )
	{
		const std::vector <T>& b = longitudinal_parameters;

		T D = ( b[1]*Fz + b[2] ) *Fz*friction_coeff;
		assert ( b[0]* ( b[1]*Fz+b[2] ) != 0 );
		T B = ( b[3]*Fz+b[4] ) *exp ( -b[5]*Fz ) / ( b[0]* ( b[1]*Fz+b[2] ) );
		T E = ( b[6]*Fz*Fz+b[7]*Fz+b[8] );
		T S = ( 100*sigma + b[9]*Fz+b[10] );
		T Fx = D*sin ( b[0] * atan ( S*B+E* ( atan ( S*B )-S*B ) ) );

		maxforce_output = D;

		assert(!isnan(Fx));
		return Fx;
	}

	/// pacejka magic formula function, lateral
	T Pacejka_Fy ( T alpha, T Fz, T gamma, T friction_coeff, T & maxforce_output )
	{
		const std::vector <T>& a = transverse_parameters;

		T D = ( a[1]*Fz+a[2] ) *Fz*friction_coeff;
		T B = a[3]*sin ( 2.0*atan ( Fz/a[4] ) ) * ( 1.0-a[5]*std::abs ( gamma ) ) / ( a[0]* ( a[1]*Fz+a[2] ) *Fz );
		assert(!isnan(B));
		T E = a[6]*Fz+a[7];
		T S = alpha + a[8]*gamma+a[9]*Fz+a[10];
		T Sv = ( ( a[11]*Fz+a[12] ) *gamma + a[13] ) *Fz+a[14];
		T Fy = D*sin ( a[0]*atan ( S*B+E* ( atan ( S*B )-S*B ) ) ) +Sv;
		
		maxforce_output = D+Sv;

		//LogO("Fy: "+fToStr(alpha,4,6)+" "+fToStr(Fz,4,6)+" "+fToStr(gamma,4,6)+" "+fToStr(friction_coeff,4,6)+" "+fToStr(maxforce_output,4,6));

		assert(!isnan(Fy));
		return Fy;
	}

	/// pacejka magic formula function, aligning
	T Pacejka_Mz ( T sigma, T alpha, T Fz, T gamma, T friction_coeff, T & maxforce_output )
	{
		const std::vector <T>& c = aligning_parameters;

		T D = ( c[1]*Fz+c[2] ) *Fz*friction_coeff;
		T B = ( c[3]*Fz*Fz+c[4]*Fz ) * ( 1.0-c[6]*std::abs ( gamma ) ) *exp ( -c[5]*Fz ) / ( c[0]*D );
		T E = ( c[7]*Fz*Fz+c[8]*Fz+c[9] ) * ( 1.0-c[10]*std::abs ( gamma ) );
		T S = alpha + c[11]*gamma+c[12]*Fz+c[13];
		T Sv = ( c[14]*Fz*Fz+c[15]*Fz ) *gamma+c[16]*Fz + c[17];
		T Mz = D*sin ( c[0]*atan ( S*B+E* ( atan ( S*B )-S*B ) ) ) +Sv;

		maxforce_output = D+Sv;

		assert(!isnan(Mz));
		return Mz;
	}

	bool operator==(const CARTIRE <T> & other) const
	{
		return (longitudinal_parameters == other.longitudinal_parameters &&
				transverse_parameters == other.transverse_parameters &&
				aligning_parameters == other.aligning_parameters);
	}

	/// optimum steering angle in degrees given load in newtons
	T GetOptimumSteeringAngle(T load) const
	{
		T sigma_hat(0);
		T alpha_hat(0);

		LookupSigmaHatAlphaHat(load, sigma_hat, alpha_hat);

		return alpha_hat;
	}

	///return the slide and slip ratios as a percentage of optimum
	std::pair <T, T> GetSlideSlipRatios() const
	{
		return std::make_pair(slideratio, slipratio);
	}
};

#endif
