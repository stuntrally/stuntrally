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

#ifndef __OISB_TRIGGER_ACTION_H__
#define __OISB_TRIGGER_ACTION_H__

#include "OISBGlobal.h"
#include "OISBAction.h"

namespace OISB
{
	/**
	 * @brief all of the states in at least one binding trigger this action
	 */
	class _OISBExport TriggerAction : public Action
	{
		public:
            /// static action type (for use in templates)
            static ActionType ACTION_TYPE;

			/**
			 * @brief constructor
			 * 
			 * @param parent parent action schema
			 * @param name name / identifier
			 */
			TriggerAction(ActionSchema* parent, const String& name);
			
			/**
			 * @brief destructor
			 */
			virtual ~TriggerAction();

            /// @copydoc Action::getActionType
            virtual ActionType getActionType() const;

            bool getValue() const;

        protected:
            /// @copydoc Action::impl_process
            virtual bool impl_process(Real delta);

        private:
            bool mValue;
	};
}

#endif
