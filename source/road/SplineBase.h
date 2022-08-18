#pragma once
#include <deque>
#include <Ogre.h>

namespace Ogre {  class Terrain;  class SceneNode;  }


class TerUtil  //  helper
{
public:
	static float GetAngle(float x, float y);  // atan(y/x)
	//  terrain
	static float         GetAngleAt(Ogre::Terrain* terrain, float x, float z, float s);
	static Ogre::Vector3 GetNormalAt(Ogre::Terrain* terrain, float x, float z, float s);
};


enum AngType {  AT_Manual=0, AT_Auto, AT_Both, AT_ALL  };
const static std::string csAngType[AT_ALL] = {"Manual", "Auto", "Both"};



//  point,  variables
//------------------------------------------------------
class SplinePoint
{
public:
	Ogre::Vector3 pos, tan;  // position, tangent (computed)
	Ogre::Real width, wtan;  // road width

	Ogre::Real mYaw,mRoll;   // manual angles, if not auto
	AngType aType;
	Ogre::Real aYaw,aRoll;   // working angles (from auto)
	Ogre::Real aY,aR;        // after prepass+
	
	Ogre::Vector4 clr;  // trail color
	int nCk;            // trail, checkpoint id

	//  on/off
	bool onTer;   // sticked on terrain
	int cols;     // has column

	int onPipe;   // driven on pipe  0 off, 1 mark for stats only, 2 inverse normal too
	int loop;	// loop type: 0 none, 1 straight, 2 side, 3 barrel, 4 double  max LoopTypes-1
				// if > 0, chk is start or end of loop (for auto camera change)

	//  next
	Ogre::Real pipe;    // pipe amount 0..1
	int idMtr;    // material id road/pipe, -1 hidden
	int idWall;   // wall (todo: material id) -1 hidden
	
	Ogre::Real chkR;    // checkpoint sphere radius (0-none)
	bool chk1st;  // 1st checkpoint (1), just once on road
	
	bool notReal;  // true means only for decoration, or point move, not real driven road
	inline bool isnt() {  return idMtr == -1 && !notReal;  }  // real hidden

	SplinePoint();
	void SetDefault();
};


//  checkpoint
//  for car checking
class CheckSphere
{
public:
	Ogre::Vector3 pos;
	Ogre::Real r = 1.f, r2 = 1.f;  // radius, r*r
	bool loop = false;  // for car camera change
	
	//  for drive progress %
	Ogre::Real dist[2] = {1.f, 1.f};  // summed distances (cur to next)
		// [0] normal, [1] reversed track
};



//------------------------------------------------------
//  Spline Base,  only interpolation
//------------------------------------------------------
class SplineBase
{
public:
	friend class CGui;


	//  points
	void clear();
	inline int getNumPoints() const {  return (int)mP.size();  }

	//  get next, prev points
	inline int getPrev(int id) const
	{	int s = (int)mP.size();  return isLooped ?  (id-1+s) % s : std::max(0,   id-1);  }
	inline int getNext(int id) const
	{	int s = (int)mP.size();  return isLooped ?  (id+1) % s   : std::min(s-1, id+1);  }
	inline int getAdd(int id, int n) const
	{	int s = (int)mP.size();  return isLooped ?  (id+n+s) % s : std::min(s-1, std::max(0, id+n));  }


	//  pos
	const Ogre::Vector3& getPos(int index) const;
	void setPos(int index, const Ogre::Vector3& value);

	SplinePoint& getPoint(int index);

	
	//  interpolate
	//  get value at a single segment of the spline  t = 0..1
	Ogre::Vector3 interpolate(int id, Ogre::Real t) const;

	//  interpolate 1 dim vars
	Ogre::Real interpWidth(int id, Ogre::Real t) const;
	
	void recalcTangents();


	//  dir, length		
	Ogre::Real GetSegLen(int seg);
	Ogre::Vector3 GetLenDir(int seg, Ogre::Real l, Ogre::Real la);
	static Ogre::Vector3 GetRot(Ogre::Real ayaw, Ogre::Real ang);


	bool isLooped = true;  ///=closed, if false begin and end are not connected
protected:

	std::deque<SplinePoint> mP;  // points
	static std::deque<SplinePoint> mPc;  // copy points
};



//--------------------------------------------------------------------------------------
//  Spline Edit,  base with editing
//--------------------------------------------------------------------------------------

class SplineEdit : public SplineBase
{
public:
	//  terrain helpers
	Ogre::Terrain* mTerrain =0;  // for on terrain, height snap

	Ogre::Real getTerH(const Ogre::Vector3& p);

	void UpdPointsH();  // set markers pos, h on ter
	

