cmake_minimum_required(VERSION 3.16)
# Include path for additional CMake library finding scripts
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

# Add a sensible build type default and warning because empty means no optimization and no debug info.
set(CMAKE_CONFIGURATION_TYPES "Debug" "Release" CACHE STRING "Configuration types")

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.22")
    cmake_policy(SET CMP0127 OLD)
endif ()

include(FindPkgConfig)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)
include(CMakeDependentOption)
include(Macros)
include(FeatureSummary)

project(StuntRally CXX C)

# Allows disabling building of components
option(BUILD_GAME             "Build the game binary."    ON)
option(BUILD_EDITOR           "Build the track editor."   ON)
option(BUILD_MASTER_SERVER    "Build the game list master server."  OFF)
option(BUILD_TRANSL_TOOL      "Build the tool for translation updating, it creates .pot"  ON)

set(_PREFIX "SR_")

# Avoid source tree pollution
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)


#  Set CXX compile flags
if( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-comment -Wno-dangling-else -Wno-deprecated-declarations -Wno-parentheses")
endif()


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
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})  # /${CMAKE_BUILD_TYPE}


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

# Subdirectories

add_subdirectory(source)
add_subdirectory(config)
add_subdirectory(dist)
add_subdirectory(data)

install(FILES Contributing.md Readme.md License.txt DESTINATION ${SHARE_INSTALL})
feature_summary(WHAT ALL)