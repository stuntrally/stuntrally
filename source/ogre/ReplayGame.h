#ifndef _Replay_h_
#define _Replay_h_

//#include "BaseApp.h"
using namespace Ogre;


/*  replay/ghost data  */
struct ReplayHeader
{
	char head[5];  //"SRrpl"
	char track[80];	// track name  (user/original, hmap crc? diff-)
	char car[32];	// car name  (.car file crc?, settings diff-)
	int ver, frameSize;  // bin data format - sizeof(ReplayFrame)
	//custom replay fps?
	ReplayHeader();
};

/*  bin data, for each frame */
struct ReplayFrame
{
	double time;	// time from start
	Vector3 pos, whPos[4];		// car pos
	Quaternion rot, whRot[4];	// car rot
	float rpm, vel;  int gear;  // for hud and sound

/*	3-x4 gain wheels sounds
	4-x4 particle emitters rate, slide */
};

class Replay
{
public:
	Replay();
	
	void LoadFile(string file), SaveFile(string file);

	void AddFrame(const ReplayFrame& frame);

private:
	ReplayHeader header;
	std::vector<ReplayFrame> frames;
};

/*
sum all
= 18 * 4B = 72 bytes per frame
> 72 * 160 fps = 11.5 kB/s
> 1min = 691 kB, 10 min = 7 MB

 saved in Game fps loop
 custom replay fps ?
 save/load in memory, disk ok

*/
#endif
