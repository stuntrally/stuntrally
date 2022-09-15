#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../vdrift/dbl.h"
#include "Road.h"

#include <OgreCamera.h>
#include <OgreTerrain.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>
using namespace Ogre;


//  choose, selection
//--------------------------------------------------------------------------------------
void SplineEdit::ChoosePoint()
{
	iChosen = iSelPoint;
}
void SplineEdit::CopyNewPoint()
{
	if (iChosen == -1)  return;
	newP = mP[iChosen];
}

//  next
void SplineEdit::PrevPoint()
{
	if (getNumPoints() != 0)
		iChosen = (iChosen-1 + getNumPoints()) % getNumPoints();
}
void SplineEdit::NextPoint()
{
	if (getNumPoints() != 0)
		iChosen = (iChosen+1) % getNumPoints();
}

void SplineEdit::FirstPoint()
{
	if (getNumPoints() != 0)
		iChosen = 0;
}
void SplineEdit::LastPoint()
{
	if (getNumPoints() != 0)
		iChosen = getNumPoints()-1;
}


//  mark need to rebuild geometry
void SplineEdit::Rebuild(bool full)
{
	rebuild = true;
	if (full)
		iDirtyId = -1;
	else
		iDirtyId = iChosen;
}


//  add/rem  select
void SplineEdit::SelAddPoint()
{
	int id = -1;
	if (iChosen   != -1)  id = iChosen;  else
	if (iSelPoint != -1)  id = iSelPoint;
	if (id != -1)
	{
		if (vSel.find(id) == vSel.end())
			vSel.insert(id);
		else
			vSel.erase(id);
	}
}
void SplineEdit::SelClear()
{
	vSel.clear();
}
void SplineEdit::SelAll()
{
	vSel.clear();
	for (size_t i=0; i < mP.size(); ++i)
		vSel.insert(i);
}
int SplineEdit::GetSelCnt()
{
	return vSel.size();
}



//  Move point
///-------------------------------------------------------------------------------------
void SplineEdit::Move1(int id, Vector3 relPos)
{
	Vector3 pos = getPos(id) + relPos;
	if (mP[id].onTer)
		pos.y = getTerH(pos) + g_Height;
	setPos(id, pos);
	vMarks[id].setPos(pos);  // upd marker
}

void SplineEdit::Move(Vector3 relPos)
{
	if (!vSel.empty())  // move sel
	{	for (auto it = vSel.begin(); it != vSel.end(); ++it)
			Move1(*it, relPos);
		bSelChng = true;
		return;
	}
	if (iChosen == -1)  {  // move one
		newP.pos.y += relPos.y;  return;  }
	else
	{	Move1(iChosen, relPos);
		Rebuild();	}
}

//  Scale1 (for tools)
void SplineEdit::Scale1(int id, Real posMul, Real hMul)
{
	Vector3 pos = getPos(id);
	if (posMul != 0.f)
	{	pos.x *= posMul;  pos.z *= posMul;  }
	if (hMul != 0.f)
		pos.y *= hMul;
	
	if (mP[id].onTer)
		pos.y = getTerH(pos) + g_Height;
	setPos(id, pos);
	vMarks[id].setPos(pos);  // upd marker
}


//  Update Points onTer
void SplineEdit::UpdPointsH()
{
	for (int id=0; id < getNumPoints(); ++id)
	{	
		Vector3 pos = getPos(id);
		if (mP[id].onTer)
		{	pos.y = getTerH(pos) + g_Height;
			setPos(id, pos);
		}
		vMarks[id].setPos(pos);  // upd marker
	}
}

Real SplineEdit::getTerH(const Vector3& p)
{
	return mTerrain ? mTerrain->getHeightAtWorldPosition(p.x, 0.f, p.z) : 0.f;
}



///  Edit Selected
///-------------------------------------------------------------------------------------

Vector3 SplineEdit::getPos0()
{
	Vector3 pos0(0,0,0);
	if (iChosen == -1)  // geom center
	{
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
			pos0 += getPos(*it);
		pos0 /= Real(vSel.size());
	}
	else  // or chosen point
		pos0 = getPos(iChosen);
		
	return pos0;
}

