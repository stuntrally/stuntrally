#pragma once
#include "dbl.h"
#include "mathvector.h"
#include "quaternion.h"
#include "rigidbody.h"

#include "carengine.h"
#include "carclutch.h"
#include "cartransmission.h"
#include "carfueltank.h"
#include "cardifferential.h"
#include "caraero.h"

#include "carsuspension.h"
#include "carwheel.h"
#include "cartire.h"
#include "carbrake.h"

#include "cardefs.h"
#include "collision_contact.h"
#include "../btOgre/BtOgreDebug.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "Buoyancy.h"

class MODEL;  class CONFIGFILE;  class COLLISION_WORLD;  class FluidBox;  class GAME;


class CARDYNAMICS : public btActionInterface
{
public:

	class SETTINGS* pSet;
	class Scene* pScene;  // for fluids
	class FluidsXml* pFluids;  // to get fluid params
	std::vector<float> inputsCopy;  // just for dbg info txt

	int numWheels;  // copy from CAR
	void SetNumWheels(int n);
	
	CARDYNAMICS();
	~CARDYNAMICS();
	
	GAME* pGame;
	bool Load(GAME* game, CONFIGFILE & c);

	void Init(
		class SETTINGS* pSet1, class Scene* pScene1, class FluidsXml* pFluids1,
		COLLISION_WORLD & world,
		const MATHVECTOR<Dbl,3> & position,
		const QUATERNION<Dbl> & orientation);
	void RemoveBlt();

// bullet interface
	virtual void updateAction(btCollisionWorld* collisionWorld, btScalar dt);
	virtual void debugDraw(btIDebugDraw* debugDrawer)	{	}

// graphics interface, interpolated
	void Update(), UpdateBuoyancy(); // update interpolated chassis state
	const MATHVECTOR<Dbl,3> & GetCenterOfMassPosition() const	{	return chassisCenterOfMass;		}
	const MATHVECTOR<Dbl,3> & GetPosition() const	{	return chassisPosition;		}
	const QUATERNION<Dbl> & GetOrientation() const	{	return chassisRotation;		}

	MATHVECTOR<Dbl,3> GetWheelPosition(WHEEL_POSITION wp) const;
	MATHVECTOR<Dbl,3> GetWheelPosition(WHEEL_POSITION wp, Dbl displacement_percent) const; // for debugging
	QUATERNION<Dbl> GetWheelOrientation(WHEEL_POSITION wp) const;

	QUATERNION<Dbl> GetUprightOrientation(WHEEL_POSITION wp) const;
	MATHVECTOR<Dbl,3> GetWheelVelocity(WHEEL_POSITION wp) const		{	return wheel_velocity[wp];	}	// worldspace wheel center position

// collision world interface
	const COLLISION_CONTACT & GetWheelContact(WHEEL_POSITION wp) const	{	return wheel_contact[wp];	}
	COLLISION_CONTACT & GetWheelContact(WHEEL_POSITION wp)				{	return wheel_contact[wp];	}

/// set from terrain blendmap
	std::vector<int> iWhOnRoad, whTerMtr, whRoadMtr;

// chassis
	float GetMass() const;
	Dbl GetSpeed() const;  Dbl GetSpeedDir() const;
	MATHVECTOR<Dbl,3> GetVelocity() const;
	MATHVECTOR<Dbl,3> GetAngularVelocity() const;
	MATHVECTOR<Dbl,3> GetEnginePosition() const;

/// custom collision params
	float coll_R, coll_R2m, coll_W, coll_H,
		coll_Hofs, coll_Wofs, coll_Lofs,
		coll_flTrig_H, com_ofs_H, com_ofs_L;
	float coll_posLfront, coll_posLback, coll_friction;
	float coll_FrWmul, coll_FrHmul, coll_TopWmul,
		coll_TopFr, coll_TopMid, coll_TopBack,
		coll_TopFrHm, coll_TopMidHm, coll_TopBackHm, coll_FrontWm;
// buoyancy params dim
	float buoy_X, buoy_Y, buoy_Z, buoy_Mul;
	
// damage
	// changed in CAR::UpdateSounds from bullet hit info
	float fDamage;  // 0-100 %

// driveline
	// driveline input
	void StartEngine();					void ShiftGear(int value);		float GetThrottle() const;
	void SetThrottle(float value);		void SetClutch(float value);
	void SetBrake(float value);			void SetHandBrake(float value);
	void SetAutoClutch(bool value);		void SetAutoShift(bool value);	void SetAutoRear(bool value);

	// speedometer/tachometer based on driveshaft rpm
	Dbl GetSpeedMPS() const;  Dbl GetTachoRPM() const;

	// driveline state access
	const CARENGINE & GetEngine() const {  return engine;  }
	const CARCLUTCH & GetClutch() const {  return clutch;  }
	const CARTRANSMISSION & GetTransmission() const {  return transmission;  }
	const CARBRAKE & GetBrake(WHEEL_POSITION pos) const {  return brake[pos];  }
	const CARWHEEL & GetWheel(WHEEL_POSITION pos) const {  return wheel[pos];  }

// traction control
	void SetABS(const bool newabs);  bool GetABSEnabled() const;  bool GetABSActive() const;
	void SetTCS(const bool newtcs);  bool GetTCSEnabled() const;  bool GetTCSActive() const;

// cardynamics
	void SetPosition(const MATHVECTOR<Dbl,3> & pos);

