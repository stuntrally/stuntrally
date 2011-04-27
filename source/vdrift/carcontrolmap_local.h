#ifndef _CARCONTROLMAP_LOCAL_H
#define _CARCONTROLMAP_LOCAL_H

#include "configfile.h"
#include "eventsystem.h"
#include "cardefs.h"
#include "car.h"

#include <list>
#include <string>

class WIDGET_CONTROLGRAB;
class GAME;

class CARCONTROLMAP_LOCAL
{
private:
	friend class WIDGET_CONTROLGRAB;
	friend class GAME;
	
	class CONTROL
	{
		public:
			enum TYPE
			{
				KEY,
    			JOY,
				MOUSE,
    			UNKNOWN
			} type;
			
			bool onetime;
			
			int joynum;
			int joyaxis;
			enum JOYAXISTYPE
			{
				POSITIVE,
    				NEGATIVE,
				BOTH
			} joyaxistype;
			int joybutton;
			//bool joybuttonlaststate;
			enum JOYTYPE
			{
				JOYAXIS,
    				JOYBUTTON,
				JOYHAT
			} joytype;
			bool joypushdown;

			int keycode;
			bool keypushdown;

			enum MOUSETYPE
			{
				MOUSEBUTTON,
    				MOUSEMOTION
			} mousetype;
			int mbutton;
			enum MOUSEDIRECTION
			{
				UP,
    				DOWN,
				LEFT,
    				RIGHT
			} mdir;
			bool last_mouse_state;
			bool mouse_push_down;

			float deadzone;
			float exponent;
			float gain;
			
			void DebugPrint(std::ostream & out)
			{
				out << type << " " << onetime << " " << joynum << " " << joyaxis << " " <<
						joyaxistype << " " << joybutton << " " << joytype << " " <<
						joypushdown << " " << keycode << " " << keypushdown << " " <<
						mousetype << " " << mbutton << " " << mdir << " " <<
						last_mouse_state << " " << mouse_push_down << " " <<
						deadzone << " " << exponent << " " << gain << std::endl;
			}
			
			void ReadFrom(std::istream & in)
			{
				int newtype, newjoyaxistype, newjoytype, newmousetype, newmdir;
				in >> newtype >> onetime >> joynum >> joyaxis >>
						newjoyaxistype >> joybutton >> newjoytype >>
						joypushdown >> keycode >> keypushdown >>
						newmousetype >> mbutton >> newmdir >>
						last_mouse_state >> mouse_push_down >>
						deadzone >> exponent >> gain;
				type=TYPE(newtype);
				joyaxistype=JOYAXISTYPE(newjoyaxistype);
				joytype=JOYTYPE(newjoytype);
				mousetype=MOUSETYPE(newmousetype);
				mdir=MOUSEDIRECTION(newmdir);
			}
			
			/*void MemDump(std::ostream & out)
			{
				for (unsigned int i = 0; i < sizeof(CONTROL); i++)
				{
					char c = ((char *) this)[i];
					std::cout << (int) c << " ";
				}
				std::cout << std::endl;
			}*/
			
			bool operator==(const CONTROL & other) const
			{
				CONTROL me = *this;
				CONTROL them = other;
				
				//don't care about certain flags
				me.onetime = 1;
				me.joypushdown = 1;
				me.keypushdown = 1;
				me.mouse_push_down = 1;
				me.deadzone = 0;
				me.exponent = 1;
				me.gain = 1;
				them.onetime = 1;
				them.joypushdown = 1;
				them.keypushdown = 1;
				them.mouse_push_down = 1;
				them.deadzone = 0;
				them.exponent = 1;
				them.gain = 1;
				
				std::stringstream mestr;
				std::stringstream themstr;
				me.DebugPrint(mestr);
				them.DebugPrint(themstr);
				
				return (mestr.str() == themstr.str());
				
				/*std::cout << "Checking:" << std::endl;
				me.DebugPrint(std::cout);
				me.MemDump(std::cout);
				them.DebugPrint(std::cout);
				them.MemDump(std::cout);
				std::cout << "Equality check: " << (std::memcmp(&me,&them,sizeof(CONTROL)) == 0) << std::endl;
				
				return (std::memcmp(&me,&them,sizeof(CONTROL)) == 0);*/
				
				//bool equality = (type == other.type) && (type == other.type) && 
			}
			
