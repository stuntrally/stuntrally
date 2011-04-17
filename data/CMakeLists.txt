
# Install data files
foreach(datadir  cars compositor editor fonts gui hud loading materials particles skies sounds terrain trees)
	install(DIRECTORY ${datadir} DESTINATION ${SHARE_INSTALL})
endforeach()

# Issue warning for missing tracks
if (EXISTS "${CMAKE_SOURCE_DIR}/data/tracks")
	add_subdirectory("${CMAKE_SOURCE_DIR}/data/tracks")
else()
	message("WARNING: No tracks could be found for installation.")
endif()

