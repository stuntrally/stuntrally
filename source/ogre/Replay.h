#pragma once
#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
#include <OgreVector3.h>
#include "half.hpp"
class CAR;
using half_float::half;

//  replay load log and check
#define LOG_RPL


//  NEW replays since 2.6
//  variable header and frame sizes
//----------------------------------------------------------------------------------------

/// size of ReplayFrame2
//= 280 Bytes per frame (minimal,approx)
//  280 * 80 fps = 22.4 kB/s
//  1 min = 1.34 MB, 10 min = 13.4 MB

struct ReplayHeader2
{
	char head[5];  // "SR\^ "
	int ver;

	std::string track;   // track name
	char track_user;  // user/original
	
	short numPlayers;
	std::vector<std::string> cars;  // car names eg. ES

	//std::vector<float> hue,sat,val;  // cars colors
	std::vector<std::string> nicks;  // multiplayer nicks

	float trees;      // trees multipler
	char num_laps;
	char networked;   // if 1, was networked, so use nicks when playing
	std::string sim_mode;  // easy, normal, etc

	ReplayHeader2();
	void Default();
};

//  car data, for each simulation frame
//--------------------------------------------
struct ReplayFrame2
{
	typedef unsigned char uchar;
	typedef unsigned short ushort;

	//  time  since game start
	float time;  //double

	//  car
	MATHVECTOR<float,3> pos;
	QUATERNION<float> rot;
	//MATHVECTOR<float,3> posEngn;  //snd engine pos --
	
	//char numWheels;
	struct RFlags  // bit fields
	{
		uchar numWheels :3;  //max 8
		uchar gear :4;  //max 16
		uchar braking :1;  //0,1 rear car lights

		uchar hasScrap :1;  //0 means scrap and screech are 0.
		uchar hasHit :1;    //1 means new hit data (colliding)
	} fl;
	
	struct RWheel
	{	//  wheel
		MATHVECTOR<float,3> pos;
		QUATERNION<half> rot;

		//  wheel trails, particles, snd
		/*struct RWhMtr
		{
			uchar surfType :3;  //3-
			uchar whTerMtr :3;  //3-
			uchar whRoadMtr :3;  //3-
		};/**/
		char surfType, whTerMtr;  //TRACKSURFACE::TYPE
		char whRoadMtr;
		char whP;  //particle type

		half squeal, slide, whVel;
		half suspVel, suspDisp;

		//  fluids
		uchar whH;  // submerge height
		half whAngVel;
		half whSteerAng;
	};
	std::vector<RWheel> wheels;

	//  hud
	half rpm,vel;
	uchar damage, clutch;
	half percent;  // track % val

	//  sound, input
	uchar throttle, steer, fboost;
	half speed, dynVel;

	//  hit continuous
	struct RScrap
	{
		half fScrap, fScreech;
	};
	std::vector<RScrap> scrap;
	
	//  hit impact, sparks
	struct RHit
	{
		half fHitTime, fParIntens,fParVel;//, fSndForce, fNormVel;
		Ogre::Vector3 vHitPos,vHitNorm;  // world hit data
		half whMudSpin, fHitForce;
		float hov_roll;  //=sph_yaw for O
	};
	std::vector<RHit> hit;
	
	ReplayFrame2();
	//void FromCar(const CAR* pCar);
	void FromOld(const struct ReplayFrame& fr);  //..
};


//  OLD replays, const size
//-----------------------------------------------------------------------------------------------------------

const static int ciRplHdrSize = 1024;
const static int cDefSize = 8*1024;
const static int ciTrkHdrSize = 32;


// note: add new vars always at end of
// ReplayHeader or ReplayFrame structs (backward compatibility)

/// size of ReplayFrame Old
//= 400 Bytes per frame
//  400 * 160 fps = 64 kB/s
//  1 min = 3.84 MB, 10 min = 38.4 MB


//  Replay  whole replay/ghost data - max 4 players, max 4 wheels
//--------------------------------------------
struct ReplayHeader
{
	char head[5];  // "SR\_ "
	typedef char ChName[32];

	char track[63];   // track name  (hmap crc? diff-)
	char track_user;  // user/original
	ChName car;       // car name  (eg. ES, .car file crc?, settings diff-)

	int ver, frameSize;  // bin data format - sizeof(ReplayFrame)
	float whR[4][4];  // cars wheels radius
	
	int numPlayers;
	float hue[4],sat[4],val[4];  // cars colors
	ChName cars[3];   // car names (when numPlayers > 1)

	ChName nicks[4];  // multiplayer nicks
	char descr[128];  // description - user text
	float trees;      // trees multipler
	char num_laps;
	char networked;   // if 1, was networked, so use nicks when playing
	char sim_mode[32];// easy, normal, etc

