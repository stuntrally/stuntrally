#include "pch.h"
#include "Replay.h"
#include "common/Def_Str.h"
#include <OgreTimer.h>
#include <string>


/*TODO:
	conv old frame to new
	conv tool for all
	load old rpl as new
	play new rpl game
	car frame as new rpl
	save new rpls
	only new rpl use
/**/
			
//  header
//----------------------------------------------------------------
ReplayHeader2::ReplayHeader2()
{
	Default();
}
void ReplayHeader2::Default()
{
	head[0] = 'S';  head[1] = 'R';  head[2] = '/';  head[3] = '^';  head[4] = 0;

	ver = 21;
	numPlayers = 1;
	trees = 1.f;
	num_laps = 1;  networked = 0;

	//for (int c=0; c<4; ++c)
	//{
	//	for (int w=0; w<4; ++w)
	//		whR[c][w] = 0.3f;
	//	hue[c] = 0.45f;  sat[c] = 0.035f;  val[c] = -0.07f;
	//}
}

void ReplayHeader2::FromOld(const struct ReplayHeader& h)
{
	track = h.track;
	track_user = h.track_user;
	
	numPlayers = h.numPlayers;
	cars.resize(numPlayers);

	cars[0] = h.car;
	int i;
	for (i=1; i < numPlayers; ++i)
		cars[i] = h.cars[i-1];

	for (i=0; i < numPlayers; ++i)
		nicks[i] = h.nicks[i];

	trees = h.trees;
	num_laps = h.num_laps;
	networked = h.networked;
	sim_mode = h.sim_mode;
}

ReplayFrame2::ReplayFrame2() //:
	////:time(0.0)
	////QUATERNION<float> rot, whRot[4];
	////float rpm,vel, clutch;  int gear;
	//throttle(0)//, steer
	////MATHVECTOR<float,3> posEngn;
	////float speed, dynVel;
	////char surfType[4], whTerMtr[4];  //TRACKSURFACE::TYPE
	////float squeal[4], slide[4], whVel[4];
	////float suspVel[4], suspDisp[4];
	//,fboost(0), percent(0.f), braking(0)
	//,fHitTime(0.f), fParIntens(0.f),fParVel(0.f)
	////Vector3 vHitPos,vHitNorm;
	//,whMudSpin(0.f), fHitForce(0.f), fCarScrap(0.f), fCarScreech(0.f)
	//,hov_roll(0.f)
{
	//pos[0]=0.f;  pos[1]=0.f;  pos[2]=0.f;  //whPos[4]
	//whSteerAng[0]=whSteerAng[1]=0.f;
	//for (int w=0; w<4; ++w)
	//{
	//	whH[w] = 0.f;  whAngVel[w] = 0.f;  whP[w] = 0;
	//	whRoadMtr[w] = 0;
	//}	
}

///  convert old frame to new
void ReplayFrame2::FromOld(const struct ReplayFrame& f)
{
	time = f.time;  // save once..

	pos = f.pos;
	rot = f.rot;
	
	//char numWheels;
	fl.numWheels = 4;  //todo: 0 for O,V1..

	fl.gear = f.gear;
	/*struct RFlags  // bit fields
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
		struct RWhMtr
		{
			uchar surfType :3;  //3-
			uchar whTerMtr :3;  //3-
			uchar whRoadMtr :3;  //3-
		};
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
	std::vector<RHit> hit;*/
}



Replay2::Replay2()
{
	Clear();
}

//  Init  once per game
void Replay2::InitHeader(const char* track, bool trk_user, const char* car, bool bClear)
{
	header.Default();
	header.track = track;  header.track_user = trk_user ? 1 : 0;
	//header.car = car;
	if (bClear)
		Clear();
}
void Replay2::Clear()
{
	frames.resize(header.numPlayers);
	for (int p=0; p < header.numPlayers; ++p)
	{	frames[p].clear();
		frames[p].reserve(cDefSize);
	}
}


