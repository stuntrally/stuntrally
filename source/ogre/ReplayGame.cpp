#include "pch.h"
#include "Replay.h"
#include "common/Def_Str.h"
#include <OgreTimer.h>
#include <string>


///  Rewind
//-------------------------------------------------------------------------------------------------
Rewind::Rewind()
{
	Clear();
}

void Rewind::Clear()
{
	for (int p=0; p < 4; ++p)
	{
		frames[p].clear();
		idLast[p] = 0;
	}
}

//  last frame time, sec
const double Rewind::GetTimeLength(int carNum) const
{
	int s = frames[carNum].size();
	return s > 0 ? frames[carNum][s-1].time : 0.0;
}

//  add (Record)
void Rewind::AddFrame(const RewindFrame& frame, int carNum)
{
	//  remove later frames
	while (frames[carNum].size() > 0 &&
		frames[carNum][ frames[carNum].size()-1 ].time >= frame.time)
		frames[carNum].pop_back();

	frames[carNum].push_back(frame);
}

//  get (Play)
//----------------------------------------------------------------
bool Rewind::GetFrame(double time, RewindFrame* pFr, int carNum)
{
	int& ic = idLast[carNum];  // last index

	int s = frames[carNum].size();
	if (ic > s-1)  ic = s-1;  // new size
	if (s < 2)  return false;  // empty

	//  find which frame for given time
	while (ic+1 < s-1 && frames[carNum][ic+1].time <= time)  ++ic;
	while (ic > 0     && frames[carNum][ic].time > time)  --ic;

	if (ic < 0 || ic >= s)
		return false;  //-
	
	//  simple, no interpolation
	*pFr = frames[carNum][ic];

	//  last time
	double end = frames[carNum][s-1].time;
	
	//  check if ended
	return time <= end;
}


///  Track's ghost
//-------------------------------------------------------------------------------------------------
TrackHeader::TrackHeader()
{
	Default();
}

void TrackHeader::Default()
{
	ver = 0;
	frameSize = sizeof(TrackFrame);
}


TrackFrame::TrackFrame()
	: time(0.f)
{	}

TrackGhost::TrackGhost()
	: idLast(0)
{	}


void TrackGhost::AddFrame(const TrackFrame& frame)
{
	if (frame.time > GetTimeLength())  // dont add before last
		frames.push_back(frame);
}

bool TrackGhost::GetFrame(float time1, TrackFrame* pFr)
{
	int& ic = idLast;  // last index

	int s = frames.size();
	if (ic > s-1)  ic = s-1;  // new size
	if (s < 2)  return false;  // empty

	float time = std::min(time1, GetTimeLength());

	//  find which frame for given time
	while (ic+1 < s-1 && frames[ic+1].time <= time)  ++ic;
	while (ic > 0     && frames[ic].time > time)  --ic;

	if (ic < 0 || ic >= s)
		return false;  //-
	
	if (ic == 0 || ic == s-1)
		*pFr = frames[ic];
	else
	{	///  linear interpolation
		const TrackFrame& t1 = frames[ic];  //cur
		const TrackFrame& t0 = frames[std::max(0, ic-1)];  //prev
		//*pFr = frames[ic];
		float f = (time - t0.time) / (t1.time - t0.time);
		(*pFr).pos = t0.pos + (t1.pos - t0.pos) * f;
		(*pFr).rot = t0.rot.QuatSlerp(t1.rot, f);
		//(*pFr).rot = t1.rot;
		(*pFr).brake = t1.brake;
		(*pFr).steer = t1.steer;
	}

	//  last time
	double end = frames[s-1].time;
	
	//  check if ended
	return time <= end;
}

const float TrackGhost::GetTimeLength() const
{
	int s = frames.size();
	return s > 0 ? frames[s-1].time : 0.f;
}

void TrackGhost::Clear()
{
	frames.clear();
	idLast = 0;
}

//  test
int TrackGhost::getNumFrames()
{
	return (int)frames.size();
}
const TrackFrame& TrackGhost::getFrame0(int id)
{
	return frames[id];
}


///  Load
//-------------------------------------------------------------------------------------------------
bool TrackGhost::LoadFile(std::string file)
{
	std::ifstream fi(file.c_str(), std::ios::binary | std::ios::in);
	if (!fi)  return false;
	Ogre::Timer ti;
	
	//  header
	char buf[ciTrkHdrSize];  memset(buf,0,ciTrkHdrSize);
	fi.read(buf, ciTrkHdrSize);
	memcpy(&header, buf, sizeof(TrackHeader));
	
	LogO(">- Load trk ghost --  file: "+file);

	Clear();
	
	//  frames
	int i=0;
	frames.reserve(cDefSize/2);  //?
	while (!fi.eof())
	{
		TrackFrame fr;
		fi.read((char*)&fr, header.frameSize/**/);

		if (i > 0 && fr.time <= frames[i-1].time)
		{
			#ifdef LOG_RPL
				LogO(">- Load trk ghost  BAD frame time  id:"+toStr(i)
					+"  t-1:"+fToStr(frames[i-1].time,5,7)+" >= t:"+fToStr(fr.time,5,7));
			#endif
		}else
			frames.push_back(fr);
		++i;
		//LogO(toStr((float)fr.time) /*+ "  p " + toStr(fr.pos)*/);
	}
    fi.close();
 
    #ifdef LOG_RPL
		LogO(">- Load trk ghost   first: "+fToStr(frames[0].time,5,7)
			+"  time: "+fToStr(GetTimeLength(),2,5)+"  frames: "+toStr(frames.size()));
	#endif

	LogO(Ogre::String("::: Time Load trk ghost: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
    return true;
}

///  Save
//-------------------------------------------------------------------------------------------------
bool TrackGhost::SaveFile(std::string file)
{
	std::ofstream of(file.c_str(), std::ios::binary | std::ios::out);
	if (!of)  return false;

	//  header
	char buf[ciTrkHdrSize];  memset(buf,0,ciTrkHdrSize);
	memcpy(buf, &header, sizeof(TrackHeader));
	of.write(buf, ciTrkHdrSize);

	//  frames
	int s = frames.size();
	for (int i=0; i < s; ++i)
		of.write((char*)&frames[i], sizeof(TrackFrame));

    of.close();
    return true;
}
