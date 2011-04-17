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
	head[0] = 'S';  head[1] = 'R';  head[2] = 'r';  head[3] = 'p';  head[4] = 'l';
	ver = 1;
	frameSize = sizeof(ReplayFrame);
	memset(track, 0, sizeof(track));
	memset(car, 0, sizeof(car));
	for (int w=0; w<4; ++w)  whR[w] = 0.3f;
}

Replay::Replay()
{
	frames.reserve(cDefSize);  //
}

//  Init  once per game
void Replay::InitHeader(const char* track, const char* car, float* whR_4)
{
	header.Default();
	strcpy(header.track, track);
	strcpy(header.car, car);
	for (int w=0; w<4; ++w)  header.whR[w] = whR_4[w];
}

///  Load
//----------------------------------------------------------------
void Replay::LoadFile(std::string file)
{
	std::ifstream fi(file.c_str(), std::ios::binary | std::ios::in);
	if (!fi)  return;
	//  header
	char buf[ciRplHdrSize];  memset(buf,0,ciRplHdrSize);
	fi.read(buf,ciRplHdrSize);
	memcpy(&header, buf, sizeof(ReplayHeader));
	frames.clear();  frames.reserve(cDefSize);

	//  frames
	while (!fi.eof())
	{
		ReplayFrame fr;
		fi.read((char*)&fr, header.frameSize/**/);
		frames.push_back(fr);
		//Log(toStr((float)fr.time) /*+ "  p " + toStr(fr.pos)*/);
	}
    fi.close();
}

///  Save
//----------------------------------------------------------------
void Replay::SaveFile(std::string file)
{
	std::ofstream of(file.c_str(), std::ios::binary | std::ios::out);
	//  header
	char buf[ciRplHdrSize];  memset(buf,0,ciRplHdrSize);
	memcpy(buf, &header, sizeof(ReplayHeader));
	of.write(buf,ciRplHdrSize);

	//  frames
	int s = frames.size();
	for (int i=0; i < s; ++i)
		of.write((char*)&frames[i], sizeof(ReplayFrame));

    of.close();
}

//  add (Record)
void Replay::AddFrame(const ReplayFrame& frame)
{
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
