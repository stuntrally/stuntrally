#pragma once

class FluidsXml;
class BltObjects;

class TracksXml;
class CarsXml;

#ifdef SR_EDITOR
	class Presets;
#else
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
	
	#ifdef SR_EDITOR	// ed only
		Presets* pre;
	#else				// game only
		ChampsXml* champs;  //ProgressXml progress[2];
		ChallXml* chall;  //ProgressLXml progressL[2];
	#endif
};
