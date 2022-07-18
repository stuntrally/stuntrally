#pragma once
#include <string>
#include <vector>
#include <map>
#include <OgreStringVector.h>


//  single Track in challenge
class ChallTrack
{
public:
	std::string name = "Jng1-Curly";
	bool reversed = 0;  // track
	int laps = 1;  // number of laps

	//  pass  -1 means not needed, you can use one or more conditions
	float passPoints = -1, timeNeeded = -1;  int passPos = -1;

	//? bronze, silver, gold percent or val
};


///  one Challenge setup
class Chall
{
public:
	//  clean name,  description text,  nameGui with colors for Gui only
	std::string name = "none", descr = "none", nameGui = "none";
	int diff = 1;  // difficulty
	int ver = 1;   // version, if changed resets progress

	float length = 0.f; // stats for display
	int type = 0;       // easy, normal etc, for gui tab
	float time = 0.f;   // total computed (sum all tracks)
	int prizes = 2;     // 0 only gold  1 gold,silver  2 gold,silver,bronze
	float factor = 1.f; // multiplier for silver,bronze points/pos prizes
	
	//  Allowed type(s) or specific vehicle(s), 1 or more
	Ogre::StringVector carTypes, cars, carsDeny;
	int whMin = 0, whMax = 10;  // allowed vehicle wheels count range [min..max]

	std::vector<ChallTrack> trks;

	//  Game setup
	//  if empty or -1 then allows any
	std::string sim_mode = "normal";
	int damage_type = 2, boost_type = 1, flip_type = 2, rewind_type = 1;
	int dmg_lap = 40;  // dmg repair on lap

	//  Hud
	bool minimap = 1, chk_arr = 0, chk_beam = 0,
		trk_ghost = 1, pacenotes = 1, trail = 1;  // deny using it if false
	bool abs = 0, tcs = 0;  // deny if false
	
	//  Pass  -1 means not needed, you can use one or more conditions
	float totalTime = -1.f, avgPoints = -1.f, avgPos = -1.f;
	bool carChng = 0;  // allow car change

	// todo: int retries;  // max track restart/reset
	// todo: max rewinds count or time ..
};


///-----  all challenges
//
class ChallXml
{
public:
	std::vector<Chall> all;
	
	bool LoadXml(std::string file, class TracksXml* times, bool check);
};


//  progress on single track
class ProgressTrackL
{
public:
	float points = 0.f, time = 0.f;  int pos = 0;  // got after stage
};

//  progress on championship
class ProgressChall
{
public:
	std::string car;   // picked car at start
	int curTrack = 0;  // index to trks
	float avgPoints = 0.f, totalTime = 0.f, avgPos = 0;  // computed from all stages
	int fin = -1;      // final prize  -1 none, 0 bronze, 1 silver, 2 gold
	
	std::string name;
	int ver = 0;
	
	std::vector<ProgressTrackL> trks;
};


///-----  progress L=challenges
//
class ProgressLXml
{
public:
	std::vector<ProgressChall> chs;
	bool LoadXml(std::string file), SaveXml(std::string file);
};
