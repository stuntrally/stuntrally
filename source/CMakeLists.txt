cmake_minimum_required(VERSION 2.8)

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
include_directories(btOgre ogre ogre/common paged-geom road tinyxml vdrift bullet)

# Compile our libraries
if (BUILD_GAME OR BUILD_EDITOR)
	foreach(ourlib   tinyxml oisb paged-geom bullet)
		add_subdirectory(${ourlib})
		list(APPEND OURLIBS ${ourlib})
	endforeach()
endif()

# Game executable
if(BUILD_GAME)
	# Glob .cpp files needed for the game
	file(GLOB GAME_SOURCES
		vdrift/*.cpp
		btOgre/*.cpp
		ogre/*.cpp
		ogre/*/*.cpp
		ogre/*/*/*.cpp
		road/*.cpp
		network/*.cpp
		network/*.hpp # For some IDE project files
	)
	if (MSVC)
		add_executable(${GAME_EXE} WIN32 ${GAME_SOURCES})
	else (MSVC)
		add_executable(${GAME_EXE} ${GAME_SOURCES})
	endif(MSVC)
	target_link_libraries(${GAME_EXE} ${OURLIBS} ${LIBS})
	install(TARGETS ${GAME_EXE} DESTINATION bin)
endif()

# Editor
if(BUILD_EDITOR)
	# Glob .cpp files needed for the editor
	file(GLOB EDITOR_SOURCES
		editor/*.cpp
		vdrift/configfile.cpp
		vdrift/pathmanager.cpp
		ogre/QTimer.cpp
		ogre/common/*.cpp
		ogre/common/MaterialGen/*.cpp
		road/*.cpp
	)

	add_executable(${EDITOR_EXE} ${EDITOR_SOURCES})
	set_target_properties(${EDITOR_EXE} PROPERTIES COMPILE_FLAGS "-DROAD_EDITOR")
	target_link_libraries(${EDITOR_EXE} ${OURLIBS} ${LIBS})
	install(TARGETS ${EDITOR_EXE} DESTINATION bin)
endif()

# Master server
if(BUILD_MASTER_SERVER)
	file(GLOB SERVER_SOURCES network/master-server/main.cpp) # Glob .cpp files needed for the server
	add_executable(${SERVER_EXE} ${SERVER_SOURCES})
	target_link_libraries(${SERVER_EXE} ${SERVER_LIBS})
	install(TARGETS ${SERVER_EXE} DESTINATION bin)
endif()


if (MSVC)
	file(GLOB GAME_SOURCES
		btOgre/*.cpp
		editor/*.cpp
		ogre/*.cpp
		ogre/common/*.cpp
		oisb/*.cpp
		paged-geom/*.cpp
		road/*.cpp
		vdrift/*.cpp
	)

    foreach( src_file ${GAME_SOURCES} )
        set_source_files_properties(
            ${src_file}
            PROPERTIES
            COMPILE_FLAGS "/Yupch.h"
            )	
	endforeach( src_file ${GAME_SOURCES} )
	
    set_source_files_properties(editor/pch.cpp
        PROPERTIES
        COMPILE_FLAGS "/Ycpch.h"
        )
		
    set_source_files_properties(ogre/pch.cpp
        PROPERTIES
        COMPILE_FLAGS "/Ycpch.h"
        )

endif (MSVC)

