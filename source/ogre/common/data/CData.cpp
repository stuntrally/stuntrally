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


void CData::Load()
{
	fluids->LoadXml(PATHMANAGER::Data() + "/materials2/fluids.xml");
	LogO(String("**** Loaded Fluids: ") + toStr(fluids->fls.size()));

	objs->LoadXml();  //  collisions.xml
	LogO(String("**** Loaded Vegetation objects: ") + toStr(objs->colsMap.size()));
	
	std::string path = PATHMANAGER::GameConfigDir();
	tracks->LoadIni(path + "/tracks.ini");
	cars->LoadXml(path + "/cars.xml");

	#ifdef SR_EDITOR
		pre->LoadXml(path + "/presets.xml");
		LogO(String("**** Loaded Presets  ter: ") + toStr(pre->ter.size())+
			"  road: " + toStr(pre->rd.size()) +
			"  grass: " + toStr(pre->gr.size()) +
			"  veget: " + toStr(pre->veg.size()) );
	#else
		champs->LoadXml(path + "/championships.xml", tracks);
		LogO(String("**** Loaded Championships: ") + toStr(champs->all.size()));

		chall->LoadXml(path + "/challenges.xml", tracks);
		LogO(String("**** Loaded Challenges: ") + toStr(chall->all.size()));
	#endif
}

CData::CData()
{
	fluids = new FluidsXml();
	objs = new BltObjects();

	tracks = new TracksXml();
	cars = new CarsXml();

	#ifdef SR_EDITOR
		pre = new Presets();
	#else
		champs = new ChampsXml();
		chall = new ChallXml();
	#endif
}

CData::~CData()
{
	delete fluids;
	delete objs;

	delete tracks;
	delete cars;

	#ifdef SR_EDITOR
		delete pre;
	#else
		delete champs;
		delete chall;
	#endif
}
