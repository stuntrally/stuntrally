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

#ifdef SR_EDITOR
#define LogR(a)  //LogO(String("~ Road  ") + a);
#else
#define LogR(a)
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


class SplineRoad : public SplineMarkEd
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

	//  File
	bool LoadFile(Ogre::String fname, bool build=true), SaveFile(Ogre::String fname);
	
	//  Rebuild
	void RebuildRoadInt(bool editorAlign=false, bool edBulletFull=false);
	void Destroy(), DestroyRoad(), DestroySeg(int id);


	//  Update
	void UpdLodVis(/*Camera* pCam,*/ float fBias=1.f, bool bFull=false);
	void SetForRnd(Ogre::String sMtr),UnsetForRnd();

	void Pick(Ogre::Camera* mCamera, Ogre::Real mx, Ogre::Real my,
			bool bRay=true, bool bAddH=false, bool bHide=false);
	void ToggleMerge();


	//  Insert  -------
	void Insert(eIns ins);
	void Delete(), DelSel();

	bool CopySel();
	void Paste(bool reverse=false);


	//  other
	const Ogre::String& getMtrStr(int seg);
	bool isPipe(int seg);
	

private:
#define vec std::vector

	//  mesh create  -------
	void CreateMesh(Ogre::SubMesh* submesh, Ogre::AxisAlignedBox& aabox,
		const vec<Ogre::Vector3>& pos, const vec<Ogre::Vector3>& norm, const vec<Ogre::Vector4>& clr,
		const vec<Ogre::Vector2>& tcs, const vec<Ogre::uint16>& idx, Ogre::String sMtrName);
	void AddMesh(Ogre::MeshPtr mesh, Ogre::String sMesh, const Ogre::AxisAlignedBox& aabox,
		Ogre::Entity** pEnt, Ogre::SceneNode** pNode, Ogre::String sEnd);

	vec<Ogre::uint16> idx, idxB;	// mesh indices

	vec<Ogre::Vector3> posBt;  // for bullet trimesh
	vec<class btTriangleMesh*> vbtTriMesh;  // for delete

	vec<Ogre::Vector3>* at_pos;

	//  add triangle, with index check
	void addTri(int f1, int f2, int f3, int i);
	int at_size, at_ilBt;  bool bltTri,blendTri;  // pars for addTri
	
	
