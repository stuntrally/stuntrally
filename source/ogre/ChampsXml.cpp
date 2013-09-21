#include "pch.h"
#include "common/Def_Str.h"
#include "common/data/TracksXml.h"
#include "ChampsXml.h"
#include "tinyxml.h"
#include "tinyxml2.h"
using namespace tinyxml2;


ChampTrack::ChampTrack() :
	laps(0), factor(1.f), reversed(0), passScore(100.f)  // default
{	}

Champ::Champ() :
	ver(1), diff(1), length(100.f), type(0), time(0.f)
{	}

ChampsXml::ChampsXml()
{	}


//  Load champs
//--------------------------------------------------------------------------------------------------------------------------------------
bool ChampsXml::LoadXml(std::string file, TracksXml* trks)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)  return false;
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	all.clear();

	///  champs
	const char* a;
	XMLElement* eCh = root->FirstChildElement("championship");
	while (eCh)
	{
		Champ c;	//name="Bridges" ver="1" difficulty="1" length="3" time="134" tutorial="1" descr=""
		a = eCh->Attribute("name");			if (a)  c.name = std::string(a);
		a = eCh->Attribute("descr");		if (a)  c.descr = std::string(a);
		a = eCh->Attribute("ver");			if (a)  c.ver = s2i(a);
		a = eCh->Attribute("difficulty");	if (a)  c.diff = s2i(a);
		a = eCh->Attribute("length");		if (a)  c.length = s2r(a);
		a = eCh->Attribute("type");			if (a)  c.type = s2i(a);
		
		//  tracks
		XMLElement* eTr = eCh->FirstChildElement("track");
		while (eTr)
		{
			ChampTrack t;	//name="S4-Hills" laps="1" factor="0.1"
			a = eTr->Attribute("name");		if (a)  t.name = std::string(a);
			a = eTr->Attribute("laps");		if (a)  t.laps = s2i(a);
			a = eTr->Attribute("factor");	if (a)  t.factor = s2r(a);
			a = eTr->Attribute("reversed");	if (a)  t.reversed = s2i(a) > 0;
			a = eTr->Attribute("passScore");if (a)  t.passScore = s2r(a);
			
			c.trks.push_back(t);
			eTr = eTr->NextSiblingElement("track");
		}

		all.push_back(c);
		eCh = eCh->NextSiblingElement("championship");
	}
	
	///  get champs total time (sum tracks times)
	if (trks)
	for (int c=0; c < all.size(); ++c)
	{
		Champ& ch = all[c];
		float allTime = 0.f;
		for (int i=0; i < ch.trks.size(); ++i)
		{
			const ChampTrack& trk = ch.trks[i];

			float time = trks->times[trk.name] * trk.laps;
			allTime += time;  // sum trk time, total champ time
		}
		ch.time = allTime;
	}
	return true;
}


//  progress on champs,tuts and their tracks
ProgressTrack::ProgressTrack() :
	points(0.f)
{	}

ProgressChamp::ProgressChamp() :
	curTrack(0), points(0.f), ver(0)
{	}


//  Load progress
//--------------------------------------------------------------------------------------------------------------------------------------
bool ProgressXml::LoadXml(std::string file)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)  return false;

	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	chs.clear();

	const char* a;
	XMLElement* eCh = root->FirstChildElement("champ");
	while (eCh)
	{
		ProgressChamp pc;
		a = eCh->Attribute("name");		if (a)  pc.name = std::string(a);
		a = eCh->Attribute("ver");		if (a)  pc.ver = s2i(a);
		a = eCh->Attribute("cur");	if (a)  pc.curTrack = s2i(a);
		a = eCh->Attribute("p");	if (a)  pc.points = s2r(a);
		
		//  tracks
		XMLElement* eTr = eCh->FirstChildElement("t");
		while (eTr)
		{
			ProgressTrack pt;
			a = eTr->Attribute("p");	if (a)  pt.points = s2r(a);
			
			pc.trks.push_back(pt);
			eTr = eTr->NextSiblingElement("t");
		}

		chs.push_back(pc);
		eCh = eCh->NextSiblingElement("champ");
	}
	return true;
}

//  Save progress
bool ProgressXml::SaveXml(std::string file)
{
	TiXmlDocument xml;	TiXmlElement root("progress");

	for (int i=0; i < chs.size(); ++i)
	{
		const ProgressChamp& pc = chs[i];
		TiXmlElement eCh("champ");
			eCh.SetAttribute("name",	pc.name.c_str() );
			eCh.SetAttribute("ver",		toStrC( pc.ver ));
			eCh.SetAttribute("cur",	toStrC( pc.curTrack ));
			eCh.SetAttribute("p",	toStrC( pc.points ));

			for (int i=0; i < pc.trks.size(); ++i)
			{
				const ProgressTrack& pt = pc.trks[i];
				TiXmlElement eTr("t");
				eTr.SetAttribute("p",	fToStr( pt.points, 1).c_str());
				eCh.InsertEndChild(eTr);
			}
		root.InsertEndChild(eCh);
	}
	xml.InsertEndChild(root);
	return xml.SaveFile(file.c_str());
}
