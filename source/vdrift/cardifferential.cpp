#include "pch.h"
#include "dbl.h"
#include "cardifferential.h"
#include "../ogre/common/Def_Str.h"


//default constructor makes an S2000-like car
CARDIFFERENTIAL::CARDIFFERENTIAL()
	: final_drive(4.1), anti_slip(600.0), anti_slip_torque(0)
	, anti_slip_torque_decel_factor(0), torque_split(0.5)
	, side1_speed(0), side2_speed(0), side1_torque(0), side2_torque(0)
{	}

void CARDIFFERENTIAL::DebugPrint(std::ostream & out)
{
	//out << "---Differential---" << std::endl;
	out << "1 RPM" << fToStr(side1_speed * 30.0 / PI_d, 0,5) << std::endl;
	out << "2 RPM" << fToStr(side2_speed * 30.0 / PI_d, 0,5) << std::endl;
	out << "1 Trq" << fToStr(side1_torque, 0,6) << std::endl;
	out << "2 Trq" << fToStr(side2_torque, 0,6) << std::endl;
}

void CARDIFFERENTIAL::ComputeWheelTorques(Dbl driveshaft_torque)
{
	//determine torque from the anti-slip mechanism
	Dbl cas = anti_slip;
	if (anti_slip_torque > 0)  // if torque sensitive
		cas = anti_slip_torque * driveshaft_torque;  //TODO: add some minimum anti-slip

	if (cas < 0)  // determine behavior for deceleration
		cas *= -anti_slip_torque_decel_factor;

	cas = std::max(0.0,cas);
	Dbl drag = clamp(cas * (side1_speed - side2_speed), -anti_slip, anti_slip);
	
	Dbl torque = driveshaft_torque * final_drive;
	side1_torque = torque*(1.0-torque_split) - drag;
	side2_torque = torque*torque_split + drag;
}
