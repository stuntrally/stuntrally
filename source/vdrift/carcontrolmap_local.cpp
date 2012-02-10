#include "pch.h"
#include "../ogre/common/Defines.h"
#include "carcontrolmap_local.h"
#include "../ogre/OgreGame.h"
#include "../oisb/OISBSystem.h"
#include "../oisb/OISBAction.h"
#include "../oisb/OISBBinding.h"
#include "../oisb/OISBAnalogAxisAction.h"
#include "../oisb/OISBAnalogAxisState.h"


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
}


///  Process Input
const std::vector <float> & CARCONTROLMAP_LOCAL::ProcessInput(int player, bool forceBrake)
{
	assert(inputs.size() == CARINPUT::ALL);

	lastinputs = inputs;

	if (OISB::System::getSingletonPtr() == NULL) return inputs;
	
	const std::string sPlr = "Player" + toStr(player+1) + "/";

	//  throttle, brake
	bool oneAxis = false;  // when brake not bound
	OISB::AnalogAxisAction* act = static_cast<OISB::AnalogAxisAction*>(
		OISB::System::getSingleton().lookupAction(sPlr+"Brake"));
	if (act)  {
		OISB::Binding* binding = act->mBindings.front();
		if (binding)
			oneAxis = binding->getNumBindables() == 0;  }

if (oneAxis)
{
	const float val = forceBrake ? 0.f : analogAction(sPlr+"Throttle", true);
	inputs[CARINPUT::THROTTLE] = val > 0.f ?  val : 0.f;
	inputs[CARINPUT::BRAKE]    = val < 0.f ? -val : 0.f;
}else{
	inputs[CARINPUT::THROTTLE] = forceBrake ? 0.f : analogAction(sPlr+"Throttle");
	const float val = forceBrake ? 0.f : analogAction(sPlr+"Brake");
	const float deadzone = 0.0001f;  // sensible deadzone for braking
	inputs[CARINPUT::BRAKE]    = (val < deadzone) ? 0.f : val;  
}

	//  steering
	const float val = forceBrake ? 0.f : analogAction(sPlr+"Steering", true);
	inputs[CARINPUT::STEER_RIGHT] = val > 0.f ?  val : 0.f;
	inputs[CARINPUT::STEER_LEFT]  = val < 0.f ? -val : 0.f;
	
	//  shift
	bool grUp = action(sPlr+"ShiftUp"), grDn = action(sPlr+"ShiftDown");
	inputs[CARINPUT::SHIFT_UP]   = grUp && !grUpOld[player];
	inputs[CARINPUT::SHIFT_DOWN] = grDn && !grDnOld[player];
	grUpOld[player] = grUp;  grDnOld[player] = grDn;
	
	//  other
	inputs[CARINPUT::HANDBRAKE] = forceBrake ? 1.f : analogAction(sPlr+"HandBrake");
	inputs[CARINPUT::BOOST]     = forceBrake ? 0.f : analogAction(sPlr+"Boost");
	inputs[CARINPUT::FLIP]      = forceBrake ? 0.f : analogAction(sPlr+"Flip", true);
	
	//  cam
	inputs[CARINPUT::PREV_CAM]	= action(sPlr+"PrevCamera");
	inputs[CARINPUT::NEXT_CAM]	= action(sPlr+"NextCamera");
	//  last chk
	inputs[CARINPUT::LAST_CHK]	= forceBrake ? false : action(sPlr+"LastChk");

	return inputs;
}
