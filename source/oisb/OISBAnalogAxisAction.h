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

#ifndef __OISB_ANALOG_AXIS_ACTION_H__
#define __OISB_ANALOG_AXIS_ACTION_H__

#include "OISBGlobal.h"
#include "OISBAction.h"

namespace OISB
{
	/**
	 * @brief all of the states in at least one binding trigger this action
	 */
	class _OISBExport AnalogAxisAction : public Action
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
			AnalogAxisAction(ActionSchema* parent, const String& name);
			
			/**
			 * @brief destructor
			 */
			virtual ~AnalogAxisAction();

            /// @copydoc Action::getActionType
            virtual ActionType getActionType() const;
            
            void setUseAbsoluteValues(bool use);

            inline bool getUseAbsoluteValues() const
            {
                return mUseAbsoluteValues;
            }

            inline Real getAbsoluteValue() const
            {
                return mAbsoluteValue;
            }

            inline Real getRelativeValue() const
            {
                return mRelativeValue;
            }

            void setMinimumValue(Real min);

            inline Real getMinimumValue() const
            {
                return mMinimumValue;
            }

            void setMaximumValue(Real max);

            inline Real getMaximumValue() const
            {
                return mMaximumValue;
            }

            void setSensitivity(Real sensitivity);

            inline Real getSensitivity() const
            {
                return mSensitivity;
            }

            void setAnalogEmulator(AnalogEmulator* emulator);

            inline AnalogEmulator* getAnalogEmulator() const
            {
                return mAnalogEmulator;
            }

            /// @copydoc PropertySet::listProperties
            virtual void listProperties(PropertyList& list);
        
        protected:
            /// @copydoc PropertySet::impl_setProperty
            virtual void impl_setProperty(const String& name, const String& value);

            /// @copydoc PropertySet::getProperty
            virtual String impl_getProperty(const String& name) const;

            /// @copydoc Action::impl_process
            virtual bool impl_process(Real delta);

        private:
            /// if true, this action takes the absolute values of the analog states (if any)
            bool mUseAbsoluteValues;

            /// stores current value of this action
			Real mAbsoluteValue;
			/// delta since last processing
			Real mRelativeValue;
			
			/// calibration clamping minimum value
			Real mMinimumValue;
			/// calibration clamping maximum value
			Real mMaximumValue;
			/// calibration sensitivity
			Real mSensitivity;

            /// class providing analog emulation, if 0, emulation is disabled
            AnalogEmulator* mAnalogEmulator;
	};
}

#endif
