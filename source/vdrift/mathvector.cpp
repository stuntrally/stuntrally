#include "pch.h"
#include "mathvector.h"
#include "unittest.h"

//#include <iostream>
using std::ostream;

QT_TEST(mathvector_test)
{
	{
		MATHVECTOR<int, 1> test1(8);
		MATHVECTOR<int, 1> test2(2);
		test2.Set(5);
		MATHVECTOR<int, 1> test3;
		test3 = test1 + test2;
		QT_CHECK_EQUAL(test3[0], 13);
	}
	
	{
		MATHVECTOR<float, 2> test1(0);
		test1.Set(3,4);
		MATHVECTOR<float, 2> test2(1);
		MATHVECTOR<float, 2> test3 (test1-test2);
		QT_CHECK_EQUAL(test3[0], 2);
		QT_CHECK_EQUAL(test3[1], 3);
	}
	
	{
		MATHVECTOR<float,3> test1;
		test1.Set(1,2,3);
		MATHVECTOR<float,3> test2(1);
		MATHVECTOR<float,3> test3 (test1+test2);
		QT_CHECK_EQUAL(test3[0], 2);
		QT_CHECK_EQUAL(test3[1], 3);
		QT_CHECK_EQUAL(test3[2], 4);
		
		test3 = test1;
		QT_CHECK(test1 == test3);
		QT_CHECK(test1 != test2);
		
		test3 = -test1;
		MATHVECTOR<float,3> answer;
		answer.Set(-1,-2,-3);
		QT_CHECK_EQUAL(test3,answer);
	}
	
	{
		MATHVECTOR<float,3> test1;
		test1.Set(1,2,3);
		MATHVECTOR<float,3> testcopy(test1);
		QT_CHECK_EQUAL(test1,testcopy);
		float v3[3];
		for (int i = 0; i < 3; ++i)
			v3[i] = i + 1;
		testcopy.Set(v3);
		QT_CHECK_EQUAL(test1,testcopy);
		MATHVECTOR<float,3> add1;
		add1.Set(1,1,1);
		for (int i = 0; i < 3; ++i)
			testcopy[i] = i;
		testcopy = testcopy + add1;
		QT_CHECK_EQUAL(test1,testcopy);
		for (int i = 0; i < 3; ++i)
			testcopy[i] = i+2;
		testcopy = testcopy - add1;
		QT_CHECK_EQUAL(test1,testcopy);
		testcopy = testcopy * 1.0;
		QT_CHECK_EQUAL(test1,testcopy);
		testcopy = testcopy / 1.0;
		QT_CHECK_EQUAL(test1,testcopy);
		QT_CHECK(test1 == testcopy);
		QT_CHECK(!(test1 == add1));
		QT_CHECK(test1 != add1);
		QT_CHECK(!(test1 != testcopy));
		for (int i = 0; i < 3; ++i)
			testcopy[i] = -(i+1);
		testcopy = -testcopy;
		MATHVECTOR<float,3> zero(0);
		for (int i = 0; i < 3; ++i)
			QT_CHECK_EQUAL(zero[i],0);
		QT_CHECK_EQUAL(test1,testcopy);
		testcopy.Set(0.0);
		QT_CHECK_EQUAL(testcopy,zero);
		testcopy = test1;
		QT_CHECK_EQUAL(testcopy,test1);
		QT_CHECK_CLOSE(test1.MagnitudeSquared(),14.0,0.001);
		QT_CHECK_CLOSE(test1.Magnitude(),3.741657,0.001);
		
		QT_CHECK_CLOSE(test1.Normalize()[0],0.267261,0.001);
		QT_CHECK_CLOSE(test1.Normalize()[1],0.534522,0.001);
		
		MATHVECTOR<float,3> test2;
		for (int i = 0; i < 3; ++i)
			QT_CHECK_EQUAL(test2[i],0);
		test2.Set(2,3,4);
		QT_CHECK_CLOSE(test1.dot(test2),20.0,0.001);
		
		test1.Set(1,-1,-2);
		test1.absify();
		QT_CHECK_EQUAL(test1,(MATHVECTOR<float,3>(1,1,2)));
	}
	
	{
		MATHVECTOR<float,3> test1;
		test1.Set(1,2,3);
		MATHVECTOR<float,3> test2;
		test2.Set(4,5,6);
		MATHVECTOR<float,3> answer;
		answer.Set(-3,6,-3);
		QT_CHECK_EQUAL(test1.cross(test2), answer);
	}
}
