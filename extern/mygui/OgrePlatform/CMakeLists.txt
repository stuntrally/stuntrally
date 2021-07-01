add_library(MyGUI.OgrePlatform STATIC
	include/MyGUI_OgreDataManager.h
	include/MyGUI_OgreDataStream.h
	include/MyGUI_OgreDiagnostic.h
	include/MyGUI_OgrePlatform.h
	include/MyGUI_OgreRTTexture.h
	include/MyGUI_OgreRenderManager.h
	include/MyGUI_OgreTexture.h
	include/MyGUI_OgreVertexBuffer.h
	src/MyGUI_OgreDataManager.cpp
	src/MyGUI_OgreDataStream.cpp
	src/MyGUI_OgreRTTexture.cpp
	src/MyGUI_OgreRenderManager.cpp
	src/MyGUI_OgreTexture.cpp
	src/MyGUI_OgreVertexBuffer.cpp)

target_include_directories(MyGUI.OgrePlatform PRIVATE ./include ${OGRE_INCLUDE_DIRS})
target_include_directories(MyGUI.OgrePlatform PUBLIC ./include)
target_link_libraries(MyGUI.OgrePlatform PRIVATE MyGUIEngine ${OGRE_LIBRARIES})
