#pragma once
#include "dbl.h"
#include "settings.h"
#include "pathmanager.h"
#include "mathvector.h"
#include "quaternion.h"
#include "car.h"
#include "collision_world.h"
#include "collision_contact.h"
#include "carcontrolmap_local.h"
#include "cartire.h"
#include "tracksurface.h"
#include "timer.h"
#include "forcefeedback.h"
//#include "../sound/SoundMgr.h"
#include <OgreTimer.h>
#include <boost/thread.hpp>

class SoundMgr;  class Sound;


class GAME
{
public:
	class App* app;

	GAME(SETTINGS* pSettings);

	void Start(std::list <std::string>& args);

	double TickPeriod() const {  return framerate;  }
	bool OneLoop(double dt);

	void ReloadSimData();
	bool reloadSimNeed,reloadSimDone;  //for tweak tire save
	
	bool InitializeSound();  void LoadHudSounds();
	void End();

	void Test();
	void Tick(double dt);

	void AdvanceGameLogic(double dt);
	void UpdateCar(CAR& car, double dt);
	void UpdateCarInputs(CAR& car);
	void UpdateTimer();
	
	/// ---  new game  ========
	void LeaveGame(bool dstTrk);  // call in this order
	bool NewGameDoLoadTrack();  // track
	bool NewGameDoLoadMisc(float pre_time);  // timer,etc
	
	
	CAR* LoadCar(const std::string& pathCar, const std::string& carname,
		const MATHVECTOR<float,3>& start_pos, const QUATERNION<float>& start_rot,
		bool islocal, bool isRemote/*=false*/, int idCar);

	void ProcessNewSettings();
	void UpdateForceFeedback(float dt);
	float GetSteerRange() const;

//  vars

	unsigned int frame, displayframe;  // physics, display frame counters
	double clocktime, target_time;  // elapsed wall clock time
	const double framerate;
	float fps_min, fps_max;

	bool benchmode, profilingmode, pause;


	//  cars  ---
	SETTINGS* settings;

	std::vector<CAR*> cars;
	std::pair <CAR*, CARCONTROLMAP_LOCAL> controls;

	COLLISION_WORLD collision;
	bool bResetObj;
	
	TIMER timer;


	//  Sound  ---
	SoundMgr* snd;
	Sound* snd_chk, *snd_chkwr,  *snd_lap, *snd_lapbest,  *snd_stage, *snd_win[3], *snd_fail;
	void UpdHudSndVol();


	///  New  carsim  -------------
	std::vector <CARTIRE> tires;  /// all tires
	std::map <std::string, int> tires_map;  // name to tires id
	bool LoadTire(CARTIRE& ct, std::string path, std::string& file);
	bool LoadTires();

	//  ref graphs, tire edit
	std::string tire_ref;  int tire_ref_id;
	void PickTireRef(std::string name);

	std::vector <TRACKSURFACE> surfaces;  /// all surfaces
	std::map <std::string, int> surf_map;  // name to surface id
	bool LoadAllSurfaces();
	
	std::vector <std::vector <std::pair<double, double> > > suspS,suspD;  /// all suspension factors files (spring, damper)
	std::map <std::string, int> suspS_map,suspD_map;  // name to susp id
	bool LoadSusp();
	
#ifdef ENABLE_FORCE_FEEDBACK
	std::unique_ptr <FORCEFEEDBACK> forcefeedback;
	double ff_update_time;
#endif
};
