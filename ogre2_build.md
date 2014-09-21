1. Build Ogre 2.0

mkdir build
cd build
cmake ..
make

1.1 Fix messed up FindOGRE.cmake:

```
diff -r 3133f07b0308 CMake/Packages/FindOGRE.cmake
--- a/CMake/Packages/FindOGRE.cmake        Sat Sep 20 20:33:33 2014 +0200
+++ b/CMake/Packages/FindOGRE.cmake        Sun Sep 21 07:31:51 2014 +0200
@@ -216,16 +216,8 @@
   set(OGRE_INCOMPATIBLE FALSE)
 endif ()
 
-if (NOT OGRE_SOURCE) # If using ogre sources, use the target names instead of library files to link.
         find_library(OGRE_LIBRARY_REL NAMES ${OGRE_LIBRARY_NAMES} HINTS ${OGRE_LIB_SEARCH_PATH} ${OGRE_PKGC_LIBRARY_DIRS} ${OGRE_FRAMEWORK_SEARCH_PATH} PATH_SUFFIXES "" "Release" "RelWithDebInfo" "MinSizeRel")
         find_library(OGRE_LIBRARY_DBG NAMES ${OGRE_LIBRARY_NAMES_DBG} HINTS ${OGRE_LIB_SEARCH_PATH} ${OGRE_PKGC_LIBRARY_DIRS} ${OGRE_FRAMEWORK_SEARCH_PATH} PATH_SUFFIXES "" "Debug")
-else()
-        if( NOT OGRE_LIBRARIES OR OGRE_LIBRARIES STREQUAL "" )
-                message( FATAL_ERROR "When using Ogre from sources, please specify target names in OGRE_LIBRARIES!" )
-        else()
-                message( "Using Ogre source instead of binary libraries - skipping library files search." )
-        endif()
-endif()
 
 make_library_set(OGRE_LIBRARY)
```


2. Build MyGUI

To find Ogre without installing it:
 
cmake .. -DOGRE_HOME=/home/scrawl/Dev/ogre  -DOGRE_BUILD=/home/scrawl/Dev/ogre/build -DOGRE_SOURCE_DIR=/home/scrawl/Dev/ogre/ -DOGRE_SOURCE=/home/scrawl/Dev/ogre 

Apply patches