			bool operator<(const CONTROL & other) const
			{
				CONTROL me = *this;
				CONTROL them = other;
				
				me.onetime = 1;
				me.joypushdown = 1;
				me.keypushdown = 1;
				me.mouse_push_down = 1;
				me.deadzone = 0;
				me.exponent = 1;
				me.gain = 1;
				them.onetime = 1;
				them.joypushdown = 1;
				them.keypushdown = 1;
				them.mouse_push_down = 1;
				them.deadzone = 0;
				them.exponent = 1;
				them.gain = 1;
				
				std::stringstream mestr;
				std::stringstream themstr;
				me.DebugPrint(mestr);
				them.DebugPrint(themstr);
				
				return (mestr.str() < themstr.str());
			}
			
			CONTROL() : type(UNKNOWN),onetime(true),joynum(0),joyaxis(0),joyaxistype(POSITIVE),
				joybutton(0),joytype(JOYAXIS),joypushdown(true),keycode(0),keypushdown(true),
				mousetype(MOUSEBUTTON),mbutton(0),mdir(UP),last_mouse_state(false),
				mouse_push_down(true),
				deadzone(0.0), exponent(1.0), gain(1.0)
			{}
	};
	
	std::map <CARINPUT::CARINPUT, std::vector<CONTROL> > controls;
	
	///the vector is indexed by CARINPUT values
	std::vector <float> inputs;
	std::vector <float> lastinputs;
	
	///used to turn legacy key names from older vdrift releases into keycodes
	std::map <std::string, int> legacy_keycodes;
	void PopulateLegacyKeycodes();
	
	///used to stringify/destringify the CARINPUT enum
	std::map <std::string, CARINPUT::CARINPUT> carinput_stringmap;
	
	std::string GetStringFromInput(const CARINPUT::CARINPUT input) const
	{
		for (std::map <std::string, CARINPUT::CARINPUT>::const_iterator i =
			carinput_stringmap.begin(); i != carinput_stringmap.end(); ++i)
		{
			if (i->second == input)
				return i->first;
		}
		
		return "INVALID";
	}
	
	CARINPUT::CARINPUT GetInputFromString(const std::string & str) const
	{
		std::map <std::string, CARINPUT::CARINPUT>::const_iterator i = carinput_stringmap.find(str);
		if (i != carinput_stringmap.end())
			return i->second;
		else
			return CARINPUT::INVALID;
	}
	
	float ApplyDeadzone(float dz, float val) const
	{
		if (std::abs(val) < dz)
			val = 0;
		else
		{
			if (val < 0)
				val = (val + dz)*(1.0/(1.0-dz));
			else
				val = (val - dz)*(1.0/(1.0-dz));
		}
	
		return val;
	}

	float ApplyGain(float gain, float val) const
	{
		val *= gain;
		if (val < -1.0)
			val = -1.0;
		if (val > 1.0)
			val = 1.0;
	
		return val;
	}
	
	float ApplyExponent(float exponent, float val) const
	{
		val = pow(val, exponent);
		if (val < -1.0)
			val = -1.0;
		if (val > 1.0)
			val = 1.0;
	
		return val;
	}
	
