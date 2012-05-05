#ifndef _CARDYNAMICS_H
#define _CARDYNAMICS_H

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
#include "caraerodynamicdevice.h"
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
	typedef double T;
	class SETTINGS* pSet;
	class Scene* pScene;  // for fluids
	class FluidsXml* pFluids;  // to get fluid params
	std::vector<float> inputsCopy;  // just for dbg info txt
	
	CARDYNAMICS();
	~CARDYNAMICS();
	
	bool Load(CONFIGFILE & c, std::ostream & error_output);

	void Init(
		class SETTINGS* pSet1, class Scene* pScene1, class FluidsXml* pFluids1,
		COLLISION_WORLD & world,
		const MODEL & chassisModel,
		const MODEL & wheelModelFront,
		const MODEL & wheelModelRear,
		const MATHVECTOR <T, 3> & position,
		const QUATERNION <T> & orientation);

// bullet interface
	virtual void updateAction(btCollisionWorld * collisionWorld, btScalar dt);
	virtual void debugDraw(btIDebugDraw * debugDrawer);

// graphics interface, interpolated!
	void Update(), UpdateBuoyancy(); // update interpolated chassis state
	const MATHVECTOR <T, 3> & GetCenterOfMassPosition() const;
	const MATHVECTOR <T, 3> & GetPosition() const;
	const QUATERNION <T> & GetOrientation() const;
	MATHVECTOR <T, 3> GetWheelPosition(WHEEL_POSITION wp) const;
	MATHVECTOR <T, 3> GetWheelPosition(WHEEL_POSITION wp, T displacement_percent) const; // for debugging
	QUATERNION <T> GetWheelOrientation(WHEEL_POSITION wp) const;

	QUATERNION <T> GetUprightOrientation(WHEEL_POSITION wp) const;
	MATHVECTOR <T, 3> GetWheelVelocity(WHEEL_POSITION wp) const;

// collision world interface
	const COLLISION_CONTACT & GetWheelContact(WHEEL_POSITION wp) const;
	COLLISION_CONTACT & GetWheelContact(WHEEL_POSITION wp);

/// set from terrain blendmap
	bool bTerrain;
	int bWhOnRoad[WHEEL_POSITION_SIZE];
	TRACKSURFACE* terSurf[WHEEL_POSITION_SIZE];

// chassis
	float GetMass() const;
	T GetSpeed() const;
	MATHVECTOR <T, 3> GetVelocity() const;
	MATHVECTOR <T, 3> GetAngularVelocity() const;
	MATHVECTOR <T, 3> GetEnginePosition() const;

/// custom collision params
	bool coll_manual;  // define collision manually
	float coll_R, coll_W, coll_H, coll_Hofs, coll_Wofs, coll_Lofs;

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
	T GetSpeedMPS() const;
	T GetTachoRPM() const;

	// driveline state access
	const CARENGINE <T> & GetEngine() const {return engine;}
	const CARCLUTCH <T> & GetClutch() const {return clutch;}
	const CARTRANSMISSION <T> & GetTransmission() const {return transmission;}
	const CARBRAKE <T> & GetBrake(WHEEL_POSITION pos) const {return brake[pos];}
	const CARWHEEL <T> & GetWheel(WHEEL_POSITION pos) const {return wheel[pos];}

// traction control
	void SetABS(const bool newabs);
	bool GetABSEnabled() const;
	bool GetABSActive() const;
	void SetTCS(const bool newtcs);
	bool GetTCSEnabled() const;
	bool GetTCSActive() const;

// cardynamics
	void SetPosition(const MATHVECTOR<T, 3> & pos);

	// move the car along z-axis until it is touching the ground, false on error
	void AlignWithGround();

	// set the steering angle to "value", where 1.0 is maximum right lock and -1.0 is maximum left lock.
	void SetSteering(const T value);
	T steerValue;  // copy from SetSteering
	T GetSteering() const {return steerValue;}

	// get the maximum steering angle in degrees
	T GetMaxSteeringAngle() const;

	const CARTIRE <T> & GetTire(WHEEL_POSITION pos) const {return tire[pos];}
	const CARSUSPENSION <T> & GetSuspension(WHEEL_POSITION pos) const {return suspension[pos];}

	MATHVECTOR <T, 3> GetTotalAero() const;
	
	T GetAerodynamicDownforceCoefficient() const;
	T GetAeordynamicDragCoefficient() const;

	MATHVECTOR< T, 3 > GetLastBodyForce() const;
	
	T GetFeedback() const;

	void UpdateTelemetry(float dt);

	// print debug info to the given ostream.  set p1, p2, etc if debug info part 1, and/or part 2, etc is desired
	void DebugPrint(std::ostream & out, bool p1, bool p2, bool p3, bool p4);

	bool Serialize(joeserialize::Serializer & s);

//protected:
public:
// chassis state
	RIGIDBODY <T> body;
	MATHVECTOR <T, 3> center_of_mass;
	COLLISION_WORLD * world;
	btRigidBody * chassis, * whTrigs;

	///  for buoyancy
	float whH[4];  // wheel submerge 0..1
	int whP[4];  // fluid particles id
	struct Polyhedron* poly;
	float body_mass;  btVector3 body_inertia;

	// interpolated chassis state
	MATHVECTOR <T, 3> chassisPosition;
	MATHVECTOR <T, 3> chassisCenterOfMass;
	QUATERNION <T> chassisRotation;
	
	// manual flip over, rocket boost
	float doFlip, doBoost, boostFuel, boostVal;

	std::list<FluidBox*> inFluids,inFluidsWh[4];  /// list of fluids this car is in (if any)
	Ogre::Vector3 vHitPos,vHitNorm;  // world hit data
	float fHitTime, fParIntens,fParVel, fHitForce,fHitForce2,fHitForce3,fCarScrap,fCarScreech;
	btVector3 velPrev;
	

