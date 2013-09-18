#pragma once
#include "common/tracksXml.h"

class FluidsXml;
class BltObjects;
class TracksXml;
class CarsXml;


class CData
{
public:
	CData();
	~CData();
	
	void Load();

	FluidsXml* fluids;	//  fluids params for car sim
	BltObjects* objs;	//  collisions.xml, for vegetation models
	
	TracksXml* tracks;  // tracks.ini info for Gui
	CarsXml* cars;		// cars info for Gui
	//UserXml user;
};
