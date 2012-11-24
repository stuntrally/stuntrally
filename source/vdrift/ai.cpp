#include "pch.h"
#include "ai.h"
#include "car.h"
#include "bezier.h"
#include "track.h"
#include "cardefs.h"
#include "mathvector.h"
#include "optional.h"
#include "unittest.h"
#include "../ogre/common/Defines.h"

//#ifndef
#define M_PI  3.14159265358979323846
//#endif

#define GRAVITY 9.8

//used to calculate brake value
#define MAX_SPEED_DIFF 6.0
#define MIN_SPEED_DIFF 1.0

//used to find the point the car should steer towards to
#define LOOKAHEAD_FACTOR1 2.25
#define LOOKAHEAD_FACTOR2 0.33

//used to detect very sharp corners like Circuit de Pau
#define LOOKAHEAD_MIN_RADIUS 8.0

//used to calculate friction coefficient
#define FRICTION_FACTOR_LONG 0.68
#define FRICTION_FACTOR_LAT 0.62

//maximum change in brake value per second
#define BRAKE_RATE_LIMIT 0.1
#define THROTTLE_RATE_LIMIT 0.1

//#define VISUALIZE_AI_DEBUG


float clamp(float val, float min, float max)
{
	assert(min < max);
	return std::min(max,std::max(min,val));
}

//note that rate_limit_neg should be positive, it gets inverted inside the function
float RateLimit(float old_value, float new_value, float rate_limit_pos, float rate_limit_neg)
{
	if (new_value - old_value > rate_limit_pos)
		return old_value + rate_limit_pos;
	else if (new_value - old_value < -rate_limit_neg)
		return old_value - rate_limit_neg;
	else
		return new_value;
}

const std::vector <float> & AI::GetInputs(CAR * car) const
{
	for( std::vector<AI_Car>::const_iterator it = AI_Cars.begin (); it != AI_Cars.end (); it++ )
	{
		if (car == it->car)
			return it->inputs;
	}
	return empty_vector;
}

void AI::add_car(CAR * car, float difficulty)
{
	assert(car);
	AI_Cars.push_back(AI_Car(car, difficulty));
	assert(car->GetTCSEnabled());
	assert(car->GetABSEnabled());
	car->SetAutoShift(true);
	car->SetAutoClutch(true);
	//car->SetAutoRear(true);
}

void AI::update(float dt, TRACK* track_p, const std::list <CAR> & othercars)
{
	updatePlan(othercars, dt);
	
	for( std::vector<AI_Car>::iterator it = AI_Cars.begin (); it != AI_Cars.end (); it++ )
	{
		analyzeOthers(&(*it), dt, othercars);
		updateGasBrake(&(*it), dt, track_p, othercars);
		updateSteer(&(*it), dt, othercars);
	}
}

MATHVECTOR<float,3> TransformToWorldspace(const MATHVECTOR<float,3> & bezierspace)
{
	return MATHVECTOR<float,3> (bezierspace[2], bezierspace[0], bezierspace[1]);
}

MATHVECTOR<float,3> TransformToPatchspace(const MATHVECTOR<float,3> & bezierspace)
{
	return MATHVECTOR<float,3> (bezierspace[1], bezierspace[2], bezierspace[0]);
}

QT_TEST(ai_test)
{
	MATHVECTOR<float,3> testvec(1,2,3);
	QT_CHECK_EQUAL(testvec, TransformToWorldspace(TransformToPatchspace(testvec)));
	QT_CHECK_EQUAL(testvec, TransformToPatchspace(TransformToWorldspace(testvec)));
}

const BEZIER * GetCurrentPatch(const CAR *c)
{
	const BEZIER *curr_patch = c->GetCurPatch(0);
	if (!curr_patch)
	{
		curr_patch = c->GetCurPatch(1); //let's try the other wheel
		if (!curr_patch)
		{
			return NULL;
		}
	}
	
	return curr_patch;
}

MATHVECTOR<float,3> GetPatchFrontCenter(const BEZIER & patch)
{
	return (patch.GetPoint(0,0) + patch.GetPoint(0,3)) * 0.5;
}

MATHVECTOR<float,3> GetPatchBackCenter(const BEZIER & patch)
{
	return (patch.GetPoint(3,0) + patch.GetPoint(3,3)) * 0.5;
}

MATHVECTOR<float,3> GetPatchDirection(const BEZIER & patch)
{
	return (GetPatchFrontCenter(patch) - GetPatchBackCenter(patch)) * 0.5;
}

MATHVECTOR<float,3> GetPatchWidthVector(const BEZIER & patch)
{
	return ((patch.GetPoint(0,0) + patch.GetPoint(3,0)) -
			(patch.GetPoint(0,3) + patch.GetPoint(3,3))) * 0.5;
}