	// move the car along z-axis until it is touching the ground, false on error
	void AlignWithGround();

	// set the steering angle to "value", where 1.0 is maximum right lock and -1.0 is maximum left lock.
	void SetSteering(const Dbl value, const float range_mul);
	Dbl steerValue;  // copy from SetSteering
	Dbl GetSteering() const {	return steerValue;	}

	// get the maximum steering angle in degrees
	Dbl GetMaxSteeringAngle() const		{	return maxangle;	}

	/*const*/ CARTIRE* GetTire(WHEEL_POSITION pos) const		  {  return wheel_contact[pos].GetSurface().tire;  }
	const CARSUSPENSION & GetSuspension(WHEEL_POSITION pos) const {  return suspension[pos];  }

	MATHVECTOR<Dbl,3> GetTotalAero() const;
	
	Dbl GetAerodynamicDownforceCoefficient() const;
	Dbl GetAeordynamicDragCoefficient() const;

	Dbl GetFeedback() const	{	return feedback;	}

	// print debug info to the given ostream.  set p1, p2, etc if debug info part 1, and/or part 2, etc is desired
	void DebugPrint(std::ostream & out, bool p1, bool p2, bool p3, bool p4);
	// common tool for reading .car tags
	static void GetWPosStr(int axle, int numWheels, WHEEL_POSITION& wl, WHEEL_POSITION& wr, std::string& pos);

public:
	///  camera bounce
	LINEARFRAME cam_body;
	MATHVECTOR<Dbl,3> cam_force;

	///  buoyancy
	std::vector<float> whH;  // wheel submerge 0..1
	std::vector<int> whP;  // fluid particles id
	std::vector<float> whDmg;  // damage from fluid
	Polyhedron poly;
	float body_mass;  btVector3 body_inertia;

	// interpolated chassis state
	MATHVECTOR<Dbl,3> chassisPosition, chassisCenterOfMass;
	QUATERNION<Dbl> chassisRotation;
	
	// manual flip over, rocket boost
	float doFlip, doBoost, boostFuel,boostFuelStart, boostVal, fBoostFov;

	std::list<FluidBox*> inFluids;  /// list of fluids this car is in (if any)
	std::vector<std::list<FluidBox*> > inFluidsWh;
	
	Ogre::Vector3 vHitPos,vHitNorm;  // world hit data
	Ogre::Vector3 vHitCarN,vHitDmgN;  float fHitDmgA;  // damage factors
	float fHitTime, fParIntens,fParVel, fHitForce,
		fHitForce2,fHitForce3, //dbg info only
		fCarScrap,fCarScreech;
	btVector3 velPrev;
	Dbl time;  // for wind only
	
	///  other vehicles  *  *  *
	VehicleType vtype;

	float sphereYaw;  // dir
	float hov_throttle, hov_roll;

	void SimulateSpaceship(Dbl dt), SimulateSphere(Dbl dt);
	std::string sHov;

	///  -- spaceship sim params (from .car)
	struct HoverPar
	{
		float hAbove, hRayLen;
		float steerForce, steerDamp, steerDampP;
		float engineForce, engineVelDec,engineVelDecR, brakeForce;
		float dampAirRes, dampSide, dampUp, dampDn, dampPmul;
		MATHVECTOR <float,3> alt, alp;  // align torque, pipe
		float pitchTq, rollTq, roll;
		float hov_vz, hov_vsat, hov_dsat;  // hover
		float hov_dampP, hov_damp;
		float hov_fall, hov_riseP, hov_rise;
		void Default();
	} hov;

// bullet to delete  -----------------
	btAlignedObjectArray<btCollisionShape*> shapes;
	btAlignedObjectArray<btActionInterface*> actions;
	btAlignedObjectArray<btTypedConstraint*> constraints;
	btAlignedObjectArray<btRigidBody*> rigids;

// chassis state  -----------------
	RIGIDBODY body;
	MATHVECTOR<Dbl,3> center_of_mass;
	COLLISION_WORLD* world;
	btRigidBody *chassis, *whTrigs;
	
// driveline state  -----------------
	CARFUELTANK fuel_tank;
	CARENGINE engine;
	CARCLUTCH clutch;
	CARTRANSMISSION transmission;
	CARDIFFERENTIAL diff_front, diff_rear, diff_center,
			diff_rear2, diff_rear3, diff_center2, diff_center3;  // 6, 8 wheels
	std::vector <CARBRAKE> brake;
	std::vector <CARWHEEL> wheel;
	
	enum { FWD = 3, RWD = 12, AWD = 15, WD6 = 25 , WD8 = 35 } drive;
	Dbl driveshaft_rpm, tacho_rpm;  float engine_vol_mul;

