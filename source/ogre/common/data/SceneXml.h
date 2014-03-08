#pragma once
#include <OgreCommon.h>
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreVector4.h>
#include <OgreColourValue.h>
#include <OgreQuaternion.h>
#include "../../../vdrift/mathvector.h"
#include "../../../vdrift/quaternion.h"

namespace Ogre {  class SceneNode;  class Entity;  }
namespace Forests {  class GrassLayer;  }


struct TerLayer		// terrain texture layer
{
	bool on, triplanar;
	float tiling;   // texture repeat
	Ogre::String texFile, texNorm;  // textures

	float dust, mud, dustS, smoke;  // particles intensities
	Ogre::ColourValue tclr;  // trail color
	
	//  blendmap
	//  min,max range and smooth range for terrain angle and height
	float angMin,angMax,angSm, hMin,hMax,hSm;
	bool nOnly;
	//  noise
	float noise, nprev, nnext2;   //  factors to blend layer +1,-1,+2
	float nFreq[2], nPers[2], nPow[2];  int nOct[2];
	
	//  surface params bind
	std::string surfName;  int surfId;
	TerLayer();
};


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
	TerLayer layerRoad;  // road[4]+pipe[4]  //todo...
	void UpdLayers();

	int triplanarLayer1, triplanarLayer2;  // which should have triplanar most (eg high mountains)
	float normScale;  // scale terrain normals
	bool emissive;
	
	//  methods
	TerData();	void Default();
	float getHeight(const float& fi, const float& fj);
};


class PagedLayer	// vegetation layer
{
public:
	bool on;
	Ogre::String name;  float dens;
	float windFx, windFy;
	int addRdist, maxRdist;  // add,max dist to road
	float minScale, maxScale, ofsY;
	float maxTerAng, minTerH, maxTerH;  // terrain
	float maxDepth;  // in fluid
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
	Ogre::Vector3 pos, rot, size;  Ogre::Vector2 tile;
	int id;  // auto set, index to FluidParams, -1 doesnt exist
	std::string name;
	class btCollisionObject* cobj;
	int idParticles;  // auto set  index for wheel particles  -1 none
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

	Object();
	void SetFromBlt();
	static Ogre::Quaternion qrFix,qrFix2;
};


class Scene		///  Scene  . . . . . . .
{
public:
	//  sky
	Ogre::String skyMtr;
	int  rainEmit,rain2Emit;  Ogre::String rainName,rain2Name;
	//  light
	float ldPitch, ldYaw;  // dir angles
	Ogre::Vector3 lDir, lAmb,lDiff,lSpec;

	//  fog
	float fogStart, fogEnd;  // lin range
	Ogre::Vector4 fogClr,fogClr2;  // 2colors sun-away  .a = intensity

	Ogre::Vector4 fogClrH;  // height fog color
	float fogHeight, fogHDensity, fogHStart, fogHEnd;


	//  game
	bool asphalt;  // use asphalt tires car
	bool denyReversed;  // track (road) dir

	float windAmt;  //, windDirYaw, windTurbulFreq,windTurbulAmp;
	float damageMul;  // reduce car damage in loops
	float gravity;  // 9.81


	//  particle types
	Ogre::String  sParDust, sParMud, sParSmoke;
	
	//  Terrain
	bool ter;  // has terrain
	bool vdr;  // vdrift track
	TerData td;

	
	//  Vegetation params
	float densTrees, densGrass;  int grDensSmooth;
	float grPage, grDist;
	float trPage, trDist, trDistImp;
	int trRdDist;  // dist from road to trees

	//  grass layers
	const static int ciNumGrLay = 6;  // all, for edit
	SGrassLayer grLayersAll[ciNumGrLay];
	SGrassChannel grChan[4];

	//  paged layers  (models: trees,rocks,etc)
	const static int ciNumPgLay = 10;  // all, for edit
	PagedLayer pgLayersAll[ciNumPgLay];
	std::vector<int> pgLayers;    // active only (on)
	void UpdPgLayers();

	
	//  preview cam
	Ogre::Vector3 camPos,camDir;
	
	//  to force regenerating impostors on different sceneries
	std::string sceneryId;
	
	//  Fuids
	std::vector<FluidBox> fluids;
	class FluidsXml* pFluidsXml;  // set this after Load
	
	//  Objects
	std::vector<Object> objects;
		
	//  methods
	Scene();  void Default(), UpdateFluidsId(), UpdateSurfId();
	class GAME* pGame;  // for all surfaces by name
	bool LoadXml(Ogre::String file, bool bTer = true), SaveXml(Ogre::String file);
};
