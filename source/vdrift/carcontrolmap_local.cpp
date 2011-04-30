#include "stdafx.h"

#include "carcontrolmap_local.h"
#include "../ogre/OgreGame.h"
#include "../oisb/OISBSystem.h"
#include "../oisb/OISBAction.h"
#include "../oisb/OISBAnalogAxisAction.h"


const std::vector <float> & CARCONTROLMAP_LOCAL::ProcessInput(class App* pApp, int player, float dt)
{
	lastinputs = inputs;
		
	//  update input
	OISB::System::getSingleton().process(dt);
	
	
	/// TODO: allow joysticks, gamepad
	#define analogAction(s) static_cast<OISB::AnalogAxisAction*>(OISB::System::getSingleton().lookupAction("Player" + toStr(player+1) + "/" + s))->getAbsoluteValue()/100.f

	inputs[CARINPUT::THROTTLE] = analogAction("Throttle");
	inputs[CARINPUT::BRAKE] = analogAction("Brake");
	
	// steering
	const float value = analogAction("Steering");
	inputs[CARINPUT::STEER_RIGHT] = value > 0.f ? value : 0.f;
	inputs[CARINPUT::STEER_LEFT]  = value < 0.f ? -value : 0.f;
	
	inputs[CARINPUT::HANDBRAKE] = analogAction("HandBrake");
	inputs[CARINPUT::BOOST] = analogAction("Boost");
	// flip over
	inputs[CARINPUT::FLIP] = analogAction("Flip");

	return inputs;
}
