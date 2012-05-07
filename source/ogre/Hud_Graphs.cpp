#include "pch.h"
#include "common/Defines.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "../vdrift/car.h"

#include "common/GraphView.h"
#include "SplitScreen.h"
#include <OgreSceneManager.h>
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
const int NG = 6;  // tire graphs count (for var load)

void App::CreateGraphs()
{
	if (!graphs.empty())  return;
	SceneManager* scm = mSplitMgr->mGuiSceneMgr;

	switch (pSet->graphs_type)
	{
	case 0:  /// bullet hit
		for (int i=0; i < 5; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGUI);
			int c = i%5;  /*clr*/
			gv->Create(512/*len*/, "graph"+toStr(c+1), i==0||i==2 ? 0.52f : 0.f/*alpha*/);
			switch (i)
			{
				case 0:  gv->CreateTitle("norm vel",	c, 0.0f, -2, 24);  break;
				case 1:  gv->CreateTitle("hit force",	c, 0.15f,-2, 24);  break;
				case 2:  gv->CreateTitle("N snd",		c, 0.0f, -2, 24);  break;
				case 3:  gv->CreateTitle("scrap",		c, 0.1f, -2, 24);  break;
				case 4:  gv->CreateTitle("screech",		c, 0.2f, -2, 24);  break;
			}
			if (i < 2)	gv->SetSize(0.f, 0.5f, 0.5f, 0.15f);
			else		gv->SetSize(0.f, 0.35f, 0.5f, 0.15f);

			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;

	case 1:  /// sound
		for (int i=0; i < 4; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGUI);
			int c = i%2*2;
			gv->Create(2*512, "graph"+toStr(c+1), c>0 ? 0.f : 0.4f);
			if (c == 0)
				gv->CreateGrid(2,1, 0.4f, 0.5f);  //64,256
			switch (i)
			{
				case 0:  gv->CreateTitle("vol ampl.",		c, 0.0f,-2, 24);  break;
				case 1:  gv->CreateTitle("pan: L up R dn",	c, 0.0f, 2, 24);  break;
				case 2:  gv->CreateTitle("wave L",			c, 0.0f,-2, 24);  break;
				case 3:  gv->CreateTitle("wave R",			c, 0.0f, 2, 24);  break;
			}
			if (i < 2)	gv->SetSize(0.00f, 0.24f, 0.40f, 0.25f);
			else		gv->SetSize(0.60f, 0.24f, 0.40f, 0.25f);

			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;

	case 2:  /// tire
	case 3:	 // susp
		for (int i=0; i < 8; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGUI);
			int c = i%4;
			gv->Create(512, "graph"+toStr(c+1), c>0 ? 0.f : (i < 14 ? 0.44f : 0.62f));
			if (c == 0)
				gv->CreateGrid(10,1, 0.2f, 0.4f);

			const static float x0 = 0.0f, x1 = 0.07f, x2 = 0.08f;
			const static char* cgt[8][2] = {
				// Front ^ Back/Rear v  Left L< Right R>
				 "FL [^"			,"FL <^"
				,"^] FR   long |"	,"^> FR susp pos"
				,"BL [_"			,"BL <v"
				,"_] BR   slip"		,"v> BR"
				,"FL [^"			,"FL <^"
				,"^] FR   lat --"	,"^> FR susp vel"
				,"BL [_"			,"BL <v"
				,"_] BR   slide"	,"v> BR"	};

			int t = pSet->graphs_type == 2 ? 0 : 1;
			float x = i%2==0 ? x0 : (t ? x2 : x1);  char y = i/2%2==0 ? -2 : -3;
			gv->CreateTitle(cgt[i][t], c, x, y, 24);

			if (i < 4)	gv->SetSize(0.00f, 0.24f, 0.40f, 0.25f);
			//else		gv->SetSize(0.60f, 0.24f, 0.40f, 0.25f);  // right
			else		gv->SetSize(0.00f, 0.50f, 0.40f, 0.25f);  // top
			
			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}	break;

	case 4:  /// tire pacejka
		for (int i=0; i < NG*2; ++i)
		{
			GraphView* gv = new GraphView(scm,mWindow,mGUI);
			int c = i%NG;  bool b = i >= NG;
			gv->Create(512, String("graph")+(b?"B":"A")+toStr(c), i>0 ? 0.f : 0.5f);
			if (c == 0)
			{	gv->CreateGrid(10,10, 0.2f, 0.4f);
				if (b)	gv->CreateTitle("", 5+8+c +2, 0.f, -2, 24);
				else	gv->CreateTitle("", 5+c   +2, 0.7f, 3, 24);
			}
			gv->SetSize(0.00f, 0.40f, 0.35f, 0.50f);

			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}
		{	//  edit vals area
			GraphView* gv = new GraphView(scm,mWindow,mGUI);
			gv->Create(1, "graphA6", 0.3f);  gv->CreateTitle("", 5+1, 0.f, -2, 24, 30);
			gv->SetSize(0.70f, 0.40f, 0.10f, 0.50f);
			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);

			//  vals descr
			gv = new GraphView(scm,mWindow,mGUI);
			gv->Create(1, "graphB6", 0.1f);  gv->CreateTitle("", 5+8, 0.f, -2, 24, 30);
			gv->SetSize(0.80f, 0.40f, 0.20f, 0.50f);
			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}
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
	case 0:  /// bullet hit  force,normvel, sndnum,scrap,screech
		if (gsi >= 5)
		if (carModels.size() > 0)
		{
			const CARDYNAMICS& cd = carModels[0]->pCar->dynamics;
			graphs[0]->AddVal(std::min(1.f, cd.fHitForce * 2.f));
			graphs[1]->AddVal(std::min(1.f, cd.fHitForce2));
			graphs[2]->AddVal(std::min(1.f, cd.fHitForce3));
			graphs[3]->AddVal(std::min(1.f, cd.fCarScrap));
			graphs[4]->AddVal(std::min(1.f, cd.fCarScreech));
		}
		break;

	case 1:  /// sound  vol,pan, wave L,R
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
	}
}

