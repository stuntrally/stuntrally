#ifndef _CARBRAKE_H
#define _CARBRAKE_H

#include "dbl.h"
#include "joeserialize.h"
#include "macros.h"
#include <iostream>
#include "../ogre/common/Defines.h"


class CARBRAKE
{
	friend class joeserialize::Serializer;
	private:
		//constants (not actually declared as const because they can be changed after object creation)
		Dbl friction; ///< sliding coefficient of friction for the brake pads on the rotor
		Dbl max_pressure; ///< maximum allowed pressure
		Dbl radius; ///< effective radius of the rotor
		Dbl area; ///< area of the brake pads
		Dbl bias; ///< the fraction of the pressure to be applied to the brake
		Dbl threshold; ///< brake locks when the linear brake velocity divided by the normal force is less than this value
		Dbl handbrake; ///< the friction factor that is applied when the handbrake is pulled.  this is usually 1.0 for rear brakes and 0.0 for front brakes, but could be any number
		
		//variables
		Dbl brake_factor;
		Dbl handbrake_factor; ///< this is separate so that ABS does not get applied to the handbrake
		bool locked;
		
		//for info only
		Dbl lasttorque;
		
		
	public:
		//default constructor makes an S2000-like car
		CARBRAKE() : friction(0.73),max_pressure(4e6),radius(0.14),area(0.015),threshold(2e-4),brake_factor(0),locked(false) {}

		void DebugPrint(std::ostream & out)
		{
			//out << "---Brake---" << std::endl;
			//out << "Brake    : " << brake_factor << std::endl;
			//out << "Handbrake: " << handbrake_factor << std::endl;
			out << "Torque " << fToStr(lasttorque, 0,5) << std::endl;
			out << "Locked  " << locked << std::endl;
		}
		
		///brake_factor ranges from 0.0 (no brakes applied) to 1.0 (brakes applied)
		void SetBrakeFactor(Dbl newfactor)
		{
			brake_factor = newfactor;
		}
		
		///brake torque magnitude
		Dbl GetTorque()
		{
			Dbl brake = brake_factor > handbrake*handbrake_factor ? brake_factor : handbrake*handbrake_factor;
			Dbl pressure = brake * bias * max_pressure;
			Dbl normal = pressure * area;
			Dbl torque = friction * normal * radius;
			lasttorque = torque;
			return torque;
		}
		
		///used by the autoclutch system to determine if the brakes are about to lock up
		bool WillLock() const
		{
		    return locked;
		}
		
        void WillLock(Dbl lock)
		{
		    locked = lock;
        }

		void SetFriction ( const Dbl& value )
		{
			friction = value;
		}
		
	
		Dbl GetFriction() const
		{
			return friction;
		}
	
		void SetMaxPressure ( const Dbl& value )
		{
			max_pressure = value;
		}
	
		void SetRadius ( const Dbl& value )
		{
			radius = value;
		}
	
		void SetArea ( const Dbl& value )
		{
			area = value;
		}
	
		void SetBias ( const Dbl& value )
		{
			bias = value;
		}
	
		bool GetLocked() const
		{
			return locked;
		}

		Dbl GetBrakeFactor() const
		{
			return brake_factor;
		}
		
		Dbl GetHandbrakeFactor() const
		{
			return handbrake_factor;
		}
		
		bool Serialize(joeserialize::Serializer & s)
		{
			_SERIALIZE_(s,brake_factor);
			_SERIALIZE_(s,handbrake_factor);
			_SERIALIZE_(s,locked);
			return true;
		}

		void SetHandbrake ( const Dbl& value )
		{
			handbrake = value;
		}
		
		///ranges from 0.0 (no brakes applied) to 1.0 (brakes applied)
		void SetHandbrakeFactor ( const Dbl& value )
		{
			handbrake_factor = value;
		}
};

#endif
