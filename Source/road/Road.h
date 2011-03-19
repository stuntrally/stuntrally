#ifndef _SplineRoad_h_
#define _SplineRoad_h_

#include "SplineBase.h"

#define  LODs  4
#define  MTRs  4

struct RoadSeg
{
	struct SegData {
		SceneNode* node;  Entity* ent;
		MeshPtr mesh;  String smesh;  };
	
	SegData road[LODs], wall[LODs], col;
	String sMtrRd,sMtrWall;  int mtrId;
	
	std::vector<Vector3> lpos;  //points for lod dist
	int nTri[LODs], mrgLod;

	bool empty;
	RoadSeg() : empty(true), mrgLod(0) {  }
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
	void Setup(String sMarkerMeshFile, Real scale,  // call this first at init
		Terrain* terrain, SceneManager* sceneMgr,  Camera* camera);
	bool LoadFile(String fname, bool build=true), SaveFile(String fname);
	
	//  Rebuild
	void RebuildRoad(bool full=false), RebuildRoadInt(),
		Destroy(), DestroyRoad(), DestroySeg(int id);

	//  Update
	void UpdLodVis(float fBias=1.f), SetForRnd(String sMtr),UnsetForRnd();
	void Pick(Camera* mCamera, Real mx, Real my, bool bAddH=false, bool bHide=false);
	void ToggleMerge();

	//  Manipulate  -------
	SplinePoint newP;  // new point for insert
	void Insert(eIns ins), Delete();

	///  change point
	void Move1(int id, Vector3 relPos), Move(Vector3 relPos);
	void RotateSel(Real relA), Scale1(int id, Real posMul);

	//  modify point
	void ToggleOnTerrain(), ToggleColums();  // on chosen point
	void AddWidth(Real relW), AddAngle(Real relA),AddAngleYaw(Real relA);
	void AddPipe(Real relP), ChgMtrId(int relId);
	void AddChkR(Real relR), AddBoxW(Real rel),AddBoxH(Real rel);
	const String& getMtrStr(int seg);  bool isPipe(int seg);
	
	//  point sel
	void PrevPoint(),NextPoint(), FirstPoint(),LastPoint(), ChoosePoint();
	void SelAddPoint(),SelClear(),SelAll(), CopyNewPoint();  int GetSelCnt();
	bool CopySel();  void Paste(bool reverse=false), DelSel();

	//  util
	Real GetSegLen(int seg);  Vector3 GetLenDir(int seg, Real l, Real la);
	static Vector3 GetRot(Real ayaw, Real ang);
	void SetTerHitVis(bool visible), UpdRot();
	

private:
	//  mesh create
	void CreateMesh(SubMesh* submesh, AxisAlignedBox& aabox,
		const std::vector<Vector3>& pos, const std::vector<Vector3>& norm, const std::vector<Vector4>& clr,
		const std::vector<Vector2>& tcs, const std::vector<Ogre::uint16>& idx, String sMtrName);
	void AddMesh(MeshPtr mesh, String sMesh, const AxisAlignedBox& aabox,
		Entity** pEnt, SceneNode** pNode, String sEnd);

	std::vector<Ogre::uint16> idx;	// mesh indices
#ifndef ROAD_EDITOR
	std::vector<Vector3> posBt;  // for bullet trimesh
#endif
	std::vector<Vector3>* at_pos;
	//  add triangle, with index check
	__forceinline void addTri(int f1, int f2, int f3, int i);
	int at_size, at_ilBt;  // pars for fi

	//  markers
	void AddMarker(Vector3 pos), SelectMarker(bool bHide=false),
		DelLastMarker(), UpdAllMarkers(), DestroyMarkers();

	String sMarkerMesh;  Real fMarkerScale, fScRot,fScHit;
	std::vector<SceneNode*> vMarkNodes;  // markers
	//  control markers
	SceneNode *ndSel,*ndChosen,*ndRot,*ndHit,*ndChk, *lastNdSel,*lastNdChosen;
	Entity* entSel,*entChs,*entRot,*entHit,*entChk;


//  vars  -----------
	//  setup
	SceneManager* mSceneMgr;
	Terrain* mTerrain;	// for height snap
	Camera* mCamera;

	friend class App;
	int iSelPoint, iChosen;  // -1 if none
	std::set<int> vSel;  // selection
	
	//  road data Segments
	std::deque<RoadSeg> vSegs;
	//  info
	int iMrgSegs, segsMrg,  iOldHide;
	bool rebuild;  int iVis, iTris, iDirtyId, idStr;


public:
	Vector3 posHit;  bool bHitTer, bSelChng;  float fLodBias;

	///  params, from xml
	String  sMtrRoad[MTRs], sMtrPipe[MTRs], sMtrWall,sMtrWallPipe, sMtrCol;

	Real fHeight;	// above terrain  ?for each point-
	Real tcMul;		// tex coord mul / unit length

	Real lenDiv0;	// triangle dim in length
	int  iw0;		// width divs

	Real skLen,skH; // skirt dims (for hiding gaps)

	Real setMrgLen; // length below which segments are merged
	bool bMerge;
	Real lposLen;   // length between LOD points

	int  colN;		// column regular polygon sides
	Real colR;		// column radius
	Real ilPmul,iwPmul;	 // length,width steps multipler for pipe

	struct stats  //  for info
	{	Real Length,WidthAvg,HeightDiff,
		OnTer,Pipes, Yaw,Pitch,Roll;
	} st;
	String  sTxtDesc;  // track description text
	
	std::vector<CheckSphere> mChks;  // checkpoint spheres
	Vector3 vStBoxDim;  // start/finish box half dimensions
	int iDir;  // -1 or +1  if road points go +/-1 with car start orientation
};

#endif