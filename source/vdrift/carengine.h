#pragma once
#include <iostream>
#include <map>
#include <fstream>

#include "dbl.h"
#include "rotationalframe.h"
#include "matrix3.h"
#include "spline.h"
#include "linearinterp.h"
#include "mathvector.h"
#include "../ogre/common/Def_Str.h"


class CARENGINE
{
private:
	// constants
	Dbl rpm_max;	///< the redline in RPMs
	//Dbl rpm_limit;	///< peak engine RPMs after which limiting occurs
	Dbl idle;		///< idle throttle percentage; this is calculated algorithmically
	Dbl start_rpm;	///< initial condition RPM
	Dbl stall_rpm;	///< RPM at which the engine dies
	Dbl fuel_consumption;	///< fuel consumed each second (in liters) is the fuel-consumption parameter times RPM times throttle
	Dbl friction;	///< friction coefficient from the engine; this is calculated algorithmically
	Dbl frict_coeffB;	///< friction coefficient

	std::map <Dbl, Dbl> torque_map;  ///< a set of RPMs that map to torque values
	Dbl mass;
	MATHVECTOR<Dbl,3> position;
	SPLINE <Dbl> torque_curve;
	
	// variables
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
	//default constructor makes an S2000-like car
	CARENGINE();

	Dbl real_pow_tq_mul;  // .car params
	std::string sound_name;

	Dbl GetTorqueCurve(const Dbl cur_throttle, const Dbl cur_rpm) const;
	
	Dbl GetFrictionTorque(Dbl cur_angvel, Dbl friction_factor, Dbl throttle_position);

	void SetInertia(const Dbl& value)
	{
		MATRIX3 <Dbl> inertia;
		inertia.Scale(value);
		crankshaft.SetInertia(inertia);
	}
	
	Dbl GetInertia() const
	{
		return crankshaft.GetInertia()[0];
	}
	
	void SetFrictionB(const Dbl& value) {	frict_coeffB = value;	}
	Dbl GetFrictionB() const			{	return frict_coeffB;	}

	void SetRpmMax(const Dbl& value)	{	rpm_max = value;	}
	Dbl GetRpmMax() const				{	return rpm_max;		}
	
	Dbl GetIdle() const		{	return idle;	}

	void SetStartRPM(const Dbl& value)	{	start_rpm = value;	}
	Dbl GetStartRPM() const				{	return start_rpm;	}
	
	void SetStallRPM(const Dbl& value)	{	stall_rpm = value;	}
	Dbl GetStallRPM() const				{	return stall_rpm;	}

	void SetFuelConsumption(const Dbl& value)	{	fuel_consumption = value;	}
	Dbl GetFuelConsumption() const				{	return fuel_consumption;	}
	
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
	void SetThrottle(const Dbl& value)	{	throttle_position = value;	}
	Dbl GetThrottle() const		{	return throttle_position;	}
	
	void SetInitialConditions()
	{
		MATHVECTOR<Dbl,3> v;
		crankshaft.SetInitialTorque(v);
		StartEngine();
	}
	
	void StartEngine()
	{
		MATHVECTOR<Dbl,3> v;
		v[0] = start_rpm * PI_d / 30.0;
		crankshaft.SetAngularVelocity(v);
	}
	
	///used to set the drag on the engine from the clutch being partially engaged
	void SetClutchTorque(Dbl newtorque)		{	clutch_torque = newtorque;	}
	
	Dbl GetAngularVelocity() const
	{
		return crankshaft.GetAngularVelocity()[0];
	}
	
	///used to set the engine speed to the transmission speed when the clutch is fully engaged
	void SetAngularVelocity(Dbl angvel)
	{
		MATHVECTOR<Dbl,3> v(angvel, 0, 0);
		crankshaft.SetAngularVelocity(v);
	}
	
	void DebugPrint(std::ostream & out);
	
	///return the sum of all torques acting on the engine (except clutch forces)
	Dbl GetTorque()
	{
		return combustion_torque + friction_torque;
	}
	
	void ComputeForces();
	
	void ApplyForces();
	
	///Set the torque curve using a vector of (RPM, torque) pairs.
	/// also recalculate engine friction
	/// the max_power_rpm value should be set to the engine's redline
	void SetTorqueCurve(Dbl max_power_rpm, std::vector <std::pair <Dbl, Dbl> > & curve);

	void SetMass(const Dbl& value)	{	mass = value;	}
	Dbl GetMass() const		{	return mass;	}

	void SetPosition(const MATHVECTOR<Dbl,3>& value)	{	position = value;	}
	MATHVECTOR<Dbl,3> GetPosition() const	{	return position;	}
	
	Dbl FuelRate() const
	{
		return fuel_consumption * GetAngularVelocity() * throttle_position;
	}

	void SetOutOfGas(bool value)	{	out_of_gas = value;		}
	
	///returns true if the engine is combusting fuel
	bool GetCombustion() const
	{
		return !stalled;
	}
};
