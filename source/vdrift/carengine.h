#ifndef _CARENGINE_H
#define _CARENGINE_H

#include <iostream>
#include <map>
#include <fstream>

#include "dbl.h"
#include "rotationalframe.h"
#include "matrix3.h"
#include "spline.h"
#include "linearinterp.h"
#include "mathvector.h"
#include "joeserialize.h"
#include "macros.h"
#include "../ogre/common/Defines.h"


class CARENGINE
{
	friend class joeserialize::Serializer;
	private:
		//constants (not actually declared as const because they can be changed after object creation)
		Dbl redline; ///< the redline in RPMs
		Dbl rpm_limit; ///< peak engine RPMs after which limiting occurs
		Dbl idle; ///< idle throttle percentage; this is calculated algorithmically
		Dbl start_rpm; ///< initial condition RPM
		Dbl stall_rpm; ///< RPM at which the engine dies
		Dbl fuel_consumption; ///< fuel consumed each second (in liters) is the fuel-consumption parameter times RPM times throttle
		Dbl friction; ///< friction coefficient from the engine; this is calculated algorithmically
		std::map <Dbl, Dbl> torque_map; ///< a set of RPMs that map to torque values
		Dbl mass;
		MATHVECTOR <Dbl, 3> position;
		SPLINE <Dbl> torque_curve;
		
		//variables
		Dbl throttle_position;
		Dbl clutch_torque;
		bool out_of_gas;
		bool rev_limit_exceeded;
		ROTATIONALFRAME crankshaft;
		
		//for info only
		Dbl friction_torque;
		Dbl combustion_torque;
		bool stalled;
		
	public:
		Dbl GetTorqueCurve(const Dbl cur_throttle, const Dbl cur_rpm) const
		{
			if (cur_rpm < 1)
				return 0.0;
			
			Dbl torque = torque_curve.Interpolate(cur_rpm);
			
			//make sure the real function only returns values > 0
			return torque*cur_throttle;
		}
		
		Dbl GetFrictionTorque(Dbl cur_angvel, Dbl friction_factor, Dbl throttle_position)
		{
			Dbl velsign = cur_angvel < 0 ? -1.0 : 1.0;
			Dbl A = 0;
			Dbl B = -230*friction;  //par..
			Dbl C = 0;
			return (A + cur_angvel * B + -velsign * C * cur_angvel * cur_angvel) *
					(1.0 - friction_factor*throttle_position);
		}
		
		//default constructor makes an S2000-like car
		CARENGINE() : redline(7800), rpm_limit(9000), idle(0.02),
			start_rpm(1000), stall_rpm(350), fuel_consumption(1e-9), friction(0.000328),
			throttle_position(0.0), clutch_torque(0.0), out_of_gas(false),
			rev_limit_exceeded(false), friction_torque(0), combustion_torque(0), stalled(false)
	    {
		    MATRIX3 <Dbl> inertia;
		    inertia.Scale(0.25);
		    crankshaft.SetInertia(inertia);
	    }

		void SetInertia ( const Dbl& value )
		{
			MATRIX3 <Dbl> inertia;
			inertia.Scale(value);
			crankshaft.SetInertia(inertia);
		}
		
		Dbl GetInertia() const
		{
			return crankshaft.GetInertia()[0];
		}

		void SetRPMLimit ( const Dbl& value )
		{
			rpm_limit = value;
		}
		
		Dbl GetRPMLimit() const
		{
			return rpm_limit;
		}
	
		void SetRedline ( const Dbl& value )
		{
			redline = value;
		}
		
		Dbl GetRedline() const
		{
			return redline;
		}
		
		Dbl GetIdle() const
		{
			return idle;
		}
	
		void SetStartRPM ( const Dbl& value )
		{
			start_rpm = value;
		}
		
		Dbl GetStartRPM() const
		{
			return start_rpm;
		}
		
