#include "pch.h"
#include "suspensionbump.h"

SUSPENSIONBUMPDETECTION::SUSPENSIONBUMPDETECTION()
: state(SETTLED),
  laststate(SETTLED),
  displacetime(0.01),
  displacevelocitythreshold(0.5),
  settletime(0.01),
  settlevelocitythreshold(0.0),
  displacetimer(0),
  settletimer(0),
  dpstart(0),
  dpend(0)
{
	
}

void SUSPENSIONBUMPDETECTION::Update(float vel, float displacementpercent, float dt)
{
	laststate = state;
	
	//switch states based on velocity
	if (state == SETTLED)
	{
		if (vel >= displacevelocitythreshold)
		{
			state = DISPLACING;
			displacetimer = displacetime;
			dpstart = displacementpercent;
		}
	}
	else if (state == DISPLACING)
	{
		if (vel < displacevelocitythreshold)
		{
			state = SETTLED;
		}
	}
	else if (state == DISPLACED)
	{
		if (vel <= settlevelocitythreshold)
		{
			state = SETTLING;
		}
	}
	else if (state == SETTLING)
	{
		//if (std::abs(vel) > settlevelocitythreshold)
		if (vel > settlevelocitythreshold)
		{
			state = DISPLACED;
		}
	}
	
	//switch states based on time
	if (state == DISPLACING)
	{
		displacetimer -= dt;
		if (displacetimer <= 0)
		{
			state = DISPLACED;
			settletimer = settletime;
		}
	}
	else if (state == SETTLING)
	{
		settletimer -= dt;
		if (settletimer <= 0)
		{
			state = SETTLED;
			dpend = displacementpercent;
		}
	}
}
