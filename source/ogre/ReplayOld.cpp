#include "pch.h"
#include "Replay.h"
#include "common/Def_Str.h"
#include <OgreTimer.h>
#include <string>


//  header
//----------------------------------------------------------------
ReplayHeader::ReplayHeader()
{
	Default();
}
void ReplayHeader::Default()
{
	head[0] = 'S';  head[1] = 'R';  head[2] = '\\';  head[3] = '_';  head[4] = 0;

	memset(track, 0, sizeof(track));  track_user = 0;
	memset(car, 0, sizeof(car));

	ver = 10;
	frameSize = sizeof(ReplayFrame);
	numPlayers = 1;
	trees = 1.f;
	num_laps = 1;  networked = 0;

	for (int c=0; c<4; ++c)
	{
		for (int w=0; w<4; ++w)
			whR[c][w] = 0.3f;
		hue[c] = 0.45f;  sat[c] = 0.035f;  val[c] = -0.07f;
	}
	memset(cars, 0, sizeof(cars));
	memset(nicks, 0, sizeof(nicks));		
	memset(descr, 0, sizeof(descr));
	memset(sim_mode, 0, sizeof(sim_mode));
}

void ReplayHeader::SafeEnd0()
{	// put 0 on last char for strings (loading older replay)
	track[62]=0;
	car[31]=0;      cars[0][31]=0;  cars[1][31]=0;  cars[2][31]=0;
	nicks[0][31]=0; nicks[1][31]=0; nicks[2][31]=0; nicks[3][31]=0;
	descr[127]=0;  sim_mode[31]=0;
}

ReplayFrame::ReplayFrame() :
	//:time(0.0)
	//QUATERNION<float> rot, whRot[4];
	//float rpm,vel, clutch;  int gear;
	throttle(0)//, steer
	//MATHVECTOR<float,3> posEngn;
	//float speed, dynVel;
	//char surfType[4], whTerMtr[4];  //TRACKSURFACE::TYPE
	//float squeal[4], slide[4], whVel[4];
	//float suspVel[4], suspDisp[4];
	,fboost(0), percent(0.f), braking(0)
	,fHitTime(0.f), fParIntens(0.f),fParVel(0.f)
	//Vector3 vHitPos,vHitNorm;
	,whMudSpin(0.f), fHitForce(0.f), fCarScrap(0.f), fCarScreech(0.f)
	,hov_roll(0.f)
{
	pos[0]=0.f;  pos[1]=0.f;  pos[2]=0.f;  //whPos[4]
	whSteerAng[0]=whSteerAng[1]=0.f;
	for (int w=0; w<4; ++w)
	{
		whH[w] = 0.f;  whAngVel[w] = 0.f;  whP[w] = 0;
		whRoadMtr[w] = 0;
	}	
}	


Replay::Replay()
{
	Clear();
}

//  Init  once per game
void Replay::InitHeader(const char* track, bool trk_user, const char* car, bool bClear)
{
	header.Default();
	strcpy(header.track, track);  header.track_user = trk_user ? 1 : 0;
	strcpy(header.car, car);
	if (bClear)
		Clear();
}
void Replay::Clear()
{
	frames.resize(header.numPlayers);
	for (int p=0; p < header.numPlayers; ++p)
	{	frames[p].clear();
		frames[p].reserve(cDefSize);
	}
}