// driveline state
	CARFUELTANK <T> fuel_tank;
	CARENGINE <T> engine;
	CARCLUTCH <T> clutch;
	CARTRANSMISSION <T> transmission;
	CARDIFFERENTIAL <T> front_differential;
	CARDIFFERENTIAL <T> rear_differential;
	CARDIFFERENTIAL <T> center_differential;
	std::vector <CARBRAKE <T> > brake;
	std::vector <CARWHEEL <T> > wheel;
	
	enum { FWD = 3, RWD = 12, AWD = 15 } drive;
	T driveshaft_rpm;
	T tacho_rpm;

	bool autoclutch, autoshift, autorear;
	bool shifted;
	int shift_gear;
	T last_auto_clutch;
	T remaining_shift_time;
	T shift_time;

// traction control state
	bool abs;
	bool tcs;
	std::vector <int> abs_active;
	std::vector <int> tcs_active;
	
// cardynamics state
	std::vector <MATHVECTOR <T, 3> > wheel_velocity;
	std::vector <MATHVECTOR <T, 3> > wheel_position;
	std::vector <QUATERNION <T> > wheel_orientation;
	std::vector <COLLISION_CONTACT> wheel_contact;
	
	std::vector <CARSUSPENSION <T> > suspension;
	std::vector <CARTIRE <T> > tire;
	std::vector <CARAERO <T> > aerodynamics;

	std::list <std::pair <T, MATHVECTOR <T, 3> > > mass_only_particles;
	
	T maxangle;
	T feedback;
	
	MATHVECTOR <T, 3> lastbodyforce; //< held so external classes can extract it for things such as applying physics to camera mounts
	
	//CARTELEMETRY telemetry;

// chassis, cardynamics
	MATHVECTOR <T, 3> GetDownVector() const;

	// wrappers (to be removed)
	QUATERNION <T> Orientation() const;
	MATHVECTOR <T, 3> Position() const;

	MATHVECTOR <T, 3> LocalToWorld(const MATHVECTOR <T, 3> & local) const;
	MATHVECTOR <T, 3> GetLocalWheelPosition(WHEEL_POSITION wp, T displacement_percent) const;

	QUATERNION <T> GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION wp) const;
	MATHVECTOR <T, 3> GetWheelPositionAtDisplacement(WHEEL_POSITION wp, T displacement_percent) const;
	
	void ApplyForce(const MATHVECTOR <T, 3> & force);
	void ApplyForce(const MATHVECTOR <T, 3> & force, const MATHVECTOR <T, 3> & offset);
	void ApplyTorque(const MATHVECTOR <T, 3> & torque);

	void UpdateWheelVelocity();
	void UpdateWheelTransform();

	// apply engine torque to chassis
	void ApplyEngineTorqueToBody();
	
	// apply aerodynamic forces / torques to chassis
	void ApplyAerodynamicsToBody(T dt);

	// update suspension diplacement, return suspension force
	MATHVECTOR <T, 3> UpdateSuspension(int i, T dt);

	// apply tire friction to body, return friction in world space
	MATHVECTOR <T, 3> ApplyTireForce(int i, const T normal_force, const QUATERNION <T> & wheel_space);

	// apply wheel torque to chassis
	void ApplyWheelTorque(T dt, T drive_torque, int i, MATHVECTOR <T, 3> tire_friction, const QUATERNION <T> & wheel_space);

	// advance chassis(body, suspension, wheels) simulation by dt
	void UpdateBody(T dt, T drive_torque[]);

	// cardynamics
	void Tick(T dt);

	void SynchronizeBody();
	void SynchronizeChassis();

	void UpdateWheelContacts();

	void InterpolateWheelContacts(T dt);

	void UpdateMass();

// driveline
	// update engine, return wheel drive torque
	void UpdateDriveline(T dt, T drive_torque[]);
	
	// apply clutch torque to engine
	void ApplyClutchTorque(T engine_drag, T clutch_speed);

	// calculate wheel drive torque
	void CalculateDriveTorque(T wheel_drive_torque[], T clutch_torque);

	// calculate driveshaft speed given wheel angular velocity
	T CalculateDriveshaftSpeed();

	// calculate throttle, clutch, gear
	void UpdateTransmission(T dt);

	// calculate clutch driveshaft rpm
	T CalculateDriveshaftRPM() const;

	bool WheelDriven(int i) const;
	
	T AutoClutch(T last_clutch, T dt) const;
	
	T ShiftAutoClutch() const;
	T ShiftAutoClutchThrottle(T throttle, T dt);
	
	// calculate next gear based on engine rpm
	int NextGear() const;
	
	// calculate downshift point based on gear, engine rpm
	T DownshiftRPM(int gear) const;

// traction control
	// do traction control system calculations and modify the throttle position if necessary
	void DoTCS(int i, T normal_force);

	// do anti-lock brake system calculations and modify the brake force if necessary
	void DoABS(int i, T normal_force);

// cardynamics initialization
	//Set the maximum steering angle in degrees
	void SetMaxSteeringAngle(T newangle);
	
	void SetDrive(const std::string & newdrive);
	
	void AddMassParticle(T newmass, MATHVECTOR <T, 3> newpos);

	void AddAerodynamicDevice(
		const MATHVECTOR <T, 3> & newpos,
		T drag_frontal_area,
		T drag_coefficient,
		T lift_surface_area,
		T lift_coefficient,
		T lift_efficiency);
		
	char IsBraking() const;
};

#endif