	void ProcessSteering(const std::string & joytype, float steerpos, float dt, bool joy_200, float carmph, float speedsens);
	void AddControl(CONTROL newctrl, const std::string & inputname, bool only_one, std::ostream & error_output)
	{
		if (carinput_stringmap.find(inputname) != carinput_stringmap.end())
		{
			if (only_one)
				controls[carinput_stringmap[inputname]].clear();
			controls[carinput_stringmap[inputname]].push_back(newctrl);
			
			//remove duplicates
			std::sort (controls[carinput_stringmap[inputname]].begin(), controls[carinput_stringmap[inputname]].end());
			std::vector<CONTROL>::iterator it = std::unique(controls[carinput_stringmap[inputname]].begin(), controls[carinput_stringmap[inputname]].end());
			controls[carinput_stringmap[inputname]].resize( it - controls[carinput_stringmap[inputname]].begin() );
		}
		else
			error_output << "Input named " << inputname << " couldn't be assigned because it isn't used" << std::endl;
	}
	
	///ramps the start value to the end value using rate button_ramp.  if button_ramp is zero, infinite rate is assumed.
	float Ramp(float start, float end, float button_ramp, float dt)
	{
		//early exits
		if (start == end) //no ramp
			return end;
		if (dt <= 0) //no time increment
			return start;
		if (button_ramp == 0) //assume infinite rate
			return end;
		
		float cur = start;
		float sign = 1.0;  //0.3;
		if (end < start)
			sign = -1.0;  //-1.2;
		if (button_ramp > 0)
			cur += button_ramp*dt*sign;
		
		//std::cout << "start: " << start << ", end: " << end << ", cur: " << cur << ", increment: "  << button_ramp*dt*sign << std::endl;
		
		if (cur < 0)
			return 0;
		if (cur > 1.0)
			return 1.0;
		return cur;
	}
	
public:
	CARCONTROLMAP_LOCAL()
	{
		carinput_stringmap["gas"] = CARINPUT::THROTTLE;
		carinput_stringmap["boost"] = CARINPUT::BOOST;
		carinput_stringmap["flipleft"] = CARINPUT::FLIPLEFT;
		carinput_stringmap["flipright"] = CARINPUT::FLIPRIGHT;
		carinput_stringmap["brake"] = CARINPUT::BRAKE;
		carinput_stringmap["handbrake"] = CARINPUT::HANDBRAKE;
		carinput_stringmap["clutch"] = CARINPUT::CLUTCH;
		carinput_stringmap["steer_left"] = CARINPUT::STEER_LEFT;
		carinput_stringmap["steer_right"] = CARINPUT::STEER_RIGHT;
		carinput_stringmap["disengage_shift_up"] = CARINPUT::SHIFT_UP;
		carinput_stringmap["disengage_shift_down"] = CARINPUT::SHIFT_DOWN;
		carinput_stringmap["start_engine"] = CARINPUT::START_ENGINE;
		carinput_stringmap["abs_toggle"] = CARINPUT::ABS_TOGGLE;
		carinput_stringmap["tcs_toggle"] = CARINPUT::TCS_TOGGLE;
		carinput_stringmap["neutral"] = CARINPUT::NEUTRAL;
		carinput_stringmap["first_gear"] = CARINPUT::FIRST_GEAR;
		carinput_stringmap["second_gear"] = CARINPUT::SECOND_GEAR;
		carinput_stringmap["third_gear"] = CARINPUT::THIRD_GEAR;
		carinput_stringmap["fourth_gear"] = CARINPUT::FOURTH_GEAR;
		carinput_stringmap["fifth_gear"] = CARINPUT::FIFTH_GEAR;
		carinput_stringmap["sixth_gear"] = CARINPUT::SIXTH_GEAR;
		carinput_stringmap["reverse"] = CARINPUT::REVERSE;
		carinput_stringmap["rear_view"] = CARINPUT::REAR_VIEW;
		
		carinput_stringmap["view_prev"] = CARINPUT::VIEW_PREV;
		carinput_stringmap["view_next"] = CARINPUT::VIEW_NEXT;
		carinput_stringmap["view_hood"] = CARINPUT::VIEW_HOOD;
		carinput_stringmap["view_incar"] = CARINPUT::VIEW_INCAR;
		carinput_stringmap["view_chaserigid"] = CARINPUT::VIEW_CHASERIGID;
		carinput_stringmap["view_chase"] = CARINPUT::VIEW_CHASE;
		carinput_stringmap["view_orbit"] = CARINPUT::VIEW_ORBIT;
		carinput_stringmap["view_free"] = CARINPUT::VIEW_FREE;
		carinput_stringmap["pan_left"] = CARINPUT::PAN_LEFT;
		carinput_stringmap["pan_right"] = CARINPUT::PAN_RIGHT;
		carinput_stringmap["pan_up"] = CARINPUT::PAN_UP;
		carinput_stringmap["pan_down"] = CARINPUT::PAN_DOWN;
		carinput_stringmap["zoom_in"] = CARINPUT::ZOOM_IN;
		carinput_stringmap["zoom_out"] = CARINPUT::ZOOM_OUT;
		carinput_stringmap["screen_shot"] = CARINPUT::SCREENSHOT;
		carinput_stringmap["pause"] = CARINPUT::PAUSE;
		carinput_stringmap["reload_shaders"] = CARINPUT::RELOAD_SHADERS;
		
		PopulateLegacyKeycodes();
	}
	