		void SetStallRPM ( const Dbl& value )
		{
			stall_rpm = value;
		}
		
		Dbl GetStallRPM() const
		{
			return stall_rpm;
		}
	
		void SetFuelConsumption ( const Dbl& value )
		{
			fuel_consumption = value;
		}
		
		Dbl GetFuelConsumption() const
		{
			return fuel_consumption;
		}
		
		void Integrate1(const Dbl dt)
		{
			crankshaft.Integrate1(dt);
		}
		
		void Integrate2(const Dbl dt)
		{
			//std::cout << "torque: " << crankshaft.GetTorque()[0] << std::endl;
			//std::cout << "ang vel prev: " << crankshaft.GetAngularVelocity()[0] << std::endl;
			crankshaft.Integrate2(dt);
			//std::cout << "ang vel next: " << crankshaft.GetAngularVelocity()[0] << std::endl;
		}
		
		const Dbl GetRPM() const
		{
			return crankshaft.GetAngularVelocity()[0] * 30.0 / PI_d;
		}
		
		///set the throttle position where 0.0 is no throttle and 1.0 is full throttle
		void SetThrottle ( const Dbl& value )
		{
			throttle_position = value;
		}
		
		Dbl GetThrottle() const
		{
			return throttle_position;
		}
		
		void SetInitialConditions()
		{
			MATHVECTOR <Dbl, 3> v;
			crankshaft.SetInitialTorque(v);
			StartEngine();
		}
		
		void StartEngine()
		{
			MATHVECTOR <Dbl, 3> v;
			v[0] = start_rpm * PI_d / 30.0;
			crankshaft.SetAngularVelocity(v);
		}
		
		///used to set the drag on the engine from the clutch being partially engaged
		void SetClutchTorque(Dbl newtorque)
		{
			clutch_torque = newtorque;
		}
		
		Dbl GetAngularVelocity() const
		{
			return crankshaft.GetAngularVelocity()[0];
		}
		
		///used to set the engine speed to the transmission speed when the clutch is fully engaged
		void SetAngularVelocity(Dbl angvel)
		{
			MATHVECTOR <Dbl, 3> v(angvel, 0, 0);
			crankshaft.SetAngularVelocity(v);
		}
		
		void DebugPrint(std::ostream & out)
		{
			out << "---Engine---" << std::endl;
			//out << "Throttle " << fToStr(throttle_position, 0,1) << std::endl;
			out << "Cmbst " << fToStr(combustion_torque, 0,5) << std::endl;
			//out << "Clutch  " << fToStr(-clutch_torque, 0,5) << std::endl;
			out << "Frict " << fToStr(friction_torque, 0,5) << std::endl;
			out << "Total " << fToStr(GetTorque(), 0,5) << std::endl;
			out << "RPM   " << fToStr(GetRPM(), 0,4) << std::endl;
			//out << "Exceeded: " << rev_limit_exceeded << std::endl;
			//out << "Running: " << !stalled << std::endl;
		}
		
		///return the sum of all torques acting on the engine (except clutch forces)
		Dbl GetTorque()
		{
			return combustion_torque + friction_torque;
		}
		
		void ComputeForces()
		{
			if (GetRPM() < stall_rpm)
			{
				stalled = true;
			}
			else
				stalled = false;
			
			//make sure the throttle is at least idling
			if (throttle_position < idle)
				throttle_position = idle;
			
			//engine friction
			Dbl cur_angvel = crankshaft.GetAngularVelocity()[0];
			
			//engine drive torque
			Dbl friction_factor = 1.0; //used to make sure we allow friction to work if we're out of gas or above the rev limit
			Dbl rev_limit = rpm_limit;
			if (rev_limit_exceeded)
				rev_limit -= 100.0; //tweakable
			
			if (GetRPM() < rev_limit)
				rev_limit_exceeded = false;
			else
				rev_limit_exceeded = true;
			
			combustion_torque = GetTorqueCurve(throttle_position, GetRPM());
			
			if (out_of_gas || rev_limit_exceeded || stalled)
			{
				friction_factor = 0.0;
				combustion_torque = 0.0;
			}
			
			friction_torque = GetFrictionTorque(cur_angvel, friction_factor, throttle_position);
			if (stalled)
			{
				//try to model the static friction of the engine
				friction_torque *= 100.0;
			}
		}
		
