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
#else
#include "../../ChampsXml.h"
#include "../../ChallengesXml.h"
#endif


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
}

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
