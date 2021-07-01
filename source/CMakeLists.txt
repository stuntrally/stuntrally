# Search include files from all source sub directories
include_directories(btOgre ogre ogre/common paged-geom road tinyxml sound vdrift)

add_library(common INTERFACE)
if(WIN32)
  target_compile_definitions(common INTERFACE
		NOMINMAX
		WIN32_LEAN_AND_MEAN
		WINVER=0x0A00
		_WIN32_WINNT=0x0A00)
endif()

target_precompile_headers(common INTERFACE
	<algorithm>
	<cassert>
	<cmath>
	<deque>
	<list>
	<map>
	<set>
	<string>
	<unordered_map>
	<vector>

	<iomanip>
	<iostream>
	<ostream>
	<sstream>
	<fstream>
	
	<stdio.h>
	<stdlib.h>
	<time.h>
	<math.h>
	
	<SDL.h>
	<vorbis/vorbisfile.h>
	<Ogre.h>
	
	<btBulletDynamicsCommon.h>)

if(WIN32)
  target_precompile_headers(common INTERFACE <windows.h>)
endif()

target_link_libraries(common INTERFACE clientext)

# Compile our libraries  ------
if (BUILD_GAME OR BUILD_EDITOR)
	add_subdirectory(btOgre)
	add_subdirectory(oics)
	add_subdirectory(paged-geom)
	add_subdirectory(tinyxml)
	add_subdirectory(sdl4ogre)
	set(CORE_LIBS btOgre oics pagedgeom tinyxml sdl4ogre)

	add_subdirectory(shiny)
	target_link_libraries(common INTERFACE ${SHINY_LIBRARIES})
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

	add_executable(stuntrally ${GAME_SOURCES})
	if(MSVC)
		set_target_properties(stuntrally PROPERTIES WIN32_EXECUTABLE ON)
	endif()
	if(WIN32 OR APPLE)
    set_target_properties(stuntrally PROPERTIES OUTPUT_NAME StuntRally)
	endif()
	target_include_directories(stuntrally PRIVATE ${OPENAL_INCLUDE_DIR} ${ENet_INCLUDE_DIRS})
	target_link_libraries(stuntrally PRIVATE common clientext ${OPENAL_LIBRARY} ${ENet_LIBRARIES} ${CORE_LIBS})
	install(TARGETS stuntrally DESTINATION bin)
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

	add_executable(sr-editor ${EDITOR_SOURCES})
	target_compile_definitions(sr-editor PRIVATE -DSR_EDITOR)
	target_link_libraries(sr-editor PRIVATE common clientext ${CORE_LIBS})
	if(MSVC)
		set_target_properties(sr-editor PROPERTIES WIN32_EXECUTABLE ON)
	endif()
	if(WIN32 OR APPLE)
    set_target_properties(sr-editor PROPERTIES OUTPUT_NAME SR-Editor)
	endif()
	install(TARGETS sr-editor DESTINATION bin)
endif()


# Master server
if (BUILD_MASTER_SERVER)
	add_executable(sr-masterserver network/master-server/main.cpp)
	target_include_directories(sr-masterserver PRIVATE ${ENet_INCLUDE_DIRS})
	target_link_libraries(sr-masterserver PRIVATE common clientext ${ENet_LIBRARIES})
	if(WIN32 OR APPLE)
    set_target_properties(sr-editor PROPERTIES OUTPUT_NAME SR-MasterServer)
	endif()
	install(TARGETS sr-masterserver DESTINATION bin)
endif()
