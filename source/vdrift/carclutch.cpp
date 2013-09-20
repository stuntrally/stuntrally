#include "pch.h"
#include "carclutch.h"
#include "../ogre/common/Def_Str.h"


void CARCLUTCH::DebugPrint(std::ostream & out)
{
	out << "---Clutch---" << std::endl;
	out << "Position " << fToStr(clutch_position, 1,3) << std::endl;
	out << "Locked  " << locked << std::endl;
	out << "Torque" << fToStr(last_torque, 0,6) << std::endl;
	out << "Engine" << fToStr(engine_speed, 0,5) << std::endl;
	out << "Drive " << fToStr(drive_speed, 0,5) << std::endl;
}

// clutch is modeled as limited higly viscous coupling
Dbl CARCLUTCH::GetTorque (Dbl n_engine_speed, Dbl n_drive_speed)
{
	engine_speed = n_engine_speed;
	drive_speed = n_drive_speed;
	Dbl new_speed_diff = engine_speed - drive_speed;
    locked = true;

    //Dbl torque_capacity = sliding_friction * max_pressure * area * radius;  // constant
    Dbl torque_capacity = clutch_max_torque;  // constant
	Dbl max_torque = clutch_position * torque_capacity;
	Dbl friction_torque = max_torque * new_speed_diff;    // viscous coupling (locked clutch)
	if (friction_torque > max_torque)
	{
	    friction_torque  = max_torque;
	    locked = false;                                 // slipping clutch
	}
	else if (friction_torque < -max_torque)
	{
	    friction_torque  = -max_torque;
	    locked = false;
	}
	
	Dbl torque = friction_torque;
	last_torque = torque;
	return torque;
}
