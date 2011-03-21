
option(BUILD_EDITOR    "Build the track editor."    ON)

# Search include files from all source sub directories
include_directories(btOgre bullet ogre paged-geom road tinyxml vdrift)

# Take every .cpp file in all subdirectories
file(GLOB GAME_SOURCES
	btOgre/*.cpp
	bullet/*/*.cpp
	bullet/*/*/*.cpp
	ogre/*.cpp
	ogre/*/*.cpp
	paged-geom/*.cpp
	road/*.cpp
	tinyxml/*.cpp
	vdrift/*.cpp
	)

file(GLOB EDITOR_SOURCES editor/*.cpp
			 vdrift/configfile.cpp
			 road/*.cpp
			 tinyxml/*.cpp			
			 paged-geom/*.cpp
			bullet/*/*.cpp
			bullet/*/*/*.cpp
			ogre/QTimer.cpp
			ogre/common/*.cpp
)

# Executable
add_executable(stuntrally ${GAME_SOURCES})

# Link all needed libraries
#TODO: MyGUI.OgrePlatform is not added by the find scripts
target_link_libraries(stuntrally ${SDL_LIBRARY} ${VorbisFile_LIBRARIES} ${OIS_LIBRARIES} ${MYGUI_LIBRARIES} MyGUI.OgrePlatform ${Ogre_LIBRARIES} ${OGRE_Terrain_LIBRARIES} ${OGRE_Paging_LIBRARIES} ${Boost_LIBRARIES})

# Editor
#TODO: Fix editor for Linux
if(BUILD_EDITOR)
	add_executable(sr-editor ${EDITOR_SOURCES})
	set_target_properties(sr-editor PROPERTIES COMPILE_FLAGS "-DROAD_EDITOR")
	target_link_libraries(sr-editor ${SDL_LIBRARY} ${VorbisFile_LIBRARIES} ${OIS_LIBRARIES} ${MYGUI_LIBRARIES} MyGUI.OgrePlatform ${Ogre_LIBRARIES} ${OGRE_Terrain_LIBRARIES} ${OGRE_Paging_LIBRARIES} ${Boost_LIBRARIES})
endif()
