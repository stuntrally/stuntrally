#pragma once

class CAR;

class SUSPENSIONBUMPDETECTION
{
public:
	SUSPENSIONBUMPDETECTION();
	
	void Update(float vel, float displacementpercent, float dt);	
	
	bool JustDisplaced() const
	{
		return (state == DISPLACED && laststate != DISPLACED);
	}
	
	bool JustSettled() const
	{
		return (state == SETTLED && laststate != SETTLED);
	}
	
	float GetTotalBumpSize() const
	{
		return dpend - dpstart;
	}

private:
	friend class CAR;
	enum
	{
		DISPLACING,
		DISPLACED,
		SETTLING,
		SETTLED
	} state, laststate;
	
//const_
	float displacetime; ///< how long the suspension has to be displacing a high velocity, uninterrupted
	float displacevelocitythreshold; ///< the threshold for high velocity
	float settletime; ///< how long the suspension has to be settled, uninterrupted
	float settlevelocitythreshold;

//var
	float displacetimer;
	float settletimer;
	float dpstart, dpend;
};
