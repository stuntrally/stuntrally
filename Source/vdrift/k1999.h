////////////////////////////////////////////////////////////////////////////
//
// Using Remi Coulom's K1999 Path-Optimisation Algorithm to calculate
// racing line.
//
// This is an adaption of Remi Coulom's K1999 driver for TORCS
//
////////////////////////////////////////////////////////////////////////////

#ifndef _K1999_H
#define _K1999_H

//#include <vector>
//#include <iostream>
//#include <fstream>

class ROADSTRIP;

class K1999
{
private:
	std::vector <double> tx;
	std::vector <double> ty;
	std::vector <double> tRInverse;
	std::vector <double> txLeft;
	std::vector <double> tyLeft;
	std::vector <double> txRight;
	std::vector <double> tyRight;
	std::vector <double> tLane;
	int Divs;

	void UpdateTxTy(int i);
	double GetRInverse(int prev, double x, double y, int next);
	void AdjustRadius(int prev, int i, int next, double TargetRInverse, double Security = 0);
	void Smooth(int Step);
	void StepInterpolate(int iMin, int iMax, int Step);
	void Interpolate(int Step);

#ifdef DRAWPATH
	void DrawPath(std::ostream &out);
#endif

public:
	bool LoadData(ROADSTRIP* road);
	void CalcRaceLine(); 
	void UpdateRoadStrip(ROADSTRIP* road);
};

#endif //_K1999_H
