#include "pch.h"
#include "Defines.h"
#include "TracksXml.h"
#include "tinyxml.h"

using namespace Ogre;
using namespace std;


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
	:n(-1),crtver(0.0),name("none"),scenery("none"),author("none")
	,fluids(0),bumps(0),jumps(0),loops(0),pipes(0),banked(0),frenzy(0),longn(0),objects(0)
	,diff(0),rating(0)
{	}

UserTrkInfo::UserTrkInfo()
	:name("") //last()
	,rating(0), laps(0), time(0)
{	}


///  User Load
///------------------------------------------------------------------------------------

bool UserXml::LoadXml(Ogre::String file)
{
	TiXmlDocument doc;
	if (!doc.LoadFile(file.c_str()))  return false;
		
	TiXmlElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	trks.clear();  trkmap.clear();

	//  tracks
	const char* a;  //int i=1;  //0 = none
	TiXmlElement* eTrk = root->FirstChildElement("track");
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

bool UserXml::SaveXml(Ogre::String file)
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

bool TracksXml::LoadXml(Ogre::String file)
{
	TiXmlDocument doc;
	if (!doc.LoadFile(file.c_str()))  return false;
		
	TiXmlElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	trks.clear();  trkmap.clear();

	///  tracks
	const char* a;  int i=1;  //0 = none
	TiXmlElement* eTrk = root->FirstChildElement("track");
	while (eTrk)
	{
		TrackInfo t;
		a = eTrk->Attribute("n");			if (a)  t.n = s2i(a);
		a = eTrk->Attribute("name");		if (a)  t.name = string(a);
		a = eTrk->Attribute("created");		if (a)  t.created = s2dt(a);
		a = eTrk->Attribute("crtver");		if (a)  t.crtver = s2r(a);
		a = eTrk->Attribute("modified");	if (a)  t.modified = s2dt(a);
		a = eTrk->Attribute("scenery");		if (a)  t.scenery = string(a);
		a = eTrk->Attribute("author");		if (a)  t.author = string(a);

		a = eTrk->Attribute("fluids");		if (a)  t.fluids = s2i(a);
		a = eTrk->Attribute("bumps");		if (a)  t.bumps = s2i(a);	a = eTrk->Attribute("jumps");		if (a)  t.jumps = s2i(a);
		a = eTrk->Attribute("loops");		if (a)  t.loops = s2i(a);	a = eTrk->Attribute("pipes");		if (a)  t.pipes = s2i(a);
		a = eTrk->Attribute("banked");		if (a)  t.banked = s2i(a);	a = eTrk->Attribute("frenzy");		if (a)  t.frenzy = s2i(a);
		a = eTrk->Attribute("long");		if (a)  t.longn = s2i(a);
		a = eTrk->Attribute("objects");		if (a)  t.objects = s2i(a);

		a = eTrk->Attribute("diff");		if (a)  t.diff = s2i(a);
		a = eTrk->Attribute("rating");		if (a)  t.rating = s2i(a);

		trks.push_back(t);
		trkmap[t.name] = i++;
		eTrk = eTrk->NextSiblingElement("track");
	}
	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------



CarInfo::CarInfo()
	:id("AA"), name("AA"), type("Other")
	,n(-1), speed(5), year(2005), rating(5)
	,easy(0.96f), norm(1.f)
{	}

///  Load  cars.xml
//-------------------------------------------------------------------------------------

bool CarsXml::LoadXml(Ogre::String file)
{
	TiXmlDocument doc;
	if (!doc.LoadFile(file.c_str()))  return false;
		
	TiXmlElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	cars.clear();  carmap.clear();  colormap.clear();

	
	//  global time mul factors
	float fEasy = 1.f, fNorm = 1.f;
	magic = 0.010f;

	const char* a;
	TiXmlElement* eGlobal = root->FirstChildElement("global");
	if (eGlobal)
	{
		a = eGlobal->Attribute("easy");  if (a)  fEasy = s2r(a);
		a = eGlobal->Attribute("norm");  if (a)  fNorm = s2r(a);
		a = eGlobal->Attribute("magic"); if (a)  magic = s2r(a);
	}

	///  cars
	int i=1;  //0 = none
	TiXmlElement* eCar = root->FirstChildElement("car");
	while (eCar)
	{
		CarInfo c;
		a = eCar->Attribute("id");		if (a)  c.id = string(a);
		a = eCar->Attribute("name");	if (a)  c.name = string(a);
		a = eCar->Attribute("type");	if (a)  c.type = string(a);

		a = eCar->Attribute("n");		if (a)  c.n = s2i(a);
		a = eCar->Attribute("speed");	if (a)  c.speed = s2i(a);
		a = eCar->Attribute("year");	if (a)  c.year = s2i(a);
		a = eCar->Attribute("rating");	if (a)  c.rating = s2i(a);

		a = eCar->Attribute("easy");	if (a)  c.easy = fEasy * s2r(a);
		a = eCar->Attribute("norm");	if (a)  c.norm = fNorm * s2r(a);

		cars.push_back(c);
		carmap[c.id] = i++;
		eCar = eCar->NextSiblingElement("car");
	}

	//  type colors
	TiXmlElement* eColor = root->FirstChildElement("color");
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
