#ifndef _ChampsXml_h_
#define _ChampsXml_h_

#include <string>
#include <vector>
#include <map>


//  single track on championship
class ChampTrack
{
public:
	std::string name;  bool reversed;  // track
	int laps;  // number of laps
	float factor;  // time factor (difficulty) - how near to best time you need to drive
	ChampTrack();
};

//  one championship data
class Champ
{
public:
	std::string name, descr;  // champ description
	int diff;  // difficulty
	int ver;  // ver, if changed reset progress..
	float length, time;  // stats to display
	int tutorial;  // test2 / tutorial1 / championship0

	std::vector<ChampTrack> trks;
	Champ();
};


///  all championships and tutorials
//
class ChampsXml
{
public:
	std::vector<Champ> champs;
	std::map<std::string, float> trkTimes;  // track times (1 lap, best time)
	
	//  methods
	bool LoadXml(std::string file);
};


//  progress on single track
class ProgressTrack
{
public:
	float score;  //int laps;
	ProgressTrack();
};

//  progress on championship
class ProgressChamp
{
public:
	int curTrack;  // index to current track, in trks
	float score;
	
	//  for ver changed checking..
	std::string name;
	int ver;
	
	std::vector<ProgressTrack> trks;
	ProgressChamp();
};

///  progress on champs,tuts and their tracks
//
class ProgressXml
{
public:
	std::vector<ProgressChamp> champs;
	bool LoadXml(std::string file), SaveXml(std::string file);
};

#endif