	ReplayHeader();
	void Default(), SafeEnd0();
};

//  Replay  car data, for each simulation frame
//--------------------------------------------
struct ReplayFrame
{
	//  time  since game start
	double time;
	//  car & wheels
	MATHVECTOR<float,3> pos, whPos[4];
	QUATERNION<float> rot, whRot[4];

	//  hud
	float rpm,vel, clutch;  int gear;  //char

	//  sound, input
	float throttle, steer;
	MATHVECTOR<float,3> posEngn;  //snd engine pos --
	float speed, dynVel;

	//  wheel trails, particles, snd
	char surfType[4], whTerMtr[4];  //TRACKSURFACE::TYPE
	float squeal[4], slide[4], whVel[4];
	float suspVel[4], suspDisp[4];
	
	float fboost;  // input, particles
	char whRoadMtr[4];
	
	//  fluids
	float whH[4], whAngVel[4];  // submerge height
	char whP[4];  //particle type
	float whSteerAng[2];
	
	float percent;  // track % val
	char braking;  // for rear car lights (bool)

	//  hit sparks
	float fHitTime, fParIntens,fParVel;//, fSndForce, fNormVel;
	Ogre::Vector3 vHitPos,vHitNorm;  // world hit data
	float whMudSpin, fHitForce, fCarScrap, fCarScreech;
	float hov_roll;  //=sph_yaw for O
	
	//uchar damage;  //todo..

	ReplayFrame();
	void FromCar(const CAR* pCar);
};

//  Replay
//--------------------------------------------
class Replay
{
public:
	Replay();
	
	bool LoadFile(std::string file, bool onlyHdr=false);
	bool SaveFile(std::string file);

	void AddFrame(const ReplayFrame& frame, int carNum);    // record
	bool GetFrame(double time, ReplayFrame* fr, int carNum);  // play
	const ReplayFrame& GetFrame0(int id){  return frames[0][id];  }

	const double GetTimeLength(int carNum=0) const;  // total time in seconds
	const int GetNumFrames() const {  return frames[0].size();  }

	//  inits only basic header data, fill the rest after
	void InitHeader(const char* track, bool trk_user, const char* car, bool bClear);
	void Clear();  // call this after header.numPlayers change
	void CopyFrom(const Replay& rpl);
	void DeleteFrames(int carNum, double fromTime);

	static bool fixOldTrkName(std::string& trk);  // old

	ReplayHeader header;
	//ReplayHeader2 header;
private:
	typedef std::vector<ReplayFrame> Frames;  // 1 player
	std::vector<Frames> frames;  // all plrs
};



//  Rewind  only data for car sim, to rewind back
//-----------------------------------------------------------------------------------------------------------
struct RewindFrame
{
	//  time  since game start
	double time;
	//  car
	MATHVECTOR<float,3> pos, vel, angvel;
	QUATERNION<float> rot;
	float fDamage, hov_roll;
	//engine rpm?..
};


///  Rewind
//  to move car back in time, not saved in file, can have less frames than replay
//--------------------------------------------
class Rewind
{
public:
	Rewind();

	void AddFrame(const RewindFrame& frame, int carNum);
	bool GetFrame(double time, RewindFrame* fr, int carNum);

	const double GetTimeLength(int carNum=0) const;  // total time in seconds

	void Clear();
private:
	std::vector<RewindFrame> frames[4];  // 4 players max (split screen)
	int idLast[4];  // last index from GetFrame (optym)
};



//  Track's ghost  reduced data
//----------------------------------------------------------------------------------------
struct TrackFrame  // for game
{
	//  time  since game start
	float time;
	//  car,  no wheels
	MATHVECTOR<float,3> pos;
	QUATERNION<float> rot;  //<half>?

	//  info
	char brake, steer;
	//short vel;  char gear;
	
	TrackFrame();
};

//  Track's ghost header
//--------------------------------------------
struct TrackHeader
{
	int ver;
	int frameSize;

	TrackHeader();
	void Default();
};

///  Track's ghost
//--------------------------------------------
class TrackGhost
{
public:
	TrackGhost();

	bool LoadFile(std::string file);
	bool SaveFile(std::string file);

	void AddFrame(const TrackFrame& frame);
	bool GetFrame(float time, TrackFrame* fr);

	const float GetTimeLength() const;
	void Clear();

	TrackHeader header;
//private:
	std::vector<TrackFrame> frames;
	int idLast;  // last index from GetFrame
	
	//  test only
	int getNumFrames();
	const TrackFrame& getFrame0(int id);
};
