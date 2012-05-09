#ifndef _CAR_H
#define _CAR_H

#include "cardynamics.h"
#include "model_joe03.h"

#include "cardefs.h"
#include "sound.h"
#include "joeserialize.h"
#include "macros.h"
#include "suspensionbumpdetection.h"
#include "crashdetection.h"
#include "enginesoundinfo.h"

class BEZIER;
class PERFORMANCE_TESTING;

namespace protocol {
	struct CarStatePackage;
}

class CAR 
{
friend class PERFORMANCE_TESTING;
friend class joeserialize::Serializer;
public:
	class App* pApp;
	class CarModel* pCarM;
	CAR();
	~CAR();
	
	bool Load(class App* pApp1,
		class SETTINGS* settings,
		CONFIGFILE & carconf,
		const std::string & carpath,
		const std::string & driverpath,
		const std::string & carname,
		const MATHVECTOR <float, 3> & init_pos,
		const QUATERNION <float> & init_rot,
		COLLISION_WORLD & world,
		bool soundenabled,
		const SOUNDINFO & sound_device_info,
		const SOUNDBUFFERLIBRARY & soundbufferlibrary,
		bool defaultabs, bool defaulttcs,
		bool isRemote,
		int idCar,
		bool debugmode,
		std::ostream & info_output,
		std::ostream & error_output);
	
	// will align car relative to track surface, returns false if the car isn't near ground
	void SetPosition(const MATHVECTOR <float, 3> & position);
	
	void Update(double dt);

	void GetSoundList(std::list <SOUNDSOURCE *> & outputlist);
	
	void GetEngineSoundList(std::list <SOUNDSOURCE *> & outputlist);


	const MATHVECTOR <float, 3> GetWheelPosition(const WHEEL_POSITION wpos) const
	{
		MATHVECTOR <float, 3> v;
		v = dynamics.GetWheelPosition(wpos);
		return v;
	}

	float GetTireRadius(const WHEEL_POSITION wpos) const
	{
		return dynamics.GetTire(wpos).GetRadius();
	}

	COLLISION_CONTACT & GetWheelContact(WHEEL_POSITION wheel_index)
	{
		return dynamics.GetWheelContact(wheel_index);
	}

	bool bRemoteCar;
	void HandleInputs(const std::vector <float> & inputs, float dt);
	
	int GetEngineRedline() const
	{
		return dynamics.GetEngine().GetRedline();
	}

	int GetEngineRPMLimit() const
	{
		return dynamics.GetEngine().GetRPMLimit();
	}


	int GetGear() const
	{
		return dynamics.GetTransmission().GetGear();
	}
	
    void SetGear(int gear)
	{
	    dynamics.ShiftGear(gear);
	}
	
	float GetClutch()
	{
		return dynamics.GetClutch().GetClutch();
	}


	void SetAutoClutch(bool value)
	{
		dynamics.SetAutoClutch(value);
	}

	void SetAutoShift(bool value)
	{
		dynamics.SetAutoShift(value);
	}

	void SetAutoRear(bool value)
	{
		dynamics.SetAutoRear(value);
	}


	void SetABS(const bool active)
	{
		dynamics.SetABS(active);
	}

	bool GetABSEnabled() const
	{
		return dynamics.GetABSEnabled();
	}

	bool GetABSActive() const
	{
		return dynamics.GetABSActive();
	}


	void SetTCS(const bool active)
	{
		dynamics.SetTCS(active);
	}

	bool GetTCSEnabled() const
	{
		return dynamics.GetTCSEnabled();
	}

	bool GetTCSActive() const
	{
		return dynamics.GetTCSActive();
	}
	
	/// return the speedometer reading (based on the driveshaft speed) in m/s
	float GetSpeedometer() const
	{
		return dynamics.GetSpeedMPS();
	}

	std::string GetCarType() const
	{
		return cartype;
	}

	void SetSector ( int value )
	{
		sector = value;
	}

	int GetSector() const
	{
		return sector;
	}

	const BEZIER * GetCurPatch(unsigned int wheel) const
	{
		assert (wheel < 4);
		return dynamics.GetWheelContact(WHEEL_POSITION(wheel)).GetPatch();
	}

	float GetLastSteer() const
	{
		return last_steer;
	}

	float GetSpeed()
	{
		return dynamics.GetSpeed();
	}
	
	MATHVECTOR <float, 3> GetTotalAero() const
	{
		return dynamics.GetTotalAero();
	}

	float GetFeedback();

	// returns a float from 0.0 to 1.0 with the amount of tire squealing going on
	float GetTireSquealAmount(WHEEL_POSITION i, float* slide=0, float* s1=0, float* s2=0) const;
	
	void EnableGlass(bool enable);

	void DebugPrint(std::ostream & out, bool p1, bool p2, bool p3, bool p4)
	{
		dynamics.DebugPrint(out, p1, p2, p3, p4);
	}
	
	bool Serialize(joeserialize::Serializer & s);

/// AI interface
	int GetEngineRPM() const
	{
		return dynamics.GetTachoRPM();
	}

	int GetEngineStallRPM() const
	{
		return dynamics.GetEngine().GetStallRPM();
	}

	MATHVECTOR <float, 3> GetCenterOfMassPosition() const
	{
		MATHVECTOR <float,3> pos;
		pos = dynamics.GetCenterOfMassPosition();
		return pos;
	}

