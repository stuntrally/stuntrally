set(LIBNAME "bullet")

# Sources
file(GLOB SOURCE_FILES */*.cpp */*/*.cpp)

# Create a static library
add_library(${LIBNAME} STATIC ${SOURCE_FILES})

link_directories(${CMAKE_CURRENT_BINARY_DIR})

