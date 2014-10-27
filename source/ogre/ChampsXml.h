#pragma once
#include <string>
#include <vector>
#include <map>


//  single track on championship
class ChampTrack
{
public:
	std::string name;  bool reversed;  // track
	int laps;  // number of laps
	//float factor;  // time factor (difficulty) - how near to best time you need to drive
	float passScore;
	ChampTrack();
};

//  one championship data
class Champ
{
public:
	std::string name, descr;  // champ description
	int diff;  // difficulty
	int ver;  // ver, if changed reset progress..
	float length;  // stats to display
	int type;  // tutorial, champ easy, normal etc
	float time;  // total, computed (sum of all tracks)

	std::vector<ChampTrack> trks;
	Champ();
	bool isTut() const {  return type < 2;  }
};


///-----  all championships and tutorials
//
class ChampsXml
{
public:
	std::vector<Champ> all;
	
	bool LoadXml(std::string file, class TracksXml* times, bool check);
	ChampsXml();
};


//  progress on single track
class ProgressTrack
{
public:
	float points;
	ProgressTrack();
};

//  progress on championship
class ProgressChamp
{
public:
	int curTrack;  // index to current track, in trks
	float points;
	
	//  for ver changed checking..
	std::string name;
	int ver;
	
	std::vector<ProgressTrack> trks;
	ProgressChamp();
};


///-----  progress on champs,tuts and their tracks
//
class ProgressXml
{
public:
	std::vector<ProgressChamp> chs;
	bool LoadXml(std::string file), SaveXml(std::string file);
};
