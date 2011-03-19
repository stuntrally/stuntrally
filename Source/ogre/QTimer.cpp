#include "stdafx.h"
#include "QTimer.h"


//  ___ ___ ___ ___  Timer class  ___ ___ ___ ___

QTimer::QTimer()
{
	iv = 0.;	iv1 = 0.5;
	
	LARGE_INTEGER FQ;
	if (QueryPerformanceFrequency( &FQ ))
		fq = double( FQ.QuadPart );
}

bool QTimer::update()
{
	QueryPerformanceCounter( &CC );
	cc = double( CC.QuadPart );
	t = cc / fq;
	
	dt = t - st;  // delta time
	if (dt < iv)  // interval
		return false;
	
	st = t;  // old time
	iFR++;  // frames count
	
	//  framerate update
	dt1 = t - st1;
	if (dt1 >= iv1)
	{
		FR = iFR / dt1;
		if (FR < 1.)  FR = 0.;
		iFR = 0.;
		st1 = t;
	}
	return true;
}
