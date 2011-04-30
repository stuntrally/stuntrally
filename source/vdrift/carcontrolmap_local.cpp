#include "stdafx.h"

#include "carcontrolmap_local.h"
#include "../ogre/OgreGame.h"
#include "../oisb/OISBSystem.h"
#include "../oisb/OISBAction.h"
#include "../oisb/OISBAnalogAxisAction.h"


void CARCONTROLMAP_LOCAL::Load(const std::string & controlfile, std::ostream & info_output, std::ostream & error_output)
{
	controls.clear();
	//info_output << "Loading car controls from: " << controlfile << endl;
	CONFIGFILE controls_config;
	if (!controls_config.Load(controlfile))
	{
		//TODO:  create a default control file
		info_output << "Car controls file " << controlfile << " doesn't exist, creating a default" << std::endl;
	}
	else
	{
		std::list<std::string> sections;
		controls_config.GetSectionList(sections);
		
		for (std::list<std::string>::const_iterator section = sections.begin(); section != sections.end(); ++section)
		{
			std::list<std::string> params;
			controls_config.GetParamList( params, *section );
			std::string type = "";
			std::string name = "";
			if (!controls_config.GetParam( *section + ".type", type )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
			if (!controls_config.GetParam( *section + ".name", name )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
			CARINPUT::CARINPUT carinput = GetInputFromString(name);
			CONTROL newctrl;
			if (carinput != CARINPUT::INVALID)
			{
				if (type == "joy")
				{
					newctrl.type = CONTROL::JOY;
					
					int joy_num = 0;
					std::string joy_type = "";
					if (!controls_config.GetParam( *section + ".joy_index", joy_num )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
					if (!controls_config.GetParam( *section + ".joy_type", joy_type )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
					newctrl.joynum = joy_num;
					if (joy_type == "button")
					{
						newctrl.joytype = CONTROL::JOYBUTTON;
						int joy_btn = 0;
						bool joy_btn_down = false;
						bool joy_btn_once = false;
						if (!controls_config.GetParam( *section + ".joy_button", joy_btn )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
						if (!controls_config.GetParam( *section + ".down", joy_btn_down )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
						if (!controls_config.GetParam( *section + ".once", joy_btn_once )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
						newctrl.joybutton = joy_btn;
						newctrl.joypushdown = joy_btn_down;
						newctrl.onetime = joy_btn_once;
					}
					else if(joy_type == "axis")
					{
						newctrl.joytype = CONTROL::JOYAXIS;
						int joy_axis = 0;
						std::string axis_type = "";
						float deadzone = 0.0;
						float exponent = 0.0;
						float gain = 0.0;
						if (!controls_config.GetParam( *section + ".joy_axis", joy_axis )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
						if (!controls_config.GetParam( *section + ".joy_axis_type", axis_type )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
						if (!controls_config.GetParam( *section + ".deadzone", deadzone )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
						if (!controls_config.GetParam( *section + ".exponent", exponent )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
						if (!controls_config.GetParam( *section + ".gain", gain )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
						newctrl.joyaxis = ( joy_axis );
						if( axis_type == "positive" )
						{
							newctrl.joyaxistype = CONTROL::POSITIVE;
						}
						else if( axis_type == "negative" )
						{
							newctrl.joyaxistype = CONTROL::NEGATIVE;
						}
						else if( axis_type == "both" )
						{
							newctrl.joyaxistype = CONTROL::BOTH;
						}
						else
						{
							error_output << "Error parsing controls, invalid joystick axis type in section " << *section << std::endl;
							continue;
						}
						newctrl.deadzone = ( deadzone );
						newctrl.exponent = ( exponent );
						newctrl.gain = ( gain );
					}
					else
					{
						error_output << "Error parsing controls, invalid joystick type in section " << *section << std::endl;
						continue;
					}
				}
				else if (type == "key")
				{
					newctrl.type = CONTROL::KEY;
					
					//string key = "";
					int keycode = 0;
					bool key_down = false;
					bool key_once = false;
					if (!controls_config.GetParam( *section + ".keycode", keycode ))
					{
						std::string keyname;
						if (!controls_config.GetParam( *section + ".key", keyname )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
						
						if (legacy_keycodes.find(keyname) == legacy_keycodes.end())
						{
							error_output << "Unknown keyname \"" << keyname << "\" parsing controls in section " << *section << std::endl;continue;
						}
						else
							keycode = legacy_keycodes[keyname];
					}
					if (!controls_config.GetParam( *section + ".down", key_down )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
					if (!controls_config.GetParam( *section + ".once", key_once )) {error_output << "Error parsing controls in section " << *section << std::endl;continue;}
					if (keycode < SDLK_LAST && keycode > SDLK_FIRST)
						newctrl.keycode = SDLKey(keycode);
					newctrl.keypushdown = ( key_down );
					newctrl.onetime = ( key_once );
				}
				else if (type == "mouse")
				{
					newctrl.type = CONTROL::MOUSE;
					
					std::string mouse_type = "";
					std::string mouse_direction = "";
					int mouse_btn = 0;
					bool mouse_btn_down = false;
					bool mouse_btn_once = false;
					controls_config.GetParam( *section + ".mouse_type", mouse_type );
					if (mouse_type == "button")
					{
						newctrl.mousetype = CONTROL::MOUSEBUTTON;
						controls_config.GetParam( *section + ".mouse_button", mouse_btn );
						controls_config.GetParam( *section + ".down", mouse_btn_down );
						controls_config.GetParam( *section + ".once", mouse_btn_once );
						newctrl.mbutton = mouse_btn;
						newctrl.mouse_push_down = mouse_btn_down;
						newctrl.onetime = mouse_btn_once;
					}
					else if (mouse_type == "motion")
					{
						newctrl.mousetype = CONTROL::MOUSEMOTION;
						controls_config.GetParam( *section + ".mouse_motion", mouse_direction );
						if (mouse_direction == "left")
							newctrl.mdir = CONTROL::LEFT;
						else if (mouse_direction == "right")
							newctrl.mdir = CONTROL::RIGHT;
						else if (mouse_direction == "up")
							newctrl.mdir = CONTROL::UP;
						else if (mouse_direction == "down")
							newctrl.mdir = CONTROL::DOWN;
						else
							error_output << "Error parsing controls, invalid mouse direction type " << mouse_direction << " in section " << *section << std::endl;
						
						if (!controls_config.GetParam( *section + ".deadzone", newctrl.deadzone )) {newctrl.deadzone=0;}
						if (!controls_config.GetParam( *section + ".exponent", newctrl.exponent )) {newctrl.exponent=1;}
						if (!controls_config.GetParam( *section + ".gain", newctrl.gain )) {newctrl.gain=1;}
					}
					else
					{
						error_output << "Error parsing controls, invalid mouse type " << mouse_type << " in section " << *section << std::endl;
					}
				}
				else
				{
					error_output << "Error parsing controls, invalid control type in section " << *section << std::endl;
					continue;
				}
			
				controls[carinput].push_back(newctrl);
			}
			else
			{
				error_output << "Unknown input type in section " << *section << std::endl;
			}
		}
	}
	
	inputs.resize(CARINPUT::INVALID, 0.0); //this looks weird, but it initialize all inputs and sets them to zero
	lastinputs.resize(CARINPUT::INVALID, 0.0); //this looks weird, but it initialize all inputs and sets them to zero
}

void CARCONTROLMAP_LOCAL::Save(CONFIGFILE & controls_config, std::ostream & info_output, std::ostream & error_output)
{
	int id = 0;
	
	for (std::map <CARINPUT::CARINPUT, std::vector <CONTROL> >::iterator n = controls.begin(); n != controls.end(); ++n)
	{
		for (std::vector <CONTROL>::iterator i = n->second.begin(); i != n->second.end(); ++i)
		{
			std::stringstream ss;
			ss << "control mapping ";
			ss << std::setfill('0') << std::setw(2) << id;
			
			CARINPUT::CARINPUT inputid = n->first;
			std::string ctrl_id = ss.str();
			
			std::string ctrl_name = "INVALID";
			for (std::map <std::string, CARINPUT::CARINPUT>::const_iterator s = carinput_stringmap.begin(); s != carinput_stringmap.end(); ++s)
				if (s->second == inputid)
					ctrl_name = s->first;
			
			controls_config.SetParam( ctrl_id + ".name", ctrl_name );
			
			CONTROL & curctrl = *i;
			
			if (curctrl.type == CONTROL::JOY)
			{
				controls_config.SetParam( ctrl_id + ".type", std::string("joy") );
				controls_config.SetParam( ctrl_id + ".joy_index", curctrl.joynum );
			
				if (curctrl.joytype == CONTROL::JOYAXIS)
				{
					controls_config.SetParam( ctrl_id + ".joy_type", std::string("axis") );
					controls_config.SetParam( ctrl_id + ".joy_axis", curctrl.joyaxis );
					switch (curctrl.joyaxistype) {
						case CONTROL::POSITIVE:
							controls_config.SetParam( ctrl_id + ".joy_axis_type", std::string("positive") );
							break;
						case CONTROL::NEGATIVE:
							controls_config.SetParam( ctrl_id + ".joy_axis_type", std::string("negative") );
							break;
						case CONTROL::BOTH:
							controls_config.SetParam( ctrl_id + ".joy_axis_type", std::string("both") );
							break;
					}
					controls_config.SetParam( ctrl_id + ".deadzone", curctrl.deadzone);
					controls_config.SetParam( ctrl_id + ".exponent", curctrl.exponent);
					controls_config.SetParam( ctrl_id + ".gain", curctrl.gain);
				}
				else if (curctrl.joytype == CONTROL::JOYBUTTON)
				{
					controls_config.SetParam(ctrl_id + ".joy_type", std::string("button"));
					controls_config.SetParam(ctrl_id + ".joy_button", curctrl.joybutton);
					controls_config.SetParam(ctrl_id + ".once", curctrl.onetime);
					controls_config.SetParam(ctrl_id + ".down", curctrl.joypushdown);
				}
			}
			else if (curctrl.type == CONTROL::KEY)
			{
				controls_config.SetParam(ctrl_id + ".type", std::string("key"));
				std::string keyname;
				for (std::map <std::string, int>::iterator k = legacy_keycodes.begin(); k != legacy_keycodes.end(); k++)
					if (k->second == curctrl.keycode)
						keyname = k->first;
				controls_config.SetParam(ctrl_id + ".key", keyname);
				controls_config.SetParam(ctrl_id + ".keycode", curctrl.keycode);
				controls_config.SetParam(ctrl_id + ".once", curctrl.onetime);
				controls_config.SetParam(ctrl_id + ".down", curctrl.keypushdown);
			}
			else if(curctrl.type == CONTROL::MOUSE)
			{
				controls_config.SetParam( ctrl_id + ".type", std::string("mouse") );
				if (curctrl.mousetype == CONTROL::MOUSEBUTTON)
				{
					controls_config.SetParam( ctrl_id + ".mouse_type", std::string("button") );
					controls_config.SetParam( ctrl_id + ".mouse_button", curctrl.mbutton );
					controls_config.SetParam( ctrl_id + ".once", curctrl.onetime );
					controls_config.SetParam( ctrl_id + ".down", curctrl.mouse_push_down );
				}
				else if (curctrl.mousetype == CONTROL::MOUSEMOTION)
				{
					std::string direction = "invalid";
					CONTROL::MOUSEDIRECTION mdir = curctrl.mdir;
					if( mdir == CONTROL::UP )
					{
						direction = "up";
					}
					else if( mdir == CONTROL::DOWN )
					{
						direction = "down";
					}
					else if( mdir == CONTROL::LEFT )
					{
						direction = "left";
					}
					else if( mdir == CONTROL::RIGHT )
					{
						direction = "right";
					}

					controls_config.SetParam( ctrl_id + ".mouse_type", std::string("motion") );
					controls_config.SetParam( ctrl_id + ".mouse_motion", direction );
					
					controls_config.SetParam( ctrl_id + ".deadzone", curctrl.deadzone);
					controls_config.SetParam( ctrl_id + ".exponent", curctrl.exponent);
					controls_config.SetParam( ctrl_id + ".gain", curctrl.gain);
				}
			}
			
			id++;
		}
	}
	
	//controls_config.Write(true, controlfile);
}

const std::vector <float> & CARCONTROLMAP_LOCAL::ProcessInput(class App* pApp, int player,
	const std::string & joytype, EVENTSYSTEM_SDL & eventsystem, float steerpos, float dt, bool joy_200, float carms, float speedsens, int screenw, int screenh, float button_ramp, bool hgateshifter)
{
	assert(inputs.size() == CARINPUT::INVALID); //this looks weird, but it ensures that our inputs vector contains exactly one item per input
	assert(lastinputs.size() == CARINPUT::INVALID); //this looks weird, but it ensures that our inputs vector contains exactly one item per input
		
	// update input
	pApp->mKeyboard->capture();  ///! may cause problems?
	OISB::System::getSingleton().process(dt);  ///! here
	
	for (std::map <CARINPUT::CARINPUT, std::vector <CONTROL> >::iterator n = controls.begin(); n != controls.end(); ++n)
	{
		float newval = 0.0;
		
		for (std::vector <CONTROL>::iterator i = n->second.begin(); i != n->second.end(); ++i)
		{
			bool handled = false;
			float tempval = newval;
			
			if (i->type == CONTROL::JOY)
			{
				//cout << "type joy" << endl;
				
				if (i->joytype == CONTROL::JOYAXIS)
				{
					float val = eventsystem.GetJoyAxis(i->joynum, i->joyaxis);
					if (i->joyaxistype == CONTROL::NEGATIVE)
						val = -val;
					val = ApplyDeadzone(i->deadzone,val);
					val = ApplyGain(i->gain,val);
					
					double absval = val;
					bool neg = false;
					if (val < 0)
					{
						absval = -val;
						neg = true;
					}
					val = ApplyExponent(i->exponent,absval);
					if (neg)
						val = -val;
					
					tempval = val;
					handled = true;
				}
				else if (i->joytype == CONTROL::JOYBUTTON)
				{
					TOGGLE button = eventsystem.GetJoyButton(i->joynum, i->joybutton);
					
					if (i->onetime)
					{
						if (i->joypushdown && button.GetImpulseRising())
							tempval = 1.0;
						else if (!i->joypushdown && button.GetImpulseFalling())
							tempval = 1.0;
						else
							tempval = 0.0;
						handled = true;
					}
					else
					{
						float downval = 1.0;
						float upval = 0.0;
						if (!i->joypushdown)
						{
							downval = 0.0;
							upval = 1.0;
						}
						
						tempval = Ramp(lastinputs[n->first], button.GetState() ? downval : upval, button_ramp, dt);
						handled = true;
					}
				}
			}
			else if (i->type == CONTROL::KEY)
			{
				//cout << "type key" << endl;
				using namespace OIS;
				
				EVENTSYSTEM_SDL::BUTTON_STATE keystate = eventsystem.GetKeyState(SDLKey(i->keycode));
				
				#define action(s) OISB::System::getSingleton().lookupAction("Player" + toStr(player+1) + "/" + s)->isActive()
				static bool grUpOld = false, grDnOld = false;
				if (n->first == CARINPUT::SHIFT_UP)		{
					bool grUp = action("ShiftUp");
					keystate.just_down = grUp && !grUpOld;
					keystate.just_up = !grUp && grUpOld;	grUpOld = grUp;  }
				if (n->first == CARINPUT::SHIFT_DOWN)	{
					bool grDn = action("ShiftDown");
					keystate.just_down = grDn && !grDnOld;
					keystate.just_up = !grDn && grDnOld;	grDnOld = grDn;  }
					
				if (i->onetime)
				{
					if (i->keypushdown && keystate.just_down)
						tempval = 1.0;
					else if (!i->keypushdown && keystate.just_up)
						tempval = 1.0;
					else
						tempval = 0.0;
					handled = true;
				}
				else
				{
					float downval = 1.0;
					float upval = 0.0;
					if (!i->keypushdown)
					{
						downval = 0.0;
						upval = 1.0;
					}
					
					//if (inputs[n->first] != keystate.down ? downval : upval) std::cout << "Key ramp: " << i->keycode << ", " << n->first << std::endl;
					tempval = Ramp(lastinputs[n->first], keystate.down ? downval : upval, button_ramp, dt);
					
					handled = true;
				}
			}
			else if (i->type == CONTROL::MOUSE)
			{
				//cout << "type mouse" << endl;
				
				if (i->mousetype == CONTROL::MOUSEBUTTON)
				{
					//cout << "mousebutton" << endl;
					
					EVENTSYSTEM_SDL::BUTTON_STATE buttonstate = eventsystem.GetMouseButtonState(i->mbutton);
					
					if (i->onetime)
					{
						if (i->mouse_push_down && buttonstate.just_down)
							tempval = 1.0;
						else if (!i->mouse_push_down && buttonstate.just_up)
							tempval = 1.0;
						else
							tempval = 0.0;
						handled = true;
					}
					else
					{
						float downval = 1.0;
						float upval = 0.0;
						if (!i->mouse_push_down)
						{
							downval = 0.0;
							upval = 1.0;
						}
						
						tempval = Ramp(lastinputs[n->first], buttonstate.down ? downval : upval, button_ramp, dt);
						handled = true;
					}
				}
				else if (i->mousetype == CONTROL::MOUSEMOTION)
				{
					//cout << "mousemotion" << endl;
					
					std::vector <int> pos = eventsystem.GetMousePosition();
					//std::cout << pos[0] << "," << pos[1] << endl;
					
					float xval = (pos[0]-screenw/2.0)/(screenw/4.0);
					if (xval < -1) xval = -1;
					if (xval > 1) xval = 1;
					
					float yval = (pos[1]-screenh/2.0)/(screenh/4.0);
					if (yval < -1) yval = -1;
					if (yval > 1) yval = 1;
					
					float val = 0;
					
					if (i->mdir == CONTROL::UP)
						val = -yval;
					else if (i->mdir == CONTROL::DOWN)
						val = yval;
					else if (i->mdir == CONTROL::LEFT)
						val = -xval;
					else if (i->mdir == CONTROL::RIGHT)
						val = xval;
					
					if (val < 0)
						val = 0;
					else if (val > 1)
						val = 1;
					
					val = ApplyDeadzone(i->deadzone,val);
					val = ApplyGain(i->gain,val);
					
					if (val < 0)
						val = 0;
					else if (val > 1)
						val = 1;
					
					double absval = val;
					bool neg = false;
					if (val < 0)
					{
						absval = -val;
						neg = true;
					}
					val = ApplyExponent(i->exponent,absval);
					if (neg)
						val = -val;
					
					if (val < 0)
						val = 0;
					else if (val > 1)
						val = 1;
					
					tempval = val;
					
					//cout << val << endl;
					
					handled = true;
				}
				//else cout << "mouse???" << endl;
			}
			//else cout << "type invalid" << endl;
			
			if (tempval > newval)
				newval = tempval;
			
			assert(handled);
		}

		if (newval < 0)
			newval = 0;
		if (newval > 1.0)
			newval = 1.0;
		
		inputs[n->first] = newval;
		
		//std::cout << "New input value: " << inputs[n->first] << std::endl;
	}
	
	if (hgateshifter)
	{
		bool havegear = inputs[CARINPUT::FIRST_GEAR] ||
				inputs[CARINPUT::SECOND_GEAR] || 
				inputs[CARINPUT::THIRD_GEAR] || 
				inputs[CARINPUT::FOURTH_GEAR] || 
				inputs[CARINPUT::FIFTH_GEAR] || 
				inputs[CARINPUT::SIXTH_GEAR] || 
				inputs[CARINPUT::REVERSE];
		if (!havegear)
			inputs[CARINPUT::NEUTRAL] = 1.0;
	}
	
	lastinputs = inputs;
	
	//do steering processing
	ProcessSteering(joytype, steerpos, dt, joy_200, carms*2.23693629, speedsens);
	
	/// TODO: make steering & throttle analog axis actions and allow joystick
	#define analogAction(s) static_cast<OISB::AnalogAxisAction*>(OISB::System::getSingleton().lookupAction("Player" + toStr(player+1) + "/" + s))->getAbsoluteValue()/100.0f
	inputs[CARINPUT::THROTTLE] = action("Throttle") ? 1.0f : 0.0f;
	inputs[CARINPUT::BRAKE] = action("Brake") ? 1.0f : 0.0f;
	
	// steering
	float steerLeft, steerRight;
	const float value = analogAction("Steering");
	if (value < 0)
	{
		steerLeft = value*-1; steerRight = 0;
	}
	else if (value > 0)
	{
		steerLeft = 0; steerRight = value;
	}
	else
	{
		steerLeft = 0; steerRight = 0;
	}
	inputs[CARINPUT::STEER_RIGHT] = steerRight;
	inputs[CARINPUT::STEER_LEFT] = steerLeft;
	
	inputs[CARINPUT::HANDBRAKE] = action("HandBrake") ? 1.0f : 0.0f;
	
	// flip over
	inputs[CARINPUT::FLIPLEFT] = action("FlipLeft");
	inputs[CARINPUT::FLIPRIGHT] = action("FlipRight");
	inputs[CARINPUT::BOOST] = action("Boost");

	return inputs;
}

float sinesmooth(float val)
{
	return sin((val-1.0f)*3.141593f*0.5f)+1.0f;
}

void CARCONTROLMAP_LOCAL::ProcessSteering(const std::string & joytype, float steerpos, float dt, bool joy_200, float carmph, float speedsens)
{
	//std::cout << "steerpos: " << steerpos << std::endl;
	
	float val = inputs[CARINPUT::STEER_RIGHT];
	if (std::abs(inputs[CARINPUT::STEER_LEFT]) > std::abs(inputs[CARINPUT::STEER_RIGHT])) //use whichever control is larger
		val = -inputs[CARINPUT::STEER_LEFT];
	
	/*if (val != 0)
	{
		std::cout << "Initial steer left: " << inputs[CARINPUT::STEER_LEFT] << std::endl;
		std::cout << "Initial steer right: " << inputs[CARINPUT::STEER_RIGHT] << std::endl;
		std::cout << "Initial val: " << val << std::endl;
	}*/
	
	//std::cout << "val: " << val << std::endl;
	
	//restrict joystick range if required
	if (joy_200)
	{
		float decimate = 4.5;

		float normalat = 30;
		float transat = 15;

		if (carmph < transat)
			decimate = 1.0;
		else if (carmph < normalat)
		{
			float coeff = (carmph - transat)/(normalat - transat);
			decimate = (decimate-1.0f)*coeff + 1.0f;
		}

		//std::cout << "Decimating: " << val << " to " << val / decimate << std::endl;
		
		val = val/decimate;
	}
	
	//do speed sensitivity
	if( speedsens != 0.0 )
	{
		float ratio = 20.0f;
		float coeff = 1.0;
		float ssco = speedsens*(1.0f-pow(val,2.0f));
		
		if (carmph > 1)
			coeff = ratio*45.0f*(1.0f-atan(carmph*80.0f*ssco)*0.6366198);

		if (coeff > 1.0f)
			coeff = 1.0f;
		
		//std::cout << "Speed sensitivity coefficient: " << coeff << std::endl;
		
		val = val*coeff;
	}
	
	//std::cout << "After speed sensitivity: " << val << std::endl;
	
	//rate-limit non-wheel controls
	if (joytype != "wheel")
	{
		//if (i->first == inputs[CARINPUT::STEER_LEFT])
		//steerpos = -steerpos;
		float steerstep = 5.0*dt;
	
		if (val > steerpos)
		{
			if (val - steerpos <= steerstep)
				steerpos = val;
			else
				steerpos += steerstep;
		}
		else
		{
			if (steerpos - val <= steerstep)
				steerpos = val;
			else
				steerpos -= steerstep;
		}
		
		if (steerpos > 1.0)
			steerpos = 1.0;
		if (steerpos < -1.0)
			steerpos = -1.0;
		
		val = steerpos;
		
		/*float coeff = 0.97;
		val = steerpos * coeff + val * (1.0-coeff);*/
	}
	
	//std::cout << "After rate limit val: " << val << std::endl;
	
	//std::cout << "Steer left: " << inputs[CARINPUT::STEER_LEFT] << std::endl;
	//std::cout << "Steer right: " << inputs[CARINPUT::STEER_RIGHT] << std::endl;
	//std::cout << "Current steering: " << car.GetLastSteer() << std::endl;
	//std::cout << "New steering: " << val << std::endl;
	
	inputs[CARINPUT::STEER_LEFT] = 0.0;
	inputs[CARINPUT::STEER_RIGHT] = 0.0;
	if (val < 0)
		inputs[CARINPUT::STEER_LEFT] = -val;
	else
		inputs[CARINPUT::STEER_RIGHT] = val;
	//inputs[CARINPUT::STEER_RIGHT] = val;
	//inputs[CARINPUT::STEER_LEFT] = -val;
	
	/*if (val != 0)
	{
		std::cout << "Steer left: " << inputs[CARINPUT::STEER_LEFT] << std::endl;
		std::cout << "Steer right: " << inputs[CARINPUT::STEER_RIGHT] << std::endl;
	}*/
}

void CARCONTROLMAP_LOCAL::PopulateLegacyKeycodes()
{
	legacy_keycodes["F1"] = 282;
	legacy_keycodes["F2"] = 283;
	legacy_keycodes["F3"] = 284;
	legacy_keycodes["F4"] = 285;
	legacy_keycodes["F5"] = 286;
	legacy_keycodes["F6"] = 287;
	legacy_keycodes["F7"] = 288;
	legacy_keycodes["F8"] = 289;
	legacy_keycodes["F9"] = 290;
	legacy_keycodes["F10"] = 291;
	legacy_keycodes["F11"] = 292;
	legacy_keycodes["F12"] = 293;
	legacy_keycodes["F13"] = 294;
	legacy_keycodes["F14"] = 295;
	legacy_keycodes["F15"] = 296;
	legacy_keycodes["BACKSPACE"] = 8;
	legacy_keycodes["TAB"] = 9;
	legacy_keycodes["CLEAR"] = 12;
	legacy_keycodes["RETURN"] = 13;
	legacy_keycodes["PAUSE"] = 19;
	legacy_keycodes["ESCAPE"] = 27;
	legacy_keycodes["SPACE"] = 32;
	legacy_keycodes["EXCLAIM"] = 33;
	legacy_keycodes["QUOTEDBL"] = 34;
	legacy_keycodes["HASH"] = 35;
	legacy_keycodes["DOLLAR"] = 36;
	legacy_keycodes["AMPERSAND"] = 38;
	legacy_keycodes["QUOTE"] = 39;
	legacy_keycodes["LEFTPAREN"] = 40;
	legacy_keycodes["RIGHTPAREN"] = 41;
	legacy_keycodes["ASTERISK"] = 42;
	legacy_keycodes["PLUS"] = 43;
	legacy_keycodes["COMMA"] = 44;
	legacy_keycodes["MINUS"] = 45;
	legacy_keycodes["PERIOD"] = 46;
	legacy_keycodes["SLASH"] = 47;
	legacy_keycodes["0"] = 48;
	legacy_keycodes["1"] = 49;
	legacy_keycodes["2"] = 50;
	legacy_keycodes["3"] = 51;
	legacy_keycodes["4"] = 52;
	legacy_keycodes["5"] = 53;
	legacy_keycodes["6"] = 54;
	legacy_keycodes["7"] = 55;
	legacy_keycodes["8"] = 56;
	legacy_keycodes["9"] = 57;
	legacy_keycodes["COLON"] = 58;
	legacy_keycodes["SEMICOLON"] = 59;
	legacy_keycodes["LESS"] = 60;
	legacy_keycodes["EQUALS"] = 61;
	legacy_keycodes["GREATER"] = 62;
	legacy_keycodes["QUESTION"] = 63;
	legacy_keycodes["AT"] = 64;
	legacy_keycodes["LEFTBRACKET"] = 91;
	legacy_keycodes["BACKSLASH"] = 92;
	legacy_keycodes["RIGHTBRACKET"] = 93;
	legacy_keycodes["CARET"] = 94;
	legacy_keycodes["UNDERSCORE"] = 95;
	legacy_keycodes["BACKQUOTE"] = 96;
	legacy_keycodes["a"] = 97;
	legacy_keycodes["b"] = 98;
	legacy_keycodes["c"] = 99;
	legacy_keycodes["d"] = 100;
	legacy_keycodes["e"] = 101;
	legacy_keycodes["f"] = 102;
	legacy_keycodes["g"] = 103;
	legacy_keycodes["h"] = 104;
	legacy_keycodes["i"] = 105;
	legacy_keycodes["j"] = 106;
	legacy_keycodes["k"] = 107;
	legacy_keycodes["l"] = 108;
	legacy_keycodes["m"] = 109;
	legacy_keycodes["n"] = 110;
	legacy_keycodes["o"] = 111;
	legacy_keycodes["p"] = 112;
	legacy_keycodes["q"] = 113;
	legacy_keycodes["r"] = 114;
	legacy_keycodes["s"] = 115;
	legacy_keycodes["t"] = 116;
	legacy_keycodes["u"] = 117;
	legacy_keycodes["v"] = 118;
	legacy_keycodes["w"] = 119;
	legacy_keycodes["x"] = 120;
	legacy_keycodes["y"] = 121;
	legacy_keycodes["z"] = 122;
	legacy_keycodes["DELETE"] = 127;
	legacy_keycodes["KP0"] = 256;
	legacy_keycodes["KP1"] = 257;
	legacy_keycodes["KP2"] = 258;
	legacy_keycodes["KP3"] = 259;
	legacy_keycodes["KP4"] = 260;
	legacy_keycodes["KP5"] = 261;
	legacy_keycodes["KP6"] = 262;
	legacy_keycodes["KP7"] = 263;
	legacy_keycodes["KP8"] = 264;
	legacy_keycodes["KP9"] = 265;
	legacy_keycodes["KP_PERIOD"] = 266;
	legacy_keycodes["KP_DIVIDE"] = 267;
	legacy_keycodes["KP_MULTIPLY"] = 268;
	legacy_keycodes["KP_MINUS"] = 269;
	legacy_keycodes["KP_PLUS"] = 270;
	legacy_keycodes["KP_ENTER"] = 271;
	legacy_keycodes["KP_EQUALS"] = 272;
	legacy_keycodes["UP"] = 273;
	legacy_keycodes["DOWN"] = 274;
	legacy_keycodes["RIGHT"] = 275;
	legacy_keycodes["LEFT"] = 276;
	legacy_keycodes["INSERT"] = 277;
	legacy_keycodes["HOME"] = 278;
	legacy_keycodes["END"] = 279;
	legacy_keycodes["PAGEUP"] = 280;
	legacy_keycodes["PAGEDOWN"] = 281;
}
