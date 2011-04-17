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

#include "OISBAnalogEmulation.h"
#include "OISBBinding.h"
#include "OISBBindable.h"
#include "OISBAnalogAxisAction.h"

#include "OISException.h"

namespace OISB
{
    AnalogEmulator::AnalogEmulator():
        mTarget(0)
    {}

    AnalogEmulator::~AnalogEmulator()
    {}

    void AnalogEmulator::setTarget(AnalogAxisAction* target)
    {
        mTarget = target;
    }

    LinearAnalogEmulator::LinearAnalogEmulator():
        mIncreaseSpeed(1.0f),
        mDecreaseSpeed(1.0f),

        mReturnEnabled(true),
        mReturnValue(0.0f),
        mReturnIncreaseSpeed(1.0f),
        mReturnDecreaseSpeed(1.0f)
    {}
           
    LinearAnalogEmulator::~LinearAnalogEmulator()
    {}

    const String& LinearAnalogEmulator::getType() const
    {
        static String type = "Linear";
        return type;
    }

    bool LinearAnalogEmulator::checkBinding(Binding* binding)
    {
        return binding->getNumBindables() == 2;
    }
            
    Real LinearAnalogEmulator::emulateRelative(Binding* binding, Real delta)
    {
        Real ret = 0.0f;

        Bindable* decrease;
        Bindable* increase;

        getBindables(binding, &decrease, &increase);

        if (decrease->isActive())
        {
            // decrease is pressed
                        
            ret = ((-1.0f) * mDecreaseSpeed * delta) * mTarget->getSensitivity();
        }
        if (increase->isActive())
        {
            // increase is pressed
                        
            ret = ((+1.0f) * mIncreaseSpeed * delta) * mTarget->getSensitivity();
        }

        if (mReturnEnabled && (!increase->isActive() && !decrease->isActive()))
        {
            // we have to do returning to the starting point there
            if (mReturnValue > mTarget->getAbsoluteValue())
            {
                ret = ((+1.0f) * mReturnIncreaseSpeed * delta) * mTarget->getSensitivity();
            }
            else if (mReturnValue < mTarget->getAbsoluteValue())
            {
                ret = ((-1.0f) * mReturnDecreaseSpeed * delta) * mTarget->getSensitivity();
            }
        }

        return ret;
    }

    Real LinearAnalogEmulator::emulateAbsolute(Binding* binding, Real delta)
    {
        Real ret = 0.0f;

        Bindable* decrease;
        Bindable* increase;

        getBindables(binding, &decrease, &increase);

        // both are pressed
        if (decrease->isActive() && increase->isActive())
        {
            // bigger speed wins
            if (mDecreaseSpeed > mIncreaseSpeed)
            {
                ret = mTarget->getMinimumValue();
            }
            else if (mDecreaseSpeed < mIncreaseSpeed)
            {
                ret = mTarget->getMaximumValue();
            }
            else
            {
                ret = (mTarget->getMinimumValue() + mTarget->getMaximumValue()) * 0.5f;
            }
        }
        else if (decrease->isActive())
        {
            ret = mTarget->getMinimumValue();
        }
        else if (increase->isActive())
        {
            ret = mTarget->getMaximumValue();
        }
        // the only remaining case is that nothing is pressed
        // in that case, there is nothing to do

        // no need to do value return, in the absolute case, it doesn't make any sense
        return ret;
    }

    void LinearAnalogEmulator::setDecreaseSpeed(Real speed)
    {
        mDecreaseSpeed = speed;
    }

    void LinearAnalogEmulator::setIncreaseSpeed(Real speed)
    {
        mIncreaseSpeed = speed;
    }

    void LinearAnalogEmulator::setSpeed(Real speed)
    {
        setDecreaseSpeed(speed);
        setIncreaseSpeed(speed);
    }

    void LinearAnalogEmulator::setReturnEnabled(bool enabled)
    {
        mReturnEnabled = enabled;
    }

    void LinearAnalogEmulator::setReturnValue(Real value)
    {
        mReturnValue = value;
    }

    void LinearAnalogEmulator::setReturnDecreaseSpeed(Real speed)
    {
        mReturnDecreaseSpeed = speed;
    }

