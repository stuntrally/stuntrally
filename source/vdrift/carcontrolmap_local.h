#pragma once
#include "cardefs.h"
#include <vector>


class CARCONTROLMAP_LOCAL
{
private:
	std::vector <float> inputs;  // indexed by CARINPUT values
	
	// shift
	bool grUpOld[6], grDnOld[6];

public:
	CARCONTROLMAP_LOCAL()
	{
		for (int i=0; i < 6; ++i)
		{	grUpOld[i] = false;  grDnOld[i] = false;  }
		Reset();
	}
	
	void Reset()
	{
		inputs.resize(CARINPUT::ALL, 0.f);
	}
	
	const std::vector <float> & ProcessInput(const float* channels, int player,
		float carspeed, float sss_effect, float sss_velfactor, bool oneAxisThrBrk=false,
		bool forceBrake=false,  // for race countdown
		bool bPerfTest=false, EPerfTest iPerfTestStage=PT_StartWait);  // perf test
};
