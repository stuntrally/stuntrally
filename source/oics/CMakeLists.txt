set(OICS_LIBRARY "oics")

# Sources

set(OICS_SOURCE_FILES
	ICSChannel.cpp
	ICSControl.cpp
	ICSInputControlSystem.cpp
	ICSInputControlSystem_keyboard.cpp
	ICSInputControlSystem_mouse.cpp
	ICSInputControlSystem_joystick.cpp
)

add_library(${OICS_LIBRARY} STATIC ${OICS_SOURCE_FILES})

target_link_libraries(${OICS_LIBRARY} ${LIBS})
