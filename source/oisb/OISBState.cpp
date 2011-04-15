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

#include "OISBState.h"
#include "OISBDevice.h"
#include "OISException.h"

namespace OISB
{
	State::State(Device* parent, const String& name):
		mParent(parent),
		mName(name),

        mIsActive(false),
        mChanged(false)
	{}
	
	State::~State()
	{}

    BindableType State::getBindableType() const
    {
        return BT_STATE;
    }

    String State::getBindableName() const
    {
        return "State: " + getFullName();
    }

    bool State::isActive() const
    {
        return mIsActive;
    }

    bool State::hasChanged() const
    {
        return mChanged;
    }

	String State::getFullName() const
	{
		return mParent->getName() + "/" + getName();
	}

    void State::listProperties(PropertyList& list)
    {
        Bindable::listProperties(list);

        list.push_back("StateName");
        list.push_back("ParentDeviceName");
    }

    void State::impl_setProperty(const String& name, const String& value)
    {
        if (name == "StateName")
        {
            OIS_EXCEPT(OIS::E_InvalidParam, "'StateName' is a read only, you can't set it!");
        }
        else if (name == "ParentDeviceName")
        {
            OIS_EXCEPT(OIS::E_InvalidParam, "'ParentDeviceName' is a read only, you can't set it!");
        }
        else
        {
            // nothing matched, delegate up
            Bindable::impl_setProperty(name, value);
        }
    }

    String State::impl_getProperty(const String& name) const
    {
        if (name == "StateName")
        {
            return getName();
        }
        else if (name == "ParentDeviceName")
        {
            // no need to check, every state must have a valid parent
            return mParent->getName();
        }
        else
        {
            // nothing matched, delegate up
            return Bindable::impl_getProperty(name);
        }
    }

    void State::activate()
    {
        mIsActive = true;

        notifyActivated();
    }

    void State::deactivate()
    {
        mIsActive = false;

        notifyDeactivated();
    }
}
