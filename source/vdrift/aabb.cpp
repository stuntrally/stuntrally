#include "pch.h"
#include "aabb.h"
#include "mathvector.h"
#include "unittest.h"

void distribute(float frustum[][4])
{
	for (int i = 1; i < 6; ++i)
	for (int n = 0; n < 4; ++n)
		frustum[i][n] = frustum[0][n];
}

QT_TEST(aabb_test)
{
	AABB <float> box1;
	AABB <float> box2;
	
	MATHVECTOR<float,3> c1;
	MATHVECTOR<float,3> c2;
	
	c1.Set(-1,-1,-1);
	c2.Set(1,1,1);
	box1.SetFromCorners(c1, c2);
	
	c1.Set(-0.01, -0.01, 0);
	c2.Set(0.01,0.01, 2);
	box2.SetFromCorners(c1, c2);
	
	QT_CHECK(box1.Intersect(box2));
	
	AABB <float> box3;
	c1.Set(-0.01, -0.01, 2);
	c2.Set(0.01,0.01, 3);
	box3.SetFromCorners(c1, c2);
	
	QT_CHECK(!box1.Intersect(box3));
	
	MATHVECTOR<float,3> orig;
	MATHVECTOR<float,3> dir;
	orig.Set(0,0,4);
	dir.Set(0,0,-1);
	
	//QT_CHECK(box1.IntersectRay(orig,dir));
	//QT_CHECK(!box1.IntersectRay(orig,dir*-1));
	
	QT_CHECK(box1.Intersect(AABB<float>::RAY(orig,dir,4)));
	QT_CHECK(!box1.Intersect(AABB<float>::RAY(orig,dir*-1,4)));
	QT_CHECK(!box1.Intersect(AABB<float>::RAY(orig,dir,1)));
	
	{
		float plane[6][4];
		plane[0][0] = 0;
		plane[0][1] = 0;
		plane[0][2] = 1;
		plane[0][3] = 10;
		distribute(plane);
		QT_CHECK(box1.Intersect(AABB<float>::FRUSTUM(plane)));
	}
	{
		float plane[6][4];
		plane[0][0] = 0;
		plane[0][1] = 0;
		plane[0][2] = 1;
		plane[0][3] = 0;
		distribute(plane);
		QT_CHECK(box1.Intersect(AABB<float>::FRUSTUM(plane)));
	}
	{
		float plane[6][4];
		plane[0][0] = 0;
		plane[0][1] = 0;
		plane[0][2] = 1;
		plane[0][3] = -10;
		distribute(plane);
		QT_CHECK(!box1.Intersect(AABB<float>::FRUSTUM(plane)));
	}
	{
		float plane[6][4];
		plane[0][0] = -1;
		plane[0][1] = 0;
		plane[0][2] = 0;
		plane[0][3] = 10000;
		distribute(plane);
		QT_CHECK(box1.Intersect(AABB<float>::FRUSTUM(plane)));
	}
	{
		float plane[6][4];
		plane[0][0] = 1;
		plane[0][1] = 0;
		plane[0][2] = 0;
		plane[0][3] = -119;
		distribute(plane);
		QT_CHECK(!box1.Intersect(AABB<float>::FRUSTUM(plane)));
	}
}
