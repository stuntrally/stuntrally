#pragma once
#include "dbl.h"
#include <iostream>


class CARBRAKE
{
private:
	// constants
	Dbl friction;	///< sliding coefficient of friction for the brake pads on the rotor
	Dbl max_pressure;	///< maximum allowed pressure
	Dbl radius;		///< effective radius of the rotor
	Dbl area;		///< area of the brake pads
	Dbl bias;		///< the fraction of the pressure to be applied to the brake
	Dbl threshold;	///< brake locks when the linear brake velocity divided by the normal force is less than this value
	Dbl handbrake;	///< the friction factor that is applied when the handbrake is pulled.  this is usually 1.0 for rear brakes and 0.0 for front brakes, but could be any number
	
	// variables
	Dbl brake_factor;
	Dbl handbrake_factor;	///< this is separate so that ABS does not get applied to the handbrake
	bool locked;
	
	//for info only
	Dbl lasttorque;
	
public:
	//default constructor makes an S2000-like car
	CARBRAKE()
		:friction(0.73),max_pressure(4e6),radius(0.14),area(0.015)
		,threshold(2e-4),brake_factor(0),locked(false),lasttorque(0)
		,bias(0.5),handbrake(0),handbrake_factor(0)
	{	}

	void DebugPrint(std::ostream & out);
	
	///brake_factor ranges from 0.0 (no brakes applied) to 1.0 (brakes applied)
	void SetBrakeFactor(Dbl newfactor)	{	brake_factor = newfactor;	}

	///ranges from 0.0 (no brakes applied) to 1.0 (brakes applied)
	void SetHandbrakeFactor (const Dbl& value)	{	handbrake_factor = value;	}
	
	///brake torque magnitude
	Dbl GetTorque();
	
	///used by the autoclutch system to determine if the brakes are about to lock up
	bool WillLock() const	{    return locked;		}
    void WillLock(Dbl lock)	{    locked = lock;		}

	void SetFriction (const Dbl& value)	{	friction = value;	}
	Dbl GetFriction() const				{	return friction;	}

	void SetMaxPressure (const Dbl& value)	{	max_pressure = value;	}
	void SetRadius (const Dbl& value)		{	radius = value;		}
	void SetArea (const Dbl& value)		{	area = value;	}
	void SetBias (const Dbl& value)		{	bias = value;	}

	bool GetLocked() const				{	return locked;	}
	Dbl GetBrakeFactor() const			{	return brake_factor;	}
	Dbl GetHandbrakeFactor() const		{	return handbrake_factor;	}
	void SetHandbrake (const Dbl& value)	{	handbrake = value;	}
};