double GetPatchRadius(const BEZIER & patch)
{
	if (patch.GetNextPatch() && patch.GetNextPatch()->GetNextPatch())
	{
		double track_radius = 0;
		
		/*MATHVECTOR<float,3> d1 = -GetPatchDirection(patch);
		MATHVECTOR<float,3> d2 = GetPatchDirection(*patch.GetNextPatch());*/
		MATHVECTOR<float,3> d1 = -(patch.GetNextPatch()->GetRacingLine() - patch.GetRacingLine());
		MATHVECTOR<float,3> d2 = patch.GetNextPatch()->GetNextPatch()->GetRacingLine() - patch.GetNextPatch()->GetRacingLine();
		d1[1] = 0;
		d2[1] = 0;
		float d1mag = d1.Magnitude();
		float d2mag = d2.Magnitude();
		float diff = d2mag - d1mag;
		double dd = ((d1mag < 0.0001) || (d2mag < 0.0001)) ? 0.0 : d1.Normalize().dot(d2.Normalize());
		float angle = acos((dd>=1.0L)?1.0L:(dd<=-1.0L)?-1.0L:dd);
		float d1d2mag = d1mag + d2mag;
		float alpha = (d1d2mag < 0.0001) ? 0.0f : (M_PI * diff + 2.0 * d1mag * angle) / d1d2mag / 2.0;
		if (fabs(alpha - M_PI/2.0) < 0.001) track_radius = 10000.0;
		else track_radius = d1mag / 2.0 / cos(alpha);
		
		return track_radius;
	}
	else //fall back
		return 0;
}

///trim the patch's width in-place
void TrimPatch(BEZIER & patch, float trimleft_front, float trimright_front, float trimleft_back, float trimright_back)
{
	MATHVECTOR<float,3> frontvector = (patch.GetPoint(0,3) - patch.GetPoint(0,0));
	MATHVECTOR<float,3> backvector = (patch.GetPoint(3,3) - patch.GetPoint(3,0));
	float frontwidth = frontvector.Magnitude();
	float backwidth = backvector.Magnitude();
	if (trimleft_front + trimright_front > frontwidth)
	{
		float scale = frontwidth/(trimleft_front + trimright_front);
		trimleft_front *= scale;
		trimright_front *= scale;
	}
	if (trimleft_back + trimright_back > backwidth)
	{
		float scale = backwidth/(trimleft_back + trimright_back);
		trimleft_back *= scale;
		trimright_back *= scale;
	}
	
	MATHVECTOR<float,3> newfl = patch.GetPoint(0,0);
	MATHVECTOR<float,3> newfr = patch.GetPoint(0,3);
	MATHVECTOR<float,3> newbl = patch.GetPoint(3,0);
	MATHVECTOR<float,3> newbr = patch.GetPoint(3,3);
	
	if (frontvector.Magnitude() > 0.001)
	{
		MATHVECTOR<float,3> trimdirection_front = frontvector.Normalize();
		newfl = patch.GetPoint(0,0) + trimdirection_front*trimleft_front;
		newfr = patch.GetPoint(0,3) - trimdirection_front*trimright_front;
	}
	
	if (backvector.Magnitude() > 0.001)
	{
		MATHVECTOR<float,3> trimdirection_back = backvector.Normalize();
		newbl = patch.GetPoint(3,0) + trimdirection_back*trimleft_back;
		newbr = patch.GetPoint(3,3) - trimdirection_back*trimright_back;
	}
	
	patch.SetFromCorners(newfl, newfr, newbl, newbr);
}

