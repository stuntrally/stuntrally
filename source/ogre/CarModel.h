#pragma once
#include <Ogre.h>
// #include <OgreVector2.h>
// #include <OgreVector3.h>
// #include <OgreQuaternion.h>
// #include <OgreVector4.h>
// #include <OgreMatrix4.h>
// #include <OgreColourValue.h>
#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
#include "../shiny/Main/MaterialInstance.hpp"
#include "../vdrift/cardefs.h"
#include "CarPosInfo.h"

namespace Ogre {  class SceneNode;  class Terrain;  class Camera;  class SceneManager;
	class ParticleSystem;  class Entity;  class RibbonTrail;  class ManualObject;  class AxisAlignedBox;  }
namespace MyGUI {  class TextBox;  }
class SETTINGS;  class GAME;  class CAR;
class Scene;  class App;  class FollowCamera;  class CarReflection;


//  CarModel is the "Ogre" part of the car.
//  It is used to put meshes together, particle emitters, etc.

class CarModel : public sh::MaterialInstanceListener
{
public:
	/// -------------------- Car Types ---------------------------
	//              Source          Physics (VDrift car)    Camera
	// CT_LOCAL:    Local player    yes	                    yes
	// CT_REMOTE:   Network	        yes	                    no
	// CT_REPLAY:   Replay file     no                      yes
	// CT_GHOST:	Ghost file		no						no
	// CT_GHOST2:	other car's ghost file
	// CT_TRACK:	track's ghost file

	enum eCarType {  CT_LOCAL=0, CT_REMOTE, CT_REPLAY,  CT_GHOST, CT_GHOST2, CT_TRACK };
	eCarType eType;
	bool isGhost()    const {  return eType >= CT_GHOST;  }
	bool isGhostTrk() const {  return eType == CT_TRACK;  }

	VehicleType vtype;

	int numWheels;
	void SetNumWheels(int n);
	
	//  ctor
	CarModel(int index, int colorId, eCarType type, const std::string& name,
		Ogre::SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* sc,
		Ogre::Camera* cam, App* app);
	~CarModel();
	
	Ogre::String sDispName;  // diplay name in opponents list (nick for CT_REMOTE)
	MyGUI::TextBox* pNickTxt;  // multiplayer nick above car
	bool updTimes, updLap;  float fLapAlpha;
	
	
	///----  model params  from .car
	float driver_view[3], hood_view[3], ground_view[3];  // mounted cameras
	float interiorOffset[3], boostOffset[3],boostSizeZ;
	float thrusterOfs[PAR_THRUST][3],thrusterSizeZ[PAR_THRUST];

	std::vector<Ogre::Vector3> brakePos;  // flares
	float brakeSize;  Ogre::ColourValue brakeClr;
	std::string sBoostParName, sThrusterPar[PAR_THRUST];
	bool bRotFix;

	std::vector<float> whRadius, whWidth;  // for tire trails
	std::vector<MATHVECTOR<float,3> > whPos;
	QUATERNION<float> qFixWh[2];
	float maxangle;  //steer

	//  exhaust position for boost particles
	bool manualExhaustPos;  // if true, use values below, if false, guess from bounding box
	bool has2exhausts;  // car has 2nd exhaust, if true, mirror exhaust 1 for position
	float exhaustPos[3];  // position of first exhaust
	
	void LoadConfig(const std::string & pathCar), Defaults();


	///--------  Create
	void Load(int startId, bool loop), Create(), CreateReflection(), Destroy();
	void CreatePart(Ogre::SceneNode* ndCar, Ogre::Vector3 vPofs,
		Ogre::String sCar2, Ogre::String sCarI, Ogre::String sMesh, Ogre::String sEnt,
		bool ghost, Ogre::uint32 visFlags,
		Ogre::AxisAlignedBox* bbox=0, Ogre::String stMtr="", bool bLogInfo=true);

	void LogMeshInfo(const Ogre::Entity* ent, const Ogre::String& name, int mul=1);
	int all_subs, all_tris;  //stats
	
	void RecreateMaterials();
	void setMtrNames(); // assign materials to entity / manualobject
	void setMtrName(const Ogre::String& entName, const Ogre::String& mtrName);
	
	
	//--------  Update
	void Update(PosInfo& posInfo, PosInfo& posInfoCam, float time);
	void UpdateKeys();  // for camera X,C, last chk F12


	//  reset camera after pos change etc	
	void First();
	int iFirst;
	
