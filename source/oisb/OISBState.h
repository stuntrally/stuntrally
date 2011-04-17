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

#ifndef __OISB_STATE_H__
#define __OISB_STATE_H__

#include "OISBGlobal.h"
#include "OISBBindable.h"

namespace OISB
{
	/**
	 * @brief enumerates possible types of input
	 */
	enum StateType
	{
		/// analog axis - characterized by floating point number, describing axis
		ST_ANALOG_AXIS,
		/// digital - characterized by one boolean - true or false
		ST_DIGITAL
	};
	
	/**
	 * @brief state of one separable input on input device
	 */
	class _OISBExport State : public Bindable
	{
		public:
			/**
			 * @brief constructor
			 * 
			 * @param parent parent input device
			 * @param name name / identifier
			 */
			State(Device* parent, const String& name);
			
			/**
			 * @brief destructor
			 */
			virtual ~State();

            /// @copydoc Bindable::getBindableType
            virtual BindableType getBindableType() const;

            /// @copydoc Bindable::getBindableName
            virtual String getBindableName() const;

            /// @copydoc Bindable::isActive
            virtual bool isActive() const;

            /// @copydoc Bindable::hasChanged
            virtual bool hasChanged() const;
			
			/**
			 * @brief retrieves parent input device
			 * 
			 * @return pointer to parent input device
			 */
			inline Device* getParent() const
			{
				return mParent;
			}
			
			/**
			 * @brief retrieves name of this input state
			 * 
			 * @return name / identifier of this input state
			 */
			inline const String& getName() const
			{
				return mName;
			}

			/**
			 * @brief retrieves full name of this input state (including parent device)
			 * 
			 * @return full name of this input state
			 */
			String getFullName() const;
			
			/**
			 * @brief retrieves type of input
			 * 
			 * @return input type
			 */
			virtual StateType getStateType() const = 0;

            /// @copydoc PropertySet::listProperties
            virtual void listProperties(PropertyList& list);

        protected:
            /// @copydoc PropertySet::impl_setProperty
            virtual void impl_setProperty(const String& name, const String& value);

            /// @copydoc PropertySet::impl_getProperty
            virtual String impl_getProperty(const String& name) const;
			
            /// internal method, sets mIsActive to true and notifies listeners
            void activate();
            /// internal method, sets mIsActive to false and notifies listeners
            void deactivate();

			/// stores parent input device
			Device* mParent;
			/// name / identifier of this input state
			const String mName;
            
            /// if true, this action is active
            bool mIsActive;

            /// if true, the active state has changed
            bool mChanged;
	};
}

#endif
