#pragma once
#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
#include <OgreVector3.h>
#include "half.hpp"
#include <OgrePrerequisites.h>  //
using Ogre::uchar;
using Ogre::ushort;
class CAR;
using half_float::half;

//  replay load log and check
#define LOG_RPL


//  NEW replays since 2.6
//  variable header and frame sizes
//----------------------------------------------------------------------------------------

/// size of ReplayFrame2
//= 202 Bytes per frame (minimal,approx)
//  202 * 80 fps = 16.1 kB/s
//  1 min = 970 kB, 10 min = 9.7 MB

struct ReplayHeader2
{
	char head[5];  // "SR/^ "
	short ver;
	float time;  // total time

	std::string track;   // track name
	char track_user;  // user/original
	
	char numPlayers;
	std::vector<std::string> cars;  // car names eg. ES
	std::vector<uchar> numWh;  //wheels count for all cars

	float trees;      // trees multipler
	char num_laps;
	char networked;   // if 1, was networked, so use nicks when playing
	std::string sim_mode;  // easy, normal, etc

	std::vector<std::string> nicks;  // multiplayer nicks
	//std::vector<float> hue,sat,val,gloss,refl;  // cars colors?

	ReplayHeader2();
	void Default(), SetHead();
	void FromOld(const struct ReplayHeader& hdr);
};

//  car data, for each simulation frame
//--------------------------------------------
enum eFlags {  b_braking=0, b_scrap, b_hit, b_fluid, b_hov };  // max 8

//  wheel
struct RWheel
{
	MATHVECTOR<float,3> pos;
	QUATERNION<half> rot;

	//  trails, particles, snd
	char surfType, whTerMtr;  //TRACKSURFACE::TYPE
	char whRoadMtr, whP;  //particle type

	//  tire
	half squeal, slide, whVel;
	half suspVel, suspDisp;

	//  fluids
	uchar whH;  // /var - submerge
	half whAngVel;
	half whSteerAng;  // 38B
};

struct RScrap
{
	half fScrap, fScreech;  // 4B
};

struct RHit
{
	half fHitForce, fParIntens, fParVel;
	Ogre::Vector3 vHitPos, vHitNorm;  // world hit data
};  // 30B

struct ReplayFrame2
{
	//dont use int, not portable

	//  time  since game start
	float time;

	//  car
	MATHVECTOR<float,3> pos;
	QUATERNION<float> rot;
	// 32B  (size in Bytes so far)
	
	// cant use bit fields, not portable
	uchar fl;  // flags, bool 1bit
	void set(eFlags e, bool b) {  if (b)  fl |= 1 << e;  else  fl &= ~(1 << e); }
	bool get(eFlags e) const   {  return ((fl >> e) & 1u) > 0;  }
	// 35B

	//  hud
	char gear;
	half rpm,vel;
	uchar damage, clutch;
	uchar percent;  // track % val
	// 41B

	//  sound, input
	uchar throttle, fboost;  char steer;
	half speed, dynVel;
	// 48B

	//  ext
	half whMudSpin;  //-2 /var - may not be saved, check flags
	half hov_roll;  //-2 /var - V1 roll_ang or sph_yaw for O


	//  wheel
	std::vector<RWheel> wheels;  // 38B  vec-24B

	//  hit continuous
	std::vector<RScrap> scrap;  // 4B
	
	//  hit impact, sparks
	half fHitTime;
	std::vector<RHit> hit;  // 30B  saved on hit only
	// 50B

	
	ReplayFrame2();
	void FromCar(const CAR* pCar, half prevHitTime);
	void FromOld(const struct ReplayFrame& fr, uchar numWh, half prevHitTime);

	//total: 50B + 4*38B = 202B min
};

//  Replay
//--------------------------------------------
class Replay2
{
public:
	Replay2();
	
	bool LoadFile(std::string file, bool onlyHdr=false);
	bool SaveFile(std::string file);

	void AddFrame(const ReplayFrame2& frame, int carNum);    // record
	bool GetFrame(float time, ReplayFrame2* fr, int carNum);  // play
	const ReplayFrame2& GetFrame0(int id){  return frames[0][id];  }

	const float GetTimeLength() const;  // total time in seconds
	const int GetNumFrames() const {  return frames.empty() ? 0 : frames[0].size();  }

	bool GetLastFrame(ReplayFrame2* pFr, int carNum);
	half GetLastHitTime(int carNum);

	//  inits only basic header data, fill the rest after
	void InitHeader(const char* track, bool trk_user, bool bClear);
	void Clear(bool time=true);  // call this after header.numPlayers change
	void ClearCars();

	void CopyFrom(const Replay2& rpl);
	void DeleteFrames(int carNum, float fromTime);

	static bool fixOldTrkName(std::string& trk);  // old

	ReplayHeader2 header;
private:
	typedef std::vector<ReplayFrame2> Frames;  // 1 player
	std::vector<Frames> frames;  // all plrs
	int idLast;  // last index from GetFrame
};


//  OLD replays, const size
//-----------------------------------------------------------------------------------------------------------

const static int ciRplHdrSize = 1024;
const static int cDefSize = 8*1024;


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
	float whR[4][4];  // cars wheels radius (not used now)
	
	int numPlayers;
	float hue[4],sat[4],val[4];  // cars colors
	ChName cars[3];   // car names (when numPlayers > 1)

	ChName nicks[4];  // multiplayer nicks
	char descr[128];  // description - user text (wasnt used)
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
	float fHitTime, fParIntens,fParVel;
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
	friend class Replay2;
	typedef std::vector<ReplayFrame> Frames;  // 1 player
	std::vector<Frames> frames;  // all plrs
	int idLast;  // last index from GetFrame
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
	int idLast[4];  // last index from GetFrame (optimisation)
};