///  Load
//-------------------------------------------------------------------------------------------------------------------------
bool Replay::LoadFile(std::string file, bool onlyHdr)
{
	std::ifstream fi(file.c_str(), std::ios::binary | std::ios::in);
	if (!fi)  return false;

	Ogre::Timer ti;
	
	//  header
	char buf[ciRplHdrSize];  memset(buf,0,ciRplHdrSize);
	fi.read(buf,ciRplHdrSize);
	memcpy(&header, buf, sizeof(ReplayHeader));
	header.numPlayers = std::max(1, std::min(4, header.numPlayers));  // range 1..4
	header.SafeEnd0();

	//  old fixes--
	if (header.ver < 9)
		header.sim_mode[0]=0;  // versions below 9 have no sim mode
	if (header.ver <= 9)
	if (!header.track_user)  // ver below 2.5
	{
		std::string trk = header.track;
		fixOldTrkName(trk);
		strcpy(header.track, trk.c_str());
	}

    #ifdef LOG_RPL
		if (!onlyHdr)
			LogO(">- Load replay --  file: "+file+"  players:"+toStr(header.numPlayers));
		if (!onlyHdr)  
		for (int p=0; p < header.numPlayers; ++p)
		{
			if (p==0)  {
				//LogO(Ogre::String(">- Load replay  nick:")+header.nicks[p]+"  car:"+header.car);
			}else{
				// versions below 8 had wrong nicks and cars in header for more than 1 player
				if (strlen(header.cars[p]) < 2)
					strcpy(header.cars[p], "3S");
				//LogO(Ogre::String(">- Load replay  nick:")+header.nicks[p]+"  car:"+Ogre::String(&header.cars[0][p]));
			}
		}
	#endif
	
	//  clear
	Clear();
	
	//  only get last frame for time len info, ?save len in hdr..
	if (onlyHdr)
	{
		fi.seekg(-header.frameSize * header.numPlayers, std::ios::end);

		for (int p=0; p < header.numPlayers; ++p)
		{
			ReplayFrame fr;
			fi.read((char*)&fr, header.frameSize);
			frames[p].push_back(fr);
		}
	    fi.close();
	    return true;
	}
	
	//  frames
	//#define RPL_ST  // log stats
	//#define RPL2_SV  // save new rpl test

	#ifdef RPL_ST  // min,max
		float c = 10e12, d = -10e12;
		ReplayFrame a,b;  //min,max
		#define n(par)  a.par = c;  b.par = d;
		n(fCarScrap)  n(fCarScreech)
		n(fHitForce)  n(fHitTime)
		n(fParIntens)  n(fParVel)
		n(squeal[0])  n(slide[0])  n(whVel[0])
		n(suspVel[0])  n(suspDisp[0])
	#endif
	
	#ifdef RPL2_SV 
		Replay2 r2;
		r2.header.FromOld(header);
	#endif
		
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
				frames[p].push_back(f);

				#ifdef RPL_ST  // min,max
				#define m(par)  if (f.par < a.par)  a.par = f.par;  if (f.par > b.par)  b.par = f.par;
				m(fCarScrap)  m(fCarScreech)
				m(fHitForce)  m(fHitTime)
				m(fParIntens)  m(fParVel)
				m(squeal[0])  m(slide[0])  m(whVel[0])
				m(suspVel[0])  m(suspDisp[0])
				
				/*LogO(
				" whP "+	iToStr(f.whP[0])+  " su "+	iToStr(f.surfType[0])+
				" m "+	iToStr(f.whTerMtr[0])+  " r "+	iToStr(f.whRoadMtr[0])
				);/**/
				LogO(
				/*" Scrp "+	fToStr(f.fCarScrap,		2,5)+  " Scre "+	fToStr(f.fCarScreech,	2,5)+
				" HitF "+	fToStr(f.fHitForce,		2,5)+  " HitT "+	fToStr(f.fHitTime,		2,5)+
				" PInt "+	fToStr(f.fParIntens,	2,5)+  " PVel "+	fToStr(f.fParVel,		2,5)+
				/**/
				/*" sql "+	fToStr(f.squeal[0],		2,5)+  " sli "+	fToStr(f.slide[0],		2,5)+
				" whV "+	fToStr(f.whVel[0],		2,5)+
				" ssv "+	fToStr(f.suspVel[0],	2,5)+  " ssd "+	fToStr(f.suspDisp[0],	2,5)+
				/**/
				" whH "+	fToStr(f.whH[0],		2,5)+
				" whAv "+	fToStr(f.whAngVel[0],	2,5)+
				" wSt "+	fToStr(f.whSteerAng[0],	2,5)+
				" mud "+	fToStr(f.whMudSpin,	2,5)+
				" hov "+	fToStr(f.hov_roll,	2,5)+  //=sph_yaw for O
				/**/
				"");
				#endif
				
				#ifdef RPL2_SV 
				ReplayFrame2 f2;
				f2.FromOld(f);
				r2.AddFrame(f2,p);
				#endif
			}
		}
		++i;
		//LogO(toStr((float)fr.time) /*+ "  p " + toStr(fr.pos)*/);
	}
    fi.close();
		
	#ifdef RPL2_SV 
		r2.SaveFile(file+"2");  // test
	#endif

	#ifdef RPL_ST  // min,max
		LogO("MIN\n"
		" Scrp "+	fToStr(a.fCarScrap,		2,5)+  " Scre "+	fToStr(a.fCarScreech,	2,5)+
		" HitF "+	fToStr(a.fHitForce,		2,5)+  " HitT "+	fToStr(a.fHitTime,		2,5)+
		" PInt "+	fToStr(a.fParIntens,	2,5)+  " PVel "+	fToStr(a.fParVel,		2,5)+

		" sql "+	fToStr(a.squeal[0],		2,5)+  " sli "+	fToStr(a.slide[0],		2,5)+
		" whV "+	fToStr(a.whVel[0],		2,5)+
		" ssv "+	fToStr(a.suspVel[0],	2,5)+  " ssd "+	fToStr(a.suspDisp[0],	2,5)+
		"");

		LogO("MAX\n"
		" Scrp "+	fToStr(b.fCarScrap,		2,5)+  " Scre "+	fToStr(b.fCarScreech,	2,5)+
		" HitF "+	fToStr(b.fHitForce,		2,5)+  " HitT "+	fToStr(b.fHitTime,		2,5)+
		" PInt "+	fToStr(b.fParIntens,	2,5)+  " PVel "+	fToStr(b.fParVel,		2,5)+

		" sql "+	fToStr(b.squeal[0],		2,5)+  " sli "+	fToStr(b.slide[0],		2,5)+
		" whV "+	fToStr(b.whVel[0],		2,5)+
		" ssv "+	fToStr(b.suspVel[0],	2,5)+  " ssd "+	fToStr(b.suspDisp[0],	2,5)+
		"\n");
	#endif

	// half
	//float ff = 0.9999999;  half h = half(ff);  float fh = float(h);
	//LogO(fToStr(ff,8,12)+" "+fToStr(h,8,12)+" "+fToStr(fh,8,12));
 
    #ifdef LOG_RPL
		LogO(">- Load replay  first: "+fToStr(frames[0][0].time,5,7)
			+"  time: "+fToStr(GetTimeLength(0),2,5)+"  frames: "+toStr(frames[0].size()));
	#endif

	LogO(Ogre::String("::: Time ReplayLoad: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	return true;
}

///  Save
//-------------------------------------------------------------------------------------------------------------------------
bool Replay::SaveFile(std::string file)
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
void Replay::AddFrame(const ReplayFrame& frame, int carNum)
{
	if (frame.time > GetTimeLength(carNum))  // dont add before last
		frames[carNum].push_back(frame);
}

//  CopyFrom
void Replay::CopyFrom(const Replay& rpl)
{
	Clear();
	for (int i=0; i < rpl.GetNumFrames(); ++i)
		frames[0].push_back(rpl.frames[0][i]);
}

//  last frame time, sec
const double Replay::GetTimeLength(int carNum) const
{
	if (carNum >= frames.size())  return 0.0;
	int s = frames[carNum].size();
	return s > 0 ? frames[carNum][s-1].time : 0.0;
}

///  get (Play)
//----------------------------------------------------------------
bool Replay::GetFrame(double time, ReplayFrame* pFr, int carNum)
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
			pFr->slide[w] = 0.f;
			pFr->squeal[w] = 0.f;
			pFr->whVel[w] = 0.f;
		}
	}
	
	//  check if ended
	return time <= end;
}

//  delete frames after current time (when time did go back)
void Replay::DeleteFrames(int c, double fromTime)
{
	if (frames[c].empty())  return;
	while (!frames[c].empty() && frames[c][ frames[c].size()-1].time >= fromTime)
		frames[c].pop_back();
}

