# Refind OpenAL to add the OpenAL::OpenAL target
if (NOT TARGET OpenAL::OpenAL)
    find_package(OpenAL QUIET CONFIG)
endif ()