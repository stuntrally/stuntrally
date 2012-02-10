#ifndef __Defines_h__
#define __Defines_h__


#define PI_d  3.14159265358979323846

#include <OgreStringConverter.h>
#define LogO(s)  Ogre::LogManager::getSingleton().logMessage(s);


#define toStr(v)   Ogre::StringConverter::toString(v)
#define toStrC(v)  Ogre::StringConverter::toString(v).c_str()

#define s2r(s)  Ogre::StringConverter::parseReal(s)
#define s2i(s)  Ogre::StringConverter::parseInt(s)
#define s2c(s)  Ogre::StringConverter::parseColourValue(s)
#define s2v(s)  Ogre::StringConverter::parseVector3(s)

/// translation
#define TR(s)  MyGUI::LanguageManager::getInstance().replaceTags(s)


#endif
