#ifndef _OgreGame_h_
#define _OgreGame_h_

#include "BaseApp.h"
#include "common/Gui_Popup.h"
#include "common/SceneXml.h"
#include "common/BltObjects.h"
#include "common/TracksXml.h"
#include "common/FluidsXml.h"
#include "common/WaterRTT.h"
#include "ChampsXml.h"
#include "ChallengesXml.h"

#include "ReplayGame.h"
#include "../vdrift/cardefs.h"
#include "CarModel.h"
#include "CarReflection.h"

#include "common/MessageBox/MessageBox.h"
#include "common/MessageBox/MessageBoxStyle.h"
#include "common/GraphView.h"

#include "../network/networkcallbacks.hpp"
#include <boost/thread.hpp>
#include <MyGUI.h>
#include <OgreShadowCameraSetup.h>

#include "../shiny/Main/Factory.hpp"


namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;  class Viewport;  class Light;
	class Terrain;  class TerrainGlobalOptions;  class TerrainGroup;  class TerrainPaging;  class PageManager;  }
namespace Forests {  class PagedGeometry;  }
namespace BtOgre  {  class DebugDrawer;  }
namespace MyGUI  {  class MultiList2;  }
namespace OISB   {  class AnalogAxisAction;  }
namespace MyGUI  {  class Slider;  }
class GraphView;
const int CarPosCnt = 8;  // size of poses queue


class App : public BaseApp, public GameClientCallback, public MasterClientCallback, public sh::MaterialListener
{
public:
	App(SETTINGS* settings, GAME* game);
	virtual ~App();
	void NullHUD();
	
	class GAME* pGame;  ///*
	void updatePoses(float time), newPoses(float time), newPerfTest(float time);
	void UpdThr();

	virtual bool keyPressed (const SDL_KeyboardEvent &arg);
	
	// stuff to be executed after BaseApp init
	void postInit(), SetFactoryDefaults();
	
	void setTranslations();
	
	std::vector<GraphView*> graphs;  /// graphs
	void CreateGraphs(),DestroyGraphs(),UpdateGraphs(),GraphsNewVals();
	int iEdTire, iTireLoad, iCurLat,iCurLong,iCurAlign, iUpdTireGr;  ///* tire edit */
	//  car perf test
	bool bPerfTest;  EPerfTest iPerfTestStage;
	void PerfLogVel(class CAR* pCar, float time);
	
	//  new positions info for every CarModel
	PosInfo carPoses[CarPosCnt][8];  // max 8 cars
	int iCurPoses[8];  // current index for carPoses queue
	std::map<int,int> carsCamNum;  // picked camera number for cars
	Ogre::Quaternion qFixCar,qFixWh;  // utility

	//  replay - full, user saves
	//  ghost - saved when best lap
	//  ghplay - ghost ride replay, loaded if was on disk
	Replay replay, ghost, ghplay;
	Rewind rewind;  // to take car back in time (after crash etc.)
	TrackGhost ghtrk;  //  ghtrk - track's ghost
	std::vector<ReplayFrame> frm;  //size:4  //  frm - used when playing replay for hud and sounds
	const Ogre::String& GetGhostFile(std::string* ghCar=NULL);  std::string GetRplListDir();
	bool isGhost2nd;  // if present (ghost but from other car)

	Scene* sc;  /// scene.xml
	FluidsXml fluidsXml;  /// fluid params xml
	BltObjects objs;  // veget collision in bullet
	Ogre::Light* sun;  void UpdFog(bool bForce=false), UpdSun();

	// vdrift static
	Ogre::StaticGeometry* mStaticGeom;
	
	// Weather  rain, snow
	Ogre::ParticleSystem *pr,*pr2;
	
	//  trees
	Forests::PagedGeometry *trees, *grass;
	
	Ogre::SceneNode* arrowNode; // checkpoint arrow
	Ogre::SceneNode* arrowRotNode; // seperate node for rotation
	Ogre::Quaternion arrowAnimStart, arrowAnimEnd, arrowAnimCur; // smooth animation
		
	void UpdateHUD(int carId, float time), ShowHUDvp(bool vp),
		SizeHUD(bool full, Ogre::Viewport* vp=NULL);
	void UpdHUDRot(int baseCarId, int carId, float vel, float rpm);
	void GetHUDVals(int id, float* vel, float* rpm, float* clutch, int* gear);
	
