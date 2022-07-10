#include "BaseApp.h"
#include "Gui_Def.h"
#include "pch.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "common/GuiCom.h"
#include "common/CScene.h"
#include "../vdrift/pathmanager.h"
#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreOverlay.h>
#include "settings.h"
#include "common/MultiList2.h"
#include "common/Slider.h"
#include "common/Gui_Popup.h"
#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreOverlay.h>
#include <MyGUI_Gui.h>
#include <MyGUI_Widget.h>
#include <MyGUI_Button.h>
#include <MyGUI_Window.h>
#include <MyGUI_TabControl.h>
using namespace MyGUI;
using namespace Ogre;
using namespace std;


CGui::CGui(App* app1)
{
	app = app1;
	pSet = app1->pSet;
	sc = app1->scn->sc;
	pGame = app1->pGame;
	hud = app1->hud;
	data = app1->scn->data;

	int i;
	for (i=0; i < iCarSt; ++i)
	{	txCarStTxt[i]=0;  txCarStVals[i]=0;  barCarSt[i]=0;  }

	for (i=0; i < ciEdCar; ++i)
		edCar[i] = 0;
}

CGui::~CGui()
{
}


//  Main menu
//----------------------------------------------------------------------------------------------------------------
void CGui::InitMainMenu()
{
	Btn btn;
	for (int i=0; i < ciMainBtns; ++i)
	{
		const String s = toStr(i);
		app->mWndMainPanels[i] = fWP("PanMenu"+s);
		Btn("BtnMenu"+s, btnMainMenu);  app->mWndMainBtns[i] = btn;
	}
	for (int i=0; i < ciRaceBtns; ++i)
	{
		const String s = toStr(i);
		app->mWndRacePanels[i] = fWP("PanRace"+s);
		Btn("BtnRace"+s, btnMainMenu);  app->mWndRaceBtns[i] = btn;
	}

	//  center
	int wx = app->mWindow->getWidth(), wy = app->mWindow->getHeight();
	
	Wnd wnd = app->mWndMain;  IntSize w = wnd->getSize();
	wnd->setPosition((wx-w.width)*0.5f, (wy-w.height)*0.5f);
	
	wnd = app->mWndRace;  w = wnd->getSize();
	wnd->setPosition((wx-w.width)*0.5f, (wy-w.height)*0.5f);


	//  Difficulty  ---
	Cmb(diffList, "DiffList", comboDiff);

	auto add = [&](int diff)
	{
		auto clr = gcom->getClrDiff(diff);
		diffList->addItem(clr+ TR("#{Diff"+toStr(diff)+"}"));
	};
	add(1);  add(2);  add(3);  add(4);  add(5);  add(6);  add(7);
	diffList->setIndexSelected(pSet->difficulty);

	//  Simulation  ---
	Cmb(simList, "SimList", comboSim);
	auto sim = [&](int clrDiff, int diff, String s)
	{
		auto clr = gcom->getClrDiff(clrDiff);
		simList->addItem(clr+ TR("#{Diff"+toStr(diff)+"}"));
		
		StringUtil::toLowerCase(s);
		if (pSet->gui.sim_mode == s)
			simList->setIndexSelected(simList->getItemCount()-1);
	};
	sim(1,2,"Easy");  sim(4,3,"Normal");
	//sim(6,4,"Hard");  // WIP test .. car HI
}


void CGui::btnMainMenu(WP wp)
{
	for (int i=0; i < ciMainBtns; ++i)
		if (wp == app->mWndMainBtns[i])
		{	switch (i)
			{
			case Menu_Race:     pSet->iMenu = MN1_Race;  break;
			case Menu_Replays:  pSet->iMenu = MN_Replays;  break;
			case Menu_Help:     pSet->iMenu = MN_Help;  break;
			case Menu_Options:  pSet->iMenu = MN_Options;  break;
			}
			app->gui->toggleGui(false);
			return;
		}
	for (int i=0; i < ciRaceBtns; ++i)
		if (wp == app->mWndRaceBtns[i])
		{	switch (i)
			{
			case Race_Single:     pSet->iMenu = MN_Single;  break;
			case Race_Tutorial:   pSet->iMenu = MN_Tutorial;  break;
			case Race_Champ:      pSet->iMenu = MN_Champ;  break;
			case Race_Challenge:  pSet->iMenu = MN_Chall;  break;
			
			case Race_HowToPlay:  pSet->iMenu = MN_HowTo;  break;
			case Race_Back:       pSet->iMenu = MN1_Main;  break;
			}
			app->gui->toggleGui(false);
			return;
		}
}

