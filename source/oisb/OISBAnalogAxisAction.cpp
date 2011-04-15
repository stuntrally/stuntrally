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

#include "OISBAnalogAxisAction.h"
#include "OISBAnalogEmulation.h"
#include "OISBBinding.h"
#include "OISBDigitalState.h"
#include "OISBAnalogAxisState.h"

#include "OISException.h"

#include <cmath>
#include <limits>

namespace OISB
{
    ActionType AnalogAxisAction::ACTION_TYPE = AT_ANALOG_AXIS;

	AnalogAxisAction::AnalogAxisAction(ActionSchema* parent, const String& name):
		Action(parent, name),

        mUseAbsoluteValues(false),

        mAbsoluteValue(0.0f),
        mRelativeValue(0.0f),

        mMinimumValue(-1.0f),
        mMaximumValue(1.0f),
        mSensitivity(1.0f),

        mAnalogEmulator(0)
	{
        setAnalogEmulator(new LinearAnalogEmulator());
    }
	
	AnalogAxisAction::~AnalogAxisAction()
	{
        // mAnalogEmulator is owned by this class, we should handle it's deletion
        if (mAnalogEmulator)
        {
            delete mAnalogEmulator;
        }
    }
	
	ActionType AnalogAxisAction::getActionType() const
    {
        return AT_ANALOG_AXIS;
    }

    void AnalogAxisAction::setUseAbsoluteValues(bool use)
    {
        mUseAbsoluteValues = use;
    }

    void AnalogAxisAction::setMinimumValue(Real min)
    {
        mMinimumValue = min;
    }

    void AnalogAxisAction::setMaximumValue(Real max)
    {
        mMaximumValue = max;
    }

    void AnalogAxisAction::setSensitivity(Real sensitivity)
    {
        mSensitivity = sensitivity;
    }

    void AnalogAxisAction::setAnalogEmulator(AnalogEmulator* emulator)
    {
        if (mAnalogEmulator)
        {
            // mAnalogEmulator is owned by this class, we should handle it's deletion
            delete mAnalogEmulator;
        }

        mAnalogEmulator = emulator;

        if (mAnalogEmulator)
        {
            mAnalogEmulator->setTarget(this);
        }
    }

    void AnalogAxisAction::listProperties(PropertyList& list)
    {
        Bindable::listProperties(list);

        list.push_back("UseAbsoluteValues");

        list.push_back("AbsoluteValue");
        list.push_back("RelativeValue");

        list.push_back("MinimumValue");
        list.push_back("MaximumValue");
        list.push_back("Sensitivity");

        list.push_back("AnalogEmulator");

        if (mAnalogEmulator)
        {
            // inherit the emulator properties
            mAnalogEmulator->listProperties(list);
        }
    }

    void AnalogAxisAction::impl_setProperty(const String& name, const String& value)
    {
        if (name == "UseAbsoluteValues")
        {
            setUseAbsoluteValues(fromString<bool>(value));
        }

        else if (name == "AbsoluteValue")
        {
            OIS_EXCEPT(OIS::E_InvalidParam, "'AbsoluteValue' is a read only, you can't set it!");
        }
        else if (name == "RelativeValue")
        {
            OIS_EXCEPT(OIS::E_InvalidParam, "'AbsoluteValue' is a read only, you can't set it!");
        }

        else if (name == "MinimumValue")
        {
            setMinimumValue(fromString<Real>(value));
        }
        else if (name == "MaximumValue")
        {
            setMaximumValue(fromString<Real>(value));
        }
        else if (name == "Sensitivity")
        {
            setSensitivity(fromString<Real>(value));
        }

        else if (name == "AnalogEmulator")
        {
            if (value == "")
            {
                setAnalogEmulator(0);
            }
            if (value == "Linear")
            {
                setAnalogEmulator(new LinearAnalogEmulator());
            }
            else
            {
                OIS_EXCEPT(OIS::E_InvalidParam, "Invalid value for 'AnalogEmulator'!");
            }
        }
        else
        {
            if (mAnalogEmulator)
            {
                try
                {
                    mAnalogEmulator->setProperty(name, value);
                }
                catch (const OIS::Exception&)
                {
                    // nothing matched, delegate up
                    Action::impl_setProperty(name, value);
                }
            }
            else
            {
                // nothing matched, delegate up
                Action::impl_setProperty(name, value);
            }
        }
    }

