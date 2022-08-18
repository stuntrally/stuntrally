#pragma once
#include "SplineBase.h"

#include <vector>
#include <deque>
#include <set>

#include <Ogre.h>
#include <OgreMesh.h>
#include <OgreAxisAlignedBox.h>

namespace Ogre {  class SceneManager;  class SceneNode;  class Entity;  class Terrain;  class Camera;  }
class btTriangleMesh;

#if defined(_WIN32) && defined(SR_EDITOR)
	// win doesnt need bullet somehow
#else
	#include "btBulletCollisionCommon.h"
#endif

#ifdef SR_EDITOR
#define LogR(a)  //LogO(String("~ Road  ") + a)
#else
#define LogR(a)  //LogO(String("~ Road  ") + a)
#endif

#define  LODs  4  // LODs each segment has
#define  MTRs  4  // road materials to choose
#define  LoopTypes  9  // for pace notes


struct RoadSeg
{
	struct SegData
	{	Ogre::SceneNode* node =0;  Ogre::Entity* ent =0;
		Ogre::MeshPtr mesh;  Ogre::String smesh ="";
	};
	
	SegData road[LODs], wall[LODs], col, blend[LODs];
	Ogre::String sMtrRd,sMtrWall,sMtrB;
	int mtrId = 0;
	
	std::vector<Ogre::Vector3> lpos;  //points for lod dist
	int nTri[LODs], mrgLod = 0;
	bool empty = true;

	RoadSeg()
	{	for (int i=0; i<LODs; ++i)  nTri[i] = 0;  }
};


//  insert before first, after chosen, after last
enum eIns
{	INS_Begin, INS_Cur, INS_CurPre, INS_End  };

enum RoadType
{	RD_Road, RD_River, RD_Decor, RD_Trail, RD_ALL  };


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
	void Defaults();
	
	//  types
	RoadType type = RD_Road;
	bool IsRoad()    // all: road, _ter, pipe, bridge wall, column etc
	{	return type == RD_Road;  }
	bool IsRiver()   // just road,  no wall, no column, no _ter materials
	{	return type == RD_River;  }
	bool IsDecor()   // just wall,  for platforms and decorations
	{	return type == RD_Decor;  }
	
	bool IsTrail()   // just road (like river), but has colors per vertex
	{	return type == RD_Trail;  }
	int trailSegId;  // for current road trace only

	bool HasRoad()
	{	return !IsDecor();  }
	bool HasWall(bool onTer)
	{	return (IsRoad() && !onTer) || IsDecor();  }
	bool HasColumns()
	{	return IsRoad();  }


	//  File
	bool LoadFile(Ogre::String fname, bool build = true), SaveFile(Ogre::String fname);
	
	//  Rebuild
	bool RebuildRoadInt(bool editorAlign = false, bool edBulletFull = false);
	void RebuildRoadPace();  ///  Rebuild road only for pacenotes, after RebuildRoadInt
	void Destroy(), DestroyRoad(), DestroySeg(int id);


	//  Update
	void UpdLodVis(float fBias = 1.f, bool bFull = false);
	void UpdLodVisMarks(Ogre::Real distSq, bool vis = false), HideMarks();
	void SetForRnd(Ogre::String sMtr), UnsetForRnd();
	void SetVisTrail(bool vis);

	void Pick(Ogre::Camera* mCamera, Ogre::Real mx, Ogre::Real my,
			bool bRay = true, bool bAddH = false, bool bHide = false);
	void SelectMarker(bool bHide = false);
	void ToggleMerge();


	//  Insert  -------
	void Insert(eIns ins);
	void Delete(), DelSel();

	bool CopySel();
	void Paste(bool reverse = false);


	//  other
	const Ogre::String& getMtrStr(int seg);  const Ogre::String& getWallMtrStr(int seg);
	bool isPipe(int seg);
	

