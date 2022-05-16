#include "pch.h"
#include "Replay.h"
#include "common/Def_Str.h"
#include <OgreTimer.h>
#include <string>
#include "CHud.h" //
using namespace std;
using namespace Ogre;


//  header
//----------------------------------------------------------------
ReplayHeader2::ReplayHeader2()
{
	Default();
}
void ReplayHeader2::SetHead()
{
	head[0] = 'S';  head[1] = 'R';  head[2] = '/';  head[3] = '^';  head[4] = 0;
}
void ReplayHeader2::Default()
{
	SetHead();
	track = "";  track_user = 0;  //trackreverse = 0;
	ver = 30;  time = -0.01f;
	
	numPlayers = 0;  trees = 1.f;
	num_laps = 1;  networked = 0;
	sim_mode = "";

	cars.clear();  numWh.clear();  nicks.clear();
}

ReplayFrame2::ReplayFrame2()
	:gear(0),fl(0)  //..
{
}

Replay2::Replay2()
	:idLast(0)
{
	Clear();
}

//  Init  once per game
void Replay2::InitHeader(const char* track, bool trk_user, bool bClear)
{
	header.Default();
	header.track = track;  header.track_user = trk_user ? 1 : 0;
	if (bClear)
		Clear();
}

void Replay2::ClearCars()
{
	int pp = header.numPlayers;
	header.cars.clear();  header.cars.resize(pp);
	header.nicks.clear();  header.nicks.resize(pp);
	header.numWh.clear();  header.numWh.resize(pp);
}

void Replay2::Clear(bool time)
{
	idLast = 0;
	if (time)
		header.time = -0.01f;  // so new 0.0 counts

	int p,pp = header.numPlayers;
	frames.resize(pp);
	for (p=0; p < pp; ++p)
	{	frames[p].clear();
		frames[p].reserve(8*1024);
	}
}


