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

#include "OISBAnalogAxisState.h"
#include <cmath>
#include <limits>

namespace OISB
{
	AnalogAxisState::AnalogAxisState(Device* parent, const String& name):
		State(parent, name),
		
		mAbsoluteValue(0),
		mRelativeValue(0),
		
		mOffset(0),
		mMinimumValue(-1),
		mMaximumValue(1),
		mSensitivity(1)
	{}
	
	AnalogAxisState::~AnalogAxisState()
	{}
	
	StateType AnalogAxisState::getStateType() const
	{
		return ST_ANALOG_AXIS;
	}

    void AnalogAxisState::_setMinimumValue(Real min)
    {
        mMinimumValue = min;
    }

    void AnalogAxisState::_setMaximumValue(Real max)
    {
        mMaximumValue = max;
    }

	void AnalogAxisState::_setAbsoluteValue(Real value)
	{
        bool willBeActive = false;

		value += mOffset;
		mRelativeValue = (value - mAbsoluteValue) * mSensitivity;
		
		if (fabs(mRelativeValue) > std::numeric_limits<Real>::epsilon())
		{
			mAbsoluteValue += mRelativeValue;
			
            mAbsoluteValue = std::min(mMaximumValue, mAbsoluteValue);
            mAbsoluteValue = std::max(mMinimumValue, mAbsoluteValue);

            willBeActive = true;
		}

        mChanged = willBeActive != mIsActive;

        if (willBeActive && !mIsActive)
        {
            activate();
        }
        else if (!willBeActive && mIsActive)
        {
            deactivate();
        }

        notifyProcessed();
	}
}
