set(CMAKE_THREAD_PREFER_PTHREAD YES)
find_package(Threads REQUIRED)


find_package(Boost REQUIRED)
find_package(Bullet REQUIRED)
find_package(OGRE REQUIRED)
find_package(SDL2 REQUIRED)
find_package(MyGUI REQUIRED)
find_package(OGG REQUIRED)
find_package(Vorbis REQUIRED)
find_package(Tinyxml2 REQUIRED)

find_package(OpenAL REQUIRED)
find_package(ENet REQUIRED)

# Add the OGRE::OGRE target 
include(cmake/dependencies-helpers/OGRE.cmake)
include(cmake/dependencies-helpers/Boost.cmake)

if(NOT TARGET OpenAL::OpenAL)
    add_library(OpenAL::OpenAL INTERFACE IMPORTED)
    set_target_properties(OpenAL::OpenAL PROPERTIES
        INTERFACE_LINK_LIBRARIES "${OPENAL_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}"
    )
endif()

set(LIBS boost::boost
        Threads::Threads
        OGRE::OGRE
        Bullet::Bullet
        SDL2::SDL2
        MyGUI::MyGUI
        Vorbis::vorbis
        Vorbis::vorbisfile
        Ogg::ogg
        OpenAL::OpenAL
        enet::enet
        tinyxml2::tinyxml2
)

set(SERVER_LIBS boost::boost enet::enet)
