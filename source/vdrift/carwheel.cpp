#include "pch.h"
#include "carwheel.h"


void CARWHEEL::DebugPrint(std::ostream & out)
{
	//out << "---Wheel---" << std::endl;
	//out << "Speed: " << fToStr(GetRPM(), 1,4) << std::endl;
	out << "Steer  " << fToStr(steer_angle, 1,5) << std::endl;
	out << "Camber " << fToStr(camber_deg, 1,5)  << std::endl;
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
	MATHVECTOR <Dbl, 3> v(0, torque, 0);
	rotation.SetTorque(v);
	angvel = GetAngularVelocity();
}

bool CARWHEEL::Serialize(joeserialize::Serializer & s)
{
	_SERIALIZE_(s,inertia_cache);
	_SERIALIZE_(s,steer_angle);
	return true;
}

void CARWHEEL::SetAdditionalInertia (const Dbl& value)
{
	additional_inertia = value;
	
	MATRIX3 <Dbl> inertia;
	inertia.Scale(inertia_cache + additional_inertia);
	rotation.SetInertia(inertia);
	
	//std::cout << inertia_cache << " + " << additional_inertia << " = " << inertia_cache + additional_inertia << std::endl;
}
