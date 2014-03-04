#include "pch.h"
#include "../vdrift/dbl.h"
#include "../ogre/common/Def_Str.h"
#include "SplineBase.h"
#include <OgreVector4.h>
#include <OgreLogManager.h>
#include <OgreTerrain.h>
using namespace Ogre;


//  terrain utility
//---------------------------------------------------------------------

Vector3 TerUtil::GetNormalAt(Terrain* terrain, float x, float z, float s)
{
	Real y0=0;
	Vector3 vx(x-s, y0, z), vz(x, y0, z-s);
	Vector3 vX(x+s, y0, z), vZ(x, y0, z+s);
	vx.y = terrain->getHeightAtWorldPosition(vx);  vX.y = terrain->getHeightAtWorldPosition(vX);
	vz.y = terrain->getHeightAtWorldPosition(vz);  vZ.y = terrain->getHeightAtWorldPosition(vZ);
	Vector3 v_x = vx-vX;  v_x.normalise();
	Vector3 v_z = vz-vZ;  v_z.normalise();
	Vector3 n = -v_x.crossProduct(v_z);  n.normalise();
	return n;
}

float TerUtil::GetAngleAt(Terrain* terrain, float x, float z, float s)
{
	//float* hf = terrain ? terrain->getHeightData() : sc->td.hfHeight;
	//Real t = sc->td.fTriangleSize * 2.f;
	//Vector3 vx(t, hf[a+1] - hf[a-1], 0);  // x+1 - x-1
	//Vector3 vz(0, hf[a+wx] - hf[a-wx], t);	// z+1 - z-1
	//Vector3 norm = -vx.crossProduct(vz);  norm.normalise();
	//Real ang = Math::ACos(norm.y).valueDegrees();

	Real y0=0;
	Vector3 vx(x-s, y0, z), vz(x, y0, z-s);
	Vector3 vX(x+s, y0, z), vZ(x, y0, z+s);
	vx.y = terrain->getHeightAtWorldPosition(vx);  vX.y = terrain->getHeightAtWorldPosition(vX);
	vz.y = terrain->getHeightAtWorldPosition(vz);  vZ.y = terrain->getHeightAtWorldPosition(vZ);
	Vector3 v_x = vx-vX;  //v_x.normalise();
	Vector3 v_z = vz-vZ;  //v_z.normalise();
	Vector3 n = -v_x.crossProduct(v_z);  n.normalise();
	Real a = Math::ACos(n.y).valueDegrees();
	return a;
}

float TerUtil::GetAngle(float x, float y)
{
	if (x == 0.f && y == 0.f)
		return 0.f;

	if (y == 0.f)
		return (x < 0.f) ? PI_d : 0.f;
	else
		return (y < 0.f) ? atan2f(-y, x) : (2*PI_d - atan2f(y, x));
}


//  ctor
SplineBase::SplineBase() :
	mAutoCalc(1), isLooped(1/**/)
{	}

SplineBase::~SplineBase()
{	}


//  Interpolate  whole spline  not used-
//---------------------------------------------------------------------
Vector3 SplineBase::interpolate(Real t) const
{
	//  get which segment this is in
	Real fSeg = t * (mP.size() - 1);
	unsigned int seg = (unsigned int)fSeg;
	t = fSeg - seg;

	return interpolate(seg, t);
}

//  Interpolate  segment
//---------------------------------------------------------------------
Vector3 SplineBase::interpolate(int id, Real t) const
{
	//assert (id < mPos.size() && "index out of bounds");

	int id1 = getNext(id);
	const Vector3& p1 = mP[id].pos, p2 = mP[id1].pos;

	//  special cases
	if (t == 0.0f)	    return p1;
	else if(t == 1.0f)  return p2;

	const Vector3& n1 = mP[id].tan, n2 = mP[id1].tan;

	//  Hermite polynomial
	Real t2 = t*t, t3 = t2*t;
	Vector4 tm(	t3* 2 + t2*-3 + 1,  t3*-2 + t2* 3,  t3 + t2*-2 + t,  t3 - t2 );

	//  powers * matCoeffs * Matrix4(point1, point2, tangent1, tangent2)
	return Vector3(
		tm.x*p1.x + tm.y*p2.x + tm.z*n1.x + tm.w*n2.x,
		tm.x*p1.y + tm.y*p2.y + tm.z*n1.y + tm.w*n2.y,
		tm.x*p1.z + tm.y*p2.z + tm.z*n1.z + tm.w*n2.z);
}

Real SplineBase::interpWidth(int id, Real t) const
{
	int id1 = getNext(id);
	const Real& p1 = mP[id].width, p2 = mP[id1].width;
	if (t == 0.0f)	    return p1;
	else if(t == 1.0f)  return p2;
	const Real& n1 = mP[id].wtan,  n2 = mP[id1].wtan;

	Real t2 = t*t, t3 = t2*t;
	Vector4 tm(	t3* 2 + t2*-3 + 1,  t3*-2 + t2* 3,  t3 + t2*-2 + t,  t3 - t2 );
	return tm.x*p1 + tm.y*p2 + tm.z*n1 + tm.w*n2;
}

