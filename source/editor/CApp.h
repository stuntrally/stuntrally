#pragma once
#include "BaseApp.h"
#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
#include "../vdrift/tracksurface.h"
#include "../vdrift/track.h"

#include <OgreCommon.h>
#include <OgreVector3.h>
#include <OgreString.h>

#include <OgreRenderTargetListener.h>
#include <OgreShadowCameraSetup.h>
#include <OgreTexture.h>

#include "../ogre/common/data/SceneXml.h"  //Object-
#include "../ogre/common/PreviewTex.h"

#define BrushMaxSize  512

//  Gui
const int ciAngSnapsNum = 7;
const Ogre::Real crAngSnaps[ciAngSnapsNum] = {0,5,15,30,45,90,180};

namespace Forests {  class PagedGeometry;  }
namespace Ogre  {  class Terrain;  class TerrainGlobalOptions;  class TerrainGroup;  class TerrainPaging;  class PageManager;
	class Light;  class Rectangle2D;  class SceneNode;  class RenderTexture;  }
namespace sh {  class Factory;  }
class Scene;  class WaterRTT;  class CData;  class CGui;  class CGuiCom;

enum ED_OBJ {  EO_Move=0, EO_Rotate, EO_Scale  };


class App : public BaseApp, public Ogre::RenderTargetListener
{
public:
	App(class SETTINGS* pSet1);
	virtual ~App();


	Scene* sc;  /// scene.xml

	CData* data;  // xmls

	//  materials
	sh::Factory* mFactory;
	//  to be executed after BaseApp init
	void postInit(), SetFactoryDefaults();

	WaterRTT* mWaterRTT;

	///  Gui
	CGui* gui;
	CGuiCom* gcom;

	PreviewTex prvView,prvRoad,prvTer;  // track tab


	// TODO:  CScene* scn;  //...
	Ogre::Light* sun;
	void UpdFog(bool bForce=false), UpdSun();

	//  Weather  rain, snow
	Ogre::ParticleSystem *pr,*pr2;
	void CreateWeather(),DestroyWeather();
	float mTimer;

	//  trees
	Forests::PagedGeometry *trees, *grass;

	
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
	Ogre::String resTrk;  void NewCommon(bool onlyTerVeget), UpdTrees();

	void CreateTerrain(bool bNewHmap=false, bool bTer=true), CreateBltTerrain(),
		GetTerAngles(int xb=0,int yb=0,int xe=0,int ye=0, bool full=true);
	void CreateTrees();

	void CreateObjects(), DestroyObjects(bool clear);
	void UpdObjPick(), PickObject(), ToggleObjSim();

	void CreateFluids(), DestroyFluids(), CreateBltFluids();
	void UpdFluidBox(), UpdateWaterRTT(Ogre::Camera* cam), UpdMtrWaterDepth();

	void CreateSkyDome(Ogre::String sMater, Ogre::Vector3 scale);


	///  rnd to tex  minimap  * * * * * * * * *	
	Ogre::SceneNode *ndPos;
	Ogre::ManualObject* mpos;
	Ogre::ManualObject* Create2D(const Ogre::String& mat, Ogre::Real s, bool dyn=false);
	Ogre::Real asp, xm1,ym1,xm2,ym2;

	const static int RTs = 4, RTsAdd = 2;
	struct SRndTrg
	{
		Ogre::Camera* rndCam;  Ogre::RenderTexture* rndTex;
		Ogre::Rectangle2D* rcMini;	Ogre::SceneNode* ndMini;
		SRndTrg() : rndCam(0),rndTex(0),rcMini(0),ndMini(0) {  }
	};
	SRndTrg rt[RTs+RTsAdd];

	void Rnd2TexSetup(), UpdMiniVis();
	virtual void preRenderTargetUpdate(const Ogre::RenderTargetEvent &evt);
	virtual void postRenderTargetUpdate(const Ogre::RenderTargetEvent &evt);
	

	//  fluids to destroy
	std::vector<Ogre::String/*MeshPtr*/> vFlSMesh;
	std::vector<Ogre::Entity*> vFlEnt;
	std::vector<Ogre::SceneNode*> vFlNd;
	int iFlCur;  bool bRecreateFluids;

	
	///  terrain
	Ogre::Terrain* terrain;
	Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;  bool mPaging;
	Ogre::TerrainPaging* mTerrainPaging;
	Ogre::PageManager* mPageManager;
	void configureTerrainDefaults(Ogre::Light* l), UpdTerErr();

	int iBlendMaps, blendMapSize;	//  mtr from ter  . . . 
	void initBlendMaps(Ogre::Terrain* terrin, int xb=0,int yb=0, int xe=0,int ye=0, bool full=true);

	float Noise(float x, float zoom, int octaves, float persistence);
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
		
	//  shadows
	void changeShadows(), UpdPSSMMaterials();
	Ogre::Vector4 splitPoints;
	Ogre::ShadowCameraSetupPtr mPSSMSetup;


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
		char newLine;  Ogre::String name;
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

	//  params
	float terSetH, mBrFilt,mBrFiltOld;
	float mBrSize[ED_ALL],mBrIntens[ED_ALL], mBrPow[ED_ALL];
	float mBrFq[ED_ALL],mBrNOf[ED_ALL];  int mBrOct[ED_ALL];


	//  brush deform
	bool getEditRect(Ogre::Vector3& pos, Ogre::Rect& brushrect, Ogre::Rect& maprect, int size, int& cx, int& cy);

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
	bool LoadStartPos(std::string path, bool tool=false), SaveStartPos(std::string path);  void UpdStartPos();
	std::vector <MATHVECTOR <float, 3> > vStartPos;
	std::vector <QUATERNION <float> >    vStartRot;
	Ogre::SceneNode* ndCar,*ndStBox,*ndFluidBox,*ndObjBox;
	Ogre::Entity*  entCar,*entStBox,*entFluidBox,*entObjBox;
	void togPrvCam();


	//  [Objects]  ----
	ED_OBJ objEd;  // edit mode

	int iObjCur;  // picked id
	int iObjLast;  // last counter, just for naming

	//  new object's type
	int iObjTNew;  // id for vObjNames
	std::vector<std::string> vObjNames;
	void SetObjNewType(int tnew), UpdObjNewNode();

	void AddNewObj(bool getName=true);

	std::set<int> vObjSel;  // selected ids for sc.objects[]
	void UpdObjSel();  // upd selected glow

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