	void Load(const std::string & controlfile, std::ostream & info_output, std::ostream & error_output);
	void Save(const std::string & controlfile, std::ostream & info_output, std::ostream & error_output)
	{
		CONFIGFILE controls_config;
		Save(controls_config, info_output, error_output);
		controls_config.Write(true, controlfile);
	}
	
	void Save(CONFIGFILE & controlfile, std::ostream & info_output, std::ostream & error_output);
	
	///query the eventsystem for info, then return the resulting input array
	const std::vector <float> & ProcessInput(class App* pApp, int player,
		const std::string & joytype, EVENTSYSTEM_SDL & eventsystem, float steerpos, float dt, bool joy_200, float carms, float speedsens, int screenw, int screenh, float button_ramp, bool hgateshifter);
	
	float GetInput(CARINPUT::CARINPUT inputid) const {assert((unsigned int) inputid < inputs.size());return inputs[inputid];}
	
	void AddInputKey(const std::string & inputname, bool analog, bool only_one, SDLKey key, std::ostream & error_output)
	{
		CONTROL newctrl;
		
		newctrl.type = CONTROL::KEY;
		newctrl.keycode = key;
		newctrl.keypushdown = true;
		newctrl.onetime = !analog;
		
		//std::cout << "Assigning input " << inputname << " keycode " << key << " onlyone " << only_one << std::endl;
		
		AddControl(newctrl, inputname, only_one, error_output);
	}
	
	void AddInputMouseButton(const std::string & inputname, bool analog, bool only_one, int mouse_btn, std::ostream & error_output)
	{
		CONTROL newctrl;
		
		newctrl.type = CONTROL::MOUSE;
		newctrl.mousetype = CONTROL::MOUSEBUTTON;
		newctrl.mbutton = mouse_btn;
		newctrl.mouse_push_down = true;
		newctrl.onetime = !analog;
		
		//std::cout << "Assigning input " << inputname << " keycode " << key << " onlyone " << only_one << std::endl;
		
		AddControl(newctrl, inputname, only_one, error_output);
	}
	
	void AddInputMouseMotion(const std::string & inputname, bool analog, bool only_one, const std::string & mouse_direction, std::ostream & error_output)
	{
		CONTROL newctrl;
		
		newctrl.type = CONTROL::MOUSE;
		newctrl.mousetype = CONTROL::MOUSEMOTION;
		if (mouse_direction == "left")
			newctrl.mdir = CONTROL::LEFT;
		else if (mouse_direction == "right")
			newctrl.mdir = CONTROL::RIGHT;
		else if (mouse_direction == "up")
			newctrl.mdir = CONTROL::UP;
		else if (mouse_direction == "down")
			newctrl.mdir = CONTROL::DOWN;
		else
			return;
		
		//std::cout << "Assigning input " << inputname << " keycode " << key << " onlyone " << only_one << std::endl;
		
		AddControl(newctrl, inputname, only_one, error_output);
	}
	
