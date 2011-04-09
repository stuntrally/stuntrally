#ifndef _BulletCollision_h_
#define _BulletCollision_h_

#include <OgreVector3.h>  // for Vector3


enum eBLTshape {  BLT_None=0, BLT_Sphere, BLT_CapsZ, BLT_ALL };
static string sBLTshape[BLT_ALL] = {"", "sphere", "capsZ" };


struct BltShape
{
	eBLTshape type;
	float radius, height;  // dims
	Ogre::Vector3 offset;  //Quaternion rot;  // pos, rot-
	float friction, restitution;  // collision pars
	
	BltShape();
};


class BltCollision
{
public:
	std::string mesh;  // for vegetation eg. tree.mesh
	Ogre::Vector3 offset;  // real mesh center, for placing on terrain
	std::vector <BltShape> shapes;
	
	BltCollision();
};


class BltObjects
{
public:
	//std::vector<BltCollision> cols;
	std::map <std::string, BltCollision> colsMap;
	std::map <std::string, BltCollision>::const_iterator colsMapFind;

	bool LoadXml();
	const BltCollision* Find(string mesh);
};


#endif