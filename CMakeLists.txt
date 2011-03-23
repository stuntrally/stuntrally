project(StuntRally CXX C)
cmake_minimum_required(VERSION 2.6)

# Avoid source tree pollution
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

# Add a sensible build type default and warning because empty means no optimization and no debug info.
if(NOT CMAKE_BUILD_TYPE)
	message("WARNING: CMAKE_BUILD_TYPE is not defined!\n         Defaulting to CMAKE_BUILD_TYPE=RelWithDebInfo. Use ccmake to set a proper value.")
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Where to include additional CMake library finding scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# We want the binaries to be easily accessible
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Data installation path
if(WIN32)
	set(SHARE_INSTALL "." CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
else()
	if(NOT SHARE_INSTALL)
		set(SHARE_INSTALL "share/games/stuntrally" CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
	endif()
endif()
mark_as_advanced(SHARE_INSTALL)
#TODO: This would be more elegant and compiler independent to do through configure_file(),
#      but that would breake other build systems for good.
add_definitions(-DSHARED_DATA_DIR="${SHARE_INSTALL}")

# Libraries

find_package(OGRE REQUIRED QUIET)
include_directories(${OGRE_INCLUDE_DIRS} ${OGRE_Terrain_INCLUDE_DIRS} ${OGRE_Paging_INCLUDE_DIRS})
link_directories(${OGRE_LIBRARY_DIRS})
list(APPEND LIBS ${OGRE_LIBRARIES} ${OGRE_Terrain_LIBRARIES} ${OGRE_Paging_LIBRARIES})

find_package(MyGUI REQUIRED QUIET)
include_directories(${MYGUI_INCLUDE_DIRS})
link_directories(${MYGUI_LIB_DIR})
list(APPEND LIBS ${MYGUI_LIBRARIES} MyGUI.OgrePlatform)

find_package(Boost COMPONENTS thread filesystem REQUIRED QUIET)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
list(APPEND LIBS ${Boost_LIBRARIES})

find_package(SDL REQUIRED QUIET)
include_directories(${SDL_INCLUDE_DIR})
list(APPEND LIBS ${SDL_LIBRARY})

find_package(OIS REQUIRED QUIET)
include_directories(${OIS_INCLUDE_DIRS})
list(APPEND LIBS ${OIS_LIBRARIES})

find_package(OGG REQUIRED QUIET)
include_directories(${OGG_INCLUDE_DIRS})
list(APPEND LIBS ${OGG_LIBRARIES})

find_package(VorbisFile REQUIRED QUIET)
include_directories(${VorbisFile_INCLUDE_DIRS})
list(APPEND LIBS ${VorbisFile_LIBRARIES})

# Subdirectories

add_subdirectory(source)
add_subdirectory(config)
if (EXISTS "${CMAKE_SOURCE_DIR}/data" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
	add_subdirectory(data)
else()
	message("WARNING: Data not found, clone the data repo as a directory named data.")
endif()
