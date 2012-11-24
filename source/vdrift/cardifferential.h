#ifndef _CARDIFFERENTIAL_H
#define _CARDIFFERENTIAL_H

#include "dbl.h"
#include "joeserialize.h"
#include "macros.h"
#include <iostream>
#include "../ogre/common/Defines.h"


///a differential that supports speed-sensitive limited slip functionality.  epicyclic (torque splitting) operation is also provided.
class CARDIFFERENTIAL
{
friend class joeserialize::Serializer;
private:
	// constants
	Dbl final_drive;	///< the gear ratio of the differential
	Dbl anti_slip;		///< this allows modelling of speed-sensitive limited-slip differentials.  this is the maximum anti_slip torque that will be applied and, for speed-sensitive limited-slip differentials, the anti-slip multiplier that's always applied.
	Dbl anti_slip_torque;	///< this allows modelling of torque sensitive limited-slip differentials.  this is the anti_slip dependence on torque.
	Dbl anti_slip_torque_deceleration_factor;	///< this allows modelling of torque sensitive limited-slip differentials that are 1.5 or 2-way.  set it to 0.0 for 1-way LSD, 1.0 for 2-way LSD, and somewhere in between for 1.5-way LSD.
	Dbl torque_split;	///< this allows modelling of epicyclic differentials.  this value ranges from 0.0 to 1.0 where 0.0 applies all torque to side1
	
	// variables
	///by convention, side1 is left or front, side2 is right or rear.
	Dbl side1_speed, side2_speed;
	Dbl side1_torque, side2_torque;

public:
	CARDIFFERENTIAL();

	void DebugPrint(std::ostream & out);

	void ComputeWheelTorques(Dbl driveshaft_torque);

	
	void SetAntiSlip(Dbl as, Dbl ast, Dbl astdf)
	{
		anti_slip = as;
		anti_slip_torque = ast;
		anti_slip_torque_deceleration_factor = astdf;
	}
	
	Dbl CalculateDriveshaftSpeed(Dbl new_side1_speed, Dbl new_side2_speed)
	{
		side1_speed = new_side1_speed;
		side2_speed = new_side2_speed;
		return final_drive * (side1_speed + side2_speed) * 0.5;
	}
	
	Dbl GetDriveshaftSpeed(Dbl new_side1_speed, Dbl new_side2_speed) const
	{
		return final_drive * (side1_speed + side2_speed) * 0.5;
	}
	
	Dbl clamp(Dbl val, Dbl min, Dbl max) const
	{
		return std::max(std::min(val,max), min);
	}


	const Dbl & GetSide1Torque() const	{	return side1_torque;	}
	const Dbl & GetSide2Torque() const	{	return side2_torque;	}
	const Dbl & GetSide1Speed() const	{	return side1_speed;		}
	const Dbl & GetSide2Speed() const	{	return side2_speed;		}

	void SetFinalDrive(const Dbl& value){	final_drive = value;	}
	Dbl GetFinalDrive() const			{	return final_drive;		}
	
	bool Serialize(joeserialize::Serializer & s);
};

#endif
