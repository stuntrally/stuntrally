#include "pch.h"
#include "CHud.h"
#include "CGui.h"
#include "../vdrift/pathmanager.h"


CGui::CGui(App* ap1)
	: app(ap1), bGI(0)
	// gui
	,mToolTip(0), mToolTipTxt(0), carList(0), trkList(0), resList(0), btRplPl(0)
	,valAnisotropy(0), valViewDist(0), valTerDetail(0), valTerDist(0), valRoadDist(0)  // detail
	,valTexSize(0), valTerMtr(0), valTerTripl(0)  // detail
	,valTrees(0), valGrass(0), valTreesDist(0), valGrassDist(0)  // paged
	,valReflSkip(0), valReflSize(0), valReflFaces(0), valReflDist(0), valWaterSize(0)  // refl
	,valShaders(0), valShadowType(0), valShadowCount(0), valShadowSize(0), valShadowDist(0)//, valShadowFilter(0)  // shadow
	,valSizeGaug(0),valTypeGaug(0),valLayoutGaug(0), valSizeMinimap(0), valZoomMinimap(0)
	,valCountdownTime(0), valDbgTxtClr(0),valDbgTxtCnt(0)
	,cmbGraphs(0), valGraphsType(0) //,slGraphT(0)  // view
	,bRkmh(0),bRmph(0), chDbgT(0),chDbgB(0),chDbgS(0), chBlt(0),chBltTxt(0), chTireVis(0)
	,chFps(0), chWire(0), chProfTxt(0), chGraphs(0)
	,chTimes(0),chMinimp(0),chOpponents(0)
	,valVolMaster(0),valVolEngine(0),valVolTires(0),valVolSusp(0),valVolEnv(0)  // sounds
	,valVolFlSplash(0),valVolFlCont(0),valVolCarCrash(0),valVolCarScrap(0)
	,valBloomInt(0), valBloomOrig(0), valBlurIntens(0)  // video
	,valDepthOfFieldFocus(0), valDepthOfFieldFar(0)  // dof
	,valHDRParam1(0), valHDRParam2(0), valHDRParam3(0)  // hdr
	,valHDRBloomInt(0), valHDRBloomOrig(0), valHDRAdaptationScale(0)
	,valHDRVignettingRadius(0), valHDRVignettingDarkness(0)
	// car
	,bRsimEasy(0), bRsimNorm(0), bReloadSim(true)
	,valCarClrH(0), valCarClrS(0), valCarClrV(0), valCarClrGloss(0), valCarClrRefl(0)
	,valNumLaps(0), valRplNumViewports(0) // setup
	,valSSSEffect(0), valSSSVelFactor(0), valSteerRangeSurf(0), valSteerRangeSim(0)
	// rpl
	,imgCar(0),carDesc(0), imgTrkIco1(0),imgTrkIco2(0), bnQuit(0)
	,cmbBoost(0), cmbFlip(0), cmbDamage(0), cmbRewind(0)
	,valLocPlayers(0), edFind(0)
	,txCarStatsTxt(0), txCarStatsVals(0)
	,txCarSpeed(0),txCarType(0), txCarAuthor(0),txTrackAuthor(0)
	,valRplPerc(0), valRplCur(0), valRplLen(0), slRplPos(0), rplList(0)
	,valRplName(0),valRplInfo(0),valRplName2(0),valRplInfo2(0), edRplName(0), edRplDesc(0)
	,rbRplCur(0), rbRplAll(0), bRplBack(0),bRplFwd(0)
	,bGuiReinit(0)
	// gui multiplayer
	,netGuiMutex(), sChatBuffer(), netGameInfo(), bUpdChat(false), iChatMove(0)
	,bRebuildPlayerList(), bRebuildGameList(), bUpdateGameInfo(), bStartGame(), bStartedGame(0)
	,tabsNet(0), panelNetServer(0), panelNetGame(0), panelNetTrack(0)
	,listServers(0), listPlayers(0), edNetChat(0), liNetEnd(0)
    ,btnNetRefresh(0), btnNetJoin(0), btnNetCreate(0), btnNetDirect(0)
    ,btnNetReady(0), btnNetLeave(0)
    ,valNetGameName(0), valNetChat(0), valNetGameInfo(0), valNetPassword(0), valTrkNet(0)
    ,edNetGameName(0), edNetChatMsg(0), edNetPassword(0)
    ,edNetNick(0), edNetServerIP(0), edNetServerPort(0), edNetLocalPort(0)
    ,iColLock(0),iColHost(0),iColPort(0)
	// chs
	,liChamps(0),liStages(0), liChalls(0), txtCh(0),valCh(0), panCh(0)
	,edChampStage(0),edChampEnd(0), edChallStage(0),edChallEnd(0)
	,edChInfo(0), edChDesc(0), pChall(0)
	,btStTut(0),  tabTut(0),  imgTut(0)
	,btStChamp(0),tabChamp(0),imgChamp(0)
	,btStChall(0),tabChall(0),imgChall(0), btChRestart(0)
	,btChampStage(0),btChallStage(0), valStageNum(0)
	,imgChampStage(0),imgChallStage(0), imgChampEndCup(0)
	,imgChallFail(0),imgChallCup(0), txChallEndC(0),txChallEndF(0)
	// other
	,edPerfTest(0),edTweakCol(0),tabTweak(0),tabEdCar(0)
	,iTireSet(0), bchAbs(0),bchTcs(0), slSSSEff(0),slSSSVel(0), slSteerRngSurf(0),slSteerRngSim(0)
	,txtTweakPath(0),cmbTweakCarSet(0), cmbTweakTireSet(0),txtTweakTire(0), txtTweakPathCol(0)
	,loadReadme(1)
	,mBindingAction(NULL), mBindingSender(NULL), tabInput(0)
	,txtInpDetail(0), panInputDetail(0), edInputIncrease(0), chOneAxis(0)
	,bUpdCarClr(1), bListTrackU(0), iCurCar(0)
{
	pSet = ap1->pSet;
	sc = ap1->sc;
	pGame = ap1->pGame;
	hud = ap1->hud;

	int i,c;
	for (i=0; i<3; ++i)
	{	txtChP[i]=0;  valChP[i]=0;  }


	for (c=0; c < 2; ++c)
	{
		trkDesc[c]=0;  imgPrv[c]=0; imgMini[c]=0; imgTer[c]=0;
		for (i=0; i < StTrk;  ++i)  stTrk[c][i] = 0;
		for (i=0; i < InfTrk; ++i)  infTrk[c][i] = 0;
	}	
	pathTrk[0] = PATHMANAGER::Tracks() + "/";
	pathTrk[1] = PATHMANAGER::TracksUser() + "/";
	sListCar = "";  sListTrack = "";

	for (i=0; i < ciEdCar; ++i)
		edCar[i] = 0;
}
