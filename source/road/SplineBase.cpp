#include "pch.h"
#include "../vdrift/dbl.h"
#include "../ogre/common/Def_Str.h"
#include "SplineBase.h"
#include <Ogre.h>
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


//  seg length
//---------------------------------------------------------------------
Real SplineBase::GetSegLen(int seg)
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

//  length dir
Vector3 SplineBase::GetLenDir(int seg, Real l, Real la)
{
	Vector3 vL0 = interpolate(seg, l);
	Vector3 vL1 = interpolate(seg, la);
	return vL1 - vL0;
}

//  rot dir
Vector3 SplineBase::GetRot(Real aYaw, Real aRoll)
{
	Real ay = aYaw * PI_d/180.f, ar = aRoll * PI_d/180.f;
	Real cb = cosf(ar);
	return Vector3( cosf(ay)*cb, sinf(ar), -sinf(ay)*cb );
}


//  Interpolate  segment
//---------------------------------------------------------------------
Vector3 SplineBase::interpolate(int id, Real t) const
{
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
	}
}

//  get, set pos
//---------------------------------------------------------------------
const Vector3& SplineBase::getPos(int index) const
{
	return mP[index].pos;
}

void SplineBase::setPos(int index, const Vector3& value)
{
	mP[index].pos = value;

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


//  default point
//
SplinePoint::SplinePoint()
{
	SetDefault();
}
void SplinePoint::SetDefault()
{
	pos = Vector3::ZERO;  tan = Vector3::ZERO;  onTer = 1;
	width = 7.f;  aYaw = 0;  aRoll = 0;  wtan = 0.f;
	mYaw = 0;  mRoll = 0;  aType = AT_Both;
	cols = 1;  pipe = 0.f;
	idMtr = 0;  idWall = 0;  chkR = 0.f;
	onPipe = 0;  loop = 0;
	chk1st = false;  notReal = false;
	nCk = -1;
}

