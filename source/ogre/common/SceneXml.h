#ifndef _SceneXml_h_
#define _SceneXml_h_

#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreColourValue.h>


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
	static int GENERATE_HMAP;
};


class PagedLayer	// vegetation layer
{
public:
	bool on;
	Ogre::String name;  Ogre::Real dens;
	Ogre::Real windFx, windFy;  int addTrRdDist;
	Ogre::Real minScale, maxScale, ofsY;
	
	PagedLayer();
};


class FluidBox		// fluid box shape - water, mud, etc.
{
public:
	Ogre::Vector3 pos, rot, size;  Ogre::Vector2 tile;
	int type;
	class btCollisionObject* cobj;
	//float density, linDamp, angDamp;
	//sinkDamp wheel spin move pars..
	FluidBox();
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

	//  trees
	Ogre::Real trPage, trDist, trDistImp;  // vis
	int  trRdDist;  // dist from road to trees
	
	//  preview cam
	Ogre::Vector3 camPos,camDir;
	
	std::vector<FluidBox> fluids;
		
	//  methods
	Scene();  void Default();
	bool LoadXml(Ogre::String file), SaveXml(Ogre::String file);
};

#endif