BEZIER AI::RevisePatch(const BEZIER * origpatch, bool use_racingline, AI_Car *c, const std::list <CAR> & allcars)
{
	BEZIER patch = *origpatch;
	
	//take into account the racing line
	//use_racingline = false;
	if (use_racingline && patch.GetNextPatch() && patch.HasRacingline())
	{
		float widthfront = std::min((patch.GetNextPatch()->GetRacingLine()-patch.GetPoint(0,0)).Magnitude(),
									 (patch.GetNextPatch()->GetRacingLine()-patch.GetPoint(0,3)).Magnitude());
		float widthback = std::min((patch.GetRacingLine()-patch.GetPoint(3,0)).Magnitude(),
									(patch.GetRacingLine()-patch.GetPoint(3,3)).Magnitude());
		float trimleft_front = (patch.GetNextPatch()->GetRacingLine() - patch.GetPoint(0,0)).Magnitude()-widthfront;
		float trimright_front = (patch.GetNextPatch()->GetRacingLine() - patch.GetPoint(0,3)).Magnitude()-widthfront;
		float trimleft_back = (patch.GetRacingLine() - patch.GetPoint(3,0)).Magnitude()-widthback;
		float trimright_back = (patch.GetRacingLine() - patch.GetPoint(3,3)).Magnitude()-widthback;
		TrimPatch(patch, trimleft_front, trimright_front, trimleft_back, trimright_back);
	}
	
	//check for revisions due to other cars
	/*const float trim_falloff_distance = 100.0; //trim fallof distance in meters per (meters per second)
	const MATHVECTOR<float,3> throttle_axis(-1,0,0); //positive is in front of the car
	std::map <const CAR *, PATH_REVISION> & revmap = path_revisions;
	for (std::map <const CAR *, PATH_REVISION>::iterator i = revmap.begin(); i != revmap.end(); i++)
	{
		if (i->first != c->car)
		{
			//compute relative info
			MATHVECTOR<float,3> myvel = c->car->GetVelocity();
			MATHVECTOR<float,3> othervel = i->first->GetVelocity();
			(-c->car->GetOrientation()).RotateVector(myvel);
			(-i->first->GetOrientation()).RotateVector(othervel);
			float speed_diff = myvel.dot(throttle_axis) - othervel.dot(throttle_axis); //positive if other car is faster //actually positive if my car is faster, right?

			float cardist_back = patch.dist_from_start - i->second.car_pos_along_track; //positive if patch is ahead of car
			float patchlen = GetPatchDirection(patch).Magnitude();
			float cardist_front = (patch.dist_from_start+patchlen) - i->second.car_pos_along_track;
			
			const float minfalloff = 10;
			const float maxfalloff = 60;
			float cur_trim_falloff_distance_fwd = minfalloff;
			float cur_trim_falloff_distance_rear = minfalloff;
			float falloff = clamp(trim_falloff_distance*std::abs(speed_diff),minfalloff,maxfalloff);
			if (speed_diff > 0)
			{
				//cur_trim_falloff_distance_fwd = falloff;
			}
			else
				cur_trim_falloff_distance_rear = falloff;
			
			float scale_front = clamp(1.0f-cardist_front/cur_trim_falloff_distance_fwd, 0, 1);
			if (cardist_front < 0)
				scale_front = clamp(1.0f+cardist_front/cur_trim_falloff_distance_rear, 0, 1);
			float scale_back = clamp(1.0f-cardist_back/cur_trim_falloff_distance_fwd, 0, 1);
			if (cardist_back < 0)
				scale_back = clamp(1.0f+cardist_back/cur_trim_falloff_distance_rear, 0, 1);
			
			std::cout << speed_diff << ", " << cur_trim_falloff_distance_fwd << ", " << cur_trim_falloff_distance_rear << ", " << cardist_front << ", " << cardist_back << ", " << scale_front << ", " << scale_back << std::endl;
			
			float trimleft_front = i->second.trimleft_front*scale_front;
			float trimright_front = i->second.trimright_front*scale_front;
			float trimleft_back = i->second.trimleft_back*scale_back;
			float trimright_back = i->second.trimright_back*scale_back;
			
			TrimPatch(patch, trimleft_front, trimright_front, trimleft_back, trimright_back);
		}
	}*/
	
	return patch;
}

