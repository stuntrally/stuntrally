#include "pch.h"
#include "../ogre/common/Defines.h"
#include "carcontrolmap_local.h"
#include "../ogre/OgreGame.h"


/*
OISB::Real analogAction(std::string name, bool full=false)
{
	OISB::AnalogAxisAction* act = static_cast<OISB::AnalogAxisAction*>(
		OISB::System::getSingleton().lookupAction(name));
	if (!act)  return 0.f;

	//  clamp full -1..1 or half 0..1
	OISB::Real val = act->getAbsoluteValue();
	return full ? std::max(-1.f, std::min(1.f, val)) : std::max(0.f, std::min(1.f, val));
}

bool action(const std::string& name)
{
	const OISB::Action* act = OISB::System::getSingleton().lookupAction(name);
	return act ? act->isActive() : false;
}*/


///  Process Input
const std::vector <float> & CARCONTROLMAP_LOCAL::ProcessInput(const float* channels, int player,
	float carspeed, float sss_effect, float sss_velfactor,
	bool forceBrake, bool bPerfTest, EPerfTest iPerfTestStage)
{
	assert(inputs.size() == CARINPUT::ALL);

	//-----------------------------------------------------------------
	if (bPerfTest)  // Perf test, automatic car input
	{
  		//CLUTCH //?-
		inputs[CARINPUT::THROTTLE] = iPerfTestStage == PT_Accel ? 1.f : 0.f;
		inputs[CARINPUT::BRAKE]    = iPerfTestStage == PT_Brake ? 1.f : 0.f;

		inputs[CARINPUT::STEER_RIGHT] = 0.f;
		inputs[CARINPUT::STEER_LEFT]  = 0.f;
		
		inputs[CARINPUT::SHIFT_UP]   = 0.f;
		inputs[CARINPUT::SHIFT_DOWN] = 0.f;
		
		inputs[CARINPUT::HANDBRAKE] = iPerfTestStage == PT_StartWait ? 1.f : 0.f;
		inputs[CARINPUT::BOOST]     = 0.f;
		inputs[CARINPUT::FLIP]      = 0.f;
		
		inputs[CARINPUT::PREV_CAM]	= channels[App::A_PrevCamera];
		inputs[CARINPUT::NEXT_CAM]	= channels[App::A_NextCamera];

		inputs[CARINPUT::LAST_CHK]	= 0.f;
		inputs[CARINPUT::REWIND]	= 0.f;

		return inputs;
	}
	//-----------------------------------------------------------------
	
	//  throttle, brake
	inputs[CARINPUT::THROTTLE] = forceBrake ? 0.f : channels[App::A_Throttle];
	const float val_ = forceBrake ? 0.f : channels[App::A_Brake];
	const float deadzone = 0.0001f;  // sensible deadzone for braking
	inputs[CARINPUT::BRAKE]    = (val_ < deadzone) ? 0.f : val_;

	//  steering
	float val = forceBrake ? 0.f : (channels[App::A_Steering]*2-1);

	//*  speed sensitive steering sss (decrease steer angle range with higher speed)
	if (sss_effect > 0.02f)
	{
		float coeff = 1.0f, carmph = abs(carspeed) * 2.23693629f;
		if (carmph > 1.0f)
		{
			//float ssco = sss_effect;  //*(1.0f-pow(val,2.0f));  //?-
			coeff = (3.f-sss_velfactor) * 450.0f * (1.0f - atan(carmph*20.0f*sss_effect) * 0.6366198f);
		}
		if (coeff > 1.0f)  coeff = 1.0f;

		//LogO("speed coeff: "+fToStr(coeff,2,4));
		//val = val >= 0.f ? powf(val,1.5f) : -powf(-val,1.5f);
		val *= coeff;
	}
	inputs[CARINPUT::STEER_RIGHT] = val > 0.f ?  val : 0.f;
	inputs[CARINPUT::STEER_LEFT]  = val < 0.f ? -val : 0.f;
	
	//  shift
	bool grUp = channels[App::A_ShiftUp], grDn = channels[App::A_ShiftDown];
	inputs[CARINPUT::SHIFT_UP]   = grUp && !grUpOld[player];
	inputs[CARINPUT::SHIFT_DOWN] = grDn && !grDnOld[player];
	grUpOld[player] = grUp;  grDnOld[player] = grDn;
	
	//  other
	inputs[CARINPUT::HANDBRAKE] = forceBrake ? 1.f : channels[App::A_HandBrake];
	inputs[CARINPUT::BOOST]     = forceBrake ? 0.f : channels[App::A_Boost];
	inputs[CARINPUT::FLIP]      = forceBrake ? 0.f : channels[App::A_Flip]*2-1;
	
	//  cam
	inputs[CARINPUT::PREV_CAM]	= channels[App::A_PrevCamera];
	inputs[CARINPUT::NEXT_CAM]	= channels[App::A_NextCamera];
	//  last chk
	inputs[CARINPUT::LAST_CHK]	= forceBrake ? false : channels[App::A_LastChk];
	inputs[CARINPUT::REWIND]	= forceBrake ? false : channels[App::A_Rewind];

	return inputs;
}