	//  color
	Ogre::ColourValue color;  // for minimap pos tri color  //float hue, sat, val;
	void ChangeClr();  //  Apply new color
		
	//  track surface for wheels
	void UpdWhTerMtr();
	Ogre::String txtDbgSurf;
	
	void UpdParsTrails(bool visible=true);
	
	
	///----  Camera, can be null
	FollowCamera* fCam;
	int iCamFluid;  // id to fluids[], -1 none
	float fCamFl;  // factor, close to surface
	float camDist;  // mul from .car

	//  Main node
	Ogre::SceneNode* pMainNode, *ndSph;
	Ogre::Vector3 posSph[2];
	Ogre::BillboardSet* brakes;
	
	void setVisible(bool visible);  // hide/show
	bool mbVisible;  float hideTime;
		
	CarReflection* pReflect;
		
	//  VDrift car
	CAR* pCar;  // all need this set (even ghost, has it from 1st car)
	
	
	///----  Logic vars
	float angCarY;  // car yaw angle for minimap
	float distFirst, distLast, distTotal;  // checks const distances set at start
	float trackPercent;  // % of track driven
	void UpdTrackPercent();

	///  Checkpoint vars,  start pos, lap
	bool bGetStPos;  Ogre::Matrix4 matStPos;  Ogre::Vector4 vStDist;
	int iInChk, iCurChk, iNextChk, iNumChks,  // cur checkpoint -1 at start
		iInWrChk, iWonPlace, iWonPlaceOld;  float iWonMsgTime;
	bool bInSt, bWrongChk;  float fChkTime;
	float timeAtCurChk;
	//bool Checkpoint(const PosInfo& posInfo, class SplineRoad* road);  // update
	Ogre::Vector3 vStartPos;  void ResetChecks(bool bDist=false), UpdNextCheck(), ShowNextChk(bool visible);
	Ogre::String sChkMtr;  bool bChkUpd;
	//  for loop camera change
	int iLoopChk, iLoopLastCam;
	
	
	///--------  common
	GAME* pGame;
	Ogre::Camera* mCamera;
	Scene* sc;
	Ogre::SceneManager* mSceneMgr;
	SETTINGS* pSet;
	App* pApp;
	
	int iIndex, iColor;  // car id, color id
	std::string sDirname;  // dir name of car (e.g. ES)
	Ogre::String resGrpId, mtrId;  // resource group name, material suffix
	std::string resCar;  // path to car textures


	//  Material names
	enum eMaterials {  Mtr_CarBody, Mtr_CarBrake,  NumMaterials  };
	std::string sMtr[NumMaterials];
			
	//--------  Particle systems
	enum EParTypes {  PAR_Smoke=0, PAR_Mud, PAR_Dust, PAR_Water, PAR_MudHard, PAR_MudSoft, PAR_ALL };
	//  par-wheels, boost-car rear, spaceship thruster, sparks-world hit
	Ogre::ParticleSystem* par[PAR_ALL][MAX_WHEELS];
	Ogre::ParticleSystem* parBoost[PAR_BOOST], *parThrust[PAR_THRUST*2], *parHit;
	std::vector<Ogre::RibbonTrail*> whTrail;  // tire trail
	std::vector<Ogre::Real> whTemp;  // spin time, approx tire temp.
	
	//  Wheels, Nodes
	std::vector<Ogre::SceneNode*> ndWh, ndWhE, ndBrake;
	Ogre::SceneNode* ndNextChk;
	Ogre::Entity* entNextChk;

	//  to destroy
	std::vector<Ogre::SceneNode*> vDelNd;		void ToDel(Ogre::SceneNode* nd);
	std::vector<Ogre::Entity*> vDelEnt;			void ToDel(Ogre::Entity* ent);
	std::vector<Ogre::ParticleSystem*> vDelPar;	void ToDel(Ogre::ParticleSystem* par);
	
		
	//  brake state
	bool bBraking;
	void UpdateBraking();
	
	//  lightmap toggle depending on distance to terrain
	Ogre::Terrain* terrain;
	bool bLightMapEnabled;
	void UpdateLightMap();
	
	//  cam,chk old states
	int iCamNextOld;
	bool bLastChkOld;

	virtual void requestedConfiguration (sh::MaterialInstance* m, const std::string& configuration);
	virtual void createdConfiguration (sh::MaterialInstance* m, const std::string& configuration);
};
