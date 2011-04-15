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

#include "OISBAction.h"
#include "OISBActionSchema.h"
#include "OISBBinding.h"

#include "OISException.h"

#include <algorithm>

namespace OISB
{
	Action::Action(ActionSchema* parent, const String& name):
		mParent(parent),
		mName(name),
		
		mIsActive(false)
	{}
	
	Action::~Action()
	{
        for (BindingList::const_iterator it = mBindings.begin(); it != mBindings.end(); ++it)
        {
            Binding* binding = *it;
            delete binding;
        }

        mBindings.clear();
    }

    BindableType Action::getBindableType() const
    {
        return BT_ACTION;
    }

    String Action::getBindableName() const
    {
        return "Action: " + getFullName();
    }

    bool Action::isActive() const
    {
        return mIsActive;
    }

    bool Action::hasChanged() const
    {
        return mChanged;
    }
	
	String Action::getFullName() const
	{
		return mParent->getName() + "/" + getName();
	}

    Binding* Action::createBinding()
    {
        Binding* ret = new Binding(this);
        mBindings.push_back(ret);

        return ret;
    }

    void Action::destroyBinding(Binding* binding)
    {
        BindingList::iterator it = std::find(mBindings.begin(), mBindings.end(), binding);

        if (it == mBindings.end())
        {
             OIS_EXCEPT(OIS::E_General, String("Given binding not found").c_str());
        }

        mBindings.erase(it);
    }

    void Action::bind(Bindable* bindable)
    {
        Binding* b = createBinding();

        b->bind(bindable);
    }

    void Action::bind(Bindable* bindable1, Bindable* bindable2)
    {
        Binding* b = createBinding();

        b->bind(bindable1);
        b->bind(bindable2);
    }

    void Action::bind(Bindable* bindable1, Bindable* bindable2, Bindable* bindable3)
    {
        Binding* b = createBinding();

        b->bind(bindable1);
        b->bind(bindable2);
        b->bind(bindable3);
    }

    void Action::bind(const String& bindable)
    {
        Binding* b = createBinding();

        b->bind(bindable);
    }

    void Action::bind(const String& bindable1, const String& bindable2)
    {
        Binding* b = createBinding();

        b->bind(bindable1);
        b->bind(bindable2);
    }

    void Action::bind(const String& bindable1, const String& bindable2, const String& bindable3)
    {
        Binding* b = createBinding();

        b->bind(bindable1);
        b->bind(bindable2);
        b->bind(bindable3);
    }

    void Action::process(Real delta)
    {
        const bool oldIsActive = mIsActive;

        if (!mParent->isEnabled())
        {
            if (isActive())
            {
                deactivate();
            }

            // nothing to do here, parent action schema is disabled
        }
        else
        {

            const bool state = impl_process(delta);
            if (state && !isActive())
            {
                activate();
            }
            else if (!state && isActive())
            {
                deactivate();
            }
        }

        mChanged = oldIsActive != mIsActive;

        notifyProcessed();
    }
    
    void Action::destroyItself()
	{
		getParent()->destroyAction(this);
	}

    void Action::listProperties(PropertyList& list)
    {
        Bindable::listProperties(list);

        list.push_back("ActionName");
        list.push_back("ParentActionSchemaName");
    }

    void Action::impl_setProperty(const String& name, const String& value)
    {
        if (name == "ActionName")
        {
            OIS_EXCEPT(OIS::E_InvalidParam, "'ActionName' is a read only, you can't set it!");
        }
        else if (name == "ParentActionSchemaName")
        {
            OIS_EXCEPT(OIS::E_InvalidParam, "'ParentActionSchemaName' is a read only, you can't set it!");
        }
        else
        {
            // nothing matched, delegate up
            Bindable::impl_setProperty(name, value);
        }
    }

    String Action::impl_getProperty(const String& name) const
    {
        if (name == "ActionName")
        {
            return getName();
        }
        else if (name == "ParentActionSchemaName")
        {
            // no need to check, every action must have a valid parent
            return mParent->getName();
        }
        else
        {
            // nothing matched, delegate up
            return Bindable::impl_getProperty(name);
        }
    }

    void Action::activate()
    {
        mIsActive = true;

        notifyActivated();
    }

    void Action::deactivate()
    {
        mIsActive = false;

        notifyDeactivated();
    }
}