void AI::updateGasBrake(AI_Car *c, float dt, TRACK* track_p, const std::list <CAR> & othercars)
{
	c->brakelook.clear();
	
	float brake_value = 0.0;
	float gas_value = 0.5;
	const float speed_percent = 1.0;

	/*if (c->car->GetEngineRPM() < c->car->GetEngineStallRPM())
		c->inputs[CARINPUT::START_ENGINE] = 1.0;
	else
		c->inputs[CARINPUT::START_ENGINE] = 0.0;*/
	
	calcMu(c, track_p);

	const BEZIER *curr_patch_ptr = GetCurrentPatch(c->car);
	//if car is not on track, just let it roll
    if (!curr_patch_ptr)
	{
		c->inputs[CARINPUT::THROTTLE] = 0.8;
		c->inputs[CARINPUT::BRAKE] = 0.0;
		return;
	}
	
	BEZIER curr_patch = RevisePatch(curr_patch_ptr, c->use_racingline, c, othercars);
	//BEZIER curr_patch = *curr_patch_ptr;

	MATHVECTOR<float,3> patch_direction = TransformToWorldspace(GetPatchDirection(curr_patch));
	
	//this version uses the velocity along tangent vector. it should calculate a lower current speed,
	//hence higher gas value or lower brake value
	//float currentspeed = c->car->chassis().cm_velocity().component(direction_vector);
	float currentspeed = c->car->GetVelocity().dot(patch_direction.Normalize());
	//this version just uses the velocity, do not care about the direction
	//float currentspeed = c->car->chassis().cm_velocity().magnitude();

	//check speed against speed limit of current patch
	float speed_limit = 0;
	if (!curr_patch.next_patch)
	{
		speed_limit = calcSpeedLimit(c, &curr_patch, NULL, c->lateral_mu, GetPatchWidthVector(*curr_patch_ptr).Magnitude())*speed_percent;
	}
	else
	{
		BEZIER next_patch = RevisePatch(curr_patch.next_patch, c->use_racingline, c, othercars);
		speed_limit = calcSpeedLimit(c, &curr_patch, &next_patch, c->lateral_mu, GetPatchWidthVector(*curr_patch_ptr).Magnitude())*speed_percent;
	}
	
	speed_limit *= c->difficulty;
	
	float speed_diff = speed_limit - currentspeed;

	if (speed_diff < 0.0)
	{
		if (-speed_diff < MIN_SPEED_DIFF) //no need to brake if diff is small
		{
			brake_value = 0.0;
		}
		else
		{
			brake_value = -speed_diff / MAX_SPEED_DIFF;
			if (brake_value > 1.0) brake_value = 1.0;
		}
		gas_value = 0.0;
	}
	else if (isnan(speed_diff) || speed_diff > MAX_SPEED_DIFF)
	{
		gas_value = 1.0;
		brake_value = 0.0;
	}
	else
	{
		gas_value = speed_diff / MAX_SPEED_DIFF;
		brake_value = 0.0;
	}

	//check upto maxlookahead distance
	float maxlookahead = calcBrakeDist(c, currentspeed, 0.0, c->longitude_mu)+10;
	//maxlookahead = 0.1;
	float dist_checked = 0.0;
	float brake_dist = 0.0;
	BEZIER patch_to_check = curr_patch;
	
#ifdef VISUALIZE_AI_DEBUG
	c->brakelook.push_back(patch_to_check);
#endif

	while (dist_checked < maxlookahead)
	{
		BEZIER * unmodified_patch_to_check = patch_to_check.next_patch;
		
		//if there is no next patch(probably a non-closed track, just let it roll
		if (!patch_to_check.next_patch)
		{
			brake_value = 0.0;
			dist_checked = maxlookahead;
			break;
		}
		else
			patch_to_check = RevisePatch(patch_to_check.next_patch, c->use_racingline, c, othercars);
		
#ifdef VISUALIZE_AI_DEBUG
		c->brakelook.push_back(patch_to_check);
#endif
		
		//speed_limit = calcSpeedLimit(c, &patch_to_check, c->lateral_mu)*speed_percent;
		if (!patch_to_check.next_patch)
		{
			speed_limit = calcSpeedLimit(c, &patch_to_check, NULL, c->lateral_mu, GetPatchWidthVector(*unmodified_patch_to_check).Magnitude())*speed_percent;
		}
		else
		{
			BEZIER next_patch = RevisePatch(patch_to_check.next_patch, c->use_racingline, c, othercars);
			speed_limit = calcSpeedLimit(c, &patch_to_check, &next_patch, c->lateral_mu, GetPatchWidthVector(*unmodified_patch_to_check).Magnitude())*speed_percent;
		}
		
		dist_checked += GetPatchDirection(patch_to_check).Magnitude();
		brake_dist = calcBrakeDist(c, currentspeed, speed_limit, c->longitude_mu);
		
		//if (brake_dist + CORNER_BRAKE_OFFSET > dist_checked)
		if (brake_dist > dist_checked)
		{
			//std::cout << "brake: limit " << speed_limit << ", cur " << currentspeed << ", brake " << brake_dist << ", dist " << dist_checked << std::endl;
			
			/*brake_value = (brake_dist + CORNER_BRAKE_OFFSET - dist_checked)*CORNER_BRAKE_GAIN;
			if (brake_value > 1.0) brake_value = 1.0;*/
			brake_value = 1.0;
			gas_value = 0.0;
			break;
		}
	}
	
	//std::cout << speed_limit << std::endl;
	if (c->car->GetGear() == 0)
	{
		c->inputs[CARINPUT::SHIFT_UP] = 1.0;
		gas_value = 0.2;
	}
	else
	{
		c->inputs[CARINPUT::SHIFT_UP] = 0.0;
	}

	/*float trafficbrake = brakeFromOthers(c, dt, othercars, speed_diff); //consider traffic avoidance bias
	if (trafficbrake > 0)
	{
		gas_value = 0.0;
		brake_value = std::max(trafficbrake, brake_value);
	}*/
	
	c->inputs[CARINPUT::THROTTLE] = RateLimit(c->inputs[CARINPUT::THROTTLE], gas_value,
											  THROTTLE_RATE_LIMIT, THROTTLE_RATE_LIMIT);
	c->inputs[CARINPUT::BRAKE] = RateLimit(c->inputs[CARINPUT::BRAKE], brake_value,
										   BRAKE_RATE_LIMIT, BRAKE_RATE_LIMIT);
	
	//c->inputs[CARINPUT::THROTTLE] = 0.0;
	//c->inputs[CARINPUT::BRAKE] = 1.0;
}

void AI::calcMu(AI_Car *c, TRACK* track_p)
{
	int i;
	float long_friction = 0.0;
	float lat_friction = 0.0;

	for (i=0; i<4; i++)
	{
		long_friction += c->car->GetTireMaxFx(WHEEL_POSITION(i));
		lat_friction += c->car->GetTireMaxFy(WHEEL_POSITION(i));
	}

	float long_mu = FRICTION_FACTOR_LONG * long_friction / c->car->GetMass() / GRAVITY;
	float lat_mu = FRICTION_FACTOR_LAT * lat_friction / c->car->GetMass() / GRAVITY;
	if (!isnan(long_mu)) c->longitude_mu = long_mu;
	if (!isnan(lat_mu)) c->lateral_mu = lat_mu;
}

