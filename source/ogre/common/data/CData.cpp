#include "pch.h"
#include "CData.h"
#include "../vdrift/pathmanager.h"
#include "../Def_Str.h"
using Ogre::String;

#include "FluidsXml.h"
#include "BltObjects.h"
#include "TracksXml.h"
#ifndef SR_EDITOR
#include "../../ChampsXml.h"
#include "../../ChallengesXml.h"
#endif

CData::CData()
{
	fluids = new FluidsXml();
	objs = new BltObjects();

	tracks = new TracksXml();
	cars = new CarsXml();

	#ifndef SR_EDITOR
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

	#ifndef SR_EDITOR
	delete champs;
	delete chall;
	#endif
}

void CData::Load()
{
	fluids->LoadXml(PATHMANAGER::Data() + "/materials2/fluids.xml");
	LogO(String("**** Loaded fluids.xml: ") + toStr(fluids->fls.size()));

	objs->LoadXml();  //  collisions.xml
	LogO(String("**** Loaded Vegetation objects: ") + toStr(objs->colsMap.size()));
	
	std::string path = PATHMANAGER::GameConfigDir();
	tracks->LoadIni(path + "/tracks.ini");
	cars->LoadXml(path + "/cars.xml");

	#ifndef SR_EDITOR
	champs->LoadXml(path + "/championships.xml", tracks);
	LogO(String("**** Loaded Championships: ") + toStr(champs->all.size()));
	chall->LoadXml(path + "/challenges.xml", tracks);
	LogO(String("**** Loaded Challenges: ") + toStr(chall->all.size()));
	#endif
}