	Ogre::SceneManager* sceneMgr() { return mSceneMgr; };

protected:
	boost::thread mThread;
	WaterRTT mWaterRTT;

	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameStart(Ogre::Real time);  void DoNetworking();
	virtual bool frameEnd(Ogre::Real time);
	float fLastFrameDT;
		
	BtOgre::DebugDrawer *dbgdraw;  /// blt dbg
	void bltDumpRecursive(class CProfileIterator* profileIterator, int spacing, std::stringstream& os);
	void bltDumpAll(std::stringstream& os);

	//  mtr reload
	enum eMaterials {
		Mtr_CarBody, Mtr_CarInterior, Mtr_CarGlass,
		Mtr_CarTireFront, Mtr_CarTireRear,
		Mtr_Road,  NumMaterials  };
	Ogre::String sMtr[NumMaterials];

	///  HUD, 2D  ------------
	class Hud  // for 1 viewport
	{
	public:
		//  gear, vel
		MyGUI::TextBox *txGear,*txVel,*txBFuel,*txDamage;
		//  times bar
		MyGUI::TextBox *txTimTxt,*txTimes;  MyGUI::ImageBox *bckTimes;
		Ogre::String sTimes;
		//  gauges
		Ogre::SceneNode *ndRpm, *ndVel, *ndRpmBk, *ndVelBk,*ndVelBm;
		Ogre::ManualObject* moRpm, *moVel, *moRpmBk, *moVelBk,*moVelBm;
		//  miniap
		Ogre::ManualObject* moMap;
		Ogre::SceneNode *ndMap;
		std::vector<Ogre::SceneNode*> vNdPos;  // car pos tris on minimap +2ghosts
		std::vector<Ogre::ManualObject*> vMoPos;  //size: 6
		
		Hud();
		void Null();
	};
	std::vector<Hud> hud;  // size: max viewports 4
	float asp, scX,scY, minX,maxX, minY,maxY;  // minimap visible range
	Ogre::SceneNode *ndLine;

	Ogre::ManualObject* Create2D(const Ogre::String& mat, Ogre::SceneManager* sceneMgr,
		Ogre::Real size, bool dyn = false, bool clr = false);

	struct OvrDbg
	{
		Ogre::OverlayElement* oL,*oR,*oS, *oU,*oX;
		OvrDbg();
	};
	std::vector<OvrDbg> ov;
	Ogre::OverlayElement *hudCountdown,*hudNetMsg,
		*hudAbs,*hudTcs, *hudWarnChk,*hudWonPlace, *hudOpp[6][3],*hudOppB;
	Ogre::Overlay *ovCountdown,*ovNetMsg, *ovCam, *ovWarnWin, *ovOpp, *ovAbsTcs, *ovCarDbg,*ovCarDbgTxt,*ovCarDbgExt;

	Ogre::String GetTimeString(float time) const, GetTimeShort(float time) const;
	void CreateHUD(bool destroy), ShowHUD(bool hideAll=false), UpdMiniTer(), UpdDbgTxtClr();
	Ogre::Vector3 projectPoint(const Ogre::Camera* cam, const Ogre::Vector3& pos);  // 2d xy, z - out info
	MyGUI::TextBox* CreateNickText(int carId, Ogre::String text);


	///  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	Ogre::String resCar, resTrk, resDrv;
	void CreateCar();
	void CreateTerrain(bool bNewHmap=false, bool bTer=true), CreateBltTerrain(), GetTerAngles(int xb=0,int yb=0,int xe=0,int ye=0, bool full=true);
	void CreateTrees(), CreateRoad(), CreateObjects(),DestroyObjects(bool clear);
	void CreateFluids(), CreateBltFluids(), UpdateWaterRTT(Ogre::Camera* cam), DestroyFluids();
	void CreateSkyDome(Ogre::String sMater, Ogre::Vector3 scale);
	void NewGame();  void NewGameDoLoad();  bool IsVdrTrack();  bool newGameRpl;
	//  fluids to destroy
	std::vector<Ogre::String/*MeshPtr*/> vFlSMesh;
	std::vector<Ogre::Entity*> vFlEnt;
	std::vector<Ogre::SceneNode*> vFlNd;
	// vdrift:
	void CreateVdrTrack(std::string strack, class TRACK* pTrack),
		CreateRacingLine(), CreateMinimap(), CreateRoadBezier();
public:
	static Ogre::ManualObject* CreateModel(Ogre::SceneManager* sceneMgr, const Ogre::String& mat,
		class VERTEXARRAY* a, Ogre::Vector3 vPofs, bool flip, bool track=false, const Ogre::String& name="");
protected:
	
