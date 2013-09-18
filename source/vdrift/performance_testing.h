#pragma once
#include "car.h"

//#include <ostream>
//#include <string>

class PERFORMANCE_TESTING
{
//private:
public:
	CAR car;
	TRACKSURFACE surface;
	std::string carstate;
	void SimulateFlatRoad();
	void ResetCar();
	void TestMaxSpeed(std::ostream & info_output, std::ostream & error_output);
	void TestStoppingDistance(bool abs, std::ostream & info_output, std::ostream & error_output);
	
public:
	PERFORMANCE_TESTING();
	void Test(const std::string & carpath, class App* pApp, std::ostream & info_output, std::ostream & error_output);
};
