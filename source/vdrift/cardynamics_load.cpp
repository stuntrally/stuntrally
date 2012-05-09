#include "pch.h"
#include "cardynamics.h"
#include "coordinatesystems.h"
#include "collision_world.h"
#include "tobullet.h"
#include "model.h"
#include "settings.h"
#include "../ogre/common/Defines.h"
#include "Buoyancy.h"
#include "../ogre/common/QTimer.h"

typedef CARDYNAMICS::T T;


CARDYNAMICS::CARDYNAMICS() :
	world(NULL), chassis(NULL), whTrigs(0),
	drive(RWD), tacho_rpm(0),
	autoclutch(true), autoshift(true), autorear(true),
	shifted(true), shift_gear(0),
	last_auto_clutch(1.0), remaining_shift_time(0.0),
	shift_time(0.2),
	abs(false), tcs(false),
	maxangle(45.0),
	bTerrain(false), pSet(0), pScene(0),
	doBoost(0), doFlip(0), boostFuel(0), boostVal(0),
	fHitTime(0), fHitForce(0), fParIntens(0), fParVel(0), //hit
	vHitPos(0,0,0), vHitNorm(0,0,0),
	steerValue(0.f), velPrev(0,0,0),
	fCarScrap(0.f), fCarScreech(0.f)
{
	for (int i=0; i<4; ++i)
	{	bWhOnRoad[i]=0;  terSurf[i]=0;
		whH[i]=0.f;  whP[i]=-1;
	}
	boostFuel = gfBoostFuelStart;

	suspension.resize ( WHEEL_POSITION_SIZE );
	wheel.resize ( WHEEL_POSITION_SIZE );
	tire.resize ( WHEEL_POSITION_SIZE );
	wheel_velocity.resize (WHEEL_POSITION_SIZE);
	wheel_position.resize ( WHEEL_POSITION_SIZE );
	wheel_orientation.resize ( WHEEL_POSITION_SIZE );
	wheel_contact.resize ( WHEEL_POSITION_SIZE );
	brake.resize ( WHEEL_POSITION_SIZE );
	abs_active.resize ( WHEEL_POSITION_SIZE, false );
	tcs_active.resize ( WHEEL_POSITION_SIZE, false );

	poly = NULL;
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


//----------------------------------------------------------------------------------------------------------------------------------
///  Load  (.car file)
//----------------------------------------------------------------------------------------------------------------------------------
bool CARDYNAMICS::Load(CONFIGFILE & c, std::ostream & error_output)
{
	QTimer ti;  ti.update(); /// time

	bTerrain = false;
	std::string drive = "RWD";
	int version(1);
	c.GetParam("version", version);
	if (version > 2)
	{
		error_output << "Unsupported car version: " << version << std::endl;
		return false;
	}
	float temp_vec3[3];

	//load the engine
	{
		float engine_mass, engine_redline, engine_rpm_limit, engine_inertia,
			engine_start_rpm, engine_stall_rpm, engine_fuel_consumption;
		MATHVECTOR <double, 3> engine_position;

		if (!c.GetParam("engine.peak-engine-rpm", engine_redline, error_output)) return false; //used only for the redline graphics
		engine.SetRedline(engine_redline);

		if (!c.GetParam("engine.rpm-limit", engine_rpm_limit, error_output)) return false;
		engine.SetRPMLimit(engine_rpm_limit);

		if (!c.GetParam("engine.inertia", engine_inertia, error_output)) return false;
		engine.SetInertia(engine_inertia);

		if (!c.GetParam("engine.start-rpm", engine_start_rpm, error_output)) return false;
		engine.SetStartRPM(engine_start_rpm);

		if (!c.GetParam("engine.stall-rpm", engine_stall_rpm, error_output)) return false;
		engine.SetStallRPM(engine_stall_rpm);

		if (!c.GetParam("engine.fuel-consumption", engine_fuel_consumption, error_output)) return false;
		engine.SetFuelConsumption(engine_fuel_consumption);

		if (!c.GetParam("engine.mass", engine_mass, error_output)) return false;
		if (!c.GetParam("engine.position", temp_vec3, error_output)) return false;
		if (version == 2)
		{
			COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(temp_vec3[0],temp_vec3[1],temp_vec3[2]);
		}
		engine_position.Set(temp_vec3[0],temp_vec3[1],temp_vec3[2]);
		engine.SetMass(engine_mass);
		engine.SetPosition(engine_position);
		AddMassParticle(engine_mass, engine_position);

		float torque_point[3];
		std::string torque_str("engine.torque-curve-00");
		std::vector <std::pair <double, double> > torques;
		int curve_num = 0;
		while (c.GetParam(torque_str, torque_point))
		{
			torques.push_back(std::pair <float, float> (torque_point[0], torque_point[1]));

			curve_num++;
			std::stringstream str;
			str << "engine.torque-curve-";
			str.width(2);
			str.fill('0');
			str << curve_num;
			torque_str = str.str();
		}
		if (torques.size() <= 1)
		{
			error_output << "You must define at least 2 torque curve points." << std::endl;
			return false;
		}
		engine.SetTorqueCurve(engine_redline, torques);
	}

	//load the clutch
	{
		float sliding, radius, area, max_pressure;

		if (!c.GetParam("clutch.sliding", sliding, error_output)) return false;
		clutch.SetSlidingFriction(sliding);

		if (!c.GetParam("clutch.radius", radius, error_output)) return false;
		clutch.SetRadius(radius);

		if (!c.GetParam("clutch.area", area, error_output)) return false;
		clutch.SetArea(area);

		if (!c.GetParam("clutch.max-pressure", max_pressure, error_output)) return false;
		clutch.SetMaxPressure(max_pressure);
	}

	//load the transmission
	{
		float time = 0;
		float ratio;
		int gears;

		c.GetParam("transmission.shift-time", time);
		shift_time = time;

		if (!c.GetParam("transmission.gear-ratio-r", ratio, error_output)) return false;
		transmission.SetGearRatio(-1, ratio);

		if (!c.GetParam("transmission.gears", gears, error_output)) return false;

		for (int i = 0; i < gears; i++)
		{
			std::stringstream s;
			s << "transmission.gear-ratio-" << i+1;
			if (!c.GetParam(s.str(), ratio, error_output)) return false;
			transmission.SetGearRatio(i+1, ratio);
		}
	}

	//load the differential(s)
	{
		float final_drive, anti_slip, anti_slip_torque(0), anti_slip_torque_deceleration_factor(0);

		if (!c.GetParam("differential.final-drive", final_drive, error_output)) return false;
		if (!c.GetParam("differential.anti-slip", anti_slip, error_output)) return false;
		c.GetParam("differential.anti-slip-torque", anti_slip_torque);
		c.GetParam("differential.anti-slip-torque-deceleration-factor", anti_slip_torque_deceleration_factor);

		std::string drivetype;
		if (!c.GetParam("drive", drivetype, error_output)) return false;
		SetDrive(drivetype);

		if (drivetype == "RWD")
		{
			rear_differential.SetFinalDrive(final_drive);
			rear_differential.SetAntiSlip(anti_slip, anti_slip_torque, anti_slip_torque_deceleration_factor);
		}
		else if (drivetype == "FWD")
		{
			front_differential.SetFinalDrive(final_drive);
			front_differential.SetAntiSlip(anti_slip, anti_slip_torque, anti_slip_torque_deceleration_factor);
		}
		else if (drivetype == "AWD")
		{
			rear_differential.SetFinalDrive(1.0);
			rear_differential.SetAntiSlip(anti_slip, anti_slip_torque, anti_slip_torque_deceleration_factor);
			front_differential.SetFinalDrive(1.0);
			front_differential.SetAntiSlip(anti_slip, anti_slip_torque, anti_slip_torque_deceleration_factor);
			center_differential.SetFinalDrive(final_drive);
			center_differential.SetAntiSlip(anti_slip, anti_slip_torque, anti_slip_torque_deceleration_factor);
		}
		else
		{
			error_output << "Unknown drive type: " << drive << std::endl;
			return false;
		}
	}

	//load the brake
	{
		for (int i = 0; i < 2; i++)
		{
			std::string pos = "front";
			WHEEL_POSITION left = FRONT_LEFT;
			WHEEL_POSITION right = FRONT_RIGHT;
			if (i == 1)
			{
				left = REAR_LEFT;
				right = REAR_RIGHT;
				pos = "rear";
			}

			float friction, max_pressure, area, bias, radius, handbrake(0);

			if (!c.GetParam("brakes-"+pos+".friction", friction, error_output)) return false;
			brake[left].SetFriction(friction);
			brake[right].SetFriction(friction);

			if (!c.GetParam("brakes-"+pos+".area", area, error_output)) return false;
			brake[left].SetArea(area);
			brake[right].SetArea(area);

			if (!c.GetParam("brakes-"+pos+".radius", radius, error_output)) return false;
			brake[left].SetRadius(radius);
			brake[right].SetRadius(radius);

			c.GetParam("brakes-"+pos+".handbrake", handbrake);
			brake[left].SetHandbrake(handbrake);
			brake[right].SetHandbrake(handbrake);

			if (!c.GetParam("brakes-"+pos+".bias", bias, error_output)) return false;
			brake[left].SetBias(bias);
			brake[right].SetBias(bias);

			if (!c.GetParam("brakes-"+pos+".max-pressure", max_pressure, error_output)) return false;
			brake[left].SetMaxPressure(max_pressure*bias);
			brake[right].SetMaxPressure(max_pressure*bias);
		}
	}

	//load the fuel tank
	{
		float pos[3];
		MATHVECTOR <double, 3> position;
		float capacity;
		float volume;
		float fuel_density;

		if (!c.GetParam("fuel-tank.capacity", capacity, error_output)) return false;
		fuel_tank.SetCapacity(capacity);

		if (!c.GetParam("fuel-tank.volume", volume, error_output)) return false;
		fuel_tank.SetVolume(volume);

		if (!c.GetParam("fuel-tank.fuel-density", fuel_density, error_output)) return false;
		fuel_tank.SetDensity(fuel_density);

		if (!c.GetParam("fuel-tank.position", pos, error_output)) return false;
		if (version == 2)
			COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(pos[0],pos[1],pos[2]);
		position.Set(pos[0],pos[1],pos[2]);
		fuel_tank.SetPosition(position);
		//AddMassParticle(fuel_density*volume, position);
	}

	//load the suspension
	{
		for (int i = 0; i < 2; i++)
		{
			std::string posstr = "front";
			std::string posshortstr = "F";
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
			MATHVECTOR <double, 3> tempvec;

			if (!c.GetParam("suspension-"+posstr+".spring-constant", spring_constant, error_output)) return false;
			suspension[posl].SetSpringConstant(spring_constant);
			suspension[posr].SetSpringConstant(spring_constant);

			if (!c.GetParam("suspension-"+posstr+".bounce", bounce, error_output)) return false;
			suspension[posl].SetBounce(bounce);
			suspension[posr].SetBounce(bounce);

			if (!c.GetParam("suspension-"+posstr+".rebound", rebound, error_output)) return false;
			suspension[posl].SetRebound(rebound);
			suspension[posr].SetRebound(rebound);

			std::vector <std::pair <double, double> > damper_factor_points;
			c.GetPoints("suspension-"+posstr, "damper-factor", damper_factor_points);
			suspension[posl].SetDamperFactorPoints(damper_factor_points);
			suspension[posr].SetDamperFactorPoints(damper_factor_points);

			std::vector <std::pair <double, double> > spring_factor_points;
			c.GetPoints("suspension-"+posstr, "spring-factor", spring_factor_points);
			suspension[posl].SetSpringFactorPoints(spring_factor_points);
			suspension[posr].SetSpringFactorPoints(spring_factor_points);

			// deprecated
			//if (!c.GetParam("suspension-"+posstr+".max-compression-velocity", maxcompvel, error_output)) return false;
			//suspension[posl].SetMaxCompressionVelocity(maxcompvel);
			//suspension[posr].SetMaxCompressionVelocity(maxcompvel);

			if (!c.GetParam("suspension-"+posstr+".travel", travel, error_output)) return false;
			suspension[posl].SetTravel(travel);
			suspension[posr].SetTravel(travel);

			if (!c.GetParam("suspension-"+posstr+".camber", camber, error_output)) return false;
			suspension[posl].SetCamber(camber);
			suspension[posr].SetCamber(camber);

			if (!c.GetParam("suspension-"+posstr+".caster", caster, error_output)) return false;
			suspension[posl].SetCaster(caster);
			suspension[posr].SetCaster(caster);

			if (!c.GetParam("suspension-"+posstr+".toe", toe, error_output)) return false;
			suspension[posl].SetToe(toe);
			suspension[posr].SetToe(toe);

			if (!c.GetParam("suspension-"+posstr+".anti-roll", anti_roll, error_output)) return false;
			suspension[posl].SetAntiRollK(anti_roll);
			suspension[posr].SetAntiRollK(anti_roll);

			if (!c.GetParam("suspension-"+posshortstr+"L.hinge", hinge, error_output)) return false;
			//cap hinge to reasonable values
			for (int i = 0; i < 3; i++)
			{
				if (hinge[i] < -100)
					hinge[i] = -100;
				if (hinge[i] > 100)
					hinge[i] = 100;
			}
			if (version == 2)
				COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(hinge[0],hinge[1],hinge[2]);
			tempvec.Set(hinge[0],hinge[1], hinge[2]);
			suspension[posl].SetHinge(tempvec);

			if (!c.GetParam("suspension-"+posshortstr+"R.hinge", hinge, error_output)) return false;
			for (int i = 0; i < 3; i++)
			{
				if (hinge[i] < -100)
					hinge[i] = -100;
				if (hinge[i] > 100)
					hinge[i] = 100;
			}
			if (version == 2)
				COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(hinge[0],hinge[1],hinge[2]);
			tempvec.Set(hinge[0],hinge[1], hinge[2]);
			suspension[posr].SetHinge(tempvec);
		}
	}

	//load the wheels
	{
		for (int i = 0; i < 4; i++)
		{
			std::string posstr;
			WHEEL_POSITION pos;
			if (i == 0)
			{
				posstr = "FL";
				pos = FRONT_LEFT;
			}
			else if (i == 1)
			{
				posstr = "FR";
				pos = FRONT_RIGHT;
			}
			else if (i == 2)
			{
				posstr = "RL";
				pos = REAR_LEFT;
			}
			else
			{
				posstr = "RR";
				pos = REAR_RIGHT;
			}

			float roll_height, mass;
			float position[3];
			MATHVECTOR <double, 3> tempvec;

			if (!c.GetParam("wheel-"+posstr+".mass", mass, error_output)) return false;
			wheel[pos].SetMass(mass);

			if (!c.GetParam("wheel-"+posstr+".roll-height", roll_height, error_output)) return false;
			wheel[pos].SetRollHeight(roll_height);

			if (!c.GetParam("wheel-"+posstr+".position", position, error_output)) return false;
			if (version == 2)
				COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(position[0],position[1],position[2]);
			tempvec.Set(position[0],position[1], position[2]);
			wheel[pos].SetExtendedPosition(tempvec);

			AddMassParticle(mass, tempvec);
		}

		//load the rotational inertia parameter from the tire section
		float front_inertia;
		float rear_inertia;
		if (!c.GetParam("tire-front.rotational-inertia", front_inertia, error_output)) return false;
		wheel[FRONT_LEFT].SetInertia(front_inertia);
		wheel[FRONT_RIGHT].SetInertia(front_inertia);

		if (!c.GetParam("tire-rear.rotational-inertia", rear_inertia, error_output)) return false;
		wheel[REAR_LEFT].SetInertia(rear_inertia);
		wheel[REAR_RIGHT].SetInertia(rear_inertia);
	}

	//load the tire parameters
	{
		WHEEL_POSITION leftside = FRONT_LEFT;
		WHEEL_POSITION rightside = FRONT_RIGHT;
		std::string posstr = "front";

		for (int p = 0; p < 2; p++)
		{
			if (p == 1)
			{
				leftside = REAR_LEFT;
				rightside = REAR_RIGHT;
				posstr = "rear";
			}

			std::vector <double> longitudinal;
			std::vector <double> lateral;
			std::vector <double> aligning;
			longitudinal.resize(11);
			lateral.resize(15);
			aligning.resize(18);

			//read lateral
			int numinfile;
			for (int i = 0; i < 15; i++)
			{
				numinfile = i;
				if (i == 11)
					numinfile = 111;
				else if (i == 12)
					numinfile = 112;
				else if (i > 12)
					numinfile -= 1;
				std::stringstream str;
				str << "tire-"+posstr+".a" << numinfile;
				float value;
				if (!c.GetParam(str.str(), value, error_output)) return false;
				lateral[i] = value;
			}

			//read longitudinal, error_output)) return false;
			for (int i = 0; i < 11; i++)
			{
				std::stringstream str;
				str << "tire-"+posstr+".b" << i;
				float value;
				if (!c.GetParam(str.str(), value, error_output)) return false;
				longitudinal[i] = value;
			}

			//read aligning, error_output)) return false;
			for (int i = 0; i < 18; i++)
			{
				std::stringstream str;
				str << "tire-"+posstr+".c" << i;
				float value;
				if (!c.GetParam(str.str(), value, error_output)) return false;
				aligning[i] = value;
			}

			tire[leftside].SetPacejkaParameters(longitudinal, lateral, aligning);
			tire[rightside].SetPacejkaParameters(longitudinal, lateral, aligning);

			float rolling_resistance[3];
			if (!c.GetParam("tire-"+posstr+".rolling-resistance", rolling_resistance, error_output)) return false;
			tire[leftside].SetRollingResistance(rolling_resistance[0], rolling_resistance[1]);
			tire[rightside].SetRollingResistance(rolling_resistance[0], rolling_resistance[1]);

			float tread;
			float radius;
			if (!c.GetParam("tire-"+posstr+".radius", radius, error_output)) return false;
			tire[leftside].SetRadius(radius);
			tire[rightside].SetRadius(radius);
			if (!c.GetParam("tire-"+posstr+".tread", tread, error_output)) return false;
			tire[leftside].SetTread(tread);
			tire[rightside].SetTread(tread);
		}

		
		QTimer tir;  tir.update(); /// time
		for (int i = 0; i < 4; i++)
		{
			tire[WHEEL_POSITION(i)].CalculateSigmaHatAlphaHat();
			//todo: copy, left and right tires are same
			//and make option to have 4 tires the same
			//tires array[] for all cars, asphalt/gravel, not in each .car file
		}
		tir.update(); /// time
		float dt = tir.dt * 1000.f;
		LogO(Ogre::String(":::: Time tires: ") + toStr(dt) + " ms");
	}

	//load the mass-only particles
	{
		MATHVECTOR <double, 3> position;
		float pos[3];
		float mass;

		if (c.GetParam("contact-points.mass", mass))
		{
			int paramnum(0);
			std::string paramname("contact-points.position-00");
			std::stringstream output_supression;
			while (c.GetParam(paramname, pos))
			{
				if (version == 2)
					COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(pos[0],pos[1],pos[2]);
				position.Set(pos[0],pos[1],pos[2]);
				AddMassParticle(mass, position);
				paramnum++;
				std::stringstream str;
				str << "contact-points.position-";
				str.width(2);
				str.fill('0');
				str << paramnum;
				paramname = str.str();
			}
		}

		std::string paramname = "particle-00";
		int paramnum = 0;
		while (c.GetParam(paramname+".mass", mass))
		{
			if (!c.GetParam(paramname+".position", pos, error_output)) return false;
			if (version == 2)
				COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(pos[0],pos[1],pos[2]);
			position.Set(pos[0],pos[1],pos[2]);
			AddMassParticle(mass, position);
			paramnum++;
			std::stringstream str;
			str << "particle-";
			str.width(2);
			str.fill('0');
			str << paramnum;
			paramname = str.str();
		}
	}

	//load the max steering angle
	{
		float maxangle = 45.0;
		if (!c.GetParam("steering.max-angle", maxangle, error_output)) return false;
		SetMaxSteeringAngle ( maxangle );
	}

	//load the driver
	{
		float mass;
		float pos[3];
		MATHVECTOR <double, 3> position;

		if (!c.GetParam("driver.mass", mass, error_output)) return false;
		if (!c.GetParam("driver.position", pos, error_output)) return false;
		if (version == 2)
		{
			COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(pos[0],pos[1],pos[2]);
		}
		position.Set(pos[0], pos[1], pos[2]);
		AddMassParticle(mass, position);
	}

	//load the aerodynamics
	{
		float drag_area, drag_c, lift_area, lift_c, lift_eff;
		float pos[3];
		MATHVECTOR <double, 3> position;

		if (!c.GetParam("drag.frontal-area", drag_area, error_output)) return false;
		if (!c.GetParam("drag.drag-coefficient", drag_c, error_output)) return false;
		if (!c.GetParam("drag.position", pos, error_output)) return false;
		if (version == 2)
			COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(pos[0],pos[1],pos[2]);
		position.Set(pos[0], pos[1], pos[2]);
		AddAerodynamicDevice(position, drag_area, drag_c, 0,0,0);

		for (int i = 0; i < 2; i++)
		{
			std::string wingpos = "front";
			if (i == 1)
				wingpos = "rear";
			if (!c.GetParam("wing-"+wingpos+".frontal-area", drag_area, error_output)) return false;
			if (!c.GetParam("wing-"+wingpos+".drag-coefficient", drag_c, error_output)) return false;
			if (!c.GetParam("wing-"+wingpos+".surface-area", lift_area, error_output)) return false;
			if (!c.GetParam("wing-"+wingpos+".lift-coefficient", lift_c, error_output)) return false;
			if (!c.GetParam("wing-"+wingpos+".efficiency", lift_eff, error_output)) return false;
			if (!c.GetParam("wing-"+wingpos+".position", pos, error_output)) return false;
			if (version == 2)
				COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(pos[0],pos[1],pos[2]);
			position.Set(pos[0], pos[1], pos[2]);
			AddAerodynamicDevice(position, drag_area, drag_c, lift_area, lift_c, lift_eff);
		}
	}

	UpdateMass();

	ti.update(); /// time
	float dt = ti.dt * 1000.f;
	LogO(Ogre::String(":::: Time car dynamics load: ") + toStr(dt) + " ms");
	return true;
}


//----------------------------------------------------------------------------------------------------------------------------------
///  Init  dynamics
//----------------------------------------------------------------------------------------------------------------------------------
void CARDYNAMICS::Init(
	class SETTINGS* pSet1, class Scene* pScene1, class FluidsXml* pFluids1,
	COLLISION_WORLD & world,
	const MODEL & chassisModel,
	const MODEL & wheelModelFront,
	const MODEL & wheelModelRear,
	const MATHVECTOR <T, 3> & position,
	const QUATERNION <T> & orientation)
{
	pSet = pSet1;  pScene = pScene1;  pFluids = pFluids1;
	this->world = &world;

	MATHVECTOR <T, 3> zero(0, 0, 0);
	body.SetPosition(position);
	body.SetOrientation(orientation);
	body.SetInitialForce(zero);
	body.SetInitialTorque(zero);

	// init engine
	engine.SetInitialConditions();


	// init chassis
	btTransform tr;
	tr.setIdentity();

	AABB <float> box = chassisModel.GetAABB();
	for (int i = 0; i < 4; i++)
	{
		MATHVECTOR <float, 3> wheelpos = GetLocalWheelPosition(WHEEL_POSITION(i), 0);

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
	const MATHVECTOR <T, 3> verticalMargin(0, 0, 0.3);
	btVector3 origin = ToBulletVector(box.GetCenter() + verticalMargin - center_of_mass);
	btVector3 size = ToBulletVector(box.GetSize() - verticalMargin);

	//btCompoundShape * chassisShape = new btCompoundShape(false);
	#if 0
		//btBoxShape * hull = new btBoxShape( btVector3(1.8,0.8,0.5) );
		btBoxShape * hull = new btBoxShape( btVector3(1.7,0.7,0.3) );
		tr.setOrigin(origin + btVector3(0,0,0.2));
		chassisShape->addChildShape(tr, hull);
	#else
	/// todo: all spheres? from car.xml / all params..
		// y| length  x- width  z^ height
		btScalar w = size.getX()*0.2, r = size.getZ()*0.3, h = 0.45;

		///  spheres
		btScalar l0 = 0.f, w0 = 0.f, h0 = 0.f;
		if (coll_manual)  // define collision manually
		{
			if (coll_R > 0.f)  r = coll_R;  l0 = coll_Lofs;
			if (coll_W > 0.f)  w = coll_W;  w0 = coll_Wofs;
			if (coll_H > 0.f)  h = coll_H;	h0 = coll_Hofs;
			origin = btVector3(l0, w0, h0);
		}
		//LogO("Car shape dims:  r="+toStr(r)+"  w="+toStr(w)+"  h="+toStr(h)+"  h0="+toStr(h0));
		//LogO("Car offset:  x="+toStr(origin.x())+"  y="+toStr(origin.y())+"  z="+toStr(origin.z()));

		const int numSph = 14;  int i = 0;
		btScalar rad[numSph];  btVector3 pos[numSph];
		pos[i] = btVector3( 1.8, -w,    -h);    	rad[i] = r*0.6;  ++i;  // front
		pos[i] = btVector3( 1.8,  w,    -h);    	rad[i] = r*0.6;  ++i;
		pos[i] = btVector3( 0.9, -w,    -h);    	rad[i] = r;		 ++i;  // front near
		pos[i] = btVector3( 0.9,  w,    -h);    	rad[i] = r;		 ++i;
		pos[i] = btVector3(-0.9, -w,    -h);    	rad[i] = r;		 ++i;  // rear near
		pos[i] = btVector3(-0.9,  w,    -h);    	rad[i] = r;		 ++i;
		pos[i] = btVector3(-1.9, -w,    -h);    	rad[i] = r*0.6;  ++i;  // rear
		pos[i] = btVector3(-1.9,  w,    -h);    	rad[i] = r*0.6;  ++i;
		pos[i] = btVector3( 0.4, -w*0.8, h*0.2);	rad[i] = r*0.6;  ++i;  // top
		pos[i] = btVector3( 0.4,  w*0.8, h*0.2);	rad[i] = r*0.6;  ++i;
		pos[i] = btVector3(-0.3, -w*0.8, h*0.4);	rad[i] = r*0.6;  ++i;
		pos[i] = btVector3(-0.3,  w*0.8, h*0.4);	rad[i] = r*0.6;  ++i;
		pos[i] = btVector3(-1.1, -w*0.8, h*0.2);	rad[i] = r*0.6;  ++i;  // top rear
		pos[i] = btVector3(-1.1,  w*0.8, h*0.2);	rad[i] = r*0.6;  ++i;
		for (i=0; i < numSph; ++i)
			pos[i] += origin;
		btMultiSphereShape* chassisShape = new btMultiSphereShape(pos, rad, numSph);
		//chassisShape->setMargin(0.2f);
	#endif


	T chassisMass = body.GetMass();// * 0.4;  // Magic multiplier makes collisions better - problem: mud is very different
	MATRIX3 <T> inertia = body.GetInertia();
	btVector3 chassisInertia(inertia[0], inertia[4], inertia[8]);

	btTransform transform;
	transform.setOrigin(ToBulletVector(position));
	transform.setRotation(ToBulletQuaternion(orientation));
	btDefaultMotionState * chassisState = new btDefaultMotionState();
	chassisState->setWorldTransform(transform);

	btRigidBody::btRigidBodyConstructionInfo info(chassisMass, chassisState, chassisShape, chassisInertia);
	info.m_angularDamping = 0.4;  // 0.0!+  0.2-  0.5
	info.m_restitution = 0.0;  //...
	info.m_friction = 0.7;  /// 0.4~ 0.75
	///  chasis^
	chassis = world.AddRigidBody(info, true, pSet->game.collis_cars);
	chassis->setActivationState(DISABLE_DEACTIVATION);
	chassis->setUserPointer(new ShapeData(ST_Car, this, 0));  ///~~
	
	world.AddAction(this);
	

	///  join chassis and wheel triggers
	//________________________________________________________
	//if (pScene->fluids.size() > 0)  // if fluids on scene ..load scene before car?
	{
		for (int w=0; w < 4; ++w)
		{
			WHEEL_POSITION wp = WHEEL_POSITION(w);
			T whR = GetTire(wp).GetRadius() * 1.2;  //bigger
			MATHVECTOR <float, 3> wheelpos = GetWheelPosition(wp, 0);
			wheelpos[2] += whR;

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
