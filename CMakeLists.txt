cmake_minimum_required(VERSION 3.16)
project(StuntRally)

option(BUILD_GAME "Build the game binary." ON)
option(BUILD_EDITOR "Build the track editor." ON)
option(BUILD_MASTER_SERVER "Build the master server." OFF)

# Avoid source tree pollution
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR
					"In-source builds are not permitted. You must build in a separate folder:\n"
					"cmake -B build -S .\n"
					"Before that, you should remove the files already created.")
endif()

# Add a sensible build type default and warning because empty means no optimization and no debug info.
if(NOT CMAKE_BUILD_TYPE)
	message(WARNING "CMAKE_BUILD_TYPE is not defined!\n"
									"Defaulting to CMAKE_BUILD_TYPE=RelWithDebInfo. Use ccmake to set a proper value.")
	set(CMAKE_BUILD_TYPE RelWithDebInfo
		CACHE STRING "Type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable multiprocessor compilation for MSVC
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

# Include path for additional CMake library finding scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# We want the binaries to be easily accessible
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Data installation path
if(WIN32)
	set(SHARE_INSTALL "."
		CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
elseif(NOT SHARE_INSTALL)
	set(SHARE_INSTALL "share/games/stuntrally"
		CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
endif()
mark_as_advanced(SHARE_INSTALL)
#TODO: This would be more elegant and compiler independent to do through configure_file(),
#      but that would break other build systems for good.
add_definitions(-DSHARED_DATA_DIR="${SHARE_INSTALL}")

add_compile_definitions($<$<CONFIG:Debug>:_DEBUG=1>)

find_package(Threads REQUIRED)

# We can significantly reduce deps if only building master server
if(BUILD_GAME OR BUILD_EDITOR)
	find_package(OGRE REQUIRED CONFIG)
	find_package(SDL2 REQUIRED CONFIG)
	find_package(Bullet REQUIRED)
	find_package(OGG REQUIRED)
	find_package(VorbisFile REQUIRED)
	find_package(MyGUI REQUIRED)

	add_subdirectory(extern/BulletFileLoader)
	add_subdirectory(extern/BulletWorldImporter)
	add_subdirectory(extern/mygui)

	add_definitions(-DOGRE_PLUGIN_DIR_REL="${OGRE_PLUGIN_DIR}")

	add_library(clientext INTERFACE)
	target_include_directories(clientext INTERFACE ${MYGUI_INCLUDE_DIRS} ${OGRE_INCLUDE_DIRS} ${BULLET_INCLUDE_DIRS} ${OGG_INCLUDE_DIRS} ${VorbisFile_INCLUDE_DIRS})
	target_link_libraries(clientext INTERFACE
	  BulletFileLoader
		BulletWorldImporter
		SDL2::SDL2
		${MYGUI_LIBRARIES}
		MyGUI_OgrePlatform_Extern
		${OGRE_LIBRARIES}
		${BULLET_LIBRARIES}
		${OGG_LIBRARIES}
		${VorbisFile_LIBRARIES})
endif()

# Only game has sound
if(BUILD_GAME)
	find_package(OpenAL REQUIRED)
endif()

# Editor doesn't need networking
if(BUILD_GAME OR BUILD_MASTER_SERVER)
	find_package(ENet REQUIRED)
endif()

add_subdirectory(source)
add_subdirectory(config)
add_subdirectory(dist)
add_subdirectory(data)
