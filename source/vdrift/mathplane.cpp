#include "pch.h"
#include "mathplane.h"
#include "unittest.h"

//#include <iostream>
using std::ostream;

QT_TEST(mathplane_test)
{
	QT_CHECK_CLOSE(MATHPLANE <float> (0,1,0,0).DistanceToPoint(MATHVECTOR<float,3> (0,0,0)), 0.0f, 0.0001f);
	QT_CHECK_CLOSE(MATHPLANE <float> (0,1,0,0).DistanceToPoint(MATHVECTOR<float,3> (1,1,1)), 1.0f, 0.0001f);
	QT_CHECK_CLOSE(MATHPLANE <float> (0,1,0,0).DistanceToPoint(MATHVECTOR<float,3> (1,-1,1)), -1.0f, 0.0001f);
	QT_CHECK_CLOSE(MATHPLANE <float> (0,1,0,-3).DistanceToPoint(MATHVECTOR<float,3> (100,3,-40)), 0.0f, 0.0001f);
}
