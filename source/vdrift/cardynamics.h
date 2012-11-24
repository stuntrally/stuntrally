#ifndef _CARDYNAMICS_H
#define _CARDYNAMICS_H

#include "dbl.h"
#include "mathvector.h"
#include "quaternion.h"
#include "rigidbody.h"
#include "carengine.h"
#include "carclutch.h"
#include "cartransmission.h"
#include "cardifferential.h"
#include "carfueltank.h"
#include "carsuspension.h"
#include "carwheel.h"
#include "cartire.h"
#include "cardefs.h"
#include "carbrake.h"
#include "caraero.h"
#include "joeserialize.h"
#include "macros.h"
#include "collision_contact.h"
#include "cartelemetry.h"
#include "../btOgre/BtOgreDebug.h"

class MODEL;
class CONFIGFILE;
class COLLISION_WORLD;
class FluidBox;

class CARDYNAMICS : public btActionInterface
{
friend class PERFORMANCE_TESTING;
friend class joeserialize::Serializer;
public:

	class SETTINGS* pSet;
	class Scene* pScene;  // for fluids
	class FluidsXml* pFluids;  // to get fluid params
	std::vector<float> inputsCopy;  // just for dbg info txt
	
	CARDYNAMICS();
	~CARDYNAMICS();
	
	bool Load(class GAME* pGame, CONFIGFILE & c, std::ostream & error_output);

	void Init(
		class SETTINGS* pSet1, class Scene* pScene1, class FluidsXml* pFluids1,
		COLLISION_WORLD & world,
		const MODEL & chassisModel,
		const MODEL & wheelModelFront,
		const MODEL & wheelModelRear,
		const MATHVECTOR <Dbl, 3> & position,
		const QUATERNION <Dbl> & orientation);

// bullet interface
	virtual void updateAction(btCollisionWorld * collisionWorld, btScalar dt);
	virtual void debugDraw(btIDebugDraw * debugDrawer);

// graphics interface, interpolated!
	void Update(), UpdateBuoyancy(); // update interpolated chassis state
	const MATHVECTOR <Dbl, 3> & GetCenterOfMassPosition() const;
	const MATHVECTOR <Dbl, 3> & GetPosition() const;
	const QUATERNION <Dbl> & GetOrientation() const;
	MATHVECTOR <Dbl, 3> GetWheelPosition(WHEEL_POSITION wp) const;
	MATHVECTOR <Dbl, 3> GetWheelPosition(WHEEL_POSITION wp, Dbl displacement_percent) const; // for debugging
	QUATERNION <Dbl> GetWheelOrientation(WHEEL_POSITION wp) const;

	QUATERNION <Dbl> GetUprightOrientation(WHEEL_POSITION wp) const;
	MATHVECTOR <Dbl, 3> GetWheelVelocity(WHEEL_POSITION wp) const;

// collision world interface
	const COLLISION_CONTACT & GetWheelContact(WHEEL_POSITION wp) const;
	COLLISION_CONTACT & GetWheelContact(WHEEL_POSITION wp);

/// set from terrain blendmap
	bool bTerrain;
	int bWhOnRoad[WHEEL_POSITION_SIZE];
	TRACKSURFACE* terSurf[WHEEL_POSITION_SIZE];

// chassis
	float GetMass() const;
	Dbl GetSpeed() const;
	MATHVECTOR <Dbl, 3> GetVelocity() const;
	MATHVECTOR <Dbl, 3> GetAngularVelocity() const;
	MATHVECTOR <Dbl, 3> GetEnginePosition() const;

/// custom collision params
	float coll_R, coll_W, coll_H, coll_Hofs, coll_Wofs, coll_Lofs;
	float coll_posLfront, coll_posLback;

// driveline
	// driveline input
	void StartEngine();
	void ShiftGear(int value);
	void SetThrottle(float value);
	void SetClutch(float value);
	void SetBrake(float value);
	void SetHandBrake(float value);
	void SetAutoClutch(bool value);
	void SetAutoShift(bool value);
	void SetAutoRear(bool value);

	// speedometer/tachometer based on driveshaft rpm
	Dbl GetSpeedMPS() const;
	Dbl GetTachoRPM() const;