	// Loading
	void LoadCleanUp(), LoadGame(), LoadScene(), LoadCar(), LoadTerrain(), LoadRoad(), LoadObjects(), LoadTrees(), LoadMisc();
	enum ELoadState { LS_CLEANUP=0, LS_GAME, LS_SCENE, LS_CAR, LS_TERRAIN, LS_ROAD, LS_OBJECTS, LS_TREES, LS_MISC, LS_ALL };
	
	// id, display name, initialised in App()
	// e.g.: 0, Cleaning up or 3, Loading scene
	std::map<int, std::string> loadingStates;
	// 1 behind map ( map.end() ): loading finished
	std::map<int, std::string>::iterator curLoadState;

	float mTimer;


	///  terrain
	Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;  bool mPaging;
	Ogre::TerrainPaging* mTerrainPaging;  Ogre::PageManager* mPageManager;
	//Vector3 getNormalAtWorldPosition(Terrain* terrain, Real x, Real z, Real s);

public:
	Ogre::Terrain* terrain; 
	int iBlendMaps, blendMapSize;	bool noBlendUpd;  //  mtr from ter  . . . 
	char* blendMtr;  // mtr [blendMapSize x blendMapSize]
protected:
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

public:
	//  road
	class SplineRoad* road;

	void changeShadows(), UpdPSSMMaterials(), setMtrSplits(Ogre::String sMtrName);
	Ogre::Vector4 splitPoints;

protected:
	Ogre::ShadowCameraSetupPtr mPSSMSetup;
	void recreateReflections();  // call after refl_mode changed


	///-----------------------------------------------------------------------------------------------------------------
	///  Gui
	///-----------------------------------------------------------------------------------------------------------------
	//  size
	void SizeGUI(); void doSizeGUI(MyGUI::EnumeratorWidgetPtr);
	std::vector<MyGUI::TabControl*> vSubTabsGame,vSubTabsOpts;

	//  shortcuts
	typedef MyGUI::WidgetPtr WP;
	typedef std::list <std::string> strlist;
	//  slider event and its text field for value
	#define SLV(name)  void sl##name(SL);  MyGUI::StaticTextPtr val##name;
	#define SL  MyGUI::Slider* wp, float val
	#define CMB MyGUI::ComboBox* wp, size_t val // combobox event args


	///  Gui common   --------------------------
	//  graphics
	SLV(Anisotropy);  SLV(ViewDist);  SLV(TerDetail);  SLV(TerDist);  SLV(RoadDist);
	SLV(TexSize);  SLV(TerMtr);  SLV(TerTripl);  // detail
	SLV(Trees);  SLV(Grass);  SLV(TreesDist);  SLV(GrassDist);  // paged
	SLV(Shaders);  SLV(ShadowType);  SLV(ShadowCount);  SLV(ShadowSize);  SLV(ShadowDist);  //SLV(ShadowFilter); // shadow
	SLV(WaterSize);  // screen
	void comboTexFilter(CMB), btnShadows(WP), btnShaders(WP), btnTrGrReset(WP),
		chkWaterReflect(WP), chkWaterRefract(WP),
		chkUseImposters(WP), chkImpostorsOnly(WP), cmbAntiAliasing(CMB);
	void setOrigPos(WP wp, const char* relToWnd);

	//  tooltip
	WP mToolTip;  MyGUI::EditPtr mToolTipTxt;
	void setToolTips(MyGUI::EnumeratorWidgetPtr widgets);
	void notifyToolTip(MyGUI::Widget* sender, const MyGUI::ToolTipInfo& info);
	void boundedMove(MyGUI::Widget *moving, const MyGUI::IntPoint & point);

	//  language
	void comboLanguage(CMB);
	std::map<std::string, MyGUI::UString> languages; // <short name, display name>
	bool bGuiReinit;  void UnfocusLists();
	MyGUI::ButtonPtr bnQuit;  void btnQuit(WP);

	//  init
	void InitGui();  bool bGI;
	void GuiCenterMouse(),GuiInitTooltip(),GuiInitLang(), GuiInitGraphics(),GuiInitTrack();
	Ogre::String GetSceneryColor(Ogre::String name);
	void AddTrkL(std::string name, int user, const class TrackInfo* ti);