//  Scale selected
void SplineEdit::ScaleSel(Real posMul)
{
	Vector3 pos0(0,0,0);  // = getPos0() ?
	if (iChosen != -1)  // 0 or chosen point
		pos0 = getPos(iChosen);

	for (auto it = vSel.begin(); it != vSel.end(); ++it)
	{	int id = *it;
		Vector3 pos = (getPos(id) - pos0) * (1.f + posMul) + pos0;
		if (mP[id].onTer)
			pos.y = getTerH(pos) + g_Height;
		setPos(id, pos);
		vMarks[id].setPos(pos);  // upd marker
	}
}

//  Rotate selected
void SplineEdit::RotateSel(Real relA, Vector3 axis, int addYawRoll)
{
	if (vSel.empty())  return;
	Vector3 pos0 = getPos0();
	
	Quaternion q;  q.FromAngleAxis(Degree(relA), axis);
	Matrix3 m;  q.ToRotationMatrix(m);
	
	//  rotate 2d yaw around center
	for (auto it = vSel.begin(); it != vSel.end(); ++it)
	{
		Vector3 pos = getPos(*it) - pos0;
		Vector3 npos = pos * m + pos0;

		pos = npos;
		if (mP[*it].onTer)
			pos.y = getTerH(pos) + g_Height;
		setPos(*it, pos);
		
		if (addYawRoll==1)  // todo: get from axis?
			mP[*it].mYaw -= relA;  // rot point yaw
		else if (addYawRoll==2)
			// todo: * mul by cos of yaw ?..
			mP[*it].mRoll -= relA;  // rot point roll
		
		vMarks[*it].setPos(pos);
		//Move1(*it, npos);
	}
	bSelChng = true;
}

//  Mirror selected
void SplineEdit::MirrorSel(bool alt)
{
	if (vSel.empty())  return;

	std::vector<SplinePoint> mRev;
	for (auto it = vSel.rbegin(); it != vSel.rend(); ++it)
		mRev.push_back(mP[*it]);
	
	int i = 0;
	for (auto it = vSel.begin(); it != vSel.end(); ++it, ++i)
	{
		SplinePoint& p = mP[*it];
		p = mRev[i];
		p.mRoll = -p.mRoll;
		if (p.aType == AT_Manual)
		{	p.mYaw = p.mYaw+180.f;
			if (p.mYaw > 360.f)  p.mYaw -= 360.f;
		}
	}
	recalcTangents();
	Rebuild(true);
}


//  Edit  modify, controls +-
//--------------------------------------------------------------------------------------

void SplineEdit::AddWidth(Real relW)    ///  Width
{
	if (!vSel.empty()) {  // sel
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].width += relW;
		bSelChng = true;	return;	}
		
	if (iChosen == -1)  {	// one
		newP.width += relW;  return;  }

	mP[iChosen].width  += relW;
	
	Rebuild();
}

void SplineEdit::AddYaw(Real relA, Real snapA, bool alt)    ///  Yaw
{	
	if (!vSel.empty()) {  // rotate sel
		RotateSel(snapA==0.f ? relA : (relA > 0.f ? snapA : -snapA),
			// todo: get Z from camera
			alt ? Vector3::UNIT_Z : Vector3::UNIT_Y, alt ? 2 : 1);  return;  }

	if (iChosen == -1)  {	newP.mYaw += relA;  return;  }

	if (snapA == 0.f)	mP[iChosen].mYaw  += relA;
	else
	{	Real a = mP[iChosen].mYaw;  int i = a / snapA + (relA > 0.f ? 1 :-1);  mP[iChosen].mYaw = i * snapA;  }
	
	Rebuild();
}

void SplineEdit::AddRoll(Real relA, Real snapA, bool alt)   ///  Roll
{
	if (!vSel.empty()) {  // scale sel
		ScaleSel(relA * 0.02f);
		bSelChng = true;	return;  }

	if (iChosen == -1)  {	newP.mRoll += relA;  return;  }

	if (snapA == 0.f)	mP[iChosen].mRoll  += relA;
	else
	{	Real a = mP[iChosen].mRoll;  int i = a / snapA + (relA > 0.f ? 1 :-1);  mP[iChosen].mRoll = i * snapA;  }

	Rebuild();
}

