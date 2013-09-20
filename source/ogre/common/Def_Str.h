#pragma once
// -- NOT to be included in headers  --

#include <OgreLogManager.h>
#include <OgreStringConverter.h>


//  Log in ogre.log
#define LogO(s)  Ogre::LogManager::getSingleton().logMessage(s)


//  to string
#define toStr(v)   Ogre::StringConverter::toString(v)
#define toStrC(v)  Ogre::StringConverter::toString(v).c_str()


//  format int,float to string
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

//  string to var
#define s2r(s)  Ogre::StringConverter::parseReal(s)
#define s2i(s)  Ogre::StringConverter::parseInt(s)
#define s2c(s)  Ogre::StringConverter::parseColourValue(s)
#define s2v(s)  Ogre::StringConverter::parseVector3(s)
#define s2v4(s)  Ogre::StringConverter::parseVector4(s)

#define b2s(b)  (b) ? "true" : "false"


//  translation
#define TR(s)  MyGUI::LanguageManager::getInstance().replaceTags(s)


const int ciShadowSizesNum = 5;
const int ciShadowSizesA[ciShadowSizesNum] = {256,512,1024,2048,4096};
