#pragma once
#include "dbl.h"
#include <iostream>


class CARCLUTCH
{
private:
	// constants
	Dbl clutch_max_torque;
	/// The torque capacity(maximum transmitted torque) of the clutch is
	/// TC = sliding * radius * area * max-pressure.
	/// It should be somewhere between one and two times the maximum enine torque. (1.25 is a good start value).
	Dbl threshold;	///< the clutch pretends to be fully engaged when engine speed -
					///  transmission speeds is less than m_threshold * normal force
	
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
		//: sliding_friction(0.27), radius(0.15), area(0.75), max_pressure(11079.26)
		: clutch_max_torque(336.53)
		, threshold(0.001), clutch_position(0.0), locked(false)
		, last_torque(0.f), engine_speed(0.f), drive_speed(0.f)
	{	}

	void DebugPrint(std::ostream & out);

	void SetMaxTorque (const Dbl& value)	{	clutch_max_torque = value;	}
	
	///set the clutch engagement, where 1.0 is fully engaged
	void SetClutch (const Dbl& value)	{	clutch_position = value;	}
	Dbl GetClutch() const				{	return clutch_position;		}
	
	// clutch is modeled as limited higly viscous coupling
	Dbl GetTorque ( Dbl n_engine_speed, Dbl n_drive_speed );


	bool IsLocked() const		{	return locked;	}
	
	Dbl GetLastTorque() const	{	return last_torque;		}
};