float AI::calcSpeedLimit(AI_Car *c, const BEZIER* patch, const BEZIER * nextpatch, float friction, float extraradius=0)
{
	assert(patch);
	
	//adjust the radius at corner exit to allow a higher speed.
	//this will get the car to accellerate out of corner
	//double track_width = GetPatchWidthVector(*patch).Magnitude();
	double adjusted_radius = GetPatchRadius(*patch);
	if (nextpatch)
	{
		if (GetPatchRadius(*nextpatch) > adjusted_radius &&
			GetPatchRadius(*patch) > LOOKAHEAD_MIN_RADIUS)
		{
			adjusted_radius += extraradius;
		}
	}
	
	//no downforce
	//float v1 = sqrt(friction * GRAVITY * adjusted_radius);

	//take into account downforce
	double denom = (1.0 - std::min(1.01, adjusted_radius * -(c->car->GetAerodynamicDownforceCoefficient()) * friction / c->car->GetMass()));
	double real = (friction * GRAVITY * adjusted_radius) / denom;
	double v2 = 1000.0; //some really big number
	if (real > 0)
		v2 = sqrt(real);
	
	//std::cout << v2 << ", " << sqrt(friction * GRAVITY * adjusted_radius) << ", " << GetPatchRadius(*patch) << ", " << acos((-GetPatchDirection(*patch)).Normalize().dot(GetPatchDirection(*patch->GetNextPatch()).Normalize()))*180.0/PI_d << " --- " << -GetPatchDirection(*patch) << " --- " << GetPatchDirection(*patch->GetNextPatch()) << std::endl;
	
	return v2;
}

float AI::calcBrakeDist(AI_Car *ai_car, float current_speed, float allowed_speed, float friction)
{
	float c = friction * GRAVITY;
	float d = (-(ai_car->car->GetAerodynamicDownforceCoefficient()) * friction + ai_car->car->GetAeordynamicDragCoefficient())
			 / ai_car->car->GetMass();
	float v1sqr = current_speed * current_speed;
	float v2sqr = allowed_speed * allowed_speed;
	return -log((c + v2sqr*d)/(c + v1sqr*d))/(2.0*d);
}

void AI::updateSteer(AI_Car *c, float dt, const std::list <CAR> & othercars)
{
	c->steerlook.clear();
	
	const BEZIER *curr_patch_ptr = GetCurrentPatch(c->car);
	//if car has no contact with track, just let it roll
	if (!curr_patch_ptr)
	{
		if (!c->last_patch) return;
		//if car is off track, steer the car towards the last patch it was on
		//this should get the car back on track
		else curr_patch_ptr = c->last_patch;
	}

	c->last_patch = curr_patch_ptr; //store the last patch car was on
	
	BEZIER curr_patch = RevisePatch(curr_patch_ptr, c->use_racingline, c, othercars);

#ifdef VISUALIZE_AI_DEBUG
	c->steerlook.push_back(curr_patch);
#endif
	
	//if there is no next patch (probably a non-closed track), let it roll
	if (!curr_patch.next_patch) return;
	
	BEZIER next_patch = RevisePatch(curr_patch.next_patch, c->use_racingline, c, othercars);

	//find the point to steer towards
	float track_width = GetPatchWidthVector(curr_patch).Magnitude();
	float lookahead = track_width * LOOKAHEAD_FACTOR1 + 
			c->car->GetVelocity().Magnitude() * LOOKAHEAD_FACTOR2;
	lookahead = 1.0;
	float length = 0.0;
	MATHVECTOR<float,3> dest_point = GetPatchFrontCenter(next_patch);
	
	while (length < lookahead)
	{
#ifdef VISUALIZE_AI_DEBUG
		c->steerlook.push_back(next_patch);
#endif
		
		length += GetPatchDirection(next_patch).Magnitude()*2.0;
		dest_point = GetPatchFrontCenter(next_patch);
		
		//if there is no next patch for whatever reason, stop lookahead
		if (!next_patch.next_patch)
		{
			length = lookahead;
			break;
		}
		
		next_patch = RevisePatch(next_patch.next_patch, c->use_racingline, c, othercars);
		
		//if next patch is a very sharp corner, stop lookahead
		if (GetPatchRadius(next_patch) < LOOKAHEAD_MIN_RADIUS)
		{
			length = lookahead;
			break;
		}
	}

	MATHVECTOR<float,3> next_position = TransformToWorldspace(dest_point);
	MATHVECTOR<float,3> car_position = c->car->GetCenterOfMassPosition();
	MATHVECTOR<float,3> car_orientation(0,1,0);
	QUATERNION<float> fixer;
	fixer.Rotate(-PI_d*0.5, 0, 0, 1);
	(c->car->GetOrientation()*fixer).RotateVector(car_orientation);

	MATHVECTOR<float,3> desire_orientation = next_position - car_position;

	//car's direction on the horizontal plane
	car_orientation[2] = 0;
	//desired direction on the horizontal plane
	desire_orientation[2] = 0;
	
	car_orientation = car_orientation.Normalize();
	desire_orientation = desire_orientation.Normalize();

	//the angle between car's direction and unit y vector (forward direction)
	double alpha = Angle(car_orientation[0], car_orientation[1]);

	//the angle between desired direction and unit y vector (forward direction)
	double beta = Angle(desire_orientation[0], desire_orientation[1]);

	//calculate steering angle and direction
	double angle = beta - alpha;
	
	//angle += steerAwayFromOthers(c, dt, othercars, angle); //sum in traffic avoidance bias
	
	if (angle > -360.0 && angle <= -180.0)
		angle = -(360.0 + angle);
	else if (angle > -180.0 && angle <= 0.0)
		angle = - angle;
	else if (angle > 0.0 && angle <= 180.0)
		angle = - angle;
	else if (angle > 180.0 && angle <= 360.0)
		angle = 360.0 - angle;
	
	float optimum_range = c->car->GetOptimumSteeringAngle();
	angle = clamp(angle, -optimum_range, optimum_range);

	float steer_value = angle / c->car->GetMaxSteeringAngle();
	if (steer_value > 1.0) steer_value = 1.0;
	else if (steer_value < -1.0) steer_value = -1.0;

	assert(!isnan(steer_value));
	c->inputs[CARINPUT::STEER_RIGHT] = steer_value;
}

