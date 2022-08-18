#pragma once
#include <vector>
#include <string>
#include <map>

#include <Ogre.h>
// #include <OgreVector3.h>


enum eBLTshape {  BLT_None=0, BLT_Sphere, BLT_CapsZ, BLT_Mesh, BLT_ALL };
static std::string sBLTshape[BLT_ALL] = {"", "sphere", "capsZ", "mesh" };


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
	std::map <std::string, bool> colNone;
	std::map <std::string, BltCollision> colsMap;
	std::map <std::string, BltCollision>::const_iterator colsMapFind;

	BltShape defPars;  // default if not found, for trimesh frict,rest params..

	bool LoadXml();
	const BltCollision* Find(std::string mesh);

	BltObjects();
};
