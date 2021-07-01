# Sources
file(GLOB SOURCE_FILES *.cpp)

# Create a static library
add_library(btOgre STATIC ${SOURCE_FILES})
target_link_libraries(btOgre PRIVATE common)