    void LinearAnalogEmulator::setReturnIncreaseSpeed(Real speed)
    {
        mReturnIncreaseSpeed = speed;
    }

    void LinearAnalogEmulator::setReturnSpeed(Real speed)
    {
        setReturnDecreaseSpeed(speed);
        setReturnIncreaseSpeed(speed);
    }

    void LinearAnalogEmulator::listProperties(PropertyList& list)
    {
        AnalogEmulator::listProperties(list);

        list.push_back("EmulationDecreaseSpeed");
        list.push_back("EmulationIncreaseSpeed");
        list.push_back("EmulationSpeed");

        list.push_back("EmulationReturnEnabled");
        list.push_back("EmulationReturnValue");
        list.push_back("EmulationReturnDecreaseSpeed");
        list.push_back("EmulationReturnIncreaseSpeed");
        list.push_back("EmulationReturnSpeed");
    }

    void LinearAnalogEmulator::impl_setProperty(const String& name, const String& value)
    {
        if (name == "EmulationDecreaseSpeed")
        {
            setDecreaseSpeed(fromString<Real>(value));
        }
        else if (name == "EmulationIncreaseSpeed")
        {
            setIncreaseSpeed(fromString<Real>(value));
        }
        else if (name == "EmulationSpeed")
        {
            setSpeed(fromString<Real>(value));
        }

        else if (name == "EmulationReturnEnabled")
        {
            setReturnEnabled(fromString<bool>(value));
        }
        else if (name == "EmulationReturnValue")
        {
            setReturnValue(fromString<Real>(value));
        }
        else if (name == "EmulationReturnDecreaseSpeed")
        {
            setReturnDecreaseSpeed(fromString<Real>(value));
        }
        else if (name == "EmulationReturnIncreaseSpeed")
        {
            setReturnIncreaseSpeed(fromString<Real>(value));
        }
        else if (name == "EmulationReturnSpeed")
        {
            setReturnSpeed(fromString<Real>(value));
        }
        else
        {
            // nothing matched, delegate up
            AnalogEmulator::impl_setProperty(name, value);
        }
    }

    String LinearAnalogEmulator::impl_getProperty(const String& name) const
    {
        if (name == "EmulationDecreaseSpeed")
        {
            return toString(getDecreaseSpeed());
        }
        else if (name == "EmulationIncreaseSpeed")
        {
            return toString(getIncreaseSpeed());
        }
        else if (name == "EmulationSpeed")
        {
            OIS_EXCEPT(OIS::E_InvalidParam, "'EmulationSpeed' is a convenience property that "
                "sets both increase and decrease emulation speed variants, you can't get it's value!");
        }

        else if (name == "EmulationReturnEnabled")
        {
            return toString(isReturnEnabled());
        }
        else if (name == "EmulationReturnValue")
        {
            return toString(getReturnValue());
        }
        else if (name == "EmulationReturnDecreaseSpeed")
        {
            return toString(getReturnDecreaseSpeed());
        }
        else if (name == "EmulationReturnIncreaseSpeed")
        {
            return toString(getReturnIncreaseSpeed());
        }
        else if (name == "EmulationReturnSpeed")
        {
            OIS_EXCEPT(OIS::E_InvalidParam, "'EmulationSpeed' is a convenience property that "
                "sets both increase and decrease emulation speed variants, you can't get it's value!");
        }
        else
        {
            // nothing matched, delegate up
            return AnalogEmulator::impl_getProperty(name);
        }
    }

    void LinearAnalogEmulator::getBindables(Binding* binding, Bindable** decrease, Bindable** increase)
    {
        // there are 2 bindables in the binding (see checkBinding)
        *decrease = 0;
        *increase = 0;

        // first check for roles
        if (binding->isBound("decrease") &&
            binding->isBound("increase"))
        {
            *decrease = binding->getBindable("decrease");
            *increase = binding->getBindable("increase");
        }
        // otherwise the first will decrease the value
        // and the second (and last) will increase the value
        else
        {
            *decrease = binding->getBindable(0);
            *increase = binding->getBindable(1);
        }
    }
}
