# Linux prefers lower-case exe names
if (WIN32 OR APPLE)
    set(GAME_EXE StuntRally)
    set(EDITOR_EXE SR-Editor)
    set(SERVER_EXE SR-MasterServer)
    set(TRANSL_EXE SR-Translator)
else()
    set(GAME_EXE stuntrally)
    set(EDITOR_EXE sr-editor)
    set(SERVER_EXE sr-masterserver)
    set(TRANSL_EXE sr-translator)
endif()

# Search include files from all source sub directories
include_directories(btOgre ogre ogre/common paged-geom road sound vdrift)

# Compile our libraries  ------
if (BUILD_GAME OR BUILD_EDITOR)

    foreach (ourlib oics paged-geom btOgre)
        add_subdirectory(${ourlib})
        list(APPEND OURLIBS ${ourlib})
    endforeach ()

    add_subdirectory(shiny)
    list(APPEND OURLIBS "shiny")
    list(APPEND OURLIBS "shiny.OgrePlatform")

    add_subdirectory(sdl4ogre)
    list(APPEND OURLIBS "sdl4ogre")
endif ()


macro(install_os_depend itemname)
    if(WIN32)
        install(TARGETS ${itemname} DESTINATION .)
    else() 
        install(TARGETS ${itemname} DESTINATION bin)
    endif()
endmacro()

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

    add_executable(${GAME_EXE} WIN32 ${GAME_SOURCES})
    target_link_libraries(${GAME_EXE} ${OURLIBS})
    target_link_libraries(${GAME_EXE} ${LIBS})
    install_os_depend(${GAME_EXE})

    target_precompile_headers(${GAME_EXE} PRIVATE ogre/pch.h)
endif ()


# Editor
if (BUILD_EDITOR)
    # Glob .cpp files needed for the editor
    file(GLOB EDITOR_SOURCES
            editor/*.cpp
            editor/*.h
            vdrift/configfile.*
            vdrift/pathmanager.*
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

    add_executable(${EDITOR_EXE} WIN32 ${EDITOR_SOURCES})
    set_target_properties(${EDITOR_EXE} PROPERTIES COMPILE_FLAGS "-DSR_EDITOR")
    target_link_libraries(${EDITOR_EXE} ${OURLIBS})
    target_link_libraries(${EDITOR_EXE} ${LIBS})
    install_os_depend(${EDITOR_EXE})
    target_precompile_headers(${EDITOR_EXE} PRIVATE editor/pch.h)
endif ()


# Master server
if (BUILD_MASTER_SERVER)
    file(GLOB SERVER_SOURCES network/master-server/main.cpp) # Glob .cpp files needed for the server
    add_executable(${SERVER_EXE} ${SERVER_SOURCES})
    target_link_libraries(${SERVER_EXE} ${SERVER_LIBS})
    install_os_depend(${SERVER_EXE})
endif ()

# Transl tool
if (BUILD_TRANSL_TOOL)
	file(GLOB TRANSL_SOURCES transl/main.cpp) # Glob .cpp files needed for the translations tool
	add_executable(${TRANSL_EXE} ${TRANSL_SOURCES})
	target_link_libraries(${TRANSL_EXE} boost::boost)
	install(TARGETS ${TRANSL_EXE} DESTINATION bin)
    target_precompile_headers(${TRANSL_EXE} PRIVATE transl/pch.h)
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
endif (MSVC)
