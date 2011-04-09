#include "stdafx.h"

#include "cardynamics.h"

#include "configfile.h"
#include "tracksurface.h"
#include "coordinatesystems.h"
#include "collision_world.h"
#include "tobullet.h"
#include "model.h"

typedef CARDYNAMICS::T T;

CARDYNAMICS::CARDYNAMICS() :
	world(NULL), chassis(NULL),
	drive(RWD), tacho_rpm(0),
	autoclutch(true), autoshift(true), autorear(true),
	shifted(true), shift_gear(0),
	last_auto_clutch(1.0), remaining_shift_time(0.0),
	shift_time(0.2),
	abs(false), tcs(false),
	maxangle(45.0),
	bTerrain(false), pApp(0)
{
	for (int i=0; i<4; ++i)
	{	bWhOnRoad[i]=0;
		terSurf[i]=0;  }

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
	//wheelBody.resize(WHEEL_POSITION_SIZE);
}

CARDYNAMICS::~CARDYNAMICS()
{}

bool CARDYNAMICS::Load(CONFIGFILE & c, std::ostream & error_output)
{
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

			float spring_constant, bounce, rebound, travel, camber, caster, toe, anti_roll, maxcompvel;
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

		for (int i = 0; i < 4; i++)
		{
			tire[WHEEL_POSITION(i)].CalculateSigmaHatAlphaHat();
		}
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

	return true;
}