void SplineEdit::AddPipe(Real relP)    ///  Pipe
{
	if (!vSel.empty()) {  // sel
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].pipe = std::max(0.f, std::min(1.f, mP[*it].pipe + relP));
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.pipe = std::max(0.f, std::min(1.f, newP.pipe + relP));  return;  }
	mP[iChosen].pipe  = std::max(0.f, std::min(1.f, mP[iChosen].pipe + relP));

	Rebuild();
}


//  toggle

void SplineEdit::ToggleOnTerrain()   ///  On Ter
{
	if (!vSel.empty()) {  // sel
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].onTer = !mP[*it].onTer;
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.onTer = !newP.onTer;  return;  }
	mP[iChosen].onTer  = !mP[iChosen].onTer;

	if (mP[iChosen].onTer)
		Move(Vector3::ZERO);
}

void SplineEdit::ToggleColumn()      ///  Column
{
	if (!vSel.empty()) {  // sel
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].cols = 1-mP[*it].cols;
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.cols = 1-newP.cols;  return;  }

	mP[iChosen].cols  = 1-mP[iChosen].cols;
	Move(Vector3::ZERO);
}

void SplineEdit::ToggleOnPipe(bool old)  ///  On Pipe  (old for stats only,  new also flips normal)
{
	#define onp(o)  old ? (o ? 0 : 1) : (o ? 0 : 2)
	
	if (!vSel.empty()) {  // sel
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].onPipe = onp(mP[*it].onPipe);
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.onPipe = onp(newP.onPipe);  return;  }

	mP[iChosen].onPipe  = onp(mP[iChosen].onPipe);
	Move(Vector3::ZERO);
}

void SplineEdit::ToggleNotReal()  ///  Not Real  (not drivable road)
{
	if (!vSel.empty()) {  // sel
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].notReal = !mP[*it].notReal;
		return;  }

	if (iChosen == -1)  {  // one
			newP.notReal = !newP.notReal;  return;  }

	mP[iChosen].notReal = !mP[iChosen].notReal;
}

void SplineEdit::ChgLoopType(int rel)   ///  Loop type,  (camera change on chk, pacenotes)
{
	if (!vSel.empty()) {  // sel
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].loop = (LoopTypes+ mP[*it].loop + rel)%LoopTypes;
		return;  }

	if (iChosen == -1)  {  // one
			newP.loop = (LoopTypes+ newP.loop + rel)%LoopTypes;  return;  }

	mP[iChosen].loop  = (LoopTypes+ mP[iChosen].loop + rel)%LoopTypes;
}


void SplineEdit::ChgMtrId(int rel)   ///  Mtr Id
{
	if (!vSel.empty()) {  // sel
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].idMtr = std::max(-1, std::min(MTRs-1, mP[*it].idMtr + rel));
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.idMtr = std::max(-1, std::min(MTRs-1, newP.idMtr + rel));  return;  }
	mP[iChosen].idMtr  = std::max(-1, std::min(MTRs-1, mP[iChosen].idMtr + rel));

	Move(Vector3::ZERO);
}

void SplineEdit::ChgWallId(int rel)   ///  Wall Id
{
	if (!vSel.empty()) {  // sel
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].idWall = std::max(-1, std::min(MTRs-1, mP[*it].idWall + rel));
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.idWall = std::max(-1, std::min(MTRs-1, newP.idWall + rel));  return;  }
	mP[iChosen].idWall  = std::max(-1, std::min(MTRs-1, mP[iChosen].idWall + rel));

	Move(Vector3::ZERO);
}

void SplineEdit::ChgAngType(int rel)   ///  Ang Type
{
	if (!vSel.empty()) {  // sel
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].aType = (AngType)std::max(0, std::min(AT_ALL-1, mP[*it].aType + rel));
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.aType = (AngType)std::max(0, std::min(AT_ALL-1, newP.aType + rel));  return;  }
	mP[iChosen].aType  = (AngType)std::max(0, std::min(AT_ALL-1, mP[iChosen].aType + rel));

	Move(Vector3::ZERO);
}

void SplineEdit::AngZero()   ///  Angles set 0
{
	if (!vSel.empty()) {  // sel
		for (auto it = vSel.begin(); it != vSel.end(); ++it)
		{	mP[*it].mYaw = 0;  mP[*it].mRoll = 0;	}
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.mYaw = 0;  newP.mRoll = 0;  return;  }
	mP[iChosen].mYaw = 0;  mP[iChosen].mRoll = 0;

	Move(Vector3::ZERO);
}


