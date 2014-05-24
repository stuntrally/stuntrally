cmake_minimum_required(VERSION 2.8)

# Install data files
# TODO: make this use all data/* subdirs without tracks, not entered by hand ..
foreach(datadir  cars carsim compositor editor fonts ghosts grass gui hud loading materials materials2 objects objects0 particles road road_s skies skies_s sounds terrain terrain_s trees-old trees trees2)
	install(DIRECTORY ${datadir} DESTINATION ${SHARE_INSTALL})
endforeach()

# Issue warning for missing tracks
if (EXISTS "${CMAKE_SOURCE_DIR}/data/tracks")
	add_subdirectory("${CMAKE_SOURCE_DIR}/data/tracks")
else()
	message("WARNING: No tracks could be found for installation.")
endif()

