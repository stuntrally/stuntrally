#pragma once

class FluidsXml;
class BltObjects;

class TracksXml;
class CarsXml;

#ifndef SR_EDITOR
class ChampsXml;
class ChallXml;
#endif


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
	//UserXml* user;

	#ifndef SR_EDITOR
	ChampsXml* champs;  //ProgressXml progress[2];
	ChallXml* chall;  //ProgressLXml progressL[2];
	#endif
};
