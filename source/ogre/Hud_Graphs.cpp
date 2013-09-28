#include "pch.h"
#include "common/Def_Str.h"
#include "CGame.h"
#include "../vdrift/game.h"
#include "../vdrift/car.h"
//#include "../vdrift/quickprof.h"
#include "common/data/SceneXml.h"
#include "common/GraphView.h"
#include "SplitScreen.h"
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
using namespace Ogre;
using namespace MyGUI;



//  Update
//-----------------------------------------
void App::UpdateGraphs()
{
	for (int i=0; i < graphs.size(); ++i)
		graphs[i]->Update();
}

void App::DestroyGraphs()
{
	for (int i=0; i < graphs.size(); ++i)
	{
		graphs[i]->Destroy();
		delete graphs[i];
	}
	graphs.clear();
}

//  util
inline double negPow(double x, double y)
{
	return (x >= 0.0) ? pow(x, y) : -pow(-x, y);
}


///  Create Graphs  .-_/\._-
//-----------------------------------------------------------------------------------
const int TireNG = 4,    // tire graphs count (for variable load)
        TireLenG = 256;  // tire graphs data length
const static String TireVar[2] = {"variable Load", "variable camber"};

void App::CreateGraphs()
{
	if (!graphs.empty())  return;
	SceneManager* scm = mSplitMgr->mGuiSceneMgr;

	switch (pSet->graphs_type)
	{
	case Gh_Fps:  /// fps
		for (int i=0; i < 2; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGui);
			int c = i;
			gv->Create(400, "graph"+toStr(c+1), i==0 ? 0.4f : 0.f);
			if (i == 0)
			{	gv->CreateGrid(4,0, 0.7f, 1.0f);
				gv->CreateTitle("120  Fps\n\n\n\n\n\n\n60",	c, 0.0f, -2, 24,10);
			}
			gv->SetSize(0.f, 0.24f, 0.4f, 0.30f);

			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;

	case Gh_CarAccelG:  /// car accel
		for (int i=0; i < 3; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGui);
			const int t[3] = {0,1,2};
			int c = t[i];
			gv->Create(256, "graph"+toStr(c+1), i==0 ? 0.45f : 0.f);
			if (i == 0)  gv->CreateGrid(6,0, 0.7f, 1.0f);
			switch(i)
			{
				case 0:  gv->CreateTitle("\n\n\n\n\n\n\n0 x",					c, 0.0f, -2, 24, 12);  break;
				case 1:  gv->CreateTitle("Car accel G's\n\n\n\n\n\n\n       y",	c, 0.f,-2, 24, 12);  break;
				case 2:  gv->CreateTitle("\n\n\n\n\n\n\n\n\n\n\n0 z",			c, 0.f,-2, 24, 12);  break;
			}
			gv->SetSize(0.f, 0.24f, 0.4f, 0.30f);

			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;

	case Gh_BulletHit:  /// bullet hit
		for (int i=0; i < 6; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGui);
			int c = i%6;  /*clr*/
			gv->Create(256/*len*/, "graph"+toStr(c+1), i==0||i==2 ? 0.52f : 0.f/*alpha*/);
			switch(i)
			{
				case 0:  gv->CreateTitle("norm vel",	c, 0.0f, -2, 24);  break;
				case 1:  gv->CreateTitle("Car Hit force",c, 0.15f,-2, 24);  break;
				case 2:  gv->CreateTitle("N snd",		c, 0.0f, -2, 24);  break;
				case 3:  gv->CreateTitle("scrap",		c, 0.1f, -2, 24);  break;
				case 4:  gv->CreateTitle("screech",		c, 0.2f, -2, 24);  break;
				case 5:  gv->CreateTitle("damage factor",c, 0.35f, -2, 24);  break;
			}
			if (i < 2 || i==5)
						gv->SetSize(0.f, 0.5f,  0.4f, 0.15f);
			else		gv->SetSize(0.f, 0.35f, 0.4f, 0.15f);

			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;

	case Gh_Sound:  /// sound
		for (int i=0; i < 4; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGui);
			int c = i%2*2;
			gv->Create(i < 2 ? 512 : 2*512, "graph"+toStr(c+1), c>0 ? 0.f : 0.4f);
			if (c == 0)
				gv->CreateGrid(2,1, 0.4f, 0.5f);  //64,256
			switch(i)
			{
				case 0:  gv->CreateTitle("Sound vol ampl.",	c, 0.0f,-2, 24);  break;
				case 1:  gv->CreateTitle("pan: L up R dn",	c, 0.0f, 2, 24);  break;
				case 2:  gv->CreateTitle("wave L",			c, 0.0f,-2, 24);  break;
				case 3:  gv->CreateTitle("wave R",			c, 0.0f, 2, 24);  break;
			}
			if (i < 2)	gv->SetSize(0.00f, 0.24f, 0.4f, 0.25f);
			else		gv->SetSize(0.60f, 0.24f, 0.4f, 0.25f);

			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;

	case Gh_TireSlips:  /// tire
	case Gh_Suspension:	 /// susp
		for (int i=0; i < 8; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGui);
			int c = i%4;
			gv->Create(256/*512*/, "graph"+toStr(c+1), c>0 ? 0.f : (i < 14 ? 0.44f : 0.62f));
			if (c == 0)
				gv->CreateGrid(10,1, 0.2f, 0.4f);

			const static float x0 = 0.0f, x1 = 0.07f, x2 = 0.08f;
			const static char* cgt[8][2] = {
				// Front ^ Back/Rear v  Left L< Right R>
				 "FL [^"				,"FL <^"
				,"^] FR   longit |"		,"^> FR  suspension pos"
				,"BL [_"				,"BL <v"
				,"_] BR   Tire slip"	,"v> BR"
				,"FL [^"				,"FL <^"
				,"^] FR   lateral --"	,"^> FR  susp vel"
				,"BL [_"				,"BL <v"
				,"_] BR   Tire slide"	,"v> BR"	};

			int t = pSet->graphs_type == Gh_TireSlips ? 0 : 1;
			float x = i%2==0 ? x0 : (t ? x2 : x1);  char y = i/2%2==0 ? -2 : -3;
			gv->CreateTitle(cgt[i][t], c, x, y, 24);

			if (i < 4)	gv->SetSize(0.00f, 0.24f, 0.4f, 0.25f);
			//else		gv->SetSize(0.60f, 0.24f, 0.4f, 0.25f);  // right
			else		gv->SetSize(0.00f, 0.50f, 0.4f, 0.25f);  // top
			
			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;


	case Gh_TireEdit:  /// tires edit pacejka
		for (int i=0; i < TireNG*2; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGui);
			int c = i%TireNG;  bool b = i >= TireNG;
			gv->Create(TireLenG, String("graph")+(b?"B":"A")+toStr(c), i>0 ? 0.f : 0.4f, true);
			if (c == 0)
			{	gv->CreateGrid(10,10, 0.2f, 0.4f);
				if (b)	gv->CreateTitle("", 5+8+c +2, 0.f, -2, 24);
				else	gv->CreateTitle("", 5+c   +2, 0.7f, 3, 24);
			}else if (i == 1)
				gv->CreateTitle("Tire forces", 5, 0.3f, -2, 24);
			else if (i == 2)
				gv->CreateTitle("", 5, 0.5f, -2, 24);
			
			gv->SetSize(0.f, 0.41f, 0.35f, 0.50f);

			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}
		{	//  edit vals area
			GraphView* gv = new GraphView(scm,mWindow,mGui);
			gv->Create(1, "graphA6", 0.4f);  gv->CreateTitle("", 5+1, 0.f, -2, 24, 30/*, true*/);
			gv->SetSize(0.73f, 0.48f, 0.07f, 0.44f);
			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);

			//  vals descr
			gv = new GraphView(scm,mWindow,mGui);
			gv->Create(1, "graphB6", 0.3f);  gv->CreateTitle("", 5+8, 0.f, -2, 24, 30/*, true*/);
			gv->SetSize(0.80f, 0.48f, 0.20f, 0.44f);
			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}
		break;
	

	case Gh_TorqueCurve:  /// torque curves, gears
		for (int w=0; w < 6*2; ++w)
		{
			int i = w % 6, n = w / 6;
			GraphView* gv = new GraphView(scm,mWindow,mGui);
			gv->Create(512, String("graph")+(n>0 ? "B":"A")+toStr(i), w>0 ? 0.f : 0.5f, true);
			if (n == 0)
			if (i == 0)	{   gv->CreateGrid(4,5, 0.f, 0.5f);
						    gv->CreateTitle("", 5, 0.7f, -2, 24, 10);  }
			else if (i==1)  gv->CreateTitle("", 7, 0.82, -2, 24, 10);
			else if (i==2)  gv->CreateTitle("", 6, 0.92, -2, 24, 10);
			else if (i==3)  gv->CreateTitle("Wheel  Torque curves\n2400 Nm", 5, 0.0f, -2, 24, 2);
			else if (i==4)  gv->CreateTitle("Car Vel.\n250 kmh", 6, 0.86f, 3, 24, 2);
			
			gv->SetSize(0.f, 0.40f, 0.45f, 0.50f);

			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;
		
	case Gh_Engine:  /// engine torque, power
		for (int i=0; i < 2; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGui);
			gv->Create(512, String("graph")+toStr(i*3+1), i>0 ? 0.f : 0.4f, true);
			if (i == 0)
			{	gv->CreateGrid(6,6, 0.f, 0.6f);
				gv->CreateTitle("Engine Torque  600 Nm", 0, 0.0f, -2, 24, 30);
			}else if (i==1)
				gv->CreateTitle("\nPower 600 bhp", 3, 0.0f, -2, 24, 2);
			//rpm min,max..
			
			gv->SetSize(0.f, 0.41f, 0.35f, 0.50f);

			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;
		
	case Gh_Clutch:
		for (int i=0; i < 4; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGui);
			gv->Create(160, String("graph")+toStr(3-i), i==0 ? 0.43f : i==3 ? 0.3f : 0.f);
			if (i == 3)
			{	gv->CreateGrid(6,1, 0.f, 0.3f);
				gv->CreateTitle("Gear", 5, 0.0f, -2, 24, 1);
				gv->SetSize(0.f, 0.27f, 0.36f, 0.07f);
			}else
			{	if (i == 0)
				{	gv->CreateGrid(4,1, 0.f, 0.2f);
					gv->CreateTitle("Rpm 7500", 2, 0.0f, -2, 24, 1);
				}else if (i==1)
					gv->CreateTitle("\nClutch", 1, 0.0f, -2, 24, 2);
				else if (i==2)
					gv->CreateTitle("\n\nLocked", 0, 0.0f, -2, 24, 3);
				gv->SetSize(0.f, 0.34f, 0.36f, 0.27f);
			}
			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;

	case Gh_Diffs:
		for (int i=0; i < 3*2; ++i)
		{	int n = i/3, c = i%3;
			GraphView* gv = new GraphView(scm,mWindow,mGui);
			gv->Create(256, String("graph")+(n?"B":"A")+toStr(c+2), n > 0 ? 0.f :  c<2 ? 0.4f : 0.3f);
			if (i == 2)
			{	gv->CreateGrid(4,1, 0.f, 0.3f);
				gv->CreateTitle("Center, torque", 2, 0.0f, -2, 24, 1);
			}else
			{	if (i == 0)
					gv->CreateTitle("Front, speed", 0, 0.0f, -2, 24, 1);
				else if (i==1)
					gv->CreateTitle("Rear",  1, 0.0f, -2, 24, 2);
				gv->CreateGrid(4,1, 0.f, 0.3f);
			}
			if (c==2)	gv->SetSize(0.f,		0.27f,			0.32f, 0.2f);
			else		gv->SetSize(0.f, c==0 ? 0.47f : 0.12f,	0.32f, 0.15f);
			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;
		

	default:
		break;
	}
}