///note that carposition must be in patch space
///returns distance from left side of the track
float GetHorizontalDistanceAlongPatch(const BEZIER & patch, MATHVECTOR<float,3> carposition)
{
	MATHVECTOR<float,3> leftside = (patch.GetPoint(0,0) + patch.GetPoint(3,0))*0.5;
	MATHVECTOR<float,3> rightside = (patch.GetPoint(0,3) + patch.GetPoint(3,3))*0.5;
	MATHVECTOR<float,3> patchwidthvector = rightside - leftside;
	return patchwidthvector.Normalize().dot(carposition-leftside);
}

float RampBetween(float val, float startat, float endat)
{
	assert(endat > startat);
	return (clamp(val,startat,endat)-startat)/(endat-startat);
}

float AI::brakeFromOthers(AI_Car *c, float dt, const std::list <CAR> & othercars, float speed_diff)
{
	const float nobiasdiff = 30;
	const float fullbiasdiff = 0;
	const float horizontal_care = 2.5; //meters to left and right which we'll brake for
	const float startateta = 10.0;
	const float fullbrakeeta = 1.0;
	const float startatdistance = 10.0;
	const float fullbrakedistance = 4.0;
	
	float mineta = 1000;
	float mindistance = 1000;
	
	for (std::map <const CAR *, AI_Car::OTHERCARINFO>::iterator i = c->othercars.begin(); i != c->othercars.end(); ++i)
	{
		if (i->second.active && std::abs(i->second.horizontal_distance) < horizontal_care)
		{
			if (i->second.fore_distance < mindistance)
			{
				mindistance = i->second.fore_distance;
				mineta = i->second.eta;
			}
		}
	}
	
	float bias = 0;
	
	float etafeedback = 0;
	float distancefeedback = 0;
	
	//correct for eta of impact
	if (mineta < 1000)
	{
		etafeedback += 1.0-RampBetween(mineta, fullbrakeeta, startateta);
	}
	
	//correct for absolute distance
	if (mindistance < 1000)
	{
		distancefeedback += 1.0-RampBetween(mineta,fullbrakedistance,startatdistance);
	}
	
	//correct for speed versus speed limit (don't bother to brake for slowpokes, just go around)
	float speedfeedback = 1.0-RampBetween(speed_diff, fullbiasdiff, nobiasdiff);
	
	//std::cout << mineta << ": " << etafeedback << ", " << mindistance << ": " << distancefeedback << ", " << speed_diff << ": " << speedfeedback << std::endl;
	
	//bias = clamp((etafeedback+distancefeedback)*speedfeedback,0,1);
	bias = clamp(etafeedback*distancefeedback*speedfeedback,0,1);
	
	return bias;
}

