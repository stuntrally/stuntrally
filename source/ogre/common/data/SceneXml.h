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


//  Presets
//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

///----  Sky
struct PSky
{
	Ogre::String mtr, clr;
	float ldYaw, ldPitch;  // sun dir
	PSky();
};

///----  Terrain layer
struct PTer
{
	Ogre::String texFile, texNorm, sc;
	std::string surfName, scn;
	float tiling;  bool triplanar;

	float dust, mud, dustS;
	SColor tclr;  // trail

	float angMin,angMax;
	float dmg;
	PTer();
};

///----  Road
struct PRoad
{
	Ogre::String mtr, sc;
	std::string surfName, scn;

	float dust, mud, dustS;
	SColor tclr;
	PRoad();
};

///----  Grass
struct PGrass
{
	Ogre::String mtr, clr, sc;  // material, colormap
	std::string scn;
	float minSx,minSy, maxSx,maxSy;  // sizes
	PGrass();
};

///----  Veget model
struct PVeget
{
	Ogre::String sc;
	std::string name, scn;
	float minScale, maxScale;
	float windFx, windFy;

	int addRdist;  // road dist
	float maxTerAng;  // terrain
	float maxDepth;  // in fluid
	PVeget();
};

///  Presets xml  with common params setup
class Presets
{
public:
	std::vector<PSky> sky;
	std::map<std::string, int> isky;
	const PSky* GetSky(std::string mtr);
	
	std::vector<PTer> ter;
	std::map<std::string, int> iter;
	const PTer* GetTer(std::string tex);
	
	std::vector<PRoad> rd;
	std::map<std::string, int> ird;
	const PRoad* GetRoad(std::string mtr);
	
	std::vector<PGrass> gr;
	std::map<std::string, int> igr;
	const PGrass* GetGrass(std::string mtr);
	
	std::vector<PVeget> veg;
	std::map<std::string, int> iveg;
	const PVeget* GetVeget(std::string mesh);

	bool LoadXml(std::string file);
};


class FluidBox		/// fluid box shape - water, mud, etc.
{
public:
	Ogre::Vector3 pos, rot, size;
	Ogre::Vector2 tile;

	int id;  // auto set, index to FluidParams, -1 doesnt exist
	std::string name;

	class btCollisionObject* cobj;
	int idParticles;  // auto set  index for wheel particles  -1 none
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


///  Scene setup xml
//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
class Scene
{
public:
	//  car start pos
	MATHVECTOR <float,3> startPos;
	QUATERNION <float>   startRot;
	std::pair <MATHVECTOR<float,3>, QUATERNION<float> > GetStart(int index);

	//  sky
	Ogre::String skyMtr;  float skyYaw;
	int  rainEmit,rain2Emit;  Ogre::String rainName,rain2Name;
	//  light
	float ldPitch, ldYaw;  // sun dir angles
	SColor lAmb,lDiff,lSpec;

	//  fog
	float fogStart, fogEnd;  // lin range
	SColor fogClr,fogClr2;  // 2colors sun-away  .a = intensity

	SColor fogClrH;  // height fog color
	float fogHeight, fogHDensity, fogHStart, fogHEnd;
	float fHDamage;  // damage from low height fog


	//  game
	bool asphalt;  // use asphalt tires car
	bool denyReversed;  // track (road) dir
	bool noWrongChks;  // dont show "wrong checkpoint" messages

	float windAmt;  //, windDirYaw, windTurbulFreq,windTurbulAmp;
	float damageMul;  // reduce car damage in loops
	float gravity;  // 9.81
	
	//  sound
	std::string sAmbient, sReverbs;  void UpdRevSet();
	struct RevSet  // copy from ReverbSet, name = sReverbs, from base if ""
	{	std::string descr,
			normal, cave, cavebig, pipe, pipebig, influid;
	} revSet;
	class ReverbsXml* pReverbsXml;  //! set this after Load
	

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
	
	//  Fluids
	std::vector<FluidBox> fluids;
	class FluidsXml* pFluidsXml;  //! set this after Load
	
	//  Objects
	std::vector<Object> objects;
	
	//  base track (new from) for info
	std::string baseTrk;
	int secEdited;  // time in seconds of track editing for info

	
	//  methods
	Scene();  void Default();
	void UpdateFluidsId(), UpdateSurfId();

	class GAME* pGame;  // for all surfaces by name
	bool LoadStartPos(Ogre::String file);
	bool LoadXml(Ogre::String file, bool bTer = true), SaveXml(Ogre::String file);
};
