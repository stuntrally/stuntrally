#ifndef _SceneXml_h_
#define _SceneXml_h_

using namespace Ogre;


struct TerLayer		// terrain texture layer
{
	bool on;
	float tiling;
	String texFile, texNorm;  // textures
	float dust, mud, dustS, smoke;
	ColourValue tclr;

	TerLayer();
};


class TerData	///  Terrain
{
public:	
	//  height field
	float* hfData;
	
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
	String name;  Real dens;
	Real windFx, windFy;  int addTrRdDist;
	Real minScale, maxScale, ofsY;
	
	PagedLayer();
};


class Scene		///  Scene
{
public:
	//  sky
	String skyMtr;
	int  rainEmit,rain2Emit;  String rainName,rain2Name;
	//  fog
	FogMode fogMode;  Vector3 fogClr;
	Real  fogExp, fogStart, fogEnd;
	//  light
	Real ldPitch, ldYaw;  // dir angles
	Vector3 lDir;  Vector3 lAmb,lDiff,lSpec;

	//  particles
	String  sParDust, sParMud, sParSmoke;
	
	//  terrain
	bool ter;  // 1 own, has terrain, 0 vdrift track
	TerData td;

	//  paged layers	
	const static int ciNumPgLay = 10;  // all, for edit
	PagedLayer pgLayersAll[ciNumPgLay];
	std::vector<int> pgLayers;    // active only (on)
	void UpdPgLayers();
	
	//  paged
	Real densTrees, densGrass;

	//  grass  -todo layers..
	Real grPage, grDist;  // vis
	Real grMinSx,grMinSy, grMaxSx,grMaxSy;  // sizes
	Real grSwayDistr, grSwayLen, grSwaySpeed;  // sway

	//  trees
	Real trPage, trDist, trDistImp;  // vis
	int  trRdDist;  // dist from road to trees
	
	//  preview cam
	Vector3 camPos,camDir;
		
	//  methods
	Scene();  void Default();
	bool LoadXml(String file), SaveXml(String file);
};

#endif