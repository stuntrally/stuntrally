#include "pch.h"
#include "rigidbody.h"
#include "unittest.h"
#include "../ogre/common/Def_Str.h"

//#include <iostream>
using std::cout;
using std::endl;

QT_TEST(rigidbody_test)
{
	RIGIDBODY body;
	MATHVECTOR<Dbl,3> initpos;
	QUATERNION<Dbl> quat;
	initpos.Set(0,0,10);
	body.SetPosition(initpos);
	quat.Rotate(-PI_d*0.5, 1, 0, 0);
	body.SetOrientation(quat);

	MATHVECTOR<float,3> localcoords;
	localcoords.Set(0,0,1);
	MATHVECTOR<float,3> expected;
	expected.Set(0,1,10);
	MATHVECTOR<float,3> pos = body.TransformLocalToWorld(localcoords);
	QT_CHECK_CLOSE(pos[0], expected[0], 0.0001);
	QT_CHECK_CLOSE(pos[1], expected[1], 0.0001);
	QT_CHECK_CLOSE(pos[2], expected[2], 0.0001);
	
	QT_CHECK_CLOSE(body.TransformWorldToLocal(pos)[0], localcoords[0], 0.0001);
	QT_CHECK_CLOSE(body.TransformWorldToLocal(pos)[1], localcoords[1], 0.0001);
	QT_CHECK_CLOSE(body.TransformWorldToLocal(pos)[2], localcoords[2], 0.0001);
}
