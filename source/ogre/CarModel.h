/*
 * CarModel is the "Ogre" part of the car.
 * It is mainly used to put the different meshes together,
 * but also for e.g. particle emitters and color change. 
 */
 
#ifndef _CarModel_H_
#define _CarModel_H_
 
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreVector4.h>
#include <OgreMatrix4.h>
#include <OgreColourValue.h>

#include "../shiny/Main/MaterialInstance.hpp"


class SETTINGS;  class GAME;  class CAR;  class Scene;  class App;  class FollowCamera;  class CarReflection;

namespace Ogre {  class SceneNode;  class Terrain;  class Camera;  class SceneManager;
	class ParticleSystem;  class Entity;  class RibbonTrail;  class ManualObject;  class AxisAlignedBox;  }
namespace MyGUI {  class TextBox;  }


//  Stores all the needed information about car coming from vdrift
//  position,rotation of car and wheels
//  and all data needed to update particles emitting rates and sounds
//  todo? remove PosInfo use ReplayFrame?
struct PosInfo
{
	bool bNew;  //  new posinfo available for Update
	//  car
	Ogre::Vector3 pos, carY;
	//  wheel
	Ogre::Vector3 whPos[4];  Ogre::Quaternion rot, whRot[4];  float whR[4];
	float whVel[4], whSlide[4], whSqueal[4];
	int whTerMtr[4],whRoadMtr[4];

	float fboost,steer, percent;  bool braking;

	//  fluids
	float whH[4],whAngVel[4], speed, whSteerAng[4];  int whP[4];
	
	//  hit sparks
	float fHitTime, fParIntens,fParVel;//, fSndForce, fNormVel;
	Ogre::Vector3 vHitPos,vHitNorm;  // world hit data
	
	//  camera view
	Ogre::Vector3 camPos;  Ogre::Quaternion camRot;

	PosInfo() : bNew(false),  // not inited
		pos(0,-200,0), percent(0.f), braking(false)
	{}
};

class CarModel : public sh::MaterialInstanceListener
{
public:
	/// -------------------- Car Types ---------------------------
	//              Source          Physics (VDrift car)    Camera
	// CT_LOCAL:    Local player    yes	                    yes
	// CT_REPLAY:   Replay file     no                      yes
	// CT_GHOST:	Replay file		no						no
	// CT_REMOTE:   Network	        yes	                    no
	enum eCarType {  CT_LOCAL=0, CT_REPLAY, CT_GHOST, CT_REMOTE };
	eCarType eType;


	CarModel( unsigned int index, eCarType type, const std::string& name,
		Ogre::SceneManager* sceneMgr, SETTINGS* set, GAME* game, Scene* sc,
		Ogre::Camera* cam, App* app, int startpos_index = -1);
	
	~CarModel();
	
	Ogre::String sDispName;  // diplay name in opponents list (nick for CT_REMOTE)
	MyGUI::TextBox* pNickTxt;  // multiplayer nick above car
	
	
	//  Create car (also calls CreateReflection)
	void Create(int car);
	void CreateReflection();
	void CreatePart(Ogre::SceneNode* ndCar, Ogre::Vector3 vPofs,
		Ogre::String sCar2, Ogre::String sCarI, Ogre::String sMesh, Ogre::String sEnt,
		bool ghost, Ogre::uint32 visFlags,
		Ogre::AxisAlignedBox* bbox=0, Ogre::String stMtr="", class VERTEXARRAY* var=0, bool bLogInfo=true);
	void LogMeshInfo(const Ogre::Entity* ent, const Ogre::String& name);
	int all_subs, all_tris;  //stats
	
	void RecreateMaterials();
	void setMtrNames(); // assign materials to entity / manualobject
	void setMtrName(const Ogre::String& entName, const Ogre::String& mtrName);
	
	//  Call this every vdrift substep with new position info
	void Update(PosInfo& posInfo, PosInfo& posInfoCam, float time);
	void UpdateKeys();  // for camera X,C, last chk F12
	
