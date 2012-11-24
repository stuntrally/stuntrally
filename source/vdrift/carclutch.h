#ifndef _CARCLUTCH_H
#define _CARCLUTCH_H

#include "dbl.h"
#include "joeserialize.h"
#include "macros.h"
//#include <iostream>
#include "../ogre/common/Defines.h"


class CARCLUTCH
{
	friend class joeserialize::Serializer;
	private:
		//constants (not actually declared as const because they can be changed after object creation)
		Dbl sliding_friction; ///< torque on the clutch is found by dividing the clutch pressure by the value in the area tag and multiplying by the radius and sliding (friction) parameters
		Dbl radius; ///< torque on the clutch is found by dividing the clutch pressure by the value in the area tag and multiplying by the radius and sliding (friction) parameters
		Dbl area; ///< torque on the clutch is found by dividing the clutch pressure by the value in the area tag and multiplying by the radius and sliding (friction) parameters
		Dbl max_pressure; ///< maximum allowed pressure on the plates
		Dbl threshold; ///< the clutch pretends to be fully engaged when engine speed - transmission speeds is less than m_threshold * normal force
		
		//variables
		Dbl clutch_position;
		bool locked;
		
		//for info only
		Dbl last_torque;
		Dbl engine_speed;
		Dbl drive_speed;
		
		
	public:
		//default constructor makes an S2000-like car
		CARCLUTCH() : sliding_friction(0.27), radius(0.15), area(0.75), max_pressure(11079.26), 
			  threshold(0.001), clutch_position(0.0), locked(false), last_torque(0.f), engine_speed(0.f), drive_speed(0.f)
		{	}

		void DebugPrint(std::ostream & out)
		{
			out << "---Clutch---" << std::endl;
			out << "Position " << fToStr(clutch_position, 1,3) << std::endl;
			out << "Locked  " << locked << std::endl;
			out << "Torque" << fToStr(last_torque, 0,6) << std::endl;
			out << "Engine" << fToStr(engine_speed, 0,5) << std::endl;
			out << "Drive " << fToStr(drive_speed, 0,5) << std::endl;
		}

		void SetSlidingFriction ( const Dbl& value )
		{
			sliding_friction = value;
		}
	
		void SetRadius ( const Dbl& value )
		{
			radius = value;
		}
	
		void SetArea ( const Dbl& value )
		{
			area = value;
		}
	
		void SetMaxPressure ( const Dbl& value )
		{
			max_pressure = value;
		}
		
		///set the clutch engagement, where 1.0 is fully engaged
		void SetClutch ( const Dbl& value )
		{
			clutch_position = value;
		}
		
		Dbl GetClutch() const
		{
			return clutch_position;
		}
		
		// clutch is modeled as limited higly viscous coupling
		Dbl GetTorque ( Dbl n_engine_speed, Dbl n_drive_speed )
		{
			engine_speed = n_engine_speed;
			drive_speed = n_drive_speed;
			Dbl new_speed_diff = engine_speed - drive_speed;
            locked = true;
		
            Dbl torque_capacity = sliding_friction * max_pressure * area * radius; // constant
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
	
		bool IsLocked() const
		{
			return locked;
		}
		
		Dbl GetLastTorque() const
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