///  add new Values to graphs (each frame)
//-----------------------------------------------------------------------------------
void App::GraphsNewVals()				// Game
{
	size_t gsi = graphs.size();
	switch (pSet->graphs_type)
	{
	case Gh_Sound:  /// sound  vol,pan, wave L,R
	if (gsi >= 4)
	{	float minL=1.f,maxL=-1.f,minR=1.f,maxR=-1.f;
		//for (int i=0; i < 4*512; i+=4)  if (sound.Init(/*2048*/512
		for (int i=0; i < 2*512; ++i)
		{
			//  wave osc
			float l = pGame->sound.waveL[i] / 32767.f * 0.5f + 0.5f;
			float r = pGame->sound.waveR[i] / 32767.f * 0.5f + 0.5f;
			//if (i%4==0)
			{
				graphs[2]->AddVal(l);  // L cyan  R yellow
				graphs[3]->AddVal(r);
			}
			//  amplutude
			if (l > maxL)  maxL = l;  if (l < minL)  minL = l;
			if (r > maxR)  maxR = r;  if (r < minR)  minR = r;
		}
		float al = (maxL-minL), ar = (maxR-minR);
		graphs[0]->AddVal((al+ar)*0.5f);       // vol ampl  cyan
		graphs[1]->AddVal((al-ar)*0.5f+0.5f);  // pan  yellow  ^L 1  _R 0
	}	break;

	case Gh_Fps:  /// fps
	if (gsi >= 1)
	{
		const RenderTarget::FrameStats& stats = mWindow->getStatistics();
		graphs[0]->AddVal(stats.lastFPS /60.f*0.5f);  // 60 fps in middle
		//graphs[1]->AddVal(1000.f/PROFILER.getAvgDuration(" frameSt",quickprof::MILLISECONDS) /60.f*0.2f);
		graphs[1]->AddVal(fLastFrameDT==0.f ? 1.f : (1.f/fLastFrameDT/60.f*0.5f));
	}	break;
	
	default:
		break;
	}
}