///  ***  Rebuild Geom DATA  ***
//---------------------------------------------------------------------------------------
	
	struct DataRoad  // global
	{
		int segs;       // count
		int sMin,sMax;  // range
		
		bool editorAlign, bulletFull;  // ed,options
		
		DataRoad(bool edAlign, bool bltFull)
			:editorAlign(edAlign), bulletFull(bltFull)
			,segs(2),sMin(0),sMax(1)  //-
		{	}
	};

	void PrepassRange(DataRoad& DR);
	void PrepassAngles(DataRoad& DR);

	struct DataLod0  // at Lod 0
	{
		vec<int> viLSteps0;
		vec<Ogre::Real> vSegTc0;  // tex coords
		vec<Ogre::Vector3> vnSeg0;  // normals
	};

	struct DataLod  // for current Lod
	{
		//>  data at cur lod
		vec<int> viL, viW;  // num steps for seg Length, Width
		vec<int> vbSegMrg;  // bool 0 if seg merged, 1 if new
		vec<Ogre::Real> vSegTc, vSegLen;
		vec<Ogre::Vector3> vwSeg;
		vec<vec <int> > viwLS;  //  width steps per length point, for each seg
		vec<int> viwEq;			// 1 if equal width steps at whole length, in seg

		Ogre::Real tcLen;      // total tex coord length u
		Ogre::Real sumLenMrg;  // total length to determine merging
		int mrgCnt;            // stats, merges counter

		int lod, iLodDiv;  //.
		Ogre::Real fLenDim;
		bool isLod0;
		
		DataLod()
			:tcLen(0.f), sumLenMrg(0.f), mrgCnt(0)
			,lod(0), iLodDiv(1), fLenDim(1.f), isLod0(true)  //-
		{	}
	};
	
	struct StatsLod  // stats for cuurent Lod
	{	//#  stats
		Ogre::Real roadLen, rdOnT, rdPipe, rdOnPipe;
		Ogre::Real avgWidth, stMaxH, stMinH;
		Ogre::Real bankAvg, bankMax;
		bool stats;

		StatsLod()
			:roadLen(0.f), rdOnT(0.f), rdPipe(0.f), rdOnPipe(0.f)
			,avgWidth(0.f), stMaxH(FLT_MIN), stMinH(FLT_MAX)
			,bankAvg(0.f), bankMax(0.f)
			,stats(0)
		{	}
	};
	
	void PrepassLod(
		const DataRoad& DR,
		DataLod0& DL0, DataLod& DL, StatsLod& ST,
		int lod, bool editorAlign);

	struct DataLodMesh  // mesh data for lod  (from merged segs)
	{	//>  W-wall  C-column  B-blend
		vec<Ogre::Vector4> clr0/*empty*/, clr, clrB;
		vec<Ogre::Vector3> pos,norm, posW,normW, posC,normC, posLod, posB,normB;
		vec<Ogre::Vector2> tcs, tcsW, tcsC, tcsB;
		int iLmrg, iLmrgW, iLmrgC, iLmrgB;
		
		DataLodMesh()
			:iLmrg(0), iLmrgW(0), iLmrgC(0), iLmrgB(0)
		{	}
		void Clear();
	};
	
	
	void BuildSeg(	// Build Segment Geometry
		const DataRoad& DR,
		DataLod0& DL0, DataLod& DL, StatsLod& ST,
		DataLodMesh& DLM,
		int segM);
	
//---------------------------------------------------------------------------------------
	
#undef vec


//  vars  -----------
	friend class App;
	friend class CGui;
public:
	Ogre::Vector3 posHit;  bool bHitTer;

	float fLodBias;
	bool bCastShadow;    // true for depth shadows
	bool bRoadWFullCol;  // road wall full collision (all triangles, or just side)
	

	//  road data Segments
	std::deque<RoadSeg> vSegs;

	//  info
	int iMrgSegs, segsMrg,  iOldHide;
	int iVis, iTris, idStr;

///  params, from xml
	//  materials
	Ogre::String  sMtrPipe[MTRs];  // use SetMtrPipe to set
	bool bMtrPipeGlass[MTRs];  // glass in mtr name

	Ogre::String  sMtrRoad[MTRs], sMtrWall,sMtrWallPipe, sMtrCol;
	void SetMtrPipe(int i, Ogre::String sMtr);


	//  geometry  ----
	//  tex coord multipliers (scale) per unit length
			//  road, wall, pipe, pipewall, column
	Ogre::Real tcMul, tcMulW, tcMulP, tcMulPW, tcMulC;

	Ogre::Real fLenDim0;	// triangle dim in length
	int  iWidthDiv0;		// width divisions (const for road, except pipes)

	//  skirt  for hiding gaps
	Ogre::Real skirtLen, skirtH;

	//  merge  for less batches
	Ogre::Real setMrgLen;   // length below which segments are merged
	bool bMerge;
	Ogre::Real lposLen;     // length between LOD points

	int  colN;                   // column regular polygon sides
	Ogre::Real colR;             // column radius
	Ogre::Real ilPmul, iwPmul;   // length,width steps multipliers for pipe


	//  stats  ----
	struct Stats  // for info only
	{
		Ogre::Real Length, WidthAvg, HeightDiff;
		Ogre::Real OnTer, Pipes, OnPipe;
		Ogre::Real bankAvg, bankMax;  // banking angle
	} st;
	void EndStats(const DataRoad& DR, const StatsLod& ST);

	Ogre::String  sTxtDesc;  // track description text


	// params for editor tool: align terrain to road
	float edWadd, edWmul;  // const added width and width multipler for whole road
};