//-----------------------------------------------------------------------------------
void CAR::GraphsNewVals(double dt)		 // CAR
{	
	size_t gsi = pApp->graphs.size();
	switch (pApp->pSet->graphs_type)
	{
	case 2:  /// tire slide,slip
		if (gsi >= 8)
		for (int i=0; i < 4; ++i)
		{
			pApp->graphs[i]->AddVal(negPow(dynamics.tire[i].slideratio, 0.2) * 0.12f +0.5f);
			pApp->graphs[i+4]->AddVal(dynamics.tire[i].slipratio * 0.1f +0.5f);
		}	break;
		
	case 3:  /// suspension
		if (gsi >= 8)
		for (int i=0; i < 4; ++i)
		{
			const CARSUSPENSION <CARDYNAMICS::T> & susp = dynamics.GetSuspension((WHEEL_POSITION)i);
			pApp->graphs[i+4]->AddVal(negPow(susp.GetVelocity(), 0.5) * 0.2f +0.5f);
			pApp->graphs[i]->AddVal(susp.GetDisplacementPercent());
		}	break;
		
	case 4:  /// tire pacejka
		static int ii = 0;  ii++;  // skip upd cntr
		int im = pApp->iUpdTireGr > 0 ? 2 : 8;  // faster when editing val
		if (ii >= im && gsi >= NG*2)
		{	ii = 0;  pApp->iUpdTireGr = 0;

			typedef CARDYNAMICS::T T;
			CARTIRE <T> & tire = dynamics.tire[0];
			const int LEN = 512;
			T* ft = new T[LEN];

			T fmin, fmax, frng, maxF;
			const bool common = 1;  // common range for all
			const bool cust = 1;
			//const T fMAX = 6000.0, max_alpha = 360.0, max_sigma = 15.0;
			const T fMAX = 6000.0, max_y = 40.0, max_x = 1.0;

			///  Fy lateral --
			for (int i=0; i < NG; ++i)
			{
				bool comi = common || i == 0;
				if (comi)
				{	fmin = FLT_MAX;  fmax = FLT_MIN;  frng = 0.0;  }
				
				for (int x=0; x < LEN; ++x)
				{
					//T yy = max_y * 2.0 * (x-LEN*0.5) / LEN;
					T yy = max_y * x / LEN;
					T n = (NG-1-i) * 0.5 + 0.1;
					T fy = tire.Pacejka_Fy(yy, n, 0, 1.0, maxF); // normF
					//T fy = tire.Pacejka_Fy(alpha, 3, n-2, 1.0, maxF); // gamma
					ft[x] = fy;

					if (comi)  // get min, max
					{	if (fy < fmin)  fmin = fy;
						if (fy > fmax)  fmax = fy;  }
				}
				if (comi)  // get range
					frng = 1.0 / (fmax - fmin);
				if (cust)
				{	fmax = fMAX;  fmin = 0.0;  frng = 1.0 / (fmax - fmin);  }
				
				for (int x = 0; x < 512; ++x)
					pApp->graphs[i]->AddVal( (ft[x] - fmin) * frng );

				if (i==0)
					pApp->graphs[i]->UpdTitle("Fy Lateral--\n"  //#33FF77
						//"min: "+fToStr((float)fmin,2,4)+"\n"+
						"max y "+fToStr((float)fmax,0,1)+
						"  x "+fToStr(max_y,0,1)+"\n");
			}

			///  Fx long |
			for (int i=0; i < NG; ++i)
			{
				bool comi = common || i == 0;
				if (comi)
				{	fmin = FLT_MAX;  fmax = FLT_MIN;  frng = 0.0;  }
				
				for (int x=0; x < LEN; ++x)
				{
					//T xx = max_x * 2.0 * (x-LEN*0.5) / LEN;
					T xx = max_x * x / LEN;
					T n = (NG-1-i) * 0.5 + 0.1;
					T fx = tire.Pacejka_Fx(xx, n, 1.0, maxF); // normF
					ft[x] = fx;

					if (comi)  // get min, max
					{	if (fx < fmin)  fmin = fx;
						if (fx > fmax)  fmax = fx;  }
				}
				if (comi)  // get range
					frng = 1.0 / (fmax - fmin);
				if (cust)
				{	fmax = fMAX;  fmin = 0.0;  frng = 1.0 / (fmax - fmin);  }
				
				for (int x = 0; x < 512; ++x)
					pApp->graphs[i+NG]->AddVal( (ft[x] - fmin) * frng );

				if (i==0)
					pApp->graphs[i+NG]->UpdTitle("Fx Longit |\n"
						//"min: "+fToStr((float)fmin,2,4)+"\n"+
						"max y "+fToStr((float)fmax,0,1)+
						"  x "+fToStr(max_x,0,1)+"\n");
			}
			delete[]ft;
			
			//  update edit values text and descr
			///----------------------------------------------------
			String ss,sd;
			//const CARTIRE <CARDYNAMICS::T>& tire = pCar->dynamics.tire[0];
			const static String sLateral[15][2] = {
				"  a0","Shape factor",
				"  a1","Load infl. on friction coeff",
				"  a2","Lateral friction coeff at load = 0",
				"  a3","Maximum stiffness",
				"  a4","Load at maximum stiffness",
				"  a5","Camber infl. on stiffness",
				"  a6","Curvature change with load",
				"  a7","Curvature at load = 0",
				"  a8","Horiz. shift because of camber",
				"  a9","Load infl. on horizontal shift",
				" a10","Horizontal shift at load = 0",
				"a111","Camber infl. on vertical shift",
				"a112","Camber infl. on vertical shift",
				" a12","Load infl. on vertical shift",
				" a13","Vertical shift at load = 0" };
			const static String sLongit[13][2] = {
				" b0","Shape factor",
				" b1","Load infl. on long. friction coeff",
				" b2","Longitudinal friction coeff at load = 0",
				" b3","Curvature factor of stiffness",
				" b4","Change of stiffness with load at load = 0",
				" b5","Change of progressivity of stiffness/load",
				" b6","Curvature change with load^2",
				" b7","Curvature change with load",
				" b8","Curvature at load = 0",
				" b9","Load infl. on horizontal shift",
				"b10","Horizontal shift at load = 0",
				"b11","Load infl. on vertical shift",
				"b12","Vertical shift at load = 0" };
			const static String sAlign[18][2] = {
				" c0","Shape factor",
				" c1","Load infl. of peak value",
				" c2","Load infl. of peak value",
				" c3","Curvature factor of stiffness",
				" c4","Change of stiffness with load at load = 0",
				" c5","Change of progressivity of stiffness/load",
				" c6","Camber infl. on stiffness",
				" c7","Curvature change with load",
				" c8","Curvature change with load",
				" c9","Curvature at load = 0",
				"c10","Camber infl. of stiffness",
				"c11","Camber infl. on horizontal shift",
				"c12","Load infl. on horizontal shift",
				"c13","Horizontal shift at load = 0",
				"c14","Camber infl. on vertical shift",
				"c15","Camber infl. on vertical shift",
				"c16","Load infl. on vertical shift",
				"c17","Vertical shift at load = 0" };

			if (pApp->iEdTire == 0)
			{
				ss += "--Lateral--\n";  sd += "\n";
				for (int i=0; i < tire.transverse_parameters.size(); ++i)
				{
					//ss += (i == pApp->iCurLat) ? "." : "  ";
					float f = tire.transverse_parameters[i];
					unsigned short p = f > 100 ? 0 : (f > 10 ? 1 : (f > 1 ? 2 : 3));
					ss += sLateral[i][0] +" "+ fToStr(f, p,5);
					ss += (i == pApp->iCurLat) ? "  <\n" : "\n";
					sd += sLateral[i][1] +"\n";
				}

				ss += "\nalpha hat\n";
				//for (int a=0; a < tire.alpha_hat.size(); ++a)
				//	ss += "  "+fToStr( tire.alpha_hat[a], 3,5) + "\n";

				int z = (int)tire.alpha_hat.size()-1;
				ss += "  "+fToStr( tire.alpha_hat[0], 3,5) + "\n";
				ss += "  "+fToStr( tire.alpha_hat[z/2], 3,5) + "\n";
				ss += "  "+fToStr( tire.alpha_hat[z], 3,5) + "\n";
			}
			else if (pApp->iEdTire == 1)
			{
				ss += "| Longit |\n";  sd += "\n";
				for (int i=0; i < tire.longitudinal_parameters.size(); ++i)
				{
					//ss += (i == pApp->iCurLong) ? "." : "  ";
					float f = tire.longitudinal_parameters[i];
					unsigned short p = f > 100 ? 0 : (f > 10 ? 1 : (f > 1 ? 2 : 3));
					ss += sLongit[i][0] +" "+ fToStr(f, p,5);
					ss += (i == pApp->iCurLong) ? "  <\n" : "\n";
					sd += sLongit[i][1] +"\n";
				}

				ss += "\nsigma hat\n";
				//for (int a=0; a < tire.sigma_hat.size(); ++a)
				//	ss += "  "+fToStr( tire.sigma_hat[a], 3,5) + "\n";

				int z = (int)tire.sigma_hat.size()-1;
				ss += "  "+fToStr( tire.sigma_hat[0], 3,5) + "\n";
				ss += "  "+fToStr( tire.sigma_hat[z/2], 3,5) + "\n";
				ss += "  "+fToStr( tire.sigma_hat[z], 3,5) + "\n";
			}
			else //if (iEdLong == 2)
			{
				ss += "o Align o\n";  sd += "\n";
				for (int i=0; i < tire.aligning_parameters.size(); ++i)
				{
					float f = tire.aligning_parameters[i];
					unsigned short p = f > 100 ? 0 : (f > 10 ? 1 : (f > 1 ? 2 : 3));
					ss += sAlign[i][0] +" "+ fToStr(f, p,5);
					ss += (i == pApp->iCurAlign) ? "  <\n" : "\n";
					sd += sAlign[i][1] +"\n";
				}
			}
			
			pApp->graphs[gsi-2]->UpdTitle(ss);
			pApp->graphs[gsi-1]->UpdTitle(sd);
			
		}	break;
		
	}
}
