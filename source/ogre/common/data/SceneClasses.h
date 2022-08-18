#pragma once
#include "SColor.h"
#include <Ogre.h>
// #include <OgreCommon.h>
// #include <OgreVector2.h>
// #include <OgreVector3.h>
// #include <OgreVector4.h>
// #include <OgreQuaternion.h>
#include "../../../vdrift/mathvector.h"
#include "../../../vdrift/quaternion.h"

namespace Ogre {  class SceneNode;  class Entity;  class ParticleSystem;  }
namespace Forests {  class GrassLayer;  }


struct TerLayer		// terrain texture layer
{
	bool on, triplanar;  // for highest slopes
	float tiling;   // scale, texture repeat
	Ogre::String texFile, texNorm;  // textures _d, _n

	float dust, mud, dustS, smoke;  // particles intensities, S size
	SColor tclr;  Ogre::Vector4 tcl;  // trail color, rgba copy
	
	///  blendmap
	//  min,max range and smooth range for terrain angle and height
	float angMin,angMax,angSm, hMin,hMax,hSm;
	bool nOnly;  // ignores above range
	//  noise
	float noise, nprev, nnext2;   //  factors to blend layer +1,-1,+2
	float nFreq[2], nPers[2], nPow[2];  int nOct[2];
	float fDamage;  // car damage per sec
	
	//  surface params bind
	std::string surfName;  int surfId;
	TerLayer();
};


//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
class TerData		///  Terrain
{
public:	
	//  height field
	float* hfHeight;
	
	//  size
	int iVertsX, iVertsY, iTerSize;  // size in vertices
	float fTriangleSize, fTerWorldSize;  // scale size
	float errorNorm;  // terrain error % at default quality
	void UpdVals();

	//  layers	
	const static int ciNumLay = 6;  // all, for edit
	TerLayer layersAll[ciNumLay];
	std::vector<int> layers;  // active only (on)
	
	bool road1mtr;  // if true (default) road has only 1 surface type
	TerLayer layerRoad[4];  // pipe[4]  //todo...
	void UpdLayers();

	//  which should have triplanar most (eg high mountains)
	int triplanarLayer1, triplanarLayer2, triplCnt;
	float normScale;  // scale terrain normals

	bool emissive;  // emissive light from specular
	float specularPow, specularPowEm;  // specular power (exponent)
	
	//  methods
	TerData();	void Default();
	float getHeight(const float& fi, const float& fj);
};


class PagedLayer	// vegetation model
{
public:
	bool on;
	Ogre::String name;  float dens;
	float windFx, windFy;
	int addRdist, maxRdist;  // add,max dist to road
	float minScale, maxScale, ofsY;
	float maxTerAng, minTerH, maxTerH;  // terrain
	float maxDepth;  // in fluid
	int cnt;  // count on track, for stats
	PagedLayer();
};


class SGrassLayer	// grass layer
{
public:
	bool on;
	float dens;
	float minSx,minSy, maxSx,maxSy;  // sizes
	float swayDistr, swayLen, swaySpeed;  // sway
	int iChan;  // which channel to use
	Ogre::String material, colorMap;
	Forests::GrassLayer *grl;  // for update
	SGrassLayer();
};

class SGrassChannel  // grass channel
{
public:
	//  min,max range and smooth range for terrain angle and height
	float angMin,angMax,angSm, hMin,hMax,hSm;
	float noise, nFreq, nPers, nPow;  int nOct;  // noise params
	float rdPow;  // road border adjust
	SGrassChannel();
};


class FluidBox		/// fluid box shape - water, mud, etc.
{
public:
	Ogre::Vector3 pos, rot, size;
	Ogre::Vector2 tile;

	int id;  // auto set, index to FluidParams, -1 doesnt exist
	std::string name;

	class btCollisionObject* cobj;
	int idParticles;   // auto set  index for wheel particles  -1 none
	bool solid, deep;  // auto set, from FluidParams
	
	FluidBox();
};


class Object		// object - mesh (static) or prop (dynamic)
{
public:
	MATHVECTOR<float,3> pos;
	QUATERNION<float> rot;
	Ogre::Vector3 scale;
	std::string name;  // mesh file name

	Ogre::SceneNode* nd;  // ogre
	Ogre::Entity* ent;
	class btDefaultMotionState* ms;  // bullet
	class btCollisionObject* co;
	class btRigidBody* rb;
	bool dyn;
	class btTransform* tr1;  // 1st pos after load, for reset

	Object();
	void SetFromBlt();
};


class SEmitter		// particles
{
public:
	std::string name;  // particle_system
	Ogre::Vector3 pos, size;
	Ogre::Vector3 up;  float rot;  // dir
	float rate;  // emit

	float upd;  // update time for static
	bool stat;  // static for, e.g. clouds

	Ogre::SceneNode* nd;  // ogre
	Ogre::ParticleSystem* ps;
	
	SEmitter();
	void UpdEmitter();
};
