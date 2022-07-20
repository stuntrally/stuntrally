# Add the OGRE target

if (${_PREFIX}USE_OGRE STREQUAL "SYSTEM")
    if (NOT TARGET OGRE::OGRE)
        find_package(OGRE QUIET CONFIG)
        if(NOT OGRE_FOUND)
            find_package(OGRE QUIET)
        endif()
        message(STATUS "Adding OGRE::OGRE target")
        add_library(OGRE::OGRE INTERFACE IMPORTED)
        set_target_properties(OGRE::OGRE PROPERTIES
                INTERFACE_LINK_LIBRARIES "${OGRE_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${OGRE_INCLUDE_DIRS}"
                )
    endif ()

    add_compile_definitions(OGRE_PLUGIN_DIR_REL="${OGRE_PLUGIN_DIR_REL}")
    add_compile_definitions(OGRE_PLUGIN_DIR_DBG="${OGRE_PLUGIN_DIR_DBG}")
else()
    add_compile_definitions(OGRE_PLUGIN_DIR_REL=".")
    add_compile_definitions(OGRE_PLUGIN_DIR_DBG=".")
endif ()