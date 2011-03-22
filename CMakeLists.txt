
# Install data files
foreach(datadir  cars editor gui materials skys terrain trees fonts hud particles sounds)
	install(DIRECTORY ${datadir} DESTINATION ${SHARE_INSTALL})
endforeach()

# Issue warning for missing tracks
if (EXISTS "${CMAKE_SOURCE_DIR}/data/tracks")
	add_subdirectory("${CMAKE_SOURCE_DIR}/data/tracks")
else()
	message("WARNING: No tracks could be found for installation.")
endif()

