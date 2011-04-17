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

#ifndef __OISB_ANALOG_AXIS_STATE_H__
#define __OISB_ANALOG_AXIS_STATE_H__

#include "OISBGlobal.h"
#include "OISBState.h"

namespace OISB
{
	/**
	 * @brief represents analog input state
	 */
	class _OISBExport AnalogAxisState : public State
	{
		public:
			/**
			 * @brief constructor
			 * 
			 * @param parent parent input device
			 * @param name name / identifier
			 */
			AnalogAxisState(Device* parent, const String& name);
			
			/**
			 * @brief destructor
			 */
			virtual ~AnalogAxisState();
			
			/**
			 * @copydoc State::getStateType
			 */
			virtual StateType getStateType() const;

            void _setMinimumValue(Real min);

            void _setMaximumValue(Real max);
			
			/**
			 * @brief sets value of this input state and perfoms normalization
			 * 
			 * @param value new value to set
			 */
			void _setAbsoluteValue(Real value);
			
			/**
			 * @brief sets value of this input state and performs normalization
			 * 
			 * @param delta delta value
			 */
			inline void _setRelativeValue(Real delta)
			{
				_setAbsoluteValue(mAbsoluteValue + delta);
			}
			
			/**
			 * @brief retrieves absolute value of this input state
			 * 
			 * @return absolute value of this input state
			 */
			inline Real getAbsoluteValue() const
			{
				return mAbsoluteValue;
			}
			
			/**
			 * @brief retrieves delta value of this input state
			 * 
			 * @return delta value (since last update) of this input state
			 */
			inline Real getRelativeValue() const
			{
				return mRelativeValue;
			}
			
		protected:
			/// stores current value of this state - should be updated in update() virtual method
			Real mAbsoluteValue;
			/// delta since last update
			Real mRelativeValue;
			
			/// calibration offset
			Real mOffset;
			/// calibration clamping minimum value
			Real mMinimumValue;
			/// calibration clamping maximum value
			Real mMaximumValue;
			/// calibration sensitivity
			Real mSensitivity;
	};
}

#endif
