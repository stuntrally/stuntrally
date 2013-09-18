#pragma once

#define PI_d  3.14159265358979323846

#include <OgreLogManager.h>
#define LogO(s)  Ogre::LogManager::getSingleton().logMessage(s)

#include <OgreStringConverter.h>
#define toStr(v)   Ogre::StringConverter::toString(v)
#define toStrC(v)  Ogre::StringConverter::toString(v).c_str()

static Ogre::String iToStr(const int v, const char width=0)
{
	std::ostringstream s;
	if (width != 0)  s.width(width);  //s.fill(fill);
	s << std::fixed << v;
	return s.str();
}
static Ogre::String fToStr(const float v, const char precision, const char width=0, const char fill=' ')
{
	std::ostringstream s;
	if (width != 0)  s.width(width);  s.fill(fill);
	s << std::fixed << std::setprecision(precision) << v;
	return s.str();
}

#define s2r(s)  Ogre::StringConverter::parseReal(s)
#define s2i(s)  Ogre::StringConverter::parseInt(s)
#define s2c(s)  Ogre::StringConverter::parseColourValue(s)
#define s2v(s)  Ogre::StringConverter::parseVector3(s)
#define s2v4(s)  Ogre::StringConverter::parseVector4(s)
#define b2s(b)  (b) ? "true" : "false"

/// translation
#define TR(s)  MyGUI::LanguageManager::getInstance().replaceTags(s)


///  boost fuel params  ----
const static float gfBoostFuelStart = 3.f,  // seconds (each lap)
	gfBoostFuelMax = 3.f,  // max val, tank	
	gfBoostFuelAddSec = 0.1f;  // add value each second

const int ciShadowNumSizes = 5;
const int ciShadowSizesA[ciShadowNumSizes] = {256,512,1024,2048,4096};
