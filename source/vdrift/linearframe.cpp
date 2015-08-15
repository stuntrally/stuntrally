#include "pch.h"
#include "linearframe.h"
#include "unittest.h"

//#include <iostream>
using std::cout;
using std::endl;

QT_TEST(linearframe_test)
{
	LINEARFRAME frame;
	frame.SetMass(1.0);
	MATHVECTOR<double,3> initpos;
	initpos.Set(0,0,0);
	frame.SetPosition(initpos);
	MATHVECTOR<double,3> initv;
	initv.Set(0,65,0);
	frame.SetVelocity(initv);
	MATHVECTOR<double,3> gravity;
	gravity.Set(0,-9.81,0);
	frame.SetInitialForce(gravity);
	
	double t = 0.0;
	
	//integrate for 10 seconds
	for (int i = 0; i < 1000; ++i)
	{
		frame.Integrate1(0.01);
		frame.ApplyForce(gravity);
		frame.Integrate2(0.01);
		t += 0.01;
	}
	
	/*cout << "t = " << t << endl;
	cout << "Calculated Position: " << frame.GetPosition() << endl;
	//cout << "Velocity: " << frame.GetVelocity() << endl;
	cout << "Expected Position: " << initv * t + gravity * t * t *0.5 << endl;*/
	
	QT_CHECK_CLOSE(frame.GetPosition()[1], (initv * t + gravity * t * t *0.5)[1], 0.0001);
	
	
	frame.SetMass(1.0);
	initpos.Set(0,0,0);
	frame.SetPosition(initpos);
	initv.Set(0,0,0);
	frame.SetVelocity(initv);
	MATHVECTOR<double,3> force;
	force.Set(0,0,0);
	frame.SetInitialForce(force);
	
	t = 0.0;
	
	//integrate for 10 seconds
	for (int i = 0; i < 1000; ++i)
	{
		frame.Integrate1(0.01);
		force.Set(0,1,0);
		force = force - frame.GetVelocity() * 10.0f;
		frame.ApplyForce(force);
		frame.Integrate2(0.01);
		t += 0.01;
	}
	
	//cout << "Velocity: " << frame.GetVelocity() << endl;
	
	QT_CHECK_CLOSE(frame.GetVelocity()[1], 0.1, 0.0001);
}