    String AnalogAxisAction::impl_getProperty(const String& name) const
    {
        if (name == "UseAbsoluteValues")
        {
            return toString(getUseAbsoluteValues());
        }

        else if (name == "AbsoluteValue")
        {
            return toString(getAbsoluteValue());
        }
        else if (name == "RelativeValue")
        {
            return toString(getRelativeValue());
        }

        else if (name == "MinimumValue")
        {
            return toString(getMinimumValue());
        }
        else if (name == "MaximumValue")
        {
            return toString(getMaximumValue());
        }
        else if (name == "Sensitivity")
        {
            return toString(getSensitivity());
        }

        if (name == "AnalogEmulator")
        {
            return mAnalogEmulator ? mAnalogEmulator->getType() : "";
        }
        else
        {
            if (mAnalogEmulator)
            {
                try
                {
                    return mAnalogEmulator->getProperty<String>(name);
                }
                catch (const OIS::Exception&)
                {
                    // nothing matched, delegate up
                    return Action::impl_getProperty(name);
                }
            }
            else
            {
                // nothing matched, delegate up
                return Action::impl_getProperty(name);
            }
        }
    }
    	
    bool AnalogAxisAction::impl_process(Real delta)
    {
        mRelativeValue = 0.0f;

        if (mUseAbsoluteValues && mBindings.size() > 1)
        {
            OIS_EXCEPT(OIS::E_General, String("Using multiple alternative bindings when mUseAbsoluteValues is "
                "true is not what you wanted to do probably").c_str());
        }

        for (BindingList::const_iterator it = mBindings.begin(); it != mBindings.end(); ++it)
        {
            Binding* binding = *it;

            const Real mOldRelativeValue = mRelativeValue;

            if (mAnalogEmulator && mAnalogEmulator->checkBinding(binding))
            {
                if (mUseAbsoluteValues)
                {
                    const Real abs = mAnalogEmulator->emulateAbsolute(binding, delta);
                    mRelativeValue = abs - mAbsoluteValue;
                    mAbsoluteValue = abs;
                }
                else
                {
                    const Real rel = mAnalogEmulator->emulateRelative(binding, delta);
                    mRelativeValue = rel;
                    mAbsoluteValue += mRelativeValue;
                }
            }
            else
            {
                if (binding->getNumBindables() > 1)
                {
                    OIS_EXCEPT(OIS::E_General, String("Having multiple states in a binding "
                        "is only allowed for emulation (analog emulator is not set or didn't "
                        "approve of the binding as well!)").c_str());
                }

                State* st = binding->getState(0);
                if (st->getStateType() != ST_ANALOG_AXIS)
                {
                    OIS_EXCEPT(OIS::E_General, String("There is only one state bound and "
                        "it is not analog axis state!").c_str());
                }

                AnalogAxisState* state = static_cast<AnalogAxisState*>(st);

                if (mUseAbsoluteValues)
                {
                    mRelativeValue = state->getAbsoluteValue() - mAbsoluteValue;
                    mAbsoluteValue = state->getAbsoluteValue();
                }
                else
                {
                    mRelativeValue = state->getRelativeValue() * mSensitivity;
                    mAbsoluteValue += mRelativeValue;
                }
            }

            binding->_setActive(mOldRelativeValue != mRelativeValue);
        }

        // clamp the value to limits
        mAbsoluteValue = std::min(mMaximumValue, mAbsoluteValue);
        mAbsoluteValue = std::max(mMinimumValue, mAbsoluteValue);

        return fabs(mRelativeValue) < std::numeric_limits<Real>::epsilon();
    }
}
