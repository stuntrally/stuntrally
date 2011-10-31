#ifndef _CARCLUTCH_H
#define _CARCLUTCH_H

//#include <iostream>
#include "joeserialize.h"
#include "macros.h"

template <typename T>
class CARCLUTCH
{
	friend class joeserialize::Serializer;
	private:
		//constants (not actually declared as const because they can be changed after object creation)
		T sliding_friction; ///< torque on the clutch is found by dividing the clutch pressure by the value in the area tag and multiplying by the radius and sliding (friction) parameters
		T radius; ///< torque on the clutch is found by dividing the clutch pressure by the value in the area tag and multiplying by the radius and sliding (friction) parameters
		T area; ///< torque on the clutch is found by dividing the clutch pressure by the value in the area tag and multiplying by the radius and sliding (friction) parameters
		T max_pressure; ///< maximum allowed pressure on the plates
		T threshold; ///< the clutch pretends to be fully engaged when engine speed - transmission speeds is less than m_threshold * normal force
		
		//variables
		T clutch_position;
		bool locked;
		
		//for info only
		T last_torque;
		T engine_speed;
		T drive_speed;
		
		
	public:
		//default constructor makes an S2000-like car
		CARCLUTCH() : sliding_friction(0.27), radius(0.15), area(0.75), max_pressure(11079.26), 
			  threshold(0.001), clutch_position(0.0), locked(false){}

		void DebugPrint(std::ostream & out)
		{
			out << "---Clutch---" << std::endl;
			out << "Position: " << clutch_position << std::endl;
			out << "Locked: " << locked << std::endl;
			out << "Torque: " << last_torque << std::endl;
			out << "Engine: " << engine_speed << std::endl;
			out << "Drive : " << drive_speed << std::endl;
		}

		void SetSlidingFriction ( const T& value )
		{
			sliding_friction = value;
		}
	
		void SetRadius ( const T& value )
		{
			radius = value;
		}
	
		void SetArea ( const T& value )
		{
			area = value;
		}
	
		void SetMaxPressure ( const T& value )
		{
			max_pressure = value;
		}
		
		///set the clutch engagement, where 1.0 is fully engaged
		void SetClutch ( const T& value )
		{
			clutch_position = value;
		}
		
		T GetClutch() const
		{
			return clutch_position;
		}
		
		// clutch is modeled as limited higly viscous coupling
		T GetTorque ( T n_engine_speed, T n_drive_speed )
		{
			engine_speed = n_engine_speed;
			drive_speed = n_drive_speed;
			T new_speed_diff = engine_speed - drive_speed;
            locked = true;
		
            T torque_capacity = sliding_friction * max_pressure * area * radius; // constant
			T max_torque = clutch_position * torque_capacity;
			T friction_torque = max_torque * new_speed_diff;    // viscous coupling (locked clutch)
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
			
			T torque = friction_torque;
			last_torque = torque;
			return torque;
		}
	
		bool IsLocked() const
		{
			return locked;
		}
		
		T GetLastTorque() const
		{
			return last_torque;
		}

		bool Serialize(joeserialize::Serializer & s)
		{
			_SERIALIZE_(s,clutch_position);
			_SERIALIZE_(s,locked);
			return true;
		}
};

#endif
