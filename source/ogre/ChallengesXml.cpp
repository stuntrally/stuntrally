#include "pch.h"
#include "common/Defines.h"
#include "ChallengesXml.h"
#include "ChampsXml.h"  //timesXml
#include "tinyxml.h"
//#include <OgreString.h>
using namespace Ogre;


ChallTrack::ChallTrack()
	:name("J1-T"), laps(0), reversed(0)
	,passPoints(7.f), timeNeeded(-1.f)  // defaults
{	}

Chall::Chall()  // defaults..
	:ver(1), diff(1), length(0.f), type(0), time(0.f)
	,name("none"), descr("none")
	,sim_mode("normal")
	,damage_type(2), boost_type(1), flip_type(2), rewind_type(1)
	,minimap(1), chk_arr(0), chk_beam(0), trk_ghost(1)
	,total_time(-1.f), avg_pos(7)
	// abs, tcs, autoshift, autorear
	// max dmg%, off road time-
{	}


//  Load challenges
//--------------------------------------------------------------------------------------------------------------------------------------
bool ChallXml::LoadXml(std::string file, TimesXml* times)
{
	TiXmlDocument doc;
	if (!doc.LoadFile(file.c_str()))  return false;
		
	TiXmlElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	ch.clear();

	///  challs
	const char* a;
	TiXmlElement* eCh = root->FirstChildElement("challenge");
	while (eCh)
	{
		Chall c;  // defaults in ctor
		a = eCh->Attribute("name");			if (a)  c.name = std::string(a);
		a = eCh->Attribute("descr");		if (a)  c.descr = std::string(a);
		a = eCh->Attribute("ver");			if (a)  c.ver = s2i(a);
		a = eCh->Attribute("difficulty");	if (a)  c.diff = s2i(a);
		a = eCh->Attribute("type");			if (a)  c.type = s2i(a);
		
		TiXmlElement* eSim = eCh->FirstChildElement("sim");
		if (eSim)
		{
			a = eSim->Attribute("mode");		if (a)  c.sim_mode = std::string(a);
			a = eSim->Attribute("damage");		if (a)  c.damage_type = s2i(a);  // range chk..
			a = eSim->Attribute("boost");		if (a)  c.boost_type = s2i(a);
			a = eSim->Attribute("flip");		if (a)  c.flip_type = s2i(a);
			a = eSim->Attribute("rewind");		if (a)  c.rewind_type = s2i(a);
		}
		//  cars
		TiXmlElement* eCarT = eCh->FirstChildElement("cartype");
		if (eCarT)
		{
			a = eCarT->Attribute("names");
			if (a)
			{	String s = a;
				c.carTypes = StringUtil::split(s, "|");
			}
		}
		//<car names="ES|S1" />
		TiXmlElement* eCar = eCh->FirstChildElement("car");
		if (eCar)
		{
			a = eCar->Attribute("names");
			if (a)
			{	String s = a;
				c.cars = StringUtil::split(s, "|");
			}
		}
		
		TiXmlElement* eHud = eCh->FirstChildElement("hud");
		if (eSim)
		{
			a = eHud->Attribute("minimap");		if (a)  c.minimap = s2i(a) > 0;
			a = eHud->Attribute("chkArrow");	if (a)  c.chk_arr = s2i(a) > 0;
			a = eHud->Attribute("chkBeam");		if (a)  c.chk_beam = s2i(a) > 0;
			a = eHud->Attribute("trkGhost");	if (a)  c.trk_ghost = s2i(a) > 0;
		}
		TiXmlElement* ePass = eCh->FirstChildElement("pass");
		if (ePass)
		{
			a = ePass->Attribute("total_time");	if (a)  c.flip_type = s2r(a);
			a = ePass->Attribute("avg_pos");	if (a)  c.rewind_type = s2r(a);
		}

		//  tracks
		TiXmlElement* eTrks = eCh->FirstChildElement("tracks");
		if (eTrks)
		{
			TiXmlElement* eTr = eTrks->FirstChildElement("t");
			while (eTr)
			{
				ChallTrack t;
				a = eTr->Attribute("name");		if (a)  t.name = std::string(a);
				a = eTr->Attribute("reversed");	if (a)  t.reversed = s2i(a) > 0;
				a = eTr->Attribute("laps");		if (a)  t.laps = s2i(a);
				
				a = eTr->Attribute("passTime");   if (a)  t.timeNeeded = s2r(a);
				a = eTr->Attribute("passPoints"); if (a)  t.passPoints = s2r(a);

				c.trks.push_back(t);
				eTr = eTr->NextSiblingElement("t");
			}
		}

		ch.push_back(c);
		eCh = eCh->NextSiblingElement("challenge");
	}
	
	///  get champs total time (sum tracks times)
	if (times)
	for (int c=0; c < ch.size(); ++c)
	{
		Chall& l = ch[c];
		float allTime = 0.f;
		for (int i=0; i < l.trks.size(); ++i)
		{
			const ChallTrack& trk = l.trks[i];

			float time = times->trks[trk.name] * trk.laps;
			allTime += time;  // sum trk time, total champ time
		}
		l.time = allTime;
	}
	return true;
}

//  progress
//--------------------------------------------------------------------------------------------------------------------------------------

ProgressTrackL::ProgressTrackL() :
	points(0.f)
{	}

ProgressChall::ProgressChall() :
	curTrack(0), points(0.f), ver(0)
{	}


//  Load progress
//--------------------------------------------------------------------------------------------------------------------------------------
bool ProgressLXml::LoadXml(std::string file)
{
	TiXmlDocument doc;
	if (!doc.LoadFile(file.c_str()))  return false;
		
	TiXmlElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	ch.clear();

	const char* a;
	TiXmlElement* eCh = root->FirstChildElement("chall");
	while (eCh)
	{
		ProgressChall pc;
		a = eCh->Attribute("name");		if (a)  pc.name = std::string(a);
		a = eCh->Attribute("ver");		if (a)  pc.ver = s2i(a);
		a = eCh->Attribute("cur");	if (a)  pc.curTrack = s2i(a);
		a = eCh->Attribute("p");	if (a)  pc.points = s2r(a);
		
		//  tracks
		TiXmlElement* eTr = eCh->FirstChildElement("t");
		while (eTr)
		{
			ProgressTrackL pt;
			a = eTr->Attribute("p");	if (a)  pt.points = s2r(a);
			
			pc.trks.push_back(pt);
			eTr = eTr->NextSiblingElement("t");
		}

		ch.push_back(pc);
		eCh = eCh->NextSiblingElement("chall");
	}
	return true;
}

//  Save progress
bool ProgressLXml::SaveXml(std::string file)
{
	TiXmlDocument xml;	TiXmlElement root("progress");

	for (int i=0; i < ch.size(); ++i)
	{
		const ProgressChall& pc = ch[i];
		TiXmlElement eCh("chall");
			eCh.SetAttribute("name",	pc.name.c_str() );
			eCh.SetAttribute("ver",		toStrC( pc.ver ));
			eCh.SetAttribute("cur",	toStrC( pc.curTrack ));
			eCh.SetAttribute("p",	toStrC( pc.points ));

			for (int i=0; i < pc.trks.size(); ++i)
			{
				const ProgressTrackL& pt = pc.trks[i];
				TiXmlElement eTr("t");
				eTr.SetAttribute("p",	fToStr( pt.points, 1).c_str());
				eCh.InsertEndChild(eTr);
			}
		root.InsertEndChild(eCh);
	}
	xml.InsertEndChild(root);
	return xml.SaveFile(file.c_str());
}
