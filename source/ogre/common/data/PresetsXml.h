#pragma once
#include "SceneXml.h"  // need SColor
#include <OgreString.h>


//  Presets xml  for editor Pick lists
//-----------------------------------------------------------

///----  Sky
struct PSky
{
	int rate = 0;  // rating  for quality, popular
	Ogre::String mtr = "sky", clr = "#C0E0FF";
	float ldYaw = 0.f, ldPitch = 0.f;  // sun dir
};

///----  Terrain layer
struct PTer
{
	int rate = 0;
	Ogre::String texFile, texNorm, sc;
	std::string surfName = "Default", scn;

	float tiling = 8.f;  bool triplanar = false;

	float dust = 0.f, mud = 0.2f, dustS = 0.f;
	SColor tclr = SColor(0.2f, 0.2f, 0.1f, 0.6f);  // trail

	float angMin = 0.f, angMax = 90.f;
	float dmg = 0.f;
};

///----  Road
struct PRoad
{
	int rate = 0;
	Ogre::String mtr, sc;
	std::string surfName, scn;

	float dust = 0.f, mud = 0.2f, dustS = 0.f;
	SColor tclr = SColor(0.2f, 0.2f, 0.1f, 0.6f);  // trail
};

///----  Grass
struct PGrass
{
	int rate = 0;
	//  material, colormap, scenery
	Ogre::String mtr, clr = "GrassClrJungle", sc;
	std::string scn;

	float minSx = 1.2f, minSy = 1.2f;
	float maxSx = 1.6f, maxSy = 1.6f;  // sizes
};

///----  Veget model
struct PVeget
{
	int rate = 0;
	Ogre::String sc;
	std::string name, scn;

	float minScale = 0.6f, maxScale = 1.2f;
	float windFx = 0.02f, windFy = 0.002f;

	int addRdist = 1;  // road dist
	float maxTerAng = 30.f;  // terrain
	float maxDepth = 0.f;  // in fluid
};


///  Presets xml  with common params setup
//-----------------------------------------------------------
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
