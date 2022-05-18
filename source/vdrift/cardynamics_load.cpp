#include "pch.h"
#include "par.h"
#include "cardynamics.h"
#include "collision_world.h"
#include "game.h"  // tire params map
#include "tobullet.h"
#include "aabb.h"
#include "settings.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/ShapeData.h"
#include "../ogre/common/Axes.h"
#include "../ogre/CarPosInfo.h"
#include "Buoyancy.h"
#include <OgreTimer.h>
using namespace std;


CARDYNAMICS::CARDYNAMICS() :
	world(NULL), chassis(NULL), whTrigs(0), pGame(0), vtype(V_Car),
	hov_throttle(0.f), hov_roll(0.f), sphereYaw(0.f),
	drive(AWD), tacho_rpm(0), engine_vol_mul(1),
	autoclutch(true), autoshift(true), autorear(true),
	shifted(true), shift_gear(0),
	last_auto_clutch(1.0), rem_shift_time(0.0),
	shift_time(0.2),
	abs(false), tcs(false),
	maxangle(45.0), ang_damp(0.4),
	/*bTerrain(false),*/ pSet(0), pScene(0),
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
	SetNumWheels(4);

	boostFuel = 0.f;  // set later when road length known

	hov.Default();
}

void CARDYNAMICS::SetNumWheels(int n)
{
	numWheels = n;
	suspension.resize(n);  wheel.resize(n);  //tire.resize(n);
	wheel_velocity.resize(n);  wheel_position.resize(n);  wheel_orientation.resize(n);
	wheel_contact.resize(n);  brake.resize(n);
	abs_active.resize(n,false);  tcs_active.resize(n,false);

	iWhOnRoad.resize(n);  whTerMtr.resize(n);  whRoadMtr.resize(n);
	whH.resize(n);  whP.resize(n);  whDmg.resize(n);
	inFluidsWh.resize(n);

	for (int i=0; i < n; ++i)
		whP[i] = -1;
}

void CARDYNAMICS::HoverPar::Default()
{
	//  hover defaults
	hAbove = 2.0f;  hRayLen = 4.0f;

	steerForce = 25.f;
	steerDamp = 10.f;  steerDampP = 14.f;

	engineForce = 18.f;
	engineVelDec = 0.006f;  engineVelDecR = 0.04f;
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
	RemoveBlt();
}

static void ConvertV2to1(float& x, float& y, float& z)
{
	float tx = x, ty = y, tz = z;
	x = ty;  y = -tx;  z = tz;
}

//  common
void CARDYNAMICS::GetWPosStr(int i, int numWheels, WHEEL_POSITION& wl, WHEEL_POSITION& wr, string& pos)
{
	if (numWheels == 2)
	{	if (i==0){	wl = wr = FRONT_LEFT;   pos = "front";  } else
		if (i==1){	wl = wr = FRONT_RIGHT;  pos = "rear";   }
	}else
	{	if (i==0){	wl = FRONT_LEFT;  wr = FRONT_RIGHT;  pos = "front";  } else
		if (i==1){	wl = REAR_LEFT;   wr = REAR_RIGHT;   pos = "rear";   } else
		if (i==2){	wl = REAR2_LEFT;  wr = REAR2_RIGHT;  pos = "rear2";  } else
		if (i==3){	wl = REAR3_LEFT;  wr = REAR3_RIGHT;  pos = "rear3";  }
	}
}


