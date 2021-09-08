cmake_minimum_required(VERSION 3.16)
# Include path for additional CMake library finding scripts
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

include(CMakeDependentOption)
include(Macros)
include(FeatureSummary)

project(StuntRally CXX C)

# Allow disabling game building
option(BUILD_GAME "Build the game binary." ON)

# Allow disabling editor building
option(BUILD_EDITOR "Build the track editor." ON)

# Allow disabling master server building
option(BUILD_MASTER_SERVER "Build the master server." OFF)

set(_PREFIX "SR_")

# Test if conan is installed
find_program(CONAN_CMD_TEST conan NO_CACHE)
cmake_dependent_option(USE_PACKAGE_MANAGER "Use conan" ON "NOT CONAN_CMD_TEST STREQUAL CONAN_CMD_TEST-NOTFOUND" OFF)
message(STATUS "Using conan: ${USE_PACKAGE_MANAGER}")

# Allow user to globally set the library preference for external libraries
cmd_option(${_PREFIX}LIB_PREFERENCE
        "Library preference [SYSTEM (if available), CONAN]"
        "CONAN"
        STRINGS "SYSTEM" "CONAN"
        )

# Special mode, that will force dependencies to the packages provided by system unless they were set to conan explicitly.
option(${_PREFIX}FORCE_SYSTEM_DEPENDENCIES "Force the use of system packages")

# Avoid source tree pollution
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

# Add a sensible build type default and warning because empty means no optimization and no debug info.
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")


# Set default compile flags for GCC
if (CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif ()


# Set default compile flags for VS
if (MSVC)
    message(STATUS "VS detected, adding /MP flag (multiprocessor compile)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /EHsc")
    add_definitions(-DNOMINMAX)
endif ()


# We want the binaries to be easily accessible
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})


# Data installation path
if (WIN32)
    set(SHARE_INSTALL "."
            CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
else ()
    if (NOT SHARE_INSTALL)
        set(SHARE_INSTALL "share/games/stuntrally"
                CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
    endif ()
endif ()
mark_as_advanced(SHARE_INSTALL)
#TODO: This would be more elegant and compiler independent to do through configure_file(),
#      but that would break other build systems for good.
add_definitions(-DSHARED_DATA_DIR="${SHARE_INSTALL}")

if (DEFINED CMAKE_BUILD_TYPE)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-D_DEBUG="")
    endif ()
endif ()


################################################################################
# Check for dependencies
################################################################################
include(DependenciesConfig)
resolve_conan_dependencies()

# Subdirectories

add_subdirectory(source)
add_subdirectory(config)
add_subdirectory(dist)
add_subdirectory(data)
