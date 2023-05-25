# Add the OGRE target

if (NOT TARGET OGRE::OGRE)
    message(STATUS "Adding OGRE::OGRE target ${OGRE_LIBRARIES}")
    add_library(OGRE::OGRE INTERFACE IMPORTED)
    set_target_properties(OGRE::OGRE PROPERTIES
            INTERFACE_LINK_LIBRARIES "${OGRE_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${OGRE_INCLUDE_DIRS}"
            )

    add_compile_definitions(OGRE_PLUGIN_DIR_REL="${OGRE_PLUGIN_DIR}")
    add_compile_definitions(OGRE_PLUGIN_DIR_DBG="${OGRE_PLUGIN_DIR}")
else()
    add_compile_definitions(OGRE_PLUGIN_DIR_REL=".")
    add_compile_definitions(OGRE_PLUGIN_DIR_DBG=".")
endif ()