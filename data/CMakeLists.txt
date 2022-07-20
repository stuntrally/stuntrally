
# Install all data/* subdirs without tracks
file (GLOB datadir "[^.]*")
foreach (dir ${datadir})
	if (IS_DIRECTORY ${dir} AND NOT ${dir} MATCHES "tracks")
		install(DIRECTORY ${dir} DESTINATION ${SHARE_INSTALL})
		#message(${dir})
	endif()
endforeach()

#foreach (datadir  cars carsim compositor editor fonts ghosts grass gui hud lessons loading materials materials2 objects objects2 objects0 objectsC particles road road_s rocks skies skies_s sounds terrain terrain_s trees-old trees trees2)
#	install(DIRECTORY ${datadir} DESTINATION ${SHARE_INSTALL})
#endforeach()

# Issue warning for missing tracks
if (EXISTS "${CMAKE_SOURCE_DIR}/data/tracks")
    add_subdirectory("${CMAKE_SOURCE_DIR}/data/tracks")
else ()
    message("WARNING: No tracks could be found for installation.")
endif ()
