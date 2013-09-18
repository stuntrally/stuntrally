#pragma once
#include "BaseApp.h"
#include "ReplayGame.h"
#include "../vdrift/cardefs.h"
#include "CarPosInfo.h"

#include <boost/thread.hpp>
#include <OgreShadowCameraSetup.h>
#include "../shiny/Main/Factory.hpp"
#include "../network/networkcallbacks.hpp"
#include "../oics/ICSChannelListener.h"


namespace Ogre {  class SceneNode;  class SceneManager;  class Light;
	class Terrain;  class TerrainGlobalOptions;  class TerrainGroup;  class TerrainPaging;  class PageManager;  }
namespace Forests {  class PagedGeometry;  }
namespace BtOgre  {  class DebugDrawer;  }
class WaterRTT;
class GAME;
class Scene;
class GraphView;
class CInput;
class CData;
const int CarPosCnt = 8;  // size of poses queue


class App : public BaseApp, public sh::MaterialListener,
			public ICS::ChannelListener
{
public:
	App(SETTINGS* settings, GAME* game);
	virtual ~App();
	
	GAME* pGame;
	

	//  BaseApp init
	void postInit(), SetFactoryDefaults();
		
	
	///  Game Cars Data
	//  new positions info for every CarModel
	PosInfo carPoses[CarPosCnt][8];  // max 8 cars
	int iCurPoses[8];  // current index for carPoses queue
	std::map<int,int> carsCamNum;  // picked camera number for cars
	Ogre::Quaternion qFixCar,qFixWh;  // utility

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


	Scene* sc;  /// scene.xml
	
	CData* data;  // all xmls

	Ogre::Light* sun;  void UpdFog(bool bForce=false), UpdSun();

	// vdrift static
	Ogre::StaticGeometry* mStaticGeom;
	
	// Weather  rain, snow
	Ogre::ParticleSystem *pr,*pr2;
	
	//  trees
	Forests::PagedGeometry *trees, *grass;
		
	Ogre::SceneManager* sceneMgr() { return mSceneMgr; };

	boost::thread mThread;  // 2nd thread for simulation

	WaterRTT* mWaterRTT;

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
	class CHud* hud;


	///  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	Ogre::String resCar, resTrk, resDrv;
	void CreateCar();
	void CreateSkyDome(Ogre::String sMater, Ogre::Vector3 scale);
	void CreateTerrain(bool bNewHmap=false, bool bTer=true), CreateBltTerrain(), GetTerAngles(int xb=0,int yb=0,int xe=0,int ye=0, bool full=true);
	void CreateTrees(), CreateRoad();

	void CreateObjects(),DestroyObjects(bool clear);
	void CreateFluids(), CreateBltFluids(), UpdateWaterRTT(Ogre::Camera* cam), DestroyFluids();

	void NewGame();  void NewGameDoLoad();  bool IsVdrTrack();  bool newGameRpl;

	//  fluids to destroy
	std::vector<Ogre::String/*MeshPtr*/> vFlSMesh;
	std::vector<Ogre::Entity*> vFlEnt;
	std::vector<Ogre::SceneNode*> vFlNd;

	// vdrift:
	void CreateVdrTrack(std::string strack, class TRACK* pTrack),
		CreateRacingLine(), CreateMinimap(), CreateRoadBezier();

	static Ogre::ManualObject* CreateModel(Ogre::SceneManager* sceneMgr, const Ogre::String& mat,
		class VERTEXARRAY* a, Ogre::Vector3 vPofs, bool flip, bool track=false, const Ogre::String& name="");

	
	// Loading
	void LoadCleanUp(), LoadGame(), LoadScene(), LoadCar(), LoadTerrain(), LoadRoad(), LoadObjects(), LoadTrees(), LoadMisc();
	enum ELoadState { LS_CLEANUP=0, LS_GAME, LS_SCENE, LS_CAR, LS_TERRAIN, LS_ROAD, LS_OBJECTS, LS_TREES, LS_MISC, LS_ALL };
	static Ogre::String cStrLoad[LS_ALL+1];
	int curLoadState;
	std::map<int, std::string> loadingStates;

	float mTimer;  // wind,water


	///  terrain
	Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;  bool mPaging;
	Ogre::TerrainPaging* mTerrainPaging;  Ogre::PageManager* mPageManager;
	//Vector3 getNormalAtWorldPosition(Terrain* terrain, Real x, Real z, Real s);

	Ogre::Terrain* terrain; 
	int iBlendMaps, blendMapSize;	bool noBlendUpd;  //  mtr from ter  . . . 
	char* blendMtr;  // mtr [blendMapSize x blendMapSize]

	void initBlendMaps(Ogre::Terrain* terrin, int xb=0,int yb=0, int xe=0,int ye=0, bool full=true);
	void configureTerrainDefaults(Ogre::Light* l), UpdTerErr();
	float Noise(float x, float zoom, int octaves, float persistance);
	float Noise(float x, float y, float zoom, int octaves, float persistance);
	//     xa  xb
	//1    .___.
	//0__./     \.___
	//   xa-s    xb+s
	inline float linRange(const float& x, const float& xa, const float& xb, const float& s)  // min, max, smooth range
	{
		if (x <= xa-s || x >= xb+s)  return 0.f;
		if (x >= xa && x <= xb)  return 1.f;
		if (x < xa)  return (x-xa)/s+1;
		if (x > xb)  return (xb-x)/s+1;
		return 0.f;
	}

	//  road
	class SplineRoad* road;

	//  shadows
	void changeShadows(), UpdPSSMMaterials(), setMtrSplits(Ogre::String sMtrName);
	Ogre::Vector4 splitPoints;

	Ogre::ShadowCameraSetupPtr mPSSMSetup;
	void recreateReflections();  // call after refl_mode changed

	virtual void materialCreated(sh::MaterialInstance* m, const std::string& configuration, unsigned short lodIndex);


	//  Input
	CInput* input;

	virtual bool keyPressed (const SDL_KeyboardEvent &arg);
	void channelChanged(ICS::Channel *channel, float currentValue, float previousValue);


	///  Gui
	//-----------------------------------------------------------------
	class CGui* gui;

	bool bRplPlay,bRplPause, bRplRec, bRplWnd;  //  game
	int carIdWin, iRplCarOfs;

	//  race pos
	int GetRacePos(float timeCur, float timeTrk, float carTimeMul, bool coldStart, float* pPoints=0);
	float GetCarTimeMul(const std::string& car, const std::string& sim_mode);

	void Ch_NewGame();


	///  graphs
	std::vector<GraphView*> graphs;
	void CreateGraphs(),DestroyGraphs();
	void UpdateGraphs(),GraphsNewVals();

	///* tire edit */
	int iEdTire, iTireLoad, iCurLat,iCurLong,iCurAlign, iUpdTireGr;

	///  car perf test
	bool bPerfTest;  EPerfTest iPerfTestStage;
	void PerfLogVel(class CAR* pCar, float time);
};