Real SplineBase::interpAYaw(int id, Real t) const  // ..
{
	int id1 = getNext(id);
	const Real& p1 = mP[id].aYaw, p2 = mP[id1].aYaw;
	if (t == 0.0f)	    return p1;
	else if(t == 1.0f)  return p2;
	const Real& n1 = mP[id].tYaw, n2 = mP[id1].tYaw;

	Real t2 = t*t, t3 = t2*t;
	Vector4 tm(	t3* 2 + t2*-3 + 1,  t3*-2 + t2* 3,  t3 + t2*-2 + t,  t3 - t2 );
	return tm.x*p1 + tm.y*p2 + tm.z*n1 + tm.w*n2;
}

Real SplineBase::interpARoll(int id, Real t) const
{
	int id1 = getNext(id);
	const Real& p1 = mP[id].aRoll, p2 = mP[id1].aRoll;
	if (t == 0.0f)	    return p1;
	else if(t == 1.0f)  return p2;
	const Real& n1 = mP[id].tRoll, n2 = mP[id1].tRoll;

	Real t2 = t*t, t3 = t2*t;
	Vector4 tm(	t3* 2 + t2*-3 + 1,  t3*-2 + t2* 3,  t3 + t2*-2 + t,  t3 - t2 );
	return tm.x*p1 + tm.y*p2 + tm.z*n1 + tm.w*n2;
}


void SplineBase::preAngle(int i)
{
	//LogO("pre + " + toStr(i));
	int i1 = getNext(i);
	//  more than 180 swirl - wrong at start/end
	const Real asw = 180;
	Real ay = mP[i].aYaw, ay1 = mP[i1].aYaw, ay21 = ay1-ay;
	//Real ar = mP[i].aRoll,ar1 = mP[i1].aRoll,ar21 = ar1-ar;

	while (ay21 > asw) {  LogO(">a1.yw21: "+toStr(ay21)+"  ay2: "+toStr(ay1)+"  ay1: "+toStr(ay));  ay21 -= 2*asw;  ay1 -= 2*asw;  }
	while (ay21 <-asw) {  LogO("<a2.yw21: "+toStr(ay21)+"  ay2: "+toStr(ay1)+"  ay1: "+toStr(ay));  ay21 += 2*asw;  ay1 += 2*asw;  }
	//while (ar21 > asw) {  LogO(">a3.rl21: "+toStr(ar21)+"  ar2: "+toStr(ar1)+"  ar1: "+toStr(ar));  ar21 -= 2*asw;  ar1 -= 2*asw;  }
	//while (ar21 <-asw) {  LogO("<a4.rl21: "+toStr(ar21)+"  ar2: "+toStr(ar1)+"  ar1: "+toStr(ar));  ar21 += 2*asw;  ar1 += 2*asw;  }
	mP[i].aY = ay;  mP[i1].aY = ay1;
	//mP[i].aR = ar;  mP[i1].aR = ar1;
}


//  Tangents
//---------------------------------------------------------------------
void SplineBase::recalcTangents()
{
	// Catmull-Rom approach
	//   tangent[i] = 0.5 * (point[i+1] - point[i-1])

	int i, num = mP.size();
	if (num < 2)  return;

	for (i=0; i < num; ++i)
	{
		// tangent   next-prev
		int next = getNext(i), prev = getPrev(i);
		mP[i].tan = 0.5 * (mP[next].pos - mP[prev].pos);
		mP[i].wtan= 0.5 * (mP[next].width - mP[prev].width);
		
		/*preAngle(prev);  preAngle(i);  preAngle(next);

		mP[i].tYaw  = 0.5 * (mP[next].aY - mP[prev].aY);
		mP[i].tRoll = 0.5 * (mP[next].aR - mP[prev].aR);/**/
	}
}

//  get, set pos
//---------------------------------------------------------------------
const Vector3& SplineBase::getPos(int index) const
{
	//assert (index < mPos.size() && "index out of bounds");
	return mP[index].pos;
}

void SplineBase::setPos(int index, const Vector3& value)
{
	//assert (index < mPos.size() && "index out of bounds");
	mP[index].pos = value;
	if (mAutoCalc)
		recalcTangents();
}

SplinePoint& SplineBase::getPoint(int index)
{
	return mP[index];
}


//  clear
void SplineBase::clear()
{
	mP.clear();
}

//  add  not used-
void SplineBase::addPoint(const Vector3& p)
{
	SplinePoint pt;
	pt.pos = p;  //..
	mP.push_back(pt);
	if (mAutoCalc)
		recalcTangents();
}

void SplineBase::setAutoCalculate(bool autoCalc)
{
	mAutoCalc = autoCalc;
}


//  default point
//
SplinePoint::SplinePoint()
{
	SetDefault();
}
void SplinePoint::SetDefault()
{
	pos = Vector3::ZERO;  tan = Vector3::ZERO;  onTer = 1;
	width = 7;  aYaw = 0;  aRoll = 0;
	mYaw = 0;  mRoll = 0;  aType = AT_Both;
	cols = 1;  pipe = 0;  idMtr = 0;  chkR = 0;
	onPipe = 0;  loopChk = 0;
	chk1st = false;
}
