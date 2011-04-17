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

#include "OISBActionSchema.h"

#include "OISBTriggerAction.h"
#include "OISBSequenceAction.h"
#include "OISBAnalogAxisAction.h"

#include "OISException.h"

#include <cassert>
#include <iostream>

namespace OISB
{
    ActionSchema::ActionSchema(const String& name):
		mName(name),
		mIsEnabled(true)
	{}

	ActionSchema::~ActionSchema()
	{
		// destroy all actions automatically, we don't instruct the user to destroy
		// these, we just destroy them ourselves
		while (mActions.size() > 0)
		{
			destroyAction(mActions.begin()->first);
		}
	}
	
	void ActionSchema::setEnabled(bool enabled)
	{
		mIsEnabled = enabled;
	}
	
	Action* ActionSchema::createAction(ActionType type, const String& name)
	{
		ActionMap::const_iterator it = mActions.find(name);
		
		if (it != mActions.end())
		{
            OIS_EXCEPT(OIS::E_Duplicate, String("Action '" + name + "' already exists in ActionSchema '" + getName() + "'").c_str());
		}

		Action* ret = new TriggerAction(this, name);
        switch (type)
        {
        case AT_TRIGGER:
            ret = new TriggerAction(this, name);
            break;
        case AT_SEQUENCE:
            ret = new SequenceAction(this, name);
            break;

        case AT_ANALOG_AXIS:
            ret = new AnalogAxisAction(this, name);
            break;

        default:
            assert(0);
            break;
        }

		mActions.insert(std::make_pair(name, ret));
		return ret;
	}

	void ActionSchema::destroyAction(const String& name)
	{
		ActionMap::iterator it = mActions.find(name);
		
		if (it == mActions.end())
		{
            OIS_EXCEPT(OIS::E_General, String("Action '" + name + "'not found in ActionSchema '" + getName() + "'").c_str());
		}
		
		mActions.erase(it);
	}
	
	void ActionSchema::destroyAction(Action* action)
	{
		destroyAction(action->getName());
	}

    Action* ActionSchema::getAction(const String& name) const
    {
        ActionMap::const_iterator it = mActions.find(name);
		
		if (it == mActions.end())
		{
            OIS_EXCEPT(OIS::E_General, String("Action '" + name + "'not found in ActionSchema '" + getName() + "'").c_str());
		}
		
		return it->second;
    }

    bool ActionSchema::hasAction(const String& name) const
    {
        ActionMap::const_iterator it = mActions.find(name);
		
		return it != mActions.end();
    }

    void ActionSchema::addListenerToAllActions(BindableListener* listener)
    {
        for (ActionMap::const_iterator it = mActions.begin(); it != mActions.end(); ++it)
		{
			it->second->addListener(listener);
		}
    }

    void ActionSchema::removeListenerFromAllActions(BindableListener* listener)
    {
        for (ActionMap::const_iterator it = mActions.begin(); it != mActions.end(); ++it)
		{
			it->second->removeListener(listener);
		}
    }

	void ActionSchema::process(Real delta)
	{
		for (ActionMap::const_iterator it = mActions.begin(); it != mActions.end(); ++it)
		{
			it->second->process(delta);
		}
	}

	void ActionSchema::dump()
	{
	    std::cout << "** Action schema: '" << mName << "'" << std::endl;

	    for (ActionMap::const_iterator it = mActions.begin(); it != mActions.end(); ++it)
        {
	        // TODO: dump action type too
            std::cout << "** - Action: " << it->second->getName() << std::endl;
        }

	    std::cout << "** End of action schema '" << mName << "'" << std::endl;
	}
}
