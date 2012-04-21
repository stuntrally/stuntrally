#include "pch.h"
#include "../ogre/common/Defines.h"
#include "Road.h"

#include <OgreCamera.h>
#include <OgreTerrain.h>
using namespace Ogre;


///  Pick marker
//---------------------------------------------------------------------------------------------------------------
void SplineRoad::Pick(Camera* mCamera, Real mx, Real my, bool bAddH, bool bHide)
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
		for (size_t i=0; i < vMarkNodes.size(); ++i)
			vMarkNodes[i]->setVisible(bAddH);
	}
	
	//  ray terrain hit pos
	if (ndHit && mTerrain)
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


//  Move point
///-------------------------------------------------------------------------------------
void SplineRoad::Move1(int id, Vector3 relPos)
{
	Vector3 pos = getPos(id) + relPos;
	if (mP[id].onTer)
		pos.y = mTerrain->getHeightAtWorldPosition(pos.x, 0, pos.z) + fHeight;
	setPos(id, pos);
	vMarkNodes[id]->setPosition(pos);  // upd marker
}

void SplineRoad::Move(Vector3 relPos)
{
	if (!vSel.empty())  // move sel
	{	for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
			Move1(*it, relPos);
		bSelChng = true;
		return;
	}
	if (iChosen == -1)  {  // move one
		newP.pos.y += relPos.y;  return;  }
	else
	{	Move1(iChosen, relPos);
		RebuildRoad();	}
}

//  Scale
void SplineRoad::Scale1(int id, Real posMul)
{
	Vector3 pos = getPos(id) * (1.f + posMul);
	if (mP[id].onTer)
		pos.y = mTerrain->getHeightAtWorldPosition(pos.x, 0, pos.z) + fHeight;
	setPos(id, pos);
	vMarkNodes[id]->setPosition(pos);  // upd marker
}

void SplineRoad::ScaleSel(Real posMul)
{
	Vector3 pos0(0,0,0);  // = getPos0() ?
	if (iChosen != -1)  // 0 or chosen point
		pos0 = getPos(iChosen);

	for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
	{	int id = *it;
		Vector3 pos = (getPos(id) - pos0) * (1.f + posMul) + pos0;
		if (mP[id].onTer)
			pos.y = mTerrain->getHeightAtWorldPosition(pos.x, 0, pos.z) + fHeight;
		setPos(id, pos);
		vMarkNodes[id]->setPosition(pos);  // upd marker
	}
}

Vector3 SplineRoad::getPos0()
{
	Vector3 pos0(0,0,0);
	if (iChosen == -1)  // geom center
	{
		for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
			pos0 += getPos(*it);
		pos0 /= Real(vSel.size());
	}
	else  // or chosen point
		pos0 = getPos(iChosen);
		
	return pos0;
}

//  Rotate selected
void SplineRoad::RotateSel(Real relA, Vector3 axis, int addYawRoll)
{
	if (vSel.empty())  return;
	Vector3 pos0 = getPos0();
	
	Quaternion q;  q.FromAngleAxis(Degree(relA), axis);
	Matrix3 m;  q.ToRotationMatrix(m);
	
	//  rotate 2d yaw around center
	for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
	{
		Vector3 pos = getPos(*it) - pos0;
		Vector3 npos = pos * m + pos0;

		pos = npos;
		if (mP[*it].onTer)
			pos.y = mTerrain->getHeightAtWorldPosition(pos.x, 0, pos.z) + fHeight;
		setPos(*it, pos);
		
		if (addYawRoll==1)  // todo: get from axis?
			mP[*it].mYaw -= relA;  // rot point yaw
		else if (addYawRoll==2)
			// todo: * mul by cos of yaw ?..
			mP[*it].mRoll -= relA;  // rot point roll
		
		vMarkNodes[*it]->setPosition(pos);
		//Move1(*it, npos);
	}
	bSelChng = true;
}


///  Add point
///-------------------------------------------------------------------------------------
void SplineRoad::Insert(eIns ins)
{
	RoadSeg rs;  SplinePoint pt = newP;  // new
	if (pt.onTer)
		pt.pos.y = mTerrain->getHeightAtWorldPosition(pt.pos.x, 0, pt.pos.z) + fHeight;

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
		mP.insert(mP.begin()+iChosen+1, pt);
		vSegs.insert(vSegs.begin()+iChosen+1, rs);
		if (ins == INS_Cur)  // INS_CurPre
			iChosen++;
	}
	recalcTangents();

	AddMarker(pt.pos);
	if (ins	!= INS_End)
		UpdAllMarkers();/**/
	RebuildRoad(/*true*/);
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
	RebuildRoad(/*true*/);
}


