# Add the Boost target

if (${_PREFIX}USE_BOOST STREQUAL "SYSTEM")
    if (NOT TARGET Boost::Boost)
        find_package(Boost REQUIRED COMPONENTS system thread filesystem wave)
        message(STATUS "Adding Boost::Boost target")
        add_library(Boost::Boost INTERFACE IMPORTED)
        set_target_properties(Boost::Boost PROPERTIES
                INTERFACE_LINK_LIBRARIES "${Boost_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}"
                )
    endif ()
endif ()