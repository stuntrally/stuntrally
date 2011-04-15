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

#include "OISBTriggerAction.h"
#include "OISBBinding.h"

namespace OISB
{
    ActionType TriggerAction::ACTION_TYPE = AT_TRIGGER;

	TriggerAction::TriggerAction(ActionSchema* parent, const String& name):
		Action(parent, name)
	{}
	
	TriggerAction::~TriggerAction()
	{}
	
	ActionType TriggerAction::getActionType() const
    {
        return AT_TRIGGER;
    }
	
    bool TriggerAction::impl_process(Real /*delta*/)
    {
        bool ret = false;

        for (BindingList::const_iterator it = mBindings.begin(); it != mBindings.end(); ++it)
        {
            Binding* binding = *it;

            if (binding->areAllBindablesActive())
            {
                binding->_setActive(true);
                ret = true;
            }

            binding->_setActive(false);
        }

        return ret;
    }
}
