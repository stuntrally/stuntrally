#ifndef _CARCONTROLMAP_LOCAL_H
#define _CARCONTROLMAP_LOCAL_H

#include "cardefs.h"


class CARCONTROLMAP_LOCAL
{
public:
	CARCONTROLMAP_LOCAL()
	{
		inputs.resize(CARINPUT::ALL, 0.f);
		lastinputs.resize(CARINPUT::ALL, 0.f);
	}
	
	// query the eventsystem for info, then return the resulting input array
	const std::vector <float> & ProcessInput(class App* pApp, int player, float dt);
	
private:
	std::vector <float> inputs;  // indexed by CARINPUT values
	std::vector <float> lastinputs;
};

#endif
