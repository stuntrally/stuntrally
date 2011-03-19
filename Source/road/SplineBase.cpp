#include "stdafx.h"
#include "SplineBase.h"


SplineBase::SplineBase() :
	mAutoCalc(1), isLooped(1)
{
}

SplineBase::~SplineBase()
{
}


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

	//  special cases
	int id1 = (id + 1) % mP.size(); // next

	const Vector3& p1 = mP[id].pos, p2 = mP[id1].pos;

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
	int id1 = (id + 1) % mP.size();
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
	int id1 = (id + 1) % mP.size();
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
	int id1 = (id + 1) % mP.size();
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
	//Log("pre + " + toStr(i));
	int i1 = (i+1) % mP.size();
	//  more than 180 swirl - wrong at start/end
	const Real asw = 180;
	Real ay = mP[i].aYaw, ay1 = mP[i1].aYaw, ay21 = ay1-ay;
	//Real ar = mP[i].aRoll,ar1 = mP[i1].aRoll,ar21 = ar1-ar;
	while (ay21 > asw) {  Log(">a1.yw21: "+toStr(ay21)+"  ay2: "+toStr(ay1)+"  ay1: "+toStr(ay));  ay21 -= 2*asw;  ay1 -= 2*asw;  }
	while (ay21 <-asw) {  Log("<a2.yw21: "+toStr(ay21)+"  ay2: "+toStr(ay1)+"  ay1: "+toStr(ay));  ay21 += 2*asw;  ay1 += 2*asw;  }
	//while (ar21 > asw) {  Log(">a3.rl21: "+toStr(ar21)+"  ar2: "+toStr(ar1)+"  ar1: "+toStr(ar));  ar21 -= 2*asw;  ar1 -= 2*asw;  }
	//while (ar21 <-asw) {  Log("<a4.rl21: "+toStr(ar21)+"  ar2: "+toStr(ar1)+"  ar1: "+toStr(ar));  ar21 += 2*asw;  ar1 += 2*asw;  }
	mP[i].aY = ay;  mP[i1].aY = ay1;
	//mP[i].aR = ar;  mP[i1].aR = ar1;
}


//  Tangents
//---------------------------------------------------------------------
void SplineBase::recalcTangents()
{
	// Catmull-Rom approach
	//   tangent[i] = 0.5 * (point[i+1] - point[i-1])

	size_t i, num = mP.size();
	if (num < 2)  return;

	//if (isLooped)
	for (i=0; i < num; ++i)
	{
		// tangent   next-prev
		size_t next = (i+1) % num, prev = (i-1+num) % num;
		mP[i].tan = 0.5 * (mP[next].pos - mP[prev].pos);
		mP[i].wtan= 0.5 * (mP[next].width - mP[prev].width);
		
		/*preAngle(prev);  preAngle(i);  preAngle(next);

		mP[i].tYaw  = 0.5 * (mP[next].aY - mP[prev].aY);
		mP[i].tRoll = 0.5 * (mP[next].aR - mP[prev].aR);/**/
	}
		/*if (i == 0)  // start
		{
			if (isLooped)	// Use numPoints-2 since numPoints-1 is the last point and == [0]
				mP[i].tan = 0.5 * (mP[1].pos - mP[numPoints-1].pos);
			else
				mP[i].tan = 0.5 * (mP[1].pos - mP[0].pos);
		}
		else if (i == numPoints-1)  // end
		{
			//if (isLooped)	// Use same tangent as already calculated for [0]
			//	mP[i].tan = mP[0].tan;
			//else
			mP[i].tan = 0.5 * (mP[i].pos - mP[i-1].pos);
		}
		else*/  // norm   prev-  -next
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

int SplineBase::getNumPoints() const
{
	return (int)mP.size();
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
	width = 6;  aYaw = 0;  aRoll = 0;
	cols = 1;  pipe = 0;  idMtr = 0;
	chkR = 0;
}
