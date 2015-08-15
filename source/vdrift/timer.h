#pragma once
#include "configfile.h"
#include "dbl.h"


class TIMER
{
private:

	class LAPTIME
	{
		private:
			bool havetime;
			double time;

		public:
			LAPTIME()
			{	Reset();  }
			void Reset()
			{
				havetime = false;
				time = 0;
			}

			bool HaveTime() const {  return havetime;  }
			double GetTimeInSeconds() const {  return time;  }

			//  convert time in seconds into output min and secs
			void GetTimeInMinutesSeconds(float & secs, int & min) const
			{
				min = (int) time / 60;
				secs = time - min*60;
			}
			void Set(double newtime)
			{
				time = newtime;
				havetime = true;
			}
			//  only set the time if we don't have a time or if the new time is faster than the current time
			void SetIfFaster(double newtime)
			{
				if (!havetime || newtime < time)
					time = newtime;
				havetime = true;
			}
	};
	
	class LAPINFO
	{
		private:
			double time_rpl;  // time from race start (for replay)
			double time;      // lap time
			
			LAPTIME lastlap;  // last lap time
			LAPTIME bestlap;  // best lap time (also from local records)
			LAPTIME bestlapRace;  // best lap time, for current race only
			
			double totaltime; // total time of a race (>=1 laps)
			int num_laps;     // current lap
			std::string cartype;

		public:
			double time_rewind;  // time from race start (for rewind, goes back when rewinding)
			double time_rewGh;   // lap time for ghost (goes back when rewinding)
			LAPINFO(const std::string & newcartype) : cartype(newcartype) {Reset();}

			void Reset()
			{
				time = totaltime = time_rpl = time_rewind = time_rewGh = 0.0;
				lastlap.Reset();
				bestlap.Reset();
				bestlapRace.Reset();
				num_laps = 0;
			}

			void Tick(float dt)
			{
				time += dt;  time_rpl += dt;  time_rewind += dt;  time_rewGh += dt;
			}
			
			void Back(float dt)  //-
			{
				time += dt;  time_rpl += dt;
				if (time < 0.0)  time = 0.0;
				if (time_rpl < 0.0)  time_rpl = 0.0;
			}
			

			void Lap(bool countit)
			{
				if (countit)
				{
					lastlap.Set(time);
					bestlap.SetIfFaster(time);
					bestlapRace.SetIfFaster(time);
				}

				totaltime += time;
				time = 0.0;  time_rewGh = 0.0;
				num_laps++;
			}

			void LapWithTime(bool countit, double curtime)
			{
				if (countit)
				{
					lastlap.Set(curtime);
					bestlap.SetIfFaster(curtime);
					bestlapRace.SetIfFaster(time);
				}

				totaltime += curtime;
				time = 0.0;  time_rewGh = 0.0;
				num_laps++;
			}

			const std::string & GetCarType() const {return cartype;}

			void DebugPrint(std::ostream & out)
			{
				out << "car=" << cartype << ", t=" << totaltime << ", tlap=" << time << ", last=" <<
					lastlap.GetTimeInSeconds() << ", best=" << bestlap.GetTimeInSeconds() <<
					", lap=" << num_laps << std::endl;
			}
			
			void RestartReplay()  // replay play restart
			{
				time = time_rpl = time_rewGh = 0.0;
			}			

			void SetTimeReplay(double t)  // replay play
			{
				time = time_rpl = time_rewGh = t;
			}	
			
			double GetTimeReplay() const{	return time_rpl;  }
			double GetTime() const		{	return time;  }
			
			double GetTimeTotal() const	{	return totaltime;	}
			double GetLastLap() const	{	return lastlap.GetTimeInSeconds();	}
			double GetBestLap() const	{	return bestlap.GetTimeInSeconds();	}
			double GetBestLapRace() const{	return bestlapRace.GetTimeInSeconds();	}
			int GetCurrentLap() const	{	return num_laps;	}
	};

	std::vector <LAPINFO> car;

	// variables for drawing text

	bool loaded;
	CONFIGFILE trackrecords; //the track records configfile
	std::string trackrecordsfile; //the filename for the track records
	//unsigned int carId; //the index for the player's car; defaults to zero

public:
	TIMER()
		:loaded(false), pretime(0.0)/*,carId(0)*/
		,waiting(false), end_sim(false)
		,netw_lap(1)
	{	}
	~TIMER()
	{	Unload();  }

	float pretime; // amount of time left in staging
	bool waiting;  // for other players in multi or in champs to close info wnd
	bool end_sim;  // simulate at end of champ, no input, wnd shown

	bool Load(const std::string & trackrecordspath, float stagingtime);
	///add a car of the given type and return the integer identifier that the track system will use
	int AddCar(const std::string & cartype) {  car.push_back(LAPINFO(cartype));  return car.size()-1;  }

	void Unload();

	void Tick(float dt);
	bool Lap(const int carId, const bool countit, bool bTrackReverse);
	
	int netw_lap;
	bool LapNetworkTime(const int carId, int lap, const double curtime);  ///+

	void DebugPrint(std::ostream & out)
	{
		for (size_t i = 0; i < car.size(); ++i)
		{
			out << i << ". ";
			car[i].DebugPrint(out);
		}
	}

	double GetPlayerTimeTot(const int carId) {	assert(carId<car.size());	return car[carId].GetTimeTotal();	}
	double GetPlayerTime(const int carId) {		assert(carId<car.size());	return car[carId].GetTime();	}
	double GetReplayTime(const int carId) {		assert(carId<car.size());	return car[carId].GetTimeReplay();  }  // replay
	void SetReplayTime(const int carId, double t){assert(carId<car.size());	car[carId].SetTimeReplay(t);  }
	void RestartReplay(const int carId)   {		assert(carId<car.size());	car[carId].RestartReplay();  }
	double& GetRewindTime(const int carId) {	assert(carId<car.size());	return car[carId].time_rewind;  }  // rewind
	double& GetRewindTimeGh(const int carId) {	assert(carId<car.size());	return car[carId].time_rewGh;  }  // rewind

	void Back(const int carId,double time) {	assert(carId<car.size());	car[carId].Back(time);  }  // back

	void Reset(int id = -1)
	{
		if (id == -1)
		{
			for (int i=0; i < car.size(); ++i)
				car[i].Reset();
		}else{
			if (car.size() > id)
				car[id].Reset();
		}
	}
	
	float GetLastLap(const int carId)	{		assert(carId<car.size());	return car[carId].GetLastLap();  }
	float GetBestLapRace(const int carId)	{	assert(carId<car.size());	return car[carId].GetBestLapRace();  }
	float GetBestLap(const int carId, bool bTrackReverse)
	{
		assert(carId<car.size());
		float curbestlap = car[carId].GetBestLap();
		float prevbest(0);

		bool haveprevbest = trackrecords.GetParam(
			car[carId].GetCarType() + (bTrackReverse ? "_rev" : "") + ".sector 0", prevbest);
		if (haveprevbest)
		{
			if (curbestlap == 0)
				return prevbest;
			else
				if (prevbest < curbestlap)
					return prevbest;
				else
					return curbestlap;
		}else
			return curbestlap;
	}
	int GetPlayerCurrentLap(const int carId) {  return GetCurrentLap(carId);  }
	int GetCurrentLap(unsigned int index)    {  assert(index<car.size());  return car[index].GetCurrentLap();  }

};
