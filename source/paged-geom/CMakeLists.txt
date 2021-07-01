# Sources
file(GLOB SOURCE_FILES *.cpp)

# Create a static library
add_library(pagedgeom STATIC ${SOURCE_FILES})
target_link_libraries(pagedgeom PRIVATE common)
target_include_directories(pagedgeom PUBLIC ./)