///  util
//---------------------------------------------------------------------------------------------------------------

//  seg len
Real SplineRoad::GetSegLen(int seg)
{
	//  iterations-1 quality
	#define lenQ  5

	Real len = 0;
	Vector3 p0;
	for (int i=0; i <= lenQ; ++i)
	{
		Vector3 p = interpolate(seg, Real(i) / lenQ);
		if (i > 0)
		{
			Vector3 l = p - p0;
			len += l.length();
		}
		p0 = p;
	}
	return len;
}

//  len dir
Vector3 SplineRoad::GetLenDir(int seg, Real l, Real la)
{
	Vector3 vL0 = interpolate(seg, l);
	Vector3 vL1 = interpolate(seg, la);
	return vL1 - vL0;
}

//  rot
Vector3 SplineRoad::GetRot(Real aYaw, Real aRoll)
{
	Real ay = aYaw * PI_d/180.f, ar = aRoll * PI_d/180.f;
	Real cb = cosf(ar);
	return Vector3( cosf(ay)*cb, sinf(ar), -sinf(ay)*cb );
}

//  hit pos
void SplineRoad::SetTerHitVis(bool visible)
{
	if (ndHit)
		ndHit->setVisible(visible);
}


///  choose, selection
//--------------------------------------------------------------------------------------

std::deque<SplinePoint> SplineRoad::mPc;  // copy points

bool SplineRoad::CopySel()
{
	if (vSel.empty())  return false;
	
	mPc.clear();
	for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
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
	RebuildRoad(true);
}

void SplineRoad::DelSel()
{
	if (vSel.empty())  return;
	for (std::set<int>::reverse_iterator it = vSel.rbegin(); it != vSel.rend(); ++it)
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
	RebuildRoad(true);
}


///  choose, selection
//--------------------------------------------------------------------------------------
void SplineRoad::ChoosePoint()
{
	iChosen = iSelPoint;
}
void SplineRoad::CopyNewPoint()
{
	if (iChosen == -1)  return;
	newP = mP[iChosen];
}

//  add/rem  multi sel
void SplineRoad::SelAddPoint()
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
void SplineRoad::SelClear()
{
	vSel.clear();
}
void SplineRoad::SelAll()
{
	vSel.clear();
	for (size_t i=0; i < mP.size(); ++i)
		vSel.insert(i);
}
int SplineRoad::GetSelCnt()
{
	return vSel.size();
}


//  next
void SplineRoad::PrevPoint()
{
	if (getNumPoints() != 0)
		iChosen = (iChosen-1 + getNumPoints()) % getNumPoints();
}
void SplineRoad::NextPoint()
{
	if (getNumPoints() != 0)
		iChosen = (iChosen+1) % getNumPoints();
}

void SplineRoad::FirstPoint()
{
	if (getNumPoints() != 0)
		iChosen = 0;
}
void SplineRoad::LastPoint()
{
	if (getNumPoints() != 0)
		iChosen = getNumPoints()-1;
}


//  modify, controls+-
//--------------------------------------------------------------------------------------
void SplineRoad::AddChkR(Real relR)    ///  ChkR
{
	if (iChosen == -1)  return;
	mP[iChosen].chkR = std::max(0.f, mP[iChosen].chkR + relR);
}
void SplineRoad::AddBoxW(Real rel)
{
	vStBoxDim.z = std::max(0.1f, vStBoxDim.z + rel);
}
void SplineRoad::AddBoxH(Real rel)
{
	vStBoxDim.y = std::max(0.1f, vStBoxDim.y + rel);
}

void SplineRoad::AddWidth(Real relW)     ///  Width
{
	if (!vSel.empty()) {  // sel
		for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].width += relW;
		bSelChng = true;	return;	}
		
	if (iChosen == -1)  {	// one
		newP.width += relW;  return;  }

	mP[iChosen].width  += relW;
	
	RebuildRoad();
}

void SplineRoad::AddYaw(Real relA, Real snapA, bool alt)    ///  Yaw
{	
	if (!vSel.empty()) {  // rotate sel
		RotateSel(snapA==0.f ? relA : (relA > 0.f ? snapA : -snapA),
			// todo: get Z from camera
			alt ? Vector3::UNIT_Z : Vector3::UNIT_Y, alt ? 2 : 1);  return;  }

	if (iChosen == -1)  {	newP.mYaw += relA;  return;  }

	if (snapA == 0.f)	mP[iChosen].mYaw  += relA;
	else
	{	Real a = mP[iChosen].mYaw;  int i = a / snapA + (relA > 0.f ? 1 :-1);  mP[iChosen].mYaw = i * snapA;  }
	
	RebuildRoad();
}

