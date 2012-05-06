#include "pch.h"
#include "ReplayGame.h"
#include "common/Defines.h"
#include "common/QTimer.h"

//  replay load log and check
#define LOG_RPL


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

	ver = 8;
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
}

void ReplayHeader::SafeEnd0()
{	// put 0 on last char for strings (loading older replay)
	track[62]=0;
	car[31]=0;      cars[0][31]=0;  cars[1][31]=0;  cars[2][31]=0;
	nicks[0][31]=0; nicks[1][31]=0; nicks[2][31]=0; nicks[3][31]=0;
	descr[127]=0;
}

ReplayFrame::ReplayFrame() :
	//:time(0.0)
	//MATHVECTOR <float,3> pos, whPos[4];
	//QUATERNION <float> rot, whRot[4];
	//float rpm,vel, clutch;  int gear;
	throttle(0)//, steer
	//MATHVECTOR <float,3> posEngn;
	//float speed, dynVel;
	//char surfType[4], whTerMtr[4];  //TRACKSURFACE::TYPE
	//float squeal[4], slide[4], whVel[4];
	//float suspVel[4], suspDisp[4];
	,fboost(0)
	,percent(0.f)
	,braking(false)
	,fHitTime(0.f), fParIntens(0.f),fParVel(0.f)
	//Vector3 vHitPos,vHitNorm;
	,whMudSpin(0.f), fHitForce(0.f)
{
	pos[0]=0.f;  pos[1]=0.f;  pos[2]=0.f;
	whSteerAng[0]=whSteerAng[1]=0.f;
	for (int w=0; w<4; ++w)
	{
		whH[w] = 0.f;  whAngVel[w] = 0.f;  whP[w] = 0;
		whRoadMtr[w] = 0;
	}	
}	


Replay::Replay()
{
	frames[0].reserve(cDefSize);  //
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
	for (int p=0; p < 4; ++p)
		frames[p].clear();
	for (int p=0; p < header.numPlayers; ++p)
		frames[p].reserve(cDefSize);
}


///  Load
//-------------------------------------------------------------------------------------------------------------------------
bool Replay::LoadFile(std::string file, bool onlyHdr)
{
	std::ifstream fi(file.c_str(), std::ios::binary | std::ios::in);
	if (!fi)  return false;

	QTimer ti;  ti.update();  /// time
	
	//  header
	char buf[ciRplHdrSize];  memset(buf,0,ciRplHdrSize);
	fi.read(buf,ciRplHdrSize);
	memcpy(&header, buf, sizeof(ReplayHeader));
	header.numPlayers = std::max(1, std::min(4, header.numPlayers));  // range 1..4
	header.SafeEnd0();

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
	for (int p=0; p < 4; ++p)
		frames[p].clear();
	
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
	int i=0;
	frames[0].reserve(cDefSize);  //?
	while (!fi.eof())
	{
		for (int p=0; p < header.numPlayers; ++p)
		{
			ReplayFrame fr;
			fi.read((char*)&fr, header.frameSize/**/);

		    #ifdef LOG_RPL
				if (i > 0 && fr.time < frames[p][i-1].time)
					LogO(">- Load replay  BAD frame time  id:"+toStr(i)+"  plr:"+toStr(p)
						+"  t-1:"+fToStr(frames[p][i-1].time,5,7)+" > t:"+fToStr(fr.time,5,7));
			#endif
			frames[p].push_back(fr);
		}
		++i;
		//LogO(toStr((float)fr.time) /*+ "  p " + toStr(fr.pos)*/);
	}
    fi.close();
 

    #ifdef LOG_RPL
		LogO(">- Load replay  first: "+fToStr(frames[0][0].time,5,7)
			+"  time: "+fToStr(GetTimeLength(0),5,7)+"  frames: "+toStr(frames[0].size()));
	#endif

	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO(Ogre::String("::: Time ReplayLoad: ") + toStr(dt) + " ms");

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
	int s = frames[0].size();

	for (int i=0; i < s; ++i)
	for (int p=0; p < header.numPlayers; ++p)
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
const float Replay::GetTimeLength(int carNum) const
{
	int s = frames[carNum].size();
	return s > 0 ? frames[carNum][s-1].time : 0.f;
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

	if (ic < 0 || ic >= frames[carNum].size())
		return false;  //-

	/*for (i=0; i < s-1; ++i)  // easiest, bad--
		if (frames[i+1].time > time)
		{	ic = i;  break;  }/**/
	
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
	if (time <= end)
		return true;
	else
		return false;
}
