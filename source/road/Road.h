#ifndef _SplineRoad_h_
#define _SplineRoad_h_

#include "SplineBase.h"

#include <vector>
#include <deque>
#include <set>

#include <OgreString.h>
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreVector4.h>

#include <OgreMesh.h>
#include <OgreAxisAlignedBox.h>

namespace Ogre {  class SceneManager;  class SceneNode;  class Entity;  class Terrain;  class Camera;  }

#if defined(_WIN32) && defined(ROAD_EDITOR)
	// win doesnt need bullet somehow
#else
	#include "btBulletCollisionCommon.h"
#endif

#define  LODs  4
#define  MTRs  4


struct RoadSeg
{
	struct SegData {
		Ogre::SceneNode* node;  Ogre::Entity* ent;
		Ogre::MeshPtr mesh;  Ogre::String smesh;
		SegData() : node(0), ent(0), smesh("") {}
	};
	
	SegData road[LODs], wall[LODs], col;
	Ogre::String sMtrRd,sMtrWall;  int mtrId;
	
	std::vector<Ogre::Vector3> lpos;  //points for lod dist
	int nTri[LODs], mrgLod;

	bool empty;
	RoadSeg() : empty(true), mrgLod(0), mtrId(0) { 
		for (int i=0;i<LODs;++i) nTri[i] = 0;
	}
};

//  insert before first, after chosen, after last
enum eIns{  INS_Begin, INS_Cur, INS_CurPre, INS_End  };


class SplineRoad : public SplineBase
{
public:
	#ifdef ROAD_EDITOR
		SplineRoad();
	#else
		class GAME* pGame;  ///*
		SplineRoad(GAME* pgame);
	#endif
	virtual ~SplineRoad();  void Defaults();

	///  Main
	void Setup(Ogre::String sMarkerMeshFile, Ogre::Real scale,  // call this first at init
		Ogre::Terrain* terrain, Ogre::SceneManager* sceneMgr,  Ogre::Camera* camera);
	bool LoadFile(Ogre::String fname, bool build=true), SaveFile(Ogre::String fname);
	
	//  Rebuild
	void RebuildRoad(bool full=false), RebuildRoadInt(),
		Destroy(), DestroyRoad(), DestroySeg(int id);

	//  Update
	void UpdLodVis(/*Camera* pCam,*/ float fBias=1.f, bool bFull=false), SetForRnd(Ogre::String sMtr),UnsetForRnd();
	void Pick(Ogre::Camera* mCamera, Ogre::Real mx, Ogre::Real my, bool bAddH=false, bool bHide=false);
	void ToggleMerge(), SetChecks();

	//  Manipulate  -------
	SplinePoint newP;  // new point for insert
	void Insert(eIns ins), Delete();

	///  change point
	void Move1(int id, Ogre::Vector3 relPos), Move(Ogre::Vector3 relPos);  Ogre::Vector3 getPos0();
	void RotateSel(Ogre::Real relA, Ogre::Vector3 axis, int addYawRoll), Scale1(int id, Ogre::Real posMul), ScaleSel(Ogre::Real posMul);

	//  modify point
	void ToggleOnTerrain(), ToggleColums();  // on chosen point
	void AddWidth(Ogre::Real relW), AddYaw(Ogre::Real relA,Ogre::Real snapA, bool alt),AddRoll(Ogre::Real relA,Ogre::Real snapA, bool alt);
	void AddPipe(Ogre::Real relP), ChgMtrId(int relId), ChgAngType(int relId), AngZero();
	void AddChkR(Ogre::Real relR), AddBoxW(Ogre::Real rel),AddBoxH(Ogre::Real rel), Set1stChk();
	const Ogre::String& getMtrStr(int seg);  bool isPipe(int seg);
	
	//  point sel
	void PrevPoint(),NextPoint(), FirstPoint(),LastPoint(), ChoosePoint();
	void SelAddPoint(),SelClear(),SelAll(), CopyNewPoint();  int GetSelCnt();
	bool CopySel();  void Paste(bool reverse=false), DelSel();

	//  util
	Ogre::Real GetSegLen(int seg);  Ogre::Vector3 GetLenDir(int seg, Ogre::Real l, Ogre::Real la);
	static Ogre::Vector3 GetRot(Ogre::Real ayaw, Ogre::Real ang);
	void SetTerHitVis(bool visible), UpdRot();
	