	///  track
	void UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const Ogre::String& sTrack, float timeCur, bool champ=false),
		ReadTrkStats(), ReadTrkStatsChamp(Ogre::String track,bool reverse);
	MyGUI::MultiList2* trkList;  MyGUI::EditPtr trkDesc[2];
	MyGUI::StaticImagePtr imgPrv[2],imgMini[2],imgTer[2], imgTrkIco1,imgTrkIco2;
	const static int StTrk = 12, InfTrk = 11;
	MyGUI::StaticTextPtr valTrkNet, stTrk[2][StTrk], infTrk[2][InfTrk];  // [2] 2nd set is for champs

	void listTrackChng(MyGUI::MultiList2* li, size_t pos), TrackListUpd(bool resetNotFound=false);
	TracksXml tracksXml;  CarsXml carsXml;  UserXml userXml;  //xml
	void btnTrkView1(WP),btnTrkView2(WP),ChangeTrackView();
	void updTrkListDim(), updChampListDim();
	//  const list column widths
	const static int colTrk[32],colCar[16],colCh[16],colChL[16],colSt[16];
	const static Ogre::String clrsDiff[9],clrsRating[6],clrsLong[10];

	void edTrkFind(MyGUI::EditPtr),edRplFind(MyGUI::EditPtr);
	Ogre::String sTrkFind,sRplFind;  MyGUI::EditPtr edFind;
	strlist liTracks,liTracksUser;  void FillTrackLists();
	std::list<TrkL> liTrk;

	void CarListUpd(bool resetNotFound=false);
	void AddCarL(std::string name, const class CarInfo* ci);
	std::list<CarL> liCar;  void FillCarList();

	//  screen
	MyGUI::ListPtr resList;
	void InitGuiScreenRes(), btnResChng(WP), ResizeOptWnd();
	void chkVidFullscr(WP), chkVidVSync(WP);

	void comboGraphicsAll(CMB), comboRenderSystem(CMB);
		
	///-----------------------------------------


	//  main menu
	void toggleGui(bool toggle=true), GuiShortcut(MNU_Btns mnu, int tab, int subtab=-1);
	void MainMenuBtn(MyGUI::WidgetPtr), MenuTabChg(MyGUI::TabPtr, size_t);  bool loadReadme;

	void UpdCarClrSld(bool upd=true), UpdCarMClr();  bool bUpdCarClr;
	void btnNetEndClose(WP);

	//  race pos
	int GetRacePos(float timeCur, float timeTrk, float carTimeMul, bool coldStart, float* pPoints=0);
	float GetCarTimeMul(const std::string& car, const std::string& sim_mode);


	///  championships & challenges
	MyGUI::ButtonPtr btStTut, btStChamp, btStChall;
	MyGUI::StaticImagePtr imgTut, imgChamp, imgChall;
	//  tabs
	MyGUI::TabPtr tabTut, tabChamp, tabChall;
	void tabTutType(MyGUI::TabPtr wp, size_t id), tabChampType(MyGUI::TabPtr wp, size_t id);
	void tabChallType(MyGUI::TabPtr wp, size_t id);

	//  stages
	MyGUI::EditBox* edChInfo,*edChDesc;  MyGUI::Widget* panCh;
	MyGUI::TextBox* txtCh,*valCh,*txtChP[3],*valChP[3];  // stages info, pass/progress
	void btnStageNext(WP), btnStagePrev(WP);  MyGUI::StaticText* valStageNum;
	void StageListAdd(int n, Ogre::String name, int laps, Ogre::String progress);
	
	//  xml  [1]= reversed  L= challenge
	ChampsXml champs;  ProgressXml progress[2];
	ChallXml chall;  ProgressLXml progressL[2];
	void ProgressSave(bool upgGui=true), ProgressLSave(bool upgGui=true);
	Chall* pChall;  // current challenge or 0 if not

	//  load
	void Ch_NewGame(), Ch_XmlLoad(), Ch_LoadEnd();
	void UpdChallDetail(int id);
	//  const
	const static Ogre::String StrPrize(int i/*0 none..3 gold*/), strPrize[4],clrPrize[4];
	const static int ciAddPos[3];  const static float cfSubPoints[3];
	
	//  common
	MyGUI::MultiList2* liStages, *liNetEnd;  void listStageChng(MyGUI::MultiList2* li, size_t pos);
	MyGUI::MultiList2* liChamps;  void listChampChng(MyGUI::MultiList2* li, size_t pos);
	MyGUI::MultiList2* liChalls;  void listChallChng(MyGUI::MultiList2* li, size_t pos);

	void btnChampStart(WP), btnChampEndClose(WP), btnChampStageBack(WP), btnChampStageStart(WP);
	void btnChallStart(WP), btnChallEndClose(WP), btnChallStageBack(WP), btnChallStageStart(WP);
	void btnChRestart(WP);  MyGUI::ButtonPtr btChRestart;

	MyGUI::ButtonPtr btChampStage, btChallStage;
	MyGUI::EditBox* edChampStage,*edChampEnd;  MyGUI::ImageBox* imgChampStage,*imgChampEndCup;
	MyGUI::EditBox* edChallStage,*edChallEnd;  MyGUI::ImageBox* imgChallStage;
	MyGUI::ImageBox* imgChallFail,*imgChallCup;  MyGUI::TextBox* txChallEndC,*txChallEndF;

	//  main
	void ChampsListUpdate(), ChampFillStageInfo(bool finished), ChampionshipAdvance(float timeCur);
	void ChallsListUpdate(), ChallFillStageInfo(bool finished), ChallengeAdvance(float timeCur);
	void btnChampInfo(WP), chkChampRev(WP), UpdChampTabVis();
	void ToolGhosts(),ToolGhostsConv();  //  _Tools_

	//  chall util
	Ogre::String StrChallCars(const Chall& ch);
	bool IsChallCar(Ogre::String name);
	bool isChallGui();  void BackFromChs();


	///  input tab
	///-----------------------------------------------------------------------------------------------------------------
	struct InputAction
	{
		std::string mName;  int mId;
		SDL_Keycode mDefaultIncrease, mDefaultDecrease;

		enum Type
		{	Trigger = 0x00,
			Axis = 0x01,     // 2-sided axis, centered in the middle, keyboard emulation with left & right keys
			HalfAxis = 0x11  // 1-sided axis, keyboard emulation with 1 key
		} mType;

		ICS::InputControlSystem* mICS;
		ICS::Control* mControl;

		InputAction(int id, const std::string& name, SDL_Keycode defaultKey, Type type)
			: mId(id), mName(name), mDefaultIncrease(defaultKey), mDefaultDecrease(SDLK_UNKNOWN), mType(type)
		{	}
		InputAction(int id, const std::string &name, SDL_Keycode defaultDecrease, SDL_Keycode defaultIncrease, Type type)
			: mId(id), mName(name), mDefaultIncrease(defaultIncrease), mDefaultDecrease(defaultDecrease), mType(Axis)
		{	}
	};
