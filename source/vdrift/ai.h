#ifndef _AI_H
#define _AI_H

#include "cardefs.h"
#include "reseatable_reference.h"

#include <vector>
#include <list>
#include <map>

class CAR;
class BEZIER;
class TRACK;


struct AI_Car
{
	AI_Car (CAR * new_car, float newdifficulty) : car (new_car), shift_time(0.0f),
		longitude_mu(0.9f), lateral_mu(0.9f),last_patch(NULL), use_racingline(true),
		difficulty(newdifficulty)
		{
			inputs.resize(CARINPUT::ALL, 0.0f);
		}
	CAR * car;
	float shift_time;
	float longitude_mu; ///<friction coefficient of the tire - longitude direction
	float lateral_mu; ///<friction coefficient of the tire - lateral direction
	const BEZIER * last_patch; ///<last patch the car was on, used in case car is off track
	bool use_racingline; ///<true allows the AI to take a proper racing line
	float difficulty;
	
	///the vector is indexed by CARINPUT values
	std::vector <float> inputs;
	
	struct OTHERCARINFO
	{
		OTHERCARINFO() : active(false) {}
		
		float horizontal_distance;
		float fore_distance;
		float eta;
		bool active;
	};
	
	std::map <const CAR *, OTHERCARINFO> othercars;
	
	std::vector <BEZIER> brakelook;
	std::vector <BEZIER> steerlook;
};

class AI
{
private:
	//for replanning the path
	struct PATH_REVISION
	{
		PATH_REVISION() : trimleft_front(0), trimright_front(0), trimleft_back(0), trimright_back(0), car_pos_along_track(0) {}
		
		float trimleft_front;
		float trimright_front;
		float trimleft_back;
		float trimright_back;
		float car_pos_along_track;
	};
	
	std::map <const CAR *, PATH_REVISION> path_revisions;
	
	std::vector <float> empty_vector;
	std::vector <AI_Car> AI_Cars;
	void updateGasBrake(AI_Car *c, float dt, TRACK* track_p, const std::list <CAR> & othercars);
	void calcMu(AI_Car *c, TRACK* track_p);
	float calcSpeedLimit(AI_Car *c, const BEZIER* patch, const BEZIER* nextpatch, float friction, float extraradius);
	float calcBrakeDist(AI_Car *c, float current_speed, float allowed_speed, float friction);
	void updateSteer(AI_Car *c, float dt, const std::list <CAR> & othercars);
	void analyzeOthers(AI_Car *c, float dt, const std::list <CAR> & othercars);
	float steerAwayFromOthers(AI_Car *c, float dt, const std::list <CAR> & othercars, float cursteer); ///< returns a float that should be added into the steering wheel command
	float brakeFromOthers(AI_Car *c, float dt, const std::list <CAR> & othercars, float speed_diff); ///< returns a float that should be added into the brake command. speed_diff is the difference between the desired speed and speed limit of this area of the track
	double Angle(double x1, double y1); ///< returns the angle in degrees of the normalized 2-vector
	//void Visualize(AI_Car *c, SCENENODE & topnode);
	BEZIER RevisePatch(const BEZIER * origpatch, bool use_racingline, AI_Car *c, const std::list <CAR> & allcars);
	void updatePlan(const std::list <CAR> & allcars, float dt);

public:
	void add_car(CAR * car, float difficulty);
	void clear_cars() { AI_Cars.clear(); path_revisions.clear(); }
	void update(float dt, TRACK* track_p, const std::list <CAR> & othercars);
	const std::vector <float> & GetInputs(CAR * car) const; ///< returns an empty vector if the car isn't AI-controlled
	//void Visualize(SCENENODE & topnode);
};

#endif //_AI_H
