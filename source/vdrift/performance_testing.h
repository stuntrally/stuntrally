#ifndef _PERFORMANCE_TESTING_H
#define _PERFORMANCE_TESTING_H

#include "car.h"

//#include <ostream>
//#include <string>
//#include <sstream>

class PERFORMANCE_TESTING
{
private:
	CAR car;
	TRACKSURFACE surface;
	std::string carstate;
	void SimulateFlatRoad();
	void ResetCar();
	void TestMaxSpeed(std::ostream & info_output, std::ostream & error_output);
	void TestStoppingDistance(bool abs, std::ostream & info_output, std::ostream & error_output);
	float ConvertToMPH(float ms) {return ms*2.23693629;}
	float ConvertToFeet(float meters) {return meters*3.2808399;}
	
public:
	PERFORMANCE_TESTING();
	void Test(const std::string & carpath, class GAME* pGame, std::ostream & info_output, std::ostream & error_output);
};

#endif
