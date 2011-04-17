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

#ifndef __OISB_ANALOG_EMULATION_H__
#define __OISB_ANALOG_EMULATION_H__

#include "OISBGlobal.h"
#include "OISBPropertySet.h"

namespace OISB
{
	/**
	 * @brief base abstract class for all classes providing analog emulation
	 */
	class _OISBExport AnalogEmulator : public PropertySet
	{
		public:
            /**
			 * @brief constructor
			 */
            AnalogEmulator();

			/**
			 * @brief destructor
			 */
			virtual ~AnalogEmulator();

            /**
             * @brief string describing the type of the emulator 
             */
            virtual const String& getType() const = 0;

            virtual void setTarget(AnalogAxisAction* target);

            inline AnalogAxisAction* getTarget() const
            {
                return mTarget;
            }

            /**
             * @brief checks whether this emulator will be able to emulate using given binding
             */
            virtual bool checkBinding(Binding* binding) = 0;

            /**
             * @brief performs the emulation and returns relative value
             */
            virtual Real emulateRelative(Binding* binding, Real delta) = 0;

            /**
             * @brief performs the emulation and returns absolute value
             */
            virtual Real emulateAbsolute(Binding* binding, Real delta) = 0;

        protected:
            AnalogAxisAction* mTarget;
	};

    class _OISBExport LinearAnalogEmulator : public AnalogEmulator
    {
        public:
            /**
             * @brief constructor
             */
            LinearAnalogEmulator();

            /**
			 * @brief destructor
			 */
            virtual ~LinearAnalogEmulator();

            /// @copydoc AnalogEmulator::getType
            virtual const String& getType() const;

            /// @copydoc AnalogEmulator::checkBinding
            virtual bool checkBinding(Binding* binding);

            /// @copydoc AnalogEmulator::emulateRelative
            virtual Real emulateRelative(Binding* binding, Real delta);

            /// @copydoc AnalogEmulator::emulateAbsolute
            virtual Real emulateAbsolute(Binding* binding, Real delta);

            /**
             * @brief sets the decrease speed (how much will the value drop when you activate the decrease bindable)
             */
            void setDecreaseSpeed(Real speed);

            /**
             * @brief gets the decrease speed
             */
            inline Real getDecreaseSpeed() const
            {
                return mDecreaseSpeed;
            }

            /**
             * @brief sets the increase speed (how much will the value rise when you activate the increase bindable)
             */
            void setIncreaseSpeed(Real speed);

            /**
             * @brief gets the increase speed
             */
            inline Real getIncreaseSpeed() const
            {
                return mIncreaseSpeed;
            }

            /**
             * @brief sets both increase and decrease speeds
             */
            void setSpeed(Real speed);

            /**
             * @brief enables/disables return (returning is when value starts to return back to the return value when nothing is pressed)
             */
            void setReturnEnabled(bool enabled);

            /**
             * @brief checks whether return is enabled
             */
            inline bool isReturnEnabled() const
            {
                return mReturnEnabled;
            }

            /**
             * @brief sets the return value (value that the action will return to when no keys are pressed)
             */
            void setReturnValue(Real value);
            
            /**
             * @brief gets the return value
             *
             * @see LinearAnalogEmulator::getReturnValue
             */
            inline Real getReturnValue() const
            {
                return mReturnValue;
            }
            
            /**
             * @brief sets the speed that the value drops when nothing is pressed and value is higher than return value
             */
            void setReturnDecreaseSpeed(Real speed);

            /**
             * @brief gets the return decrease speed
             */
            inline Real getReturnDecreaseSpeed() const
            {
                return mReturnDecreaseSpeed;
            }

            /**
             * @brief sets the speed that the value rises when nothing is pressed and value is lower than return value
             */
            void setReturnIncreaseSpeed(Real speed);

            /**
             * @brief gets the return increase speed
             */
            inline Real getReturnIncreaseSpeed() const
            {
                return mReturnIncreaseSpeed;
            }

            /**
             * @brief sets both increase and decrease return speeds
             */
            void setReturnSpeed(Real speed);

            /// @copydoc PropertySet::listProperties
            virtual void listProperties(PropertyList& list);

        protected:
            /// @copydoc PropertySet::impl_setProperty
            virtual void impl_setProperty(const String& name, const String& value);

            /// @copydoc PropertySet::impl_getProperty
            virtual String impl_getProperty(const String& name) const;
        
        private:
            /// internal method that picks up bindables from given binding
            void getBindables(Binding* binding, Bindable** decrease, Bindable** increase);

            /// speed of decrease
            Real mDecreaseSpeed;
            /// speed of increase
            Real mIncreaseSpeed;

            /// if true, the position returns to the return point if no buttons are pressed
            bool mReturnEnabled;
            /// only used when return is enabled, this is the point the value returns to if nothing is pressed
            Real mReturnValue;
            /// speed of the return from increase (decreasing return)
            Real mReturnDecreaseSpeed;
            /// speed of the return from decrease (increasing return)
            Real mReturnIncreaseSpeed;
    };
}

#endif
