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

#ifndef __OISB_DIGITAL_STATE_H__
#define __OISB_DIGITAL_STATE_H__

#include "OISBGlobal.h"
#include "OISBState.h"

namespace OISB
{
	/**
	 * @brief represents digital input state
	 */
	class _OISBExport DigitalState : public State
	{
		public:
			/**
			 * @brief constructor
			 * 
			 * @param parent parent input device
			 * @param name name / identifier
			 */
			DigitalState(Device* parent, const String& name);
			
			/**
			 * @brief destructor
			 */
			virtual ~DigitalState();
			
			/**
			 * @copydoc State::geStateType
			 */
			virtual StateType getStateType() const;
			
			/**
			 * @brief sets new value of this input state
			 * 
			 * @param value new value to set
			 */
			void _setValue(bool value);

            //void _setScanCode(OIS::KeyCode code);

            /*inline OIS::KeyCode getScanCode() const
            {
                return mScanCode;
            }

            //void _setCharCode(unsigned int code);

            inline unsigned int getCharCode() const
            {
                return mCharCode;
            }*/

        protected:
            //OIS::KeyCode mScanCode;
            //unsigned int mCharCode;
	};
}

#endif
