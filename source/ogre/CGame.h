#pragma once
#include "common/Gui_Def.h"
#include "BaseApp.h"
#include "ReplayGame.h"
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

const int CarPosCnt = 8;  // size of poses queue


class App : public BaseApp,
			public sh::MaterialListener,
			public ICS::ChannelListener
{
public:
	App(SETTINGS* settings, GAME* game);
	virtual ~App();
	
	CScene* scn;
	CData* data;  //p
	
	GAME* pGame;
	

	//  BaseApp init
	void postInit(), SetFactoryDefaults();
		
	
	///  Game Cars Data
	//  new positions info for every CarModel
	PosInfo carPoses[CarPosCnt][8];  // max 8 cars
	int iCurPoses[8];  // current index for carPoses queue
	std::map<int,int> carsCamNum;  // picked camera number for cars
	
	void newPoses(float time), newPerfTest(float time);  // vdrift
	void updatePoses(float time);  // ogre
	void UpdThr();

	//  replay - full, user saves
	//  ghost - saved when best lap
	//  ghplay - ghost ride replay, loaded if was on disk
	Replay replay, ghost, ghplay;
	Rewind rewind;  // to take car back in time (after crash etc.)
	TrackGhost ghtrk;  //  ghtrk - track's ghost

	std::vector<ReplayFrame> frm;  //size:4  //  frm - used when playing replay for hud and sounds

	bool isGhost2nd;  // if present (ghost but from other car)
	std::vector<float> vTimeAtChks;  // track ghost's times at road checkpoints
	float fLastTime;  // thk ghost total time
		

	boost::thread mThread;  // 2nd thread for simulation


	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameStart(Ogre::Real time);  void DoNetworking();
	virtual bool frameEnd(Ogre::Real time);
	float fLastFrameDT;
		
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
	void CreateCar();
	void CreateRoad();

	void CreateObjects(),DestroyObjects(bool clear);

	void NewGame();  void NewGameDoLoad();  bool IsVdrTrack();  bool newGameRpl;

	//  fluids to destroy
	std::vector<Ogre::String/*MeshPtr*/> vFlSMesh;
	std::vector<Ogre::Entity*> vFlEnt;
	std::vector<Ogre::SceneNode*> vFlNd;

	//  vdrift
	void CreateVdrTrack(std::string strack, class TRACK* pTrack),
		CreateRacingLine(), CreateRoadBezier();

	static Ogre::ManualObject* CreateModel(Ogre::SceneManager* sceneMgr, const Ogre::String& mat,
		class VERTEXARRAY* a, Ogre::Vector3 vPofs, bool flip, bool track=false, const Ogre::String& name="");

	
	//  Loading
	void LoadCleanUp(), LoadGame(), LoadScene(), LoadCar(), LoadTerrain(), LoadRoad(), LoadObjects(), LoadTrees(), LoadMisc();
	enum ELoadState { LS_CLEANUP=0, LS_GAME, LS_SCENE, LS_CAR, LS_TERRAIN, LS_ROAD, LS_OBJECTS, LS_TREES, LS_MISC, LS_ALL };
	static Ogre::String cStrLoad[LS_ALL+1];
	int curLoadState;
	std::map<int, std::string> loadingStates;

	float mTimer;  // wind,water


	//  mtr from ter  . . . 
	int blendMapSize;  char* blendMtr;
	void GetTerMtrIds();


	void recreateReflections();  // call after refl_mode changed

	virtual void materialCreated(sh::MaterialInstance* m, const std::string& configuration, unsigned short lodIndex);


	//  Input
	CInput* input;

	virtual bool keyPressed (const SDL_KeyboardEvent &arg);
	void channelChanged(ICS::Channel *channel, float currentValue, float previousValue);


	///  Gui
	//-----------------------------------------------------------------
	CGui* gui;
	CGuiCom* gcom;

	PreviewTex prvView,prvRoad,prvTer;  // track tab
	PreviewTex prvStCh;  // champ,chall stage view

	bool bRplPlay,bRplPause, bRplRec, bRplWnd;  //  game
	int carIdWin, iRplCarOfs;

	//  race pos
	int GetRacePos(float timeCur, float timeTrk, float carTimeMul, bool coldStart, float* pPoints=0);
	float GetCarTimeMul(const std::string& car, const std::string& sim_mode);

	void Ch_NewGame();


	///  graphs
	std::vector<GraphView*> graphs;
	void CreateGraphs(), DestroyGraphs();
	void UpdateGraphs(), GraphsNewVals();

	///* tire edit */
	int iEdTire, iTireLoad, iCurLat,iCurLong,iCurAlign, iUpdTireGr;

	///  car perf test
	bool bPerfTest;  EPerfTest iPerfTestStage;
	void PerfLogVel(class CAR* pCar, float time);
};
