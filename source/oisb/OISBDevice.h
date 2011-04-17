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

#ifndef __OISB_DEVICE_H__
#define __OISB_DEVICE_H__

#include "OISBGlobal.h"
#include "OISBString.h"

#include <map>

namespace OISB
{
	/**
	 * @brief any device that is able to interact with the user (keyboard, mouse, joystick, touchscreen, ...)
	 */
	class _OISBExport Device
	{
		public:
			/**
			 * @brief constructor
			 */
			Device();
			
			/**
			 * @brief destructor
			 */
			virtual ~Device();
			
			/**
			 * @brief retrieves name of this device
			 * 
			 * @return name / identifier of this input device
			 */
			virtual const String& getName() const = 0;
			
			/**
			 * @brief retrieves input state of given name
			 * 
			 * @param name name to lookup
			 * @return pointer to the resulting state
			 */
			State* getState(const String& name) const;

            /**
			 * @brief check whether this device has a state with given name
			 * 
			 * @param name name to lookup
			 * @return true if the device has the given state
			 */
            bool hasState(const String& name) const;
			
            /**
             * @brief adds listener to all states within this device
             */
            void addListenerToAllStates(BindableListener* listener);

            /**
             * @brief removes previously added listener from all states within this device
             */
            void removeListenerFromAllStates(BindableListener* listener);

			/**
			 * @brief updates all input states of this device
			 */
			virtual void process(Real delta) = 0;
			
			/**
			 * @brief a method to ease debugging, dumps all this device to stdout
			 */
			void dump();

		protected:
			/**
			 * @brief adds input state
			 * 
			 * @param state state to add
			 */
			void addState(State* state);
			
			/**
			 * @brief removes previously added input state
			 * 
			 * @param state state to remove
			 */
			void removeState(State* state);
			
			typedef std::map<String, State*> StateMap;
			/// stores input states by name hash
			StateMap mStates;
	};
}

#endif
