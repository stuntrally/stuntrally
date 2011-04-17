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

#include "OISBSequenceAction.h"
#include "OISBBinding.h"
#include "OISBState.h"

namespace OISB
{
    ActionType SequenceAction::ACTION_TYPE = AT_SEQUENCE;

	SequenceAction::SequenceAction(ActionSchema* parent, const String& name):
		Action(parent, name),

        mTimeout(1.0f)
	{}
	
	SequenceAction::~SequenceAction()
	{}
	
	ActionType SequenceAction::getActionType() const
    {
        return AT_SEQUENCE;
    }

    Binding* SequenceAction::createBinding()
    {
        Binding* ret = Action::createBinding();
        mBindingStates.push_back(BindingState());

        return ret;
    }
    
    void SequenceAction::destroyBinding(Binding* binding)
    {
        Action::destroyBinding(binding);

        // todo: for now this disrupts all bindings "in progress",
        //       it could be implemented smarter
        mBindingStates.clear();
        for (size_t i = 0; i < mBindings.size(); ++i)
        {
            mBindingStates.push_back(BindingState());
        }
    }
	
    bool SequenceAction::impl_process(Real delta)
    {
        bool ret = false;

        BindingList::const_iterator it = mBindings.begin();
        BindingStateList::iterator sit = mBindingStates.begin();

        while (it != mBindings.end() && sit != mBindingStates.end())
        {
            Binding* binding = *it;
            BindingState& bstate = *sit;

            if (binding->getNumBindables() == 0)
            {
                // it's empty, so the result is not active!
                ++it; ++sit;
                continue;
            }

            if (bstate.position > 0)
            {
                bstate.timeout -= delta;
            }

            if (bstate.timeout < 0)
            {
                bstate.position = 0;
                bstate.timeout = 0.0f;
            }

            Bindable* bindable = binding->getBindable(bstate.position);
            if (bindable->isActive())
            {
                ++bstate.position;
                bstate.timeout = mTimeout;
            }

            if (bstate.position >= binding->getNumBindables())
            {
                bstate.position = 0;
                bstate.timeout = 0.0f;

                ret = true;
                binding->_setActive(true);
            }
            else
            {
                binding->_setActive(false);
            }

            ++it; ++sit;
        }

        return ret;
    }
}
