#include "pch.h"
#include "crashdetection.h"

#include "assert.h"

CRASHDETECTION::CRASHDETECTION()
: lastvel(0), 
  curmaxdecel(0),
  maxdecel(0),
  deceltrigger(200)
{
	
}

void CRASHDETECTION::Update(float vel, float dt)
{
	maxdecel = 0;
	
	float decel = (lastvel - vel)/dt;
	
	//std::cout << "Decel: " << decel << std::endl;
	
	if (decel > deceltrigger && curmaxdecel == 0)
	{
		//idle, start capturing decel
		curmaxdecel = decel;
	}
	else if (curmaxdecel > 0)
	{
		//currently capturing, check for max
		if (decel > curmaxdecel)
		{
			curmaxdecel = decel;
		}
		else
		{
			maxdecel = curmaxdecel;
			assert(maxdecel > deceltrigger);
			curmaxdecel = 0;
		}
	}
	
	lastvel = vel;
}
