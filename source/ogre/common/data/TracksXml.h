#pragma once
#include <vector>
#include <map>
#include <string>


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

class TrackInfo
{
public:
	int n;  // unique id for track, from ini
	float crtver;  // created in ver
	std::string name, scenery, author;
	Date created, modified;

	//  track characteristics  (char)
	int fluids,bumps,jumps,loops,pipes,banked,frenzy,longn,objects;
	int diff, rating;

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
	
	bool LoadIni(std::string file);
	TracksXml() {  }
};


//--------------------------------------------------------------------

//  user Track's info
//  rating, stats ...

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


//--------------------------------------------------------------------

//  Car's additional info
//  shown on gui [Car] tab, in detailed view
//  for sorting by speed, type, etc.

class CarInfo
{
public:
	std::string id, type, author;

	int n, speed, year, rating;
	int rateuser, drivenlaps;  // todo: user info ...
	
	float easy, norm;  // time mul factors, for race postion, in sim modes

	CarInfo();
};


//  all tracks infos
class CarsXml
{
public:
	std::vector<CarInfo> cars;
	std::map<std::string, int> carmap;  // 0 if not found
	std::map<std::string, std::string> colormap;  // car type to list color
	float magic;
	
	//  methods
	bool LoadXml(std::string file);
	CarsXml(){  }
};
