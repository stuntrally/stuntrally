#ifndef _QTimer_H_
#define _QTimer_H_


class QTimer
{
	private:
		double cc,fq, t,st, dt1,st1, iFR;
		LARGE_INTEGER CC;

	public:
		double dt, FR, iv, iv1;
		// delta time, FrameRate, interval, intervalFR

		QTimer();
		bool update();
};

#endif
