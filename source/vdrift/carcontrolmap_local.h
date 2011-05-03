#ifndef _CARCONTROLMAP_LOCAL_H
#define _CARCONTROLMAP_LOCAL_H

#include "cardefs.h"


class CARCONTROLMAP_LOCAL
{
private:
	std::vector <float> inputs;  // indexed by CARINPUT values
	std::vector <float> lastinputs;

public:
	CARCONTROLMAP_LOCAL()
	{
		//Reset();
	}
	
	void Reset()
	{
		inputs.resize(CARINPUT::ALL, 0.f);
		lastinputs.resize(CARINPUT::ALL, 0.f);
	}
	
	const std::vector <float> & ProcessInput(class App* pApp, int player, float dt);
};

#endif
