#ifndef _ChallengesXml_h_
#define _ChallengesXml_h_

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
	
	//  allowed types or cars 1 or more
	Ogre::StringVector carTypes, cars;

	std::vector<ChallTrack> trks;

	//  game setup
	//  if empty or -1 then allows any
	std::string sim_mode;
	int damage_type, boost_type, flip_type, rewind_type;

	//  hud
	bool minimap, chk_arr, chk_beam, trk_ghost;  // deny using it if false
	
	//  pass  -1 means not needed, you can use one or more conditions
	float totalTime, avgPoints, avgPos;

	// abs, tcs, autoshift, autorear
	// max dmg%, off road time-

	Chall();
};


///-----  all challenges
//
class ChallXml
{
public:
	std::vector<Chall> all;
	
	bool LoadXml(std::string file, class TimesXml* times);
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
	std::string car;  //todo: picked car ..
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

#endif