	//  Car color, After these values are changed, ChangeClr() should be called
	Ogre::ColourValue color;  // for minimap pos tri color  //float hue, sat, val;
	void ChangeClr(int car);  //  Apply new color
		
	//  track surface for wheels
	void UpdWhTerMtr();
	Ogre::String txtDbgSurf;
	
	//  Update trails
	void UpdParsTrails(bool visible=true);
	
	//  Follow camera for this car.
	//  This can be null (for remote [network] cars)
	FollowCamera* fCam;
	
	//  Main node. later, we will add sub-nodes for body, interior, glass and wheels.
	Ogre::SceneNode* pMainNode, *ndSph;
	Ogre::Vector3 posSph[2];
	
	void setVisible(bool visible);  // hide/show
	bool mbVisible;  float hideTime;
		
	//  Handles our cube map.
	CarReflection* pReflect;
		
	Ogre::Terrain* terrain;
	
	//  VDrift car (can be null)
	CAR* pCar;
	
	float angCarY;  // car yaw angle for minimap
	float distFirst, distLast, distTotal;  // checks const distances set at start
	float trackPercent;  void UpdTrackPercent();  // % of track driven

	//  start pos, lap  checkpoint vars
	bool bGetStPos;  Ogre::Matrix4 matStPos;  Ogre::Vector4 vStDist;
	int iInChk, iCurChk, iNextChk, iNumChks,  // cur checkpoint -1 at start
		iWonPlace, iWonPlaceOld;  float iWonMsgTime;
	bool bInSt, bWrongChk;  float fChkTime;  int iChkWrong;
	//bool Checkpoint(const PosInfo& posInfo, class SplineRoad* road);  // update
	Ogre::Vector3 vStartPos;  void ResetChecks(bool bDist=false);
	
	//  access to vdrift stuff
	GAME* pGame;
	Ogre::Camera* mCamera;
private:
	
	//  Scene, needed to get particle settings
	Scene* sc;

	//  SceneManager to use
	Ogre::SceneManager* mSceneMgr;

	//  Material names, will be initialized in Create()
	enum eMaterials {  Mtr_CarBody, Mtr_CarBrake,  NumMaterials  };
	std::string sMtr[NumMaterials];
			
	//  Particle systems
	enum EParTypes {  PAR_Smoke=0, PAR_Mud, PAR_Dust, PAR_Water, PAR_MudHard, PAR_MudSoft, PAR_ALL };
	Ogre::ParticleSystem* par[PAR_ALL][4];
	Ogre::ParticleSystem* pb[2], *ph;  // boost-car rear, sparks-world hit
	Ogre::RibbonTrail* whTrl[4];  // tire trail
	Ogre::Real wht[4];  // spin time (approx tire temp.)
	
	//  Nodes
	Ogre::SceneNode *ndWh[4], *ndWhE[4], *ndBrake[4];
	
	//  Dir name of car (e.g. ES)
public:
	std::string sDirname;

	//  index for the car (e.g. when we have 2 cars, they have indices 0 and 1)
	//  needed for cloned materials & textures
	int iIndex;
private:
	
	//  Path to car textures, e.g. /usr/share/stuntrally/data/cars/CT/textures
	std::string resCar;
		
	//  brake state
	bool bBraking;
	void RefreshBrakingMaterial();
	
	//  lightmap enable/disable depending on dist. to terrain
	bool bLightMapEnabled;
	void UpdateLightMap();
	
	//  cam,chk old states
	int iCamNextOld;
	bool bLastChkOld;
	
	//  Our settings.
	SETTINGS* pSet;
	App* pApp;

public:
    virtual void requestedConfiguration (sh::MaterialInstance* m, const std::string& configuration);
    virtual void createdConfiguration (sh::MaterialInstance* m, const std::string& configuration);

};

#endif
