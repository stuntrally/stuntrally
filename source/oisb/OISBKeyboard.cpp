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

#include "OISBKeyboard.h"
#include "OISBDigitalState.h"
#include "OISException.h"
#include "OISKeyboard.h"

#include <sstream>

namespace OISB
{
	Keyboard::Keyboard(OIS::Keyboard* keyboard):
		mKeyboard(keyboard)
	{
		for (unsigned int i = 0; i < 256; ++i)
		{
			String name = mKeyboard->getAsString(static_cast<OIS::KeyCode>(i));
			if (name == "")
			{
				name = "Unknown";
			}

            if (!hasState(name))
			{
				mKeys[i] = new DigitalState(this, name);
                //mKeys[i]->_setScanCode(static_cast<OIS::KeyCode>(i));
                //mKeys[i]->_setCharCode(
				addState(mKeys[i]);
			}
            else
			{
                std::stringstream s;

                s << i;
                String i_str = s.str();

				name += " (" + i_str + ")";
				mKeys[i] = new DigitalState(this, name);
				addState(mKeys[i]);
			}
		}
	}
			
	Keyboard::~Keyboard()
	{
		for (unsigned int i = 0; i < 256; ++i)
		{
			removeState(mKeys[i]);
			delete mKeys[i];
			mKeys[i] = 0;
		}
	}

	const String& Keyboard::getName() const
	{
		static String name = "Keyboard";
		return name;
	}

	void Keyboard::process(Real delta)
	{
		for (unsigned int i = 0; i < 256; ++i)
		{
			mKeys[i]->_setValue(mKeyboard->isKeyDown(static_cast<OIS::KeyCode>(i)));
		}
	}
}
