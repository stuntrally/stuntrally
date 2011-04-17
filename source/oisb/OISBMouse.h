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

#ifndef __OISB_MOUSE_H__
#define __OISB_MOUSE_H__

#include "OISBGlobal.h"
#include "OISBDevice.h"

namespace OISB
{
    /**
     * @brief this class wraps OIS' mouse
     *
     * @note you don't need to use this directly!
     */
	class _OISBExport Mouse : public Device
	{
		public:
			Mouse(OIS::Mouse* mouse);
			
			virtual ~Mouse();

			virtual const String& getName() const;

			virtual void process(Real delta);

            virtual void notifyScreenResize(Real width, Real height);

		private:
            /// OIS' implementation mouse
			OIS::Mouse* mMouse;

			AnalogAxisState* mXAxis;
			AnalogAxisState* mYAxis;
			AnalogAxisState* mWheel;

			DigitalState* mLeftButton;
			DigitalState* mRightButton;
			DigitalState* mMiddleButton;

			DigitalState* mButton3;
			DigitalState* mButton4;
			DigitalState* mButton5;
			DigitalState* mButton6;
			DigitalState* mButton7;
	};
}

#endif
