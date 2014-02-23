#pragma once
#include <OgrePlatform.h>

#if OGRE_PLATFORM != OGRE_PLATFORM_WIN32
	#include <sys/time.h>
#else
	#include <windows.h>
#endif


class QTimer
{
	private:
		double cc,fq, t,st, dt1,st1, iFR;
		bool first;
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			LARGE_INTEGER CC;
		#else
			timespec startTime;
		#endif

	public:
		double dt, FR, iv, iv1;
		// delta time, FrameRate, interval, intervalFR

		QTimer(bool upd = true);  // update in ctor

		bool update();  // returns false if delta time didnt reach interval yet
		float get();  // update and return dt in milisec (just for measure)
};
