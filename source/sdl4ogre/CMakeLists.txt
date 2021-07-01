add_library(sdl4ogre STATIC
	sdlinputwrapper.cpp
	sdlcursormanager.cpp
	sdlwindowhelper.cpp

	OISCompat.h
	cursormanager.hpp)

target_link_libraries(sdl4ogre PRIVATE common)
