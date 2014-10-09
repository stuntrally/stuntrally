#pragma once
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

#if defined(_WIN32) && defined(SR_EDITOR)
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
	
	SegData road[LODs], wall[LODs], col, blend[LODs];
	Ogre::String sMtrRd,sMtrWall,sMtrB;  int mtrId;
	
	std::vector<Ogre::Vector3> lpos;  //points for lod dist
	int nTri[LODs], mrgLod;

	bool empty;
	RoadSeg() : empty(true), mrgLod(0), mtrId(0) { 
		for (int i=0;i<LODs;++i) nTri[i] = 0;
	}
};

//  insert before first, after chosen, after last
enum eIns{  INS_Begin, INS_Cur, INS_CurPre, INS_End  };


class SplineRoad : public SplineEdit
{
public:
	#ifdef SR_EDITOR
		class App* pApp;  ///*
		SplineRoad(App* papp);
	#else
		class GAME* pGame;  ///*
		SplineRoad(GAME* pgame);
	#endif
	virtual ~SplineRoad();
	void Defaults();

	///  Main
	//   call this first at init
	void Setup(Ogre::String sMarkerMeshFile, Ogre::Real scale,
		Ogre::Terrain* terrain, Ogre::SceneManager* sceneMgr,  Ogre::Camera* camera);
	bool LoadFile(Ogre::String fname, bool build=true), SaveFile(Ogre::String fname);
	
	//  Rebuild
	void RebuildRoadInt(bool editorAlign=false, bool edBulletFull=false);
	void Destroy(), DestroyRoad(), DestroySeg(int id);

	//  Update
	void UpdLodVis(/*Camera* pCam,*/ float fBias=1.f, bool bFull=false), SetForRnd(Ogre::String sMtr),UnsetForRnd();
	void Pick(Ogre::Camera* mCamera, Ogre::Real mx, Ogre::Real my,  bool bRay=true, bool bAddH=false, bool bHide=false);
	void ToggleMerge(), SetChecks();


	//  Manipulate  -------
	void Insert(eIns ins), Delete();

	//  edit start
	void AddBoxW(Ogre::Real rel),AddBoxH(Ogre::Real rel), Set1stChk();
	const Ogre::String& getMtrStr(int seg);  bool isPipe(int seg);
	
	bool CopySel();
	void Paste(bool reverse=false), DelSel();

	//  util
	void SetTerHitVis(bool visible), UpdRot();
	
	bool bCastShadow;  // true for depth shadows
	bool bRoadWFullCol;  // road wall full collision (all triangles, or just side)
	

private:
	//  mesh create  -------
	void CreateMesh(Ogre::SubMesh* submesh, Ogre::AxisAlignedBox& aabox,
		const std::vector<Ogre::Vector3>& pos, const std::vector<Ogre::Vector3>& norm, const std::vector<Ogre::Vector4>& clr,
		const std::vector<Ogre::Vector2>& tcs, const std::vector<Ogre::uint16>& idx, Ogre::String sMtrName);
	void AddMesh(Ogre::MeshPtr mesh, Ogre::String sMesh, const Ogre::AxisAlignedBox& aabox,
		Ogre::Entity** pEnt, Ogre::SceneNode** pNode, Ogre::String sEnd);

	std::vector<Ogre::uint16> idx, idxB;	// mesh indices

	std::vector<Ogre::Vector3> posBt;  // for bullet trimesh
	std::vector<class btTriangleMesh*> vbtTriMesh;  // for delete

	std::vector<Ogre::Vector3>* at_pos;

	//  add triangle, with index check
	inline void addTri(int f1, int f2, int f3, int i);
	int at_size, at_ilBt;  bool bltTri,blendTri;  // pars for addTri


	//  control/edit markers  -------
	void AddMarker(Ogre::Vector3 pos), SelectMarker(bool bHide=false),
		DelLastMarker(), UpdAllMarkers(), DestroyMarkers();

	Ogre::String sMarkerMesh;  Ogre::Real fMarkerScale, fScRot,fScHit;

	Ogre::SceneNode *ndSel,*ndChosen,*ndRot,*ndHit,*ndChk, *lastNdSel,*lastNdChosen;
	Ogre::Entity* entSel,*entChs,*entRot,*entHit,*entChk;


//  vars  -----------
	//  setup
	Ogre::SceneManager* mSceneMgr;
public:
	Ogre::Camera* mCamera;
private:
	friend class App;
	friend class CGui;

	//  road data Segments
	std::deque<RoadSeg> vSegs;
	//  info
	int iMrgSegs, segsMrg,  iOldHide;
	int iVis, iTris, idStr;

	Ogre::String  sMtrPipe[MTRs];  // use SetMtrPipe to set
	bool bMtrPipeGlass[MTRs];  // glass in mtr name
public:
	Ogre::Vector3 posHit;  bool bHitTer;  float fLodBias;

	///  params, from xml
	Ogre::String  sMtrRoad[MTRs], sMtrWall,sMtrWallPipe, sMtrCol;
	void SetMtrPipe(int i, Ogre::String sMtr);

	Ogre::Real tcMul,tcMulW,tcMulP,tcMulPW,tcMulC;	// tex coord mul per unit length - road,wall,pipe,pipewall,column

	Ogre::Real lenDiv0;	 // triangle dim in length
	int  iw0;		// width divs

	Ogre::Real skirtLen,skirtH;//, skirtPipeMul;  // skirt dims (for hiding gaps)

	Ogre::Real setMrgLen;  // length below which segments are merged
	bool bMerge;
	Ogre::Real lposLen;    // length between LOD points

	int  colN;		// column regular polygon sides
	Ogre::Real colR;		// column radius
	Ogre::Real ilPmul,iwPmul;	 // length,width steps multiplier for pipe

	struct stats  //  for info
	{
		Ogre::Real Length, WidthAvg, HeightDiff;
		Ogre::Real OnTer, Pipes, OnPipe;
		Ogre::Real bankAvg, bankMax;  // banking angle
	} st;
	Ogre::String  sTxtDesc;  // track description text
	
	std::vector<CheckSphere> mChks;  // checkpoint spheres
	Ogre::Real chksRoadLen;   // for %, sum of all mChks[].dist (without last)
	Ogre::Vector3 vStBoxDim;  // start/finish box half dimensions
	int iDir;  // -1 or +1  if road points go +/-1 with car start orientation
	int iChkId1,iChkId1Rev;   // 1st chekpoint index (and for reversed) for mChks[]

	// params for editor tool: align terrain to road
	float edWadd,edWmul;  // const added width and width multipler for whole road
};
