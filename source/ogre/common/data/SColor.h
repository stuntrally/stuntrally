#pragma once
#include <Ogre.h>
// #include <OgreCommon.h>
// #include <OgreVector3.h>
// #include <OgreVector4.h>
// #include <OgreColourValue.h>
#include <string>


//  custom Color . . . . . . .
class SColor
{
public:
	// hue,sat 0..1   a = anything
	//  val    0..a   over 1 are additive, eg. bright desert
	//  alpha  0..a   over 1 are additive, for light fog
	//  neg    0..a   gives negative offset, for darkening, antilight
	float h, s, v,  a,  n;

	//  load from old and convert, ver < 2.4
	void LoadRGB(Ogre::Vector3 rgb);  //  can be -a..a
	
	//  get rgba value for shaders
	Ogre::Vector3 GetRGB() const;
	Ogre::Vector3 GetRGB1() const;  // limited to 0..1 for image
	Ogre::Vector4 GetRGBA() const;
	Ogre::ColourValue GetClr() const;

	//  from string, old  r g b,  r g b a,  or  h s v a n
	void Load(const char* s);
	std::string Save() const;
	std::string Check(std::string t);

	SColor();
	SColor(float h, float s, float v, float a=1.f, float n=0.f);
};