private:
///  ***  MESH  ****
//---------------------------------------------------------------------------------------

	void CreateMesh( Ogre::SubMesh* submesh, Ogre::AxisAlignedBox& aabox,
		const std::vector<Ogre::Vector3>& pos, const std::vector<Ogre::Vector3>& norm, const std::vector<Ogre::Vector4>& clr,
		const std::vector<Ogre::Vector2>& tcs, const std::vector<Ogre::uint16>& idx, Ogre::String sMtrName);

	void AddMesh( Ogre::MeshPtr mesh, Ogre::String sMesh, const Ogre::AxisAlignedBox& aabox,
		Ogre::Entity** pEnt, Ogre::SceneNode** pNode, Ogre::String sEnd);

	std::vector<Ogre::uint16>    idx, idxB;	  // mesh indices

	std::vector<Ogre::Vector3>   posBt;       // for bullet trimesh
	std::vector<btTriangleMesh*> vbtTriMesh;  // for delete

	const std::vector<Ogre::Vector3>*  at_pos =0;

	//  add triangle, with index check
	void addTri(int f1, int f2, int f3, int i);

	int at_size, at_ilBt;
	bool bltTri, blendTri;  // params for addTri
	
	
///  ***  Rebuild Geom DATA  ***
//---------------------------------------------------------------------------------------
	
	struct DataRoad  // global
	{
		int segs = 0;            // count
		int sMin = 0, sMax = 0;  // range
		
		bool editorAlign, bulletFull;  // ed,options
		
		DataRoad(bool edAlign, bool bltFull)
			:editorAlign(edAlign), bulletFull(bltFull)
		{	}
	};

	void PrepassRange(DataRoad& DR);
	void PrepassAngles(DataRoad& DR);

	struct DataLod0   // at Lod 0
	{
		std::vector<int>            v0_iL;  // length steps
		std::vector<Ogre::Real>     v0_tc;  // tex coords
		std::vector<Ogre::Vector3>  v0_N;   // normals
		std::vector<int>          v0_Loop;  // bool, inside loop
		void Clear()
		{	v0_iL.clear();  v0_tc.clear();  v0_N.clear();  v0_Loop.clear();  }
	}
	DL0;  // stays after build since N is used for SetChecks
	void SetChecks();  // Init  1st in file load, 2nd time for N


	struct DataLod   // for current Lod
	{
		//>  data at cur lod
		std::vector<int>  v_iL, v_iW;  // num Length and Width steps for each seg
		std::vector<int>  v_bMerge;    // bool 0 if seg merged, 1 if new
		
		std::vector<Ogre::Real>     v_tc, v_len;  // total length
		std::vector<Ogre::Vector3>  v_W;   // width dir
		
		std::vector<std::vector <int> >  v_iWL;  //  width steps per length point, for each seg
		std::vector<int>  v_iwEq;	   // 1 if equal width steps at whole length, in seg, 0 has transition

		Ogre::Real tcLen = 0.f;      // total tex coord length u
		Ogre::Real sumLenMrg = 0.f;  // total length to determine merging
		int mrgCnt = 0;              // stats, merges counter

		//  LOD vars
		int lod = 0, iLodDiv = 1;  //.
		Ogre::Real fLenDim = 1.f;
		bool isLod0 = true, isPace = false;
	};
	
	struct StatsLod   // stats for current Lod
	{	//#  stats
		Ogre::Real roadLen = 0.f, rdOnT = 0.f, rdPipe = 0.f, rdOnPipe = 0.f;
		Ogre::Real avgWidth = 0.f, stMaxH = FLT_MIN, stMinH = FLT_MAX;
		Ogre::Real bankAvg = 0.f, bankMax = 0.f;
		bool stats = false;
	};
	
	void PrepassLod(
		const DataRoad& DR,
		DataLod0& DL0, DataLod& DL, StatsLod& ST,
		int lod, bool editorAlign);


	struct DataLodMesh   // mesh data for lod  (from merged segs)
	{
		//>  W-wall  C-column  B-blend
		std::vector<Ogre::Vector4>  clr0/*empty*/, clr, clrB;
		std::vector<Ogre::Vector3>  pos,norm, posW,normW, posC,normC, posLod, posB,normB;
		std::vector<Ogre::Vector2>  tcs, tcsW, tcsC, tcsB;

		int iLmrg = 0, iLmrgW = 0, iLmrgC = 0, iLmrgB = 0;
		
		void Clear();
	};

