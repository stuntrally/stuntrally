#ifndef _GAME_H
#define _GAME_H

#include "eventsystem.h"
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
#include "replay.h"
#include "forcefeedback.h"
#include "ai.h"
#include "quickmp.h"

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
	void UpdateCar(CAR & car, double dt);
	void UpdateDriftScore(CAR & car, double dt);
	void UpdateCarInputs(CAR & car);
	void UpdateTimer();

	void ProcessGUIInputs();
	void ProcessGameInputs();

	bool NewGame(bool playreplay=false, bool opponents=false, int num_laps=0);
	void LeaveGame();
	bool LoadTrack(const std::string & trackname);
	bool LoadCar(const std::string & carname, const MATHVECTOR <float, 3> & start_position, const QUATERNION <float> & start_orientation, bool islocal, bool isai, const std::string & carfile=""); ///< carfile is a string containing an entire .car file (e.g. XS.car) and is used instead of reading from disk.  this is optional

	void PopulateValueLists(std::map<std::string, std::list <std::pair<std::string,std::string> > > & valuelists);
	void PopulateReplayList(std::list <std::pair <std::string, std::string> > & replaylist);
	void PopulateCarPaintList(const std::string & carname, std::list <std::pair <std::string, std::string> > & carpaintlist);

	enum OPTION_ACTION	{	SAVE, LOAD	};
	void LoadSaveOptions(OPTION_ACTION action, std::map<std::string, std::string> & options);
	//void CollapseSceneToDrawlistmap(SCENENODE & node, std::map < DRAWABLE_FILTER *, std::vector <SCENEDRAW> > & outputmap, bool clearfirst);

	void LoadingScreen(float progress, float max);
	void ProcessNewSettings();
	void UpdateForceFeedback(float dt);

	std::string GetReplayRecordingFilename();
	void ParallelUpdate(int carindex);

	//void BeginDraw();
	//void BeginStartingUp();
	//void DoneStartingUp();
	//bool LastStartWasSuccessful() const;

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

	std::string controlgrab_page;
	std::string controlgrab_input;
	bool controlgrab_analog;
	bool controlgrab_only_one;
	std::pair <int,int> controlgrab_mouse_coords;
	CARCONTROLMAP_LOCAL::CONTROL controlgrab_editcontrol;
	std::vector <EVENTSYSTEM_SDL::JOYSTICK> controlgrab_joystick_state;

	EVENTSYSTEM_SDL eventsystem;
	SOUND sound;
	SOUNDBUFFERLIBRARY generic_sounds;
	SETTINGS* settings;
	PATHMANAGER pathmanager;
	TRACK track;

	std::list <CAR> cars;
	std::map <CAR *, int> cartimerids;
	std::pair <CAR *, CARCONTROLMAP_LOCAL> carcontrols_local;
public:
	COLLISION_WORLD collision;
	
	TIMER timer;
	REPLAY replay;
	AI ai;

#ifdef ENABLE_FORCE_FEEDBACK
	std::auto_ptr <FORCEFEEDBACK> forcefeedback;
	double ff_update_time;
#endif

public:
	GAME(std::ostream & info_out, std::ostream & err_out, SETTINGS* pSettings);

	void Start(std::list <std::string> & args);
};

#endif
