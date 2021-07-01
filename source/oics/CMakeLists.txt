add_library(oics STATIC
	ICSChannel.cpp
	ICSControl.cpp
	ICSInputControlSystem.cpp
	ICSInputControlSystem_keyboard.cpp
	ICSInputControlSystem_mouse.cpp
	ICSInputControlSystem_joystick.cpp)

target_link_libraries(oics PRIVATE common)
