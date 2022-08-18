#pragma once
#include <Ogre.h>
// #include <OgreVector3.h>
// #include <OgreQuaternion.h>
#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
#include "../half.hpp"


struct Axes
{
	static void Init();
	static Ogre::Quaternion qFixCar,qFixWh;

	//  to ogre from vdrift
	static void toOgre(Ogre::Vector3& vOut, const MATHVECTOR<float,3>& vIn);
	static Ogre::Vector3 toOgre(const MATHVECTOR<float,3>& vIn);

	static Ogre::Quaternion toOgre(const QUATERNION<float>& vIn);  // car
	static Ogre::Quaternion toOgre(const QUATERNION<double>& vIn);
	static Ogre::Quaternion toOgreW(const QUATERNION<half_float::half>& vIn);  // wheels
	static Ogre::Quaternion toOgreW(const QUATERNION<float>& vIn);
	static Ogre::Quaternion toOgreW(const QUATERNION<double>& vIn);
};
