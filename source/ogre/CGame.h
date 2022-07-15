#pragma once
#include "common/Gui_Def.h"
#include "BaseApp.h"
#include "Replay.h"
#include "ReplayTrk.h"
#include "../vdrift/cardefs.h"
#include "CarPosInfo.h"

#include <boost/thread.hpp>
#include <OgreShadowCameraSetup.h>
#include "../shiny/Main/Factory.hpp"
#include "../network/networkcallbacks.hpp"
#include "../oics/ICSChannelListener.h"
#include "common/PreviewTex.h"


namespace Ogre {  class SceneNode;  class SceneManager;  }
namespace BtOgre  {  class DebugDrawer;  }
class CScene;  class CData;  class CInput;  class GraphView;
class GAME;  class CHud;  class CGui;  class CGuiCom;


class App : public BaseApp,
			public sh::MaterialListener,
			public ICS::ChannelListener
{
public:
	App(SETTINGS* settings, GAME* game);
	virtual ~App();
	void ShutDown();
	
	CScene* scn =0;
	CData* data =0;  //p
	
	GAME* pGame =0;
	

	//  BaseApp init
	void postInit(), SetFactoryDefaults();
		
	
	///  Game Cars Data
	//  new positions info for every CarModel
	PosInfo carPoses[CarPosCnt][MAX_CARS];  // max 16cars
	int iCurPoses[MAX_CARS];  // current index for carPoses queue
	std::map<int,int> carsCamNum;  // picked camera number for cars
	
	void newPoses(float time), newPerfTest(float time);  // vdrift
	void updatePoses(float time);  // ogre
	void UpdThr();

	//  replay - full, saved by user
	//  ghost - saved on best lap
	//  ghplay - ghost ride replay, loaded if was on disk, replaced when new
	Replay2 replay, ghost, ghplay;
	Rewind rewind;  // to take car back in time (after crash etc.)
	TrackGhost ghtrk;  //  ghtrk - track's ghost

	std::vector<ReplayFrame2> frm;  //size:16  //  frm - used when playing replay for hud and sounds

	bool isGhost2nd;  // if present (ghost but from other car)
	std::vector<float> vTimeAtChks;  // track ghost's times at road checkpoints
	float fLastTime;  // thk ghost total time
		

	boost::thread mThread;  // 2nd thread for simulation


	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameStart(Ogre::Real time);  void DoNetworking();
	virtual bool frameEnd(Ogre::Real time);
	float fLastFrameDT;

	bool isTweakTab();
		
	BtOgre::DebugDrawer *dbgdraw;  /// blt dbg

	//  mtr reload
	enum eMaterials {
		Mtr_CarBody, Mtr_CarInterior, Mtr_CarGlass,
		Mtr_CarTireFront, Mtr_CarTireRear,
		Mtr_Road,  NumMaterials  };
	Ogre::String sMtr[NumMaterials];


	///  HUD
	CHud* hud;


	///  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	Ogre::String resCar, resTrk, resDrv;
	void CreateCar(), CreateRoads(), CreateRoadsInt(), CreateTrail(Ogre::Camera* cam);
	void CreateObjects(), DestroyObjects(bool clear), ResetObjects();

	void NewGame(bool force=false);
	void NewGameDoLoad();  bool IsVdrTrack();  bool newGameRpl;

	bool dstTrk;  // destroy track, false if same
	Ogre::String oldTrack;  bool oldTrkUser;

	//  fluids to destroy
	std::vector<Ogre::String/*MeshPtr*/> vFlSMesh;
	std::vector<Ogre::Entity*> vFlEnt;
	std::vector<Ogre::SceneNode*> vFlNd;

	
	//  Loading
	void LoadCleanUp(), LoadGame(), LoadScene(), LoadCar(), LoadTerrain(), LoadRoad(), LoadObjects(), LoadTrees(), LoadMisc();
	enum ELoadState { LS_CLEANUP=0, LS_GAME, LS_SCENE, LS_CAR, LS_TERRAIN, LS_ROAD, LS_OBJECTS, LS_TREES, LS_MISC, LS_ALL };
	static Ogre::String cStrLoad[LS_ALL+1];
	int curLoadState;
	std::map<int, std::string> loadingStates;

	float mTimer;  // wind,water


	//  mtr from ter  . . . 
	int blendMapSize;  char* blendMtr =0;
	void GetTerMtrIds();


	void recreateReflections();  // call after refl_mode changed

	virtual void materialCreated(sh::MaterialInstance* m, const std::string& configuration, unsigned short lodIndex);


	//  Input
	CInput* input =0;

	virtual bool keyPressed(const SDL_KeyboardEvent &arg);
	void channelChanged(ICS::Channel *channel, float currentValue, float previousValue);


	///  Gui
	//-----------------------------------------------------------------
	CGui* gui =0;
	CGuiCom* gcom =0;

	PreviewTex prvView,prvRoad,prvTer;  // track tab
	PreviewTex prvStCh;  // champ,chall stage view

	bool bHideHudBeam;  // hides beam when replay or no road
	bool bHideHudArr;	// hides arrow when replay,splitscreen
	bool bHideHudPace;  // hides pacenotes when same or deny by challenge
	bool bHideHudTrail; // hides trail if denied by challenge
	
	bool bRplPlay,bRplPause, bRplRec, bRplWnd;  //  game
	int carIdWin, iRplCarOfs, iRplSkip;

	//  race pos
	int GetRacePos(float timeCur, float timeTrk, float carTimeMul, bool coldStart, float* pPoints=0);
	float GetCarTimeMul(const std::string& car, const std::string& sim_mode);

	void Ch_NewGame();


	///  graphs
	std::vector<GraphView*> graphs;
	void CreateGraphs(), DestroyGraphs();
	void UpdateGraphs(), GraphsNewVals();

	///* tire edit */
	const static int TireNG;
	int iEdTire, iTireLoad, iCurLat,iCurLong,iCurAlign, iUpdTireGr;

	///  car perf test
	bool bPerfTest;  EPerfTest iPerfTestStage;
	void PerfLogVel(class CAR* pCar, float time);
};
