#pragma once
#include "BaseApp.h"
#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
#include "../vdrift/tracksurface.h"
#include "../vdrift/track.h"
#include "../ogre/common/data/SceneXml.h"  //Object-
#include "../ogre/common/PreviewTex.h"

#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreString.h>
#include <OgreRenderTargetListener.h>
#include <OgreShadowCameraSetup.h>
#include <OgreTexture.h>
#include "../shiny/Main/Factory.hpp"

#define BrushMaxSize  512

//  Gui
const int ciAngSnapsNum = 7;
const Ogre::Real crAngSnaps[ciAngSnapsNum] = {0,5,15,30,45,90,180};

namespace Ogre  {  class Rectangle2D;  class SceneNode;  class RenderTexture;  }
namespace sh {  class Factory;  }
class CScene;  class CGui;  class CGuiCom;

enum ED_OBJ {  EO_Move=0, EO_Rotate, EO_Scale  };


class App : public BaseApp,
			public sh::MaterialListener,
			public Ogre::RenderTargetListener
{
public:
	App(class SETTINGS* pSet1);
	virtual ~App();

	class Instanced* inst;

	CScene* scn;

	//  materials
	sh::Factory* mFactory;
	void postInit(), SetFactoryDefaults();
	virtual void materialCreated(sh::MaterialInstance* m, const std::string& configuration, unsigned short lodIndex);


	///  Gui
	CGui* gui;
	CGuiCom* gcom;

	PreviewTex prvView,prvRoad,prvTer;  // track tab

	float mTimer;

	
	///  main
	void LoadTrack(), SaveTrack(), UpdateTrack();
	
	void SetEdMode(ED_MODE newMode);
	void UpdVisGui(), UpdEditWnds();
	void UpdWndTitle(), SaveCam();


	bool keyPressed(const SDL_KeyboardEvent &arg);

	void LoadTrackEv(), SaveTrackEv(), UpdateTrackEv();
	enum TrkEvent {  TE_None=0, TE_Load, TE_Save, TE_Update  } eTrkEvent;

	virtual void createScene();
	virtual void destroyScene();

	virtual bool frameStarted(const Ogre::FrameEvent& evt);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
	virtual bool frameEnded(const Ogre::FrameEvent& evt);

	void processMouse(double dt);
	Ogre::Vector3 vNew;	void editMouse();
	

	//  create  . . . . . . . . . . . . . . . . . . . . . . . . 
	bool bNewHmap, bTrGrUpd;
	Ogre::String resTrk;  void NewCommon(bool onlyTerVeget);

	void CreateObjects(), DestroyObjects(bool clear), ResetObjects();
	void UpdObjPick(), PickObject(), ToggleObjSim();


	///  rnd to tex  minimap  * * * * * * * * *	
	Ogre::SceneNode *ndPos;
	Ogre::ManualObject* mpos;
	Ogre::ManualObject* Create2D(const Ogre::String& mat, Ogre::Real s, bool dyn=false);
	Ogre::Real asp, xm1,ym1,xm2,ym2;

	const static int RTs = 4, RTsAdd = 2;
	struct SRndTrg
	{
		Ogre::Camera* cam;  Ogre::RenderTexture* tex;
		Ogre::Rectangle2D* mini;	Ogre::SceneNode* ndMini;
		SRndTrg() : cam(0),tex(0),mini(0),ndMini(0) {  }
	};
	SRndTrg rt[RTs+RTsAdd];

	void Rnd2TexSetup(), UpdMiniVis();
	virtual void preRenderTargetUpdate(const Ogre::RenderTargetEvent &evt);
	virtual void postRenderTargetUpdate(const Ogre::RenderTargetEvent &evt);
	

	//  fluids
	int iFlCur;  bool bRecreateFluids;
	void UpdFluidBox(), UpdMtrWaterDepth();
	

	//  terrain cursor, circle mesh
	Ogre::ManualObject* moTerC;
	Ogre::SceneNode* ndTerC;
	void TerCircleInit(), TerCircleUpd();

	//  brush preview tex
	void createBrushPrv();
	void updateBrushPrv(bool first=false), updateTerPrv(bool first=false);

	bool bUpdTerPrv;
	Ogre::TexturePtr brushPrvTex, terPrvTex;
	const static int BrPrvSize = 128, TerPrvSize = 256;


	///<>  terrain edit, brush
	enum EBrShape
	{   BRS_Triangle=0, BRS_Sinus, BRS_Noise, BRS_Noise2, BRS_Ngon, BRS_ALL  }  mBrShape[ED_ALL];
	const static Ogre::String csBrShape[BRS_ALL];

	struct BrushSet  // brush preset ----
	{
		ED_MODE edMode;  int curBr;
		float Size,Intens,Pow,Fq,NOf;
		int Oct;  EBrShape shape;
		float Filter,HSet;
		signed char newLine;
		Ogre::String name;
	};

	const static int brSetsNum = 87;
	const static BrushSet brSets[brSetsNum];
	const static float brClr[4][3];

	void SetBrushPreset(int id);
	void updBrush();


	//  brush vars
	int curBr;
	bool bTerUpd,bTerUpdBlend;  char sBrushTest[512];
	float* pBrFmask, *mBrushData;
	bool brLockPos;

	//  params
	float terSetH, mBrFilt,mBrFiltOld;
	float mBrSize[ED_ALL],mBrIntens[ED_ALL], mBrPow[ED_ALL];
	float mBrFq[ED_ALL],mBrNOf[ED_ALL];  int mBrOct[ED_ALL];


	//  brush deform
	bool getEditRect(Ogre::Vector3& pos, Ogre::Rect& brushrect, Ogre::Rect& maprect, int size, int& cx, int& cy);

	//  terrain edit
	void deform(Ogre::Vector3 &pos, float dtime, float brMul);
	void height(Ogre::Vector3 &pos, float dtime, float brMul);

	void smooth(Ogre::Vector3 &pos, float dtime);
	void smoothTer(Ogre::Vector3 &pos, float avg, float dtime);
	void calcSmoothFactor(Ogre::Vector3 &pos, float& avg, int& sample_count);

	void filter(Ogre::Vector3 &pos, float dtime, float brMul);


	///  bullet world, simulate
	class btDiscreteDynamicsWorld* world;
	class btDefaultCollisionConfiguration* config;
	class btCollisionDispatcher* dispatcher;
	class bt32BitAxisSweep3* broadphase;
	class btSequentialImpulseConstraintSolver* solver;

	void BltWorldInit(), BltWorldDestroy(), BltClear(), BltUpdate(float dt);


	//  tools, road  -in base
	void SaveGrassDens(), SaveWaterDepth();
	void AlignTerToRoad();
	int iSnap;  Ogre::Real angSnap;


	//  car start
	void UpdStartPos();
	Ogre::SceneNode* ndCar,*ndStBox,*ndFluidBox,*ndObjBox;
	Ogre::Entity*  entCar,*entStBox,*entFluidBox,*entObjBox;
	void togPrvCam();


	//  [Objects]  ----
	ED_OBJ objEd;  // edit mode

	int iObjCur;  // picked id
	int iObjLast;  // last counter, just for naming

	//  new object's type
	int iObjTNew;  // id for vObjNames
	std::vector<std::string> vObjNames, vBuildings;
	void SetObjNewType(int tnew), UpdObjNewNode();

	void AddNewObj(bool getName=true);

	std::set<int> vObjSel;  // selected ids for sc.objects[]
	void UpdObjSel();  // upd selected glow
	Ogre::Vector3 GetObjPos0();  // sel center

	bool objSim;  // dynamic simulate on
	Object objNew;  //Object*..

	std::vector<Object> vObjCopy;  // copied objects



	//-  vdrift track
	TRACK* track;
	Ogre::StaticGeometry* mStaticGeom;

	class btCollisionObject* trackObject;
	class btTriangleIndexVertexArray* trackMesh;

	bool IsVdrTrack();
	bool LoadTrackVdr(const std::string & trackname);
	void CreateVdrTrack(std::string strack, class TRACK* pTrack),
		CreateVdrTrackBlt(), DestroyVdrTrackBlt();
	static Ogre::ManualObject* CreateModel(Ogre::SceneManager* sceneMgr, const Ogre::String& mat,
		class VERTEXARRAY* a, Ogre::Vector3 vPofs, bool flip, bool track=false, const Ogre::String& name="");

	//  surfaces
	std::vector <TRACKSURFACE> surfaces;  // all
	std::map <std::string, int> surf_map;  // name to surface id
	bool LoadAllSurfaces();
	
};