```
diff --git a/Common/Base/Ogre/BaseManager.cpp b/Common/Base/Ogre/BaseManager.cpp
index aa7496e..6bf4040 100644
--- a/Common/Base/Ogre/BaseManager.cpp
+++ b/Common/Base/Ogre/BaseManager.cpp
@@ -8,6 +8,12 @@
 #include "BaseManager.h"
 #include <MyGUI_OgrePlatform.h>
 
+#include <Compositor/OgreCompositorManager2.h>
+#include <Compositor/OgreCompositorNodeDef.h>
+#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>
+#include <Compositor/OgreCompositorWorkspaceDef.h>
+#include <OgreFrameStats.h>
+
 #if MYGUI_PLATFORM == MYGUI_PLATFORM_WIN32
 #	include <windows.h>
 #elif MYGUI_PLATFORM == MYGUI_PLATFORM_LINUX
@@ -100,23 +106,57 @@ namespace base
 			::SendMessageA((HWND)handle, WM_SETICON, 1, (LPARAM)hIconBig);
 	#endif
 
-		mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC, "BaseSceneManager");
+		const size_t numThreads = std::max<int>(1, Ogre::PlatformInformation::getNumLogicalCores());
+		Ogre::InstancingTheadedCullingMethod threadedCullingMethod = Ogre::INSTANCING_CULLING_SINGLETHREAD;
+		if(numThreads > 1) Ogre::InstancingTheadedCullingMethod threadedCullingMethod = Ogre::INSTANCING_CULLING_THREADED;
+		mSceneManager = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, numThreads, threadedCullingMethod);
+
+		//mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC, "BaseSceneManager");
+
 
 		mCamera = mSceneManager->createCamera("BaseCamera");
 		mCamera->setNearClipDistance(5);
 		mCamera->setPosition(400, 400, 400);
 		mCamera->lookAt(0, 150, 0);
 
+		mRoot->initialiseCompositor();
+		Ogre::CompositorManager2* pCompositorManager = mRoot->getCompositorManager2();
+		const Ogre::String workspaceName = "scene workspace";
+		const Ogre::IdString workspaceNameHash = workspaceName;
+		//pCompositorManager->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue::Black);
+
+		Ogre::CompositorNodeDef *nodeDef = pCompositorManager->addNodeDefinition( "myworkspace" );
+		//Input texture
+		nodeDef->addTextureSourceName( "WindowRT", 0, Ogre::TextureDefinitionBase::TEXTURE_INPUT );
+		nodeDef->setNumTargetPass( 1 );
+		{
+			Ogre::CompositorTargetDef *targetDef = nodeDef->addTargetPass( "WindowRT" );
+			targetDef->setNumPasses( 1 );
+			{
+				{
+					Ogre::CompositorPassSceneDef *passScene = static_cast<Ogre::CompositorPassSceneDef*>
+																( targetDef->addPass( Ogre::PASS_SCENE ) );
+					passScene->mShadowNode = Ogre::IdString();
+				}
+			}
+		}
+		Ogre::CompositorWorkspaceDef *workDef = pCompositorManager->addWorkspaceDefinition( workspaceName );
+		workDef->connectOutput( nodeDef->getName(), 0 );
+
+		pCompositorManager->addWorkspace(mSceneManager, mWindow, mCamera, workspaceNameHash, true);
+
 		// Create one viewport, entire window
-		Ogre::Viewport* vp = mWindow->addViewport(mCamera);
+		//Ogre::Viewport* vp = mWindow->addViewport(mCamera);
 		// Alter the camera aspect ratio to match the viewport
-		mCamera->setAspectRatio((float)vp->getActualWidth() / (float)vp->getActualHeight());
+		//mCamera->setAspectRatio((float)vp->getActualWidth() / (float)vp->getActualHeight());
 
 		// Set default mipmap level (NB some APIs ignore this)
 		Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
 
 		mSceneManager->setAmbientLight(Ogre::ColourValue::White);
-		Ogre::Light* light = mSceneManager->createLight("MainLight");
+		Ogre::SceneNode* lightNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
+		Ogre::Light* light = mSceneManager->createLight();
+		lightNode->attachObject(light);
 		light->setType(Ogre::Light::LT_DIRECTIONAL);
 		Ogre::Vector3 vec(-0.3f, -0.3f, -0.3f);
 		vec.normalise();
@@ -144,9 +184,11 @@ namespace base
 	void BaseManager::run()
 	{
 		// инициализируем все рендер таргеты
-		mRoot->getRenderSystem()->_initRenderTargets();
+		//mRoot->getRenderSystem()->_initRenderTargets();
 
 		// крутимся бесконечно
+
+		/*
 		while (true)
 		{
 			Ogre::WindowEventUtilities::messagePump();
@@ -167,6 +209,8 @@ namespace base
 #endif
 
 		};
+		*/
+		mRoot->startRendering();
 	}
 
 	void BaseManager::destroy()
@@ -482,10 +526,11 @@ namespace base
 
 		try
 		{
-			const Ogre::RenderTarget::FrameStats& stats = mWindow->getStatistics();
-			result["FPS"] = MyGUI::utility::toString(stats.lastFPS);
-			result["triangle"] = MyGUI::utility::toString(stats.triangleCount);
-			result["batch"] = MyGUI::utility::toString(stats.batchCount);
+			const Ogre::FrameStats* stats = mRoot->getFrameStats();
+			result["FPS"] = MyGUI::utility::toString(stats->getFps());
+			const Ogre::RenderTarget::FrameStats rtStats = mWindow->getStatistics();
+			result["triangle"] = MyGUI::utility::toString(rtStats.triangleCount);
+			result["batch"] = MyGUI::utility::toString(rtStats.batchCount);
 			result["batch gui"] = MyGUI::utility::toString(MyGUI::OgreRenderManager::getInstance().getBatchCount());
 		}
 		catch (...)
diff --git a/Common/Base/Ogre/BaseManager.h b/Common/Base/Ogre/BaseManager.h
index 856d2e9..be63acc 100644
--- a/Common/Base/Ogre/BaseManager.h
+++ b/Common/Base/Ogre/BaseManager.h
@@ -8,6 +8,8 @@
 #define __BASE_MANAGER_H__
 
 #include <Ogre.h>
+#include <OgreFrameListener.h>
+
 #include <MyGUI.h>
 
 #include "InputManager.h"
diff --git a/Demos/CMakeLists.txt b/Demos/CMakeLists.txt
index a4d4242..c1d50fe 100644
--- a/Demos/CMakeLists.txt
+++ b/Demos/CMakeLists.txt
@@ -1,7 +1,8 @@
 add_subdirectory(Demo_Colour)
 add_subdirectory(Demo_Console)
 add_subdirectory(Demo_Controllers)
-add_subdirectory(Demo_Gui)
+# FIXME port RenderBox
+#add_subdirectory(Demo_Gui)
 add_subdirectory(Demo_ItemBox)
 add_subdirectory(Demo_PanelView)
 add_subdirectory(Demo_Picking)
@@ -11,7 +12,8 @@ if (MYGUI_BUILD_PLUGINS)
 		add_subdirectory(Demo_PluginBerkeliumWidget)
 	endif ()
 endif ()
-add_subdirectory(Demo_RenderBox)
+# FIXME port RenderBox
+#add_subdirectory(Demo_RenderBox)
 add_subdirectory(Demo_ScrollView)
 add_subdirectory(Demo_Themes)
 add_subdirectory(Demo_Pointers)
diff --git a/Platforms/Ogre/OgrePlatform/include/MyGUI_OgreRenderManager.h b/Platforms/Ogre/OgrePlatform/include/MyGUI_OgreRenderManager.h
index aa847a7..35e8cf9 100644
--- a/Platforms/Ogre/OgrePlatform/include/MyGUI_OgreRenderManager.h
+++ b/Platforms/Ogre/OgrePlatform/include/MyGUI_OgreRenderManager.h
@@ -13,6 +13,7 @@
 #include "MyGUI_RenderManager.h"
 
 #include <Ogre.h>
+#include <OgreFrameListener.h>
 
 #include "MyGUI_LastHeader.h"
 
@@ -24,7 +25,8 @@ namespace MyGUI
 		public IRenderTarget,
 		public Ogre::WindowEventListener,
 		public Ogre::RenderQueueListener,
-		public Ogre::RenderSystem::Listener
+		public Ogre::RenderSystem::Listener,
+		public Ogre::FrameListener
 	{
 	public:
 		OgreRenderManager();
@@ -93,6 +95,7 @@ namespace MyGUI
 #endif
 
 	private:
+		bool frameRenderingQueued(const Ogre::FrameEvent &evt);
 		virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);
 		virtual void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation);
 		virtual void windowResized(Ogre::RenderWindow* _window);
diff --git a/Platforms/Ogre/OgrePlatform/include/MyGUI_OgreTexture.h b/Platforms/Ogre/OgrePlatform/include/MyGUI_OgreTexture.h
index 20dc5fe..19e089b 100644
--- a/Platforms/Ogre/OgrePlatform/include/MyGUI_OgreTexture.h
+++ b/Platforms/Ogre/OgrePlatform/include/MyGUI_OgreTexture.h
@@ -13,6 +13,8 @@
 
 #include <OgreResource.h>
 #include <OgreTexture.h>
+#include <OgrePixelBox.h>
+#include <OgreSharedPtr.h>
 
 #include "MyGUI_LastHeader.h"
 
diff --git a/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreRTTexture.cpp b/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreRTTexture.cpp
index 70b4f0f..bcfefd4 100644
--- a/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreRTTexture.cpp
+++ b/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreRTTexture.cpp
@@ -55,7 +55,7 @@ namespace MyGUI
 		if (mViewport == nullptr)
 		{
 			mViewport = rtt->addViewport(nullptr);
-			mViewport->setClearEveryFrame(false);
+			//mViewport->setClearEveryFrame(false);
 			mViewport->setOverlaysEnabled(false);
 		}
 
diff --git a/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreRenderManager.cpp b/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreRenderManager.cpp
index 2948ab4..3025bc0 100644
--- a/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreRenderManager.cpp
+++ b/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreRenderManager.cpp
@@ -39,6 +39,7 @@ namespace MyGUI
 
 	void OgreRenderManager::initialise(Ogre::RenderWindow* _window, Ogre::SceneManager* _scene)
 	{
+		Ogre::Root::getSingleton().addFrameListener(this);
 		MYGUI_PLATFORM_ASSERT(!mIsInitialise, getClassTypeName() << " initialised twice");
 		MYGUI_PLATFORM_LOG(Info, "* Initialise: " << getClassTypeName());
 
@@ -74,6 +75,7 @@ namespace MyGUI
 
 	void OgreRenderManager::shutdown()
 	{
+		Ogre::Root::getSingleton().removeFrameListener(this);
 		MYGUI_PLATFORM_ASSERT(mIsInitialise, getClassTypeName() << " is not initialised");
 		MYGUI_PLATFORM_LOG(Info, "* Shutdown: " << getClassTypeName());
 
@@ -179,23 +181,28 @@ namespace MyGUI
 		}
 	}
 
-	void OgreRenderManager::renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation)
+	bool OgreRenderManager::frameRenderingQueued(const Ogre::FrameEvent& evt)
 	{
+if (!mWindow->getNumViewports())
+	return true;
+
 		Gui* gui = Gui::getInstancePtr();
 		if (gui == nullptr)
-			return;
-
-		if (Ogre::RENDER_QUEUE_OVERLAY != queueGroupId)
-			return;
+			return true;
+Ogre::Root::getSingleton().getRenderSystem()->_setViewport(mWindow->getViewport(0));
+		//if (Ogre::RENDER_QUEUE_OVERLAY != queueGroupId)
+			//return;
 
+		/*
 		Ogre::Viewport* viewport = mSceneManager->getCurrentViewport();
 		if (nullptr == viewport
 			|| !viewport->getOverlaysEnabled())
-			return;
+			return true;
 
 		if (mWindow->getNumViewports() <= mActiveViewport
 			|| viewport != mWindow->getViewport(mActiveViewport))
-			return;
+			return true;
+		*/
 
 		mCountBatch = 0;
 
@@ -217,6 +224,11 @@ namespace MyGUI
 		mUpdate = false;
 	}
 
+	void OgreRenderManager::renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation)
+	{
+		std::cout << "RQ started " << std::endl;
+	}
+
 	void OgreRenderManager::renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation)
 	{
 	}
diff --git a/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreTexture.cpp b/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreTexture.cpp
index 45ec397..a78e4c1 100644
--- a/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreTexture.cpp
+++ b/Platforms/Ogre/OgrePlatform/src/MyGUI_OgreTexture.cpp
@@ -11,6 +11,7 @@
 #include "MyGUI_OgreDiagnostic.h"
 #include "MyGUI_OgreRTTexture.h"
 #include <Ogre.h>
+#include <OgreTexture.h>
 
 #include "MyGUI_LastHeader.h"
 
```

3. Install MyGUI to a local prefix

cmake .. -DCMAKE_INSTALL_PREFIX=/opt/mygui-ogre2

3. Build SR

To find OGRE and MyGUI:

export PKG_CONFIG_PATH=/opt/mygui2/lib/pkgconfig
cmake .. -DOGRE_HOME=/home/scrawl/Dev/ogre  -DOGRE_BUILD=/home/scrawl/Dev/ogre/build -DOGRE_SOURCE_DIR=/home/scrawl/Dev/ogre/ -DOGRE_SOURCE=/home/scrawl/Dev/ogre 

In CMake, the INCLUDE_DIR for Ogre components will not be set correctly and must be adjusted manually, e.g. to
/home/scrawl/Dev/ogre/Components/Overlay/include
