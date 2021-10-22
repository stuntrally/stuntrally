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

elseif(${_PREFIX}USE_BOOST STREQUAL "CONAN")
    get_target_property(Boost_LIBRARIES CONAN_PKG::boost INTERFACE_LINK_LIBRARIES)
    foreach(conf IN ITEMS release debug)
        list(REMOVE_ITEM Boost_LIBRARIES 
            "CONAN_LIB::boost_boost_unit_test_framework${conf}" 
            "CONAN_LIB::boost_boost_test_exec_monitor${conf}"
            "CONAN_LIB::boost_boost_prg_exec_monitor${conf}"
        )
    endforeach()
    set_target_properties(CONAN_PKG::boost PROPERTIES INTERFACE_LINK_LIBRARIES "${Boost_LIBRARIES}")
endif ()