	bool autoclutch, autoshift, autorear, shifted;
	int shift_gear;
	Dbl last_auto_clutch, rem_shift_time, shift_time;  //remaining

// traction control state
	bool abs, tcs;
	std::vector <int> abs_active, tcs_active;
	
// cardynamics state  -----------------
	std::vector <MATHVECTOR<Dbl,3> > wheel_velocity, wheel_position;
	std::vector <QUATERNION<Dbl> > wheel_orientation;
	std::vector <COLLISION_CONTACT> wheel_contact;
	
	std::vector <CARSUSPENSION> suspension;
	std::vector <CARAERO> aerodynamics;

	std::list <std::pair <Dbl, MATHVECTOR<Dbl,3> > > mass_only_particles;
	
	Dbl feedback, maxangle, flip_mul;
	Dbl ang_damp;  Dbl rot_coef[4];  /// new
	
// chassis, cardynamics
	MATHVECTOR<Dbl,3> GetDownVector() const;

	// wrappers (to be removed)
	QUATERNION<Dbl> Orientation() const;
	MATHVECTOR<Dbl,3> Position() const;

	MATHVECTOR<Dbl,3> LocalToWorld(const MATHVECTOR<Dbl,3> & local) const;
	MATHVECTOR<Dbl,3> GetLocalWheelPosition(WHEEL_POSITION wp, Dbl displacement_percent) const;

	QUATERNION<Dbl> GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION wp) const;
	MATHVECTOR<Dbl,3> GetWheelPositionAtDisplacement(WHEEL_POSITION wp, Dbl displacement_percent) const;
	
	void ApplyForce(const MATHVECTOR<Dbl,3> & force);
	void ApplyForce(const MATHVECTOR<Dbl,3> & force, const MATHVECTOR<Dbl,3> & offset);
	void ApplyTorque(const MATHVECTOR<Dbl,3> & torque);

	void UpdateWheelVelocity();
	void UpdateWheelTransform();

	void ApplyEngineTorqueToBody();		// apply engine torque to chassis
	void ApplyAerodynamicsToBody(Dbl dt);	// apply aerodynamic forces / torques to chassis

	MATHVECTOR<Dbl,3> UpdateSuspension(int i, Dbl dt);	// update suspension diplacement, return suspension force

	// apply tire friction to body, return friction in world space
	MATHVECTOR<Dbl,3> ApplyTireForce(int i, const Dbl normal_force, const QUATERNION<Dbl> & wheel_space);
	// apply wheel torque to chassis
	void ApplyWheelTorque(Dbl dt, Dbl drive_torque, int i, MATHVECTOR<Dbl,3> tire_friction, const QUATERNION<Dbl> & wheel_space);

	void Tick(Dbl dt);  /// update simulation
	void UpdateBody(Dbl dt, Dbl drive_torque[]);	// advance chassis(body, suspension, wheels) simulation by dt

	void UpdateMass();  Dbl fBncMass;
	void SynchronizeBody();
	void SynchronizeChassis();

	void UpdateWheelContacts();
	void InterpolateWheelContacts(Dbl dt);

// driveline
	void UpdateDriveline(Dbl dt, Dbl drive_torque[]);	// update engine, return wheel drive torque
	void ApplyClutchTorque(Dbl engine_drag, Dbl clutch_speed);	// apply clutch torque to engine

	void CalculateDriveTorque(Dbl wheel_drive_torque[], Dbl clutch_torque);		// calculate wheel drive torque
	Dbl CalculateDriveshaftSpeed();		// calculate driveshaft speed given wheel angular velocity
	void UpdateTransmission(Dbl dt);	// calculate throttle, clutch, gear
	Dbl CalculateDriveshaftRPM() const;	// calculate clutch driveshaft rpm
	
	Dbl AutoClutch(Dbl last_clutch, Dbl dt) const;
	Dbl ShiftAutoClutch() const;
	Dbl ShiftAutoClutchThrottle(Dbl throttle, Dbl dt);
	
	int NextGear() const;	// calculate next gear based on engine rpm
	Dbl DownshiftRPM(int gear, float avg_whH=0.f) const;	// calculate downshift point based on gear, engine rpm

// traction control
	void DoTCS(int i, Dbl normal_force);	// do traction control system calculations and modify the throttle position if necessary
	void DoABS(int i, Dbl normal_force);	// do anti-lock brake system calculations and modify the brake force if necessary

// cardynamics initialization
	//Set the maximum steering angle in degrees
	void SetMaxSteeringAngle(Dbl newangle)	{	maxangle = newangle;	}
	void SetAngDamp(Dbl newang)				{	ang_damp = newang;	}
	
	void SetDrive(const std::string & newdrive);
	bool WheelDriven(int i) const  {	return (1 << i) & drive;	}
	
	void AddMassParticle(Dbl newmass, MATHVECTOR<Dbl,3> newpos);

	void AddAerodynamicDevice( const MATHVECTOR<Dbl,3> & newpos,
		Dbl drag_frontal_area, Dbl drag_coefficient,
		Dbl lift_surface_area, Dbl lift_coefficient, Dbl lift_efficiency);
		
	char IsBraking() const;
};
