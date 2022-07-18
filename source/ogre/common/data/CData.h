#pragma once
#include <map>
#include <string>

class FluidsXml;
class BltObjects;
class ReverbsXml;

class TracksXml;
class CarsXml;

#ifdef SR_EDITOR
	class Presets;
#else
	class ColorsXml;
	class ChampsXml;
	class ChallXml;
	class Chall;
#endif


class CData
{
public:
	CData();
	~CData();
	
	void Load(std::map <std::string, int>* surf_map=0, bool check=false);  //

	FluidsXml* fluids;	//  fluids params for car sim
	BltObjects* objs;	//  collisions.xml, for vegetation models
	ReverbsXml* reverbs;  // reverb presets for sceneries, used in scene.xml
	
	TracksXml* tracks;  // tracks.ini info for Gui
	CarsXml* cars;		// cars info for Gui
	//UserXml* user;
	
	#ifdef SR_EDITOR	// ed only
		Presets* pre;
	#else				// game only
		ColorsXml* colors;  // car colors.ini
		ChampsXml* champs;  //ProgressXml progress[2];
		ChallXml* chall;  //ProgressLXml progressL[2];
	#endif

	//  get drivability, vehicle on track fitness
	float GetDrivability(std::string car, std::string trk, bool track_user);
	#ifndef SR_EDITOR
		bool IsChallCar(const Chall* ch, std::string name);
	#endif
};
