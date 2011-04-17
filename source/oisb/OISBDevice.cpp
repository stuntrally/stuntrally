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

#include "OISBDevice.h"
#include "OISBState.h"
#include "OISException.h"

#include <iostream>

namespace OISB
{
	Device::Device()
	{}

	Device::~Device()
	{}
	
	State* Device::getState(const String& name) const
	{
		StateMap::const_iterator it = mStates.find(name);
		if (it == mStates.end())
		{
			OIS_EXCEPT(OIS::E_General, String("State '" + name + "' not found in Device '" + getName() + "'").c_str());
		}
		
		return it->second;
	}

    bool Device::hasState(const String& name) const
    {
        StateMap::const_iterator it = mStates.find(name);
		if (it == mStates.end())
		{
			return false;
		}

        return true;
    }

    void Device::addListenerToAllStates(BindableListener* listener)
    {
        for (StateMap::const_iterator it = mStates.begin(); it != mStates.end(); ++it)
		{
			it->second->addListener(listener);
		}
    }

    void Device::removeListenerFromAllStates(BindableListener* listener)
    {
        for (StateMap::const_iterator it = mStates.begin(); it != mStates.end(); ++it)
		{
			it->second->removeListener(listener);
		}
    }
	
	void Device::addState(State* state)
	{
		StateMap::const_iterator it = mStates.find(state->getName());
		
		if (it != mStates.end())
		{
			OIS_EXCEPT(OIS::E_Duplicate, String("State with a name '" + state->getName() + "' already exists in Device '" + getName() + "'").c_str());
		}
		
		mStates.insert(std::make_pair(state->getName(), state));
	}
	
	void Device::removeState(State* state)
	{
		StateMap::iterator it = mStates.find(state->getName());
		
		if (it == mStates.end())
		{
			OIS_EXCEPT(OIS::E_General, String("State with a name '" + state->getName() + "' not found in Device '" + getName() + "'").c_str());
		}
		
		mStates.erase(it);
	}

	void Device::dump()
	{
	    std::cout << "** Device: '" << getName() << "'" << std::endl;

	    for (StateMap::const_iterator it = mStates.begin(); it != mStates.end(); ++it)
        {
	        // TODO: dump state type too
            std::cout << "** - State: " << it->second->getName() << std::endl;
        }

        std::cout << "** End of device '" << getName() << "'" << std::endl;
	}
}