	MATHVECTOR <float, 3> GetPosition() const
	{
		MATHVECTOR <float,3> pos;
		pos = dynamics.GetPosition();
		return pos;
	}

	QUATERNION <float> GetOrientation() const
	{
		QUATERNION <float> q;
		q = dynamics.GetOrientation();
		return q;
	}

	float GetAerodynamicDownforceCoefficient() const
	{
		return dynamics.GetAerodynamicDownforceCoefficient();
	}

	float GetAeordynamicDragCoefficient() const
	{
		return dynamics.GetAeordynamicDragCoefficient();
	}

	float GetMass() const
	{
		return dynamics.GetMass();
	}

	MATHVECTOR <float, 3> GetVelocity() const
	{
		MATHVECTOR <float, 3> vel;
		vel = dynamics.GetVelocity();
		return vel;
	}

	MATHVECTOR <float, 3> GetAngularVelocity() const
	{
		MATHVECTOR <float, 3> vel;
		vel = dynamics.GetAngularVelocity();
		return vel;
	}

	float GetTireMaxFx(WHEEL_POSITION tire_index) const
	{
		return dynamics.GetTire(tire_index).GetMaximumFx(GetMass()*0.25*9.81);
	}

	float GetTireMaxFy(WHEEL_POSITION tire_index) const
	{
		return dynamics.GetTire(tire_index).GetMaximumFy(GetMass()*0.25*9.81, 0.0);
	}
	
	float GetTireMaxMz(WHEEL_POSITION tire_index) const
	{
		return dynamics.GetTire(tire_index).GetMaximumMz(GetMass()*0.25*9.81, 0.0);
	}
	
	// optimum steering angle in degrees
	float GetOptimumSteeringAngle() const
	{
		return dynamics.GetTire(FRONT_LEFT).GetOptimumSteeringAngle(GetMass()*0.25*9.81);
	}

	// maximum steering angle in degrees
	float GetMaxSteeringAngle() const
	{
		return dynamics.GetMaxSteeringAngle();
	}

	// Networking
	protocol::CarStatePackage GetCarStatePackage() const;
	void UpdateCarState(const protocol::CarStatePackage& state);

	///  new
	int id;  // index of car (same as for carModels)
	bool bResetPos;
	void ResetPos(bool fromStart=true,int chkId=0);
	void SavePosAtCheck();

//protected:
public:
	CARDYNAMICS dynamics;

	MODEL_JOE03 bodymodel, interiormodel, glassmodel, drivermodel;
	
	MATHVECTOR<float, 3> driver_view_position, hood_view_position;
	
	SUSPENSIONBUMPDETECTION suspbump[4];
	CRASHDETECTION crashdetection,crashdetection2;

	std::map <std::string, SOUNDBUFFER> soundbuffers;
	std::list <std::pair <ENGINESOUNDINFO, SOUNDSOURCE> > enginesounds;

	MODEL_JOE03 wheelmodelfront, floatingmodelfront;
	MODEL_JOE03 wheelmodelrear, floatingmodelrear;

	/// sounds
	SOUNDSOURCE tiresqueal[4], grasssound[4], gravelsound[4], tirebump[4];  // tires
	SOUNDSOURCE crashsound[Ncrashsounds];  float crashsoundtime[Ncrashsounds];
	SOUNDSOURCE roadnoise, boostsnd, crashscrap,crashscreech;  // cont.
	SOUNDSOURCE mudsnd, watersnd[Nwatersounds], mud_cont,water_cont;  // fluids
	bool fluidHitOld;  float whMudSpin;  ///new vars, for snd
	
	// internal variables that might change during driving (so, they need to be serialized)
	float last_steer;
	float trackPercentCopy;  // copy from CarModel for network

	std::string cartype;
	class SETTINGS* pSet;  // for sound vol
	int sector; //the last lap timing sector that the car hit
	const BEZIER * curpatch[4]; //the last bezier patch that each wheel hit
	
	float mz_nominalmax;  // the nominal maximum Mz force, used to scale force feedback

	bool LoadInto(
		const std::string & joefile,
		MODEL_JOE03 & output_model,
  		std::ostream & error_output);
	
	void UpdateSounds(float dt);
	
	bool LoadSounds(
		const std::string & carpath,
		const std::string & carname,
		const SOUNDINFO & sound_device_info,
		const SOUNDBUFFERLIBRARY & soundbufferlibrary,
		std::ostream & info_output,
		std::ostream & error_output);
		

	//-------------------------------------------------------------------------------
	void GraphsNewVals(double dt);
	
	///-- models offset
	float vInteriorOffset[3];
	// exhaust position for boost particles 
	bool manualExhaustPos; // if true, use values below, if false, guess from bounding box
	float exhaustPosition[3]; // position of first exhaust
	bool has2exhausts; // car has 2nd exhaust ; if true, mirror exhaust 1 for position

	
	//  for new game reset  and goto last checkp.
	MATHVECTOR <float, 3> posAtStart, posLastCheck[2];
	QUATERNION <float> rotAtStart, rotLastCheck[2];

	//  car inputs  (new: cam,chk)
	int iCamNext;
	bool bLastChk,bLastChkOld;
};

#endif
