#ifndef _ChallengesXml_h_
#define _ChallengesXml_h_

#include <string>
#include <vector>
#include <map>
#include <OgreStringVector.h>


//  single track on challenge
class ChallTrack
{
public:
	std::string name;  bool reversed;  // track
	int laps;  // number of laps
	float passPoints, timeNeeded;  // if n/a then -1
	//todo: bronze, silver, gold ?percent or val
	ChallTrack();
};

//  one challegne data
class Chall
{
public:
	std::string name, descr;  // description text
	int diff;  // difficulty
	int ver;  // version, if changed resets progress

	float length;  // stats for display
	int type;  // tutorial, easy, normal etc
	float time;  // total computed (sum all tracks)
	
	//  allowed types or cars 1 or more
	Ogre::StringVector carTypes, cars;

	std::vector<ChallTrack> trks;

	//  if empty or -1 then allows any
	std::string sim_mode;
	int damage_type, boost_type, flip_type, rewind_type;

	bool minimap, chk_arr, chk_beam, trk_ghost;  // deny using it if true
	float total_time, avg_pos;  // total none if -1

	// abs, tcs, autoshift, autorear
	// max dmg%, off road time-

	Chall();
};


///-----  all challenges
//
class ChallXml
{
public:
	std::vector<Chall> ch;
	
	bool LoadXml(std::string file, class TimesXml* times);
};


//  progress on single track
class ProgressTrackL
{
public:
	float points;
	ProgressTrackL();
};

//  progress on championship
class ProgressChall
{
public:
	int curTrack;  // index to trks
	float points;
	
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
	std::vector<ProgressChall> ch;
	bool LoadXml(std::string file), SaveXml(std::string file);
};

#endif
