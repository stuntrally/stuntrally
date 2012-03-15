#include "pch.h"
#include "timer.h"
#include "unittest.h"

//#include <string>
//#include <sstream>

using std::string;
using std::endl;
using std::vector;
using std::stringstream;

bool TIMER::Load(const std::string & trackrecordspath, float stagingtime, std::ostream & error_output)
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
}

void TIMER::Tick(float dt)
{
	//float elapsed_time = dt;

	//pretime -= dt;
	//if (pretime > 0)
	//	elapsed_time -= dt;

	//if (pretime < 0)
	//	elapsed_time += -pretime;

	//assert(elapsed_time >= 0);

	if (pretime > 0.f && !waiting)
	{	pretime -= dt;
		dt = 0.f;  }
	if (waiting)
		dt = 0.f;

	for (std::vector <LAPINFO>::iterator i = car.begin(); i != car.end(); ++i)
		i->Tick(dt);
}

bool TIMER::Lap(const int carId, const int prevsector, const int nextsector, const bool countit, bool bTrackReverse)
{
	//assert(carId < car.size());
	if (carId >= car.size())  return false;  //-
	bool newbest = false;  // new lap best time

	if (countit)
	{
		std::stringstream secstr;
		secstr << "sector " << nextsector;
		string lastcar;
		/*if (trackrecords.GetParam("last.car", lastcar))
		{
			if (lastcar != car[carId].GetCarType()) //clear last lap time
			trackrecords.SetParam("last.sector 0", (float)0.0);
		}*/
		trackrecords.SetParam(string(bTrackReverse ? "rev_" : "") + "last." + secstr.str(), (float) car[carId].GetTime());
		trackrecords.SetParam(string(bTrackReverse ? "rev_" : "") + "last.car", car[carId].GetCarType());

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

	if (nextsector == 0)
	{
		car[carId].Lap(countit);
		if (loaded)
			trackrecords.Write(true, trackrecordsfile);
	}
	return newbest;
}

bool TIMER::LapNetworkTime(const int carId, const double curtime)
{
	car[carId].LapWithTime(true, curtime);
	return false;
}


class PLACE
{
    private:
        int index;
        int laps;
        double distance;

    public:
        PLACE(int newindex, int newlaps, double newdistance) : index(newindex), laps(newlaps), distance(newdistance) {}

        int GetIndex() const {return index;}

        bool operator< (const PLACE & other) const
        {
            if (laps == other.laps)
                return distance > other.distance;
            else
                return laps > other.laps;
        }
};

std::pair <int, int> TIMER::GetCarPlace(int index)
{
    assert(index<(int)car.size());
    assert(index >= 0);

    int place = 1;
    int total = car.size();

    std::list <PLACE> distances;

    for (int i = 0; i < (int)car.size(); i++)
    {
        distances.push_back(PLACE(i, car[i].GetCurrentLap(), car[i].GetLapDistance()));
    }

    distances.sort();

    int curplace = 1;
    for (std::list <PLACE>::iterator i = distances.begin(); i != distances.end(); ++i)
    {
        if (i->GetIndex() == index)
            place = curplace;

        curplace++;
    }

    return std::make_pair(place, total);
}
