#include "pch.h"
#include "CData.h"
#include "../vdrift/pathmanager.h"
#include "../Def_Str.h"
using Ogre::String;

#include "FluidsXml.h"
#include "BltObjects.h"
#include "TracksXml.h"
#ifdef SR_EDITOR
	#include "SceneXml.h"
	#include "PresetsXml.h"
#else
	#include "../../ChampsXml.h"
	#include "../../ChallengesXml.h"
#endif


//  get drivability, vehicle on track fitness
float CData::GetDrivability(std::string car, std::string trk, bool track_user)
{
	if (track_user)  return -1.f;  // unknown

	int cid = cars->carmap[car];
	if (cid == 0)  return -1.f;
	const CarInfo& ci = cars->cars[cid-1];

	int tid = tracks->trkmap[trk];
	if (tid == 0)  return -1.f;
	const TrackInfo& ti = tracks->trks[tid-1];

	float undrv = 0.f;  // 0 drivable .. 1 undrivable
	int w = std::max(0, ci.width - 3);
	undrv += 0.8f * w/3.f * ti.narrow /3.f;
	undrv += 0.2f * w/3.f * ti.obstacles /4.f;
	undrv += 0.7f * ci.bumps /3.f * ti.bumps /4.f;  // * tweak params

	undrv += 1.1f * ci.jumps /3.f * ti.jumps /4.f;
	undrv += 1.1f * ci.loops /4.f * ti.loops; // /5.f;
	undrv += 1.4f * ci.pipes /4.f * ti.pipes /4.f;

	bool wnt = (ti.scenery == "Winter") || (ti.scenery == "WinterWet");
	if (wnt && ci.wheels >= 2)  // too slippery for fast cars
		undrv += 0.7f * std::max(0.f, ci.speed -7.f) /2.f;  // /10.f;

	return std::min(1.f, undrv);
}

#ifndef SR_EDITOR  // game
bool CData::IsChallCar(const Chall* ch, std::string name)
{
	int i,s;
	if (!ch->cars.empty())
		for (auto& c : ch->cars)  // allow specified
			if (c == name)  return true;
	
	if (!ch->carsDeny.empty())
		for (auto& c : ch->carsDeny)  // deny specified
			if (c == name)  return false;

	if (!ch->carTypes.empty())
	{	s = ch->carTypes.size();

		int id = cars->carmap[name]-1;
		if (id >= 0)
		{
			const auto& ci = cars->cars[id];
			String type = ci.type;

			if (ci.wheels < ch->whMin || ci.wheels > ch->whMax)
				return false;  // deny type if wheels not allowed

			for (i=0; i < s; ++i)
				if (type == ch->carTypes[i])  return true;
	}	}
	return false;
}
#endif


//  Load all xmls
void CData::Load(std::map <std::string, int>* surf_map, bool check)
{
	//  common
	fluids->LoadXml(PATHMANAGER::Data() + "/materials2/fluids.xml", /**/surf_map);
	LogO(String("**** Loaded Fluids: ") + toStr(fluids->fls.size()));

	objs->LoadXml();  //  collisions.xml
	LogO(String("**** Loaded Vegetation objects: ") + toStr(objs->colsMap.size()));
	
	std::string snd = PATHMANAGER::Sounds();
	reverbs->LoadXml(snd + "/reverbs.xml");
	LogO(String("**** Loaded Reverbs sets: ") + toStr(reverbs->revs.size()));

	//  cars and tracks
	std::string path = PATHMANAGER::GameConfigDir();
	tracks->LoadIni(path + "/tracks.ini", check);
	cars->LoadXml(path + "/cars.xml");
	
	#ifdef SR_EDITOR  // ed
		pre->LoadXml(path + "/presets.xml");
		LogO(String("**** Loaded Presets  sky: ") + toStr(pre->sky.size())+
			"  ter: " + toStr(pre->ter.size()) +
			"  road: " + toStr(pre->rd.size()) +
			"  grass: " + toStr(pre->gr.size()) +
			"  veget: " + toStr(pre->veg.size()) );
	#else	// game
		colors->LoadIni(path + "/colors.ini");
		LogO(String("**** Loaded Car Colors: ") + toStr(colors->v.size()));

		champs->LoadXml(path + "/championships.xml", tracks, check);
		LogO(String("**** Loaded Championships: ") + toStr(champs->all.size()));

		chall->LoadXml(path + "/challenges.xml", tracks, check);
		LogO(String("**** Loaded Challenges: ") + toStr(chall->all.size()));
	#endif


	#ifndef SR_EDITOR  // game challs drivability
	if (check)
	{
		LogO("))) Checking for low drivability in all challs, tracks, cars");
		for (const Chall& ch : chall->all)
			for (const ChallTrack& t : ch.trks)
			{
				for (const CarInfo& c : cars->cars)
				if (IsChallCar(&ch, c.id))
				{
					float drv = GetDrivability(c.id, t.name, false);
					float drvp = (1.f - drv) * 100.f;
					if (drvp < 10.f)  // par  undrivable
						LogO("U  "+ ch.name +"  "+ t.name +"  "+ c.id +"  "+ fToStr(drvp,0,2) +" %");
			}	}
		LogO("");
	}
	#endif
}


//  ctor
CData::CData()
{
	fluids = new FluidsXml();
	objs = new BltObjects();
	reverbs = new ReverbsXml();

	tracks = new TracksXml();
	cars = new CarsXml();

	#ifdef SR_EDITOR
		pre = new Presets();
	#else
		colors = new ColorsXml();
		champs = new ChampsXml();
		chall = new ChallXml();
	#endif
}

CData::~CData()
{
	delete fluids;
	delete objs;
	delete reverbs;

	delete tracks;
	delete cars;

	#ifdef SR_EDITOR
		delete pre;
	#else
		delete colors;
		delete champs;
		delete chall;
	#endif
}