//---------------------------------------------------------------------------------------------------------------

//  util
bool SplineRoad::isPipe(int seg)
{
	int seg1 = (seg+1) % getNumPoints();
	return mP[seg].pipe > 0.f || mP[seg1].pipe > 0.f;
}

//  materials
//  info text only
const String& SplineRoad::getMtrStr(int seg)
{
	static String sHid = TR("#{Hidden}");
	if (seg < 0)  // new
	{
		int i = newP.idMtr;
		if (i==-1)  return sHid;
		return newP.pipe == 0.f ? sMtrRoad[i] : sMtrPipe[i];
	}
	int i = mP[seg].idMtr;
	if (i==-1)  return sHid;
	return !isPipe(seg) ? sMtrRoad[i] : sMtrPipe[i];
}
const String& SplineRoad::getWallMtrStr(int seg)
{
	static String sHid = TR("#{Hidden}");
	if (seg < 0)  // new
	{
		int i = newP.idWall;
		if (i==-1)  return sHid;
		return newP.pipe == 0.f ? sMtrWall : sMtrWallPipe;
	}
	int i = mP[seg].idWall;
	if (i==-1)  return sHid;
	return !isPipe(seg) ? sMtrWall : sMtrWallPipe;  //todo: more [i]..
}

void SplineRoad::SetMtrPipe(int i, String sMtr)
{
	sMtrPipe[i] = sMtr;  // check if glass in mtr name
	bMtrPipeGlass[i] = strstr(sMtr.c_str(), "lass") != 0;
}

void SplineRoad::updMtrRoadTer()
{
	auto& mm = MaterialManager::getSingleton();
	for (int i=0; i < MTRs; ++i)
		bMtrRoadTer[i] = (bool)mm.getByName(sMtrRoad[i] + "_ter");
}


///  Add point
///-------------------------------------------------------------------------------------
void SplineRoad::Insert(eIns ins)
{
	RoadSeg rs;  SplinePoint pt = newP;  // new
	pt.chk1st = false;  // clear 1st chk
	
	if (pt.onTer)
		pt.pos.y = getTerH(pt.pos) + g_Height;

	if (ins	== INS_Begin)
		iChosen = -1;

	if (ins == INS_End)  // end
	{
		mP.push_back(pt);  //recalcTangents();
		vSegs.push_back(rs);
		iChosen = getNumPoints()-1;  //sel last
	}
	else if (iChosen == -1)  // begin  or none sel
	{
		mP.push_front(pt);
		vSegs.push_front(rs);  //recalcTangents();
	}
	else  // middle
	{
		if (ins == INS_CurPre)  // divide in middle
		{
			SplinePoint p1 = mP[iChosen];
			SplinePoint p2 = mP[getNext(iChosen)];
			pt = p1;
			pt.pos   =  interpolate(iChosen, 0.5f);
			pt.width =  interpWidth(iChosen, 0.5f);
			pt.aRoll += (p2.aRoll - p1.aRoll) * 0.5f;
			pt.aYaw  += (p2.aYaw  - p1.aYaw ) * 0.5f;
			pt.pipe  += (p2.pipe  - p1.pipe ) * 0.5f;
		}
		mP.insert(mP.begin()+iChosen+1, pt);
		vSegs.insert(vSegs.begin()+iChosen+1, rs);
		if (ins == INS_Cur)  // INS_CurPre
			++iChosen;
	}
	recalcTangents();

	AddMarker(pt.pos);
	if (ins	!= INS_End)
		UpdAllMarkers();/**/
	Rebuild(/*true*/);
}

///  Delete point
///-------------------------------------------------------------------------------------
void SplineRoad::Delete()
{
	if (iChosen == -1)  return;
	bool last = (iChosen == getNumPoints()-1);

	// remove from sel all ?.
	vSel.erase(getNumPoints()-1);

	DestroySeg(iChosen);
	DelLastMarker();/**/
	lastNdChosen = 0;

	if (last)	mP.pop_back();
	else		mP.erase(mP.begin() + iChosen);
	if (last)	vSegs.pop_back();
	else		vSegs.erase(vSegs.begin() + iChosen);
	
	if (iChosen >= getNumPoints())
		iChosen = getNumPoints()-1;
	//iChosen = -1;  // cancel sel-
	
	recalcTangents();
	UpdAllMarkers();
	Rebuild(/*true*/);
}


