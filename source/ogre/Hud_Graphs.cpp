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
		}
		break;
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
		}
		break;
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
				,"^] FR   slide"	,"^> FR susp vel"
				,"BL [_"			,"BL <v"
				,"_] BR   lat --"	,"v> BR"	};

			int t = pSet->graphs_type == 2 ? 0 : 1;
			float x = i%2==0 ? x0 : (t ? x2 : x1);  char y = i/2%2==0 ? -2 : -3;
			gv->CreateTitle(cgt[i][t], c, x, y, 24);

			if (i < 4)	gv->SetSize(0.00f, 0.24f, 0.40f, 0.25f);
			//else		gv->SetSize(0.60f, 0.24f, 0.40f, 0.25f);  // right
			else		gv->SetSize(0.00f, 0.50f, 0.40f, 0.25f);  // top
			
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
	switch (pSet->graphs_type)
	{
	case 0:  /// bullet hit  force,normvel, sndnum,scrap,screech
		if (graphs.size() >= 5)
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
	if (graphs.size() >= 4)
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

void CAR::GraphsNewVals(double dt)		 // CAR
{	
	switch (pApp->pSet->graphs_type)
	{
	case 2:  /// tire slide,slip
		if (pApp->graphs.size() >= 8)
		for (int i=0; i < 4; ++i)
		{
			pApp->graphs[i]->AddVal(negPow(dynamics.tire[i].slideratio, 0.2) * 0.12f +0.5f);
			pApp->graphs[i+4]->AddVal(dynamics.tire[i].slipratio * 0.1f +0.5f);
		}	break;
		
	case 3:  /// suspension
		if (pApp->graphs.size() >= 8)
		for (int i=0; i < 4; ++i)
		{
			const CARSUSPENSION <CARDYNAMICS::T> & susp = dynamics.GetSuspension((WHEEL_POSITION)i);
			pApp->graphs[i+4]->AddVal(negPow(susp.GetVelocity(), 0.5) * 0.2f +0.5f);
			pApp->graphs[i]->AddVal(susp.GetDisplacementPercent());
		}	break;
	}
}