void AI::analyzeOthers(AI_Car *c, float dt, const std::list <CAR> & othercars)
{
	//const float speed = std::max(1.0f,c->car->GetVelocity().Magnitude());
	const float half_carlength = 1.25; //in meters
	
	//std::cout << speed << ": " << authority << std::endl;
	
	const MATHVECTOR<float,3> steer_right_axis(0,-1,0);
	const MATHVECTOR<float,3> throttle_axis(1,0,0); //positive is in front of the car
	
#ifdef VISUALIZE_AI_DEBUG
	c->avoidancedraw->ClearLine();
#endif
	
	for (std::list <CAR>::const_iterator i = othercars.begin(); i != othercars.end(); ++i)
	{
		if (&(*i) != c->car)
		{
			struct AI_Car::OTHERCARINFO & info = c->othercars[&(*i)];
			
			//find direction of othercar in our frame
			MATHVECTOR<float,3> relative_position = i->GetCenterOfMassPosition() - c->car->GetCenterOfMassPosition();
			(-c->car->GetOrientation()).RotateVector(relative_position);
			
			//std::cout << relative_position.dot(throttle_axis) << ", " << relative_position.dot(steer_right_axis) << std::endl;
			
			//only make a move if the other car is within our distance limit
			float fore_position = relative_position.dot(throttle_axis);
			//float speed_diff = i->GetVelocity().dot(throttle_axis) - c->car->GetVelocity().dot(throttle_axis); //positive if other car is faster
			
			MATHVECTOR<float,3> myvel = c->car->GetVelocity();
			MATHVECTOR<float,3> othervel = i->GetVelocity();
			(-c->car->GetOrientation()).RotateVector(myvel);
			(-i->GetOrientation()).RotateVector(othervel);
			float speed_diff = othervel.dot(throttle_axis) - myvel.dot(throttle_axis); //positive if other car is faster
			
			//std::cout << speed_diff << std::endl;
			//float distancelimit = clamp(distancelimitcoeff*-speed_diff, distancelimitmin, distancelimitmax);
			const float fore_position_offset = -half_carlength;
			if (fore_position > fore_position_offset)// && fore_position < distancelimit) //only pay attention to cars roughly in front of us
			{
				//float horizontal_distance = relative_position.dot(steer_right_axis); //fallback method if not on a patch
				//float orig_horiz = horizontal_distance;
				
				const BEZIER * othercarpatch = GetCurrentPatch(&(*i));
				const BEZIER * mycarpatch = GetCurrentPatch(c->car);
				
				if (othercarpatch && mycarpatch)
				{
					float my_track_placement = GetHorizontalDistanceAlongPatch(*mycarpatch, TransformToPatchspace(c->car->GetCenterOfMassPosition()));
					float their_track_placement = GetHorizontalDistanceAlongPatch(*othercarpatch, TransformToPatchspace(i->GetCenterOfMassPosition()));
					
					float speed_diff_denom = clamp(speed_diff, -100, -0.01);
					float eta = (fore_position-fore_position_offset)/-speed_diff_denom;
					
					info.fore_distance = fore_position;
					
					if (!info.active)
						info.eta = eta;
					else
						info.eta = RateLimit(info.eta, eta, 10.f*dt, 10000.f*dt);
					
					float horizontal_distance = their_track_placement - my_track_placement;
					//if (!info.active)
						info.horizontal_distance = horizontal_distance;
					/*else
						info.horizontal_distance = RateLimit(info.horizontal_distance, horizontal_distance, spacingdistance*dt, spacingdistance*dt);*/
					
					//std::cout << info.horizontal_distance << ", " << info.eta << std::endl;
					
					info.active = true;
				}
				else
					info.active = false;
				
				//std::cout << orig_horiz << ", " << horizontal_distance << ",    " << fore_position << ", " << speed_diff << std::endl;
				
				/*if (!min_horizontal_distance)
					min_horizontal_distance = optional <float> (horizontal_distance);
				else if (std::abs(min_horizontal_distance.get()) > std::abs(horizontal_distance))
					min_horizontal_distance = optional <float> (horizontal_distance);*/
			}
			else
				info.active = false;
		}
	}
}

/*float AI::steerAwayFromOthers(AI_Car *c, float dt, const std::list <CAR> & othercars, float cursteer)
{
	const float spacingdistance = 3.0; //how far left and right we target for our spacing in meters (center of mass to center of mass)
	const float ignoredistance = 6.0;
	
	float eta = 1000;
	float min_horizontal_distance = 1000;
	QUATERNION<float> otherorientation;
	
	for (std::map <const CAR *, AI_Car::OTHERCARINFO>::iterator i = c->othercars.begin(); i != c->othercars.end(); i++)
	{
		if (i->second.active && std::abs(i->second.horizontal_distance) < std::abs(min_horizontal_distance))
		{
			min_horizontal_distance = i->second.horizontal_distance;
			eta = i->second.eta;
			otherorientation = i->first->GetOrientation();
		}
	}
	
	if (min_horizontal_distance >= ignoredistance)
		return 0.0;
	
	float sidedist = min_horizontal_distance;
	float d = std::abs(sidedist) - spacingdistance*0.5;
	if (d < spacingdistance)
	{
		const MATHVECTOR<float,3> forward(1,0,0);
		MATHVECTOR<float,3> otherdir = forward;
		otherorientation.RotateVector(otherdir); //rotate to worldspace
		(-c->car->GetOrientation()).RotateVector(otherdir); //rotate to this car space
		otherdir[2] = 0; //remove vertical component
		otherdir = otherdir.Normalize(); //resize to unit size
		if (otherdir[0] > 0) //heading in the same direction
		{
			float diffangle = (180.0/PI_d)*asin(otherdir[1]); //positive when other car is pointing to our right
			
			if (diffangle * -sidedist < 0) //heading toward collision
			{
				const float c = spacingdistance * 0.5;
				d = std::max(0.0f, d - c);
				float psteer = diffangle;
				
				psteer = cursteer*(d/c)+1.5*psteer*(1.0f-d/c);
				
				std::cout << diffangle << ", " << d/c << ", " << psteer << std::endl;
				
				if (cursteer*psteer > 0 && std::abs(cursteer) > std::abs(psteer))
					return 0.0; //no bias, already more than correcting
				else
					return psteer - cursteer;
			}
		}
		else
			std::cout << "wrong quadrant: " << otherdir << std::endl;
	}
	else
	{
		std::cout << "out of range: " << d << ", " << spacingdistance << std::endl;
	}
	
	return 0.0;
}*/