///  Load
//-------------------------------------------------------------------------------------------------------------------------
bool Replay2::LoadFile(std::string file, bool onlyHdr)
{
	std::ifstream fi(file.c_str(), std::ios::binary | std::ios::in);
	if (!fi)  return false;

	Ogre::Timer ti;
	
	//  header
	//char buf[ciRplHdrSize];  memset(buf,0,ciRplHdrSize);
	//fi.read(buf,ciRplHdrSize);
	//memcpy(&header, buf, sizeof(ReplayHeader2));
	//header.numPlayers = std::max(1, std::min(4, header.numPlayers));  // range 1..4
	//header.SafeEnd0();

    #ifdef LOG_RPL
		if (!onlyHdr)
			LogO(">- Load replay --  file: "+file+"  players:"+toStr(header.numPlayers));
	#endif
	
	//  clear
	Clear();
	#if 0
	//  only get last frame for time len info, ?save len in hdr..
	/*if (onlyHdr)
	{
		//fi.seekg(-header.frameSize * header.numPlayers, std::ios::end);

		for (int p=0; p < header.numPlayers; ++p)
		{
			ReplayFrame fr;
			fi.read((char*)&fr, header.frameSize);
			frames[p].push_back(fr);
		}
	    fi.close();
	    return true;
	}*/
	
	//  frames
		//min,max
		float c = 10e12, d = -10e12;
		ReplayFrame a,b;  //min,max
		#define n(par)  a.par = c;  b.par = d;
		n(fCarScrap)  n(fCarScreech)
		n(fHitForce)  n(fHitTime)
		n(fParIntens)  n(fParVel)
		n(squeal[0])  n(slide[0])
		n(suspVel[0])  n(suspDisp[0])

	int i=0,p;
	while (!fi.eof())
	{

		for (p=0; p < header.numPlayers; ++p)
		{
			ReplayFrame f;
			fi.read((char*)&f, header.frameSize/**/);

			if (i > 0 && f.time < frames[p][i-1].time)
			{
			    #ifdef LOG_RPL
					LogO(">- Load replay  BAD frame time  id:"+toStr(i)+"  plr:"+toStr(p)
						+"  t-1:"+fToStr(frames[p][i-1].time,5,7)+" > t:"+fToStr(f.time,5,7));
				#endif
			}else
			{
				#define m(par)  if (f.par < a.par)  a.par = f.par;  if (f.par > b.par)  b.par = f.par;
				m(fCarScrap)  m(fCarScreech)
				m(fHitForce)  m(fHitTime)
				m(fParIntens)  m(fParVel)
				m(squeal[0])  m(slide[0])
				m(suspVel[0])  m(suspDisp[0])
				
				frames[p].push_back(f);
			}
		}
		++i;
	}
    fi.close();

    #ifdef LOG_RPL
		LogO(">- Load replay  first: "+fToStr(frames[0][0].time,5,7)
			+"  time: "+fToStr(GetTimeLength(0),2,5)+"  frames: "+toStr(frames[0].size()));
	#endif

	LogO(Ogre::String("::: Time ReplayLoad: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	#endif
	return true;
}

///  Save
//-------------------------------------------------------------------------------------------------------------------------
bool Replay2::SaveFile(std::string file)
{
	std::ofstream of(file.c_str(), std::ios::binary | std::ios::out);
	if (!of)  return false;

	//  header
	char buf[ciRplHdrSize];  memset(buf,0,ciRplHdrSize);
	memcpy(buf, &header, sizeof(ReplayHeader));
	of.write(buf,ciRplHdrSize);

	//  frames
	int s = frames[0].size(), i,p;

	for (i=0; i < s; ++i)
	for (p=0; p < header.numPlayers; ++p)
		of.write((char*)&frames[p][i], sizeof(ReplayFrame));

    of.close();
    return true;
}

//  add (Record)
void Replay2::AddFrame(const ReplayFrame2& frame, int carNum)
{
	if (frame.time > GetTimeLength(carNum))  // dont add before last
		frames[carNum].push_back(frame);
}

//  CopyFrom
void Replay2::CopyFrom(const Replay2& rpl)
{
	Clear();
	for (int i=0; i < rpl.GetNumFrames(); ++i)
		frames[0].push_back(rpl.frames[0][i]);
}

//  last frame time, sec
const double Replay2::GetTimeLength(int carNum) const
{
	if (carNum >= frames.size())  return 0.0;
	int s = frames[carNum].size();
	return s > 0 ? frames[carNum][s-1].time : 0.0;
}

///  get (Play)
//----------------------------------------------------------------
bool Replay2::GetFrame(double time, ReplayFrame2* pFr, int carNum)
{
	static int ic = 0;  // last index for current frame

	int s = frames[carNum].size();
	if (ic > s-1)  ic = s-1;  // new size
	if (s < 2)  return false;  // empty

	///  find which frame for given time
	//if (ic+2 == s)  return false;  // end

	//  search up
	while (ic+1 < s-1 && frames[carNum][ic+1].time <= time)  ++ic;
	//  search down
	while (ic > 0     && frames[carNum][ic].time > time)  --ic;

	if (ic < 0 || ic >= s)
		return false;  //-
	
	///  simple, no interpolation
	#if 1
	if (pFr)
		*pFr = frames[carNum][ic];
	#else
	//  linear interp ..
	if (pFr)
	{
		//  cur <= time < next
		const ReplayFrame& fc = frames[ic], fn = frames[ic+1];

		float m = (time - fc.time) / (fn.time - fc.time);  // [0..1]
		//Ogre::LogManager::getSingleton().logMessage(toStr(m));

		ReplayFrame fr = fc;
		fr.pos = (fn.pos - fc.pos) * m + fc.pos;

		for (int w=0; w<4; ++w)
		{
			fr.whPos[w] = (fn.whPos[w] - fc.whPos[w]) * m + fc.whPos[w];
		//Quaternion q;  q.Slerp(m, fc.rot, fn.rot);
		}
		//... all data
		*pFr = fr;
	}
	#endif

	//  last time
	double end = frames[carNum][s-1].time;
	if (time >= end)
	{
		pFr->fboost = 0.f;
		//  clear emitters at end
		for (int w=0; w < 4; ++w)
		{
			//pFr->slide[w] = 0.f;
			//pFr->squeal[w] = 0.f;
			//pFr->whVel[w] = 0.f;
		}
	}
	
	//  check if ended
	return time <= end;
}

//  delete frames after current time (when time did go back)
void Replay2::DeleteFrames(int c, double fromTime)
{
	if (frames[c].empty())  return;
	while (!frames[c].empty() && frames[c][ frames[c].size()-1].time >= fromTime)
		frames[c].pop_back();
}

