#include "pch.h"
#include "carbrake.h"
#include "../ogre/common/Def_Str.h"


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
