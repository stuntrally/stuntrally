#include "stdafx.h"
#include "QTimer.h"


//  ___ ___ ___ ___  Timer class  ___ ___ ___ ___

QTimer::QTimer()
{

	iv = 0.;	iv1 = 0.5;
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	LARGE_INTEGER FQ;
	if (QueryPerformanceFrequency( &FQ ))
		fq = double( FQ.QuadPart );
#else
	clock_gettime(CLOCK_MONOTONIC, &startTime);
#endif
}

bool QTimer::update()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
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
#else
	// FIXME: only supports dt calculation, nothing else
	timespec newt;
	clock_gettime(CLOCK_MONOTONIC, &newt);
	dt = (newt.tv_sec - startTime.tv_sec) + (newt.tv_nsec - startTime.tv_nsec) / 1000000000.0;
	startTime = newt;
#endif
	return true;
}
