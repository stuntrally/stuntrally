#include "pch.h"
#include "performance_testing.h"
#include "configfile.h"
#include "../ogre/CGame.h"  // pApp
#include "../ogre/common/Def_Str.h"
using namespace std;


PERFORMANCE_TESTING::PERFORMANCE_TESTING()
{
	surface.type = TRACKSURFACE::ASPHALT;
	surface.bumpWaveLength = 10;  surface.bumpAmplitude = 0.01;
	surface.frictionTread = 1;  //surface.frictionNonTread = 1;
	surface.rollingDrag = 1.0;  //surface.rollResistanceCoefficient = 0.2;
}

void PERFORMANCE_TESTING::ResetCar()
{
	MATHVECTOR<double,3> initial_position(0,0,0);
	car.dynamics.SetPosition(initial_position);

	car.dynamics.SetTCS(true);
	car.dynamics.SetABS(true);
	car.SetAutoShift(true);
	car.SetAutoClutch(true);
}

//  to be called inside a test's main loop
void PERFORMANCE_TESTING::SimulateFlatRoad()
{
	//simulate an infinite, flat road
	for (int i = 0; i < 4; ++i)
	{
		MATHVECTOR<float,3> wp = car.dynamics.GetWheelPosition(WHEEL_POSITION(i));
		float depth = wp[2] - car.GetTireRadius(WHEEL_POSITION(i));
		MATHVECTOR<float,3> pos(wp[0], wp[1], 0);
		MATHVECTOR<float,3> norm(0, 0, 1);
		car.GetWheelContact(WHEEL_POSITION(i)).Set(pos, norm, depth, &surface, NULL, NULL);
	}
}


///  Init
//------------------------------------------------------------------------------------------------------------
void PERFORMANCE_TESTING::Test(const string & carpath, class App* pApp,
	ostream & info_output, ostream & error_output)
{
	car.pApp = pApp;		car.pGame = pApp->pGame;
	car.pSet = pApp->pSet;	car.dynamics.pSet = pApp->pSet;
	info_output << "----- Car performance test start for: " << carpath << endl;

	//  load car dynamics
	CONFIGFILE cf;
	if (!cf.Load(carpath))  {
		error_output << "Error loading car configuration file" << endl;  return;  }

	if (!car.dynamics.Load(car.pGame, cf, error_output))  {
		error_output << "Error during car dynamics load" << endl;  return;  }
	
	info_output << " Car Summary:  " << carpath << "\n" << setprecision(0) <<
		"Total mass [kg]: " << car.dynamics.GetMass() << "\n" << setprecision(3) <<
		"Center of mass [m] L,W,H: " << car.dynamics.center_of_mass << endl;

	//  CARDYNAMICS::Init  stuff
	MATHVECTOR<Dbl,3> zero(0,0,0);  QUATERNION<Dbl> orientation;
	//car.dynamics.body.SetPosition(position);
	car.dynamics.body.SetOrientation(orientation);
	car.dynamics.body.SetInitialForce(zero);
	car.dynamics.body.SetInitialTorque(zero);
	car.dynamics.engine.SetInitialConditions();
	
	for (int i = 0; i < WHEEL_POSITION_SIZE; i++)
	{
		car.dynamics.wheel[WHEEL_POSITION(i)].SetInitialConditions();
		car.dynamics.wheel_velocity[i].Set(0.0);
		car.dynamics.wheel_position[i] = car.dynamics.GetWheelPositionAtDisplacement(WHEEL_POSITION(i), 0);
		car.dynamics.wheel_orientation[i] = orientation * car.dynamics.GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION(i));
	}
	car.dynamics.UpdateWheelTransform();
	//car.dynamics.AlignWithGround();//--

	//  tests  -----
	TestMaxSpeed(info_output, error_output);
	//TestStoppingDistance(false, info_output, error_output);
	//TestStoppingDistance(true, info_output, error_output);

	info_output << "-----  Car performance test complete" << endl;
}


