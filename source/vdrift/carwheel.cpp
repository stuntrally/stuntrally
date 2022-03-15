#include "pch.h"
#include "carwheel.h"
#include "../ogre/common/Def_Str.h"


void CARWHEEL::DebugPrint(std::ostream & out)
{
	//out << "---Wheel---" << std::endl;
	//out << "Speed: " << fToStr(GetRPM(), 1,4) << std::endl;
	out << "Steer  " << fToStr(steer_angle, 1,5) << std::endl;
	//out << "SteerMax " << fToStr(steer_max, 1,5) << std::endl;
	out << "Camber " << fToStr(camber_deg, 1,5)  << std::endl;
	//out << "Slide|ratio:" << fToStr(slips.slideratio, 2,5) << std::endl;
	//out << "Slip-angle:" << fToStr(slips.slipratio, 2,6) << std::endl;
	out << "Slide|  " << fToStr(slips.slide, 2,5) << std::endl;
	out << "SlipA- " << fToStr(slips.slip, 2,6) << std::endl;
}

void CARWHEEL::SetInertia(Dbl new_inertia)
{
	inertia_cache = new_inertia;
	MATRIX3 <Dbl> inertia;
	inertia.Scale(new_inertia);
	rotation.SetInertia(inertia);
}

void CARWHEEL::SetTorque(const Dbl torque)
{
	MATHVECTOR<Dbl,3> v(0, torque, 0);
	rotation.SetTorque(v);
	angvel = GetAngularVelocity();
}

/*void CARWHEEL::SetAdditionalInertia (const Dbl& value)
{
	additional_inertia = value;
	
	MATRIX3 <Dbl> inertia;
	inertia.Scale(inertia_cache + additional_inertia);
	rotation.SetInertia(inertia);
	
	//std::cout << inertia_cache << " + " << additional_inertia << " = " << inertia_cache + additional_inertia << std::endl;
}*/

Dbl CARWHEEL::GetRollingResistance(const Dbl velocity, const Dbl rolling_resistance_factor) const
{
	// surface influence on rolling resistance
	Dbl rolling_resistance = rolling_res_lin * rolling_resistance_factor;

	// heat due to tire deformation increases rolling resistance
	// approximate by quadratic function
	rolling_resistance += velocity * velocity * rolling_res_quad;

	// rolling resistance direction
	Dbl resistance = rolling_resistance;
	if (velocity < 0)  resistance = -resistance;

	return resistance;
}
