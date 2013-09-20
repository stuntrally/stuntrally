#pragma once
#include "dbl.h"
#include <iostream>
#include <map>


class CARTRANSMISSION
{
private:
	// constants
	std::map <int, Dbl> gear_ratios;	///< gear number and ratio.  reverse gears are negative integers. neutral is zero.
	int forward_gears;	///< the number of consecutive forward gears
	int reverse_gears;	///< the number of consecutive reverse gears
	
	// variables
	int gear;	///< the current gear
	
	//for info only
	Dbl driveshaft_rpm;
	Dbl crankshaft_rpm;
	
	
public:
	//default constructor makes an S2000-like car
	CARTRANSMISSION()
		: gear(0), driveshaft_rpm(0), crankshaft_rpm(0)
		, forward_gears(1), reverse_gears(0)
	{	gear_ratios [0] = 0.0;	}

	void DebugPrint(std::ostream & out);
	
	int GetGear() const			{	return gear;	}
	int GetForwardGears() const	{	return forward_gears;	}
	int GetReverseGears() const	{	return reverse_gears;	}

	void Shift(int newgear)
	{
		if (newgear <= forward_gears && newgear >= -reverse_gears)
			gear = newgear;
	}
	
	///ratio is: driveshaft speed / crankshaft speed
	void SetGearRatio(int gear, Dbl ratio);
	
	Dbl GetGearRatio(int gear) const;
	
	Dbl GetCurrentGearRatio() const	{	return GetGearRatio(gear);	}
	
	///get the torque on the driveshaft due to the given torque at the clutch
	Dbl GetTorque(Dbl clutch_torque)
	{
		return clutch_torque * gear_ratios[gear];
	}
	
	///get the rotational speed of the clutch given the rotational speed of the driveshaft
	Dbl CalculateClutchSpeed(Dbl driveshaft_speed);
	
	///get the rotational speed of the clutch given the rotational speed of the driveshaft (const)
	Dbl GetClutchSpeed(Dbl driveshaft_speed) const;
};
