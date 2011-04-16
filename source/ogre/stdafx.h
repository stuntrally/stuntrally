#pragma once

///  std
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <set>
#include <cassert>

#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <time.h>

///  win
#include <OgrePlatform.h>
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	#define WINVER 0x0510
	#define _WIN32_WINNT 0x0510
	#define _WIN32_WINDOWS 0x0410
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <process.h>
#endif

#include "tinyxml.h"
//#include <math.h>
#ifndef M_PI
#define M_PI	3.141592654
#endif

///  SDL, Sound
#include <SDL.h>
#include <vorbis/vorbisfile.h>

///  Ogre
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreWindowEventUtilities.h>

#define PI  Ogre::Math::PI
#define toStr(v)  Ogre::StringConverter::toString(v)
#define toStrC(v)  Ogre::StringConverter::toString(v).c_str()

#define Log(s)  Ogre::LogManager::getSingleton().logMessage(s);

#define s2r(s)  StringConverter::parseReal(s)
#define s2i(s)  StringConverter::parseInt(s)
#define s2c(s)  StringConverter::parseColourValue(s)
#define s2v(s)  StringConverter::parseVector3(s)

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreTerrainQuadTreeNode.h>
#include <OgreTerrainMaterialGeneratorA.h>
#include <OgreTerrainPaging.h>

#include "btBulletDynamicsCommon.h"
#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>

/// translation
#define TR(s) MyGUI::LanguageManager::getInstance().replaceTags(s)
