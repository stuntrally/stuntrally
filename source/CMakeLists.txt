cmake_minimum_required(VERSION 2.8)

# Allow disabling game building
option(BUILD_GAME      "Build the game binary."     ON)
# Allow disabling editor building
option(BUILD_EDITOR    "Build the track editor."    ON)

# Linux prefers lower-case exe names
if(WIN32 OR APPLE)
	set(GAME_EXE StuntRally)
	set(EDITOR_EXE SR-Editor)
else()
	set(GAME_EXE stuntrally)
	set(EDITOR_EXE sr-editor)
endif()

# Search include files from all source sub directories
include_directories(btOgre bullet ogre paged-geom road tinyxml vdrift)

# Compile our libraries
foreach(ourlib   tinyxml bullet oisb paged-geom)
	add_subdirectory(${ourlib})
	list(APPEND OURLIBS ${ourlib})
endforeach()

# Game executable
if(BUILD_GAME)
	# Glob .cpp files needed for the game
	file(GLOB GAME_SOURCES
		vdrift/*.cpp
		btOgre/*.cpp
		ogre/*.cpp
		ogre/*/*.cpp
		road/*.cpp
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
		road/*.cpp
	)

	add_executable(${EDITOR_EXE} ${EDITOR_SOURCES})
	set_target_properties(${EDITOR_EXE} PROPERTIES COMPILE_FLAGS "-DROAD_EDITOR")
	target_link_libraries(${EDITOR_EXE} ${OURLIBS} ${LIBS})
	install(TARGETS ${EDITOR_EXE} DESTINATION bin)
endif()

if (MSVC)
	file(GLOB GAME_SOURCES
		btOgre/*.cpp
		bullet/*.cpp
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

