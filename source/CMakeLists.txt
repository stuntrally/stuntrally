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
include_directories(btOgre ogre ogre/common paged-geom road tinyxml sound vdrift)


# Compile our libraries  ------
if (BUILD_GAME OR BUILD_EDITOR)

	foreach(ourlib  oics tinyxml paged-geom btOgre)
		add_subdirectory(${ourlib})
		list(APPEND OURLIBS ${ourlib})
	endforeach()

	add_subdirectory(shiny)
	list(APPEND OURLIBS "shiny")
	list(APPEND OURLIBS "shiny.OgrePlatform")

	add_subdirectory(sdl4ogre)
	list(APPEND OURLIBS "sdl4ogre")
endif()


# Game executable  ------
if (BUILD_GAME)

	# Glob .cpp files needed for the game
	file(GLOB GAME_SOURCES
		vdrift/*
		sound/*
		ogre/*.h
		ogre/*.cpp
		ogre/common/*.cpp
		ogre/common/*.h
		ogre/common/*/*.cpp
		ogre/common/*/*.h
		road/*
		network/*.hpp
		network/*.cpp
		../config/*
		../data/gui/Game.layout
		../data/materials/*.*
	)

	source_group(sound sound/*)
	source_group(road road/*)
	source_group(ogre ogre/*)
	source_group(common ogre/common/*)
	source_group(network network*)
	source_group(ogre_gui ogre/gui*)
	source_group(ogre_hud ogre/hud*)
	source_group(vdrift\\rest vdrift/*)
	#source_group(vdrift\\game Buoy|coll|game|sound|settings)
	source_group(vdrift_car vdrift/car*)
	source_group(xml ../config/*)
	source_group(xml_gui ../data/*/core*.xml)
	source_group(mat ../data/materials/*.*)

	if (MSVC)
		add_executable(${GAME_EXE} WIN32 ${GAME_SOURCES})
	else (MSVC)
		add_executable(${GAME_EXE} ${GAME_SOURCES})
	endif(MSVC)
	target_link_libraries(${GAME_EXE} ${OURLIBS} ${LIBS})
	install(TARGETS ${GAME_EXE} DESTINATION bin)
endif()


# Editor
if (BUILD_EDITOR)
	# Glob .cpp files needed for the editor
	file(GLOB EDITOR_SOURCES
		editor/*.cpp
		editor/*.h
		vdrift/configfile.*
		vdrift/pathmanager.*
		vdrift/track.*
		vdrift/k1999.*
		vdrift/model.*
		vdrift/vertexarray.*
		vdrift/roadstrip.*
		vdrift/bezier.*
		vdrift/objectloader.*
		vdrift/joepack.*
		vdrift/track_object.*
		vdrift/model_joe03.*
		vdrift/roadpatch.*
		vdrift/cartire.*
		vdrift/par.*
		ogre/common/*.cpp
		ogre/common/*.h
		ogre/common/*/*.cpp
		ogre/common/*/*.h
		ogre/ReplayTrk.*
		road/*
		../data/gui/Editor.layout
	)

	source_group(road road/*)
	source_group(editor editor/*)
	source_group(common ogre/common/*)
	source_group(editor_gui editor/gui*)
	source_group(vdrift vdrift/*)

	add_executable(${EDITOR_EXE} ${EDITOR_SOURCES})
	set_target_properties(${EDITOR_EXE} PROPERTIES COMPILE_FLAGS "-DSR_EDITOR")
	target_link_libraries(${EDITOR_EXE} ${OURLIBS} ${LIBS})
	install(TARGETS ${EDITOR_EXE} DESTINATION bin)
endif()


# Master server
if (BUILD_MASTER_SERVER)
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
		paged-geom/*.cpp
		road/*.cpp
		vdrift/*.cpp
		sound/*.cpp
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

