#pragma once
#include <vector>
#include <map>
#include <string>

namespace tinyxml2	{	class XMLElement;	}


struct Date
{
	int day,month,year;
	bool operator < (const Date& d) const
	{
		return year < d.year && month < d.month && day < d.day;
	}
	Date() : day(1),month(1),year(10) {  }
};


//  Track's additional info
//  shown on gui [Track] tab, in detailed view
//  for sorting by date, difficulty, etc.
//--------------------------------------------------------------------
class TrackInfo
{
public:
	int n;  // unique id for track, from ini
	float crtver;  // created in ver
	int ver;  // x10
	std::string name, nshrt,  scenery, author;
	Date created, modified;

	//  track characteristics  (char)
	int fluids,bumps, jumps,loops,pipes;
	int	banked,frenzy,longn, objects,obstacles;
	int diff, rating, sum;

	int nn;  // number got from name eg. for D15-.. it is 15
	bool test,testC;  // starts with Test.. or TestC..
	bool vdrift;  // from author name

	TrackInfo();
};


//  all tracks infos
//.................................
class TracksXml
{
public:
	std::vector<TrackInfo> trks;
	std::map<std::string, int> trkmap;  // 0 if not found
	std::map<std::string, float> times;  // track times
	
	int cntAll;
	bool LoadIni(std::string file, bool check);
	TracksXml()
		:cntAll(0)
	{	}
};


//  user Track's info
//  rating, stats ...  not yet used
//-------------------------------------
class UserTrkInfo
{
public:
	std::string name;
	int rating;
	Date last;  // driven
	int laps;  float time;

	UserTrkInfo();
};


//  user xml (tracks)
class UserXml
{
public:
	std::vector<UserTrkInfo> trks;
	std::map<std::string, int> trkmap;  // 0 if not found
	
	//  methods
	bool LoadXml(std::string file), SaveXml(std::string file);
	UserXml() {  }
};


//  Car's additional info
//  shown on gui [Car] tab, in detailed view
//  for sorting by speed, type, etc.
//--------------------------------------------------------------------
class CarInfo
{
public:
	std::string id, type, author;
	bool car;

	float speed;
	int n, year, rating;
	
	//  time mul factors, for race postion, in sim modes
	float easy, norm;

	CarInfo();
};


//  all cars infos
//.................................
class CarsXml
{
public:
	std::vector<CarInfo> cars;
	std::map<std::string, int> carmap;  // 0 if not found
	std::map<std::string, std::string> colormap;  // car type to list color
	float magic;
	
	//  methods
	bool LoadXml(std::string file);
	CarsXml()
		:magic(0.010f)
	{	}
};


//  car colors.ini  for Gui
//--------------------------------------------------------------------
class CarColor
{
public:
	float hue, sat, val, refl, gloss;
};


//  all colors
class ColorsXml
{
public:
	std::vector<CarColor> v;
	int perRow, imgSize;  // gui params
	
	//  methods
	bool LoadIni(std::string file);
	ColorsXml()
		:perRow(12), imgSize(18)
	{	}
};


//  Reverb presets on sceneries  ed combo
//--------------------------------------------------------------------
struct ReverbSet
{
	std::string name, descr,  // info
		normal, cave, cavebig, pipe, pipebig, influid;  // reverb preset names
};

class ReverbsXml
{
public:
	ReverbSet base;  // inherited defaults if not set
	std::vector<ReverbSet> revs;
	std::map<std::string, int> revmap;  // 0 if not found
	
	//  methods
	bool LoadXml(std::string file);
	void GetParams(tinyxml2::XMLElement* e, ReverbSet& r);
	ReverbsXml()
	{	}
};
