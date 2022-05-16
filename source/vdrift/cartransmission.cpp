#include "pch.h"
#include "cartransmission.h"
#include "../ogre/common/Def_Str.h"


void CARTRANSMISSION::DebugPrint(std::ostream & out)
{
	out << "---Transmission---" << std::endl;
	out << "Gear ratio " << gear_ratios[gear] << std::endl;
	out << "CrankRPM " << fToStr(crankshaft_rpm, 0,5) << std::endl;
	out << "DriveRPM " << fToStr(driveshaft_rpm, 0,5) << std::endl;
}

///ratio is: driveshaft speed / crankshaft speed
void CARTRANSMISSION::SetGearRatio(int gear, Dbl ratio)
{
	gear_ratios[gear] = ratio;
	
	//find out how many consecutive forward gears we have
	forward_gears = 0;
	int key = 1;
	while (gear_ratios.find (key) != gear_ratios.end ())
	{
		++forward_gears;
		++key;
	}

	//find out how many consecutive forward gears we have
	reverse_gears = 0;
	key = -1;
	while (gear_ratios.find (key) != gear_ratios.end ())
	{
		++reverse_gears;
		--key;
	}
}

Dbl CARTRANSMISSION::GetGearRatio(int gear) const
{
	Dbl ratio = 1.0;
	auto i = gear_ratios.find(gear);
	if (i != gear_ratios.end())
		ratio = i->second;
	return ratio;
}

///get the rotational speed of the clutch given the rotational speed of the driveshaft
Dbl CARTRANSMISSION::CalculateClutchSpeed(Dbl driveshaft_speed)
{
	driveshaft_rpm = driveshaft_speed * 30.0 / PI_d;
	crankshaft_rpm = driveshaft_speed * gear_ratios[gear] * 30.0 / PI_d;
	return driveshaft_speed * gear_ratios[gear];
}

///get the rotational speed of the clutch given the rotational speed of the driveshaft (const)
Dbl CARTRANSMISSION::GetClutchSpeed(Dbl driveshaft_speed) const
{
	auto i = gear_ratios.find(gear);
	assert(i != gear_ratios.end());
	return driveshaft_speed * i->second;
}
