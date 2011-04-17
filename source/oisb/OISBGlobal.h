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

#ifndef __OISB_GLOBAL_H__
#define __OISB_GLOBAL_H__

#include "OISPrereqs.h"

// Default is blank for most OS's
#define _OISBExport

//-------------- Determine Compiler ---------------------------------
#if defined( _MSC_VER )
#	define OISB_MSVC_COMPILER
#elif defined( __GNUC__ )
#	if defined( __WIN32__ ) || defined( _WIN32 )
#		define OISB_MINGW_COMPILER
#	else
#		define OISB_GCC_COMPILER
#	endif
#elif defined( __BORLANDC__ )
#	define OISB_BORLAND_COMPILER
#else
#	error No Recognized Compiler!
#endif

// --------------- Determine Operating System Platform ---------------
#if defined( __WIN32__ ) || defined( _WIN32 ) // Windows 2000, XP, ETC
#	if defined ( _XBOX )
#		define OISB_XBOX_PLATFORM
#	else
#		define OISB_WIN32_PLATFORM
#		ifndef OISB_STATIC_LIB
#			undef _OISBExport
			//Ignorable Dll interface warning...
#           if !defined(OISB_MINGW_COMPILER)
#			    pragma warning (disable : 4251)
#           endif
#			if defined( OISB_NONCLIENT_BUILD )
#				define _OISBExport __declspec( dllexport )
#			else
#               if defined(OISB_MINGW_COMPILER)
#                   define _OISBExport
#               else
#				    define _OISBExport __declspec( dllimport )
#               endif
#			endif
#		endif
#	endif
#elif defined( __APPLE_CC__ ) // Apple OS X
    // Device                                       Simulator
#   if __IPHONE_OS_VERSION_MIN_REQUIRED >= 20201 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 20000
//#   if __IPHONE_OS_VERSION_MIN_REQUIRED >= 30000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 30000
#       define OISB_IPHONE_PLATFORM
#   else
#       define OISB_APPLE_PLATFORM
#   endif
#   undef _OISBExport
#   define _OISBExport __attribute__((visibility("default")))
#else //Probably Linux
#	define OISB_LINUX_PLATFORM
#endif

namespace OISB
{
    typedef float Real;

    class Action;
    class ActionSchema;
    class AnalogAxisState;
    class AnalogAxisAction;
    class AnalogEmulator;
    class AnalogPlaneAction;
    class Bindable;
    class BindableListener;
    class Binding;
    class DebugBindableListener;
    class DigitalState;
    class Device;
    class Keyboard;
    class LinearAnalogEmulator;
    class Mouse;
    class JoyStick;
    class PropertySet;
    class SequenceAction;
    class System;
    class State;
    class TriggerAction;
}

#endif