///  selection copy, paste, delete
//--------------------------------------------------------------------------------------

std::deque<SplinePoint> SplineBase::mPc;  // copy points

bool SplineRoad::CopySel()
{
	if (vSel.empty())  return false;
	
	mPc.clear();
	for (auto it = vSel.begin(); it != vSel.end(); ++it)
		mPc.push_back(mP[*it]);
	return true;
}

void SplineRoad::Paste(bool reverse)
{
	if (!bHitTer || iChosen==-1 || mPc.size()==0)  return;
	
	Vector3 c(0,0,0);  // center of sel points
	for (int i=0; i < mPc.size(); ++i)
		c += mPc[i].pos;
	c *= 1.f / Real(mPc.size());
	c.y = 0.f;  //c xz only

	vSel.clear();
	for (int i=0; i < mPc.size(); ++i)
	{
		newP = mPc[i];  // [!reverse ? i : mPc.size()-1-i];
		newP.pos += posHit - c;  // move center to hit pos
		Insert(INS_Cur/*INS_CurPre*/);
		vSel.insert(iChosen);  // select just inserted
	}
	if (reverse)  // rot 180
		RotateSel(180, Vector3::UNIT_Y, 1);
	Rebuild(true);
}

void SplineRoad::DelSel()
{
	if (vSel.empty())  return;
	for (auto it = vSel.rbegin(); it != vSel.rend(); ++it)
	{
		iChosen = *it;
		//Delete();
		bool last = (iChosen == getNumPoints()-1);

		DestroySeg(iChosen);
		DelLastMarker();/**/
		lastNdChosen = 0;

		if (last)	mP.pop_back();
		else		mP.erase(mP.begin() + iChosen);
		if (last)	vSegs.pop_back();
		else		vSegs.erase(vSegs.begin() + iChosen);
	}
	if (iChosen >= getNumPoints())
		iChosen = getNumPoints()-1;
	vSel.clear();
	
	recalcTangents();
	UpdAllMarkers();
	Rebuild(true);
}


///  Pick marker
//---------------------------------------------------------------------------------------------------------------
void SplineRoad::Pick(Camera* mCamera, Real mx, Real my,  bool bRay, bool bAddH, bool bHide)
{
	iSelPoint = -1;
	//if (vMarkNodes.size() != getNumPoints())
	//	return;  // assert
	
	Ray ray = mCamera->getCameraToViewportRay(mx,my);  // 0..1
	const Vector3& pos = mCamera->getDerivedPosition(), dir = ray.getDirection();
	const Plane& pl = mCamera->getFrustumPlane(FRUSTUM_PLANE_NEAR);
	Real plDist = FLT_MAX;
	const Real sphR = 2.4f;  //par

	for (int i=0; i < (int)getNumPoints(); ++i)
	{
		// ray to sphere dist
		const Vector3& posSph = getPos(i);
		const Vector3 ps = pos - posSph;
		Vector3 crs = ps.crossProduct(dir);
		Real dist = crs.length() / dir.length();
		// dist to camera
		Real plD = pl.getDistance(posSph);

		if (dist < sphR &&
			plD > 0 && plD < plDist)
		{
			plDist = plD;
			iSelPoint = i;
		}
	}
	
	SelectMarker(bHide);
	
	//  hide/show all markers
	int iHide = bHide ? 1 : 0;
	if (iHide != iOldHide)
	{	iOldHide = iHide;
		for (size_t i=0; i < vMarks.size(); ++i)
			vMarks[i].setVis(!bHide);
	}
	
	//  ray terrain hit pos
	if (bRay && ndHit && mTerrain)
	{
		std::pair<bool, Vector3> p = mTerrain->rayIntersects(ray);
		bHitTer = p.first;  //ndHit->setVisible(bHitTer);
		posHit = p.second;
		
		if (bHitTer)
		{
			Vector3 pos = posHit;
			//if (iChosen == -1)  // for new
			if (!newP.onTer && bAddH)
				pos.y = newP.pos.y;
			ndHit->setPosition(pos);
		}
	}
}
