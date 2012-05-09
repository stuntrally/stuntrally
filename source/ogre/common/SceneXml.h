#ifndef _SceneXml_h_
#define _SceneXml_h_

#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreVector2.h>
#include <OgreColourValue.h>
#include <OgreQuaternion.h>
#include "../../vdrift/mathvector.h"
#include "../../vdrift/quaternion.h"

namespace Ogre {  class SceneNode;  class Entity;  }


struct TerLayer		// terrain texture layer
{
	bool on;
	float tiling;   // texture repeat
	Ogre::String texFile, texNorm;  // textures

	float dust, mud, dustS, smoke;  // particles intensities
	Ogre::ColourValue tclr;  // trail color
	
	//  min,max range and smooth range for angle and height for blendmap
	float angMin,angMax,angSm, hMin,hMax,hSm;
	float noise;  bool bNoiseOnly;  // blendmap noise

	TerLayer();
};


class TerData		///  Terrain
{
public:	
	//  height field
	float* hfHeight,*hfAngle;
	
	//  size
	int iVertsX, iVertsY, iTerSize;  // size in vertices
	float fTriangleSize, fTerWorldSize;  // scale size
	void UpdVals();

	//  layers	
	const static int ciNumLay = 6;  // all, for edit
	TerLayer layersAll[ciNumLay];
	std::vector<int> layers;  // active only (on)
	TerLayer layerRoad;
	void UpdLayers();
	
	//  methods
	TerData();	void Default();
	float getHeight(const float& fi, const float& fj);
};


class PagedLayer	// vegetation layer
{
public:
	bool on;
	Ogre::String name;  Ogre::Real dens;
	Ogre::Real windFx, windFy;
	int addTrRdDist;  // dist to road
	Ogre::Real minScale, maxScale, ofsY;
	Ogre::Real maxTerAng, minTerH, maxTerH;  // terrain
	Ogre::Real maxDepth;  // in fluid
	
	PagedLayer();
};


class FluidBox		// fluid box shape - water, mud, etc.
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
	MATHVECTOR <float, 3> pos;
	QUATERNION <float> rot;
	Ogre::Vector3 scale;
	std::string name;  // mesh file name

	Ogre::SceneNode* nd;  // ogre
	Ogre::Entity* ent;  
	class btDefaultMotionState* ms;  // bullet
	class btCollisionObject* cobj;

	Object();
	void SetFromBlt();
	static Ogre::Quaternion qrFix,qrFix2;
};


class Scene		///  Scene
{
public:
	//  sky
	Ogre::String skyMtr;
	int  rainEmit,rain2Emit;  Ogre::String rainName,rain2Name;
	//  fog
	Ogre::FogMode fogMode;  Ogre::Vector3 fogClr;
	Ogre::Real  fogExp, fogStart, fogEnd;
	//  light
	Ogre::Real ldPitch, ldYaw;  // dir angles
	Ogre::Vector3 lDir, lAmb,lDiff,lSpec;

	//  particles
	Ogre::String  sParDust, sParMud, sParSmoke;
	
	//  terrain
	bool ter;  // 1 own, has terrain, 0 vdrift track
	TerData td;

	//  paged layers	
	const static int ciNumPgLay = 10;  // all, for edit
	PagedLayer pgLayersAll[ciNumPgLay];
	std::vector<int> pgLayers;    // active only (on)
	void UpdPgLayers();
	
	//  paged
	Ogre::Real densTrees, densGrass;  int grDensSmooth;

	//  grass  -todo layers..
	Ogre::Real grPage, grDist;  // vis
	Ogre::Real grMinSx,grMinSy, grMaxSx,grMaxSy;  // sizes
	Ogre::Real grSwayDistr, grSwayLen, grSwaySpeed;  // sway
	Ogre::Real grTerMaxAngle, grTerMaxHeight;
	Ogre::String grassMtr, grassColorMap;

	//  trees
	Ogre::Real trPage, trDist, trDistImp;  // vis
	int  trRdDist;  // dist from road to trees
	
	//  preview cam
	Ogre::Vector3 camPos,camDir;
	
	//  to force regenerating impostors on different sceneries
	int sceneryId;
	
	//  fuids
	std::vector<FluidBox> fluids;
	class FluidsXml* pFluidsXml;  // set this after Load
	
	//  objects
	std::vector<Object> objects;
		
	//  methods
	Scene();  void Default(), UpdateFluidsId();
	bool LoadXml(Ogre::String file), SaveXml(Ogre::String file);
};

#endif