void CGui::tabMainMenu(Tab tab, size_t id)
{
	//_  game tab change
	if (tab == app->mWndTabsGame)
	{
		if (id == TAB_Car)
			app->gui->CarListUpd();  //  off filtering by chall
	
		app->mWndTrkFilt->setVisible(false);  //

		if (id == TAB_Multi)
		{	//  back to mplr tab, upload game info
									//_ only for host..
			if (app->mMasterClient && app->gui->valNetPassword->getVisible())
			{	app->gui->uploadGameInfo();
				app->gui->updateGameInfoGUI();
			}
			//- app->gui->evBtnNetRefresh(0);  // upd games list (don't, breaks game start)
		}
	}
	
	if (id != 0)  return;  // <back
	tab->setIndexSelected(1);  // dont switch to 0
	if (pSet->iMenu >= MN_Single && pSet->iMenu <= MN_Chall)
		pSet->iMenu = MN1_Race;
	else
		pSet->iMenu = MN1_Main;
	app->gui->toggleGui(false);  // back to main
}


//  Game Simulation change  Race menu
//---------------------------------------------------------
void CGui::comboSim(Cmb cmb, size_t val)
{
	int damage = 0;
	switch (val)
	{
	case 0:  pSet->gui.sim_mode = "easy";    damage = 1;  break;
	case 1:  pSet->gui.sim_mode = "normal";  damage = 2;  break;
	case 2:  pSet->gui.sim_mode = "hard";    damage = 2;  break;
	}
	//  game
	pSet->gui.damage_type = damage;  cmbDamage->setIndexSelected(damage);

	bReloadSim = true;
	tabTireSet(0,iTireSet);  listCarChng(carList,0);
}


//  Game Difficulty change  Race menu
//----------------------------------------------------------------------------------------------------------------
void CGui::comboDiff(Cmb cmb, size_t val)
{
	LogO("+ comboDiff" + toStr(val));
	pSet->difficulty = val;

	auto resetFilter = [&]()
	{
		for (int i=0; i < COL_FIL; ++i)
		{	pSet->col_fil[0][i] = pSet->colFilDef[0][i];
			pSet->col_fil[1][i] = pSet->colFilDef[1][i];
	}	};

	auto SetDiff = [&](bool sortUp, int sortCol,  bool filter, int diffMax,
		int pipes, int jumps, int len,
		bool beam, bool arrow, bool trail, bool minimap,
		string car, string track)
	{
		//  tracks
		gcom->trkList->mSortColumnIndex = pSet->tracks_sort = sortCol;
		gcom->trkList->mSortUp = pSet->tracks_sortup = sortUp;  gcom->trkList->mSortUpOld = !sortUp;
		pSet->tracks_filter = filter;  resetFilter();  gcom->ckTrkFilter.SetValue(filter);

		pSet->col_fil[1][1] = diffMax;  gcom->svTrkFilMax[1].SetValueI(diffMax);  // upd filt wnd gui
		pSet->col_fil[1][7] = jumps;  gcom->svTrkFilMax[7].SetValueI(jumps);
		pSet->col_fil[1][9] = pipes;  gcom->svTrkFilMax[9].SetValueI(pipes);
		pSet->col_fil[1][13] = len;  gcom->svTrkFilMax[13].SetValueI(len);
		//  trk, car
		pSet->gui.track = track;
		pSet->gui.car[0] = car;
		for (int i=0; i < carList->getItemCount(); ++i)
			if (carList->getItemNameAt(i).substr(7) == car)
				carList->setIndexSelected(i);
		//  hud
		ckBeam.SetValue(beam);  ckArrow.SetValue(arrow);
		ckTrailShow.SetValue(trail);  ckMinimap.SetValue(minimap);
	};

	switch (val)
	{// up,col, filt,diff, pipes,jmp,len  bm,ar,tr,mi  car,trk
	case 0:  SetDiff(1,6,  1,2, 0,0, 3,  1,1,1,1, "V2", "Isl2-Sandy");  break;
	case 1:  SetDiff(1,6,  1,3, 1,1, 6,  0,1,1,1, "ES", "Isl12-Beach");  break;  //Isl5-Shore
	case 2:  SetDiff(1,6,  1,4, 2,2, 9,  0,0,1,1, "HI", "Jng6-Fun");  break;  // Isl6-Flooded
	case 3:  SetDiff(0,3,  1,5, 4,4,14,  0,0,1,1, "HI", "Isl14-Ocean");  break;
	case 4:  SetDiff(0,3,  0,6, 4,4,24,  0,0,0,1, "SX", "Grc9-Oasis");  break;
	case 5:  SetDiff(0,17, 0,6, 4,4,24,  0,0,0,0, "SX", "Mos5-Factory");  break;  // Isl17-AdapterIslands
	case 6:  SetDiff(0,17, 0,6, 4,4,24,  0,0,0,0, "U6", "Uni7-GlassStairs");  break;
	}
	app->mWndRaceBtns[1]->setVisible(val < 4);  // tutorials
	gcom->TrackListUpd(true);  gcom->listTrackChng(gcom->trkList,0);
	listCarChng(carList,0);

	ChampsListUpdate();
	ChallsListUpdate();
}