	//  point sel  ----
	void ChoosePoint();  // choose one
	void PrevPoint(),NextPoint(), FirstPoint(),LastPoint();
	void CopyNewPoint();  // set new point params from chosen

	void SelAddPoint();  // toggle sel
	void SelClear(),SelAll();
	int GetSelCnt();  // select many

	//  modify road point  ----
	void ToggleOnTerrain(), ToggleColumn();
	void ChgMtrId(int rel);  // next
	void ChgWallId(int rel);  // next
	void ChgAngType(int rel), AngZero();

	void ToggleOnPipe(bool old=false);  // extras
	void ChgLoopType(int rel), ToggleNotReal();


	///  Edit  ====
	void Move1(int id, Ogre::Vector3 relPos);
	void Move(Ogre::Vector3 relPos);  // 1 or sel
	void Scale1(int id, Ogre::Real posMul, Ogre::Real hMul);

	void AddWidth(Ogre::Real relW);
	void AddRoll(Ogre::Real relA,Ogre::Real snapA, bool alt);  // changes camber
	void AddYaw( Ogre::Real relA,Ogre::Real snapA, bool alt);  // auto-
	void AddPipe(Ogre::Real relP);


	//  Edit Selected  ====
	Ogre::Vector3 getPos0();  // selection center point (or chosen)

	void RotateSel(Ogre::Real relA, Ogre::Vector3 axis, int addYawRoll);
	void ScaleSel(Ogre::Real posMul);
	void MirrorSel(bool alt);  // reverse order of points

	
protected:	
	SplinePoint newP;  // new point for insert

	//  selection  ----
	//  chosen stays, SelPoint is under mouse Pick
	int iChosen = -1, iSelPoint = -1;  // -1 if none
	std::set<int> vSel;  // selected points

	bool bSelChng = 0;  // rebuild road after end of selection change


	//  rebuild, mark only  ----
	bool rebuild = false;
	int iDirtyId = -1;
	void Rebuild(bool full = false);


	Ogre::Real g_Height = 0.1f;	 ///geom  above terrain global

	
	struct Mark  // marker node  ----
	{
		Ogre::SceneNode* nd =0; //,*ndC;
		Ogre::Entity* ent =0; //,*entC;
		
		void setPos(Ogre::Vector3 pos);
		void setVis(bool vis);
	};
	std::vector<Mark> vMarks;
};


//--------------------------------------------------------------------------------------
//  Spline EditChk,  with checkpoints and car start
//--------------------------------------------------------------------------------------

class SplineEditChk : public SplineEdit
{
public:
	//  edit chks
	void AddChkR(Ogre::Real relR, bool dontCheckR=false);  // change radius
	void AddBoxW(Ogre::Real rel), AddBoxH(Ogre::Real rel);  // start dim
	void Set1stChk();


//  checkpoint spheres  ----
	std::vector<CheckSphere> mChks;
	Ogre::Vector3 vStBoxDim;   // start/finish box, half dimensions

	int iDir = 1;     // -1 or +1  if road points go +/-1 with car start orientation
	int iChkId1 = 0, iChkId1Rev = 0;   // 1st chekpoint index (and for reversed) for mChks[]

	Ogre::Real chksRoadLen = 1.f;      // for %, sum of all mChks[].dist (without last)
};


//--------------------------------------------------------------------------------------
//  Spline MarkEd,  with Markers (spheres)
//--------------------------------------------------------------------------------------

class SplineMarkEd : public SplineEditChk
{
public:
	//  Setup, call this on Init
	void Setup(Ogre::String sMarkerMeshFile, Ogre::Real scale,
		Ogre::Terrain* terrain, Ogre::SceneManager* sceneMgr,  Ogre::Camera* camera, int idx);
	
	void createMarker(Ogre::String name, Ogre::String mat,
					Ogre::Entity*& ent, Ogre::SceneNode*& nd);

	//  control markers  -------
	void AddMarker(Ogre::Vector3 pos);
	void DestroyMarker(int id), DelLastMarker(), UpdAllMarkers(), DestroyMarkers();
	//  util
	void SetTerHitVis(bool visible), UpdRot();


//  ogre vars
	Ogre::SceneManager* mSceneMgr =0;
	Ogre::Camera* mCamera =0;

	//  setup vars
	int idRd = 0;  // index for more roads
	Ogre::String sMarkerMesh;
	Ogre::Real fMarkerScale = 1.f, fScRot = 1.8f, fScHit = 0.8f;  // scales

	int lastNdSel = -2, lastNdChosen = -2;
	Ogre::SceneNode *ndSel =0, *ndChosen =0, *ndRot =0, *ndHit =0, *ndChk =0;
	Ogre::Entity   *entSel =0, *entChs =0, *entRot =0, *entHit =0, *entChk =0;
};
