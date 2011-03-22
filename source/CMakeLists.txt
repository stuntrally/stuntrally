
# Allow disabling editor building
option(BUILD_EDITOR    "Build the track editor."    ON)

# Linux prefers lower-case exe names
if(WIN32 OR APPLE)
	set(GAME_EXE StuntRally)
	set(EDITOR_EXE SR-Editor)
else()
	set(GAME_EXE stuntrally)
	set(EDITOR_EXE sr-editor)
endif()

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
	vdrift/pathmanager.cpp
	road/*.cpp
	tinyxml/*.cpp			
	paged-geom/*.cpp
	bullet/*/*.cpp
	bullet/*/*/*.cpp
	ogre/QTimer.cpp
	ogre/common/*.cpp
)

# Game executable
#TODO: MyGUI.OgrePlatform is not added by the find scripts
add_executable(${GAME_EXE} ${GAME_SOURCES})
target_link_libraries(${GAME_EXE} ${SDL_LIBRARY} ${VorbisFile_LIBRARIES} ${OIS_LIBRARIES} ${MYGUI_LIBRARIES} MyGUI.OgrePlatform ${Ogre_LIBRARIES} ${OGRE_Terrain_LIBRARIES} ${OGRE_Paging_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS ${GAME_EXE} DESTINATION bin)

# Editor
if(BUILD_EDITOR)
	add_executable(${EDITOR_EXE} ${EDITOR_SOURCES})
	set_target_properties(${EDITOR_EXE} PROPERTIES COMPILE_FLAGS "-DROAD_EDITOR")
	target_link_libraries(${EDITOR_EXE} ${SDL_LIBRARY} ${VorbisFile_LIBRARIES} ${OIS_LIBRARIES} ${MYGUI_LIBRARIES} MyGUI.OgrePlatform ${Ogre_LIBRARIES} ${OGRE_Terrain_LIBRARIES} ${OGRE_Paging_LIBRARIES} ${Boost_LIBRARIES})
	install(TARGETS ${EDITOR_EXE} DESTINATION bin)
endif()
