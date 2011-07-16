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

#include <boost/algorithm/string/predicate.hpp> // String starts_with
#include <cassert>

#if 1
	#include <OgreLogManager.h>
	#define LogExc(err,str)  {  Ogre::LogManager::getSingletonPtr()->logMessage( String("#### OISB:") + str );  }
#else
	#define LogExc(err,str)  {  OIS_EXCEPT( err, str );  }
#endif

//  (Bindable*)1 is a fake bindable value, it stays in xml


namespace OISB
{
	Binding::Binding(Action* parent):
		mParent(parent), mOptional(false)
	{}
	
	Binding::~Binding()
	{
        mBindables.clear();
    }

    void Binding::_setActive(bool active)
    {
        mIsActive = active;
    }
	
	void Binding::bind(Bindable* bindable, const String& role, const String& role2)
    {
		if (!bindable)
		{
            LogExc(OIS::E_Duplicate, String("NULL Binding of action '" + mParent->getFullName()).c_str());
			mBindables.push_back(std::make_pair(role2, bindable));
			return;
		}
    
        if (isBound(bindable))
        {
            LogExc(OIS::E_Duplicate, String("Binding of action '" + mParent->getFullName() + "' already contains bindable '" + (!bindable ? "" : bindable->getBindableName()) + "'").c_str());
        }
		else
		{
			mBindables.push_back(std::make_pair(role, bindable));
		}
    }

    void Binding::bind(const String& bindable, const String& role)
    {
        Bindable* b = System::getSingleton().lookupBindable(bindable);

		if (!b)
			mBindables.push_back(std::make_pair(role, (Bindable*)1));  // fake bindable, stays in xml
		else
			mBindables.push_back(std::make_pair(role, b));

        /*if (b)
			bind(b, role, "");
		else
		{
			bind(NULL, bindable, role);  // newer throw
			// dummy bind...
			//if (mOptional)
			//	bind(NULL, bindable);
			//else
			//	LogExc(OIS::E_General, String("Lookup of bindable '" + bindable + "' failed").c_str());
		}*/
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

        LogExc(OIS::E_General, String("Binding of action '" + mParent->getFullName() + "' doesn't contain bindable '" + bindable->getBindableName() + "'").c_str());
    }

    void Binding::unbind(const String& bindable)
    {
        Bindable* b = System::getSingleton().lookupBindable(bindable);

        if (!b)
        {
            LogExc(OIS::E_General, String("Lookup of bindable '" + bindable + "' failed").c_str());
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
            LogExc(OIS::E_General, "Out of bounds");
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
            LogExc(OIS::E_General, "Bindable at this index isn't a state!");
        }

        return static_cast<State*>(ret);
    }

    Action* Binding::getAction(size_t idx) const
    {
        Bindable* ret = getBindable(idx);
        if (ret->getBindableType() != BT_ACTION)
        {
            LogExc(OIS::E_General, "Bindable at this index isn't an action!");
        }

        return static_cast<Action*>(ret);
    }
    
	String Binding::getRole(Bindable* bindable) const
	{
        for (BindableList::const_iterator it = mBindables.begin(); it != mBindables.end(); ++it)
        {
            if (it->second == bindable)
            {
                return it->first;
            }
        }
        return "";
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

        LogExc(OIS::E_General, String("Binding of action '" + mParent->getFullName() + "' doesn't contain any bindable of role '" + role + "'").c_str());
		return 0;
    }

    State* Binding::getState(const String& role) const
    {
        Bindable* ret = getBindable(role);
        if (ret->getBindableType() != BT_STATE)
        {
            LogExc(OIS::E_General, "Bindable with such a role isn't a state!");
        }

        return static_cast<State*>(ret);
    }

    Action* Binding::getAction(const String& role) const
    {
        Bindable* ret = getBindable(role);
        if (ret->getBindableType() != BT_ACTION)
        {
            LogExc(OIS::E_General, "Bindable with such a role isn't an action!");
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
		if (mBindables.size() == 0) return false;
        for (BindableList::const_iterator it = mBindables.begin(); it != mBindables.end(); ++it)
        {
            Bindable* bindable = it->second;
            
            if (bindable && bindable->isActive())
            {
                return true;
            }
        }

        return false;
    }

    bool Binding::areAllBindablesActive() const
    {
		if (mBindables.size() == 0) return false;
        for (BindableList::const_iterator it = mBindables.begin(); it != mBindables.end(); ++it)
        {
            Bindable* bindable = it->second;
            
            if (!bindable || bindable == (Bindable*)1 || !bindable->isActive())
            {
                return false;
            }
        }

        return true;
    }
}
