
#include "coordinatesystems.h"

void COORDINATESYSTEMS::ConvertCarCoordinateSystemV2toV1(float & x, float & y, float & z)
{
	float tempx = x;
	float tempy = y;
	float tempz = z;
	
	x = tempy;
	y = -tempx;
	z = tempz;
}
