#pragma once
#include <string>
#include <vector>
#include <map>
#include <OgreStringVector.h>


//  single Track in challenge
class ChallTrack
{
public:
	std::string name;  bool reversed;  // track
	int laps;  // number of laps

	//  pass  -1 means not needed, you can use one or more conditions
	float passPoints, timeNeeded;  int passPos;

	//todo: bronze, silver, gold ?percent or val
	ChallTrack();
};

///  one Challenge setup
class Chall
{
public:
	std::string name, descr;  // description text
	int diff;  // difficulty
	int ver;  // version, if changed resets progress

	float length;  // stats for display
	int type;  // easy, normal etc, for gui tab
	float time;  // total computed (sum all tracks)
	int prizes;  // 0 only gold  1 gold,silver  2 gold,silver,bronze
	float factor;  // multiplier for silver,bronze points/pos prizes
	
	//  allowed types or cars 1 or more
	Ogre::StringVector carTypes, cars;

	std::vector<ChallTrack> trks;

	//  game setup
	//  if empty or -1 then allows any
	std::string sim_mode;
	int damage_type, boost_type, flip_type, rewind_type;
	int dmg_lap;  // dmg repair on lap

	//  hud
	bool minimap, chk_arr, chk_beam,
		trk_ghost, pacenotes;  // deny using it if false
	bool abs,tcs;  // deny if false
	
	//  pass  -1 means not needed, you can use one or more conditions
	float totalTime, avgPoints, avgPos;
	bool carChng;  // allow car change

	// autoshift, autorear
	// max dmg%, off road time-
	//..int retries;  // max track restart/reset

	Chall();
};


///-----  all challenges
//
class ChallXml
{
public:
	std::vector<Chall> all;
	
	bool LoadXml(std::string file, class TracksXml* times, bool check);
	ChallXml();
};


//  progress on single track
class ProgressTrackL
{
public:
	float points, time;  int pos;  // got after stage
	ProgressTrackL();
};

//  progress on championship
class ProgressChall
{
public:
	std::string car;  // picked car at start
	int curTrack;  // index to trks
	float avgPoints, totalTime, avgPos;  // computed from all stages
	int fin;  // final prize -1 none, 0 bronze, 1 silver, 2 gold
	
	std::string name;
	int ver;
	
	std::vector<ProgressTrackL> trks;
	ProgressChall();
};


///-----  progress L=challenges
//
class ProgressLXml
{
public:
	std::vector<ProgressChall> chs;
	bool LoadXml(std::string file), SaveXml(std::string file);
};
