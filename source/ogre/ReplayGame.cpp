#include "stdafx.h"
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

	ver = 2;
	frameSize = sizeof(ReplayFrame);
	numPlayers = 1;

	for (int c=0; c<4; ++c)
	{
		for (int w=0; w<4; ++w)
			whR[c][w] = 0.3f;
		hue[c] = 0.45f;  sat[c] = 0.035f;  val[c] = -0.07f;
		if (c < 3)
			memset(cars[c], 0, sizeof(car));
	}
	memset(descr, 0, sizeof(descr));
}

Replay::Replay()
{
	frames.reserve(cDefSize);  //
}

//  Init  once per game
void Replay::InitHeader(const char* track, bool trk_user, const char* car, bool bClear)
{
	header.Default();
	strcpy(header.track, track);  header.track_user = trk_user ? 1 : 0;
	strcpy(header.car, car);
	if (bClear)
	{	frames.clear();  frames.reserve(cDefSize);	}
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
	
	frames.clear();
	//  only get last frame for time len info, ?save len in hdr..
	if (onlyHdr)
	{
		fi.seekg(-header.frameSize, std::ios::end);

		ReplayFrame fr;
		fi.read((char*)&fr, header.frameSize/**/);
		frames.push_back(fr);

	    fi.close();
	    return true;
	}
	
	//  frames
	frames.reserve(cDefSize);
	while (!fi.eof())
	{
		ReplayFrame fr;
		fi.read((char*)&fr, header.frameSize/**/);
		frames.push_back(fr);
		//Log(toStr((float)fr.time) /*+ "  p " + toStr(fr.pos)*/);
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
	int s = frames.size();
	for (int i=0; i < s; ++i)
		of.write((char*)&frames[i], sizeof(ReplayFrame));

    of.close();
    return true;
}

//  add (Record)
void Replay::AddFrame(const ReplayFrame& frame)
{
	if (frame.time > GetTimeLength())  // dont add before last
		frames.push_back(frame);
}

//  last frame time, sec
float Replay::GetTimeLength()
{
	int s = frames.size();
	if (s > 0)
		return frames[s-1].time;
	else
		return 0.f;
}

///  get (Play)
//----------------------------------------------------------------
bool Replay::GetFrame(double time, ReplayFrame* pFr)
{
	static int ic = 0;  // last index for current frame

	int s = frames.size();
	if (ic > s-1)  ic = s-1;  // new size
	if (s < 2)  return false;  // empty

	///  find which frame for given time
	//if (ic+2 == s)  return false;  // end

	//  search up
	while (ic+1 < s-1 && frames[ic+1].time <= time)  ++ic;
	//  search down
	while (ic > 0     && frames[ic].time > time)  --ic;

	/*for (i=0; i < s-1; ++i)  // easiest, bad--
		if (frames[i+1].time > time)
		{	ic = i;  break;  }/**/
	
	///  simple, no interpolation
	#if 1
	if (pFr)
		*pFr = frames[ic];
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

	//  last time  check if ended
	if (time <= frames[s-1].time)
		return true;
	else
		return false;
}
