#ifndef _CARCLUTCH_H
#define _CARCLUTCH_H

#include "dbl.h"
#include "joeserialize.h"
#include "macros.h"
#include <iostream>
#include "../ogre/common/Defines.h"


class CARCLUTCH
{
	friend class joeserialize::Serializer;
	private:
		// constants
		Dbl sliding_friction;	///< torque on the clutch is found by dividing the clutch pressure by the value in the area tag and multiplying by the radius and sliding (friction) parameters
		Dbl radius;		///< torque on the clutch is found by dividing the clutch pressure by the value in the area tag and multiplying by the radius and sliding (friction) parameters
		Dbl area;		///< torque on the clutch is found by dividing the clutch pressure by the value in the area tag and multiplying by the radius and sliding (friction) parameters
		Dbl max_pressure;	///< maximum allowed pressure on the plates
		Dbl threshold;	///< the clutch pretends to be fully engaged when engine speed - transmission speeds is less than m_threshold * normal force
		
		// variables
		Dbl clutch_position;
		bool locked;
		
		//for info only
		Dbl last_torque;
		Dbl engine_speed;
		Dbl drive_speed;
		
	public:
		//default constructor makes an S2000-like car
		CARCLUTCH()
			: sliding_friction(0.27), radius(0.15), area(0.75), max_pressure(11079.26)
			, threshold(0.001), clutch_position(0.0), locked(false)
			, last_torque(0.f), engine_speed(0.f), drive_speed(0.f)
		{	}

		void DebugPrint(std::ostream & out);

		void SetSlidingFriction (const Dbl& value)	{	sliding_friction = value;	}
		void SetRadius (const Dbl& value)	{	radius = value;		}
		void SetArea (const Dbl& value)		{	area = value;	}
		void SetMaxPressure (const Dbl& value)	{	max_pressure = value;	}
		
		///set the clutch engagement, where 1.0 is fully engaged
		void SetClutch (const Dbl& value)	{	clutch_position = value;	}
		
		Dbl GetClutch() const		{	return clutch_position;		}
		
		// clutch is modeled as limited higly viscous coupling
		Dbl GetTorque ( Dbl n_engine_speed, Dbl n_drive_speed );

	
		bool IsLocked() const		{	return locked;	}
		
		Dbl GetLastTorque() const	{	return last_torque;		}

		bool Serialize(joeserialize::Serializer & s);
};

#endif
