#ifndef _CARENGINE_H
#define _CARENGINE_H

#include <iostream>
#include <map>
#include <fstream>

#include "rotationalframe.h"
#include "matrix3.h"
#include "spline.h"
#include "linearinterp.h"
#include "mathvector.h"
#include "joeserialize.h"
#include "macros.h"

template <typename T>
class CARENGINE
{
	friend class joeserialize::Serializer;
	private:
		//constants (not actually declared as const because they can be changed after object creation)
		T redline; ///< the redline in RPMs
		T rpm_limit; ///< peak engine RPMs after which limiting occurs
		T idle; ///< idle throttle percentage; this is calculated algorithmically
		T start_rpm; ///< initial condition RPM
		T stall_rpm; ///< RPM at which the engine dies
		T fuel_consumption; ///< fuel consumed each second (in liters) is the fuel-consumption parameter times RPM times throttle
		T friction; ///< friction coefficient from the engine; this is calculated algorithmically
		std::map <T, T> torque_map; ///< a set of RPMs that map to torque values
		T mass;
		MATHVECTOR <T, 3> position;
		SPLINE <T> torque_curve;
		
		//variables
		T throttle_position;
		T clutch_torque;
		bool out_of_gas;
		bool rev_limit_exceeded;
		ROTATIONALFRAME <T> crankshaft;
		
		//for info only
		T friction_torque;
		T combustion_torque;
		bool stalled;
		
		T GetTorqueCurve(const T cur_throttle, const T cur_rpm) const
		{
			if (cur_rpm < 1)
				return 0.0;
			
			T torque = torque_curve.Interpolate(cur_rpm);
			
			//make sure the real function only returns values > 0
			return torque*cur_throttle;
		}
		
		T GetFrictionTorque(T cur_angvel, T friction_factor, T throttle_position)
		{
			T velsign = 1.0;
			if (cur_angvel < 0)
				velsign = -1.0;
			T A = 0;
			T B = -1300*friction;
			T C = 0;
			return (A + cur_angvel * B + -velsign * C * cur_angvel * cur_angvel) *
					(1.0 - friction_factor*throttle_position);
		}
		
	public:
		//default constructor makes an S2000-like car
		CARENGINE() : redline(7800), rpm_limit(9000), idle(0.02),
			start_rpm(1000), stall_rpm(350), fuel_consumption(1e-9), friction(0.000328),
			throttle_position(0.0), clutch_torque(0.0), out_of_gas(false),
			rev_limit_exceeded(false), friction_torque(0), combustion_torque(0), stalled(false)
				    {
					    MATRIX3 <T> inertia;
					    inertia.Scale(0.25);
					    crankshaft.SetInertia(inertia);
				    }

		void SetInertia ( const T& value )
		{
			MATRIX3 <T> inertia;
			inertia.Scale(value);
			crankshaft.SetInertia(inertia);
		}
		
		T GetInertia() const
		{
			return crankshaft.GetInertia()[0];
		}

		void SetRPMLimit ( const T& value )
		{
			rpm_limit = value;
		}
		
		T GetRPMLimit() const
		{
			return rpm_limit;
		}
	
		void SetRedline ( const T& value )
		{
			redline = value;
		}
		
		T GetRedline() const
		{
			return redline;
		}
		
		T GetIdle() const
		{
			return idle;
		}
	
		void SetStartRPM ( const T& value )
		{
			start_rpm = value;
		}
		
		T GetStartRPM() const
		{
			return start_rpm;
		}
		
		void SetStallRPM ( const T& value )
		{
			stall_rpm = value;
		}
		
		T GetStallRPM() const
		{
			return stall_rpm;
		}
	
		void SetFuelConsumption ( const T& value )
		{
			fuel_consumption = value;
		}
		
		T GetFuelConsumption() const
		{
			return fuel_consumption;
		}
		
		void Integrate1(const T dt)
		{
			crankshaft.Integrate1(dt);
		}
		
		void Integrate2(const T dt)
		{
			//std::cout << "torque: " << crankshaft.GetTorque()[0] << std::endl;
			//std::cout << "ang vel prev: " << crankshaft.GetAngularVelocity()[0] << std::endl;
			crankshaft.Integrate2(dt);
			//std::cout << "ang vel next: " << crankshaft.GetAngularVelocity()[0] << std::endl;
		}
		
		const T GetRPM() const
		{
			return crankshaft.GetAngularVelocity()[0] * 30.0 / 3.141593;
		}
		
		///set the throttle position where 0.0 is no throttle and 1.0 is full throttle
		void SetThrottle ( const T& value )
		{
			throttle_position = value;
		}
		
