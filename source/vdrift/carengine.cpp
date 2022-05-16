#include "pch.h"
#include "carengine.h"
#include "../ogre/common/Def_Str.h"


CARENGINE::CARENGINE()
	:rpm_max(7800), /*rpm_limit(9000),*/ idle(0.02), frict_coeffB(230), real_pow_tq_mul(1.0)
	,start_rpm(1000), stall_rpm(350), fuel_consumption(1e-9), friction(0.000328)
	,throttle_position(0.0), clutch_torque(0.0), out_of_gas(false), mass(200)
	,rev_limit_exceeded(false), friction_torque(0), combustion_torque(0), stalled(false)
{
    MATRIX3 <Dbl> inertia;
    inertia.Scale(0.25);
    crankshaft.SetInertia(inertia);
}

Dbl CARENGINE::GetTorqueCurve(const Dbl cur_throttle, const Dbl cur_rpm) const
{
	if (cur_rpm < 1)
		return 0.0;
	
	Dbl torque = torque_curve.Interpolate(cur_rpm);
	
	//  make sure the real function only returns values > 0
	return torque*cur_throttle;
}

Dbl CARENGINE::GetFrictionTorque(Dbl cur_angvel, Dbl friction_factor, Dbl throttle_position)
{
	Dbl velsign = cur_angvel < 0 ? -1.0 : 1.0;
	Dbl B = frict_coeffB * friction;
	//return (A - cur_angvel * B - velsign * C * cur_angvel * cur_angvel) *
	return (- cur_angvel * B) *
			(1.0 - friction_factor*throttle_position);
}

void CARENGINE::DebugPrint(std::ostream & out)
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

void CARENGINE::ComputeForces()
{
	if (GetRPM() < stall_rpm)
		stalled = true;
	else
		stalled = false;
	
	//  make sure the throttle is at least idling
	if (throttle_position < idle)
		throttle_position = idle;
	
	//  engine friction
	Dbl cur_angvel = crankshaft.GetAngularVelocity()[0];
	
	//  engine drive torque
	Dbl friction_factor = 1.0; // used to make sure we allow friction to work if we're out of gas or above the rev limit
	Dbl rev_limit = rpm_max+500; // rpm_limit;
	if (rev_limit_exceeded)
		rev_limit -= 400.0;  ///par
	
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
		//  try to model the static friction of the engine
		friction_torque *= 100.0;
	}
}

void CARENGINE::ApplyForces()
{
	MATHVECTOR<Dbl,3> total_torque(0);
	
	total_torque[0] += combustion_torque;
	total_torque[0] += friction_torque;
	total_torque[0] -= clutch_torque;
	
	crankshaft.SetTorque(total_torque);
}

//  Set the torque curve using a vector of (RPM, torque) pairs.
//  also recalculate engine friction
//  the max_power_rpm value should be set to the engine's redline
void CARENGINE::SetTorqueCurve(Dbl max_power_rpm, std::vector <std::pair <Dbl, Dbl> > & curve)
{
	torque_curve.Clear();
	
	assert(curve.size() > 1);
	
	//  ensure we have a smooth curve down to 0 RPM
	if (curve[0].first != 0)
		torque_curve.AddPoint(0,0);
	
	for (auto i : curve)
	{
		torque_curve.AddPoint(i.first, i.second);
	}
	
	//  ensure we have a smooth curve for over-revs
	torque_curve.AddPoint(curve[curve.size()-1].first + 10000, 0);
	
	//  write out a debug torque curve file
	/*std::ofstream f("out.dat");
	for (Dbl i = 0; i < curve[curve.size()-1].first+1000; i+= 20) f << i << " " << torque_curve.Interpolate(i) << std::endl;*/
	//for (unsigned int i = 0; i < curve.size(); ++i) f << curve[i].first << " " << curve[i].second << std::endl;
	
	//  calculate engine friction
	Dbl max_power_angvel = max_power_rpm * PI_d/30.0;
	Dbl max_power = torque_curve.Interpolate(max_power_rpm)*max_power_angvel;
	friction = max_power / (max_power_angvel*max_power_angvel*max_power_angvel);
	
	//  calculate idle throttle position
	for (idle = 0; idle < 1.0; idle += 0.01)
	{
		if (GetTorqueCurve(idle, start_rpm) > -GetFrictionTorque(start_rpm*PI_d/30.0, 1.0, idle))
		{
			//std::cout << "Found idle throttle: " << idle << ", " << GetTorqueCurve(idle, start_rpm) << ", " << friction_torque << std::endl;
			break;
		}
	}
}
