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
class btTriangleMesh;

#if defined(_WIN32) && defined(SR_EDITOR)
	// win doesnt need bullet somehow
#else
	#include "btBulletCollisionCommon.h"
#endif

#ifdef SR_EDITOR
#define LogR(a)  //LogO(String("~ Road  ") + a)
#else
#define LogR(a)
#endif

#define  LODs  4
#define  MTRs  4
#define  LoopTypes  8  // for pace notes


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
	bool RebuildRoadInt(bool editorAlign=false, bool edBulletFull=false);
	void RebuildRoadPace();  ///  Rebuild road only for pacenotes, after RebuildRoadInt
	void Destroy(), DestroyRoad(), DestroySeg(int id);


	//  Update
	void UpdLodVis(/*Camera* pCam,*/ float fBias=1.f, bool bFull=false);
	void SetForRnd(Ogre::String sMtr),UnsetForRnd();

	void Pick(Ogre::Camera* mCamera, Ogre::Real mx, Ogre::Real my,
			bool bRay=true, bool bAddH=false, bool bHide=false);
	void SelectMarker(bool bHide=false);
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

	const std::vector<Ogre::Vector3>*  at_pos;

	//  add triangle, with index check
	void addTri(int f1, int f2, int f3, int i);

	int at_size, at_ilBt;
	bool bltTri, blendTri;  // pars for addTri
	
	
///  ***  Rebuild Geom DATA  ***
//---------------------------------------------------------------------------------------
	
	struct DataRoad  // global
	{
		int segs;        // count
		int sMin, sMax;  // range
		
		bool editorAlign, bulletFull;  // ed,options
		
		DataRoad(bool edAlign, bool bltFull)
			:editorAlign(edAlign), bulletFull(bltFull)
			,segs(0), sMin(0), sMax(0)
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

		Ogre::Real tcLen;      // total tex coord length u
		Ogre::Real sumLenMrg;  // total length to determine merging
		int mrgCnt;            // stats, merges counter

		//  LOD vars
		int lod, iLodDiv;  //.
		Ogre::Real fLenDim;
		bool isLod0, isPace;
		
		DataLod()
			:tcLen(0.f), sumLenMrg(0.f), mrgCnt(0)
			,lod(0), iLodDiv(1), fLenDim(1.f),
			isLod0(true), isPace(false)
		{	}
	};
	
	struct StatsLod   // stats for cuurent Lod
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

	struct DataLodMesh   // mesh data for lod  (from merged segs)
	{
		//>  W-wall  C-column  B-blend
		std::vector<Ogre::Vector4>  clr0/*empty*/, clr, clrB;
		std::vector<Ogre::Vector3>  pos,norm, posW,normW, posC,normC, posLod, posB,normB;
		std::vector<Ogre::Vector2>  tcs, tcsW, tcsC, tcsB;

		int iLmrg, iLmrgW, iLmrgC, iLmrgB;
		
		DataLodMesh()
			:iLmrg(0), iLmrgW(0), iLmrgC(0), iLmrgB(0)
		{	}
		void Clear();
	};

public:  ///  pacenotes prepass data
	struct PaceM
	{
		Ogre::Vector3 pos, pos2;
		float aa;
		int used;  bool vis, notReal, onTer;
		int loop;  bool jump,jumpR, onPipe,onPipeE;
		PaceM()
			:used(-1), aa(0.f)
			,vis(1), notReal(0), onTer(1)
			,loop(0), jump(0),jumpR(0), onPipe(0),onPipeE(0)
		{	}
	};
	std::vector<PaceM> vPace;
private:
	
	struct DataSeg  // for segment
	{
		int seg,seg1,seg0;
		int mtrId;
		bool onTer;
		bool pipe;
		bool hasBlend;
		int iwC;
		bool jfw0,jfw1,jfw2;  // jump front walls
	};
	
	//  Build Segment Geometry
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
	Ogre::Vector3 posHit;  bool bHitTer;
	
	int iOldHide, idStr;  // upd var

	bool bMerge;
	float fLodBias;      // upd par, detail
	
	bool bCastShadow;    // true for depth shadows
	bool bRoadWFullCol;  // road wall full collision (all triangles, or just side)


	//  road data Segments
	std::deque<RoadSeg> vSegs;


///  params, from xml
	//  materials
	Ogre::String  sMtrPipe[MTRs];  // use SetMtrPipe to set
	bool bMtrPipeGlass[MTRs];  // glass in mtr name

	Ogre::String  sMtrRoad[MTRs], sMtrWall,sMtrWallPipe, sMtrCol;
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

	int  g_ColNSides;          // column regular polygon sides
	Ogre::Real g_ColRadius;    // column radius
	Ogre::Real g_P_il_mul, g_P_iw_mul;   // length,width steps multipliers for pipe


	Ogre::String  sTxtDesc;  // track description text

	//  for editor tool: align terrain to road
	float ed_Wadd, ed_Wmul;  // const added width and width multipler for whole road


	//  stats  ----
	struct Stats  // for info only
	{
		int iMrgSegs, segsMrg;
		int iVis, iTris;  // in upd vis

		Ogre::Real Length, WidthAvg, HeightDiff;
		Ogre::Real OnTer, Pipes, OnPipe;
		Ogre::Real bankAvg, bankMax;  // banking angle
		
		Stats();
		void Reset();
	} st;

	void End0Stats(const DataLod& DL, const StatsLod& ST);
	void EndStats(const DataRoad& DR, const StatsLod& ST);
};