///  Load
//-------------------------------------------------------------------------------------------------------
bool Replay2::LoadFile(string file, bool onlyHdr)
{
	ifstream fi(file.c_str(), ios::binary | ios::in);
	if (!fi)  return false;

	Ogre::Timer ti;
	
	//  header check
	fi.read(header.head,5);
	if (strcmp(header.head,"SR\\_")==0)
	{
		LogO(">- Load replay2: "+file+"  Error: Old format not supported");
		return false;
	}
	if (strcmp(header.head,"SR/^")!=0)
	{
		LogO(">- Load replay2: "+file+"  Error: Unknown header");
		return false;
	}
	
	//  load new
	uchar l;  int i,s;  char buf[256];
	#define rd(a)  fi.read((char*)&a, sizeof(a))
	#define rs(s)  {  fi.read((char*)&l, 1);  if (l>0)  fi.read(buf, l);  buf[l]=0;  s = buf;  }  //string
	//TODO: endianness, swap 2bytes..  ENDIAN_SWAP_16

	//  header  ------
	ReplayHeader2& h = header;
	rd(h.ver);  rd(h.time);
	rs(h.track)  rd(h.track_user);
	String ss = ">- Load replay2  ";
	ss += h.track+"  ";
	ss += StrTime(h.time)+"  ";

	rd(h.numPlayers);  s = h.numPlayers;
	h.cars.clear();  h.cars.resize(s);
	h.numWh.clear();  h.numWh.resize(s);
	for (i=0; i < s; ++i)
	{	rs(h.cars[i])  ss += h.cars[i]+" ";  }  ss+=" ";
	for (i=0; i < s; ++i)
	{	rd(h.numWh[i]);  ss += toStr(h.numWh[i])+" ";  }  ss+=" ";

	rd(h.trees);  rd(h.num_laps);
	rd(h.networked);  rs(h.sim_mode)

	h.nicks.clear();  h.nicks.resize(s);
	if (h.networked)
		for (i=0; i < s; ++i)
		{	rs(h.nicks[i])  ss += h.nicks[i]+" ";  }  ss+=" ";

	#ifdef LOG_RPL
		LogO(ss);
		//if (!onlyHdr)
		//	LogO(">- Load replay2: "+file+"  players:"+toStr(h.numPlayers));
	#endif
	
	Clear(false);  //  clear

	if (onlyHdr){	fi.close();  return true;  }
	
	//  frames  ------
	i=0;  int p,w;  float prevTime = -1.f;
	while (!fi.eof())
	{
		float time;  rd(time);  // once
		for (p=0; p < header.numPlayers; ++p)
		{
			ReplayFrame2 f;
			f.time = time;
			//  car
			rd(f.pos);  rd(f.rot);  rd(f.fl);  //b_braking etc
			//  hud
			rd(f.gear);  rd(f.rpm);  rd(f.vel);
			rd(f.damage);  rd(f.clutch);  rd(f.percent);
			//  sound, input
			rd(f.throttle);  rd(f.steer);  rd(f.fboost);
			rd(f.speed);  rd(f.dynVel);
			//  ext
			if (f.get(b_hov))  rd(f.hov_roll);
			bool flu = f.get(b_fluid);
			if (flu)  rd(f.whMudSpin);
			
			//  wheels
			int ww = header.numWh[p];
			for (w=0; w < ww; ++w)
			{
				RWheel wh;
				rd(wh.pos);  rd(wh.rot);
				//  trl, par, snd
				rd(wh.surfType);  rd(wh.whTerMtr);
				rd(wh.whRoadMtr);  rd(wh.whP);
				//  tire
				rd(wh.squeal);  rd(wh.slide);  rd(wh.whVel);
				rd(wh.suspVel);  rd(wh.suspDisp);
				//  fluids
				if (flu)  rd(wh.whH);
				rd(wh.whAngVel);  rd(wh.whSteerAng);
				f.wheels.push_back(wh);
			}

			//  hit data
			if (f.get(b_scrap))
			{
				RScrap s;
				rd(s.fScrap);  rd(s.fScreech);
				f.scrap.push_back(s);
			}
			rd(f.fHitTime);
			if (f.get(b_hit))
			{
				RHit h;
				rd(h.fHitForce);  rd(h.fParIntens);  rd(h.fParVel);
				rd(h.vHitPos.x);   rd(h.vHitPos.y);   rd(h.vHitPos.z);
				rd(h.vHitNorm.x);  rd(h.vHitNorm.y);  rd(h.vHitNorm.z);
				f.hit.push_back(h);
			}

			if (time <= prevTime)
			{
				#ifdef LOG_RPL
				LogO(">- Load replay2  =time  id:"+toStr(i)+"  plr:"+toStr(p)+"  t-1:"+fToStr(prevTime,5,7)+" => t:"+fToStr(time,5,7));
				#endif
			}else
			if (!fi.eof())
				frames[p].push_back(f);
		}
		++i;  prevTime = time;
	}
	fi.close();

    #ifdef LOG_RPL
		if (frames.empty() || frames[0].empty())
			LogO(">- Load replay2  empty!!  time: "+fToStr(GetTimeLength(),2,5));
		else
			LogO(">- Load replay2  plr: "+toStr(header.numPlayers)+"  t1st: "+fToStr(frames[0][0].time,5,7)+
				"  time: "+fToStr(GetTimeLength(),2,5)+"  frames: "+toStr(frames[0].size()));
	#endif

	LogO(String("::: Time Replay2 Load: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	return true;
}

///  Save
//-------------------------------------------------------------------------------------------------------
bool Replay2::SaveFile(string file)
{
	ofstream of(file.c_str(), ios::binary | ios::out);
	if (!of)  return false;

	uchar l;  int i,s;
	#define ws(s)  {  l = s.length();  of.write((char*)&l, 1);  of.write(s.c_str(), l);  }  //string
	#define wr(a)  of.write((char*)&a, sizeof(a))
	//todo: portability, endianness for shorts?..

	//  header  ------
	const ReplayHeader2& h = header;
	header.SetHead();
	of.write((char*)&h.head, 5);
	wr(h.ver);  wr(h.time);
	ws(h.track)  wr(h.track_user);

	wr(h.numPlayers);
	s = h.numPlayers;  // car names
	for (i=0; i < s; ++i)  ws(h.cars[i])
	for (i=0; i < s; ++i)  wr(h.numWh[i]);

	wr(h.trees);  wr(h.num_laps);
	wr(h.networked);  ws(h.sim_mode)

	if (h.networked)
		for (i=0; i < s; ++i)  ws(h.nicks[i])

	
	//  frames  ------
	s = frames[0].size();  int p,w;
	//s = 1;  p = 1;  //test

	for (i=0; i < s; ++i)
	{
		float time = frames[0][i].time;
		wr(time);  // once
		for (p=0; p < header.numPlayers; ++p)
		{
			ReplayFrame2 f = frames[p][i];
			//  car
			wr(f.pos);  wr(f.rot);  wr(f.fl);  //b_braking etc
			//  hud
			wr(f.gear);  wr(f.rpm);  wr(f.vel);
			wr(f.damage);  wr(f.clutch);  wr(f.percent);
			//  sound, input
			wr(f.throttle);  wr(f.steer);  wr(f.fboost);
			wr(f.speed);  wr(f.dynVel);
			//  ext
			if (f.get(b_hov))  wr(f.hov_roll);
			bool flu = f.get(b_fluid);
			if (flu)  wr(f.whMudSpin);
			
			//  wheels
			int ww = f.wheels.size();
			for (w=0; w < ww; ++w)
			{
				const RWheel& wh = f.wheels[w];
				wr(wh.pos);  wr(wh.rot);
				//  trl, par, snd
				wr(wh.surfType);  wr(wh.whTerMtr);
				wr(wh.whRoadMtr);  wr(wh.whP);
				//  tire
				wr(wh.squeal);  wr(wh.slide);  wr(wh.whVel);
				wr(wh.suspVel);  wr(wh.suspDisp);
				//  fluids
				if (flu)  wr(wh.whH);
				wr(wh.whAngVel);  wr(wh.whSteerAng);
			}

			//  hit data
			if (f.get(b_scrap) /*&& scrap.size()==1*/)
			{
				const RScrap& s = f.scrap[0];
				wr(s.fScrap);  wr(s.fScreech);
			}
			wr(f.fHitTime);
			if (f.get(b_hit) /*&& hit.size()==1*/)
			{
				const RHit& h = f.hit[0];
				wr(h.fHitForce);  wr(h.fParIntens);  wr(h.fParVel);
				wr(h.vHitPos.x);   wr(h.vHitPos.y);   wr(h.vHitPos.z);
				wr(h.vHitNorm.x);  wr(h.vHitNorm.y);  wr(h.vHitNorm.z);
			}
		}
	}

    of.close();
    return true;
}
//-------------------------------------------------------------------------------------------------------


//  add (Record)
void Replay2::AddFrame(const ReplayFrame2& frame, int carNum)
{
	if (carNum > 0 || frame.time > GetTimeLength())  // dont add before last
	{
		frames[carNum].push_back(frame);
		header.time = frame.time;
	}
}

//  CopyFrom
void Replay2::CopyFrom(const Replay2& rpl)
{
	header.numPlayers = rpl.header.numPlayers;
	Clear();
	header.time = rpl.header.time;

	//  plr 1 only, for ghost
	for (int i=0; i < rpl.GetNumFrames(); ++i)
		frames[0].push_back(rpl.frames[0][i]);
}

//  last frame time, sec
const float Replay2::GetTimeLength() const
{
	return header.time;
}

//  get Last
bool Replay2::GetLastFrame(ReplayFrame2* pFr, int carNum)
{
	int s = frames[carNum].size();
	if (s < 2)  return false;  // empty

	*pFr = frames[carNum][s-1];
	return false;
}

half Replay2::GetLastHitTime(int carNum)
{
	int s = frames[carNum].size();
	if (s < 2)  return half(0.f);  // empty

	return frames[carNum][s-1].fHitTime;
}


///  get (Play)
//----------------------------------------------------------------
bool Replay2::GetFrame(float time1, ReplayFrame2* pFr, int carNum)
{
	if (frames.empty())  return false;
	int& ic = idLast;  // last index

	int s = frames[carNum].size();
	if (ic > s-1)  ic = s-1;  // new size
	if (s < 2)  return false;  // empty

	///  find which frame for given time
	float time = std::min(time1, GetTimeLength());
	while (ic+1 < s-1 && frames[carNum][ic+1].time <= time)  ++ic;
	while (ic > 0     && frames[carNum][ic].time > time)  --ic;

	if (ic < 0 || ic >= s)
		return false;  //-


	if (ic == 0 || ic == s-1)
		*pFr = frames[carNum][ic];
	else
	{	///  linear interpolation
		const ReplayFrame2& t1 = frames[carNum][ic];  //cur
		const ReplayFrame2& t0 = frames[carNum][std::max(0, ic-1)];  //prev
		*pFr = frames[carNum][ic];  // rest, no interp

		float dt = t1.time - t0.time;
		if (dt > 0.0001f)
		{
			float f = (time - t0.time) / dt;
			(*pFr).pos = t0.pos + (t1.pos - t0.pos) * f;
			(*pFr).rot = t0.rot.QuatSlerp(t1.rot, f);
		
			int w, ww = t0.wheels.size();
			for (w=0; w < ww; ++w)
			{
				(*pFr).wheels[w].pos = t0.wheels[w].pos + (t1.wheels[w].pos - t0.wheels[w].pos) * f;
				//(*pFr).wheels[w].rot = t0.wheels[w].rot.QuatSlerp(t1.wheels[w].rot, f);  // no need
			}
	}	}

	//  last time
	float end = GetTimeLength();
	if (time1 >= end)
	{
		pFr->fboost = 0.f;
		//  clear emitters at end..
		int w, ww = (*pFr).wheels.size();
		for (w=0; w < ww; ++w)
		{
			RWheel& wh = (*pFr).wheels[w];
			wh.slide = wh.squeal = wh.whVel = half(0.f);
		}
	}
	
	//  check if ended
	return time1 <= end;
}

//  delete frames after current time (when time did go back)
void Replay2::DeleteFrames(int c, float fromTime)
{
	if (frames[c].empty())  return;
	while (!frames[c].empty() && frames[c][ frames[c].size()-1 ].time >= fromTime)
		frames[c].pop_back();
	header.time = fromTime;
}

