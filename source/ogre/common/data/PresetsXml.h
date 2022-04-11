#pragma once
#include "SceneXml.h"  // need SColor
#include <OgreString.h>


//  Presets xml  for editor
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