void CARDYNAMICS::Init(class App* pApp1,
	COLLISION_WORLD & world,
	const MODEL & chassisModel,
	const MODEL & wheelModelFront,
	const MODEL & wheelModelRear,
	const MATHVECTOR <T, 3> & position,
	const QUATERNION <T> & orientation)
{
	pApp = pApp1;
	this->world = &world;

	MATHVECTOR <T, 3> zero(0, 0, 0);
	body.SetPosition(position);
	body.SetOrientation(orientation);
	body.SetInitialForce(zero);
	body.SetInitialTorque(zero);

	// init engine
	engine.SetInitialConditions();

#if 1
	// init chassis
	btTransform tr;
	tr.setIdentity();
/*
	// convex hull collision shape (one order of magnitude framerate drop)
	localtransform.setOrigin(ToBulletVector(-center_of_mass));
	const float * vertices = NULL;
	int vertices_size = 0;
	chassisModel.GetVertexArray().GetVertices(vertices, vertices_size);
	btConvexHullShape * hull = new btConvexHullShape(vertices, vertices_size / 3, 3 * sizeof(float));
*/
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

	const MATHVECTOR <T, 3> verticalMargin(0, 0, 0.3);
	btVector3 origin = ToBulletVector(box.GetCenter() + verticalMargin - center_of_mass);
	btVector3 size = ToBulletVector(box.GetSize() - verticalMargin);

	//btCompoundShape * chassisShape = new btCompoundShape(false);
	///  chassis shapes
	#if 0
		//btBoxShape * hull = new btBoxShape( btVector3(1.8,0.8,0.5) );
		btBoxShape * hull = new btBoxShape( btVector3(1.7,0.7,0.3) );
		tr.setOrigin(origin + btVector3(0,0,0.2));
		chassisShape->addChildShape(tr, hull);
	#else
	///  from car.xml ....
		// y| x- z^h
		//btScalar h = size.getX()*0.3, r = size.getZ()*0.4, zh = 0.3;
		btScalar h = size.getX()*0.4, r = size.getZ()*0.3, zh = 0.45;  //0.4 CT  0.5 3S -r*0.2;  h 1.33  r 0.4

		///  spheres
		btScalar h0 = 0.f;
		if (coll_manual)  // define collision manually
		{
			r = coll_R;  h0 = coll_Hofs;
		}
		const int numSph = 14;  h *= 0.5f;  int i = 0;
		btScalar rad[numSph];  btVector3 pos[numSph];
		pos[i] = origin + btVector3( 1.8,-h,h0 + -zh);  rad[i] = r*0.6;  ++i;  // front
		pos[i] = origin + btVector3( 1.8, h,h0 + -zh);  rad[i] = r*0.6;  ++i;
		pos[i] = origin + btVector3( 0.9,-h,h0 + -zh);  rad[i] = r;  ++i;
		pos[i] = origin + btVector3( 0.9, h,h0 + -zh);  rad[i] = r;  ++i;
		//pos[i] = origin + btVector3( 0.0,-h,-zh);  rad[i] = r;  ++i;  // center
		//pos[i] = origin + btVector3( 0.0, h,-zh);  rad[i] = r;  ++i;
		pos[i] = origin + btVector3(-0.9,-h,h0 + -zh);  rad[i] = r;  ++i;
		pos[i] = origin + btVector3(-0.9, h,h0 + -zh);  rad[i] = r;  ++i;
		pos[i] = origin + btVector3(-1.9,-h,h0 + -zh);  rad[i] = r*0.6;  ++i;  // rear
		pos[i] = origin + btVector3(-1.9, h,h0 + -zh);  rad[i] = r*0.6;  ++i;
		pos[i] = origin + btVector3( 0.4,-h*0.8,h0 + zh*0.2);  rad[i] = r*0.6;  ++i;  // top
		pos[i] = origin + btVector3( 0.4, h*0.8,h0 + zh*0.2);  rad[i] = r*0.6;  ++i;
		pos[i] = origin + btVector3(-0.3,-h*0.8,h0 + zh*0.4);  rad[i] = r*0.6;  ++i;
		pos[i] = origin + btVector3(-0.3, h*0.8,h0 + zh*0.4);  rad[i] = r*0.6;  ++i;
		pos[i] = origin + btVector3(-1.1,-h*0.8,h0 + zh*0.2);  rad[i] = r*0.6;  ++i;
		pos[i] = origin + btVector3(-1.1, h*0.8,h0 + zh*0.2);  rad[i] = r*0.6;  ++i;/**/
		btMultiSphereShape* chassisShape = new btMultiSphereShape(pos, rad, numSph/**/);

	/*  tr.setOrigin(origin + btVector3( 1.8,0,-zh));	btCapsuleShape* c1 = new btCapsuleShape(r*0.6,h);	chassisShape->addChildShape(tr, c1);
		tr.setOrigin(origin + btVector3(-0.9,0,-zh));	btCapsuleShape* c3 = new btCapsuleShape(r,h);	chassisShape->addChildShape(tr, c3);
		tr.setOrigin(origin + btVector3( 0.0,0,-zh));	btCapsuleShape* c5 = new btCapsuleShape(r,h);	chassisShape->addChildShape(tr, c5);
		tr.setOrigin(origin + btVector3( 0.9,0,-zh));	btCapsuleShape* c4 = new btCapsuleShape(r,h);	chassisShape->addChildShape(tr, c4);
		tr.setOrigin(origin + btVector3(-1.9,0,-zh));	btCapsuleShape* c2 = new btCapsuleShape(r*0.6,h);	chassisShape->addChildShape(tr, c2);
		// top
		tr.setOrigin(origin + btVector3( 0.4,0,zh*0.2));	btCapsuleShape* c6 = new btCapsuleShape(r*0.6,h*0.8);	chassisShape->addChildShape(tr, c6);
		tr.setOrigin(origin + btVector3(-0.3,0,zh*0.4));	btCapsuleShape* c7 = new btCapsuleShape(r*0.6,h*0.8);	chassisShape->addChildShape(tr, c7);
		tr.setOrigin(origin + btVector3(-1.1,0,zh*0.2));	btCapsuleShape* c8 = new btCapsuleShape(r*0.6,h*0.8);	chassisShape->addChildShape(tr, c8);*/
	#endif


	T chassisMass = body.GetMass();
	MATRIX3 <T> inertia = body.GetInertia();
	btVector3 chassisInertia(inertia[0], inertia[4], inertia[8]);

	btTransform transform;
	transform.setOrigin(ToBulletVector(position));
	transform.setRotation(ToBulletQuaternion(orientation));
	btDefaultMotionState * chassisState = new btDefaultMotionState();
	chassisState->setWorldTransform(transform);

	btRigidBody::btRigidBodyConstructionInfo info(chassisMass, chassisState, chassisShape, chassisInertia);
	info.m_angularDamping = 0.4;  // 0.2-  0.5
	info.m_restitution = 0.0;  //...
	info.m_friction = 0.7;  /// 0.4~ 0.75
	///  chasis^
	chassis = world.AddRigidBody(info);
	chassis->setActivationState(DISABLE_DEACTIVATION);
	world.AddAction(this);
#else
	// init chassis
	T chassisMass = body.GetMass();
	MATRIX3 <T> inertia = body.GetInertia();
	btVector3 chassisInertia(inertia[0], inertia[4], inertia[8]);

	btTransform transform;
	transform.setOrigin(ToBulletVector(position));
	transform.setRotation(ToBulletQuaternion(orientation));
	btDefaultMotionState * chassisState = new btDefaultMotionState();
	chassisState->setWorldTransform(transform);
	
	btVector3 origin, size;
	GetCollisionBox(chassisModel, wheelModelFront, wheelModelRear, origin, size);
	
	btCollisionShape * chassisShape = NULL;
	chassisShape = CreateCollisionShape(origin, size);
	
	// create rigid body
	btRigidBody::btRigidBodyConstructionInfo info(chassisMass, chassisState, chassisShape, chassisInertia);
	info.m_angularDamping = 0.5;
	info.m_friction = 0.5;
	chassis = world.AddRigidBody(info);
	chassis->setContactProcessingThreshold(0); // internal edge workaround(swept sphere shape required)
	world.AddAction(this);
#endif

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

// calculate bounding box from chassis, wheel meshes
void CARDYNAMICS::GetCollisionBox(
	const MODEL & chassisModel,
	const MODEL & wheelModelFront,
	const MODEL & wheelModelRear,
	btVector3 & center,
	btVector3 & size)
{
	AABB <float> box = chassisModel.GetAABB();
	float bottom = box.GetCenter()[2] - box.GetSize()[2] * 0.5;
	for (int i = 0; i < 4; i++)
	{
		MATHVECTOR <float, 3> wheelpos = GetLocalWheelPosition(WHEEL_POSITION(i), 0);

		const MODEL * wheelmodel = &wheelModelFront;
		if (i > 1) wheelmodel = &wheelModelRear;

		float sidefactor = 1.0;
		if (i == 1 || i == 3) sidefactor = -1.0;

		AABB <float> wheelaabb;
		wheelaabb.SetFromCorners(
			wheelpos - wheelmodel->GetAABB().GetSize() * 0.5 * sidefactor,
			wheelpos + wheelmodel->GetAABB().GetSize() * 0.5 * sidefactor);
		box.CombineWith(wheelaabb);
	}
	float bottom_new = box.GetCenter()[2] - box.GetSize()[2] * 0.5;
	const float delta = 0.1;
	MATHVECTOR <T, 3> offset(0, 0, bottom - bottom_new + delta);

	center = ToBulletVector(box.GetCenter() + offset * 0.5 - center_of_mass);
	size = ToBulletVector(box.GetSize() - offset);
}

// create collision shape from bounding box
btCollisionShape * CARDYNAMICS::CreateCollisionShape(const btVector3 & center, const btVector3 & size)
{
/*	// use two capsules to approximate collision box
	assert(size[0] > size[1] && size[1] > size[2]);
	btScalar radius = size[2] * 0.5;
	btScalar height = size[0] - size[2];
	btVector3 sideOffset(0, size[1] * 0.5 - size[2] * 0.5, 0);
	btCollisionShape * hull0 = new btCapsuleShapeX(radius, height);
	btCollisionShape * hull1 = new btCapsuleShapeX(radius, height);
	btVector3 origin0 = center + sideOffset;
	btVector3 origin1 = center - sideOffset;
	
	btCompoundShape * shape = new btCompoundShape(false);
	btTransform localtransform;
	localtransform.setIdentity();
	localtransform.setOrigin(origin0);
	shape->addChildShape(localtransform, hull0);
	localtransform.setOrigin(origin1);
	shape->addChildShape(localtransform, hull1);*/
	
	// use btMultiSphereShape(4 spheres) to approximate bounding box
	btVector3 hsize = 0.5 * size;
	int min = hsize.minAxis();
	int max = hsize.maxAxis();
	btVector3 maxAxis(0, 0, 0);
	maxAxis[max] = 1;
	int numSpheres = 4;
	btScalar radius = hsize[min];
	btScalar radii[4] = {radius, radius, radius, radius};
	btVector3 positions[4];
	btVector3 offset0 = hsize - btVector3(radius, radius, radius);
	btVector3 offset1 = offset0 - 2 * offset0[max] * maxAxis;
	positions[0] = center + offset0;
	positions[1] = center + offset1;
	positions[2] = center - offset0;
	positions[3] = center - offset1;
	btMultiSphereShape * shape = new btMultiSphereShape(positions, radii, numSpheres);
	
	return shape;
}
