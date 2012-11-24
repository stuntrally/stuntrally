#include "pch.h"
#include "carbrake.h"


void CARBRAKE::DebugPrint(std::ostream & out)
{
	//out << "---Brake---" << std::endl;
	//out << "Brake    : " << brake_factor << std::endl;
	//out << "Handbrake: " << handbrake_factor << std::endl;
	out << "Torque " << fToStr(lasttorque, 0,5) << std::endl;
	out << "Locked  " << locked << std::endl;
}

///brake torque magnitude
Dbl CARBRAKE::GetTorque()
{
	Dbl brake = brake_factor > handbrake*handbrake_factor ? brake_factor : handbrake*handbrake_factor;
	Dbl pressure = brake * bias * max_pressure;
	Dbl normal = pressure * area;
	Dbl torque = friction * normal * radius;
	lasttorque = torque;
	return torque;
}

bool CARBRAKE::Serialize(joeserialize::Serializer & s)
{
	_SERIALIZE_(s,brake_factor);
	_SERIALIZE_(s,handbrake_factor);
	_SERIALIZE_(s,locked);
	return true;
}
