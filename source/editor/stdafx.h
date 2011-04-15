#pragma once

#include <OgrePlatform.h>
#if OGRE_PLATFORM  == OGRE_PLATFORM_WIN32
	#define WINVER 0x0501
	#define _WIN32_WINNT 0x0501
	#define _WIN32_WINDOWS 0x0410
	#define WIN32_LEAN_AND_MEAN
	//  win
	#include <windows.h>
	#include <strsafe.h>
#endif

#include "tinyxml.h"

//  Ogre
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreMeshManager.h>
#include <OgreSubMesh.h>
#include <OgreMaterialManager.h>

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreTerrainQuadTreeNode.h>
#include <OgreTerrainMaterialGeneratorA.h>
#include <OgreTerrainPaging.h>
#include <OgreShadowCameraSetup.h>
#include <OgreShadowCameraSetupLiSPSM.h>
#include <OgreShadowCameraSetupPSSM.h>

using namespace Ogre;
#ifndef M_PI
#define M_PI	3.141592654
#endif 
#define PI  Ogre::Math::PI 
#define toStr(v)   Ogre::StringConverter::toString(v)
#define toStrC(v)  Ogre::StringConverter::toString(v).c_str()

#define Log(s)  Ogre::LogManager::getSingleton().logMessage(s);

#define s2r(s)  StringConverter::parseReal(s)
#define s2i(s)  StringConverter::parseInt(s)
#define s2c(s)  StringConverter::parseColourValue(s)
#define s2v(s)  StringConverter::parseVector3(s)

#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>

/// translation
#define _tr_(s) MyGUI::LanguageManager::getInstance().replaceTags(s)
