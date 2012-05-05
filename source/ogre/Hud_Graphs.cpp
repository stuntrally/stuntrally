#include "pch.h"
#include "common/Defines.h"
#include "OgreGame.h"
#include "../vdrift/game.h"

#include "common/GraphView.h"
#include "SplitScreen.h"
#include <OgreSceneManager.h>
using namespace Ogre;



///  Create Graphs  .-_/\._-
//-----------------------------------------------------------------------------------
void App::CreateGraphs()
{
	if (!graphs.empty())  return;
	SceneManager* scm = mSplitMgr->mGuiSceneMgr;

	switch (pSet->graphs_type)
	{
	case 0:  // bullet hit
		for (int i=0; i < 5; ++i)
		{
			GraphView* gv = new GraphView(scm);
			gv->Create(512, "graph"+toStr(i%5+1), 0.13f);
			if (i < 2)	gv->SetSize(0.f, 0.5f, 0.5f, 0.15f);
			else		gv->SetSize(0.f, 0.35f, 0.5f, 0.15f);
			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}
		break;
	case 1:  // sound
		for (int i=0; i < 4; ++i)
		{
			GraphView* gv = new GraphView(scm);
			gv->Create(2*512, "graph"+toStr(i%2*2+1), 0.13f);
			if (i < 2)	gv->SetSize(0.00f, 0.24f, 0.40f, 0.25f);
			else		gv->SetSize(0.60f, 0.24f, 0.40f, 0.25f);
			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}
		break;
	case 2:  // car tires
		for (int i=0; i < 8; ++i)
		{
			GraphView* gv = new GraphView(scm);
			gv->Create(512, "graph"+toStr(i%4+1), 0.13f);
			if (i < 4)	gv->SetSize(0.00f, 0.24f, 0.40f, 0.25f);
			else		gv->SetSize(0.60f, 0.24f, 0.40f, 0.25f);
			gv->SetVisible(pSet->show_graphs);
			graphs.push_back(gv);
		}
		break;
	}
}

///  add new Values to graphs (each frame)
//-----------------------------------------------------------------------------------
void App::GraphsNewVals()
{
	if (graphs.size() < 4)  return;
	
	switch (pSet->graphs_type)
	{
	case 0:  // bullet hit  force,normvel, sndnum,scrap,screech
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

	case 1:  // sound  vol,pan, wave L,R
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

	//case 2:  // car wheels slip,slide  in CAR::Update
	//	break;
	}
}

//  Update
//-----------------------------------------------------------------------------------
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