void SplineRoad::AddRoll(Real relA, Real snapA, bool alt)   ///  Roll
{
	if (!vSel.empty()) {  // scale sel
		ScaleSel(relA * 0.02f);
		bSelChng = true;	return;  }

	if (iChosen == -1)  {	newP.mRoll += relA;  return;  }

	if (snapA == 0.f)	mP[iChosen].mRoll  += relA;
	else
	{	Real a = mP[iChosen].mRoll;  int i = a / snapA + (relA > 0.f ? 1 :-1);  mP[iChosen].mRoll = i * snapA;  }

	RebuildRoad();
}

void SplineRoad::ToggleOnTerrain()   ///  On Ter
{
	if (!vSel.empty()) {  // sel
		for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].onTer = !mP[*it].onTer;
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.onTer = !newP.onTer;  return;  }
	mP[iChosen].onTer  = !mP[iChosen].onTer;

	if (mP[iChosen].onTer)
		Move(Vector3::ZERO);
}

void SplineRoad::ToggleColums()      ///  Column
{
	if (!vSel.empty()) {  // sel
		for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].cols = 1-mP[*it].cols;
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.cols = 1-newP.cols;  return;  }

	mP[iChosen].cols  = 1-mP[iChosen].cols;
	Move(Vector3::ZERO);
}

void SplineRoad::AddPipe(Real relP)    ///  Pipe
{
	if (!vSel.empty()) {  // sel
		for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].pipe = std::max(0.f, std::min(1.f, mP[*it].pipe + relP));
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.pipe = std::max(0.f, std::min(1.f, newP.pipe + relP));  return;  }
	mP[iChosen].pipe  = std::max(0.f, std::min(1.f, mP[iChosen].pipe + relP));

	RebuildRoad();
}

void SplineRoad::ChgMtrId(int relId)   ///  Mtr Id
{
	if (!vSel.empty()) {  // sel
		for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].idMtr = std::max(-1, std::min(MTRs-1, mP[*it].idMtr + relId));
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.idMtr = std::max(-1, std::min(MTRs-1, newP.idMtr + relId));  return;  }
	mP[iChosen].idMtr  = std::max(-1, std::min(MTRs-1, mP[iChosen].idMtr + relId));

	Move(Vector3::ZERO);
}

void SplineRoad::ChgAngType(int relId)   ///  Ang Type
{
	if (!vSel.empty()) {  // sel
		for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
			mP[*it].aType = (AngType)std::max(0, std::min(AT_ALL-1, mP[*it].aType + relId));
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.aType = (AngType)std::max(0, std::min(AT_ALL-1, newP.aType + relId));  return;  }
	mP[iChosen].aType  = (AngType)std::max(0, std::min(AT_ALL-1, mP[iChosen].aType + relId));

	Move(Vector3::ZERO);
}

void SplineRoad::AngZero()   ///  Angles set 0
{
	if (!vSel.empty()) {  // sel
		for (std::set<int>::const_iterator it = vSel.begin(); it != vSel.end(); ++it)
		{	mP[*it].mYaw = 0;  mP[*it].mRoll = 0;	}
		bSelChng = true;	return;  }

	if (iChosen == -1)  {  // one
			newP.mYaw = 0;  newP.mRoll = 0;  return;  }
	mP[iChosen].mYaw = 0;  mP[iChosen].mRoll = 0;

	Move(Vector3::ZERO);
}

void SplineRoad::Set1stChk()
{
	if (iChosen >= 0 && iChosen < getNumPoints())
	if (mP[iChosen].chkR > 0.f)
		iP1 = std::max(0, std::min(getNumPoints()-1, iChosen));
}

//  util
bool SplineRoad::isPipe(int seg)
{
	int seg1 = (seg+1) % getNumPoints();
	return mP[seg].pipe > 0.f || mP[seg1].pipe > 0.f;
}
//  info text only
const String& SplineRoad::getMtrStr(int seg)
{
	if (seg < 0)  // new
		return newP.pipe == 0.f ? sMtrRoad[newP.idMtr] : sMtrPipe[newP.idMtr];
	int i = mP[seg].idMtr;
	static String sHid = "Hidden";
	if (i < 0)  return sHid;
	return !isPipe(seg) ? sMtrRoad[i] : sMtrPipe[i];
}
