#ifndef _ChampsXml_h_
#define _ChampsXml_h_

#include <string>
#include <vector>
#include <map>


class ChampTrack
{
public:
	std::string name;
	int laps;  float factor;

	ChampTrack();
};

class Champ
{
public:
	std::string name, descr;  int ver;
	int diff;  float length,time;  bool tutorial;

	std::vector<ChampTrack> trks;
	Champ();
};


//  all championships and tutorials
class ChampsXml
{
public:
	std::vector<Champ> champs;
	std::map<std::string, float> trkTimes;  // track times (1 lap, best time)
	
	//  methods
	bool LoadXml(std::string file);
};


//  progress on champs,tuts and their tracks
class ProgressTrack
{
public:
	float score;  //int laps;
	ProgressTrack();
};

class ProgressChamp
{
public:
	int curTrack;  float score;
	std::vector<ProgressTrack> trks;
	ProgressChamp();
};

class ProgressXml
{
public:
	std::vector<ProgressChamp> champs;
	bool LoadXml(std::string file), SaveXml(std::string file);
};

#endif
