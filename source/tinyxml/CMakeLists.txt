# Sources
file(GLOB SOURCE_FILES *.cpp)

# Create a static library
add_library(tinyxml STATIC ${SOURCE_FILES})
target_link_libraries(tinyxml PRIVATE common)