float AI::steerAwayFromOthers(AI_Car *c, float dt, const std::list <CAR> & othercars, float cursteer)
{
	const float spacingdistance = 3.5; //how far left and right we target for our spacing in meters (center of mass to center of mass)
	const float horizontal_meters_per_second = 5.0; //how fast we want to steer away in horizontal meters per second
	const float speed = std::max(1.0f,c->car->GetVelocity().Magnitude());
	const float authority = std::min(10.0,(180.0/PI_d)*atan(horizontal_meters_per_second/speed)); //steering bias authority limit magnitude in degrees
	const float gain = 4.0; //amplify steering command by this factor
	const float mineta = 1.0; //fastest reaction time in seconds
	const float etaexponent = 1.0;
	
	float eta = 1000;
	float min_horizontal_distance = 1000;
	
	for (std::map <const CAR *, AI_Car::OTHERCARINFO>::iterator i = c->othercars.begin(); i != c->othercars.end(); ++i)
	{
		if (i->second.active && std::abs(i->second.horizontal_distance) < std::abs(min_horizontal_distance))
		{
			min_horizontal_distance = i->second.horizontal_distance;
			eta = i->second.eta;
		}
	}
	
	if (min_horizontal_distance == 1000)
		return 0.0;
	
	eta = std::max(eta, mineta);
	
	float bias = clamp(min_horizontal_distance, -spacingdistance, spacingdistance);
	if (bias < 0)
		bias = -bias - spacingdistance;
	else
		bias = spacingdistance - bias;
	
	bias *= pow(mineta,etaexponent)*gain/pow(eta,etaexponent);
	clamp(bias, -spacingdistance, spacingdistance);
	
	//std::cout << "min horiz: " << min_horizontal_distance << ", eta: " << eta << ", " << bias << std::endl;
	
	return (bias/spacingdistance)*authority;
}

double AI::Angle(double x1, double y1)
{
	return atan2(y1, x1) * 180.0 / M_PI;
}

optional <float> GetDistanceFromPatchToPatch(const BEZIER * frontpatch, const BEZIER * backpatch)
{
	float dist = 0;
	const BEZIER * curpatch = backpatch;
	while (curpatch != frontpatch && curpatch)
	{
		dist += GetPatchDirection(*curpatch).Magnitude();
		
		if (curpatch == frontpatch)
			return optional <float> (dist);
		
		curpatch = curpatch->GetNextPatch();
	}
	
	return optional <float> ();
}

///update our navigation markers
void AI::updatePlan(const std::list <CAR> & allcars, float dt)
{
	/* //update our account of the cars
	const float minwidth = 0.3;
	for (std::list <CAR>::const_iterator i = allcars.begin(); i != allcars.end(); i++)
	{
		const BEZIER * origpatch = GetCurrentPatch(&(*i));
		if (origpatch)
		{
			const BEZIER & patch = *origpatch;
			
			const float car_radius = 1.5; //treat a car like a circle
			MATHVECTOR<float,3> carpos = TransformToPatchspace(i->GetCenterOfMassPosition());
			MATHVECTOR<float,3> leftside = (patch.GetPoint(0,0) + patch.GetPoint(3,0))*0.5;
			MATHVECTOR<float,3> rightside = (patch.GetPoint(0,3) + patch.GetPoint(3,3))*0.5;
			MATHVECTOR<float,3> patchwidthvector = rightside - leftside;
			MATHVECTOR<float,3> frontcenter = (patch.GetPoint(0,0) + patch.GetPoint(0,3))*0.5;
			MATHVECTOR<float,3> backcenter = (patch.GetPoint(3,0) + patch.GetPoint(3,3))*0.5;
			MATHVECTOR<float,3> patchdirectionvector = frontcenter - backcenter;
			
			float patchwidth = patchwidthvector.Magnitude();
			float caroffsetfromleft = patchwidthvector.Normalize().dot(carpos-leftside);
			float trimleft = 0;
			float trimright = 0;
			
			if (caroffsetfromleft < patchwidth*0.5) //left side
			{
				float newwidth = patchwidth - std::max(0.0f,caroffsetfromleft+car_radius);
				newwidth = std::max(minwidth, newwidth);
				trimleft = patchwidth - newwidth;
			}
			else //right side
			{
				float newwidth = patchwidth - std::max(0.0f,(patchwidth - caroffsetfromleft + car_radius));
				newwidth = std::max(minwidth, newwidth);
				trimright = patchwidth - newwidth;
			}
			
			float caralong = GetPatchDirection(*origpatch).Normalize().dot(carpos-backcenter);
			
			{
				PATH_REVISION & rev = path_revisions[&(*i)];
				rev.trimleft_front = trimleft;
				rev.trimleft_back = trimleft;
				rev.trimright_front = trimright;
				rev.trimright_back = trimright;
				rev.car_pos_along_track = caralong + origpatch->dist_from_start;
			}
		}
		else
		{
			PATH_REVISION & rev = path_revisions[&(*i)];
			rev.trimleft_front = 0;
			rev.trimleft_back = 0;
			rev.trimright_front = 0;
			rev.trimright_back = 0;
			//rev.car_pos_along_track = caralong + origpatch->dist_from_start;
		}
	}*/
}
