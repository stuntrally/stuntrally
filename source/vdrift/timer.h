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
			LAPTIME() {Reset();}
			void Reset()
			{
				havetime = false;
				time = 0;
			}
			bool HaveTime() const {return havetime;}
			double GetTimeInSeconds() const {return time;}
			///convert time in seconds into output min and secs
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
			///only set the time if we don't have a time or if the new time is faster than the current time
			void SetIfFaster(double newtime)
			{
				if (!havetime || newtime < time)
					time = newtime;
				havetime = true;
			}
	};
	
	class DRIFTSCORE
	{
		private:
			float score;
			float thisdriftscore;
			bool drifting;
			float max_angle;
			float max_speed;
			
		public:
			DRIFTSCORE() {Reset();}
			void Reset()
			{
				score = 0;
				SetDrifting(false, false);
			}

			void SetScore ( float value )
			{
				score = value;
			}
			
			float GetScore() const
			{
				return score;
			}

			void SetDrifting ( bool value, bool countit )
			{
				if (!value && drifting && countit && thisdriftscore + GetBonusScore() > 5.0)
				{
					score += thisdriftscore + GetBonusScore();
					//std::cout << "Incrementing score: " << score << std::endl;
				}
				//else if (!value && drifting) std::cout << "Not scoring: " << countit << ", " << thisdriftscore << std::endl;
				
				if (!value)
				{
					thisdriftscore = 0;
					max_angle = 0;
					max_speed = 0;
				}
				
				drifting = value;
			}
		
			bool GetDrifting() const
			{
				return drifting;
			}

			void SetThisDriftScore ( float value )
			{
				thisdriftscore = value;
			}
			
			float GetThisDriftScore() const
			{
				return thisdriftscore;
			}

			void SetMaxAngle ( float value )
			{
				max_angle = value;
			}
		
			void SetMaxSpeed ( float value )
			{
				max_speed = value;
			}

			float GetMaxAngle() const
			{
				return max_angle;
			}
		
			float GetMaxSpeed() const
			{
				return max_speed;
			}
			
			float GetBonusScore() const
			{
				return max_speed / 2.0 + max_angle * 40.0 / PI_d + thisdriftscore; //including thisdriftscore here is redundant on purpose to give more points to long drifts
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
			double lapdistance; // total track distance driven this lap in meters
			DRIFTSCORE driftscore;

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
			
			double GetTimeReplay() const  // for replay
			{
				return time_rpl;
			}
			
			double GetTime() const
			{
				return time;
			}
			
			double GetTimeTotal() const
			{
				return totaltime;
			}

			double GetLastLap() const
			{
				return lastlap.GetTimeInSeconds();
			}

			double GetBestLap() const
			{
				return bestlap.GetTimeInSeconds();
			}

			double GetBestLapRace() const
			{
				return bestlapRace.GetTimeInSeconds();
			}

			int GetCurrentLap() const
			{
			    return num_laps;
			}

			void UpdateLapDistance(double newdistance)
			{
			    lapdistance = newdistance;
			}

			double GetLapDistance() const {return lapdistance;}

			const DRIFTSCORE & GetDriftScore() const
			{
				return driftscore;
			}
			
			DRIFTSCORE & GetDriftScore()
			{
				return driftscore;
			}
	};

	std::vector <LAPINFO> car;

	// variables for drawing text

	bool loaded;
	CONFIGFILE trackrecords; //the track records configfile
	std::string trackrecordsfile; //the filename for the track records
	//unsigned int carId; //the index for the player's car; defaults to zero

public:
	TIMER() : loaded(false),pretime(0.0)/*,carId(0)*/,waiting(false) {	}
	~TIMER() {	Unload();	}

	float pretime; // amount of time left in staging
	bool waiting;  // for other players in multi or in champs to close info wnd

	bool Load(const std::string & trackrecordspath, float stagingtime, std::ostream & error_output);
	///add a car of the given type and return the integer identifier that the track system will use
	int AddCar(const std::string & cartype) {  car.push_back(LAPINFO(cartype));  return car.size()-1;  }

	void Unload();

	void Tick(float dt);
	bool Lap(const int carId, const int prevsector, const int nextsector, const bool countit, bool bTrackReverse);
	bool LapNetworkTime(const int carId, const double curtime);  ///+

	void UpdateDistance(const int carId, const double newdistance)
	{
	    assert(carId < car.size());
	    car[carId].UpdateLapDistance(newdistance);
	}
	void DebugPrint(std::ostream & out)
	{
		for (unsigned int i = 0; i < car.size(); i++)
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
	
	float GetLastLap(const int carId) {		assert(carId<car.size());		return car[carId].GetLastLap();  }
	float GetBestLapRace(const int carId)	{	assert(carId<car.size());	return car[carId].GetBestLapRace();		}
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
		}
		else
			return curbestlap;
	}
	int GetPlayerCurrentLap(const int carId) {		return GetCurrentLap(carId);	}
	int GetCurrentLap(unsigned int index) {assert(index<car.size());return car[index].GetCurrentLap();}


	float GetDriftScore(unsigned int index) const
	{
		assert(index<car.size());
		return car[index].GetDriftScore().GetScore();
	}
	
	float GetThisDriftScore(unsigned int index) const
	{
		assert(index<car.size());
		return car[index].GetDriftScore().GetThisDriftScore() + car[index].GetDriftScore().GetBonusScore();
	}
	
	//  drift score -
	bool GetIsDrifting(unsigned int index) const
	{
		//assert(index<car.size());
		if (index < car.size())
			return false;
		return car[index].GetDriftScore().GetDrifting();
	}
	
	void SetIsDrifting(unsigned int index, bool newdrift, bool countthedrift)
	{
		assert(index<car.size());
		car[index].GetDriftScore().SetDrifting(newdrift, countthedrift);
	}
	
	void IncrementThisDriftScore(unsigned int index, float incrementamount)
	{
		assert(index<car.size());
		car[index].GetDriftScore().SetThisDriftScore(car[index].GetDriftScore().GetThisDriftScore()+incrementamount);
	}
	
	void UpdateMaxDriftAngleSpeed(unsigned int index, float angle, float speed)
	{
		assert(index<car.size());
		if (angle > car[index].GetDriftScore().GetMaxAngle())
			car[index].GetDriftScore().SetMaxAngle(angle);
		if (speed > car[index].GetDriftScore().GetMaxSpeed())
			car[index].GetDriftScore().SetMaxSpeed(speed);
	}
};
