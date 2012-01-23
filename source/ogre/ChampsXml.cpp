#include "pch.h"
#include "Defines.h"
#include "ChampsXml.h"
#include "tinyxml.h"

using namespace Ogre;


ChampTrack::ChampTrack() :
	laps(0), factor(1.f)
{	}

Champ::Champ() :
	ver(1), diff(1), length(100.f), time(60.f), tutorial(0)
{	}


//  Load
//--------------------------------------------------------------------------------------------------------------------------------------
bool ChampsXml::LoadXml(std::string file)
{
	TiXmlDocument doc;
	if (!doc.LoadFile(file.c_str()))  return false;
		
	TiXmlElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	champs.clear();  trkTimes.clear();

	///  champs
	const char* a;
	TiXmlElement* eCh = root->FirstChildElement("championship");
	while (eCh)
	{
		Champ c;	//name="Bridges" ver="1" difficulty="1" length="3" time="134" tutorial="1" descr=""
		a = eCh->Attribute("name");			if (a)  c.name = std::string(a);
		a = eCh->Attribute("descr");		if (a)  c.descr = std::string(a);
		a = eCh->Attribute("ver");			if (a)  c.ver = s2i(a);
		a = eCh->Attribute("difficulty");	if (a)  c.diff = s2i(a);
		a = eCh->Attribute("length");		if (a)  c.length = s2r(a);
		a = eCh->Attribute("time");			if (a)  c.time = s2r(a);
		a = eCh->Attribute("tutorial");		if (a)  c.tutorial = s2i(a) == 1;
		
		//  tracks
		TiXmlElement* eTr = eCh->FirstChildElement("track");
		while (eTr)
		{
			ChampTrack t;	//name="S4-Hills" laps="1" factor="0.1"
			a = eTr->Attribute("name");		if (a)  t.name = std::string(a);
			a = eTr->Attribute("laps");		if (a)  t.laps = s2i(a);
			a = eTr->Attribute("factor");	if (a)  t.factor = s2r(a);
			
			c.trks.push_back(t);
			eTr = eTr->NextSiblingElement("track");
		}

		champs.push_back(c);
		eCh = eCh->NextSiblingElement("championship");
	}

	///  tracks best time
	TiXmlElement* eTim = root->FirstChildElement("times");
	if (eTim)
	{
		//  tracks
		TiXmlElement* eTr = eTim->FirstChildElement("track");
		while (eTr)
		{
			std::string trk;  float time = 60.f;	//name="TestC4-ow" time="12.1"
			a = eTr->Attribute("name");		if (a)  trk = std::string(a);
			a = eTr->Attribute("time");		if (a)  time = s2r(a);

			trkTimes[trk] = time;
			eTr = eTr->NextSiblingElement("track");
		}
	}
	return true;
}


//  progress on champs,tuts and their tracks
ProgressTrack::ProgressTrack() :
	score(0.f)//, laps(0)
{	}

ProgressChamp::ProgressChamp() :
	curTrack(0), score(0.f)
{	}


//  Load progress
//--------------------------------------------------------------------------------------------------------------------------------------
bool ProgressXml::LoadXml(std::string file)
{
	TiXmlDocument doc;
	if (!doc.LoadFile(file.c_str()))  return false;
		
	TiXmlElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	champs.clear();

	const char* a;
	TiXmlElement* eCh = root->FirstChildElement("championship");
	while (eCh)
	{
		ProgressChamp pc;
		int curTrack;  float score;
		//a = eCh->Attribute("name");			if (a)  pc.name = std::string(a);
		a = eCh->Attribute("curTrack");			if (a)  pc.curTrack = s2i(a);
		a = eCh->Attribute("score");			if (a)  pc.score = s2r(a);
		
		//  tracks
		TiXmlElement* eTr = eCh->FirstChildElement("track");
		while (eTr)
		{
			ProgressTrack pt;
			a = eTr->Attribute("score");	if (a)  pt.score = s2r(a);
			
			pc.trks.push_back(pt);
			eTr = eTr->NextSiblingElement("track");
		}

		champs.push_back(pc);
		eCh = eCh->NextSiblingElement("championship");
	}
	return true;
}

//  Save progress
bool ProgressXml::SaveXml(std::string file)
{
	TiXmlDocument xml;	TiXmlElement root("progress");

	for (int i=0; i < champs.size(); ++i)
	{
		const ProgressChamp& pc = champs[i];
		TiXmlElement eCh("champ");
			eCh.SetAttribute("curTrack",	toStrC( pc.curTrack ));
			eCh.SetAttribute("score",		toStrC( pc.score ));

			for (int i=0; i < pc.trks.size(); ++i)
			{
				const ProgressTrack& pt = pc.trks[i];
				TiXmlElement eTr("track");
				eTr.SetAttribute("score",		toStrC( pt.score ));
				eCh.InsertEndChild(eTr);
			}
		root.InsertEndChild(eCh);
	}
	xml.InsertEndChild(root);
	return xml.SaveFile(file.c_str());
}