public:
	// These IDs are referenced in the user config files.
	// To keep them valid, make sure to:
	// - Add new actions at the end of the enum
	// - Instead of deleting an action, replace it with a dummy one eg A_Unused
	enum Actions
	{	A_ShowOptions, A_PrevTab, A_NextTab, A_RestartGame, A_ResetGame, A_Screenshot, NumActions	};
	enum PlayerActions
	{	A_Throttle, A_Brake, A_Steering, A_HandBrake, A_Boost, A_Flip,
		A_ShiftUp, A_ShiftDown, // TODO: Shift up/down could be a single "shift" action
		A_PrevCamera, A_NextCamera, A_LastChk, A_Rewind, NumPlayerActions
	};
	float mPlayerInputState[4][NumPlayerActions];
	boost::mutex mPlayerInputStateMutex;
protected:
	void CreateInputTab(const std::string& title, bool playerTab, const std::vector<InputAction>& actions, ICS::InputControlSystem* ICS);
	void InitInputGui(), inputBindBtnClicked(WP), inputUnbind(WP), inputBindBtn2(WP, int, int, MyGUI::MouseButton mb);
	// bind=1: "<Assign>"
	// bind=2: "Key1, <Assign>"
	// bind=0: "Key1, Key2"
	void UpdateInputButton(MyGUI::Button* button, const InputAction& action, int bind=0);
	void LoadInputDefaults();
	void LoadInputDefaults(std::vector<InputAction>& actions, ICS::InputControlSystem* ICS);

	InputAction* mBindingAction;
	MyGUI::Button* mBindingSender;
	virtual void mouseAxisBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		ICS::InputControlSystem::NamedAxis axis, ICS::Control::ControlChangingDirection direction);
	virtual void keyBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		SDL_Keycode key, ICS::Control::ControlChangingDirection direction);
	virtual void mouseButtonBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		unsigned int button, ICS::Control::ControlChangingDirection direction);
	virtual void joystickAxisBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		int deviceId, int axis, ICS::Control::ControlChangingDirection direction);
	virtual void joystickButtonBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		int deviceId, unsigned int button, ICS::Control::ControlChangingDirection direction);
	virtual void joystickPOVBindingDetected(ICS::InputControlSystem* ICS, ICS::Control* control,
		int deviceId, int pov, ICS::InputControlSystem::POVAxis axis, ICS::Control::ControlChangingDirection direction);

	std::vector<InputAction> mInputActions;
	std::vector<InputAction> mInputActionsPlayer[4];

	virtual void channelChanged(ICS::Channel* channel, float currentValue, float previousValue);
	virtual void notifyInputActionBound(bool complete);

	bool actionIsActive(std::string, std::string);
	void UpdateInputBars(), inputDetailBtn(WP);

	//  joy events
	MyGUI::TabPtr tabInput;  void tabInputChg(MyGUI::TabPtr, size_t);
	MyGUI::StaticTextPtr txtInpDetail;  MyGUI::WidgetPtr panInputDetail;  MyGUI::ButtonPtr chOneAxis;
	MyGUI::EditPtr edInputIncrease;  void editInput(MyGUI::EditPtr), btnInputInv(WP), chkOneAxis(WP);
	void comboInputKeyAllPreset(CMB);  bool TabInputId(int* pId);


	///  tweak  -----------------------------------------
	const static int ciEdCar = 12;
	MyGUI::EditPtr edCar[ciEdCar],edPerfTest, edTweakCol;  MyGUI::TabPtr tabTweak, tabEdCar;
	MyGUI::StaticTextPtr txtTweakPath, txtTweakTire, txtTweakPathCol;
	MyGUI::ComboBoxPtr cmbTweakCarSet, cmbTweakTireSet;
	void CmbTweakCarSet(CMB), CmbTweakTireSet(CMB), CmbEdTweakCarSet(MyGUI::EditPtr), CmbEdTweakTireSet(MyGUI::EditPtr);
	void TweakToggle(), TweakCarSave(),TweakCarLoad(), TweakTireSave();
	void TweakColUpd(bool user), TweakColLoad(),TweakColSave();
	void btnTweakCarSave(WP),btnTweakCarLoad(WP), btnTweakTireSave(WP), btnTweakColSave(WP);
	void tabCarEdChng(MyGUI::TabPtr, size_t);
	//  graphs
	MyGUI::ComboBox* cmbGraphs;  void comboGraphs(CMB);  MyGUI::StaticTextPtr valGraphsType;


	//  sliders  -----------------------------------------
	SLV(Particles);  SLV(Trails);
	SLV(ReflSkip);  SLV(ReflSize);  SLV(ReflFaces);  SLV(ReflDist);  SLV(ReflMode); // refl
	SLV(SizeGaug);  SLV(TypeGaug);  SLV(SizeMinimap);  SLV(SizeArrow);  SLV(ZoomMinimap);
	SLV(CountdownTime);  // view
	SLV(DbgTxtClr);  SLV(DbgTxtCnt);
	SLV(VolMaster);  SLV(VolEngine);  SLV(VolTires);  SLV(VolSusp);  SLV(VolEnv);  // sounds
	SLV(VolFlSplash);  SLV(VolFlCont);  SLV(VolCarCrash);  SLV(VolCarScrap);
	
	SLV(CarClrH);  SLV(CarClrS);  SLV(CarClrV);  SLV(CarClrGloss);  SLV(CarClrRefl);  // car clr
	SLV(BloomInt);  SLV(BloomOrig);  SLV(BlurIntens);  // video
	SLV(DepthOfFieldFocus);  SLV(DepthOfFieldFar);  // dof
	SLV(HDRParam1);  SLV(HDRParam2);  SLV(HDRParam3);  // hdr
	SLV(HDRBloomInt);  SLV(HDRBloomOrig);  SLV(HDRAdaptationScale);
	SLV(HDRVignettingRadius);  SLV(HDRVignettingDarkness);
	SLV(NumLaps);  SLV(RplNumViewports);  // setup
	SLV(SSSEffect);  SLV(SSSVelFactor);  SLV(SteerRangeSurf);  SLV(SteerRangeSim);
	
	//  checks
	void chkGauges(WP),	chkArrow(WP),chkBeam(WP), chkDigits(WP),
		chkMinimap(WP), chkMiniZoom(WP), chkMiniRot(WP), chkMiniTer(WP), chkMiniBorder(WP),  // view
		chkFps(WP), chkWireframe(WP), 
		chkCamInfo(WP), chkTimes(WP), chkOpponents(WP), chkOpponentsSort(WP), chkCamTilt(WP),
		chkCarDbgBars(WP), chkCarDbgTxt(WP), chkCarDbgSurf(WP), chkGraphs(WP),
		chkBltDebug(WP), chkBltProfilerTxt(WP), chkProfilerTxt(WP),
		chkReverse(WP), chkParticles(WP), chkTrails(WP),
		chkAbs(WP), chkTcs(WP), chkGear(WP), chkRear(WP), chkRearInv(WP),  // car
		chkOgreDialog(WP), chkAutoStart(WP), chkEscQuits(WP),
		chkStartInMain(WP), chkBltLines(WP), chkLoadPics(WP), chkMultiThread(WP),  // startup
		chkVidEffects(WP), chkVidBloom(WP), chkVidHDR(WP), chkVidBlur(WP), UpdBloomVals(), chkVidSSAO(WP), // effects
		chkVidSoftParticles(WP), chkVidGodRays(WP), chkVidDepthOfField(WP), chkVidBoostFOV(WP),
		chkVegetCollis(WP), chkCarCollis(WP), chkRoadWCollis(WP), chkDynObjects(WP);  //game

	// gui car tire set gravel/asphalt
	int iTireSet;
	void tabTireSet(MyGUI::TabPtr wp, size_t id);
	MyGUI::ButtonPtr bchAbs,bchTcs;  MyGUI::Slider* slSSSEff,*slSSSVel,*slSteerRngSurf,*slSteerRngSim;

	void imgBtnCarClr(WP), btnCarClrRandom(WP), toggleWireframe();
	MyGUI::ButtonPtr bRkmh, bRmph;  void radKmh(WP), radMph(WP);
	MyGUI::ButtonPtr bRsimEasy, bRsimNorm;  void radSimEasy(WP), radSimNorm(WP);  bool bReloadSim;
	MyGUI::ButtonPtr chFps,chWire, chBlt,chBltTxt, chProfTxt, chDbgT,chDbgB,chDbgS, chGraphs,
		chTimes,chMinimp,chOpponents;

	///  replay  -----------------------------
	MyGUI::StaticTextPtr valRplPerc, valRplCur, valRplLen,
		valRplName,valRplInfo,valRplName2,valRplInfo2;
	MyGUI::Slider* slRplPos;  void slRplPosEv(SL);
	MyGUI::EditPtr edRplName, edRplDesc;
	void btnRplLoad(WP), btnRplSave(WP), btnRplDelete(WP), btnRplRename(WP),  // btn
		chkRplAutoRec(WP), chkRplChkGhost(WP), chkRplChkBestOnly(WP), chkRplChkPar(WP),
		chkRplChkRewind(WP), chkRplChkGhostOther(WP), chkRplChkTrackGhost(WP),  // replay
		btnRplToStart(WP),btnRplToEnd(WP), btnRplPlay(WP),  // controls
		btnRplCur(WP),btnRplAll(WP),chkRplGhosts(WP);  // radio
	MyGUI::ButtonPtr rbRplCur, rbRplAll;

	void btnRplBackDn(WP,int,int,MyGUI::MouseButton),btnRplBackUp(WP,int,int,MyGUI::MouseButton);
	void btnRplFwdDn(WP,int,int,MyGUI::MouseButton),btnRplFwdUp(WP,int,int,MyGUI::MouseButton);
	bool bRplBack,bRplFwd;
		
	void msgRplDelete(MyGUI::Message*, MyGUI::MessageBoxStyle);
	
	void btnNumPlayers(WP);  void chkSplitVert(WP), chkStartOrd(WP);
	MyGUI::StaticTextPtr valLocPlayers;
	
	MyGUI::StaticTextPtr txCarStatsTxt,txCarStatsVals,
		txCarSpeed,txCarType, txCarAuthor,txTrackAuthor;
	void UpdCarStatsTxt();  // car stats


