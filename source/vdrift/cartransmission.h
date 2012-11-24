#ifndef _CARTRANSMISSION_H
#define _CARTRANSMISSION_H

#include "dbl.h"
#include "joeserialize.h"
#include "macros.h"
//#include <iostream>
#include "../ogre/common/Defines.h"


class CARTRANSMISSION
{
	friend class joeserialize::Serializer;
	private:
		//constants (not actually declared as const because they can be changed after object creation)
		std::map <int, Dbl> gear_ratios; ///< gear number and ratio.  reverse gears are negative integers. neutral is zero.
		int forward_gears; ///< the number of consecutive forward gears
		int reverse_gears; ///< the number of consecutive reverse gears
		
		//variables
		int gear; ///< the current gear
		
		//for info only
		Dbl driveshaft_rpm;
		Dbl crankshaft_rpm;
		
		
	public:
		//default constructor makes an S2000-like car
		CARTRANSMISSION() : gear(0), driveshaft_rpm(0), crankshaft_rpm(0), forward_gears(1), reverse_gears(0)
		{	gear_ratios [0] = 0.0;	}

		void DebugPrint(std::ostream & out)
		{
			out << "---Transmission---" << std::endl;
			out << "Gear ratio " << gear_ratios[gear] << std::endl;
			out << "CrankRPM " << fToStr(crankshaft_rpm, 0,5) << std::endl;
			out << "DriveRPM " << fToStr(driveshaft_rpm, 0,5) << std::endl;
		}
		
		int GetGear() const
		{
			return gear;
		}
		
		int GetForwardGears() const
		{
			return forward_gears;
		}
		
		int GetReverseGears() const
		{
			return reverse_gears;
		}

		void Shift(int newgear)
		{
			if (newgear <= forward_gears && newgear >= -reverse_gears)
				gear = newgear;
		}
		
		///ratio is: driveshaft speed / crankshaft speed
		void SetGearRatio(int gear, Dbl ratio)
		{
			gear_ratios[gear] = ratio;
			
			//find out how many consecutive forward gears we have
			forward_gears = 0;
			int key = 1;
			while (gear_ratios.find (key) != gear_ratios.end ())
			{
				forward_gears++;
				key++;
			}

			//find out how many consecutive forward gears we have
			reverse_gears = 0;
			key = -1;
			while (gear_ratios.find (key) != gear_ratios.end ())
			{
				reverse_gears++;
				key--;
			}
		}
		
		Dbl GetGearRatio(int gear) const
		{
			Dbl ratio = 1.0;
			std::map <int, Dbl>::const_iterator i = gear_ratios.find(gear);
			if (i != gear_ratios.end())
				ratio = i->second;
			return ratio;
		}
		
		Dbl GetCurrentGearRatio() const
		{
			return GetGearRatio(gear);
		}
		
		///get the torque on the driveshaft due to the given torque at the clutch
		Dbl GetTorque(Dbl clutch_torque)
		{
			return clutch_torque*gear_ratios[gear];
		}
		
		///get the rotational speed of the clutch given the rotational speed of the driveshaft
		Dbl CalculateClutchSpeed(Dbl driveshaft_speed)
		{
			driveshaft_rpm = driveshaft_speed * 30.0 / PI_d;
			crankshaft_rpm = driveshaft_speed * gear_ratios[gear] * 30.0 / PI_d;
			return driveshaft_speed * gear_ratios[gear];
		}
		
		///get the rotational speed of the clutch given the rotational speed of the driveshaft (const)
		Dbl GetClutchSpeed(Dbl driveshaft_speed) const
		{
			std::map <int, Dbl>::const_iterator i = gear_ratios.find(gear);
			assert(i != gear_ratios.end());
			return driveshaft_speed * i->second;
		}
		
		bool Serialize(joeserialize::Serializer & s)
		{
			_SERIALIZE_(s,gear);
			return true;
		}
};

#endif
