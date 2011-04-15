/*
The zlib/libpng License

Copyright (c) 2009-2010 Martin Preisler

This software is provided 'as-is', without any express or implied warranty. In no event will
the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial 
applications, and to alter it and redistribute it freely, subject to the following
restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that 
		you wrote the original software. If you use this software in a product, 
		an acknowledgment in the product documentation would be appreciated but is 
		not required.

    2. Altered source versions must be plainly marked as such, and must not be 
		misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#include "OISBJoyStick.h"
#include "OISBAnalogAxisState.h"
#include "OISBDigitalState.h"

#include <OISJoyStick.h>

#include <stdio.h>

// TODO: finish sliders, POV

namespace OISB
{
	JoyStick::JoyStick(OIS::JoyStick* joy):
		mJoyStick(joy)
	{
		int num_axis = mJoyStick->getNumberOfComponents(OIS::OIS_Axis);
		for(int a = 0; a<num_axis; a++)
		{
			char tmp[255]="";
			sprintf(tmp, "Axis %d", a);
			axis.push_back(new AnalogAxisState(this, String(tmp)));
		}

		int num_buttons = mJoyStick->getNumberOfComponents(OIS::OIS_Button);
		for(int a = 0; a<num_buttons; a++)
		{
			char tmp[255]="";
			sprintf(tmp, "Button %d", a);
			buttons.push_back(new DigitalState(this, String(tmp)));
		}

		int num_slider = mJoyStick->getNumberOfComponents(OIS::OIS_Slider);
		for(int a = 0; a<num_slider; a++)
		{
			char tmp[255]="";
			sprintf(tmp, "Slider %d", a);
			axis.push_back(new AnalogAxisState(this, String(tmp)));
		}

		int num_pov = mJoyStick->getNumberOfComponents(OIS::OIS_POV);
		for(int a = 0; a<num_pov; a++)
		{
			char tmp[255]="";
			sprintf(tmp, "POV %d", a);
			// TODO: fix POV
			buttons.push_back(new DigitalState(this, String(tmp)));
		}

		// now add the states
		for(std::vector<AnalogAxisState*>::iterator it = axis.begin(); it != axis.end(); it++)
			addState(*it);
		for(std::vector<DigitalState*>::iterator it = buttons.begin(); it != buttons.end(); it++)
			addState(*it);
	}
			
	JoyStick::~JoyStick()
	{
		for(std::vector<AnalogAxisState*>::iterator it = axis.begin(); it != axis.end(); it++)
		{
			removeState(*it);
			delete(*it);
		}
		axis.clear();
		for(std::vector<DigitalState*>::iterator it = buttons.begin(); it != buttons.end(); it++)
		{
			removeState(*it);
			delete(*it);
		}
		buttons.clear();
	}

	const String& JoyStick::getName() const
	{
		//static String name = "JoyStick" + mJoyStick->getID() mJoyStick->vendor();
		static String name = mJoyStick->vendor();
		return name;
	}

	void JoyStick::process(Real delta)
	{
		const OIS::JoyStickState& state = mJoyStick->getJoyStickState();
		for(unsigned int a = 0; a < state.mAxes.size(); a++)
		{
			if(state.mAxes[a].absOnly)
				axis[a]->_setAbsoluteValue(state.mAxes[a].abs / (float)OIS::JoyStick::MAX_AXIS);
			else
				axis[a]->_setRelativeValue(state.mAxes[a].rel / (float)OIS::JoyStick::MAX_AXIS);
		}
		for(unsigned int b = 0; b < state.mButtons.size(); b++)
		{
			buttons[b]->_setValue(state.mButtons[b]);
		}

	}

}
