#include "pch.h"
#include "rotationalframe.h"
#include "unittest.h"

//#include <iostream>
using std::cout;
using std::endl;

QT_TEST(rotationalframe_test)
{
	{
	ROTATIONALFRAME frame;
	//frame.SetInertia(1.0);
	QUATERNION<double> initorient;
	frame.SetOrientation(initorient);
	MATHVECTOR<double,3> initv;
	initv.Set(0,0,0);
	frame.SetAngularVelocity(initv);
	MATHVECTOR<double,3> torque;
	torque.Set(0,0,0);
	frame.SetInitialTorque(torque);
	
	//integrate for 10 seconds
	for (int i = 0; i < 1000; ++i)
	{
		frame.Integrate1(0.01);
		torque.Set(0,1,0);
		torque = torque - frame.GetAngularVelocity() * 10.0f;
			frame.ApplyTorque(torque);
		frame.Integrate2(0.01);
	}
	
	/*cout << "t = " << t << endl;
	cout << "Calculated Orientation: " << frame.GetOrientation() << endl;
	cout << "Calculated Velocity: " << frame.GetAngularVelocity() << endl;*/
	
	QT_CHECK_CLOSE(frame.GetAngularVelocity()[1], 0.1, 0.0001);
	}
	
	{
		ROTATIONALFRAME frame;
		//frame.SetInertia(1.0);
		QUATERNION<double> initorient;
		frame.SetOrientation(initorient);
		MATHVECTOR<double,3> initv;
		initv.Set(0,0,0);
		frame.SetAngularVelocity(initv);
		MATHVECTOR<double,3> torque;
		torque.Set(0,1,0);
		frame.SetInitialTorque(torque);
		MATRIX3 <double> inertia;
		inertia.Scale(0.1);
		frame.SetInertia(inertia);
		
		//integrate for 10 seconds
		for (int i = 0; i < 1000; ++i)
		{
			frame.Integrate1(0.01);
			frame.ApplyTorque(torque);
			frame.Integrate2(0.01);
		}
		
		//cout << "t = " << t << endl;
		//cout << "Calculated Orientation: " << frame.GetOrientation() << endl;
		//cout << "Calculated Velocity: " << frame.GetAngularVelocity() << endl;
			
		//QT_CHECK_CLOSE(frame.GetAngularVelocity()[1], 0.1, 0.0001);
		QT_CHECK_CLOSE(frame.GetAngularVelocity()[1], 100., 0.0001);
	}
}