//-----------------------------------------------------------------------------------
void CAR::GraphsNewVals(double dt)		 // CAR
{	
	size_t gsi = pApp->graphs.size();
	if (pApp->pSet->graphs_type != Gh_TireEdit)
	switch (pApp->pSet->graphs_type)
	{
	case Gh_BulletHit:  /// bullet hit  force,normvel, sndnum,scrap,screech
		if (gsi >= 6)
		{
			const CARDYNAMICS& cd = dynamics;
			pApp->graphs[0]->AddVal(std::min(1.f, cd.fHitForce * 2.f));
			pApp->graphs[1]->AddVal(std::min(1.f, cd.fHitForce2));
			pApp->graphs[2]->AddVal(std::min(1.f, cd.fHitForce3));
			pApp->graphs[3]->AddVal(std::min(1.f, cd.fCarScrap));
			pApp->graphs[4]->AddVal(std::min(1.f, cd.fCarScreech));
			pApp->graphs[5]->AddVal(cd.fHitDmgA);
		}
		break;

	case Gh_CarAccelG:  /// car accel x,y,z
		if (gsi >= 3)
		{
			MATHVECTOR<Dbl,3> v = dynamics.body.GetForce();
			(-dynamics.Orientation()).RotateVector(v);
			float m = dynamics.body.GetMass();
			//LogO("mass: "+fToStr(m,1,5)+"  x: "+fToStr(v[0]/m,2,4)+"  y: "+fToStr(v[1]/m,2,4)+"  z: "+fToStr(v[2]/m,2,4));

			for (int i=0; i < 3; ++i)
				pApp->graphs[i]->AddVal( std::max(0.f, std::min(1.f, float(
					v[i]/m *0.63f /9.81f/3.f + (i==2 ? 0.f : 0.5f) ) )));
		}	break;
		
	case Gh_TireSlips:  /// tire slide,slip
		if (gsi >= 8)
		for (int i=0; i < 4; ++i)
		{
			pApp->graphs[i]->AddVal(negPow(dynamics.wheel[i].slips.slideratio, 0.2) * 0.12f +0.5f);
			//pApp->graphs[i]->AddVal(dynamics.wheel[i].slips.slide * 0.1f +0.5f);
			pApp->graphs[i+4]->AddVal(dynamics.wheel[i].slips.slipratio * 0.1f +0.5f);
		}	break;
		
	case Gh_Suspension:  /// suspension
		if (gsi >= 8)
		for (int i=0; i < 4; ++i)
		{
			const CARSUSPENSION& susp = dynamics.GetSuspension((WHEEL_POSITION)i);
			pApp->graphs[i+4]->AddVal(negPow(susp.GetVelocity(), 0.5) * 0.2f +0.5f);
			pApp->graphs[i]->AddVal(susp.GetDisplacementPercent());
		}	break;

		
	case Gh_TorqueCurve:  /// torque curves, gears
	//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
	{	static int ii = 0;  ++ii;  // skip upd cntr
		if (ii >= 1/*! 10*/ && gsi >= 6*2)
		{	ii = 0;
			const CARDYNAMICS& d = dynamics;
			const Dbl fin = d.diff_center.GetFinalDrive();
			const Dbl r = 1.0 / (2 * PI_d * d.wheel[0].GetRadius());

			String s0="  ratio\n", s1="rpmLow\n", s2="velMax\n";  // text legend
			for (int i=0; i < 6; ++i)
			{
				int g = i+1;
				bool bValid = i < d.transmission.GetForwardGears();
				bool bCur = (g == GetGear()) && d.clutch.IsLocked();

				const Dbl gr = bValid ? d.transmission.GetGearRatio(g) : 0.01, grfin = gr * fin;

				Dbl engRpm = d.engine.GetRPM();
				Dbl downRpm = i>0 ? d.DownshiftRPM(g) : d.engine.GetStartRPM();

				Dbl rmax = d.engine.GetRpmMax(),
				    rmin = d.engine.GetStartRPM(), rng = rmax-rmin;
				Dbl rpmOld = 0;

				//  graph  ------
				for (int x = 0; x < 512; ++x)
				{
					// 100 kmh = 28 car.m/s = 15 wh.rps = 102 eng.rps = 6100 eng.rpm
					//       / 3.6     /1.95 (2*PI*r)   *6.87 ratio = (3.9 * 1.76) 3rd gear
					Dbl xx = x/511.0;  // 0..1
					Dbl vel = xx * 250.0 / 3.6;  // 250 kmh max, in m/s
					Dbl whrps = vel * r;  // wheel revs per sec
					Dbl rpm = whrps * grfin * 60.0;  // engine rpm

					Dbl tq = gr * d.engine.GetTorqueCurve(1.0, rpm);
					if (rpm > rmax)  tq = 0.0;  if (rpm < rmin)  tq = 0.0;  // lines down ||

					Dbl v = tq / 2400.0;  // 2400 max
					if (bCur && engRpm > rpmOld && engRpm <= rpm)  // cur rpm mark ^
						v = 1.0;

					pApp->graphs[i]->AddVal(v);  // torque
					if (i>0)
					{	v = rpm < downRpm  //downRpm > rpmOld && downRpm <= rpm
							? 0.0 : std::max(0.0, std::min(1.0, (rpm-rmin)/rng ));  // rpm range
						pApp->graphs[i+6]->AddVal(v);  }
					rpmOld = rpm;
				}
				pApp->graphs[i]->SetUpdate();
				if (i>0)
				pApp->graphs[i+6]->SetUpdate();

				//  text  ------
				if (bValid)
				{
					Dbl velMax = 3.6 * d.engine.GetRpmMax() / (grfin * 60.0) / r;
					if (!bValid)  velMax = 0;
					
					s0 += toStr(g)+": "+fToStr(gr,3,5)+"\n";
					s1 += fToStr(downRpm,0,4)+"\n";
					s2 += fToStr(velMax,0,3)+"\n";
				}
			}
			s0 += "  final\n    "+fToStr(fin,3,5);
			pApp->graphs[0]->UpdTitle(s0);  pApp->graphs[1]->UpdTitle(s1);  pApp->graphs[2]->UpdTitle(s2);
	}	}	break;

		
	case Gh_Engine:  /// engine torque, power
	{	static int ii = 0;  ++ii;  // skip upd cntr
		if (ii >= 10 && gsi >= 2)
		{	ii = 0;
			const CARENGINE& eng = dynamics.engine;
			float maxTrq = 0.f, maxPwr = 0.f;
			int rpmMaxTq = 0, rpmMaxPwr = 0;
			float rmin = eng.GetStartRPM(), rmax = eng.GetRpmMax(), rng = rmax - rmin;

			for (int x = 0; x < 512; ++x)
			{
				float r = x/512.f * rng + rmin;
				float tq = eng.GetTorqueCurve(1.0, r);
				float pwr = tq * 2.0 * PI_d * r / 60.0 * 0.001 * 1.341;  //kW  // 1kW = 1.341 bhp
				if (tq > maxTrq)  {  maxTrq = tq;  rpmMaxTq = r;  }
				if (pwr > maxPwr)  {  maxPwr = pwr;  rpmMaxPwr = r;  }

				pApp->graphs[0]->AddVal( tq / 600.0 );
				pApp->graphs[1]->AddVal( pwr / 600.0 );
			}
			pApp->graphs[0]->SetUpdate();
			pApp->graphs[1]->SetUpdate();
			//pApp->graphs[0]->UpdTitle(ss);
	}	}	break;

	
	case Gh_Clutch:  /// clutch,rpm,gears
		if (gsi >= 4)
		{
			const CARENGINE& eng = dynamics.engine;
			const CARCLUTCH& clu = dynamics.clutch;
			pApp->graphs[0]->AddVal(eng.GetRPM() / 7500.0);
			#if 0
				MATHVECTOR<Dbl,3> vel = dynamics.GetVelocity(), vx(1,0,0);
				dynamics.Orientation().RotateVector(vx);
				Dbl d = vel.dot(vx),
					velCar = vel.Magnitude() * (d >= 0.0 ? 1 : -1), velWh = dynamics.GetSpeedMPS();
				//pApp->graphs[1]->AddVal(velCar * 0.02f));
				//pApp->graphs[2]->AddVal(velWh * 0.02f);
				if (velWh < 1.1f && velWh > -1.1f)
					d = 1.f;
				else
					d = fabs(velCar >= velWh ? velCar/velWh : velWh/velCar);
				pApp->graphs[2]->AddVal(/*std::min(1.f, std::max(1.f,*/ d * 0.5f);
			#else
			pApp->graphs[1]->AddVal(clu.GetClutch() * 0.3f + 0.15f);
			pApp->graphs[2]->AddVal(clu.IsLocked() ? 0.15f : 0.f);
			#endif
			pApp->graphs[3]->AddVal(GetGear() / 6.f);
		}	break;


	case Gh_Diffs:  /// differentials
		if (gsi >= 6)
		for (int i=0; i < 3*2; ++i)
		{
			CARDIFFERENTIAL* diff;
			switch (i%3)
			{
			case 0: diff = &dynamics.diff_front;  break;
			case 1: diff = &dynamics.diff_rear;  break;
			case 2: diff = &dynamics.diff_center;  break;
			}
			Dbl d;
			if (i/3==0) {
				d = diff->GetSide1Speed() - diff->GetSide2Speed();
				d = negPow(d, 0.4)*0.07 + 0.5;  }  // blue
			else {
				d = diff->GetSide1Torque() - diff->GetSide2Torque();
				d = d*0.0004 + 0.5;  }  // orange
			pApp->graphs[i]->AddVal(d);
		}	break;
		
	}
	else  ///Gh_TireEdit:  /// tire pacejka
	//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
	{	static int ii = 0;  ++ii;  // skip upd cntr
		const int im = pApp->iUpdTireGr > 0 ? 2 : 8;  // faster when editing val
		if (ii >= im && gsi >= TireNG*2)
		{	ii = 0;  pApp->iUpdTireGr = 0;

			const CARTIRE* tire = dynamics.GetTire(FRONT_LEFT);
			Dbl* ft = new Dbl[TireLenG];

			Dbl fmin, fmax, frng, maxF;
			const bool common = 1;  // common range for all
			const bool cust = 1;
			//  RANGE  gui sld ..
			const Dbl fMAX = 9000.0, max_y = pApp->sc->asphalt ? 40.0 : 80.0, max_x = 1.0;

			///  Fy lateral --
			for (int i=0; i < TireNG; ++i)
			{
				bool comi = common || i == 0;
				if (comi)
				{	fmin = FLT_MAX;  fmax = FLT_MIN;  frng = 0.0;  }
				
				for (int x=0; x < TireLenG; ++x)
				{
					//Dbl yy = max_y * 2.0 * (x-LEN*0.5) / LEN;
					Dbl yy = max_y * x / TireLenG;
					Dbl n = (TireNG-1-i+1) * 0.65;
					Dbl fy = !pApp->iTireLoad ? tire->Pacejka_Fy(yy, n, 0, 1.0, maxF)  // normF
											: tire->Pacejka_Fy(yy, 3, n-2, 1.0, maxF); // camber
					ft[x] = fy;

					if (comi)  // get min, max
					{	if (fy < fmin)  fmin = fy;
						if (fy > fmax)  fmax = fy;  }
				}
				if (comi)  // get range
					frng = 1.0 / (fmax - fmin);
				if (cust)
				{	fmax = fMAX;  fmin = 0.0;  frng = 1.0 / (fmax - fmin);  }
				
				for (int x = 0; x < TireLenG; ++x)
					pApp->graphs[i]->AddVal( (ft[x] - fmin) * frng );
				pApp->graphs[i]->SetUpdate();

				if (i==0)
					pApp->graphs[i]->UpdTitle("Fy Lateral--\n"  //#33FF77
						//"min: "+fToStr((float)fmin,2,4)+"\n"+
						"max y "+fToStr((float)fmax,0,1)+
						"  x "+fToStr(max_y,0,1)+"\n");
			}

			///  Fx long |
			for (int i=0; i < TireNG; ++i)
			{
				bool comi = common || i == 0;
				if (comi)
				{	fmin = FLT_MAX;  fmax = FLT_MIN;  frng = 0.0;  }
				
				for (int x=0; x < TireLenG; ++x)
				{
					//Dbl xx = max_x * 2.0 * (x-LEN*0.5) / LEN;
					Dbl xx = max_x * x / TireLenG;
					Dbl n = (TireNG-1-i+1) * 0.65;
					Dbl fx = pApp->iEdTire != 2 ? tire->Pacejka_Fx(xx, n, 1.0, maxF)  // normF
							 : (!pApp->iTireLoad ? tire->Pacejka_Mz(xx, 0, n, 0.0, 1.0, maxF)    // align- norm
												 : tire->Pacejka_Mz(0, xx, n, 0.0, 1.0, maxF));  // align- camber
					ft[x] = fx;

					if (comi)  // get min, max
					{	if (fx < fmin)  fmin = fx;
						if (fx > fmax)  fmax = fx;  }
				}
				if (comi)  // get range
					frng = 1.0 / (fmax - fmin);
				if (cust)
				{	fmax = fMAX;  fmin = 0.0;  frng = 1.0 / (fmax - fmin);  }
				
				for (int x = 0; x < TireLenG; ++x)
					pApp->graphs[i+TireNG]->AddVal( (ft[x] - fmin) * frng );
				pApp->graphs[i+TireNG]->SetUpdate();

				if (i==0)
					pApp->graphs[i+TireNG]->UpdTitle("Fx Longit |\n"
						//"min: "+fToStr((float)fmin,2,4)+"\n"+
						"max y "+fToStr((float)fmax,0,1)+
						"  x "+fToStr(max_x,0,1)+"\n");
			}
			delete[]ft;
			
			//  update edit values text and descr
			///----------------------------------------------------
			const static String sLateral[15][2] = {
				"  a0","#F0FFFFShape factor",
				"  a1","#C0E0FFLoad infl. on friction coeff",
				"  a2","#F0FFFFLateral friction coeff at load = 0",
				"  a3","#F0FFFFMaximum stiffness",
				"  a4","#F0FFFFLoad at maximum stiffness",
				"  a5","#C0E0FF-Camber infl. on stiffness",
				"  a6","Curvature change with load",
				"  a7","Curvature at load = 0",
				"  a8","#A0C0D0  -Horiz. shift because of camber",
				"  a9","  Load infl. on horizontal shift",
				" a10","  Horizontal shift at load = 0",
				"a111","  -Camber infl. on vertical shift",
				"a112","  -Camber infl. on vertical shift",
				" a12","  Load infl. on vertical shift",
				" a13","  Vertical shift at load = 0" };
			const static String sLongit[13][2] = {
				"  b0","#FFFFF0Shape factor",
				"  b1","#F0F0A0Load infl. on long. friction coeff",
				"  b2","#FFFFF0Longit. friction coeff at load = 0",
				"  b3","#F0F0A0Curvature factor of stiffness",
				"  b4","#F0F0A0Change of stiffness with load at load = 0",
				"  b5","#E0C080Change of progressivity/load",  //of stiffness
				"  b6","Curvature change with load^2",
				"  b7","Curvature change with load",
				"  b8","Curvature at load = 0",
				"  b9","#D0D0A0  Load infl. on horizontal shift",
				" b10","  Horizontal shift at load = 0",
				" b11","  Load infl. on vertical shift",
				" b12","  Vertical shift at load = 0" };
			const static String sAlign[18][2] = {
				" c0","#E0FFE0Shape factor",
				" c1","Load infl. of peak value",
				" c2","Load infl. of peak value",
				" c3","Curvature factor of stiffness",
				" c4","Change of stiffness with load at load = 0",
				" c5","Change of progressivity/load",
				" c6","-Camber infl. on stiffness",
				" c7","Curvature change with load",
				" c8","Curvature change with load",
				" c9","Curvature at load = 0",
				"c10","-Camber infl. of stiffness",
				"c11","  -Camber infl. on horizontal shift",
				"c12","  Load infl. on horizontal shift",
				"c13","  Horizontal shift at load = 0",
				"c14","  -Camber infl. on vertical shift",
				"c15","  -Camber infl. on vertical shift",
				"c16","  Load infl. on vertical shift",
				"c17","  Vertical shift at load = 0" };
			const static String sCommon = "#C8C8F0Pacejka's Magic Formula coeffs\n";

			String ss,sd;
			if (pApp->iEdTire == 0)
			{
				ss += "#A0F0FF--Lateral--\n";  sd += sCommon;
				for (int i=0; i < tire->lateral.size(); ++i)
				{
					//ss += (i == pApp->iCurLat) ? "." : "  ";
					float f = tire->lateral[i];
					char p = f > 100 ? 0 : (f > 10 ? 1 : (f > 1 ? 2 : 3));
					bool cur = (i == pApp->iCurLat);

					ss += cur ? "#A0EEFF" : "#E0FFFF";
					ss += sLateral[i][0] +" "+ fToStr(f, p,5);
					ss += cur ? "  <\n" : "\n";
					sd += sLateral[i][1] +"\n";
				}

				ss += "\n#C0F0F0alpha hat\n";
				//for (int a=0; a < tire->alpha_hat.size(); ++a)
				//	ss += "  "+fToStr( tire->alpha_hat[a], 3,5) + "\n";

				int z = (int)tire->alpha_hat.size()-1;
				ss += "  "+fToStr( tire->alpha_hat[0], 3,5) + "\n";
				ss += "  "+fToStr( tire->alpha_hat[z/2], 3,5) + "\n";
				ss += "  "+fToStr( tire->alpha_hat[z], 3,5) + "\n";
			}
			else if (pApp->iEdTire == 1)
			{
				ss += "#FFFF70| Longit |\n";  sd += sCommon;
				for (int i=0; i < tire->longitudinal.size(); ++i)
				{
					//ss += (i == pApp->iCurLong) ? "." : "  ";
					float f = tire->longitudinal[i];
					char p = f > 100 ? 0 : (f > 10 ? 1 : (f > 1 ? 2 : 3));
					bool cur = (i == pApp->iCurLong);

					ss += cur ? "#FFE090" : "#FFFFD0";
					ss += sLongit[i][0] +" "+ fToStr(f, p,5);
					ss += cur ? "  <\n" : "\n";
					sd += sLongit[i][1] +"\n";
				}

				ss += "\n#F0F0C0sigma hat\n";
				//for (int a=0; a < tire->sigma_hat.size(); ++a)
				//	ss += "  "+fToStr( tire->sigma_hat[a], 3,5) + "\n";

				int z = (int)tire->sigma_hat.size()-1;
				ss += "  "+fToStr( tire->sigma_hat[0], 3,5) + "\n";
				ss += "  "+fToStr( tire->sigma_hat[z/2], 3,5) + "\n";
				ss += "  "+fToStr( tire->sigma_hat[z], 3,5) + "\n";
			}
			else //if (pApp->iEdTire == 2)
			{
				ss += "#D0FFD0o Align o\n";  sd += sCommon;
				for (int i=0; i < tire->aligning.size(); ++i)
				{
					float f = tire->aligning[i];
					char p = f > 100 ? 0 : (f > 10 ? 1 : (f > 1 ? 2 : 3));
					bool cur = (i == pApp->iCurAlign);

					ss += cur ? "#80FF80" : "#E0FFE0";
					ss += sAlign[i][0] +" "+ fToStr(f, p,5);
					ss += cur ? "  <\n" : "\n";
					sd += sAlign[i][1] +"\n";
				}
			}
			
			pApp->graphs[gsi-2]->UpdTitle(ss);
			pApp->graphs[gsi-1]->UpdTitle(sd);
			pApp->graphs[2]->UpdTitle(TireVar[pApp->iTireLoad]); //-
		}
	}
}
