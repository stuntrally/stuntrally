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
	: app(app1), bGI(0)
	,mGui(0), gcom(0), popup(0)
	//  gui
	,carList(0), btRplPl(0)
	,btNewGameCar(0)
	//  hud
	,valCountdownTime(0)
	,cmbGraphs(0), valGraphsType(0)
	//  view
	,bRkmh(0),bRmph(0)
	//  car
	, bRsimEasy(0), bRsimNorm(0), bRsimHard(0), bReloadSim(true)
	,valRplNumViewports(0)
	,viewBox(0), viewCar(0)
	,graphV(0), graphVel(0),graphVGrid(0)
	,graphS(0), graphSSS(0),graphSGrid(0)
	//  setup
	, iTireSet(0)
	,bchAbs(0),bchTcs(0)
	//  replay
	,imgCar(0),carDesc(0),tbPlr(0),tbPlr2(0)
	,cmbBoost(0), cmbFlip(0), cmbDamage(0), cmbRewind(0)
	,valLocPlayers(0)
	,txCarSpeed(0),txCarType(0),txCarYear(0), barCarSpeed(0)
	,txCarRating(0), txCarDiff(0), txCarWidth(0)
	,txCarAuthor(0),txTrackAuthor(0)
	,valRplPerc(0), valRplCur(0), valRplLen(0)
	,slRplPos(0), rplList(0)
	,valRplName(0),valRplInfo(0)
	,valRplName2(0),valRplInfo2(0)
	,edRplName(0), edRplDesc(0)
	,rbRplCur(0), rbRplAll(0)
	, bRplBack(0),bRplFwd(0)
	//  gui multiplayer
	,netGuiMutex(), netGameInfo()
	,bUpdChat(false), iChatMove(0)
	,bRebuildPlayerList(0), bRebuildGameList(0), bUpdateGameInfo(0)
	,bStartGame(0), bStartedGame(0)
	,tabsNet(0)
	,panNetServer(0),panNetServer2(0), panNetGame(0), panNetTrack(0)
	,listServers(0), listPlayers(0)
	,edNetChat(0), liNetEnd(0)
	,btnNetRefresh(0), btnNetJoin(0), btnNetCreate(0), btnNetDirect(0)
	,btnNetReady(0), btnNetLeave(0)
	,valNetGameInfo(0), valNetPassword(0), valTrkNet(0)
	,edNetGameName(0), edNetChatMsg(0), edNetPassword(0)
	,edNetNick(0), edNetServerIP(0), edNetServerPort(0), edNetLocalPort(0)
	,iColLock(0),iColHost(0),iColPort(0)
	//  chs
	,liChamps(0),liStages(0), liChalls(0)
	,txtCh(0),valCh(0), panCh(0)
	,edChampStage(0),edChampEnd(0)
	,edChallStage(0),edChallEnd(0)
	, pChall(0)
	,edChInfo(0), edChDesc(0)
	,btStTut(0),  tabTut(0),  imgTut(0)
	,btStChamp(0),tabChamp(0),imgChamp(0)
	,btStChall(0),tabChall(0),imgChall(0), btChRestart(0)
	,btChampStage(0), btChallStage(0), valStageNum(0)
	,imgChampStage(0),imgChallStage(0), imgChampEndCup(0)
	,imgChallFail(0),imgChallCup(0)
	,txChallEndC(0),txChallEndF(0),txChampEndF(0)
	, iChSnd(0)
	//  other
	,edPerfTest(0),edTweakCol(0)
	,tabTweak(0),tabEdCar(0)
	,txtTweakPath(0)
	,edTweakTireSet(0),txtTweakTire(0)
	,liTwkTiresUser(0),liTwkTiresOrig(0)
	, idTwkSurf(-1), liTwkSurfaces(0)
	,txtTweakPathCol(0)
	, loadReadme(1)
	, iCurCar(0)
	,edHintTitle(0), edHintText(0), iHintCur(0), iHints(17)
	//  input
	,mBindingAction(NULL), mBindingSender(NULL)
	,tabInput(0)
	,txtInpDetail(0), panInputDetail(0)
	,edInputIncrease(0), chOneAxis(0)
{
	pSet = app1->pSet;
	sc = app1->scn->sc;
	pGame = app1->pGame;
	hud = app1->hud;
	data = app1->scn->data;

	int i,c;
	for (i=0; i < 3; ++i)
	{	txtChP[i]=0;  valChP[i]=0;  }

	for (i=0; i < iCarSt; ++i)
	{	txCarStTxt[i]=0;  txCarStVals[i]=0;  barCarSt[i]=0;  }

	sListCar = "";

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

	//  difficulty
	Cmb(diffList, "DiffList", comboDiff);
	diffList->addItem(TR("#{Diff2}"));
	diffList->addItem(TR("#{Diff3}"));
	diffList->addItem(TR("#{Diff4}"));
	diffList->addItem(TR("#{Diff5}"));
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


//  Game difficulty change  Race menu
//----------------------------------------------------------------------------------------------------------------
void CGui::comboDiff(Cmb cmb, size_t val)
{
	LogO("+ comboDiff");
	int i;

	auto resetFilter = [&]()
	{
		for (int i=0; i < COL_FIL; ++i)
		{	pSet->col_fil[0][i] = pSet->colFilDef[0][i];
			pSet->col_fil[1][i] = pSet->colFilDef[1][i];
	}	};

	auto SetDiff = [&](
		bool sortUp, int sortCol,  bool filt, int diffMax,
		bool beam, bool arrow, bool trail, bool easy)
	{
		//  tracks
		gcom->trkList->mSortColumnIndex = pSet->tracks_sort = sortCol;
		gcom->trkList->mSortUp = pSet->tracks_sortup = sortUp;
		pSet->tracks_filter = filt;  resetFilter();  // upd filt wnd gui ...
		pSet->col_fil[1][2] = diffMax;
		pSet->gui.sim_mode = easy ? "easy" : "normal";
		//  hud
		ckBeam.SetValue(beam);
		ckArrow.SetValue(arrow);
		ckTrailShow.SetValue(trail);
	};

	switch (val)
	{
	case 0:  SetDiff(1, 1,  1, 2,  1,1,1,1);  break;
	case 1:  SetDiff(1, 1,  1, 3,  0,0,1,1);  break;
	case 2:  SetDiff(0, 3,  1, 5,  0,0,1,0);  break;
	case 3:  SetDiff(0, 3,  0, 6,  0,0,0,0);  break;
	}
	gcom->TrackListUpd(true);
}
