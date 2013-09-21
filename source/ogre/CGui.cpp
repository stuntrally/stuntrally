#include "pch.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "../vdrift/pathmanager.h"


CGui::CGui(App* app1)
	: app(app1), bGI(0)
	// gui
	,carList(0), btRplPl(0)
	// hud
	,valCountdownTime(0)
	,cmbGraphs(0), valGraphsType(0) //,slGraphT(0)
	// view
	,bRkmh(0),bRmph(0)
	,chFps(0), chWire(0), chProfTxt(0), chGraphs(0)
	,chTimes(0),chMinimp(0),chOpponents(0)
	// dbg
	,chDbgT(0),chDbgB(0),chDbgS(0)
	,chBlt(0),chBltTxt(0), chTireVis(0)
	// car
	, bRsimEasy(0), bRsimNorm(0), bReloadSim(true)
	,valCarClrH(0), valCarClrS(0), valCarClrV(0)
	,valCarClrGloss(0), valCarClrRefl(0)
	,valRplNumViewports(0) // setup
	,valSSSEffect(0), valSSSVelFactor(0)
	,valSteerRangeSurf(0), valSteerRangeSim(0)
	, iTireSet(0), bchAbs(0),bchTcs(0)
	,slSSSEff(0),slSSSVel(0)
	,slSteerRngSurf(0),slSteerRngSim(0)
	// replay
	,imgCar(0),carDesc(0)
	,cmbBoost(0), cmbFlip(0), cmbDamage(0), cmbRewind(0)
	,valLocPlayers(0)
	,txCarStatsTxt(0), txCarStatsVals(0)
	,txCarSpeed(0),txCarType(0)
	,txCarAuthor(0),txTrackAuthor(0)
	,valRplPerc(0), valRplCur(0), valRplLen(0)
	,slRplPos(0), rplList(0)
	,valRplName(0),valRplInfo(0)
	,valRplName2(0),valRplInfo2(0)
	,edRplName(0), edRplDesc(0)
	,rbRplCur(0), rbRplAll(0), bRplBack(0),bRplFwd(0)
	// gui multiplayer
	,netGuiMutex(), netGameInfo()
	,bUpdChat(false), iChatMove(0)
	,bRebuildPlayerList(0), bRebuildGameList(0), bUpdateGameInfo(0)
	,bStartGame(0), bStartedGame(0)
	,tabsNet(0)
	,panelNetServer(0), panelNetGame(0), panelNetTrack(0)
	,listServers(0), listPlayers(0)
	,edNetChat(0), liNetEnd(0)
    ,btnNetRefresh(0), btnNetJoin(0), btnNetCreate(0), btnNetDirect(0)
    ,btnNetReady(0), btnNetLeave(0)
    ,valNetGameName(0), valNetChat(0)
    ,valNetGameInfo(0), valNetPassword(0), valTrkNet(0)
    ,edNetGameName(0), edNetChatMsg(0), edNetPassword(0)
    ,edNetNick(0), edNetServerIP(0), edNetServerPort(0), edNetLocalPort(0)
    ,iColLock(0),iColHost(0),iColPort(0)
	// chs
	,liChamps(0),liStages(0), liChalls(0), txtCh(0),valCh(0), panCh(0)
	,edChampStage(0),edChampEnd(0), edChallStage(0),edChallEnd(0)
	, pChall(0)
	,edChInfo(0), edChDesc(0)
	,btStTut(0),  tabTut(0),  imgTut(0)
	,btStChamp(0),tabChamp(0),imgChamp(0)
	,btStChall(0),tabChall(0),imgChall(0), btChRestart(0)
	,btChampStage(0),btChallStage(0), valStageNum(0)
	,imgChampStage(0),imgChallStage(0), imgChampEndCup(0)
	,imgChallFail(0),imgChallCup(0), txChallEndC(0),txChallEndF(0)
	// other
	,edPerfTest(0),edTweakCol(0)
	,tabTweak(0),tabEdCar(0)
	,txtTweakPath(0),cmbTweakCarSet(0)
	,cmbTweakTireSet(0),txtTweakTire(0)
	,txtTweakPathCol(0)
	, loadReadme(1)
	, bUpdCarClr(1), iCurCar(0)
	// input
	,mBindingAction(NULL), mBindingSender(NULL), tabInput(0)
	,txtInpDetail(0), panInputDetail(0), edInputIncrease(0), chOneAxis(0)
{
	pSet = app1->pSet;
	sc = app1->sc;
	pGame = app1->pGame;
	hud = app1->hud;
	data = app1->data;

	int i,c;
	for (i=0; i<3; ++i)
	{	txtChP[i]=0;  valChP[i]=0;  }

	sListCar = "";

	for (i=0; i < ciEdCar; ++i)
		edCar[i] = 0;
}
