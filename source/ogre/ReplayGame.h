#ifndef _Replay_h_
#define _Replay_h_

#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"

const static int ciRplHdrSize = 1024;
const static int cDefSize = 8*1024;


//  replay/ghost data

struct ReplayHeader
{
	char head[5];  // "SR\_ "

	char track[63];   // track name  (hmap crc? diff-)
	char track_user;  // user/original
	char car[32];     // car name  (.car file crc?, settings diff-)

	int ver, frameSize;  // bin data format - sizeof(ReplayFrame)
	float whR[4][4];  // cars wheels radius
	
	int numPlayers;
	float hue[4],sat[4],val[4];  // cars colors
	char cars[32][3];  // car names (when numPlayers > 1)
	char descr[128];   // description - user text

	// custom replay fps 60, interpolation ...
	ReplayHeader();  void Default();
};


//  bin data, for each frame

struct ReplayFrame
{
	//  time  since game start
	double time;
	//  car & wheels
	MATHVECTOR <float,3> pos, whPos[4];
	QUATERNION <float> rot, whRot[4];

	//  hud
	float rpm,vel, clutch;  int gear;  //char

	//  sound, input
	float throttle, steer;
	MATHVECTOR <float,3> posEngn;  //snd engine pos --
	float speed, dynVel;

	//  wheel trails, particles, snd
	char surfType[4], whTerMtr[4];  //TRACKSURFACE::TYPE
	float squeal[4], slide[4], whVel[4];
	float suspVel[4], suspDisp[4];
	
	float fboost;  // input, particles
	char whRoadMtr[4];

	/// sizeof:  tm 8 car 12*5 wh 16*5 h 16 snd 16 12 whtr 8 16*5
	/// = 280 Bytes per frame
	//  280 * 160 fps = 44.8 kB/s
	//  1min = 2,69 MB, 10 min = 26,9 MB
};


class Replay
{
public:
	Replay();
	
	bool LoadFile(std::string file, bool onlyHdr=false);
	bool SaveFile(std::string file);

	void AddFrame(const ReplayFrame& frame, int carNum);    // record
	bool GetFrame(double time, ReplayFrame* fr, int carNum);  // play

	const float GetTimeLength(int carNum=0) const;  // total time in seconds
	const int GetNumFrames() const {  return frames[0].size();  }

	//  inits only basic header data, fill the rest after
	void InitHeader(const char* track, bool trk_user, const char* car, bool bClear);
	void Clear();
	void CopyFrom(const Replay& rpl);

	ReplayHeader header;
private:
	std::vector<ReplayFrame> frames[4];
};

#endif
