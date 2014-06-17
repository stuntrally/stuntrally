#include "pch.h"
#include "../Def_Str.h"
#include "TracksXml.h"
#include "tinyxml.h"
#include "tinyxml2.h"

using namespace Ogre;
using namespace std;
using namespace tinyxml2;


Date s2dt(const char* a)
{
	Date d;  d.day=0; d.month=0; d.year=0;
	TIXML_SSCANF(a,"%2d-%2d-%2d",&d.day,&d.month,&d.year);
	return d;
}
string dt2s(const Date& dt)
{
	return fToStr(dt.day,0,2)+"-"+fToStr(dt.month,0,2)+"-"+fToStr(dt.year,0,2);
}


//-------------------------------------------------------------------------------------
TrackInfo::TrackInfo()
	:n(-1), crtver(0.f), name("none")
	,scenery("none"), author("none")
	,objects(0), fluids(0), bumps(0)
	,jumps(0), loops(0), pipes(0)
	,banked(0), frenzy(0), longn(0)
	,diff(0), rating(0)
	,nn(0), test(0), testC(0)
{	}

UserTrkInfo::UserTrkInfo()
	:name("") //last()
	,rating(0), laps(0), time(0)
{	}


///  User Load
///------------------------------------------------------------------------------------

bool UserXml::LoadXml(std::string file)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)  return false;
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	trks.clear();  trkmap.clear();

	//  tracks
	const char* a;  //int i=1;  //0 = none
	XMLElement* eTrk = root->FirstChildElement("track");
	while (eTrk)
	{
		UserTrkInfo t;
		a = eTrk->Attribute("n");		if (a)  t.name = string(a);

		a = eTrk->Attribute("date");	if (a)  t.last = s2dt(a);
		a = eTrk->Attribute("rate");	if (a)  t.rating = s2i(a);
		a = eTrk->Attribute("laps");	if (a)  t.laps = s2i(a);
		a = eTrk->Attribute("time");	if (a)  t.time = s2r(a);

		trks.push_back(t);
		trkmap[t.name] = trks.size();  //i++;
		eTrk = eTrk->NextSiblingElement("track");
	}
	return true;
}


///  User Save
///------------------------------------------------------------------------------------

bool UserXml::SaveXml(std::string file)
{
	TiXmlDocument xml;	TiXmlElement root("tracks");

	for (int i=0; i < trks.size(); ++i)
	{
		const UserTrkInfo& t = trks[i];
		TiXmlElement trk("t");
		trk.SetAttribute("n",		t.name.c_str());

		trk.SetAttribute("date",	dt2s(t.last).c_str());
		trk.SetAttribute("rate",	toStrC( t.rating ));
		trk.SetAttribute("laps",	toStrC( t.laps ));
		trk.SetAttribute("time",	toStrC( t.time ));
		
		root.InsertEndChild(trk);
	}
	xml.InsertEndChild(root);
	return xml.SaveFile(file.c_str());
}


//  Load
//--------------------------------------------------------------------------------------------------------------------------------------
//  Load Ini
//--------------------------------------------------------------------------------------------------------------------------------------
bool TracksXml::LoadIni(std::string file)
{
	//  clear
	trks.clear();  trkmap.clear();  times.clear();

	int i=1;  // 0 = not found
	char s[256], name[32],scenery[24],author[32];
	float time=0.f;
	TrackInfo t;

	std::ifstream fs(file.c_str());
	if (fs.fail())  return false;
	while (fs.good())
	{
		fs.getline(s,254);

		//  starting with digit
		if (strlen(s) > 0 && s[0] >= '0' && s[0] <= '9')
		{
		//114,E1-Lakes  v=2.0 06/04/13 07/04/13 :Finland  |o=0 w=1 ~=1 J=2 L=0 P=0 /=1 s=1 l=2 !=2 *=4  T=107.8  a:CH
			sscanf(s,
			"%d,%s v=%f %d/%d/%d %u/%u/%u :%s |o=%d w=%d ~=%d J=%d L=%d P=%d /=%d s=%d l=%d !=%d *=%d  T=%f a:%s"
				,&t.n, name, &t.crtver
					,&t.created.day, &t.created.month, &t.created.year
					,&t.modified.day, &t.modified.month, &t.modified.year
				,scenery
				,&t.objects, &t.fluids, &t.bumps, &t.jumps, &t.loops, &t.pipes
				,&t.banked, &t.frenzy, &t.longn, &t.diff, &t.rating
				,&time, author);

			t.name = name;
			t.scenery = scenery;
			t.author = author;
			
			//  get number from name, id for scenery
			t.nn = 0;
			size_t p2 = t.name.find('-'), p1;
			if (p2 != string::npos)
			{
				p1 = t.name.find_first_of("1234567890", 1);  //1 ignore 0W
				if (p1 != string::npos && p1 <= p2-1)
				{
					string ss = t.name.substr(p1, p2-1);
					t.nn = atoi(ss.c_str());
			}	}
			
				t.testC = t.name.length() > 5 ? t.name.substr(0,5)=="TestC" : false;
			if (t.testC)  t.test = false;  else
				t.test  = t.name.length() > 4 ? t.name.substr(0,4)=="Test"  : false;
			t.vdrift = author == "VDrift";

			trks.push_back(t);
			trkmap[name] = i++;
			times[name] = time;
		}
	}
	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------



CarInfo::CarInfo()
	:id("AA"), type("Other")
	,n(-1), speed(5), year(2005), rating(5)
	,easy(0.96f), norm(1.f), author("")
{	}

///  Load  cars.xml
//-------------------------------------------------------------------------------------

bool CarsXml::LoadXml(std::string file)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)  return false;

	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	cars.clear();  carmap.clear();  colormap.clear();

	
	//  global time mul factors
	float fEasy = 1.f, fNorm = 1.f;
	magic = 0.010f;

	const char* a;
	XMLElement* eGlobal = root->FirstChildElement("global");
	if (eGlobal)
	{
		a = eGlobal->Attribute("easy");  if (a)  fEasy = s2r(a);
		a = eGlobal->Attribute("norm");  if (a)  fNorm = s2r(a);
		a = eGlobal->Attribute("magic"); if (a)  magic = s2r(a);
	}

	///  cars
	int i=1;  //0 = none
	XMLElement* eCar = root->FirstChildElement("car");
	while (eCar)
	{
		CarInfo c;
		a = eCar->Attribute("id");		if (a)  c.id = string(a);
		a = eCar->Attribute("type");	if (a)  c.type = string(a);

		a = eCar->Attribute("n");		if (a)  c.n = s2i(a);
		a = eCar->Attribute("speed");	if (a)  c.speed = s2i(a);
		a = eCar->Attribute("year");	if (a)  c.year = s2i(a);
		a = eCar->Attribute("rating");	if (a)  c.rating = s2i(a);

		a = eCar->Attribute("easy");	if (a)  c.easy = fEasy * s2r(a);
		a = eCar->Attribute("norm");	if (a)  c.norm = fNorm * s2r(a);
		
		a = eCar->Attribute("author");	if (a)  c.author = string(a);

		cars.push_back(c);
		carmap[c.id] = i++;
		eCar = eCar->NextSiblingElement("car");
	}

	//  type colors
	XMLElement* eColor = root->FirstChildElement("color");
	while (eColor)
	{
		string type, clr;
		a = eColor->Attribute("type");	if (a)  type = string(a);
		a = eColor->Attribute("color");	if (a)  clr = string(a);

		if (!type.empty() && !clr.empty())
			colormap[type] = clr;
		eColor = eColor->NextSiblingElement("color");
	}
	return true;
}
