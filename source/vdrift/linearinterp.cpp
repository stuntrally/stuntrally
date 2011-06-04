#include "pch.h"
#include "linearinterp.h"
#include "unittest.h"

#include <cmath>

#include <iostream>
using std::ostream;

QT_TEST(linearinterp_test)
{
	{
		LINEARINTERP <float> l;
		QT_CHECK_CLOSE(l.Interpolate(1),0,0.0001);
	}
	
	{
		LINEARINTERP <float> l(3.1);
		QT_CHECK_CLOSE(l.Interpolate(1),3.1,0.0001);
	}
	
	{
		LINEARINTERP <float> l;
		l.AddPoint(2,1);
		QT_CHECK_CLOSE(l.Interpolate(1),1,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(2),1,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(3),1,0.0001);
	}
	
	{
		LINEARINTERP <float> l;
		l.AddPoint(2,1);
		l.AddPoint(3,2);
		
		l.SetBoundaryMode(LINEARINTERP<float>::CONSTANTSLOPE);
		
		QT_CHECK_CLOSE(l.Interpolate(1),0,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(0),-1,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(2.5),1.5,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(2.75),1.75,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(3),2,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(3.5),2.5,0.0001);
	}
	
	{
		LINEARINTERP <float> l;
		l.AddPoint(2,1);
		l.AddPoint(3,2);
		
		QT_CHECK_CLOSE(l.Interpolate(1),1,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(0),1,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(2.5),1.5,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(2.75),1.75,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(3),2,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(3.5),2,0.0001);
	}
	
	{
		LINEARINTERP <float> l;
		l.AddPoint(2,1);
		l.AddPoint(1,3);
		l.AddPoint(3,4);
		
		l.SetBoundaryMode(LINEARINTERP<float>::CONSTANTSLOPE);
		
		QT_CHECK_CLOSE(l.Interpolate(1),3,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(0),5,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(-1),7,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(2.5),2.5,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(3),4,0.0001);
		QT_CHECK_CLOSE(l.Interpolate(3.5),5.5,0.0001);
	}
}
