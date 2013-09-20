#pragma once
#include "dbl.h"
#include "mathvector.h"


class CARAERO
{
private:
	// constants
	Dbl air_density;	///the current air density in kg/m^3
	Dbl drag_frontal_area;	///the projected frontal area in m^2, used for drag calculations
	Dbl drag_coefficient;	///the drag coefficient, a unitless measure of aerodynamic drag
	Dbl lift_surface_area;	///the wing surface area in m^2
	Dbl lift_coefficient;	///a unitless lift coefficient
	Dbl lift_efficiency;	///the efficiency of the wing, a unitless value from 0.0 to 1.0
	MATHVECTOR<Dbl,3> position;	///the position that the drag and lift forces are applied on the body
	
	// variables
	
	//for info only
	mutable MATHVECTOR<Dbl,3> lift_vector;
	mutable MATHVECTOR<Dbl,3> drag_vector;
	
	
public:
	//default constructor makes an aerodynamically transparent device (i.e. no drag or lift)
	CARAERO() : air_density(1.2), drag_frontal_area(0), drag_coefficient(0),
		lift_surface_area(0), lift_coefficient(0), lift_efficiency(0)
	{	}

	void DebugPrint(std::ostream & out);
	
	void Set(const MATHVECTOR<Dbl,3> & newpos, Dbl new_drag_frontal_area, Dbl new_drag_coefficient,
		Dbl new_lift_surface_area, Dbl new_lift_coefficient, Dbl new_lift_efficiency)
	{
		position = newpos;
		drag_frontal_area = new_drag_frontal_area;
		drag_coefficient = new_drag_coefficient;
		lift_surface_area = new_lift_surface_area;
		lift_coefficient = new_lift_coefficient;
		lift_efficiency = new_lift_efficiency;
	}

	const MATHVECTOR<Dbl,3> & GetPosition() const
	{
		return position;
	}

	MATHVECTOR<Dbl,3> GetForce(const MATHVECTOR<Dbl,3> & bodyspace_wind_vector, bool updStats=true) const;
	
	Dbl GetAerodynamicDownforceCoefficient() const
	{
		return 0.5 * air_density * lift_coefficient * lift_surface_area;
	}
	
	Dbl GetAeordynamicDragCoefficient() const
	{
		return 0.5 * air_density * (drag_coefficient * drag_frontal_area + lift_coefficient * lift_coefficient * lift_surface_area * (1.0-lift_efficiency));
	}
	
	MATHVECTOR<Dbl,3> GetLiftVector() const	{	return lift_vector;		}
	MATHVECTOR<Dbl,3> GetDragVector() const	{	return drag_vector;		}
};
