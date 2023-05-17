# Add the Boost target

if (NOT TARGET boost::boost)
    find_package(Boost REQUIRED COMPONENTS system thread filesystem wave)
    message(STATUS "Adding boost::boost target")
    add_library(boost::boost INTERFACE IMPORTED)
    set_target_properties(boost::boost PROPERTIES
            INTERFACE_LINK_LIBRARIES "${Boost_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}"
            )
endif ()