	void AddInputJoyButton(const std::string & inputname, bool analog, bool only_one, int joy_num, int joy_btn, std::ostream & error_output)
	{
		CONTROL newctrl;
		
		newctrl.type = CONTROL::JOY;
		newctrl.joynum = joy_num;
		newctrl.joytype = CONTROL::JOYBUTTON;
		newctrl.joybutton = joy_btn;
		newctrl.joypushdown = true;
		newctrl.onetime = !analog;
		
		//std::cout << "Assigning input " << inputname << " keycode " << key << " onlyone " << only_one << std::endl;
		
		AddControl(newctrl, inputname, only_one, error_output);
	}
	
	void AddInputJoyAxis(const std::string & inputname, bool analog, bool only_one, int joy_num, int joy_axis, const std::string & axis_type, std::ostream & error_output)
	{
		CONTROL newctrl;
		
		newctrl.type = CONTROL::JOY;
		newctrl.joytype = CONTROL::JOYAXIS;
		newctrl.joynum = joy_num;
		float deadzone = 0.0;
		float exponent = 1.0;
		float gain = 1.0;
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
			return;
		}
		newctrl.deadzone = ( deadzone );
		newctrl.exponent = ( exponent );
		newctrl.gain = ( gain );
		
		//std::cout << "Assigning input " << inputname << " keycode " << key << " onlyone " << only_one << std::endl;
		
		AddControl(newctrl, inputname, only_one, error_output);
	}
	
	void DeleteControl(CONTROL ctrltodel, const std::string & inputname, std::ostream & error_output)
	{
		if (carinput_stringmap.find(inputname) != carinput_stringmap.end())
		{
			assert(controls.find(carinput_stringmap[inputname]) != controls.end());
			
			std::vector<CONTROL>::iterator todel = controls[carinput_stringmap[inputname]].end();
			
			for (std::vector<CONTROL>::iterator i =
				controls[carinput_stringmap[inputname]].begin();
				i != controls[carinput_stringmap[inputname]].end(); ++i)
			{
				if (ctrltodel == *i)
					todel = i;
				else
				{
					/*std::cout << "this: ";
					i->DebugPrint(std::cout);
					std::cout << "not : ";
					ctrltodel.DebugPrint(std::cout);*/
				}
			}
			assert (todel != controls[carinput_stringmap[inputname]].end());
			controls[carinput_stringmap[inputname]].erase(todel);
		}
		else
			error_output << "Input named " << inputname << " couldn't be deleted because it isn't used" << std::endl;
	}
	
	void UpdateControl(CONTROL ctrltoupdate, const std::string & inputname, std::ostream & error_output)
	{
		if (carinput_stringmap.find(inputname) != carinput_stringmap.end())
		{
			//input name was found
			
			//verify a control list was found for this input
			assert(controls.find(carinput_stringmap[inputname]) != controls.end());
			
			bool did_update = false;
			
			for (std::vector<CONTROL>::iterator i =
				controls[carinput_stringmap[inputname]].begin();
				i != controls[carinput_stringmap[inputname]].end(); ++i)
			{
				if (ctrltoupdate == *i)
				{
					*i = ctrltoupdate;
					did_update = true;
					//std::cout << "New deadzone: " << i->deadzone << std::endl;
				}
				else
				{
					/*std::cout << "this: ";
					i->DebugPrint(std::cout);
					std::cout << "not : ";
					ctrltodel.DebugPrint(std::cout);*/
				}
			}
			
			assert(did_update);
		}
		else
			error_output << "Input named " << inputname << " couldn't be deleted because it isn't used" << std::endl;
	}

	const std::vector< float > & GetInputs() const
	{
		return inputs;
	}
	
};

#endif
