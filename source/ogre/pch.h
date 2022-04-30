#ifndef __PCH_H__
#define __PCH_H__

//  this is the precompiled header file for VS, only for Windows build
#ifdef _MSC_VER
// include file for project specific include files that are used frequently, but are changed infrequently

///  win
#define WINVER 0x0510
#define _WIN32_WINNT 0x0510
#define _WIN32_WINDOWS 0x0410
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <process.h>

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
#include <math.h>

#include "tinyxml.h"
#include "tinyxml2.h"
#include "half.hpp"

///  SDL, Sound
#include <SDL.h>
#include <vorbis/vorbisfile.h>
///  Ogre
#include <Ogre.h>
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreWindowEventUtilities.h>

#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreTerrainQuadTreeNode.h>
#include <OgreTerrainMaterialGeneratorA.h>
#include <OgreTerrainPaging.h>

#include "btBulletDynamicsCommon.h"
#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>

#endif
#endif