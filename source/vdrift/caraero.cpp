#include "pch.h"
#include "caraero.h"
#include "../ogre/common/Def_Str.h"


void CARAERO::DebugPrint(std::ostream & out)
{
	out << "---" << std::endl;
	out << "Drag " << fToStr(drag_vector[0], 0,5) <<" "<< fToStr(drag_vector[1], 0,4) <<" "<< fToStr(drag_vector[2], 0,5) << std::endl;
	out << "Lift " << fToStr(lift_vector[0], 0,5) <<" "<< fToStr(lift_vector[1], 0,4) <<" "<< fToStr(lift_vector[2], 0,5) << std::endl;
}

MATHVECTOR<Dbl,3> CARAERO::GetForce(const MATHVECTOR<Dbl,3> & bodyspace_wind_vector, bool updStats) const
{
	//calculate drag force
	MATHVECTOR<Dbl,3> drag_vec = bodyspace_wind_vector * bodyspace_wind_vector.Magnitude() * 0.5 *
			air_density * drag_coefficient * drag_frontal_area;
	
	//calculate lift force and associated drag
	Dbl wind_speed = -bodyspace_wind_vector[0]; //positive wind speed when the wind is heading at us
	if (wind_speed < 0)
		wind_speed = -wind_speed * 0.2; //assume the surface doesn't generate much lift when in reverse
	const Dbl k = 0.5 * air_density * wind_speed * wind_speed;
	const Dbl lift = k * lift_coefficient * lift_surface_area;
	const Dbl drag = -lift_coefficient * lift * (1.0 -  lift_efficiency);
	MATHVECTOR<Dbl,3> lift_vec = MATHVECTOR<Dbl,3> (drag, 0, lift);
	
	MATHVECTOR<Dbl,3> force = drag_vec + lift_vec;
	if (updStats)
	{
		drag_vector = drag_vec;
		lift_vector = lift_vec;
	}	
	return force;
}
