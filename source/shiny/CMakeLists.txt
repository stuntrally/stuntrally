cmake_minimum_required(VERSION 2.8)

option(BUILD_OGRE_PLATFORM "build the Ogre platform" ON)

set(LIBNAME "shiny")
set(OGRE_PLATFORM_NAME "shiny.OgrePlatform")

# Sources
file(GLOB SOURCE_FILES Main/*.cpp)
file(GLOB OGRE_PLATFORM_SOURCE_FILES Platforms/Ogre/*.cpp)

add_library(${LIBNAME} STATIC ${SOURCE_FILES})

if (BUILD_OGRE_PLATFORM)
	add_library(${OGRE_PLATFORM_NAME} STATIC ${OGRE_PLATFORM_SOURCE_FILES})
endif()

link_directories(${CMAKE_CURRENT_BINARY_DIR})

