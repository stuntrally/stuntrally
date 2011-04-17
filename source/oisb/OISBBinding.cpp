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

#include "OISBBinding.h"
#include "OISBAction.h"
#include "OISBState.h"
#include "OISBSystem.h"

#include "OISException.h"

#include <cassert>

namespace OISB
{
	Binding::Binding(Action* parent):
		mParent(parent)
	{}
	
	Binding::~Binding()
	{
        mBindables.clear();
    }

    void Binding::_setActive(bool active)
    {
        mIsActive = active;
    }
	
	void Binding::bind(Bindable* bindable, const String& role)
    {
        if (isBound(bindable))
        {
            OIS_EXCEPT(OIS::E_Duplicate, String("Binding of action '" + mParent->getFullName() + "' already contains bindable '" + bindable->getBindableName() + "'").c_str());
        }

        mBindables.push_back(std::make_pair(role, bindable));
    }

    void Binding::bind(const String& bindable, const String& role)
    {
        Bindable* b = System::getSingleton().lookupBindable(bindable);

        if (!b)
        {
            OIS_EXCEPT(OIS::E_General, String("Lookup of bindable '" + bindable + "' failed").c_str());
        }

        bind(b, role);
    }

    void Binding::unbind(Bindable* bindable)
    {
        for (BindableList::iterator it = mBindables.begin(); it != mBindables.end(); ++it)
        {
            if (it->second == bindable)
            {
                mBindables.erase(it);
                return;
            }
        }

        OIS_EXCEPT(OIS::E_General, String("Binding of action '" + mParent->getFullName() + "' doesn't contain bindable '" + bindable->getBindableName() + "'").c_str());
    }

    void Binding::unbind(const String& bindable)
    {
        Bindable* b = System::getSingleton().lookupBindable(bindable);

        if (!b)
        {
            OIS_EXCEPT(OIS::E_General, String("Lookup of bindable '" + bindable + "' failed").c_str());
        }

        unbind(b);
    }

    bool Binding::isBound(Bindable* bindable) const
    {
        for (BindableList::const_iterator it = mBindables.begin(); it != mBindables.end(); ++it)
        {
            if (it->second == bindable)
            {
                return true;
            }
        }

        return false;
    }

    bool Binding::isBound(const String& role) const
    {
        for (BindableList::const_iterator it = mBindables.begin(); it != mBindables.end(); ++it)
        {
            if (it->first == role)
            {
                return true;
            }
        }

        return false;
    }

    Bindable* Binding::getBindable(size_t idx) const
    {
        if (idx >= mBindables.size())
        {
            OIS_EXCEPT(OIS::E_General, "Out of bounds");
        }

        BindableList::const_iterator it = mBindables.begin();
        std::advance(it, idx);
        
        return it->second;
    }

    State* Binding::getState(size_t idx) const
    {
        Bindable* ret = getBindable(idx);
        if (ret->getBindableType() != BT_STATE)
        {
            OIS_EXCEPT(OIS::E_General, "Bindable at this index isn't a state!");
        }

        return static_cast<State*>(ret);
    }

    Action* Binding::getAction(size_t idx) const
    {
        Bindable* ret = getBindable(idx);
        if (ret->getBindableType() != BT_ACTION)
        {
            OIS_EXCEPT(OIS::E_General, "Bindable at this index isn't an action!");
        }

        return static_cast<Action*>(ret);
    }

    Bindable* Binding::getBindable(const String& role) const
    {
        for (BindableList::const_iterator it = mBindables.begin(); it != mBindables.end(); ++it)
        {
            if (it->first == role)
            {
                return it->second;
            }
        }

        OIS_EXCEPT(OIS::E_General, String("Binding of action '" + mParent->getFullName() + "' doesn't contain any bindable of role '" + role + "'").c_str());
    }

    State* Binding::getState(const String& role) const
    {
        Bindable* ret = getBindable(role);
        if (ret->getBindableType() != BT_STATE)
        {
            OIS_EXCEPT(OIS::E_General, "Bindable with such a role isn't a state!");
        }

        return static_cast<State*>(ret);
    }

    Action* Binding::getAction(const String& role) const
    {
        Bindable* ret = getBindable(role);
        if (ret->getBindableType() != BT_ACTION)
        {
            OIS_EXCEPT(OIS::E_General, "Bindable with such a role isn't an action!");
        }

        return static_cast<Action*>(ret);
    }

    void Binding::getBindables(const String& role, std::list<Bindable*>& target)
    {
        // todo: this could use some optimalization

        for (BindableList::const_iterator it = mBindables.begin(); it != mBindables.end(); ++it)
        {
            if (it->first == role)
            {
                target.push_back(it->second);
            }
        }
    }

    bool Binding::isAnyBindableActive() const
    {
        for (BindableList::const_iterator it = mBindables.begin(); it != mBindables.end(); ++it)
        {
            Bindable* bindable = it->second;
            
            if (bindable->isActive())
            {
                return true;
            }
        }

        return false;
    }

    bool Binding::areAllBindablesActive() const
    {
        for (BindableList::const_iterator it = mBindables.begin(); it != mBindables.end(); ++it)
        {
            Bindable* bindable = it->second;
            
            if (!bindable->isActive())
            {
                return false;
            }
        }

        return true;
    }
}