	// driveline state access
	const CARENGINE & GetEngine() const {  return engine;  }
	const CARCLUTCH & GetClutch() const {  return clutch;  }
	const CARTRANSMISSION & GetTransmission() const {  return transmission;  }
	const CARBRAKE & GetBrake(WHEEL_POSITION pos) const {  return brake[pos];  }
	const CARWHEEL & GetWheel(WHEEL_POSITION pos) const {  return wheel[pos];  }

// traction control
	void SetABS(const bool newabs);
	bool GetABSEnabled() const;
	bool GetABSActive() const;
	void SetTCS(const bool newtcs);
	bool GetTCSEnabled() const;
	bool GetTCSActive() const;

// cardynamics
	void SetPosition(const MATHVECTOR<Dbl, 3> & pos);

	// move the car along z-axis until it is touching the ground, false on error
	void AlignWithGround();

	// set the steering angle to "value", where 1.0 is maximum right lock and -1.0 is maximum left lock.
	void SetSteering(const Dbl value);
	Dbl steerValue;  // copy from SetSteering
	Dbl GetSteering() const {return steerValue;}

	// get the maximum steering angle in degrees
	Dbl GetMaxSteeringAngle() const;

	const CARTIRE & GetTire(WHEEL_POSITION pos) const {  return tire[pos];  }
	const CARSUSPENSION & GetSuspension(WHEEL_POSITION pos) const {  return suspension[pos];  }

	MATHVECTOR <Dbl, 3> GetTotalAero() const;
	
	Dbl GetAerodynamicDownforceCoefficient() const;
	Dbl GetAeordynamicDragCoefficient() const;

	MATHVECTOR< Dbl, 3 > GetLastBodyForce() const;
	
	Dbl GetFeedback() const;

	void UpdateTelemetry(float dt);

	// print debug info to the given ostream.  set p1, p2, etc if debug info part 1, and/or part 2, etc is desired
	void DebugPrint(std::ostream & out, bool p1, bool p2, bool p3, bool p4);

	bool Serialize(joeserialize::Serializer & s);

//protected:
public:
// chassis state
	RIGIDBODY body;
	MATHVECTOR <Dbl, 3> center_of_mass;
	COLLISION_WORLD * world;
	btRigidBody * chassis, * whTrigs;

	///  for buoyancy
	float whH[4];  // wheel submerge 0..1
	int whP[4];  // fluid particles id
	struct Polyhedron* poly;
	float body_mass;  btVector3 body_inertia;
	//float sumWhTest;  //test dbg out

	// interpolated chassis state
	MATHVECTOR <Dbl, 3> chassisPosition;
	MATHVECTOR <Dbl, 3> chassisCenterOfMass;
	QUATERNION <Dbl> chassisRotation;
	
	// manual flip over, rocket boost
	float doFlip, doBoost, boostFuel, boostVal;

	std::list<FluidBox*> inFluids,inFluidsWh[4];  /// list of fluids this car is in (if any)
	Ogre::Vector3 vHitPos,vHitNorm;  // world hit data
	float fHitTime, fParIntens,fParVel, fHitForce,fHitForce2,fHitForce3,fCarScrap,fCarScreech;
	btVector3 velPrev;
	Dbl time;  // for wind only
	

// driveline state
	CARFUELTANK fuel_tank;
	CARENGINE engine;
	CARCLUTCH clutch;
	CARTRANSMISSION transmission;
	CARDIFFERENTIAL diff_front;
	CARDIFFERENTIAL diff_rear;
	CARDIFFERENTIAL diff_center;
	std::vector <CARBRAKE> brake;
	std::vector <CARWHEEL> wheel;
	
	enum { FWD = 3, RWD = 12, AWD = 15 } drive;
	Dbl driveshaft_rpm;
	Dbl tacho_rpm;

	bool autoclutch, autoshift, autorear;
	bool shifted;
	int shift_gear;
	Dbl last_auto_clutch;
	Dbl remaining_shift_time;
	Dbl shift_time;

// traction control state
	bool abs;
	bool tcs;
	std::vector <int> abs_active;
	std::vector <int> tcs_active;
	
// cardynamics state
	std::vector <MATHVECTOR <Dbl, 3> > wheel_velocity;
	std::vector <MATHVECTOR <Dbl, 3> > wheel_position;
	std::vector <QUATERNION <Dbl> > wheel_orientation;
	std::vector <COLLISION_CONTACT> wheel_contact;
	
	std::vector <CARSUSPENSION> suspension;
	std::vector <CARTIRE> tire;
	std::vector <CARAERO> aerodynamics;

	std::list <std::pair <Dbl, MATHVECTOR <Dbl, 3> > > mass_only_particles;
	
	Dbl maxangle;
	Dbl feedback;
	
