
# Search include files from all source sub directories
include_directories(btOgre bullet ogre paged-geom road tinyxml vdrift)

# Take every .cpp file in all subdirectories
#TODO: Editor
file(GLOB SOURCE_FILES
	btOgre/*.cpp
	bullet/*/*.cpp
	bullet/*/*/*.cpp
	#editor/*.cpp
	ogre/*.cpp
	ogre/*/*.cpp
	paged-geom/*.cpp
	road/*.cpp
	tinyxml/*.cpp
	vdrift/*.cpp
	)

# Executable
add_executable(stuntrally ${SOURCE_FILES})

# Link all needed libraries
#TODO: MyGUI.OgrePlatform is not added by the find scripts
target_link_libraries(stuntrally ${SDL_LIBRARY} ${VorbisFile_LIBRARIES} ${OIS_LIBRARIES} ${MYGUI_LIBRARIES} MyGUI.OgrePlatform ${Ogre_LIBRARIES} ${OGRE_Terrain_LIBRARIES} ${OGRE_Paging_LIBRARIES})

