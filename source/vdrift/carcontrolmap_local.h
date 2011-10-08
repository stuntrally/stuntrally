#ifndef _CARCONTROLMAP_LOCAL_H
#define _CARCONTROLMAP_LOCAL_H

#include "cardefs.h"
#include <vector>


class CARCONTROLMAP_LOCAL
{
private:
	std::vector <float> inputs;  // indexed by CARINPUT values
	std::vector <float> lastinputs;
	
	// shift
	bool grUpOld[4], grDnOld[4];

public:
	CARCONTROLMAP_LOCAL()
	{
		for (int i=0; i<4; i++) {grUpOld[i] = false; grDnOld[i] = false;}
		//Reset();
	}
	
	void Reset()
	{
		inputs.resize(CARINPUT::ALL, 0.f);
		lastinputs.resize(CARINPUT::ALL, 0.f);
	}
	
	const std::vector <float> & ProcessInput(int player);
};

#endif