	Dbl ang_damp;  Dbl rot_coef[4];  /// new
	
	MATHVECTOR <Dbl, 3> lastbodyforce; //< held so external classes can extract it for things such as applying physics to camera mounts
	
	//CARTELEMETRY telemetry;

// chassis, cardynamics
	MATHVECTOR <Dbl, 3> GetDownVector() const;

	// wrappers (to be removed)
	QUATERNION <Dbl> Orientation() const;
	MATHVECTOR <Dbl, 3> Position() const;

	MATHVECTOR <Dbl, 3> LocalToWorld(const MATHVECTOR <Dbl, 3> & local) const;
	MATHVECTOR <Dbl, 3> GetLocalWheelPosition(WHEEL_POSITION wp, Dbl displacement_percent) const;

	QUATERNION <Dbl> GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION wp) const;
	MATHVECTOR <Dbl, 3> GetWheelPositionAtDisplacement(WHEEL_POSITION wp, Dbl displacement_percent) const;
	
	void ApplyForce(const MATHVECTOR <Dbl, 3> & force);
	void ApplyForce(const MATHVECTOR <Dbl, 3> & force, const MATHVECTOR <Dbl, 3> & offset);
	void ApplyTorque(const MATHVECTOR <Dbl, 3> & torque);

	void UpdateWheelVelocity();
	void UpdateWheelTransform();

	// apply engine torque to chassis
	void ApplyEngineTorqueToBody();
	
	// apply aerodynamic forces / torques to chassis
	void ApplyAerodynamicsToBody(Dbl dt);

	// update suspension diplacement, return suspension force
	MATHVECTOR <Dbl, 3> UpdateSuspension(int i, Dbl dt);

	// apply tire friction to body, return friction in world space
	MATHVECTOR <Dbl, 3> ApplyTireForce(int i, const Dbl normal_force, const QUATERNION <Dbl> & wheel_space);

	// apply wheel torque to chassis
	void ApplyWheelTorque(Dbl dt, Dbl drive_torque, int i, MATHVECTOR <Dbl, 3> tire_friction, const QUATERNION <Dbl> & wheel_space);

	// advance chassis(body, suspension, wheels) simulation by dt
	void UpdateBody(Dbl dt, Dbl drive_torque[]);

	// cardynamics
	void Tick(Dbl dt);

	void SynchronizeBody();
	void SynchronizeChassis();

	void UpdateWheelContacts();

	void InterpolateWheelContacts(Dbl dt);

	void UpdateMass();

// driveline
	// update engine, return wheel drive torque
	void UpdateDriveline(Dbl dt, Dbl drive_torque[]);
	
	// apply clutch torque to engine
	void ApplyClutchTorque(Dbl engine_drag, Dbl clutch_speed);

	// calculate wheel drive torque
	void CalculateDriveTorque(Dbl wheel_drive_torque[], Dbl clutch_torque);

	// calculate driveshaft speed given wheel angular velocity
	Dbl CalculateDriveshaftSpeed();

	// calculate throttle, clutch, gear
	void UpdateTransmission(Dbl dt);

	// calculate clutch driveshaft rpm
	Dbl CalculateDriveshaftRPM() const;

	bool WheelDriven(int i) const;
	
	Dbl AutoClutch(Dbl last_clutch, Dbl dt) const;
	
	Dbl ShiftAutoClutch() const;
	Dbl ShiftAutoClutchThrottle(Dbl throttle, Dbl dt);
	
	// calculate next gear based on engine rpm
	int NextGear() const;
	
	// calculate downshift point based on gear, engine rpm
	Dbl DownshiftRPM(int gear) const;

// traction control
	// do traction control system calculations and modify the throttle position if necessary
	void DoTCS(int i, Dbl normal_force);

	// do anti-lock brake system calculations and modify the brake force if necessary
	void DoABS(int i, Dbl normal_force);

// cardynamics initialization
	//Set the maximum steering angle in degrees
	void SetMaxSteeringAngle(Dbl newangle);

	void SetAngDamp( Dbl newang );
	
	void SetDrive(const std::string & newdrive);
	
	void AddMassParticle(Dbl newmass, MATHVECTOR <Dbl, 3> newpos);

	void AddAerodynamicDevice(
		const MATHVECTOR <Dbl, 3> & newpos,
		Dbl drag_frontal_area,
		Dbl drag_coefficient,
		Dbl lift_surface_area,
		Dbl lift_coefficient,
		Dbl lift_efficiency);
		
	char IsBraking() const;
};

#endif