public:
	bool bRplPlay,bRplPause, bRplRec, bRplWnd;  //  game
	int carIdWin, iCurCar, iRplCarOfs;
protected:
	MyGUI::ButtonPtr btRplPl;  void UpdRplPlayBtn();
	///---------------------------------------

	//  game
	void btnNewGame(WP),btnNewGameStart(WP);
	MyGUI::MultiList2* carList;  MyGUI::ListPtr rplList;  void updReplaysList();
	void listRplChng(MyGUI::List* li, size_t pos),  changeCar();
	void listCarChng(MyGUI::MultiList2* li, size_t pos),  changeTrack();
	int LNext(MyGUI::MultiList2* lp, int rel, int ofs), LNext(MyGUI::ListPtr lp, int rel, int ofs),
		LNext(MyGUI::MultiList* lp, int rel);  // util next in list
	void LNext(int rel);  void tabPlayer(MyGUI::TabPtr wp, size_t id);

	Ogre::String sListCar,sListTrack;  int bListTrackU;
	Ogre::String pathTrk[2];  Ogre::String TrkDir();
	Ogre::String PathListTrk(int user=-1), PathListTrkPrv(int user/*=-1*/, Ogre::String track);

	MyGUI::StaticImagePtr imgCar;  MyGUI::EditPtr carDesc;
	MyGUI::ComboBoxPtr cmbBoost, cmbFlip, cmbDamage, cmbRewind;
	void comboBoost(CMB), comboFlip(CMB), comboDamage(CMB), comboRewind(CMB);

	GuiPopup popup;

	///  multiplayer  ---------------------------------------
	void rebuildGameList(), rebuildPlayerList();
	void updateGameInfo(), updateGameSet(), updateGameInfoGUI();
	void setNetGuiHosting(bool enabled);
	void gameListChanged(protocol::GameList list);

	void peerConnected(PeerInfo peer), peerDisconnected(PeerInfo peer);
	void peerInfo(PeerInfo peer);
	void peerMessage(PeerInfo peer, std::string msg);
	void peerState(PeerInfo peer, uint8_t state);
	void gameInfo(protocol::GameInfo game);
	void startRace(), returnToLobby();
	void timeInfo(ClientID id, uint8_t lap, double time);
	void error(std::string what);
	void join(std::string host, std::string port, std::string password);
	void uploadGameInfo();

	mutable boost::mutex netGuiMutex;
	MyGUI::UString sChatBuffer,sChatLast1,sChatLast2;  int iChatMove;
	void AddChatMsg(const MyGUI::UString& clr, const MyGUI::UString& msg, bool add=true);
	protocol::GameInfo netGameInfo;

	bool bRebuildPlayerList, bRebuildGameList;
	bool bUpdateGameInfo, bStartGame, bStartedGame, bUpdChat;

	///  multiplayer gui  --------------------
	MyGUI::TabPtr tabsNet;  //void tabNet(TabPtr tab, size_t id);
	MyGUI::WidgetPtr panelNetServer,panelNetGame,panelNetTrack;
	MyGUI::MultiListPtr listServers, listPlayers;
	MyGUI::EditPtr edNetChat;  // chat area, set text through sChatBuffer
	int iColLock,iColHost,iColPort;  //ids of columns in listServers, set in gui init

	MyGUI::ButtonPtr btnNetRefresh,btnNetJoin,btnNetCreate,btnNetDirect;
	MyGUI::ButtonPtr btnNetReady,btnNetLeave;
	void evBtnNetRefresh(WP);
	void evBtnNetJoin(WP), evBtnNetJoinLockedClose();
	void evBtnNetCreate(WP);
	void evBtnNetDirect(WP),evBtnNetDirectClose();
	void evBtnNetReady(WP),evBtnNetLeave(WP);

	MyGUI::StaticTextPtr valNetGameName, valNetChat, valNetGameInfo, valNetPassword;
	MyGUI::ButtonPtr btnNetSendMsg;  void chatSendMsg();
	MyGUI::EditPtr edNetGameName, edNetChatMsg, edNetPassword,
		edNetNick, edNetServerIP, edNetServerPort, edNetLocalPort;
	void evEdNetGameName(MyGUI::EditPtr), evEdNetPassword(MyGUI::EditPtr),
		evEdNetNick(MyGUI::EditPtr), evEdNetServerIP(MyGUI::EditPtr),
		evEdNetServerPort(MyGUI::EditPtr), evEdNetLocalPort(MyGUI::EditPtr);


public:
	virtual void materialCreated(sh::MaterialInstance* m, const std::string& configuration, unsigned short lodIndex);

	bool GetCarPath(std::string* pathCar/*out*/,
		std::string* pathSave/*=0*/, std::string* pathSaveDir/*=0*/,
		std::string carname, /*std::string tweakSetup="",*/ bool forceOrig=false);
};

#endif
