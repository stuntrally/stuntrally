#include "pch.h"
#include "Replay.h"


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