	bool bCastShadow;  // true for depth shadows
	bool bRoadWFullCol;  // road wall full collision (all triangles, or just side)
	

private:
	//  mesh create
	void CreateMesh(Ogre::SubMesh* submesh, Ogre::AxisAlignedBox& aabox,
		const std::vector<Ogre::Vector3>& pos, const std::vector<Ogre::Vector3>& norm, const std::vector<Ogre::Vector4>& clr,
		const std::vector<Ogre::Vector2>& tcs, const std::vector<Ogre::uint16>& idx, Ogre::String sMtrName);
	void AddMesh(Ogre::MeshPtr mesh, Ogre::String sMesh, const Ogre::AxisAlignedBox& aabox,
		Ogre::Entity** pEnt, Ogre::SceneNode** pNode, Ogre::String sEnd);

	std::vector<Ogre::uint16> idx;	// mesh indices
#ifndef ROAD_EDITOR
	std::vector<Ogre::Vector3> posBt;  // for bullet trimesh
	std::vector<btTriangleMesh*> vbtTriMesh;  // for delete
#endif
	std::vector<Ogre::Vector3>* at_pos;
	//  add triangle, with index check
	inline void addTri(int f1, int f2, int f3, int i);
	int at_size, at_ilBt;  // pars for fi

	//  markers
	void AddMarker(Ogre::Vector3 pos), SelectMarker(bool bHide=false),
		DelLastMarker(), UpdAllMarkers(), DestroyMarkers();

	Ogre::String sMarkerMesh;  Ogre::Real fMarkerScale, fScRot,fScHit;
	std::vector<Ogre::SceneNode*> vMarkNodes;  // markers
	//  control markers
	Ogre::SceneNode *ndSel,*ndChosen,*ndRot,*ndHit,*ndChk, *lastNdSel,*lastNdChosen;
	Ogre::Entity* entSel,*entChs,*entRot,*entHit,*entChk;

	static std::deque<SplinePoint> mPc;  // copy points

//  vars  -----------
	//  setup
	Ogre::SceneManager* mSceneMgr;
	Ogre::Terrain* mTerrain;	// for height snap
public:
	Ogre::Camera* mCamera;
private:

	friend class App;
	int iSelPoint, iChosen;  // -1 if none
	std::set<int> vSel;  // selection
	
	//  road data Segments
	std::deque<RoadSeg> vSegs;
	//  info
	int iMrgSegs, segsMrg,  iOldHide;
	bool rebuild;  int iVis, iTris, iDirtyId, idStr;


public:
	Ogre::Vector3 posHit;  bool bHitTer, bSelChng;  float fLodBias;

	///  params, from xml
	Ogre::String  sMtrRoad[MTRs], sMtrPipe[MTRs], sMtrWall,sMtrWallPipe, sMtrCol;

	Ogre::Real fHeight;	// above terrain  ?for each point-
	Ogre::Real tcMul;		// tex coord mul / unit length

	Ogre::Real lenDiv0;	// triangle dim in length
	int  iw0;		// width divs

	Ogre::Real skLen,skH; // skirt dims (for hiding gaps)

	Ogre::Real setMrgLen; // length below which segments are merged
	bool bMerge;
	Ogre::Real lposLen;   // length between LOD points

	int  colN;		// column regular polygon sides
	Ogre::Real colR;		// column radius
	Ogre::Real ilPmul,iwPmul;	 // length,width steps multiplier for pipe

	struct stats  //  for info
	{	Ogre::Real Length,WidthAvg,HeightDiff,
		OnTer,Pipes, Yaw,Pitch,Roll;
	} st;
	Ogre::String  sTxtDesc;  // track description text
	
	std::vector<CheckSphere> mChks;  // checkpoint spheres
	Ogre::Real chksRoadLen;  // for %, sum of all mChks[].dist (without last)
	Ogre::Vector3 vStBoxDim;  // start/finish box half dimensions
	int iDir;  // -1 or +1  if road points go +/-1 with car start orientation
	int iP1;  // 1st chk - road point index (not reversed) for mP[]
	int iChkId1,iChkId1Rev;  // 1st chekpoint index (and for reversed) for mChks[]

	int iTexSize;  //setting textures size for mtr name _s, call rebuild after change
};

#endif
