#include "pch.h"
#include "dbl.h"
#include "quaternion.h"
#include "unittest.h"
#include "../ogre/common/Def_Str.h"

//#include <iostream>
using std::ostream;

QT_TEST(quaternion_test)
{
	QUATERNION<float> test1;
	QT_CHECK_EQUAL(test1.x(),0.0);
	QT_CHECK_EQUAL(test1.y(),0.0);
	QT_CHECK_EQUAL(test1.z(),0.0);
	QT_CHECK_EQUAL(test1.w(),1.0);
	QUATERNION<float> test2;
	test2.LoadIdentity();
	QT_CHECK_EQUAL(test1,test2);
	test1.w() = 0.7;
	test1.Normalize();
	QT_CHECK_EQUAL(test1.Magnitude(),1.0);
	float mat[16];
	test1.GetMatrix4(mat);
	QT_CHECK_EQUAL(mat[0],1.0);
	QT_CHECK_EQUAL(mat[1],0.0);
	QT_CHECK_EQUAL(mat[2],0.0);
	QT_CHECK_EQUAL(mat[3],0.0);
	QT_CHECK_EQUAL(mat[4],0.0);
	QT_CHECK_EQUAL(mat[5],1.0);
	QT_CHECK_EQUAL(mat[6],0.0);
	QT_CHECK_EQUAL(mat[7],0.0);
	QT_CHECK_EQUAL(mat[8],0.0);
	QT_CHECK_EQUAL(mat[9],0.0);
	QT_CHECK_EQUAL(mat[10],1.0);
	QT_CHECK_EQUAL(mat[11],0.0);
	QT_CHECK_EQUAL(mat[12],0.0);
	QT_CHECK_EQUAL(mat[13],0.0);
	QT_CHECK_EQUAL(mat[14],0.0);
	QT_CHECK_EQUAL(mat[15],1.0);
	
	float vec[3];
	vec[0] = 0;
	vec[1] = 0;
	vec[2] = 1;
	test1.LoadIdentity();
	test1.Rotate(PI_d*0.5, 0.0, 1.0, 0.0);
	test1.RotateVector(vec);
	QT_CHECK_CLOSE(vec[0], 1.0, 0.001);
	QT_CHECK_CLOSE(vec[1], 0.0, 0.001);
	QT_CHECK_CLOSE(vec[2], 0.0, 0.001);
	//std::cout << vec[0] << "," << vec[1] << "," << vec[2] << std::endl;
	
	test2.LoadIdentity();
	test1.Rotate(PI_d*0.5, 0.0, 0.0, 1.0);
	QT_CHECK_CLOSE(test1.GetAngleBetween(test2),PI_d*0.5,0.001);
	
	test1.LoadIdentity();
	test1.Rotate(PI_d*0.75, 0.0, 1.0, 0.0);
	test2.LoadIdentity();
	test2.Rotate(PI_d*0.25, 0.0, 1.0, 0.0);
	
	vec[0] = 0;
	vec[1] = 0;
	vec[2] = 1;
	test1.QuatSlerp(test2, 0.5).RotateVector(vec);
	QT_CHECK_CLOSE(vec[0], 1.0, 0.001);
	QT_CHECK_CLOSE(vec[1], 0.0, 0.001);
	QT_CHECK_CLOSE(vec[2], 0.0, 0.001);
}
