#ifndef _CARTRANSMISSION_H
#define _CARTRANSMISSION_H

#include "joeserialize.h"
#include "macros.h"

//#include <iostream>

template <typename T>
class CARTRANSMISSION
{
	friend class joeserialize::Serializer;
	private:
		//constants (not actually declared as const because they can be changed after object creation)
		std::map <int, T> gear_ratios; ///< gear number and ratio.  reverse gears are negative integers. neutral is zero.
		int forward_gears; ///< the number of consecutive forward gears
		int reverse_gears; ///< the number of consecutive reverse gears
		
		//variables
		int gear; ///< the current gear
		
		//for info only
		T driveshaft_rpm;
		T crankshaft_rpm;
		
		
	public:
		//default constructor makes an S2000-like car
		CARTRANSMISSION() : gear(0), driveshaft_rpm(0), crankshaft_rpm(0) {gear_ratios [0] = 0.0;}

		void DebugPrint(std::ostream & out)
		{
			out << "---Transmission---" << std::endl;
			out << "Gear ratio: " << gear_ratios[gear] << std::endl;
			out << "Crankshaft RPM: " << crankshaft_rpm << std::endl;
			out << "Driveshaft RPM: " << driveshaft_rpm << std::endl;
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
		void SetGearRatio(int gear, T ratio)
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
		
		T GetGearRatio(int gear) const
		{
			T ratio = 1.0;
			typename std::map <int, T>::const_iterator i = gear_ratios.find(gear);
			if (i != gear_ratios.end())
				ratio = i->second;
			return ratio;
		}
		
		T GetCurrentGearRatio() const
		{
			return GetGearRatio(gear);
		}
		
		///get the torque on the driveshaft due to the given torque at the clutch
		T GetTorque(T clutch_torque)
		{
			return clutch_torque*gear_ratios[gear];
		}
		
		///get the rotational speed of the clutch given the rotational speed of the driveshaft
		T CalculateClutchSpeed(T driveshaft_speed)
		{
			driveshaft_rpm = driveshaft_speed * 30.0 / 3.141593;
			crankshaft_rpm = driveshaft_speed * gear_ratios[gear] * 30.0 / 3.141593;
			return driveshaft_speed * gear_ratios[gear];
		}
		
		///get the rotational speed of the clutch given the rotational speed of the driveshaft (const)
		T GetClutchSpeed(T driveshaft_speed) const
		{
			typename std::map <int, T>::const_iterator i = gear_ratios.find(gear);
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
