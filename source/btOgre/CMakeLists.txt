set(LIBNAME "btOgre")

# Sources
file(GLOB SOURCE_FILES *.cpp)

# Create a static library
add_library(${LIBNAME} STATIC ${SOURCE_FILES})

target_link_libraries(${LIBNAME} ${LIBS})
