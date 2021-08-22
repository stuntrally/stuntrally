set(SHINY_MAJOR_VERSION 0)
set(SHINY_MINOR_VERSION 3)

# This is NOT intended as a stand-alone build system! Instead, you should include this from the main CMakeLists of your project.
# Make sure to link against Ogre, boost::filesystem and boost::wave.

#find_package(Boost REQUIRED QUIET COMPONENTS system filesystem wave)

option(SHINY_BUILD_OGRE_PLATFORM "build the Ogre platform" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
if(BUILD_SHARED_LIBS)
    set(SHINY_LIBRARY_TYPE SHARED)
else(BUILD_SHARED_LIBS)
    set(SHINY_LIBRARY_TYPE STATIC)
endif(BUILD_SHARED_LIBS)

set(SHINY_LIBRARY "shiny")
set(SHINY_OGREPLATFORM_LIBRARY "shiny.OgrePlatform")

# Sources of shiny
set(SOURCE_FILES
    Main/Factory.cpp
    Main/MaterialInstance.cpp
    Main/MaterialInstancePass.cpp
    Main/MaterialInstanceTextureUnit.cpp
    Main/Platform.cpp
    Main/Preprocessor.cpp
    Main/PropertyBase.cpp
    Main/ScriptLoader.cpp
    Main/ShaderInstance.cpp
    Main/ShaderSet.cpp
)

add_definitions(-DBOOST_ALL_NO_LIB)
add_library(${SHINY_LIBRARY} ${SHINY_LIBRARY_TYPE} ${SOURCE_FILES})
target_link_libraries(${SHINY_LIBRARY} ${LIBS})
if(BUILD_SHARED_LIBS)
    set_target_properties(${SHINY_LIBRARY} PROPERTIES SOVERSION "${SHINY_MAJOR_VERSION}.${SHINY_MINOR_VERSION}")
endif(BUILD_SHARED_LIBS)

set(SHINY_LIBRARIES ${SHINY_LIBRARY})

if (SHINY_BUILD_OGRE_PLATFORM)

    # Sources of shiny.OgrePlatform
    set(OGRE_PLATFORM_SOURCE_FILES
        Platforms/Ogre/OgreGpuProgram.cpp
        Platforms/Ogre/OgreMaterial.cpp
        Platforms/Ogre/OgreMaterialSerializer.cpp
        Platforms/Ogre/OgrePass.cpp
        Platforms/Ogre/OgrePlatform.cpp
        Platforms/Ogre/OgreTextureUnitState.cpp
    )
    file(GLOB OGRE_PLATFORM_SOURCE_FILES Platforms/Ogre/*.cpp)

    add_definitions(-DBOOST_ALL_NO_LIB)
    add_library(${SHINY_OGREPLATFORM_LIBRARY} ${SHINY_LIBRARY_TYPE} ${OGRE_PLATFORM_SOURCE_FILES})
    add_dependencies(${SHINY_OGREPLATFORM_LIBRARY} ${SHINY_LIBRARY})
    set(SHINY_LIBRARIES ${SHINY_LIBRARIES} ${SHINY_OGREPLATFORM_LIBRARY})
    target_link_libraries(${SHINY_OGREPLATFORM_LIBRARY} ${LIBS})
endif()

set(SHINY_LIBRARY ${SHINY_LIBRARY} PARENT_SCOPE)

if (DEFINED SHINY_BUILD_MATERIAL_EDITOR)
    add_subdirectory(Editor)

    set(SHINY_BUILD_EDITOR_FLAG ${SHINY_BUILD_EDITOR_FLAG} PARENT_SCOPE)
endif()

link_directories(${CMAKE_CURRENT_BINARY_DIR})
set(SHINY_LIBRARIES ${SHINY_LIBRARIES} PARENT_SCOPE)
