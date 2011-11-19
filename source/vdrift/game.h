#ifndef _GAME_H
#define _GAME_H

#include "settings.h"
#include "pathmanager.h"
#include "mathvector.h"
#include "quaternion.h"

#include "car.h"
#include "collision_world.h"
#include "collision_contact.h"
#include "carcontrolmap_local.h"

#include "sound.h"
#include "track.h"

#include "timer.h"
#include "forcefeedback.h"
#include "ai.h"

#include "../ogre/QTimer.h"


class GAME
{
//private:
public:
	class App* pOgreGame;
	float TickPeriod() const {return framerate;}
	bool OneLoop();  std::string strProfInfo;
	QTimer qtim;
	
	bool ParseArguments(std::list <std::string> & args);
	bool InitializeSound();
	void End();

	void Test();
	void Tick(float dt);

	void AdvanceGameLogic();
	void UpdateCar(CAR & car, int i, double dt);
	void UpdateDriftScore(CAR & car, double dt);
	void UpdateCarInputs(CAR & car, int i);
	void UpdateTimer();

	void ProcessGameInputs();

	//bool NewGame(bool playreplay=false, bool opponents=false, int num_laps=0);
	
	/// ---  new game  ========
	bool NewGameDoCleanup();  // call in this order
	bool NewGameDoLoadTrack();
	/// ---  create cars here
	bool NewGameDoLoadMisc();
	
	
	void LeaveGame();
	bool LoadTrack(const std::string & trackname);
	CAR* LoadCar(const std::string & carname, const MATHVECTOR <float, 3> & start_position, const QUATERNION <float> & start_orientation, bool islocal, bool isai, const std::string & carfile=""); ///< carfile is a string containing an entire .car file (e.g. XS.car) and is used instead of reading from disk.  this is optional

	//void PopulateValueLists(std::map<std::string, std::list <std::pair<std::string,std::string> > > & valuelists);

	enum OPTION_ACTION	{	SAVE, LOAD	};
	void LoadSaveOptions(OPTION_ACTION action, std::map<std::string, std::string> & options);
	//void CollapseSceneToDrawlistmap(SCENENODE & node, std::map < DRAWABLE_FILTER *, std::vector <SCENEDRAW> > & outputmap, bool clearfirst);

	void LoadingScreen(float progress, float max);
	void ProcessNewSettings();
	void UpdateForceFeedback(float dt);

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
	SOUNDBUFFERLIBRARY generic_sounds;
	SETTINGS* settings;
	TRACK track;

	std::list <CAR> cars;
	std::map <CAR*, int> cartimerids;
	std::pair <CAR*, CARCONTROLMAP_LOCAL> carcontrols_local;
public:
	COLLISION_WORLD collision;
	
	TIMER timer;
	//AI ai;

public:
	GAME(std::ostream & info_out, std::ostream & err_out, SETTINGS* pSettings);

	void Start(std::list <std::string> & args);
};

#endif