public:  ///  pacenotes prepass data
	struct PaceM
	{
		Ogre::Vector3 pos, pos2;
		float aa = 0.f;
		int used = -1;  bool vis =1, notReal =0, onTer =1;
		int loop = 0;  bool jump =0,jumpR =0, onPipe =0,onPipeE =0;
	};
	std::vector<PaceM> vPace;
private:
	
	struct DataSeg  // for segment
	{
		int seg,seg1,seg0;
		int mtrId, wallId;
		bool onTer;
		bool pipe;
		bool hasBlend;
		int iwC;
		bool jfw0,jfw1,jfw2;  // jump front walls
	};
	

	//  Build Segment Geometry  ----
	void BuildSeg(
		const DataRoad& DR,
		const DataLod0& DL0, DataLod& DL, StatsLod& ST,
		DataLodMesh& DLM, DataSeg& DS, int segM, bool full);
		
		
	void createSeg_Meshes(
		const DataLod& DL,
		const DataLodMesh& DLM, DataSeg& DS, RoadSeg& rs);
	
	void createSeg_Collision(
		const DataLodMesh& DLM, const DataSeg& DS);
	
//---------------------------------------------------------------------------------------


//  vars
	friend class App;
	friend class CGui;
public:
	Ogre::Vector3 posHit;  bool bHitTer = false;
	
	int iOldHide = -1, idStr = 0;  // upd var

	bool bMerge;
	float fLodBias = 1.f;     // upd par, detail
	
	bool bCastShadow = 0;    // true for depth shadows
	bool bRoadWFullCol = 0;  // road wall full collision (all triangles, or just side)


	//  road data Segments
	std::deque<RoadSeg> vSegs;


///  params, from xml
	//  materials
	Ogre::String  sMtrPipe[MTRs];  // use SetMtrPipe to set
	bool bMtrPipeGlass[MTRs];  // glass in mtr name

	Ogre::String  sMtrRoad[MTRs], sMtrWall,sMtrWallPipe, sMtrCol;
	bool bMtrRoadTer[MTRs];  // if _ter material present in .mat
	void updMtrRoadTer();
	void SetMtrPipe(int i, Ogre::String sMtr);


	//  geometry  ----
	//  tex coord multipliers (scale) per unit length
			//  road, wall, pipe, pipewall, column
	Ogre::Real g_tcMul, g_tcMulW,  g_tcMulP, g_tcMulPW,  g_tcMulC;

	Ogre::Real g_LenDim0;	// triangle dim in length
	int  g_iWidthDiv0;		// width divisions (const for road, except pipes)

	//  skirt  for hiding gaps
	Ogre::Real g_SkirtLen, g_SkirtH;

	//  merge  for less batches
	Ogre::Real g_MergeLen;     // length below which segments are merged
	Ogre::Real g_LodPntLen;    // length between LOD points

	Ogre::Real g_VisDist;      // max visible distance
	Ogre::Real g_VisBehind;    // distance visible behind, meh

	int  g_ColNSides;          // column regular polygon sides
	Ogre::Real g_ColRadius;    // column radius
	Ogre::Real g_P_il_mul, g_P_iw_mul;   // length,width steps multipliers for pipe


	Ogre::String sTxtDescr;   // track description text, how it looks
	Ogre::String sTxtAdvice;  // track advice text, how to drive

	//  for editor tool: align terrain to road
	float ed_Wadd = 0.f, ed_Wmul = 1.f;  // const added width and width multipler for whole road


	//  stats  ----
	struct Stats  // for info only
	{
		int iMrgSegs = 0, segsMrg = 0;
		int iVis = 0, iTris = 0;  // in upd vis

		Ogre::Real Length = 0.f, WidthAvg = 0.f, HeightDiff = 0.f;
		Ogre::Real OnTer = 0.f, Pipes = 0.f, OnPipe = 0.f;
		Ogre::Real bankAvg = 0.f, bankMax = 0.f;  // banking angle
		
		void Reset();
	} st;

	void End0Stats(const DataLod& DL, const StatsLod& ST);
	void EndStats(const DataRoad& DR, const StatsLod& ST);
};
