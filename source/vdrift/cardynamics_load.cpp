#include "pch.h"
#include "par.h"
#include "cardynamics.h"
#include "collision_world.h"
#include "game.h"  // tire params map
#include "tobullet.h"
#include "model.h"
#include "settings.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/ShapeData.h"
#include "../ogre/CarPosInfo.h"
#include "Buoyancy.h"
#include <OgreTimer.h>
using namespace std;


CARDYNAMICS::CARDYNAMICS() :
	world(NULL), chassis(NULL), whTrigs(0), pGame(0),
	hover(false), hov_throttle(0.f), hov_roll(0.f),
	sphere(false), sphereYaw(0.f),
	drive(AWD), tacho_rpm(0), engine_vol_mul(1),
	autoclutch(true), autoshift(true), autorear(true),
	shifted(true), shift_gear(0),
	last_auto_clutch(1.0), remaining_shift_time(0.0),
	shift_time(0.2),
	abs(false), tcs(false),
	maxangle(45.0), ang_damp(0.4),
	/*bTerrain(false),*/ pSet(0), pScene(0), poly(NULL),
	doBoost(0.f), doFlip(0.f), boostVal(0.f), fBoostFov(0.f),
	boostFuel(0.f),boostFuelStart(0.f),
	fHitTime(0), fHitForce(0), fParIntens(0), fParVel(0), //hit
	vHitPos(0,0,0), vHitNorm(0,0,0), vHitCarN(0,0,0), vHitDmgN(0,0,0), fHitDmgA(0),
	steerValue(0.f), velPrev(0,0,0),
	fCarScrap(0.f), fCarScreech(0.f),
	time(0.0), fDamage(0), fBncMass(1.0), cam_force(0,0,0)
	//coll_R, coll_W, coll_H, coll_Hofs, coll_Wofs, coll_Lofs
	//coll_posLfront, coll_posLback
{
	for (int i=0; i<4; ++i)
	{	iWhOnRoad[i]=0;
		whTerMtr[i]=0;  whRoadMtr[i]=0;
		whH[i]=0.f;  whP[i]=-1.f;  whDmg[i]=0.f;
	}
	boostFuel = 0.f;  // set later when road length known

	for (int i=0; i<4; ++i)
		rot_coef[i] = 0.0;

	suspension.resize( WHEEL_POSITION_SIZE );
	wheel.resize( WHEEL_POSITION_SIZE );
	//tire.resize( WHEEL_POSITION_SIZE );
	wheel_velocity.resize(WHEEL_POSITION_SIZE);
	wheel_position.resize( WHEEL_POSITION_SIZE );
	wheel_orientation.resize( WHEEL_POSITION_SIZE );
	wheel_contact.resize( WHEEL_POSITION_SIZE );
	brake.resize( WHEEL_POSITION_SIZE );
	abs_active.resize( WHEEL_POSITION_SIZE, false );
	tcs_active.resize( WHEEL_POSITION_SIZE, false );

	hov.Default();
}

void CARDYNAMICS::HoverPar::Default()
{
	//  hover defaults
	hAbove = 2.0f;  hRayLen = 4.0f;

	steerForce = 25.f;
	steerDamp = 10.f;  steerDampP = 14.f;

	engineForce = 18.f;  engineVelDec = 0.006f;
	brakeForce  = 22.f;

	dampAirRes = 42.f;  dampSide = 2100.f;
	dampUp = 1050.f;  dampDn = 3150.f;
	dampPmul = 2.86f;

	alp[0] = 42;  alp[1] = 42;  alp[2] = 32;
	alt[0] = 21;  alt[1] = 21;  alt[2] = 21;
	pitchTq = 30.f;  rollTq = 0.f;  roll = 3.f;

	hov_vz = 0.384f;  hov_vsat = 0.98f;  hov_dsat = 0.9f;
	hov_dampP = 0.5;  hov_damp = 3.6;
	hov_fall = 500.f;  hov_riseP = 2000.f;  hov_rise = 6000.f;
}

CARDYNAMICS::~CARDYNAMICS()
{
	if (poly)
	{
		delete[] poly->verts;
		delete[] poly->faces;
	}
	delete poly;
}

static void ConvertV2to1(float & x, float & y, float & z)
{
	float tx = x, ty = y, tz = z;
	x = ty;  y = -tx;  z = tz;
}


