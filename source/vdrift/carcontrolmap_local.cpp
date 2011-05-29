#include "../ogre/Defines.h"
#include "carcontrolmap_local.h"
#include "../ogre/OgreGame.h"
#include "../oisb/OISBSystem.h"
#include "../oisb/OISBAction.h"
#include "../oisb/OISBAnalogAxisAction.h"
#include "../oisb/OISBAnalogAxisState.h"


const std::vector <float> & CARCONTROLMAP_LOCAL::ProcessInput(class App* pApp, int player, float dt)
{
	assert(inputs.size() == CARINPUT::ALL);

	lastinputs = inputs;

	#define action(s) OISB::System::getSingleton().lookupAction("Player" + toStr(player+1) + "/" + s)->isActive()
	#define analogAction(s) static_cast<OISB::AnalogAxisAction*>(OISB::System::getSingleton().lookupAction("Player" + toStr(player+1) + "/" + s))->getAbsoluteValue()/100.0f

	inputs[CARINPUT::THROTTLE] = analogAction("Throttle");
	inputs[CARINPUT::BRAKE] = analogAction("Brake");
	
	// steering
	const float value = analogAction("Steering");
	inputs[CARINPUT::STEER_RIGHT] = value > 0.f ? value : 0.f;
	inputs[CARINPUT::STEER_LEFT]  = value < 0.f ? -value : 0.f;
	
	// shift
	bool grUp = action("ShiftUp");
	inputs[CARINPUT::SHIFT_UP] = grUp && !grUpOld[player];
	grUpOld[player] = grUp;
	bool grDn = action("ShiftDown");
	inputs[CARINPUT::SHIFT_DOWN] = grDn && !grDnOld[player];
	grDnOld[player] = grDn;

	
	inputs[CARINPUT::HANDBRAKE] = analogAction("HandBrake");
	inputs[CARINPUT::BOOST] = analogAction("Boost");
	inputs[CARINPUT::FLIP] = analogAction("Flip");

	return inputs;
}
