#include "pch.h"
#include "ReplayTrk.h"
#include "common/Def_Str.h"
#include <OgreTimer.h>
#include <string>


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
	float end = frames[s-1].time;
	
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
bool TrackGhost::LoadFile(std::string file, bool bLog)
{
	std::ifstream fi(file.c_str(), std::ios::binary | std::ios::in);
	if (!fi)  return false;
	Ogre::Timer ti;
	
	//  header
	char buf[ciTrkHdrSize];  memset(buf,0,ciTrkHdrSize);
	fi.read(buf, ciTrkHdrSize);
	memcpy(&header, buf, sizeof(TrackHeader));
	
	if (bLog)  LogO(">- Load trk ghost --  file: "+file);

	Clear();
	
	//  frames
	int i=0;
	frames.reserve(4*1024);  //?
	while (!fi.eof())
	{
		TrackFrame fr;
		fi.read((char*)&fr, header.frameSize/**/);

		if (i > 0 && fr.time <= frames[i-1].time)
		{
			#ifdef LOG_RPL
				if (bLog)  LogO(">- Load trk ghost  BAD frame time  id:"+toStr(i)
					+"  t-1:"+fToStr(frames[i-1].time,5,7)+" >= t:"+fToStr(fr.time,5,7));
			#endif
		}else
			frames.push_back(fr);
		++i;
		//if (bLog)  LogO(toStr((float)fr.time) /*+ "  p " + toStr(fr.pos)*/);
	}
    fi.close();
 
    #ifdef LOG_RPL
		if (bLog)  LogO(">- Load trk ghost   first: "+fToStr(frames[0].time,5,7)
			+"  time: "+fToStr(GetTimeLength(),2,5)+"  frames: "+toStr(frames.size()));
	#endif

	if (bLog)  LogO(Ogre::String("::: Time Load trk ghost: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
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
