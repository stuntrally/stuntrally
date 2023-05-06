#pragma once
#include "../vdrift/mathvector.h"
#include "../vdrift/quaternion.h"
#include <Ogre.h>
// #include <OgreVector3.h>
#include "half.hpp"
// #include <OgrePrerequisites.h>  //
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

	float trees;      // trees multiplier
	char num_laps;
	char networked;   // if 1, was networked, so use nicks when playing
	std::string sim_mode;  // easy, normal, etc

	std::vector<std::string> nicks;  // multiplayer nicks
	//std::vector<float> hue,sat,val,gloss,refl;  // cars colors?
	//char trackreverse;  // todo:

	ReplayHeader2();
	void Default(), SetHead();
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

	ReplayHeader2 header;
private:
	typedef std::vector<ReplayFrame2> Frames;  // 1 player
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
