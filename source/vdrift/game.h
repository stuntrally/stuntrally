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

#include "sound.h"
#include "track.h"

#include "timer.h"
#include "forcefeedback.h"

#include <OgreTimer.h>
#include <boost/thread.hpp>


class GAME
{
public:
	class App* app;

	double TickPeriod() const {  return framerate;  }

	bool OneLoop(double dt);

	std::string strProfInfo;
	
	bool ParseArguments(std::list <std::string> & args);
	bool InitializeSound();
	void End();

	void Test();
	void Tick(double dt);

	void AdvanceGameLogic(double dt);
	void UpdateCar(CAR & car, double dt);
	void UpdateDriftScore(CAR & car, double dt);
	void UpdateCarInputs(CAR & car);
	void UpdateTimer();

	//bool NewGame(bool playreplay=false, bool opponents=false, int num_laps=0);
	
	/// ---  new game  ========
	bool NewGameDoCleanup();  // call in this order
	bool NewGameDoLoadTrack();
	/// ---  create cars here
	bool NewGameDoLoadMisc(float pre_time);
	
	
	void LeaveGame();
	bool LoadTrack(const std::string & trackname);
	CAR* LoadCar(const std::string & pathCar, const std::string & carname, const MATHVECTOR<float,3> & start_position,
		const QUATERNION<float> & start_orientation, bool islocal, bool isai,
		bool isRemote/*=false*/, int idCar);

	//void PopulateValueLists(std::map<std::string, std::list <std::pair<std::string,std::string> > > & valuelists);

	enum OPTION_ACTION	{	SAVE, LOAD	};
	void LoadSaveOptions(OPTION_ACTION action, std::map<std::string, std::string> & options);
	//void CollapseSceneToDrawlistmap(SCENENODE & node, std::map < DRAWABLE_FILTER *, std::vector <SCENEDRAW> > & outputmap, bool clearfirst);

	void LoadingScreen(float progress, float max);
	void ProcessNewSettings();
	void UpdateForceFeedback(float dt);
	float GetSteerRange() const;  //inline?

//  vars

	std::ostream & info_output;
	std::ostream & error_output;
	unsigned int frame; ///< physics frame counter
	unsigned int displayframe; ///< display frame counter
	double clocktime; ///< elapsed wall clock time
	double target_time;

	const double framerate;
	std::vector <float> fps_track;
	int fps_position;
	float fps_min,fps_max;

	bool multithreaded;
	bool benchmode;
	bool dumpfps;
	bool pause;
	unsigned int particle_timer;
	std::vector <std::pair<std::string, std::string> > opponents; //pairs of car names and car paints for opponents
	int race_laps;
	bool debugmode;
	bool profilingmode;


	SOUND sound;
	SOUND_LIB sound_lib;
	///  hud 2d sounds  //)
	SOUNDSOURCE snd_chk, snd_chkwr,  snd_lap, snd_lapbest,  snd_stage, snd_win[3], snd_fail;
	void UpdHudSndVol();


	SETTINGS* settings;
	TRACK track;

	std::list <CAR> cars;
	std::pair <CAR*, CARCONTROLMAP_LOCAL> carcontrols_local;

	//  carsim
	std::vector <CARTIRE> tires;  /// New  all tires
	std::map <std::string, int> tires_map;  // name to tires id
	bool LoadTire(CARTIRE& ct, std::string path, std::string& file);
	bool LoadTires();

	std::vector <TRACKSURFACE> surfaces;  /// New  all surfaces
	std::map <std::string, int> surf_map;  // name to surface id
	bool LoadAllSurfaces();
	
	std::vector <std::vector <std::pair<double, double> > > suspS,suspD;  /// New  all suspension factors files (spring, damper)
	std::map <std::string, int> suspS_map,suspD_map;  // name to susp id
	bool LoadSusp();
	
#ifdef ENABLE_FORCE_FEEDBACK
	std::auto_ptr <FORCEFEEDBACK> forcefeedback;
	double ff_update_time;
#endif
	
public:
	COLLISION_WORLD collision;
	
	TIMER timer;

public:
	GAME(std::ostream & info_out, std::ostream & err_out, SETTINGS* pSettings);

	void Start(std::list <std::string> & args);
	void ReloadSimData();
};
