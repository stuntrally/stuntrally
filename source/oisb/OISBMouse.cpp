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

#include "OISBMouse.h"
#include "OISBAnalogAxisState.h"
#include "OISBDigitalState.h"

#include <OISMouse.h>

namespace OISB
{
	Mouse::Mouse(OIS::Mouse* mouse):
		mMouse(mouse)
	{
		mXAxis = new AnalogAxisState(this, "X Axis");
		addState(mXAxis);
		mYAxis = new AnalogAxisState(this, "Y Axis");
		addState(mYAxis);
		mWheel = new AnalogAxisState(this, "Wheel");
		addState(mWheel);

		mLeftButton = new DigitalState(this, "Left Button");
		addState(mLeftButton);
		mRightButton = new DigitalState(this, "Right Button");
		addState(mRightButton);
		mMiddleButton = new DigitalState(this, "Middle Button");
		addState(mMiddleButton);

		mButton3 = new DigitalState(this, "Button 3");
		addState(mButton3);
		mButton4 = new DigitalState(this, "Button 4");
		addState(mButton4);
		mButton5 = new DigitalState(this, "Button 5");
		addState(mButton5);
		mButton6 = new DigitalState(this, "Button 6");
		addState(mButton6);
		mButton7 = new DigitalState(this, "Button 7");
		addState(mButton7);

        notifyScreenResize(1024.0f, 768.0f);
	}
			
	Mouse::~Mouse()
	{	
		removeState(mLeftButton);
		delete mLeftButton;
		mLeftButton = 0;
		removeState(mRightButton);
		delete mRightButton;
		mRightButton = 0;
		removeState(mMiddleButton);
		delete mMiddleButton;
		mMiddleButton = 0;

		removeState(mButton3);
		delete mButton3;
		mButton3 = 0;
		removeState(mButton4);
		delete mButton4;
		mButton4 = 0;
		removeState(mButton5);
		delete mButton5;
		mButton5 = 0;
		removeState(mButton6);
		delete mButton6;
		mButton6 = 0;
		removeState(mButton7);
		delete mButton7;
		mButton7 = 0;

		removeState(mXAxis);
		delete mXAxis;
		mXAxis = 0;
		removeState(mYAxis);
		delete mYAxis;
		mYAxis = 0;
		removeState(mWheel);
		delete mWheel;
		mWheel = 0;
	}

	const String& Mouse::getName() const
	{
		static String name = "Mouse";
		return name;
	}

	void Mouse::process(Real delta)
	{
		const OIS::MouseState& state = mMouse->getMouseState();

		mXAxis->_setRelativeValue(static_cast<Real>(state.X.rel));
		mYAxis->_setRelativeValue(static_cast<Real>(state.Y.rel));
		mWheel->_setRelativeValue(static_cast<Real>(state.Z.rel));

		mLeftButton->_setValue(state.buttonDown(OIS::MB_Left));
		mRightButton->_setValue(state.buttonDown(OIS::MB_Right));
		mMiddleButton->_setValue(state.buttonDown(OIS::MB_Middle));

		mButton3->_setValue(state.buttonDown(OIS::MB_Button3));
		mButton4->_setValue(state.buttonDown(OIS::MB_Button4));
		mButton5->_setValue(state.buttonDown(OIS::MB_Button5));
		mButton6->_setValue(state.buttonDown(OIS::MB_Button6));
		mButton7->_setValue(state.buttonDown(OIS::MB_Button7));
	}

    void Mouse::notifyScreenResize(Real width, Real height)
    {
        mXAxis->_setMinimumValue(0.0f);
        mXAxis->_setMaximumValue(width);

        mYAxis->_setMinimumValue(0.0f);
        mYAxis->_setMaximumValue(height);
    }
}
