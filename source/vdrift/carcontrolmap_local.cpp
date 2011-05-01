#include "stdafx.h"

#include "carcontrolmap_local.h"
#include "../ogre/OgreGame.h"
#include "../oisb/OISBSystem.h"
#include "../oisb/OISBAction.h"
#include "../oisb/OISBAnalogAxisAction.h"


const std::vector <float> & CARCONTROLMAP_LOCAL::ProcessInput(class App* pApp, int player, float dt)
{
	assert(inputs.size() == CARINPUT::ALL);

	// update input
	OISB::System::getSingleton().process(dt);

	
	lastinputs = inputs;
	/// TODO: allow joysticks, gamepad
	#define action(s) OISB::System::getSingleton().lookupAction("Player" + toStr(player+1) + "/" + s)->isActive()
	#define analogAction(s) static_cast<OISB::AnalogAxisAction*>(OISB::System::getSingleton().lookupAction("Player" + toStr(player+1) + "/" + s))->getAbsoluteValue()/100.0f
	inputs[CARINPUT::THROTTLE] = action("Throttle") ? 1.0f : 0.0f;
	inputs[CARINPUT::BRAKE] = action("Brake") ? 1.0f : 0.0f;
	
	// steering
	const float value = analogAction("Steering");
	inputs[CARINPUT::STEER_RIGHT] = value > 0.f ? value : 0.f;
	inputs[CARINPUT::STEER_LEFT]  = value < 0.f ? -value : 0.f;
	
	inputs[CARINPUT::HANDBRAKE] = action("HandBrake") ? 1.0f : 0.0f;
	
	// flip over
	inputs[CARINPUT::FLIPLEFT] = action("FlipLeft");
	inputs[CARINPUT::FLIPRIGHT] = action("FlipRight");
	inputs[CARINPUT::BOOST] = action("Boost");

	return inputs;
}
