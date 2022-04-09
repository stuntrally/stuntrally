project(StuntRally CXX C)
cmake_minimum_required(VERSION 2.8)

# Allows disabling building of components
option(BUILD_GAME             "Build the game binary."    ON)
option(BUILD_EDITOR           "Build the track editor."   ON)
option(BUILD_MASTER_SERVER    "Build the game list master server."  OFF)
option(BUILD_TRANSL_TOOL      "Build the tool for translation updating, it creates .pot"  ON)


# Avoid source tree pollution
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

# Add a sensible build type default and warning because empty means no optimization and no debug info.
if(NOT CMAKE_BUILD_TYPE)
	message("WARNING: CMAKE_BUILD_TYPE is not defined!\n         Defaulting to CMAKE_BUILD_TYPE=RelWithDebInfo. Use ccmake to set a proper value.")
	set(CMAKE_BUILD_TYPE RelWithDebInfo
	CACHE STRING "Type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)


# Set default compile flags for GCC
if(CMAKE_COMPILER_IS_GNUCXX)
	message(STATUS "GCC detected, adding compile flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()


# Set default compile flags for VS
if(MSVC)
	message(STATUS "VS detected, adding /MP flag (multiprocessor compile)")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()



# Include path for additional CMake library finding scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# We want the binaries to be easily accessible
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})


# Data installation path
if(WIN32)
	set(SHARE_INSTALL "."
	CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
else()
	if(NOT SHARE_INSTALL)
		set(SHARE_INSTALL "share/games/stuntrally"
		CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
	endif()
endif()
mark_as_advanced(SHARE_INSTALL)
#TODO: This would be more elegant and compiler independent to do through configure_file(),
#      but that would break other build systems for good.
add_definitions(-DSHARED_DATA_DIR="${SHARE_INSTALL}")

if (DEFINED CMAKE_BUILD_TYPE)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_definitions(-D_DEBUG="")
	endif()
endif()


if(WIN32)
	set(Boost_USE_STATIC_LIBS   ON)
endif(WIN32)

set(BOOST_COMPONENTS system thread filesystem wave)


find_package(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED QUIET)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
list(APPEND LIBS ${Boost_LIBRARIES})


# We can significantly reduce deps if only building master server or transl tools
if(BUILD_GAME OR BUILD_EDITOR)
    find_package(OGRE QUIET CONFIG) # first try 1.10+ style
    if(${OGRE_FOUND})
        # 1.10+ puts everything in OGRE_INCLUDE_DIRS & OGRE_LIBRARIES
        include_directories(${OGRE_INCLUDE_DIRS})
        list(APPEND LIBS ${OGRE_LIBRARIES})
        add_definitions(-DOGRE_PLUGIN_DIR_REL="${OGRE_PLUGIN_DIR}")
        add_definitions(-DOGRE_PLUGIN_DIR_DBG="${OGRE_PLUGIN_DIR}")
    else()
        find_package(OGRE REQUIRED QUIET)
        include_directories(${OGRE_INCLUDE_DIRS} ${OGRE_Terrain_INCLUDE_DIRS} ${OGRE_Paging_INCLUDE_DIRS})
        # Overlay moved to separate component in 1.9
        if (OGRE_VERSION_MAJOR GREATER 1 OR OGRE_VERSION_MINOR GREATER 8)
            include_directories(${OGRE_Overlay_INCLUDE_DIRS})
        endif()
        link_directories(${OGRE_LIBRARY_DIRS})
        list(APPEND LIBS ${OGRE_LIBRARIES} ${OGRE_Terrain_LIBRARIES} ${OGRE_Paging_LIBRARIES})
        # Overlay moved to separate component in 1.9
        if (OGRE_VERSION_MAJOR GREATER 1 OR OGRE_VERSION_MINOR GREATER 8)
            list(APPEND LIBS ${OGRE_Overlay_LIBRARIES})
        endif()
        add_definitions(-DOGRE_PLUGIN_DIR_REL="${OGRE_PLUGIN_DIR_REL}")
        add_definitions(-DOGRE_PLUGIN_DIR_DBG="${OGRE_PLUGIN_DIR_DBG}")
    endif()

	find_package(Bullet REQUIRED QUIET)
	include_directories(${BULLET_INCLUDE_DIRS})
	list(APPEND LIBS ${BULLET_LIBRARIES})

	find_package(SDL2 REQUIRED)
	include_directories(${SDL2_INCLUDE_DIR})
	link_directories(${SDL2_LIBRARY_DIRS})
	list(APPEND LIBS ${SDL2_LIBRARY})

	find_package(MyGUI REQUIRED QUIET)
	include_directories(${MYGUI_INCLUDE_DIRS})
	include_directories(${MYGUI_PLATFORM_INCLUDE_DIRS})
	link_directories(${MYGUI_LIB_DIR})

	IF(MSVC)
		list(APPEND LIBS ${MYGUI_LIBRARIES} ${MYGUI_PLATFORM_LIBRARIES})
	ELSE(MSVC)
		list(APPEND LIBS ${MYGUI_LIBRARIES} MyGUI.OgrePlatform) #TODO: MyGUI.OgrePlatform is not added by the find script
	ENDIF(MSVC)

	foreach(lib  OGG VorbisFile)    # Simple libraries added in a loop
		find_package(${lib} REQUIRED QUIET)
		include_directories(${${lib}_INCLUDE_DIRS})
		list(APPEND LIBS ${${lib}_LIBRARIES})
	endforeach()

	if(UNIX)
		list(APPEND LIBS "rt")   # For clock_gettime()
	endif()
endif()

# only game has sound
if(BUILD_GAME)
	find_package(OpenAL REQUIRED)
	include_directories(${OPENAL_INCLUDE_DIR})
	list(APPEND LIBS ${OPENAL_LIBRARY})
endif()

# Editor doesn't need networking
if(BUILD_GAME OR BUILD_MASTER_SERVER)
	find_package(ENet REQUIRED QUIET)
	include_directories(${ENet_INCLUDE_DIRS})
	list(APPEND LIBS ${ENet_LIBRARIES})
endif()

# Server dependencies
list(APPEND SERVER_LIBS ${ENet_LIBRARIES} ${Boost_LIBRARIES})

# Transl dependencies ?
list(APPEND TRANSL_LIBS ${Boost_LIBRARIES})


# Subdirectories

add_subdirectory(source)
add_subdirectory(config)
add_subdirectory(dist)
add_subdirectory(data)

