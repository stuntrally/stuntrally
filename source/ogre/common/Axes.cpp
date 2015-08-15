#include "pch.h"
#include "Axes.h"
#include "../../vdrift/dbl.h"
using namespace Ogre;
using half_float::half;


//  transform axes, vdrift to ogre  car & wheels
//-----------------------------------------------------------------------
Quaternion Axes::qFixCar, Axes::qFixWh;

void Axes::Init()
{
	Quaternion qr;  {
	QUATERNION<double> fix;  fix.Rotate(PI_d, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixCar = qr;  }
	QUATERNION<double> fix;  fix.Rotate(PI_d/2, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixWh = qr;
}

void Axes::toOgre(Vector3& vOut, const MATHVECTOR<float,3>& vIn)
{
	vOut.x = vIn[0];  vOut.y = vIn[2];  vOut.z = -vIn[1];
}
Vector3 Axes::toOgre(const MATHVECTOR<float,3>& vIn)
{
	return Vector3(vIn[0], vIn[2], -vIn[1]);
}

//  car
Quaternion Axes::toOgre(const QUATERNION<float>& vIn)
{
	Quaternion q(vIn[0], -vIn[3], vIn[1], vIn[2]);
	return q * qFixCar;
}
Quaternion Axes::toOgre(const QUATERNION<double>& vIn)
{
	Quaternion q(vIn[0], -vIn[3], vIn[1], vIn[2]);
	return q * qFixCar;
}

//  wheels
Quaternion Axes::toOgreW(const QUATERNION<half>& vIn)
{
	Quaternion q(vIn[0], -vIn[3], vIn[1], vIn[2]);
	return q * qFixWh;
}
Quaternion Axes::toOgreW(const QUATERNION<float>& vIn)
{
	Quaternion q(vIn[0], -vIn[3], vIn[1], vIn[2]);
	return q * qFixWh;
}
Quaternion Axes::toOgreW(const QUATERNION<double>& vIn)
{
	Quaternion q(vIn[0], -vIn[3], vIn[1], vIn[2]);
	return q * qFixWh;
}
