#ifndef _CARDIFFERENTIAL_H
#define _CARDIFFERENTIAL_H

#include "joeserialize.h"
#include "macros.h"

//#include <iostream>

///a differential that supports speed-sensitive limited slip functionality.  epicyclic (torque splitting) operation is also provided.
template <typename T>
class CARDIFFERENTIAL
{
	friend class joeserialize::Serializer;
	private:
		//constants (not actually declared as const because they can be changed after object creation)
		T final_drive; ///< the gear ratio of the differential
		T anti_slip; ///< this allows modelling of speed-sensitive limited-slip differentials.  this is the maximum anti_slip torque that will be applied and, for speed-sensitive limited-slip differentials, the anti-slip multiplier that's always applied.
		T anti_slip_torque; ///< this allows modelling of torque sensitive limited-slip differentials.  this is the anti_slip dependence on torque.
		T anti_slip_torque_deceleration_factor; ///< this allows modelling of torque sensitive limited-slip differentials that are 1.5 or 2-way.  set it to 0.0 for 1-way LSD, 1.0 for 2-way LSD, and somewhere in between for 1.5-way LSD.
		T torque_split; ///< this allows modelling of epicyclic differentials.  this value ranges from 0.0 to 1.0 where 0.0 applies all torque to side1
		
		//variables
		///by convention, side1 is left or front, side2 is right or rear.
		T side1_speed;
		T side2_speed;
		T side1_torque;
		T side2_torque;
		
		//for info only
		
		
		
	public:
		//default constructor makes an S2000-like car
		CARDIFFERENTIAL() : final_drive(4.1), anti_slip(600.0), anti_slip_torque(0), anti_slip_torque_deceleration_factor(0), torque_split(0.5), side1_speed(0), side2_speed(0), side1_torque(0), side2_torque(0) {}

		void DebugPrint(std::ostream & out)
		{
			out << "---Differential---" << std::endl;
			out << "Side 1 RPM: " << side1_speed * 30.0 / 3.141593 << std::endl;
			out << "Side 2 RPM: " << side2_speed * 30.0 / 3.141593 << std::endl;
			out << "Side 1 Torque: " << side1_torque << std::endl;
			out << "Side 2 Torque: " << side2_torque << std::endl;
		}

		void SetFinalDrive ( const T& value )
		{
			final_drive = value;
		}
		
		void SetAntiSlip(T as, T ast, T astdf)
		{
			anti_slip = as;
			anti_slip_torque = ast;
			anti_slip_torque_deceleration_factor = astdf;
		}
		
		T CalculateDriveshaftSpeed(T new_side1_speed, T new_side2_speed)
		{
			side1_speed = new_side1_speed;
			side2_speed = new_side2_speed;
			return final_drive * (side1_speed + side2_speed) * 0.5;
		}
		
		T GetDriveshaftSpeed(T new_side1_speed, T new_side2_speed) const
		{
			return final_drive * (side1_speed + side2_speed) * 0.5;
		}
		
		T clamp(T val, T min, T max) const
		{
			return std::max(std::min(val,max), min);
		}
		
		void ComputeWheelTorques(T driveshaft_torque)
		{
			//determine torque from the anti-slip mechanism
			T current_anti_slip = anti_slip;
			if (anti_slip_torque > 0) //if torque sensitive
				current_anti_slip = anti_slip_torque*driveshaft_torque; //TODO: add some minimum anti-slip
			if (current_anti_slip < 0) //determine behavior for deceleration
			{
				current_anti_slip *= -anti_slip_torque_deceleration_factor;
			}
			current_anti_slip = std::max(T(0),current_anti_slip);
			T drag = clamp(current_anti_slip * (side1_speed - side2_speed),-anti_slip,anti_slip);
			
			T torque = driveshaft_torque * final_drive;
			side1_torque = torque*(1.0-torque_split) - drag;
			side2_torque = torque*torque_split + drag;
		}

		const T & GetSide1Torque() const
		{
			return side1_torque;
		}
	
		const T & GetSide2Torque() const
		{
			return side2_torque;
		}
		
		const T & GetSide1Speed() const
		{
			return side1_speed;
		}
	
		const T & GetSide2Speed() const
		{
			return side2_speed;
		}

		T GetFinalDrive() const
		{
			return final_drive;
		}
		
		bool Serialize(joeserialize::Serializer & s)
		{
			_SERIALIZE_(s,side1_speed);
			_SERIALIZE_(s,side2_speed);
			_SERIALIZE_(s,side1_torque);
			_SERIALIZE_(s,side2_torque);
			return true;
		}
};

#endif
