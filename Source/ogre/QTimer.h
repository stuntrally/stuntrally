#ifndef _QTimer_H_
#define _QTimer_H_

#include <OgrePlatform.h>

class QTimer
{
	private:
		double cc,fq, t,st, dt1,st1, iFR;
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		LARGE_INTEGER CC;
#endif

	public:
		double dt, FR, iv, iv1;
		// delta time, FrameRate, interval, intervalFR

		QTimer();
		bool update();
};

#endif
