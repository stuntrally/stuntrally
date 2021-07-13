add_library(MyGUI_OgrePlatform_Local STATIC
	MyGUI_OgreDataManager.h
	MyGUI_OgreDataStream.h
	MyGUI_OgreDiagnostic.h
	MyGUI_OgrePlatform.h
	MyGUI_OgreRTTexture.h
	MyGUI_OgreRenderManager.h
	MyGUI_OgreTexture.h
	MyGUI_OgreVertexBuffer.h
	MyGUI_OgreDataManager.cpp
	MyGUI_OgreDataStream.cpp
	MyGUI_OgreRTTexture.cpp
	MyGUI_OgreRenderManager.cpp
	MyGUI_OgreTexture.cpp
	MyGUI_OgreVertexBuffer.cpp)

target_include_directories(MyGUI_OgrePlatform_Local PRIVATE ./include ${OGRE_INCLUDE_DIRS} ${MYGUI_INCLUDE_DIRS})
target_include_directories(MyGUI_OgrePlatform_Local PUBLIC ./include)
target_link_libraries(MyGUI_OgrePlatform_Local PRIVATE ${OGRE_LIBRARIES} ${MYGUI_LIBRARIES})
