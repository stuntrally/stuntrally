#pragma once

class ENGINESOUNDINFO
{
	public:
		float minrpm, maxrpm, naturalrpm, fullgainrpmstart, fullgainrpmend;
		enum
		{
			POWERON,
			POWEROFF,
			BOTH
		} power;

		ENGINESOUNDINFO() : minrpm(1.0), maxrpm(100000.0), naturalrpm(7000.0),fullgainrpmstart(minrpm),fullgainrpmend(maxrpm),power(BOTH) {}

		bool operator < (const ENGINESOUNDINFO & other) const {return minrpm < other.minrpm;}
};
