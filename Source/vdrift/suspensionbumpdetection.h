#ifndef _SUSPENSIONBUMPDETECTION_H
#define _SUSPENSIONBUMPDETECTION_H

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
	
	const float displacetime; ///< how long the suspension has to be displacing a high velocity, uninterrupted
	const float displacevelocitythreshold; ///< the threshold for high velocity
	const float settletime; ///< how long the suspension has to be settled, uninterrupted
	const float settlevelocitythreshold;
	
	float displacetimer;
	float settletimer;
	float dpstart, dpend;
};
#endif // _SUSPENSIONBUMPDETECTION_H
