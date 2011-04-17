
# Allow disabling editor building
option(BUILD_EDITOR           "Build the track editor."    ON)
# Allow disabling master server building
option(BUILD_MASTER_SERVER    "Build the master server."    OFF)

# Linux prefers lower-case exe names
if(WIN32 OR APPLE)
	set(GAME_EXE StuntRally)
	set(EDITOR_EXE SR-Editor)
	set(SERVER_EXE SR-MasterServer)
else()
	set(GAME_EXE stuntrally)
	set(EDITOR_EXE sr-editor)
	set(SERVER_EXE sr-masterserver)
endif()

# Search include files from all source sub directories
include_directories(btOgre bullet ogre paged-geom road tinyxml vdrift)

# Compile our libraries
foreach(ourlib   tinyxml bullet oisb paged-geom)
	add_subdirectory(${ourlib})
	list(APPEND OURLIBS ${ourlib})
endforeach()

# Glob .cpp files needed for the game
file(GLOB GAME_SOURCES
	vdrift/*.cpp
	btOgre/*.cpp
	ogre/*.cpp
	ogre/*/*.cpp
	road/*.cpp
	network/*.cpp
)

# Glob .cpp files needed for the editor
file(GLOB EDITOR_SOURCES
	editor/*.cpp
	vdrift/configfile.cpp
	vdrift/pathmanager.cpp
	ogre/QTimer.cpp
	ogre/common/*.cpp
	road/*.cpp
)

# Game executable
add_executable(${GAME_EXE} ${GAME_SOURCES})
target_link_libraries(${GAME_EXE} ${OURLIBS} ${LIBS})
install(TARGETS ${GAME_EXE} DESTINATION bin)

# Editor
if(BUILD_EDITOR)
	add_executable(${EDITOR_EXE} ${EDITOR_SOURCES})
	set_target_properties(${EDITOR_EXE} PROPERTIES COMPILE_FLAGS "-DROAD_EDITOR")
	target_link_libraries(${EDITOR_EXE} ${OURLIBS} ${LIBS})
	install(TARGETS ${EDITOR_EXE} DESTINATION bin)
endif()

# Master server
if(BUILD_MASTER_SERVER)
	file(GLOB SERVER_SOURCES network/master-server/*.cpp) # Glob .cpp files needed for the server
	add_executable(${SERVER_EXE} ${SERVER_SOURCES})
	target_link_libraries(${SERVER_EXE} ${SERVER_LIBS})
	install(TARGETS ${SERVER_EXE} DESTINATION bin)
endif()