///  Test
//------------------------------------------------------------------------------------------------------------
void PERFORMANCE_TESTING::TestMaxSpeed(ostream & info_output, ostream & error_output)
{
	info_output << "Testing maximum speed" << endl;
	ResetCar();

	double t = 0.0, dt = 0.004, maxtime = 300.0;
	int i = 0;  const int ii = (int)(0.1/dt);  // write interval

	vector <float> inputs(CARINPUT::ALL, 0.0);
	inputs[CARINPUT::THROTTLE] = 1.0;

	pair <float, float> maxspeed;  maxspeed.first = 0;  maxspeed.second = 0;
	float lastsecondspeed = 0;
	float stopthreshold = 0.001;  // if the accel (in m/s^2) is less than this value, discontinue the testing

	float timeto100start = 0;  // don't start the 0-100 kmh clock until the car is moving at a threshold speed to account for the crappy launch that the autoclutch gives
	//float timeto100startthreshold = 2.23;  // threshold speed to start 0-60 clock in m/s
	float timeto100startthreshold = 0.01;  // threshold speed to start 0-60 clock in m/s
	float timeto100 = maxtime, timeto30 = maxtime, timeto60 = maxtime, timeto160 = maxtime, timeto200 = maxtime;

	float timetoquarter = maxtime;
	float quarterspeed = 0;
	
	string downforcestr = "N/A";
	
	while (t < maxtime)
	{
		SimulateFlatRoad();

		if (t == 0)	inputs[CARINPUT::SHIFT_UP] = 1.f;
		else		inputs[CARINPUT::SHIFT_UP] = 0.f;
		inputs[CARINPUT::THROTTLE] = 1.0;

		car.dynamics.Tick(dt);

		car.HandleInputs(inputs, dt);


		double vel = car.dynamics.GetSpeed();
		
		if (vel > maxspeed.second)
		{
			maxspeed.first = t;
			maxspeed.second = vel;
			stringstream dfs;
			dfs << -car.GetTotalAero()[2] << " N; " << -car.GetTotalAero()[2]/car.GetTotalAero()[0] << ":1 lift/drag";
			downforcestr = dfs.str();
		}

		if (vel < timeto100startthreshold)
			timeto100start = t;

		if (vel < 100/3.6)  timeto100 = t;
		if (vel <  30/3.6)  timeto30 = t;
		if (vel <  60/3.6)  timeto60 = t;
		if (vel < 160/3.6)  timeto160 = t;
		if (vel < 200/3.6)  timeto200 = t;

		if (car.dynamics.GetCenterOfMassPosition().Magnitude() > 402.3 && timetoquarter == maxtime)
		{
			//quarter mile!
			timetoquarter = t - timeto100start;
			quarterspeed = vel;
		}
		/* fixme
		car.remaining_shift_time-=dt;
		if (car.remaining_shift_time < 0)
			car.remaining_shift_time = 0;
		*/

		if (i % ii == 0)  // every second
		{
			#if 1
			if (vel - lastsecondspeed < stopthreshold && vel > 200/3.6)  {
				//info_output << "Maximum speed attained at " << maxspeed.first << " s" << endl;
				break;  }
			if (car.GetEngineRPM() < 1)  {
				error_output << "Car stalled during launch, t=" << t << endl;
				break;  }
			#endif

			///
			info_output << fToStr(t,1,4) << "s, " << fToStr(vel*3.6, 1,5) << " d " << fToStr((lastsecondspeed - vel)*3.6, 1,5) << " kmh, gear " << car.dynamics.transmission.GetGear() << ", rpm " << fToStr(car.GetEngineRPM(),0,4)
				//<< ", sli " << fToStr(car.dynamics.tire[0].slide, 1,4)
				//<< ", slp " << fToStr(car.dynamics.tire[1].slide, 1,4)
				<< endl;
			lastsecondspeed = vel;
		}

		t += dt;
		i++;
	}

	info_output << "Maximum speed: " << fToStr(maxspeed.second*3.6, 1,4) << " kmh at " << maxspeed.first << " s" << endl;
	info_output << "Downforce at max speed: " << downforcestr << endl;
	info_output << "time to  30 kmh: " << timeto30-timeto100start << " s" << endl;
	info_output << "time to  60 kmh: " << timeto60-timeto100start << " s" << endl;
	info_output << "time to 100 kmh: " << timeto100-timeto100start << " s" << endl;
	info_output << "time to 160 kmh: " << timeto160-timeto100start << " s" << endl;
	info_output << "time to 200 kmh: " << timeto200-timeto100start << " s" << endl;
	info_output << "1/4 mile time: " << timetoquarter << " s" << " at " << fToStr(quarterspeed*3.6, 1,4) << " kmh" << endl;
}


