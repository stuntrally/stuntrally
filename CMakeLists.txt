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


# Libraries

find_package(OGRE REQUIRED QUIET)
include_directories(${OGRE_INCLUDE_DIRS} ${OGRE_Terrain_INCLUDE_DIRS} ${OGRE_Paging_INCLUDE_DIRS})
link_directories(${OGRE_LIBRARY_DIRS})

find_package(MyGUI REQUIRED QUIET)
include_directories(${MYGUI_INCLUDE_DIRS})

find_package(SDL REQUIRED QUIET)
include_directories(${SDL_INCLUDE_DIR})

find_package(OIS REQUIRED QUIET)
include_directories(${OIS_INCLUDE_DIRS})

find_package(VorbisFile REQUIRED QUIET)
include_directories(${VorbisFile_INCLUDE_DIRS})


# Subdirectories
add_subdirectory(Source)
