#include "pch.h"

////////////////////////////////////////////////////////////////////////////
//
// Using Remi Coulom's K1999 Path-Optimisation Algorithm to calculate
// racing line.
//
// This is an adaption of Remi Coulom's K1999 driver for TORCS
//
////////////////////////////////////////////////////////////////////////////

#include "k1999.h"
#include "track.h"

//#include <cassert>

#define SecurityR  100.0 // Security radius
#define SideDistExt  2.0 // Security distance wrt outside
#define SideDistInt  1.0 // Security distance wrt inside
#define Iterations   100 // Number of smoothing operations
#define Mag(x,y) sqrt((x)*(x)+(y)*(y))
#define Min(X,Y) ((X)<(Y)?(X):(Y))
#define Max(X,Y) ((X)>(Y)?(X):(Y))

/////////////////////////////////////////////////////////////////////////////
// Update tx and ty arrays
/////////////////////////////////////////////////////////////////////////////
void K1999::UpdateTxTy(int i)
{
 tx[i] = tLane[i] * txRight[i] + (1 - tLane[i]) * txLeft[i];
 ty[i] = tLane[i] * tyRight[i] + (1 - tLane[i]) * tyLeft[i];
}                                                                               

/////////////////////////////////////////////////////////////////////////////
// Draw a path (use gnuplot)
/////////////////////////////////////////////////////////////////////////////
#ifdef DRAWPATH
void K1999::DrawPath(std::ostream &out)
{
 for (int i = 0; i <= Divs; i++)
 {
  int j = i % Divs;
  out << txLeft[j] << ' ' << tyLeft[j] << ' ';
  out << tx[j] << ' ' << ty[j] << ' ';
  out << txRight[j] << ' ' << tyRight[j] << ' ';
  out << tLane[j] << ' ' << tRInverse[j] << '\n';
 }
 out << '\n';
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Compute the inverse of the radius
/////////////////////////////////////////////////////////////////////////////
double K1999::GetRInverse(int prev, double x, double y, int next)
{
 double x1 = tx[next] - x;
 double y1 = ty[next] - y;
 double x2 = tx[prev] - x;
 double y2 = ty[prev] - y;
 double x3 = tx[next] - tx[prev];
 double y3 = ty[next] - ty[prev];
 
 double det = x1 * y2 - x2 * y1;
 double n1 = x1 * x1 + y1 * y1;
 double n2 = x2 * x2 + y2 * y2;
 double n3 = x3 * x3 + y3 * y3;
 double nnn = sqrt(n1 * n2 * n3);
 
 double c = 2 * det / nnn;
 return c;
}

/////////////////////////////////////////////////////////////////////////////
// Change lane value to reach a given radius
/////////////////////////////////////////////////////////////////////////////
void K1999::AdjustRadius(int prev, int i, int next, double TargetRInverse, double Security)
{
 double OldLane = tLane[i];

 double Width = Mag((txLeft[i]-txRight[i]),(tyLeft[i]-tyRight[i]));

 //
 // Start by aligning points for a reasonable initial lane
 //
 tLane[i] = (-(ty[next] - ty[prev]) * (txLeft[i] - tx[prev]) +
              (tx[next] - tx[prev]) * (tyLeft[i] - ty[prev])) /
            ( (ty[next] - ty[prev]) * (txRight[i] - txLeft[i]) -
              (tx[next] - tx[prev]) * (tyRight[i] - tyLeft[i]));
 // the original algorithm allows going outside the track 
 /*
 if (tLane[i] < -0.2)
  tLane[i] = -0.2;
 else if (tLane[i] > 1.2)
  tLane[i] = 1.2;
 */
 if (tLane[i] < 0.0)
  tLane[i] = 0.0;
 else if (tLane[i] > 1.0)
  tLane[i] = 1.0;

 UpdateTxTy(i);
 
 //
 // Newton-like resolution method
 //
 const double dLane = 0.0001;
 
 double dx = dLane * (txRight[i] - txLeft[i]);
 double dy = dLane * (tyRight[i] - tyLeft[i]);
 
 double dRInverse = GetRInverse(prev, tx[i] + dx, ty[i] + dy, next);
 
 if (dRInverse > 0.000000001)
 {
  tLane[i] += (dLane / dRInverse) * TargetRInverse;
 
  double ExtLane = (SideDistExt + Security) / Width;
  double IntLane = (SideDistInt + Security) / Width;
  if (ExtLane > 0.5)
   ExtLane = 0.5;
  if (IntLane > 0.5)
   IntLane = 0.5;
 
  if (TargetRInverse >= 0.0)
  {
   if (tLane[i] < IntLane)
    tLane[i] = IntLane;
   if (1 - tLane[i] < ExtLane)
   {
    if (1 - OldLane < ExtLane)
     tLane[i] = Min(OldLane, tLane[i]);
    else
     tLane[i] = 1 - ExtLane;
   }
  }
  else
  {
   if (tLane[i] < ExtLane)
   {
    if (OldLane < ExtLane)
     tLane[i] = Max(OldLane, tLane[i]);
    else
     tLane[i] = ExtLane;
   }
   if (1 - tLane[i] < IntLane)
    tLane[i] = 1 - IntLane;
  }
 }
 
 UpdateTxTy(i);
}

/////////////////////////////////////////////////////////////////////////////
// Smooth path
/////////////////////////////////////////////////////////////////////////////
void K1999::Smooth(int Step)
{
 int prev = ((Divs - Step) / Step) * Step;
 int prevprev = prev - Step;
 int next = Step;
 int nextnext = next + Step;
 
 assert(prev >= 0);
 //std::cout << Divs << ", " << Step << ", " << prev << ", " << tx.size() << std::endl;
 assert(prev < (int)tx.size());
 assert(prev < (int)ty.size());
 assert(next < (int)tx.size());
 assert(next < (int)ty.size());
 
 for (int i = 0; i <= Divs - Step; i += Step)
 {
  double ri0 = GetRInverse(prevprev, tx[prev], ty[prev], i);
  double ri1 = GetRInverse(i, tx[next], ty[next], nextnext);
  double lPrev = Mag(tx[i] - tx[prev], ty[i] - ty[prev]);
  double lNext = Mag(tx[i] - tx[next], ty[i] - ty[next]);

  double TargetRInverse = (lNext * ri0 + lPrev * ri1) / (lNext + lPrev);

  double Security = lPrev * lNext / (8 * SecurityR);
  AdjustRadius(prev, i, next, TargetRInverse, Security);
 
  prevprev = prev;
  prev = i;
  next = nextnext;
  nextnext = next + Step;
  if (nextnext > Divs - Step)
   nextnext = 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Interpolate between two control points
/////////////////////////////////////////////////////////////////////////////
void K1999::StepInterpolate(int iMin, int iMax, int Step)
{
 int next = (iMax + Step) % Divs;
 if (next > Divs - Step)
  next = 0;
 
 int prev = (((Divs + iMin - Step) % Divs) / Step) * Step;
 if (prev > Divs - Step)
  prev -= Step;
 
 double ir0 = GetRInverse(prev, tx[iMin], ty[iMin], iMax % Divs);
 double ir1 = GetRInverse(iMin, tx[iMax % Divs], ty[iMax % Divs], next);
 for (int k = iMax; --k > iMin;)
 {
  double x = double(k - iMin) / double(iMax - iMin);
  double TargetRInverse = x * ir1 + (1 - x) * ir0;
  AdjustRadius(iMin, k, iMax % Divs, TargetRInverse);
 }
}
 
/////////////////////////////////////////////////////////////////////////////
// Calls to StepInterpolate for the full path
/////////////////////////////////////////////////////////////////////////////
void K1999::Interpolate(int Step)
{
 if (Step > 1)
 {
  int i;
  for (i = Step; i <= Divs - Step; i += Step)
   StepInterpolate(i - Step, i, Step);
  StepInterpolate(i - Step, Divs, Step);
 }
}

void K1999::CalcRaceLine()
{
	const unsigned int stepsize = 128;
	
	//abort if the track isn't long enough
	if (tx.size() < stepsize)
		return;
	
 //
 // Smoothing loop
 //
 for (int Step = stepsize; (Step /= 2) > 0;)
 {
  for (int i = Iterations * int(sqrt(float(Step))); --i >= 0;)
   Smooth(Step);
  Interpolate(Step);
 }
 
 //
 // Compute curvature along the path
 //
 for (int i = Divs; --i >= 0;)
 {
  int next = (i + 1) % Divs;
  int prev = (i - 1 + Divs) % Divs;

  double rInverse = GetRInverse(prev, tx[i], ty[i], next);
  tRInverse[i] = rInverse;
 }

#ifdef DRAWPATH
 std::ofstream ofs("k1999.path");
 DrawPath(ofs);
#endif
} 


//
//VDrift specific functions below
//

bool K1999::LoadData(ROADSTRIP* road)
{
	tx.clear();
	ty.clear();
	tRInverse.clear();
	txLeft.clear();
	tyLeft.clear();
	txRight.clear();
	tyRight.clear();
	tLane.clear();

	const std::list <ROADPATCH> & patchlist = road->GetPatchList();
	Divs = patchlist.size();
	
	int count = 0;

	for (std::list <ROADPATCH>::const_iterator i = patchlist.begin(); i != patchlist.end(); ++i)
	{
		txLeft.push_back(i->GetPatch().GetPoint(3,0)[0]);
		tyLeft.push_back(-i->GetPatch().GetPoint(3,0)[2]);//this originally looked at the z coordinate -JDV
		txRight.push_back(i->GetPatch().GetPoint(3,3)[0]);
		tyRight.push_back(-i->GetPatch().GetPoint(3,3)[2]);//this originally looked at the z coordinate -JDV
		tLane.push_back(0.5);
		tx.push_back(0.0);
		ty.push_back(0.0);
		tRInverse.push_back(0.0);
		UpdateTxTy(count);
		
		count++;
	}

	if (road->GetClosed()) //a closed circuit
		return true;
	else
		return false;
}

void K1999::UpdateRoadStrip(ROADSTRIP* road)
{
	std::list <ROADPATCH> & patchlist = road->GetPatchList();
	int count = 0;
	
	for (std::list <ROADPATCH>::iterator i = patchlist.begin(); i != patchlist.end(); ++i)
	{
		i->SetTrackCurvature(tRInverse[count]);
		i->SetRacingLine(i->GetPatch().GetPoint(3,0)*(1.0-tLane[count]) + i->GetPatch().GetPoint(3,3)*(tLane[count]));
		//std::cout << i->GetPatch().GetPoint(3,0)*(1.0-tLane[count]) + i->GetPatch().GetPoint(3,3)*(tLane[count]) << std::endl;
		
		count++;
	}

	tx.clear();
	ty.clear();
	tRInverse.clear();
	txLeft.clear();
	tyLeft.clear();
	txRight.clear();
	tyRight.clear();
	tLane.clear();
}
