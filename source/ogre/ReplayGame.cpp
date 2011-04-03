#include "stdafx.h"
#include "ReplayGame.h"
//#include "../vdrift/settings.h"


///  Replay ...
//---------------------------------------------------------------------------------------------------------------

/*
struct ReplayFrame
{
	double time;	// time from start
	Vector3 pos, whPos[4];		// car pos
	Quaternion rot, whRot[4];	// car rot
	float rpm, vel;  int gear;  // for hud and sound
};
*/

ReplayHeader::ReplayHeader()
{
	head[0] = 'S';  head[1] = 'R';  head[2] = 'r';  head[3] = 'p';  head[4] = 'l';
	ver = 1;
	frameSize = sizeof(ReplayFrame);
	memset(track, 0, sizeof(track));
	memset(car, 0, sizeof(car));
}

Replay::Replay()
{
	frames.reserve(10000);  //
}

void Replay::LoadFile(string file)
{
	ifstream f(file.c_str(), std::ios_base::binary | std::ios_base::in);
}

void Replay::SaveFile(string file)
{
	ofstream of(file.c_str(), std::ios_base::binary | std::ios_base::out);
}

void Replay::AddFrame(const ReplayFrame& frame)
{
	frames.push_back(frame);
}