//----------------------------------------------------------------------------------------------------------------------------------
///  Load  (.car file)
//----------------------------------------------------------------------------------------------------------------------------------
bool CARDYNAMICS::Load(GAME* game, CONFIGFILE& c)
{
	pGame = game;
	Ogre::Timer ti;

	//wheels count
	int nw = 0;
	c.GetParam("wheels", nw);
	if (nw >= 2 && nw <= MAX_WHEELS)
		SetNumWheels(nw);

	//bTerrain = false;
	string drive = "RWD";
	int version = 2;
	c.GetParam("version", version);
	if (version > 2)
	{
		LogO(".car Error: Unsupported .car version: "+toStr(version));
		return false;
	}
	float temp_vec3[3];

	//load engine
	{
		float mass, rpm_limit, inertia, friction,
			start_rpm, stall_rpm, fuel_consumption;
		MATHVECTOR<double,3> position;
		
		if (!c.GetParamE("engine.sound", engine.sound_name))
			engine.sound_name = "engine";

		if (!c.GetParamE("engine.rpm-limit", rpm_limit))  return false;
		engine.SetRpmMax(rpm_limit);

		if (!c.GetParamE("engine.inertia", inertia))  return false;
		engine.SetInertia(inertia);

		if (!c.GetParamE("engine.friction", friction))  return false;
		engine.SetFrictionB(friction);

		if (!c.GetParamE("engine.start-rpm", start_rpm))  return false;
		engine.SetStartRPM(start_rpm);

		if (!c.GetParamE("engine.stall-rpm", stall_rpm))  return false;
		engine.SetStallRPM(stall_rpm);

		if (!c.GetParamE("engine.fuel-consumption", fuel_consumption))  return false;
		engine.SetFuelConsumption(fuel_consumption);

		if (!c.GetParamE("engine.mass", mass))  return false;
		if (!c.GetParamE("engine.position", temp_vec3))  return false;
		
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
			LogO(".car Error: You must define at least 2 torque curve points.");
			return false;
		}
		engine.SetTorqueCurve(rpm_limit, torques);

		//load clutch
		{
			float mul;  //max_torque = sliding * radius * area * max_pressure;
			if (!c.GetParamE("clutch.max-torque-mul", mul))  return false;
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

	//load transmission
	{
		float time = 0;
		float ratio;
		int gears;

		c.GetParam("transmission.shift-delay", time);
		shift_time = time;

		if (!c.GetParamE("transmission.gear-ratio-r", ratio))  return false;
		transmission.SetGearRatio(-1, ratio);

		if (!c.GetParamE("transmission.gears", gears))  return false;

		for (int i = 0; i < gears; ++i)
		{
			stringstream s;
			s << "transmission.gear-ratio-" << i+1;
			if (!c.GetParamE(s.str(), ratio))  return false;
			transmission.SetGearRatio(i+1, ratio);
		}
	}

	//load differential(s)
	string drivetype;
	if (!c.GetParamE("drive", drivetype))  return false;

	if (drivetype == "hover")  //>
	{	vtype = V_Spaceship;  drivetype = "AWD";  }
	else if (drivetype == "sphere")
	{	vtype = V_Sphere;  drivetype = "AWD";  }

	SetDrive(drivetype);

	float final, a, a_tq(0), a_tq_dec(0);
	///  new 3 sets
	if (drivetype == "AWD" &&
		c.GetParam("diff-center.final-drive", a))
	{
		c.GetParamE("diff-rear.anti-slip", a);
		c.GetParam("diff-rear.torque", a_tq);
		c.GetParam("diff-rear.torque-dec", a_tq_dec);
		diff_rear.SetFinalDrive(1.0);
		diff_rear.SetAntiSlip(a, a_tq, a_tq_dec);

		c.GetParamE("diff-front.anti-slip", a);
		c.GetParam("diff-front.torque", a_tq);
		c.GetParam("diff-front.torque-dec", a_tq_dec);
		diff_front.SetFinalDrive(1.0);
		diff_front.SetAntiSlip(a, a_tq, a_tq_dec);

		c.GetParamE("diff-center.final-drive", final);
		c.GetParamE("diff-center.anti-slip", a);
		c.GetParam("diff-center.torque", a_tq);
		c.GetParam("diff-center.torque-dec", a_tq_dec);
		diff_center.SetFinalDrive(final);
		diff_center.SetAntiSlip(a, a_tq, a_tq_dec);
	}
	else  // old 1 for all
	{
		if (!c.GetParamE("differential.final-drive", final))  return false;
		if (!c.GetParamE("differential.anti-slip", a))  return false;
		c.GetParam("differential.torque", a_tq);
		c.GetParam("differential.torque-dec", a_tq_dec);

		if (drivetype == "RWD")
		{
			diff_rear.SetFinalDrive(final);		diff_rear.SetAntiSlip(a, a_tq, a_tq_dec);
		}
		else if (drivetype == "FWD")
		{
			diff_front.SetFinalDrive(final);	diff_front.SetAntiSlip(a, a_tq, a_tq_dec);
		}
		else if (drivetype == "AWD")
		{
			diff_rear.SetFinalDrive(1.0);		diff_rear.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_front.SetFinalDrive(1.0);		diff_front.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_center.SetFinalDrive(final);	diff_center.SetAntiSlip(a, a_tq, a_tq_dec);
		}
		else if (drivetype == "6WD")
		{
			diff_front.SetFinalDrive(1.0);		diff_front.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_rear.SetFinalDrive(1.0);		diff_rear.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_center.SetFinalDrive(1.0);		diff_center.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_rear2.SetFinalDrive(1.0);		diff_rear2.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_center2.SetFinalDrive(final);	diff_center2.SetAntiSlip(a, a_tq, a_tq_dec);
		}
		else if (drivetype == "8WD")
		{
			diff_front.SetFinalDrive(1.0);		diff_front.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_rear.SetFinalDrive(1.0);		diff_rear.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_center.SetFinalDrive(1.0);		diff_center.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_rear2.SetFinalDrive(1.0);		diff_rear2.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_rear3.SetFinalDrive(1.0);		diff_rear3.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_center2.SetFinalDrive(1.0);	diff_center2.SetAntiSlip(a, a_tq, a_tq_dec);
			diff_center3.SetFinalDrive(final);	diff_center3.SetAntiSlip(a, a_tq, a_tq_dec);
		}
		else
		{	LogO(".car Error: Unknown drive type: "+drive);
			return false;
		}
	}

	//load brake  (broad lake)
	{	int axles = std::max(2, numWheels/2);
		for (int i = 0; i < axles; ++i)
		{
			WHEEL_POSITION wl, wr;  string pos;
			GetWPosStr(i, numWheels, wl, wr, pos);

			float friction, max_pressure, area, bias, radius, handbrake = 0.f;

			if (!c.GetParamE("brakes-"+pos+".friction", friction))  return false;
			brake[wl].SetFriction(friction);  brake[wr].SetFriction(friction);

			if (!c.GetParamE("brakes-"+pos+".area", area))  return false;
			brake[wl].SetArea(area);  brake[wr].SetArea(area);

			if (!c.GetParamE("brakes-"+pos+".radius", radius))  return false;
			brake[wl].SetRadius(radius);  brake[wr].SetRadius(radius);

			c.GetParam("brakes-"+pos+".handbrake", handbrake);
			brake[wl].SetHandbrake(handbrake);  brake[wr].SetHandbrake(handbrake);

			if (!c.GetParamE("brakes-"+pos+".bias", bias))  return false;
			brake[wl].SetBias(bias);  brake[wr].SetBias(bias);

			if (!c.GetParamE("brakes-"+pos+".max-pressure", max_pressure))  return false;
			brake[wl].SetMaxPressure(max_pressure*bias);  brake[wr].SetMaxPressure(max_pressure*bias);
		}
	}

	//load fuel tank
	{
		float pos[3];
		MATHVECTOR<double,3> position;
		float capacity, volume, fuel_density;

		if (!c.GetParamE("fuel-tank.capacity", capacity))  return false;
		fuel_tank.SetCapacity(capacity);

		if (!c.GetParamE("fuel-tank.volume", volume))  return false;
		fuel_tank.SetVolume(volume);

		if (!c.GetParamE("fuel-tank.fuel-density", fuel_density))  return false;
		fuel_tank.SetDensity(fuel_density);

		if (!c.GetParamE("fuel-tank.position", pos))  return false;
		if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
		position.Set(pos[0],pos[1],pos[2]);
		fuel_tank.SetPosition(position);
		//AddMassParticle(fuel_density*volume, position);
	}

	//load suspension
	{
		for (int i = 0; i < numWheels/2; ++i)
		{
			string pos = "front", possh = "F";  WHEEL_POSITION wl = FRONT_LEFT, wr = FRONT_RIGHT;
			if (i >= 1){  pos = "rear";  possh = "R";  wl = REAR_LEFT;  wr = REAR_RIGHT;  }
			if (i == 2){  wl = REAR2_LEFT;  wr = REAR2_RIGHT;  } else
			if (i == 3){  wl = REAR3_LEFT;  wr = REAR3_RIGHT;  }

			float spring_constant, bounce, rebound, travel, camber, caster, toe, anti_roll;//, maxcompvel;

			if (!c.GetParamE("suspension-"+pos+".spring-constant", spring_constant))  return false;
			suspension[wl].SetSpringConstant(spring_constant);  suspension[wr].SetSpringConstant(spring_constant);

			if (!c.GetParamE("suspension-"+pos+".bounce", bounce))  return false;
			suspension[wl].SetBounce(bounce);  suspension[wr].SetBounce(bounce);

			if (!c.GetParamE("suspension-"+pos+".rebound", rebound))  return false;
			suspension[wl].SetRebound(rebound);  suspension[wr].SetRebound(rebound);

			string file;
			if (c.GetParam("suspension-"+pos+".factors-file", file))
			{
				int id = game->suspS_map[file]-1;
				if (id == -1)  {  id = 0;
					LogO(".car Error: Can't find suspension spring factors file: "+file);  }

				suspension[wl].SetSpringFactorPoints(game->suspS[id]);  suspension[wr].SetSpringFactorPoints(game->suspS[id]);

				id = game->suspD_map[file]-1;
				if (id == -1)  {  id = 0;
					LogO(".car Error: Can't find suspension damper factors file: "+file);  }
				
				suspension[wl].SetDamperFactorPoints(game->suspD[id]);  suspension[wr].SetDamperFactorPoints(game->suspD[id]);
			}else
			{	//  factor points
				vector <pair <double, double> > damper, spring;
				c.GetPoints("suspension-"+pos, "damper-factor", damper);
				suspension[wl].SetDamperFactorPoints(damper);  suspension[wr].SetDamperFactorPoints(damper);

				c.GetPoints("suspension-"+pos, "spring-factor", spring);
				suspension[wl].SetSpringFactorPoints(spring);  suspension[wr].SetSpringFactorPoints(spring);
			}

			if (!c.GetParamE("suspension-"+pos+".travel", travel))  return false;
			suspension[wl].SetTravel(travel);  suspension[wr].SetTravel(travel);

			if (!c.GetParamE("suspension-"+pos+".camber", camber))  return false;
			suspension[wl].SetCamber(camber);  suspension[wr].SetCamber(camber);

			if (!c.GetParamE("suspension-"+pos+".caster", caster))  return false;
			suspension[wl].SetCaster(caster);  suspension[wr].SetCaster(caster);

			if (!c.GetParamE("suspension-"+pos+".toe", toe))  return false;
			suspension[wl].SetToe(toe);  suspension[wr].SetToe(toe);

			if (!c.GetParamE("suspension-"+pos+".anti-roll", anti_roll))  return false;
			suspension[wl].SetAntiRollK(anti_roll);  suspension[wr].SetAntiRollK(anti_roll);

			//  hinge
			float hinge[3];  MATHVECTOR<Dbl,3> vec;

			if (!c.GetParamE("suspension-"+possh+"L.hinge", hinge))  return false;
			//cap hinge to reasonable values
			for (int i = 0; i < 3; ++i)
			{
				if (hinge[i] < -100)	hinge[i] = -100;
				if (hinge[i] > 100)		hinge[i] = 100;
			}
			if (version == 2)  ConvertV2to1(hinge[0],hinge[1],hinge[2]);
			vec.Set(hinge[0],hinge[1], hinge[2]);
			suspension[wl].SetHinge(vec);

			if (!c.GetParamE("suspension-"+possh+"R.hinge", hinge))  return false;
			for (int i = 0; i < 3; ++i)
			{
				if (hinge[i] < -100)	hinge[i] = -100;
				if (hinge[i] > 100)		hinge[i] = 100;
			}
			if (version == 2)  ConvertV2to1(hinge[0],hinge[1],hinge[2]);
			vec.Set(hinge[0],hinge[1], hinge[2]);
			suspension[wr].SetHinge(vec);
		}
	}

	//load wheels
	{
		for (int i = 0; i < numWheels; ++i)
		{
			string sPos = sCfgWh[i];
			WHEEL_POSITION wp = WHEEL_POSITION(i);

			float roll_h, mass, steer;
			float pos[3];  MATHVECTOR<Dbl,3> vec;

			if (!c.GetParamE("wheel-"+sPos+".mass", mass))  return false;
			wheel[wp].SetMass(mass);

			if (!c.GetParamE("wheel-"+sPos+".roll-height", roll_h))  return false;
			wheel[wp].SetRollHeight(roll_h);

			if (numWheels == 4)  // cars default
				wheel[wp].SetSteerMax(i < REAR_LEFT ? 1.0 : 0.0);
			//  6,8 wheels  have custom
			if (c.GetParam("wheel-"+sPos+".steer", steer))
				wheel[wp].SetSteerMax(steer);

			if (!c.GetParamE("wheel-"+sPos+".position", pos))  return false;
			if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
			vec.Set(pos[0],pos[1], pos[2]);
			wheel[wp].SetExtendedPosition(vec);

			AddMassParticle(mass, vec);
		}

		//load the rotational inertia parameter from the tire section
		float front,rear;
		if (c.GetParamE("tire-both.rotational-inertia", front))
			rear = front;
		else
		{	if (!c.GetParamE("tire-front.rotational-inertia", front))  return false;
			if (!c.GetParamE("tire-rear.rotational-inertia", rear))  return false;
		}
		wheel[FRONT_LEFT].SetInertia(front);
		wheel[FRONT_RIGHT].SetInertia(front);

		for (int i=REAR_LEFT; i <= REAR3_RIGHT; ++i)
			if (i < numWheels)	wheel[i].SetInertia(rear);
	}

	//load tire parameters
	{
		float val;
		bool both = c.GetParam("tire-both.radius", val);

		int axles = std::max(2, numWheels/2);
		for (int i = 0; i < axles; ++i)
		{
			WHEEL_POSITION wl, wr;  string pos;
			GetWPosStr(i, numWheels, wl, wr, pos);
			if (both)  pos = "both";

			float rolling_resistance[3];
			if (!c.GetParamE("tire-"+pos+".rolling-resistance", rolling_resistance))  return false;
			wheel[wl].SetRollingResistance(rolling_resistance[0], rolling_resistance[1]);
			wheel[wr].SetRollingResistance(rolling_resistance[0], rolling_resistance[1]);

			float radius, ray_len, friction;
			if (!c.GetParamE("tire-"+pos+".radius", radius))  return false;
			wheel[wl].SetRadius(radius);
			wheel[wr].SetRadius(radius);
			
			if (c.GetParam("tire-"+pos+".ray-length", ray_len))
			{	wheel[wl].SetRayLength(ray_len);
				wheel[wr].SetRayLength(ray_len);  }
			
			if (c.GetParam("tire-"+pos+".friction", friction))
			{	wheel[wl].SetFriction(friction);
				wheel[wr].SetFriction(friction);  }
		}
	}

	//load mass-only particles
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
			if (!c.GetParamE(paramname+".position", pos))  return false;
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

	//load max steering angle
	{
		float maxangle = 26.f;
		if (!c.GetParamE("steering.max-angle", maxangle))  return false;
		SetMaxSteeringAngle( maxangle );

		float a = 1.f;
		c.GetParam("steering.flip-pow-mul", a);	 flip_mul = a;
	}
	///car angular damping -new
	{
		float a = 0.4f;
		c.GetParamE("steering.angular-damping", a);
		SetAngDamp(a);

		a=0.f;  c.GetParam("rot_drag.roll", a);  rot_coef[0] = a;
		a=0.f;  c.GetParam("rot_drag.pitch", a); rot_coef[1] = a;
		a=0.f;  c.GetParam("rot_drag.yaw", a);	 rot_coef[2] = a;
		a=0.f;  c.GetParam("rot_drag.yaw2", a);	 rot_coef[3] = a;
	}

	//load driver
	{
		float mass;
		float pos[3];
		MATHVECTOR<double,3> position;

		if (!c.GetParamE("driver.mass", mass))  return false;
		if (!c.GetParamE("driver.position", pos))  return false;
		if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
		position.Set(pos[0], pos[1], pos[2]);
		AddMassParticle(mass, position);
	}

	//load aerodynamics
	{
		float drag_area, drag_c, lift_area, lift_c, lift_eff;
		float pos[3];
		MATHVECTOR<double,3> position;

		if (!c.GetParamE("drag.frontal-area", drag_area))  return false;
		if (!c.GetParamE("drag.drag-coefficient", drag_c))  return false;
		if (!c.GetParamE("drag.position", pos))  return false;
		if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
		position.Set(pos[0], pos[1], pos[2]);
		AddAerodynamicDevice(position, drag_area, drag_c, 0,0,0);

		for (int i = 0; i < 2; ++i)
		{
			string wingpos = i==1 ? "rear" : "front";
			if (!c.GetParamE("wing-"+wingpos+".frontal-area", drag_area))  return false;
			if (!c.GetParamE("wing-"+wingpos+".drag-coefficient", drag_c))  return false;
			if (!c.GetParamE("wing-"+wingpos+".surface-area", lift_area))  return false;
			if (!c.GetParamE("wing-"+wingpos+".lift-coefficient", lift_c))  return false;
			if (!c.GetParamE("wing-"+wingpos+".efficiency", lift_eff))  return false;
			if (!c.GetParamE("wing-"+wingpos+".position", pos))  return false;
			if (version == 2)  ConvertV2to1(pos[0],pos[1],pos[2]);
			position.Set(pos[0], pos[1], pos[2]);
			AddAerodynamicDevice(position, drag_area, drag_c, lift_area, lift_c, lift_eff);
		}
	}

	//  hover params
	if (vtype != V_Car)
	{
		c.GetParam("hover.hAbove",		 hov.hAbove);		c.GetParam("hover.hRayLen",		 hov.hRayLen);
										 
		c.GetParam("hover.steerForce",	 hov.steerForce);
		c.GetParam("hover.steerDamp",	 hov.steerDamp);	c.GetParam("hover.steerDampP",	 hov.steerDampP);
										 
		c.GetParam("hover.engineForce",	 hov.engineForce);
		c.GetParam("hover.engineVelDec", hov.engineVelDec);	c.GetParam("hover.engineVelDecR", hov.engineVelDecR);
		c.GetParam("hover.brakeForce",	 hov.brakeForce);
										 
		c.GetParam("hover.dampAirRes",	 hov.dampAirRes);	c.GetParam("hover.dampSide",	 hov.dampSide);
		c.GetParam("hover.dampUp",		 hov.dampUp);		c.GetParam("hover.dampDn",		 hov.dampDn);
		c.GetParam("hover.dampPmul",	 hov.dampPmul);

		float al[3];
		c.GetParam("hover.alignTqP", al);  hov.alp.Set(al[0],al[1],al[2]);
		c.GetParam("hover.alignTq",  al);  hov.alt.Set(al[0],al[1],al[2]);
		c.GetParam("hover.pitchTq",  hov.pitchTq);
		c.GetParam("hover.rollTq",   hov.rollTq);		c.GetParam("hover.roll",     hov.roll);

		c.GetParam("hover_h.hov_vz",   hov.hov_vz);		c.GetParam("hover_h.hov_vsat", hov.hov_vsat);
		c.GetParam("hover_h.hov_dsat", hov.hov_dsat);
		c.GetParam("hover_h.hov_dampP",hov.hov_dampP);	c.GetParam("hover_h.hov_damp", hov.hov_damp);
		c.GetParam("hover_h.hov_fall", hov.hov_fall);
		c.GetParam("hover_h.hov_riseP",hov.hov_riseP);	c.GetParam("hover_h.hov_rise", hov.hov_rise);
	}

	UpdateMass();

	LogO(":::: Time car dynamics load: " + fToStr(ti.getMilliseconds(),0,3) + " ms");
	return true;
}


//----------------------------------------------------------------------------------------------------------------------------------
///  Init  dynamics
//----------------------------------------------------------------------------------------------------------------------------------
void CARDYNAMICS::Init(
	class SETTINGS* pSet1, class Scene* pScene1, class FluidsXml* pFluids1,
	COLLISION_WORLD& world,
	const MATHVECTOR<Dbl,3>& position, const QUATERNION<Dbl>& orientation)
{
	pSet = pSet1;  pScene = pScene1;  pFluids = pFluids1;
	this->world = &world;

	MATHVECTOR<Dbl,3> zero(0, 0, 0);
	body.SetPosition(position);  body.SetOrientation(orientation);
	body.SetInitialForce(zero);  body.SetInitialTorque(zero);
	cam_body.SetPosition(zero);  cam_body.SetInitialForce(zero);

	if (vtype == V_Sphere)
	{	Ogre::Quaternion q = Axes::toOgre(orientation);
		sphereYaw = PI_d - q.getYaw().valueRadians();
	}
	
	// init engine
	engine.SetInitialConditions();


	// init chassis
	btTransform tr;  tr.setIdentity();

	AABB <float> box;
	for (int i = 0; i < numWheels; ++i)
	{
		MATHVECTOR<float,3> wheelpos = GetLocalWheelPosition(WHEEL_POSITION(i), 0);

		AABB <float> wheelaabb;
		wheelaabb.SetFromCorners(wheelpos, wheelpos);
		box.CombineWith(wheelaabb);
	}


	///  chassis shape  ---------------------------------------------------------
	const MATHVECTOR<Dbl,3> verticalMargin(0, 0, 0.3);
	btVector3 origin = ToBulletVector(box.GetCenter() + verticalMargin - center_of_mass);
	btVector3 size = ToBulletVector(box.GetSize() - verticalMargin);

	btCollisionShape* chassisShape;
	if (vtype == V_Sphere)
	{	chassisShape = new btSphereShape(1.f);
		//chassisShape = new btBoxShape(btVector3(1.f,1.f,0.2f));
		chassisShape->setMargin(0.05f);  //!? doesnt work, bounces too much
	}else
	{	// y| length  x- width  z^ height
		btScalar w = size.getX()*0.2, r = size.getZ()*0.3, h = 0.45;

		///  spheres
		btScalar l0 = 0.f, w0 = 0.f, h0 = 0.f;
		if (coll_R > 0.f)  r = coll_R;  l0 = coll_Lofs;
		if (coll_W > 0.f)  w = coll_W;  w0 = coll_Wofs;
		if (coll_H > 0.f)  h = coll_H;	h0 = coll_Hofs;
		origin = btVector3(l0, w0, h0);

		const int numSph = 14;  int i = 0;
		btScalar rad[numSph];  btVector3 pos[numSph];

		btScalar r2 = r * coll_R2m;
		btScalar l1 = coll_posLfront, l2 = coll_posLback, l1m = l1*0.5, l2m = l2*0.5;
		float ww = vtype == V_Spaceship ? coll_FrWmul : 1.f;
		float wt = coll_TopWmul * ww, wf = coll_FrontWm;

		rad[i] = r2;  pos[i] = btVector3( l1 , -w*ww*wf, -h*coll_FrHmul);  ++i;  // front
		rad[i] = r2;  pos[i] = btVector3( l1 ,  w*ww*wf, -h*coll_FrHmul);  ++i;
		rad[i] = r;   pos[i] = btVector3( l1m, -w*ww,    -h*coll_FrHmul);  ++i;  // front near
		rad[i] = r;   pos[i] = btVector3( l1m,  w*ww,    -h*coll_FrHmul);  ++i;
		
		rad[i] = r;   pos[i] = btVector3( l2m, -w,    -h);  ++i;  // rear near
		rad[i] = r;   pos[i] = btVector3( l2m,  w,    -h);  ++i;
		rad[i] = r2;  pos[i] = btVector3( l2 , -w,    -h);  ++i;  // rear
		rad[i] = r2;  pos[i] = btVector3( l2 ,  w,    -h);  ++i;
		
		rad[i] = r2;  pos[i] = btVector3( coll_TopFr,  -w*wt*wf, h*coll_TopFrHm  );  ++i;  // top
		rad[i] = r2;  pos[i] = btVector3( coll_TopFr,   w*wt*wf, h*coll_TopFrHm  );  ++i;
		rad[i] = r2;  pos[i] = btVector3( coll_TopMid, -w*wt,    h*coll_TopMidHm );  ++i;
		rad[i] = r2;  pos[i] = btVector3( coll_TopMid,  w*wt,    h*coll_TopMidHm );  ++i;
		rad[i] = r2;  pos[i] = btVector3( coll_TopBack,-w*wt,    h*coll_TopBackHm);  ++i;  // top rear
		rad[i] = r2;  pos[i] = btVector3( coll_TopBack, w*wt,    h*coll_TopBackHm);  ++i;

		for (i=0; i < numSph; ++i)
			pos[i] += origin;
		chassisShape = new btMultiSphereShape(pos, rad, numSph);
		chassisShape->setMargin(0.02f);  //par?
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
	shapes.push_back(chassisShape);

	///  chasis^
	chassis = world.AddRigidBody(info, true, pSet->game.collis_cars);  rigids.push_back(chassis);
	//TODO: update this when car rewinds.. so they don't collide and "explode"
	//chassis->getBroadphaseProxy()->m_collisionFilterMask = 0; //setCollisionFilterMask();
	chassis->setActivationState(DISABLE_DEACTIVATION);
	chassis->setUserPointer(new ShapeData(ST_Car, this, 0));  ///~~
	
	world.world->addAction(this);  actions.push_back(this);
	

	///  join chassis and wheel triggers
	//________________________________________________________
	{
		for (int w=0; w < numWheels; ++w)
		{
			WHEEL_POSITION wp = WHEEL_POSITION(w);
			Dbl whR = GetWheel(wp).GetRadius() * 1.2;  //par bigger
			MATHVECTOR<float,3> wheelpos = GetWheelPosition(wp, 0);  //par
			wheelpos[0] += coll_Lofs;
			wheelpos[2] += coll_flTrig_H;

			btSphereShape* whSph = new btSphereShape(whR);
			//btCylinderShapeX* whSph = new btCylinderShapeX(btVector3(whR,whR,whR)); /todo..
			whTrigs = new btRigidBody(0.001f, 0, whSph);
			
			whTrigs->setUserPointer(new ShapeData(ST_Wheel, this, 0, w));  ///~~
			whTrigs->setActivationState(DISABLE_DEACTIVATION);
			whTrigs->setCollisionFlags(whTrigs->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

			world.world->addRigidBody(whTrigs);  rigids.push_back(whTrigs);
			world.shapes.push_back(whSph);  shapes.push_back(whSph);
				
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

			world.world->addConstraint(constr, true);  constraints.push_back(constr);
		}

		///  init poly for buoyancy computations
		//________________________________________________________
		poly.verts.clear();  poly.verts.resize(8);
		poly.faces.clear();  poly.faces.resize(12);

		const float& hx = buoy_X, hy = buoy_Y, hz = buoy_Z;  // box dim
		poly.verts[0] = Vec3(-hx,-hy,-hz);  poly.verts[1] = Vec3(-hx,-hy, hz);
		poly.verts[2] = Vec3(-hx, hy,-hz);  poly.verts[3] = Vec3(-hx, hy, hz);
		poly.verts[4] = Vec3( hx,-hy,-hz);  poly.verts[5] = Vec3( hx,-hy, hz);
		poly.verts[6] = Vec3( hx, hy,-hz);  poly.verts[7] = Vec3( hx, hy, hz);

		poly.faces[0] = BFace(0,1,3);  poly.faces[1] = BFace(0,3,2);
		poly.faces[2] = BFace(6,3,7);  poly.faces[3] = BFace(6,2,3);
		poly.faces[4] = BFace(4,6,5);  poly.faces[5] = BFace(6,7,5);
		poly.faces[6] = BFace(4,5,0);  poly.faces[7] = BFace(0,5,1);
		poly.faces[8] = BFace(5,7,1);  poly.faces[9] = BFace(7,3,1);
		poly.faces[10]= BFace(0,6,4);  poly.faces[11]= BFace(0,2,6);

		poly.length = 1.f;  //  for drag torque-
		poly.volume = ComputeVolume(poly);

		body_mass = buoy_Mul * 1900.f * 2.688f;  // car density-
		body_inertia = (4.f * body_mass / 12.f) * btVector3(hy*hz, hx*hz, hx*hy);
	}
	//-------------------------------------------------------------	


	// init wheels, suspension
	for (int i = 0; i < numWheels; ++i)
	{
		wheel[WHEEL_POSITION(i)].SetInitialConditions();
		wheel_velocity[i].Set(0.0);
		wheel_position[i] = GetWheelPositionAtDisplacement(WHEEL_POSITION(i), 0);
		wheel_orientation[i] = orientation * GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION(i));
	}

	AlignWithGround();//--
}


//  remove from bullet
//-------------------------------------------------------------	
void CARDYNAMICS::RemoveBlt()
{
	int i,c;
	for (i = 0; i < constraints.size(); ++i)
	{
		world->world->removeConstraint(constraints[i]);
		delete constraints[i];
	}
	constraints.resize(0);
	
	for (i = rigids.size()-1; i >= 0; i--)
	{
		btRigidBody* body = rigids[i];
		if (body && body->getMotionState())
			delete body->getMotionState();

		world->world->removeRigidBody(body);

		ShapeData* sd = (ShapeData*)body->getUserPointer();
		delete sd;
		delete body;
	}
	
	for (i = 0; i < shapes.size(); ++i)
	{
		btCollisionShape* shape = shapes[i];
		world->shapes.remove(shape);  // duplicated

		if (shape->isCompound())
		{
			btCompoundShape* cs = (btCompoundShape *)shape;
			for (c = 0; c < cs->getNumChildShapes(); ++c)
				delete cs->getChildShape(c);
		}
		ShapeData* sd = (ShapeData*)shape->getUserPointer();
		delete sd;
		delete shape;
	}
	shapes.resize(0);
	
	for (i = 0; i < actions.size(); ++i)
		world->world->removeAction(actions[i]);

	actions.resize(0);
}
