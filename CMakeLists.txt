
# Install data files
file(GLOB DATADIRS "[^.]*")
foreach(datadir ${DATADIRS})
	if(IS_DIRECTORY ${datadir})
		install(DIRECTORY ${datadir} DESTINATION ${SHARE_INSTALL})
	endif()
endforeach()

# Issue warning for missing tracks
if (EXISTS "${CMAKE_SOURCE_DIR}/data/tracks")
	add_subdirectory("${CMAKE_SOURCE_DIR}/data/tracks")
else()
	message("WARNING: No tracks could be found for installation.")
endif()