//----------------------------------------------------------------------------------------------------------------------------------
///  Load  (.car file)
//----------------------------------------------------------------------------------------------------------------------------------
bool CARDYNAMICS::Load(GAME* game, CONFIGFILE & c, ostream & error_output)
{
	pGame = game;
	Ogre::Timer ti;

	//bTerrain = false;
	string drive = "RWD";
	int version(1);
	c.GetParam("version", version);
	if (version > 2)
	{
		error_output << "Unsupported car version: " << version << endl;
		return false;
	}
	float temp_vec3[3];

	//load the engine
	{
		float mass, rpm_limit, inertia, friction,
			start_rpm, stall_rpm, fuel_consumption;
		MATHVECTOR<double,3> position;

		if (!c.GetParam("engine.rpm-limit", rpm_limit, error_output))  return false;
		engine.SetRpmMax(rpm_limit);

		if (!c.GetParam("engine.inertia", inertia, error_output))  return false;
		engine.SetInertia(inertia);

		if (!c.GetParam("engine.friction", friction, error_output))  return false;
		engine.SetFrictionB(friction);

		if (!c.GetParam("engine.start-rpm", start_rpm, error_output))  return false;
		engine.SetStartRPM(start_rpm);

		if (!c.GetParam("engine.stall-rpm", stall_rpm, error_output))  return false;
		engine.SetStallRPM(stall_rpm);

		if (!c.GetParam("engine.fuel-consumption", fuel_consumption, error_output))  return false;
		engine.SetFuelConsumption(fuel_consumption);

		if (!c.GetParam("engine.mass", mass, error_output))  return false;
		if (!c.GetParam("engine.position", temp_vec3, error_output))  return false;
		if (version == 2)  ConvertV2to1(temp_vec3[0],temp_vec3[1],temp_vec3[2]);
		position.Set(temp_vec3[0],temp_vec3[1],temp_vec3[2]);
		engine.SetMass(mass);
		engine.SetPosition(position);
		AddMassParticle(mass, position);

		float mul = 1.f, max_torque = 0;
		c.GetParam("engine.torque-val-mul", mul);

		float torque_point[3];
		string torque_str("engine.torque-curve-00");
		vector <pair <double, double> > torques;
		int curve_num = 0;
		while (c.GetParam(torque_str, torque_point))
		{
			max_torque = max(max_torque, torque_point[1] * mul);
			torques.push_back(pair <float, float> (torque_point[0], torque_point[1] * mul));

			curve_num++;
			stringstream str;
			str << "engine.torque-curve-";  str.width(2);  str.fill('0');
			str << curve_num;
			torque_str = str.str();
		}
		if (torques.size() <= 1)
		{
			error_output << "You must define at least 2 torque curve points." << endl;
			return false;
		}
		engine.SetTorqueCurve(rpm_limit, torques);

		//load the clutch
		{
			float mul;  //max_torque = sliding * radius * area * max_pressure;
			//if (!c.GetParam("clutch.max-torque", max_torque, error_output))  return false;
			if (!c.GetParam("clutch.max-torque-mul", mul, error_output))  return false;
			clutch.SetMaxTorque(max_torque * mul);
		}

		//  factor for stats  -
		mul = 1.f;
		if (c.GetParam("engine.real-pow-tq-mul", mul))
			engine.real_pow_tq_mul = mul;
		
		mul = 1.f;
		if (c.GetParam("engine.sound-vol-mul", mul))
			engine_vol_mul = mul;
	}

	//load the transmission
	{
		float time = 0;
		float ratio;
		int gears;

		c.GetParam("transmission.shift-delay", time);
		shift_time = time;

		if (!c.GetParam("transmission.gear-ratio-r", ratio, error_output))  return false;
		transmission.SetGearRatio(-1, ratio);

		if (!c.GetParam("transmission.gears", gears, error_output))  return false;

		for (int i = 0; i < gears; i++)
		{
			stringstream s;
			s << "transmission.gear-ratio-" << i+1;
			if (!c.GetParam(s.str(), ratio, error_output))  return false;
			transmission.SetGearRatio(i+1, ratio);
		}
	}

	//load the differential(s)
	string drivetype;
	if (!c.GetParam("drive", drivetype, error_output))  return false;

	if (drivetype == "hover")  //>
	{	hover = true;  drivetype = "AWD";  }
	else if (drivetype == "sphere")
	{	sphere = true;  drivetype = "AWD";  }
	//if (!hover)
		SetDrive(drivetype);

	float final_drive, a, a_tq(0), a_tq_dec(0);
	///  new 3 sets
	if (drivetype == "AWD" &&
		c.GetParam("diff-center.final-drive", a))
	{
		c.GetParam("diff-rear.anti-slip", a, error_output);
		c.GetParam("diff-rear.torque", a_tq);
		c.GetParam("diff-rear.torque-dec", a_tq_dec);
		diff_rear.SetFinalDrive(1.0);
		diff_rear.SetAntiSlip(a, a_tq, a_tq_dec);

		c.GetParam("diff-front.anti-slip", a, error_output);
		c.GetParam("diff-front.torque", a_tq);
		c.GetParam("diff-front.torque-dec", a_tq_dec);
		diff_front.SetFinalDrive(1.0);
		diff_front.SetAntiSlip(a, a_tq, a_tq_dec);

		c.GetParam("diff-center.final-drive", final_drive, error_output);
		c.GetParam("diff-center.anti-slip", a, error_output);
		c.GetParam("diff-center.torque", a_tq);
		c.GetParam("diff-center.torque-dec", a_tq_dec);
		diff_center.SetFinalDrive(final_drive);
		diff_center.SetAntiSlip(a, a_tq, a_tq_dec);
	}
	else  // old 1 for all
	{
		if (!c.GetParam("differential.final-drive", final_drive, error_output))  return false;
		if (!c.GetParam("differential.anti-slip", a, error_output))  return false;
		c.GetParam("differential.torque", a_tq);
		c.GetParam("differential.torque-dec", a_tq_dec);

		if (drivetype == "RWD")
		{
			diff_rear.SetFinalDrive(final_drive);
			diff_rear.SetAntiSlip(a, a_tq, a_tq_dec);
		}
		else if (drivetype == "FWD")
		{
			diff_front.SetFinalDrive(final_drive);
			diff_front.SetAntiSlip(a, a_tq, a_tq_dec);
		}
		else if (drivetype == "AWD")
		{
			diff_rear.SetFinalDrive(1.0);
			diff_rear.SetAntiSlip(a, a_tq, a_tq_dec);

			diff_front.SetFinalDrive(1.0);
			diff_front.SetAntiSlip(a, a_tq, a_tq_dec);

			diff_center.SetFinalDrive(final_drive);
			diff_center.SetAntiSlip(a, a_tq, a_tq_dec);
		}else
		{	error_output << "Unknown drive type: " << drive << endl;
			return false;
		}
	}

	//load the brake
	{
		for (int i = 0; i < 2; i++)
		{
			string pos = "front";
			WHEEL_POSITION left = FRONT_LEFT;
			WHEEL_POSITION right = FRONT_RIGHT;
			if (i == 1)
			{
				left = REAR_LEFT;
				right = REAR_RIGHT;
				pos = "rear";
			}

			float friction, max_pressure, area, bias, radius, handbrake(0);

			if (!c.GetParam("brakes-"+pos+".friction", friction, error_output))  return false;
			brake[left].SetFriction(friction);
			brake[right].SetFriction(friction);

			if (!c.GetParam("brakes-"+pos+".area", area, error_output))  return false;
			brake[left].SetArea(area);
			brake[right].SetArea(area);

			if (!c.GetParam("brakes-"+pos+".radius", radius, error_output))  return false;
			brake[left].SetRadius(radius);
			brake[right].SetRadius(radius);

			c.GetParam("brakes-"+pos+".handbrake", handbrake);
			brake[left].SetHandbrake(handbrake);
			brake[right].SetHandbrake(handbrake);

			if (!c.GetParam("brakes-"+pos+".bias", bias, error_output))  return false;
			brake[left].SetBias(bias);
			brake[right].SetBias(bias);

			if (!c.GetParam("brakes-"+pos+".max-pressure", max_pressure, error_output))  return false;
			brake[left].SetMaxPressure(max_pressure*bias);
			brake[right].SetMaxPressure(max_pressure*bias);
		}
	}

	//load the fuel tank
	{
		float pos[3];
		MATHVECTOR<double,3> position;
		float capacity, volume, fuel_density;

		if (!c.GetParam("fuel-tank.capacity", capacity, error_output))  return false;
		fuel_tank.SetCapacity(capacity);

		if (!c.GetParam("fuel-tank.volume", volume, error_output))  return false;
		fuel_tank.SetVolume(volume);

		if (!c.GetParam("fuel-tank.fuel-density", fuel_density, error_output))  return false;
		fuel_tank.SetDensity(fuel_density);

		if (!c.GetParam("fuel-tank.position", pos, error_output))  return false;
		if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
		position.Set(pos[0],pos[1],pos[2]);
		fuel_tank.SetPosition(position);
		//AddMassParticle(fuel_density*volume, position);
	}

	//load the suspension
	{
		for (int i = 0; i < 2; i++)
		{
			string posstr = "front";
			string posshortstr = "F";
			WHEEL_POSITION posl = FRONT_LEFT;
			WHEEL_POSITION posr = FRONT_RIGHT;
			if (i == 1)
			{
				posstr = "rear";
				posshortstr = "R";
				posl = REAR_LEFT;
				posr = REAR_RIGHT;
			}

			float spring_constant, bounce, rebound, travel, camber, caster, toe, anti_roll;//, maxcompvel;
			float hinge[3];
			MATHVECTOR<double,3> tempvec;

			if (!c.GetParam("suspension-"+posstr+".spring-constant", spring_constant, error_output))  return false;
			suspension[posl].SetSpringConstant(spring_constant);
			suspension[posr].SetSpringConstant(spring_constant);

			if (!c.GetParam("suspension-"+posstr+".bounce", bounce, error_output))  return false;
			suspension[posl].SetBounce(bounce);
			suspension[posr].SetBounce(bounce);

			if (!c.GetParam("suspension-"+posstr+".rebound", rebound, error_output))  return false;
			suspension[posl].SetRebound(rebound);
			suspension[posr].SetRebound(rebound);

			string file;
			if (c.GetParam("suspension-"+posstr+".factors-file", file))
			{
				int id = game->suspS_map[file]-1;
				if (id == -1)  {  id = 0;
					error_output << "Can't find suspension spring factors file: " << file << endl;  }

				suspension[posl].SetSpringFactorPoints(game->suspS[id]);
				suspension[posr].SetSpringFactorPoints(game->suspS[id]);

				id = game->suspD_map[file]-1;
				if (id == -1)  {  id = 0;
					error_output << "Can't find suspension damper factors file: " << file << endl;  }
				
				suspension[posl].SetDamperFactorPoints(game->suspD[id]);
				suspension[posr].SetDamperFactorPoints(game->suspD[id]);
			}else
			{	//  factor points
				vector <pair <double, double> > damper, spring;
				c.GetPoints("suspension-"+posstr, "damper-factor", damper);
				suspension[posl].SetDamperFactorPoints(damper);
				suspension[posr].SetDamperFactorPoints(damper);

				c.GetPoints("suspension-"+posstr, "spring-factor", spring);
				suspension[posl].SetSpringFactorPoints(spring);
				suspension[posr].SetSpringFactorPoints(spring);
			}

			if (!c.GetParam("suspension-"+posstr+".travel", travel, error_output))  return false;
			suspension[posl].SetTravel(travel);
			suspension[posr].SetTravel(travel);

			if (!c.GetParam("suspension-"+posstr+".camber", camber, error_output))  return false;
			suspension[posl].SetCamber(camber);
			suspension[posr].SetCamber(camber);

			if (!c.GetParam("suspension-"+posstr+".caster", caster, error_output))  return false;
			suspension[posl].SetCaster(caster);
			suspension[posr].SetCaster(caster);

			if (!c.GetParam("suspension-"+posstr+".toe", toe, error_output))  return false;
			suspension[posl].SetToe(toe);
			suspension[posr].SetToe(toe);

			if (!c.GetParam("suspension-"+posstr+".anti-roll", anti_roll, error_output))  return false;
			suspension[posl].SetAntiRollK(anti_roll);
			suspension[posr].SetAntiRollK(anti_roll);

			if (!c.GetParam("suspension-"+posshortstr+"L.hinge", hinge, error_output))  return false;
			//cap hinge to reasonable values
			for (int i = 0; i < 3; i++)
			{
				if (hinge[i] < -100)	hinge[i] = -100;
				if (hinge[i] > 100)		hinge[i] = 100;
			}
			if (version == 2)  ConvertV2to1(hinge[0],hinge[1],hinge[2]);
			tempvec.Set(hinge[0],hinge[1], hinge[2]);
			suspension[posl].SetHinge(tempvec);

			if (!c.GetParam("suspension-"+posshortstr+"R.hinge", hinge, error_output))  return false;
			for (int i = 0; i < 3; i++)
			{
				if (hinge[i] < -100)	hinge[i] = -100;
				if (hinge[i] > 100)		hinge[i] = 100;
			}
			if (version == 2)  ConvertV2to1(hinge[0],hinge[1],hinge[2]);
			tempvec.Set(hinge[0],hinge[1], hinge[2]);
			suspension[posr].SetHinge(tempvec);
		}
	}

	//load the wheels
	{
		for (int i = 0; i < 4; i++)
		{
			string sPos;
			WHEEL_POSITION wp;
			if (i == 0)		{	sPos = "FL";	wp = FRONT_LEFT;	}
			else if (i == 1){	sPos = "FR";	wp = FRONT_RIGHT;	}
			else if (i == 2){	sPos = "RL";	wp = REAR_LEFT;	}
			else			{	sPos = "RR";	wp = REAR_RIGHT;	}

			float roll_h, mass;
			float pos[3];
			MATHVECTOR<double,3> vec;

			if (!c.GetParam("wheel-"+sPos+".mass", mass, error_output))  return false;
			wheel[wp].SetMass(mass);

			if (!c.GetParam("wheel-"+sPos+".roll-height", roll_h, error_output))  return false;
			wheel[wp].SetRollHeight(roll_h);

			if (!c.GetParam("wheel-"+sPos+".position", pos, error_output))  return false;
			if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
			vec.Set(pos[0],pos[1], pos[2]);
			wheel[wp].SetExtendedPosition(vec);

			AddMassParticle(mass, vec);
		}

		//load the rotational inertia parameter from the tire section
		float front_inertia;
		float rear_inertia;
		if (c.GetParam("tire-both.rotational-inertia", front_inertia, error_output))
			rear_inertia = front_inertia;
		else
		{	if (!c.GetParam("tire-front.rotational-inertia", front_inertia, error_output))  return false;
			if (!c.GetParam("tire-rear.rotational-inertia", rear_inertia, error_output))  return false;
		}
		wheel[FRONT_LEFT].SetInertia(front_inertia);
		wheel[FRONT_RIGHT].SetInertia(front_inertia);

		wheel[REAR_LEFT].SetInertia(rear_inertia);
		wheel[REAR_RIGHT].SetInertia(rear_inertia);
	}

	//load the tire parameters
	{
		WHEEL_POSITION leftside = FRONT_LEFT;
		WHEEL_POSITION rightside = FRONT_RIGHT;
		float value;
		bool both = c.GetParam("tire-both.radius", value);
		string posstr = both ? "both" : "front";

		for (int p = 0; p < 2; ++p)
		{
			if (p == 1)
			{
				leftside = REAR_LEFT;
				rightside = REAR_RIGHT;
				if (!both)  posstr = "rear";
			}

			float rolling_resistance[3];
			if (!c.GetParam("tire-"+posstr+".rolling-resistance", rolling_resistance, error_output))  return false;
			wheel[leftside].SetRollingResistance(rolling_resistance[0], rolling_resistance[1]);
			wheel[rightside].SetRollingResistance(rolling_resistance[0], rolling_resistance[1]);

			float radius;
			if (!c.GetParam("tire-"+posstr+".radius", radius, error_output))  return false;
			wheel[leftside].SetRadius(radius);
			wheel[rightside].SetRadius(radius);
		}
	}

	//load the mass-only particles
	{
		MATHVECTOR<double,3> position;
		float pos[3], mass;

		if (c.GetParam("contact-points.mass", mass))
		{
			int paramnum(0);
			string paramname("contact-points.position-00");
			stringstream output_supression;
			while (c.GetParam(paramname, pos))
			{
				if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
				position.Set(pos[0],pos[1],pos[2]);
				AddMassParticle(mass, position);
				paramnum++;
				stringstream str;
				str << "contact-points.position-";  str.width(2);  str.fill('0');
				str << paramnum;
				paramname = str.str();
			}
		}

		string paramname = "particle-00";
		int paramnum = 0;
		while (c.GetParam(paramname+".mass", mass))
		{
			if (!c.GetParam(paramname+".position", pos, error_output))  return false;
			if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
			position.Set(pos[0],pos[1],pos[2]);
			AddMassParticle(mass, position);
			paramnum++;
			stringstream str;
			str << "particle-";  str.width(2);  str.fill('0');
			str << paramnum;
			paramname = str.str();
		}
	}

	//load the max steering angle
	{
		float maxangle = 26.f;
		if (!c.GetParam("steering.max-angle", maxangle, error_output))  return false;
		SetMaxSteeringAngle( maxangle );

		float a = 1.f;
		c.GetParam("steering.flip-pow-mul", a);	 flip_mul = a;
	}
	///car angular damping -new
	{
		float a = 0.4f;
		c.GetParam("steering.angular-damping", a, error_output);
		SetAngDamp(a);

		a=0.f;  c.GetParam("rot_drag.roll", a);  rot_coef[0] = a;
		a=0.f;  c.GetParam("rot_drag.pitch", a); rot_coef[1] = a;
		a=0.f;  c.GetParam("rot_drag.yaw", a);	 rot_coef[2] = a;
		a=0.f;  c.GetParam("rot_drag.yaw2", a);	 rot_coef[3] = a;
	}

	//load the driver
	{
		float mass;
		float pos[3];
		MATHVECTOR<double,3> position;

		if (!c.GetParam("driver.mass", mass, error_output))  return false;
		if (!c.GetParam("driver.position", pos, error_output))  return false;
		if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
		position.Set(pos[0], pos[1], pos[2]);
		AddMassParticle(mass, position);
	}

	//load the aerodynamics
	{
		float drag_area, drag_c, lift_area, lift_c, lift_eff;
		float pos[3];
		MATHVECTOR<double,3> position;

		if (!c.GetParam("drag.frontal-area", drag_area, error_output))  return false;
		if (!c.GetParam("drag.drag-coefficient", drag_c, error_output))  return false;
		if (!c.GetParam("drag.position", pos, error_output))  return false;
		if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
		position.Set(pos[0], pos[1], pos[2]);
		AddAerodynamicDevice(position, drag_area, drag_c, 0,0,0);

		for (int i = 0; i < 2; i++)
		{
			string wingpos = "front";
			if (i == 1)
				wingpos = "rear";
			if (!c.GetParam("wing-"+wingpos+".frontal-area", drag_area, error_output))  return false;
			if (!c.GetParam("wing-"+wingpos+".drag-coefficient", drag_c, error_output))  return false;
			if (!c.GetParam("wing-"+wingpos+".surface-area", lift_area, error_output))  return false;
			if (!c.GetParam("wing-"+wingpos+".lift-coefficient", lift_c, error_output))  return false;
			if (!c.GetParam("wing-"+wingpos+".efficiency", lift_eff, error_output))  return false;
			if (!c.GetParam("wing-"+wingpos+".position", pos, error_output))  return false;
			if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
			position.Set(pos[0], pos[1], pos[2]);
			AddAerodynamicDevice(position, drag_area, drag_c, lift_area, lift_c, lift_eff);
		}
	}

	//  hover params
	if (hover /*|| sphere*/)
	{
		c.GetParam("hover.hAbove",		 hov.hAbove);		c.GetParam("hover.hRayLen",		 hov.hRayLen);
										 
		c.GetParam("hover.steerForce",	 hov.steerForce);
		c.GetParam("hover.steerDamp",	 hov.steerDamp);	c.GetParam("hover.steerDampP",	 hov.steerDampP);
										 
		c.GetParam("hover.engineForce",	 hov.engineForce);	c.GetParam("hover.engineVelDec", hov.engineVelDec);
		c.GetParam("hover.brakeForce",	 hov.brakeForce);
										 
		c.GetParam("hover.dampAirRes",	 hov.dampAirRes);	c.GetParam("hover.dampSide",	 hov.dampSide);
		c.GetParam("hover.dampUp",		 hov.dampUp);		c.GetParam("hover.dampDn",		 hov.dampDn);
		c.GetParam("hover.dampPmul",	 hov.dampPmul);

		float al[3];
		c.GetParam("hover.alignTqP", al);  hov.alp.Set(al[0],al[1],al[2]);
		c.GetParam("hover.alignTq",  al);  hov.alt.Set(al[0],al[1],al[2]);
		c.GetParam("hover.pitchTq",  hov.pitchTq);
		c.GetParam("hover.rollTq",   hov.rollTq);		c.GetParam("hover.roll",     hov.roll);

		c.GetParam("hover.hov_vz",	 hov.hov_vz);		c.GetParam("hover.hov_vsat", hov.hov_vsat);
		c.GetParam("hover.hov_dsat", hov.hov_dsat);
		c.GetParam("hover.hov_dampP",hov.hov_dampP);	c.GetParam("hover.hov_damp", hov.hov_damp);
		c.GetParam("hover.hov_fall", hov.hov_fall);
		c.GetParam("hover.hov_riseP",hov.hov_riseP);	c.GetParam("hover.hov_rise", hov.hov_rise);
	}

	UpdateMass();

	LogO(Ogre::String(":::: Time car dynamics load: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	return true;
}


//----------------------------------------------------------------------------------------------------------------------------------
///  Init  dynamics
//----------------------------------------------------------------------------------------------------------------------------------
void CARDYNAMICS::Init(
	class SETTINGS* pSet1, class Scene* pScene1, class FluidsXml* pFluids1,
	COLLISION_WORLD & world,
	const MODEL & chassisModel, const MODEL & wheelModelFront, const MODEL & wheelModelRear,
	const MATHVECTOR<Dbl,3> & position, const QUATERNION<Dbl> & orientation)
{
	pSet = pSet1;  pScene = pScene1;  pFluids = pFluids1;
	this->world = &world;

	MATHVECTOR<Dbl,3> zero(0, 0, 0);
	body.SetPosition(position);
	body.SetOrientation(orientation);
	body.SetInitialForce(zero);
	body.SetInitialTorque(zero);
	cam_body.SetPosition(zero);
	cam_body.SetInitialForce(zero);

	if (sphere)
	{	Ogre::Quaternion q = Axes::toOgre(orientation);
		sphereYaw = PI_d - q.getYaw().valueRadians();
	}
	
	// init engine
	engine.SetInitialConditions();


	// init chassis
	btTransform tr;
	tr.setIdentity();

	AABB <float> box = chassisModel.GetAABB();
	for (int i = 0; i < 4; i++)
	{
		MATHVECTOR<float,3> wheelpos = GetLocalWheelPosition(WHEEL_POSITION(i), 0);

		const MODEL * wheelmodel = &wheelModelFront;
		if (i > 1) wheelmodel = &wheelModelRear;

		AABB <float> wheelaabb;
		float sidefactor = 1.0;
		if (i == 1 || i == 3) sidefactor = -1.0;

		wheelaabb.SetFromCorners(
			wheelpos - wheelmodel->GetAABB().GetSize() * 0.5 * sidefactor,
			wheelpos + wheelmodel->GetAABB().GetSize() * 0.5 * sidefactor);
		box.CombineWith(wheelaabb);
	}


	///  chassis shape  ---------------------------------------------------------
	const MATHVECTOR<Dbl,3> verticalMargin(0, 0, 0.3);
	btVector3 origin = ToBulletVector(box.GetCenter() + verticalMargin - center_of_mass);
	btVector3 size = ToBulletVector(box.GetSize() - verticalMargin);

	btCollisionShape* chassisShape;
	if (sphere)
	{	chassisShape = new btSphereShape(1.f);
		//chassisShape = new btBoxShape(btVector3(1.f,1.f,0.2f));
		chassisShape->setMargin(0.05f);  //!? doesnt work, bounces too much
	}else
	{	/// todo: all params in .car
		// y| length  x- width  z^ height
		btScalar w = size.getX()*0.2, r = size.getZ()*0.3, h = 0.45;

		///  spheres
		btScalar l0 = 0.f, w0 = 0.f, h0 = 0.f;
		if (coll_R > 0.f)  r = coll_R;  l0 = coll_Lofs;
		if (coll_W > 0.f)  w = coll_W;  w0 = coll_Wofs;
		if (coll_H > 0.f)  h = coll_H;	h0 = coll_Hofs;
		origin = btVector3(l0, w0, h0);

		btScalar r2 = r*0.6;
		btScalar l1 = coll_posLfront, l2 = coll_posLback, l1m = l1*0.5, l2m = l2*0.5;

		//LogO("Car shape dims:  r="+toStr(r)+"  w="+toStr(w)+"  h="+toStr(h)+"  h0="+toStr(h0));
		//LogO("Car offset:  x="+toStr(origin.x())+"  y="+toStr(origin.y())+"  z="+toStr(origin.z()));

		const int numSph = 14;  int i = 0;
		btScalar rad[numSph];  btVector3 pos[numSph];
		float ww = hover ? 0.2f : 1.f;  ///
		pos[i] = btVector3( l1 , -w*ww, -h);    	rad[i] = r2;  ++i;  // front
		pos[i] = btVector3( l1 ,  w*ww, -h);    	rad[i] = r2;  ++i;
		pos[i] = btVector3( l1m, -w*ww, -h);    	rad[i] = r;   ++i;  // front near
		pos[i] = btVector3( l1m,  w*ww, -h);    	rad[i] = r;   ++i;

		pos[i] = btVector3( l2m, -w,    -h);    	rad[i] = r;   ++i;  // rear near
		pos[i] = btVector3( l2m,  w,    -h);    	rad[i] = r;   ++i;
		pos[i] = btVector3( l2 , -w,    -h);    	rad[i] = r2;  ++i;  // rear
		pos[i] = btVector3( l2 ,  w,    -h);    	rad[i] = r2;  ++i;

		pos[i] = btVector3( 0.4, -w*0.8*ww, h*0.2);	rad[i] = r2;  ++i;  // top
		pos[i] = btVector3( 0.4,  w*0.8*ww, h*0.2);	rad[i] = r2;  ++i;
		pos[i] = btVector3(-0.3, -w*0.8*ww, h*0.4);	rad[i] = r2;  ++i;
		pos[i] = btVector3(-0.3,  w*0.8*ww, h*0.4);	rad[i] = r2;  ++i;
		pos[i] = btVector3(-1.1, -w*0.8*ww, h*0.2);	rad[i] = r2;  ++i;  // top rear
		pos[i] = btVector3(-1.1,  w*0.8*ww, h*0.2);	rad[i] = r2;  ++i;

		for (i=0; i < numSph; ++i)
			pos[i] += origin;
		chassisShape = new btMultiSphereShape(pos, rad, numSph);
		chassisShape->setMargin(0.02f);
	}


	Dbl chassisMass = body.GetMass();
	MATRIX3 <Dbl> inertia = body.GetInertia();
	btVector3 chassisInertia(inertia[0], inertia[4], inertia[8]);

	btTransform transform;
	transform.setOrigin(ToBulletVector(position));
	transform.setRotation(ToBulletQuaternion(orientation));
	btDefaultMotionState * chassisState = new btDefaultMotionState();
	chassisState->setWorldTransform(transform);

	btRigidBody::btRigidBodyConstructionInfo info(chassisMass, chassisState, chassisShape, chassisInertia);
	info.m_angularDamping = ang_damp;
	info.m_restitution = 0.0;  //...
	info.m_friction = coll_friction;  /// 0.4~ 0.7
	///  chasis^
	chassis = world.AddRigidBody(info, true, pSet->game.collis_cars);
	chassis->setActivationState(DISABLE_DEACTIVATION);
	chassis->setUserPointer(new ShapeData(ST_Car, this, 0));  ///~~
	
	world.AddAction(this);
	

	///  join chassis and wheel triggers
	//________________________________________________________
	{
		for (int w=0; w < 4; ++w)
		{
			WHEEL_POSITION wp = WHEEL_POSITION(w);
			Dbl whR = GetWheel(wp).GetRadius() * 1.2;  //par bigger
			MATHVECTOR<float,3> wheelpos = GetWheelPosition(wp, 0);  //par
			wheelpos[0] += coll_Lofs;
			wheelpos[2] += coll_flTrig_H;

			btSphereShape* whSph = new btSphereShape(whR);
			//btCylinderShapeX* whSph = new btCylinderShapeX(btVector3(whR,whR,whR));//todo..
			whTrigs = new btRigidBody(0.001f, 0, whSph);
			
			whTrigs->setUserPointer(new ShapeData(ST_Wheel, this, 0, w));  ///~~
			whTrigs->setActivationState(DISABLE_DEACTIVATION);
			whTrigs->setCollisionFlags(whTrigs->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
			world.world->addRigidBody(whTrigs);
			world.shapes.push_back(whSph);
				
			//todo: collision mask only to fluid triggers
			//todo: optimize- 1 constr only or none?
			//todo: cylinders? fixed constr\_
			/*btTransform f1,f2;  f1.setIdentity();  f2.setIdentity();
			f1.setOrigin(ToBulletVector(wheelpos));
			btGeneric6DofConstraint* constr = new btGeneric6DofConstraint(*chassis, *whTrigs, f1, f2, true);
			constr->setLimit(0,0,0);  constr->setLimit(1,0,1);  constr->setLimit(2,0,0);
			//constr->setLimit(3,0,0);
			//constr->setLimit(4,0,0);
			constr->setLimit(5,0,0);/*??*/
			btTypedConstraint* constr = new btPoint2PointConstraint(*chassis, *whTrigs,
				ToBulletVector(wheelpos), btVector3(0,0,0));
			world.world->addConstraint(constr, true);
			world.constraints.push_back(constr);
		}

		///  init poly for buoyancy computations
		//________________________________________________________
		if (poly == NULL)
		{
			poly = new Polyhedron();
			poly->numVerts = 8;  poly->numFaces = 12;
			poly->verts = new Vec3[8];
			poly->faces = new Face[12];

			float hx = 1.2f, hy = 0.7f, hz = 0.4f;  // box dim
			poly->verts[0] = Vec3(-hx,-hy,-hz);	poly->verts[1] = Vec3(-hx,-hy, hz);
			poly->verts[2] = Vec3(-hx, hy,-hz);	poly->verts[3] = Vec3(-hx, hy, hz);
			poly->verts[4] = Vec3( hx,-hy,-hz);	poly->verts[5] = Vec3( hx,-hy, hz);
			poly->verts[6] = Vec3( hx, hy,-hz);	poly->verts[7] = Vec3( hx, hy, hz);

			poly->faces[0] = Face(0,1,3);	poly->faces[1] = Face(0,3,2);	poly->faces[2] = Face(6,3,7);	poly->faces[3] = Face(6,2,3);
			poly->faces[4] = Face(4,6,5);	poly->faces[5] = Face(6,7,5);	poly->faces[6] = Face(4,5,0);	poly->faces[7] = Face(0,5,1);
			poly->faces[8] = Face(5,7,1);	poly->faces[9] = Face(7,3,1);	poly->faces[10]= Face(0,6,4);	poly->faces[11]= Face(0,2,6);

			poly->length = 1.0f;  //  approx. length-?
			poly->volume = ComputeVolume(*poly);

			body_mass = 1900.0f * 2.688;  //poly->volume;  // car density
			body_inertia = (4.0f * body_mass / 12.0f) * btVector3(hy*hz, hx*hz, hx*hy);
		}
	}
	//-------------------------------------------------------------	


	// init wheels, suspension
	for (int i = 0; i < WHEEL_POSITION_SIZE; i++)
	{
		wheel[WHEEL_POSITION(i)].SetInitialConditions();
		wheel_velocity[i].Set(0.0);
		wheel_position[i] = GetWheelPositionAtDisplacement(WHEEL_POSITION(i), 0);
		wheel_orientation[i] = orientation * GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION(i));
	}

	AlignWithGround();//--
}