		T GetThrottle() const
		{
			return throttle_position;
		}
		
		void SetInitialConditions()
		{
			MATHVECTOR <T, 3> v;
			crankshaft.SetInitialTorque(v);
			StartEngine();
		}
		
		void StartEngine()
		{
			MATHVECTOR <T, 3> v;
			v[0] = start_rpm * 3.141593 / 30.0;
			crankshaft.SetAngularVelocity(v);
		}
		
		///used to set the drag on the engine from the clutch being partially engaged
		void SetClutchTorque(T newtorque)
		{
			clutch_torque = newtorque;
		}
		
		T GetAngularVelocity() const
		{
			return crankshaft.GetAngularVelocity()[0];
		}
		
		///used to set the engine speed to the transmission speed when the clutch is fully engaged
		void SetAngularVelocity(T angvel)
		{
			MATHVECTOR <T, 3> v(angvel, 0, 0);
			crankshaft.SetAngularVelocity(v);
		}
		
		void DebugPrint(std::ostream & out)
		{
			out << "---Engine---" << std::endl;
			out << "Throttle: " << throttle_position << std::endl;
			out << "Combustion: " << combustion_torque << std::endl;
			out << "Clutch  : " << -clutch_torque << std::endl;
			out << "Friction: " << friction_torque << std::endl;
			out << "Total   : " << GetTorque() << std::endl;
			out << "RPM: " << GetRPM() << std::endl;
			out << "Exceeded: " << rev_limit_exceeded << std::endl;
			out << "Running: " << !stalled << std::endl;
		}
		
		///return the sum of all torques acting on the engine (except clutch forces)
		T GetTorque()
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
			T cur_angvel = crankshaft.GetAngularVelocity()[0];
			
			//engine drive torque
			T friction_factor = 1.0; //used to make sure we allow friction to work if we're out of gas or above the rev limit
			T rev_limit = rpm_limit;
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
			MATHVECTOR <T, 3> total_torque(0);
			
			total_torque[0] += combustion_torque;
			total_torque[0] += friction_torque;
			total_torque[0] -= clutch_torque;
			
			crankshaft.SetTorque(total_torque);
		}
		
		///Set the torque curve using a vector of (RPM, torque) pairs.
		/// also recalculate engine friction
		/// the max_power_rpm value should be set to the engine's redline
		void SetTorqueCurve(T max_power_rpm, std::vector <std::pair <T, T> > & curve)
		{
			torque_curve.Clear();
			
			//this value accounts for the fact that the torque curves are usually measured
			// on a dyno, but we're interested in the actual crankshaft power
			const T dyno_correction_factor = 1.0;//1.14;
			
			assert(curve.size() > 1);
			
			//ensure we have a smooth curve down to 0 RPM
			if (curve[0].first != 0)
				torque_curve.AddPoint(0,0);
			
			for (typename std::vector <std::pair <T, T> >::iterator i = curve.begin();
				i != curve.end(); ++i)
			{
				torque_curve.AddPoint(i->first, i->second*dyno_correction_factor);
			}
			
			//ensure we have a smooth curve for over-revs
			torque_curve.AddPoint(curve[curve.size()-1].first + 10000, 0);
			
			//write out a debug torque curve file
			/*std::ofstream f("out.dat");
			for (T i = 0; i < curve[curve.size()-1].first+1000; i+= 20) f << i << " " << torque_curve.Interpolate(i) << std::endl;*/
			//for (unsigned int i = 0; i < curve.size(); i++) f << curve[i].first << " " << curve[i].second << std::endl;
			
			//calculate engine friction
			T max_power_angvel = max_power_rpm*3.14153/30.0;
			T max_power = torque_curve.Interpolate(max_power_rpm)*max_power_angvel;
			friction = max_power / (max_power_angvel*max_power_angvel*max_power_angvel);
			
			//calculate idle throttle position
			for (idle = 0; idle < 1.0; idle += 0.01)
			{
				if (GetTorqueCurve(idle, start_rpm) > -GetFrictionTorque(start_rpm*3.141593/30.0, 1.0, idle))
				{
					//std::cout << "Found idle throttle: " << idle << ", " << GetTorqueCurve(idle, start_rpm) << ", " << friction_torque << std::endl;
					break;
				}
			}
		}

	void SetMass ( const T& value )
	{
		mass = value;
	}
	

	T GetMass() const
	{
		return mass;
	}

	void SetPosition ( const MATHVECTOR< T, 3 >& value )
	{
		position = value;
	}
	

	MATHVECTOR< T, 3 > GetPosition() const
	{
		return position;
	}
	
	T FuelRate() const
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
