#pragma once
#include "dbl.h"
#include "cardynamics.h"
#include "cardefs.h"
#include "suspensionbump.h"
#include "crashdetection.h"

namespace protocol {  struct CarStatePackage;  }
class Sound;


class CAR
{
public:
	class GAME* pGame;
	class App* pApp;
	class CarModel* pCarM;

	CAR();
	~CAR();

	int numWheels;
	void SetNumWheels(int n);
	
	bool Load(class App* pApp1,
		CONFIGFILE & carconf, const std::string & carname,
		const MATHVECTOR<float,3> & init_pos, const QUATERNION<float> & init_rot,
		COLLISION_WORLD & world,	bool abs, bool tcs,
		bool isRemote, int idCar, bool debugmode);
	
	// will align car relative to track surface, returns false if the car isn't near ground
	void SetPosition(const MATHVECTOR<float,3> & pos, const QUATERNION<float> & rot);
	void SetPosition(const MATHVECTOR<Dbl,3> & pos, const QUATERNION<Dbl> & rot);
	void SetPosition1(const MATHVECTOR<float,3> & pos);
	
	void Update(double dt);


	const MATHVECTOR<float,3> GetWheelPosition(const WHEEL_POSITION wpos) const
	{
		MATHVECTOR<float,3> v;
		v = dynamics.GetWheelPosition(wpos);
		return v;
	}

	float GetTireRadius(const WHEEL_POSITION wpos) const
	{
		return dynamics.GetWheel(wpos).GetRadius();
	}

	COLLISION_CONTACT & GetWheelContact(WHEEL_POSITION wheel_index)
	{
		return dynamics.GetWheelContact(wheel_index);
	}

	bool bRemoteCar;
	void HandleInputs(const std::vector <float> & inputs, float dt);
	

	int GetGear() const
	{	return dynamics.GetTransmission().GetGear();  }
	
    void SetGear(int gear)
	{	dynamics.ShiftGear(gear);  }
	
	float GetClutch() const
	{	return dynamics.GetClutch().GetClutch();  }

	void SetAutoClutch(bool value)
	{	dynamics.SetAutoClutch(value);  }


	void SetAutoShift(bool value){	dynamics.SetAutoShift(value);	}
	void SetAutoRear(bool value){	dynamics.SetAutoRear(value);	}

	void SetABS(const bool active){	dynamics.SetABS(active);	}
	bool GetABSEnabled() const	{	return dynamics.GetABSEnabled();	}
	bool GetABSActive() const	{	return dynamics.GetABSActive();		}

	void SetTCS(const bool active){	dynamics.SetTCS(active);	}
	bool GetTCSEnabled() const	{	return dynamics.GetTCSEnabled();	}
	bool GetTCSActive() const	{	return dynamics.GetTCSActive();		}
	
	/// return the speedometer reading (based on the driveshaft speed) in m/s
	float GetSpeedometer() const
	{
		return dynamics.vtype != V_Car ?
			dynamics.GetVelocity().Magnitude() : dynamics.GetSpeedMPS();
	}

	std::string GetCarType() const	{	return cartype;	}

	float GetLastSteer() const	{	return last_steer;	}
	float GetSpeed() const	{		return dynamics.GetSpeed();	}

	float GetSpeedDir()	{	return dynamics.GetSpeedDir();	}
	
	MATHVECTOR<float,3> GetTotalAero() const
	{	return dynamics.GetTotalAero();	}

	float GetFeedback();

	// returns a float from 0.0 to 1.0 with the amount of tire squealing going on
	float GetTireSquealAmount(WHEEL_POSITION i, float* slide=0, float* s1=0, float* s2=0) const;
	
	void DebugPrint(std::ostream & out, bool p1, bool p2, bool p3, bool p4)
	{
		dynamics.DebugPrint(out, p1, p2, p3, p4);
	}
	
	int GetEngineRPM() const	{	return dynamics.GetTachoRPM();	}
	int GetEngineStallRPM() const{	return dynamics.GetEngine().GetStallRPM();	}

	MATHVECTOR<float,3> GetPosition() const
	{
		MATHVECTOR<float,3> pos;
		pos = dynamics.GetPosition();
		return pos;
	}
	QUATERNION<float> GetOrientation() const
	{
		QUATERNION<float> q;
		q = dynamics.GetOrientation();
		return q;
	}

	float GetAerodynamicDownforceCoefficient() const
	{	return dynamics.GetAerodynamicDownforceCoefficient();	}

	float GetAeordynamicDragCoefficient() const
	{	return dynamics.GetAeordynamicDragCoefficient();	}

	float GetMass() const	{	return dynamics.GetMass();	}

	MATHVECTOR<float,3> GetVelocity() const
	{
		MATHVECTOR<float,3> vel;
		vel = dynamics.GetVelocity();
		return vel;
	}
	MATHVECTOR<float,3> GetAngularVelocity() const
	{
		MATHVECTOR<float,3> vel;
		vel = dynamics.GetAngularVelocity();
		return vel;
	}

	// Networking
	protocol::CarStatePackage GetCarStatePackage() const;
	void UpdateCarState(const protocol::CarStatePackage& state);

	///  new
	int id;  // index of car (same as for carModels)
	bool bResetPos;
	void ResetPos(bool fromStart=true);
	void SavePosAtCheck();
	void SetPosRewind(const MATHVECTOR<float,3>& pos, const QUATERNION<float>& rot, const MATHVECTOR<float,3>& vel, const MATHVECTOR<float,3>& angvel);

public:
	CARDYNAMICS dynamics;

	std::vector<SUSPENSIONBUMPDETECTION> suspbump;
	CRASHDETECTION crashdetection2;


	///  Sounds  ---------------
	struct CARsounds
	{
		Sound* engine;
		std::vector<Sound*> asphalt, grass, gravel, bump;  // tires
		
		std::vector<Sound*> crash;
		std::vector<float> crashtime, bumptime, bumpvol;
		
		Sound* wind, *boost, *scrap,*screech;  // cont.
		Sound* mud, *mud_cont,*water_cont;  // fluids
		std::vector<Sound*> water;
		bool fluidHitOld;  float whMudSpin;  ///new vars, for snd

		CARsounds();
		void SetNumWheels(int n);
		void Destroy();
	} sounds;
	
	//  internal variables that might change during driving (so, they need to be serialized)
	float last_steer;
	float trackPercentCopy;  // copy from CarModel for network

	std::string cartype;
	class SETTINGS* pSet;  // for sound vol

	float mz_nominalmax;  // the nominal maximum Mz force, used to scale force feedback

	void UpdateSounds(float dt);
	bool LoadSounds(const std::string & carpath);
		

	//-------------------------------------------------------------------------------
	void GraphsNewVals(double dt);
	
	
	//  for new game reset  and goto last checkp.
	MATHVECTOR<Dbl,3> posAtStart, posLastCheck;
	QUATERNION<Dbl> rotAtStart, rotLastCheck;
	float dmgLastCheck, sphYawAtStart;

	//  car inputs (new)
	int iCamNext;
	bool bLastChk,bLastChkOld, bRewind,bRewindOld;
	float timeRew;
};
