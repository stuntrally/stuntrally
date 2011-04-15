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

#ifndef __OISB_JOYSTICK_H__
#define __OISB_JOYSTICK_H__

#include "OISBGlobal.h"
#include "OISBDevice.h"

#include <vector>

namespace OISB
{
    /**
     * @brief this class wraps OIS' JoyStick
     *
     * @note you don't need to use this directly!
     */
	class _OISBExport JoyStick : public Device
	{
		public:
			JoyStick(OIS::JoyStick* joy);
			
			virtual ~JoyStick();

			virtual const String& getName() const;

			virtual void process(Real delta);


		private:
            /// OIS' implementation JoyStick
			OIS::JoyStick* mJoyStick;

			std::vector<AnalogAxisState*> axis;
			std::vector<DigitalState*>    buttons;
	};
}

#endif // __OISB_JOYSTICK_H__
