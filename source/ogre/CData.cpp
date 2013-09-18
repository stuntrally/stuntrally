#include "pch.h"
#include "CData.h"
#include "../vdrift/pathmanager.h"
#include "common/Defines.h"

#include "common/FluidsXml.h"
#include "common/BltObjects.h"
using Ogre::String;


CData::CData()
{
	fluids = new FluidsXml();
	objs = new BltObjects();
	tracks = new TracksXml;
	cars = new CarsXml;
}

void CData::Load()
{
	fluids->LoadXml(PATHMANAGER::Data() + "/materials2/fluids.xml");
	LogO(String("**** Loaded fluids.xml: ") + toStr(fluids->fls.size()));

	objs->LoadXml();  //  collisions.xml
	LogO(String("**** Loaded Vegetation objects: ") + toStr(objs->colsMap.size()));
	
	tracks->LoadIni(PATHMANAGER::GameConfigDir() + "/tracks.ini");
	cars->LoadXml(PATHMANAGER::GameConfigDir() + "/cars.xml");
}

CData::~CData()
{
	delete fluids;
	delete objs;
	delete tracks;
	delete cars;
}