///  Test stop
//------------------------------------------------------------------------------------------------------------
void PERFORMANCE_TESTING::TestStoppingDistance(bool abs, ostream & info_output, ostream & error_output)
{
	info_output << "Testing stopping distance" << endl;
	ResetCar();
	car.dynamics.SetABS(abs);

	double t = 0.0, dt = 0.004, maxtime = 300.0;
	int i = 0;  const int ii = (int)(1.0/dt);  // write interval

	vector <float> inputs(CARINPUT::ALL, 0.0);

	inputs[CARINPUT::THROTTLE] = 1.0;

	float stopthreshold = 0.1; //if the speed (in m/s) is less than this value, discontinue the testing
	MATHVECTOR<double,3> stopstart; //where the stopping starts
	float brakestartspeed = 26.82; //speed at which to start braking, in m/s (26.82 m/s is 60 mph)

	bool accelerating = true; //switches to false once 60 mph is reached

	while (t < maxtime)
	{
		SimulateFlatRoad();

		if (t == 0)	inputs[CARINPUT::SHIFT_UP] = 1.f;
		else		inputs[CARINPUT::SHIFT_UP] = 0.f;

		if (accelerating)
		{	inputs[CARINPUT::THROTTLE] = 1.0;
			inputs[CARINPUT::BRAKE] = 0.0;
		}else
		{	inputs[CARINPUT::THROTTLE] = 0.0;
			inputs[CARINPUT::BRAKE] = 1.0;
			//inputs[CARINPUT::NEUTRAL] = 1.0;
		}

		car.HandleInputs(inputs, dt);

		car.dynamics.Tick(dt);

		double vel = car.dynamics.GetSpeed();

		if (vel >= brakestartspeed && accelerating)  // stop accelerating and hit the brakes
		{
			accelerating = false;
			stopstart = car.dynamics.GetWheelPosition(WHEEL_POSITION(0));
			//info_output  << "hitting the brakes at " << t << ", " << vel << endl;
		}

		if (!accelerating && vel < stopthreshold)
			break;

		if (car.GetEngineRPM() < 1)  {
			error_output << "Car stalled during launch, t=" << t << endl;
			break;  }
			
		/* fixme
		car.remaining_shift_time-=dt;
		if (car.remaining_shift_time < 0)
			car.remaining_shift_time = 0;
		*/

		///
		if (i % ii == 0)  // every second
			info_output  << fToStr(t,0,2) << "s, " << fToStr(vel*3.6,1,4) << " kmh, gear " << car.GetGear() << ", rpm " << fToStr(car.GetEngineRPM(),0,4) << endl;

		t += dt;
		++i;
	}

	MATHVECTOR<double,3> stopend = car.dynamics.GetWheelPosition(WHEEL_POSITION(0));

	info_output << "60-0 stopping distance ";
	if (abs)	info_output << "(ABS)";
	else		info_output << "(no ABS)";
	info_output << ": " << (stopend-stopstart).Magnitude() << " m" << endl;
}