		void ApplyForces()
		{
			MATHVECTOR <Dbl, 3> total_torque(0);
			
			total_torque[0] += combustion_torque;
			total_torque[0] += friction_torque;
			total_torque[0] -= clutch_torque;
			
			crankshaft.SetTorque(total_torque);
		}
		
		///Set the torque curve using a vector of (RPM, torque) pairs.
		/// also recalculate engine friction
		/// the max_power_rpm value should be set to the engine's redline
		void SetTorqueCurve(Dbl max_power_rpm, std::vector <std::pair <Dbl, Dbl> > & curve)
		{
			torque_curve.Clear();
			
			//this value accounts for the fact that the torque curves are usually measured
			// on a dyno, but we're interested in the actual crankshaft power
			const Dbl dyno_correction_factor = 1.0;//1.14;
			
			assert(curve.size() > 1);
			
			//ensure we have a smooth curve down to 0 RPM
			if (curve[0].first != 0)
				torque_curve.AddPoint(0,0);
			
			for (std::vector <std::pair <Dbl, Dbl> >::iterator i = curve.begin();
				i != curve.end(); ++i)
			{
				torque_curve.AddPoint(i->first, i->second*dyno_correction_factor);
			}
			
			//ensure we have a smooth curve for over-revs
			torque_curve.AddPoint(curve[curve.size()-1].first + 10000, 0);
			
			//write out a debug torque curve file
			/*std::ofstream f("out.dat");
			for (Dbl i = 0; i < curve[curve.size()-1].first+1000; i+= 20) f << i << " " << torque_curve.Interpolate(i) << std::endl;*/
			//for (unsigned int i = 0; i < curve.size(); i++) f << curve[i].first << " " << curve[i].second << std::endl;
			
			//calculate engine friction
			Dbl max_power_angvel = max_power_rpm * PI_d/30.0;
			Dbl max_power = torque_curve.Interpolate(max_power_rpm)*max_power_angvel;
			friction = max_power / (max_power_angvel*max_power_angvel*max_power_angvel);
			
			//calculate idle throttle position
			for (idle = 0; idle < 1.0; idle += 0.01)
			{
				if (GetTorqueCurve(idle, start_rpm) > -GetFrictionTorque(start_rpm*PI_d/30.0, 1.0, idle))
				{
					//std::cout << "Found idle throttle: " << idle << ", " << GetTorqueCurve(idle, start_rpm) << ", " << friction_torque << std::endl;
					break;
				}
			}
		}

	void SetMass ( const Dbl& value )
	{
		mass = value;
	}
	

	Dbl GetMass() const
	{
		return mass;
	}

	void SetPosition ( const MATHVECTOR< Dbl, 3 >& value )
	{
		position = value;
	}
	

	MATHVECTOR< Dbl, 3 > GetPosition() const
	{
		return position;
	}
	
	Dbl FuelRate() const
	{
		return fuel_consumption * GetAngularVelocity() * throttle_position;
	}

	void SetOutOfGas ( bool value )
	{
		out_of_gas = value;
	}
	
	bool Serialize(joeserialize::Serializer & s)
	{
		_SERIALIZE_(s,throttle_position);
		_SERIALIZE_(s,clutch_torque);
		_SERIALIZE_(s,out_of_gas);
		_SERIALIZE_(s,rev_limit_exceeded);
		_SERIALIZE_(s,crankshaft);
		return true;
	}
	
	///returns true if the engine is combusting fuel
	bool GetCombustion() const
	{
		return !stalled;
	}
};

#endif
