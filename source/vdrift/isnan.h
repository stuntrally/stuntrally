#pragma once

#ifdef _WIN32
	static bool isnan(float number)  {  return (number != number);  }
	static bool isnan(double number) {  return (number != number);  }
#else  // gcc, c++11
	#include <cmath>
	using std::isnan;
#endif
