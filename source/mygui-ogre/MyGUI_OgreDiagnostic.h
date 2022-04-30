/*!
	@file
	@author		Albert Semenov
	@date		06/2009
*/

#ifndef MYGUI_OGRE_DIAGNOSTIC_H_
#define MYGUI_OGRE_DIAGNOSTIC_H_

#include "MyGUI_Prerequest.h"

#define MYGUI_PLATFORM_LOG_SECTION "Platform"
#define MYGUI_PLATFORM_LOG_FILENAME "MyGUI.log"
#define MYGUI_PLATFORM_LOG(level, text) MYGUI_LOGGING(MYGUI_PLATFORM_LOG_SECTION, level, text)

#define MYGUI_PLATFORM_EXCEPT(dest) \
do { \
	MYGUI_PLATFORM_LOG(Critical, dest); \
	std::ostringstream stream; \
	stream << dest << "\n"; \
	MYGUI_BASE_EXCEPT(stream.str().c_str(), "MyGUI"); \
} while (false)

#define MYGUI_PLATFORM_ASSERT(exp, dest) \
do { \
	if ( ! (exp) ) \
	{ \
		MYGUI_PLATFORM_LOG(Critical, dest); \
		std::ostringstream stream; \
		stream << dest << "\n"; \
		MYGUI_BASE_EXCEPT(stream.str().c_str(), "MyGUI"); \
	} \
} while (false)

#endif // MYGUI_OGRE_DIAGNOSTIC_H_
