#include "pch.h"
#include "timer.h"


bool TIMER::Load(const std::string & trackrecordspath, float stagingtime)
{
	Unload();

	car.clear();

	pretime = stagingtime;

	trackrecordsfile = trackrecordspath;

	trackrecords.Load(trackrecordsfile);

	loaded = true;

	return true;
}

void TIMER::Unload()
{
	if (loaded)
	{
		trackrecords.Write(true, trackrecordsfile);
		//std::cout << "Writing records to: " << trackrecordsfile << endl;
	}
	trackrecords.Clear();
	loaded = false;
	
	netw_lap = 1;  // reset, expect 1st lap
}

void TIMER::Tick(float dt)
{
	if (pretime > 0.f && !waiting)
	{	pretime -= dt;
		dt = 0.f;
	}
	if (waiting)
		dt = 0.f;

	for (auto& l : car)
		l.Tick(dt);
}

bool TIMER::Lap(const int carId, const bool countit, bool bTrackReverse)
{
	//assert(carId < car.size());
	if (carId >= car.size())  return false;  //-
	bool newbest = false;  // new lap best time

	if (countit)
	{
		std::stringstream secstr;
		secstr << "sector 0";
		std::string lastcar;
		/*if (trackrecords.GetParam("last.car", lastcar))
		{
			if (lastcar != car[carId].GetCarType()) //clear last lap time
			trackrecords.SetParam("last.sector 0", (float)0.0);
		}*/
		trackrecords.SetParam(std::string(bTrackReverse ? "rev_" : "") + "last." + secstr.str(), (float) car[carId].GetTime());
		trackrecords.SetParam(std::string(bTrackReverse ? "rev_" : "") + "last.car", car[carId].GetCarType());

		float prevbest = 0;
		bool haveprevbest = trackrecords.GetParam(
				car[carId].GetCarType() + (bTrackReverse ? "_rev" : "") + "." + secstr.str(), prevbest);
		if (car[carId].GetTime() < prevbest || !haveprevbest)
		{
			trackrecords.SetParam(
				car[carId].GetCarType() + (bTrackReverse ? "_rev" : "") + "." + secstr.str(), (float) car[carId].GetTime());
			newbest = true;
		}
	}

	car[carId].Lap(countit);
	if (loaded)
		trackrecords.Write(true, trackrecordsfile);

	return newbest;
}


bool TIMER::LapNetworkTime(const int carId, int lap, const double curtime)
{
	//if (lap == netw_lap)  // deny same lap..
	if (curtime > 4.f)  // sec
	{
		car[carId].LapWithTime(true, curtime);
		
		++netw_lap;  // allow only once per lap
		return true;
	}
	return false;
}
