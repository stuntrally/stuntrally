#include "pch.h"
#include "carsuspension.h"
#include "../ogre/common/Def_Str.h"


void CARSUSPENSION::DebugPrint(std::ostream & out)
{
	//out << "---Suspension---" << std::endl;
	out << "Disp  " << fToStr(displacement, 2,4) << std::endl;
	out << "Vel  " << fToStr(velocity, 2,5) << std::endl;
	//always 1?..
	//out << "Spring " << fToStr(spring_factors.Interpolate(displacement), 2,4) << std::endl;
	//out << "Damp   " << fToStr(damper_factors.Interpolate(std::abs(velocity)), 2,4) << std::endl;
}

///compute the suspension force for the given time interval and external displacement
Dbl CARSUSPENSION::Update(Dbl dt, Dbl ext_displacement)
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

	/*const Dbl inv_mass = 1/20.0;
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

const Dbl CARSUSPENSION::GetForce(Dbl displacement, Dbl velocity)
{
	Dbl damping = bounce;
	if (velocity < 0) damping = rebound;

	//compute damper factor based on curve
	Dbl dampfactor = damper_factors.Interpolate(std::abs(velocity));

	//compute spring factor based on curve
	Dbl springfactor = spring_factors.Interpolate(displacement);
	//LogO("sus: "+fToStr(springfactor,2,4)+" dmp: "+fToStr(dampfactor,2,4));

	Dbl spring_force = -displacement * spring_constant * springfactor; //when compressed, the spring force will push the car in the positive z direction
	Dbl damp_force = -velocity * damping * dampfactor; //when compression is increasing, the damp force will push the car in the positive z direction
	Dbl force = spring_force + damp_force;

	return force;
}

void CARSUSPENSION::SetDamperFactorPoints(std::vector <std::pair <Dbl, Dbl> > & curve)
{
	for (auto& i : curve)
	{
		damper_factors.AddPoint(i.first, i.second);
		//LogO("sus damper "+fToStr(i->first,2,4)+" "+fToStr(i->second,2,4));
	}
	//LogO("sus d--");
}

void CARSUSPENSION::SetSpringFactorPoints(std::vector <std::pair <Dbl, Dbl> > & curve)
{
	for (auto& i : curve)
	{
		spring_factors.AddPoint(i.first, i.second);
		//LogO("sus spring "+fToStr(i->first,2,4)+" "+fToStr(i->second,2,4));
	}
	//LogO("sus s--");
}
