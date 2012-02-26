#include "pch.h"
#include "ReplayGame.h"
//#include "../vdrift/settings.h"


//  header
//----------------------------------------------------------------
ReplayHeader::ReplayHeader()
{
	Default();
}
void ReplayHeader::Default()
{
	head[0] = 'S';  head[1] = 'R';  head[2] = '\\';  head[3] = '_';  head[4] = ' ';

	memset(track, 0, sizeof(track));  track_user = 0;
	memset(car, 0, sizeof(car));

	ver = 6;  /// todo: if <= 3 car colors from -1..1 to 0..1
	frameSize = sizeof(ReplayFrame);
	numPlayers = 1;
	trees = 1.f;

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
//----------------------------------------------------------------
bool Replay::LoadFile(std::string file, bool onlyHdr)
{
	std::ifstream fi(file.c_str(), std::ios::binary | std::ios::in);
	if (!fi)  return false;
	
	//  header
	char buf[ciRplHdrSize];  memset(buf,0,ciRplHdrSize);
	fi.read(buf,ciRplHdrSize);
	memcpy(&header, buf, sizeof(ReplayHeader));
	header.numPlayers = std::max(1, std::min(4, header.numPlayers));  // range 1..4
	
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
	frames[0].reserve(cDefSize);  //?
	while (!fi.eof())
	{
		for (int p=0; p < header.numPlayers; ++p)
		{
			ReplayFrame fr;
			fi.read((char*)&fr, header.frameSize/**/);
			frames[p].push_back(fr);
		}
		//LogO(toStr((float)fr.time) /*+ "  p " + toStr(fr.pos)*/);
	}
    fi.close();
    return true;
}

///  Save
//----------------------------------------------------------------
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
