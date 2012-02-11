#ifndef _TracksXml_h_
#define _TracksXml_h_

#include <OgreCommon.h>


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
	int n;  float crtver;
	std::string name, scenery, author;
	Date created, modified;

	// track characteristics  (char)
	int fluids,bumps,jumps,loops,pipes,banked,frenzy,longn;
	int diff, rating;
	int rateuser, drivenlaps;  // todo: user info ...

	TrackInfo();
};


//  all tracks infos
class TracksXml
{
public:
	std::vector<TrackInfo> trks;
	std::map<std::string, int> trkmap;  // 0 if not found
	
	//  methods
	//TracksXml();  void Default();
	bool LoadXml(Ogre::String file), SaveXml(Ogre::String file);
};


//  tracks list item - with info for sorting
struct TrkL
{
	std::string name;
	const TrackInfo* ti;
	const class App* pA;
	bool test;  //Test*
};

#endif