#include "pch.h"
#include "common/Defines.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "SplitScreen.h"
#include "../paged-geom/PagedGeometry.h"
#include "common/RenderConst.h"
#include "common/MaterialGen/MaterialFactory.h"

#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreTerrainPaging.h>
#include <OgrePageManager.h>
#include <OgreManualObject.h>
using namespace Ogre;


//  ctors  -----------------------------------------------
App::App(SETTINGS *settings, GAME *game)
	:pGame(game), ndLine(0), bGI(0), mThread()
	// ovr
	,hudCountdown(0),hudNetMsg(0), hudAbs(0),hudTcs(0)
	,hudTimes(0), hudWarnChk(0),hudWonPlace(0), hudOppB(0)
	,ovCountdown(0),ovNetMsg(0), ovAbsTcs(0), ovCarDbg(0),ovCarDbgTxt(0)
	,ovCam(0), ovTimes(0), ovWarnWin(0), ovOpp(0)
	// hud
	,asp(1)//,  xcRpm(0), ycRpm(0), xcVel(0), ycVel(0)
	,scX(1),scY(1), minX(0),maxX(0), minY(0),maxY(0)
	,arrowNode(0),arrowRotNode(0)
	// ter
	,mTerrainGlobals(0), mTerrainGroup(0), terrain(0), mPaging(false)
	,mTerrainPaging(0), mPageManager(0)
	// gui
	,mToolTip(0), mToolTipTxt(0), carList(0), trkMList(0), resList(0), btRplPl(0)
	,valAnisotropy(0), valViewDist(0), valTerDetail(0), valTerDist(0), valRoadDist(0)  // detail
	,valTrees(0), valGrass(0), valTreesDist(0), valGrassDist(0)  // paged
	,valReflSkip(0), valReflSize(0), valReflFaces(0), valReflDist(0), valWaterSize(0)  // refl
	,valShaders(0), valShadowType(0), valShadowCount(0), valShadowSize(0), valShadowDist(0), valShadowFilter(0)  // shadow
	,valSizeGaug(0),valTypeGaug(0), valSizeMinimap(0), valZoomMinimap(0)
	,valCountdownTime(0),valGraphsType(0),slGraphT(0)  // view
	,bRkmh(0),bRmph(0), chDbgT(0),chDbgB(0), chBlt(0),chBltTxt(0)
	,chFps(0), chWire(0), chProfTxt(0), chGraphs(0)
	,chTimes(0),chMinimp(0),chOpponents(0)
	,valVolMaster(0),valVolEngine(0),valVolTires(0),valVolSusp(0),valVolEnv(0)  // sounds
	,valVolFlSplash(0),valVolFlCont(0),valVolCarCrash(0),valVolCarScrap(0)
	,imgCar(0), imgTrkIco1(0),imgTrkIco2(0), bnQuit(0)
	,valCar(0), valLocPlayers(0), edFind(0)
	,valRplPerc(0), valRplCur(0), valRplLen(0), slRplPos(0), rplList(0)
	,valRplName(0),valRplInfo(0),valRplName2(0),valRplInfo2(0), edRplName(0), edRplDesc(0)
	,rbRplCur(0), rbRplAll(0), rbRplGhosts(0), bRplBack(0),bRplFwd(0), newGameRpl(0)
	,bRplPlay(0), bRplPause(0), bRplRec(0), bRplWnd(1), bGuiReinit(0)
	// gui multiplayer
	,netGuiMutex(), sChatBuffer(), netGameInfo(), bUpdChat(false), iChatMove(0)
	,bRebuildPlayerList(), bRebuildGameList(), bUpdateGameInfo(), bStartGame()
	,tabsNet(0), panelNetServer(0), panelNetGame(0), panelNetTrack(0)
	,listServers(0), listPlayers(0), edNetChat(0), imgNetTrack(0)
    ,btnNetRefresh(0), btnNetJoin(0), btnNetCreate(0), btnNetDirect(0)
    ,btnNetReady(0), btnNetLeave(0)
    ,valNetGames(0), valNetGameName(0), valNetChat(0), valNetTrack(0), valNetPassword(0)
    ,edNetGameName(0), edNetChatMsg(0), edNetTrackInfo(0), edNetPassword(0)
    ,edNetNick(0), edNetServerIP(0), edNetServerPort(0), edNetLocalPort(0)
    ,iColLock(0),iColHost(0),iColPort(0)
	// game
	,blendMtr(0), iBlendMaps(0), dbgdraw(0), noBlendUpd(0), blendMapSize(513), bListTrackU(0)
	,grass(0), trees(0), road(0)
	,pr(0),pr2(0), sun(0), carIdWin(-1), iCurCar(0), bUpdCarClr(1), iRplCarOfs(0)
	,lastAxis(-1), axisCnt(0), txtJAxis(0), txtJBtn(0), txtInpDetail(0), panInputDetail(0)
	,edInputMin(0), edInputMax(0), edInputMul(0), edInputReturn(0), edInputIncrease(0), actDetail(0), cmbInpDetSet(0)
	,liChamps(0),liStages(0), edChampStage(0),edChampEnd(0), imgChampStage(0), liNetEnd(0)
	,iEdTire(0),iCurLat(0),iCurLong(0),iCurAlign(0), iUpdTireGr(0)
{
	pSet = settings;
	int i,c;
	NullHUD();

	for (c=0; c < 2; ++c)
	{
		trkDesc[c]=0;  valTrk[c]=0;  imgPrv[c]=0; imgMini[c]=0; imgTer[c]=0;
		for (i=0; i < StTrk;  ++i)  stTrk[c][i] = 0;
		for (i=0; i < InfTrk; ++i)  infTrk[c][i] = 0;
	}	
	pathTrk[0] = PATHMANAGER::GetTrackPath() + "/";
	pathTrk[1] = PATHMANAGER::GetTrackPathUser() + "/";
	resCar = "";  resTrk = "";  resDrv = "";
	sListCar = "";  sListTrack = "";

	for (int o=0; o < 5; ++o)  for (c=0; c < 3; ++c)
		hudOpp[o][c] = 0;
		
	for (i=0; i < 5; ++i)
	{	ovL[i]=0;  ovR[i]=0;  ovS[i]=0;  ovU[i]=0;  }
	
	//  util for update rot
	Quaternion qr;  {
	QUATERNION <double> fix;  fix.Rotate(PI_d, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixCar = qr;  }
	QUATERNION <double> fix;  fix.Rotate(PI_d/2, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixWh = qr;

	for (i=0; i < 4; ++i)
	{	txGear[i]=0;  txVel[i]=0;  txBFuel[i]=0;  }

	if (pSet->multi_thr)
		mThread = boost::thread(boost::bind(&App::UpdThr, boost::ref(*this)));;
}

void App::NullHUD()
{
	int i,c;
	for (i=0; i < 4; ++i)
	{	ndMap[i]=0;  moMap[i]=0;
		moRpm[i]=0;  moVel[i]=0;
		ndRpm[i]=0;  ndVel[i]=0;
		moRpmBk[i]=0;  moVelBk[i]=0;  moVelBm[i]=0;
		ndRpmBk[i]=0;  ndVelBk[i]=0;  ndVelBm[i]=0;
		for (c=0; c < 5; ++c)
		{	vNdPos[i][c]=0;  vMoPos[i][c]=0;  }
	}
}


String App::TrkDir() {
	int u = pSet->game.track_user ? 1 : 0;		return pathTrk[u] + pSet->game.track + "/";  }

String App::PathListTrk(int user) {
	int u = user == -1 ? bListTrackU : user;	return pathTrk[u] + sListTrack;  }

App::~App()
{
	mShutDown = true;
	if (mThread.joinable())
		mThread.join();

	delete road;
	if (mTerrainPaging) {
		OGRE_DELETE mTerrainPaging;
		OGRE_DELETE mPageManager;
	} else {
		OGRE_DELETE mTerrainGroup;
	}
	OGRE_DELETE mTerrainGlobals;

	OGRE_DELETE dbgdraw;
}

void App::postInit()
{
	mSplitMgr->pApp = this;
	
	materialFactory = new MaterialFactory();
	materialFactory->pApp = this;
	materialFactory->setSceneManager(mSceneMgr);
	materialFactory->setShadows(pSet->shadow_type >= 2);
	materialFactory->setShadowsDepth(pSet->shadow_type >= 3);
	materialFactory->setShadowsSoft(pSet->shadow_type == 4);
	materialFactory->setShaderQuality(pSet->shaders);
	materialFactory->setShadowsFilterSize(pSet->shadow_filter);
	materialFactory->setReflect(pSet->water_reflect);
	materialFactory->setRefract(pSet->water_refract);
	if (pSet->tex_size == 0)
		materialFactory->setTexSize(0);
	else if (pSet->tex_size == 1)
		materialFactory->setTexSize(4096);
}

void App::setTranslations()
{
	// loading states
	loadingStates.clear();
	loadingStates.insert(std::make_pair(LS_CLEANUP, String(TR("#{LS_CLEANUP}"))));
	loadingStates.insert(std::make_pair(LS_GAME, String(TR("#{LS_GAME}"))));
	loadingStates.insert(std::make_pair(LS_SCENE, String(TR("#{LS_SCENE}"))));
	loadingStates.insert(std::make_pair(LS_CAR, String(TR("#{LS_CAR}"))));

	loadingStates.insert(std::make_pair(LS_TER, String(TR("#{LS_TER}"))));
	loadingStates.insert(std::make_pair(LS_ROAD, String(TR("#{LS_ROAD}"))));
	loadingStates.insert(std::make_pair(LS_OBJS, String(TR("#{LS_OBJS}"))));
	loadingStates.insert(std::make_pair(LS_TREES, String(TR("#{LS_TREES}"))));

	loadingStates.insert(std::make_pair(LS_MISC, String(TR("#{LS_MISC}"))));
}

void App::recreateCarMtr()
{
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); ++it) {
		(*it)->RecreateMaterials(); (*it)->setMtrNames();
	}
}

void App::destroyScene()
{
	for (int i=0; i < graphs.size(); ++i)
		delete graphs[i];

	for (int i=0; i<4; ++i)
		pSet->cam_view[i] = carsCamNum[i];

	// Delete all cars
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); ++it)
		delete (*it);

	carModels.clear();
	//carPoses.clear();
	
	mToolTip = 0;  //?

	if (road)
	{	road->DestroyRoad();  delete road;  road = 0;  }

	if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }

	if (pGame)
		pGame->End();
	delete[] sc.td.hfHeight;
	delete[] sc.td.hfAngle;
	delete[] blendMtr;  blendMtr = 0;

	BaseApp::destroyScene();
}

ManualObject* App::Create2D(const String& mat, SceneManager* sceneMgr, Real s, bool dyn, bool clr)
{
	ManualObject* m = sceneMgr->createManualObject();
	m->setDynamic(dyn);
	m->setUseIdentityProjection(true);
	m->setUseIdentityView(true);
	m->setCastShadows(false);

	m->estimateVertexCount(4);
	m->begin(mat, RenderOperation::OT_TRIANGLE_STRIP);
	m->position(-s,-s*asp, 0);  m->textureCoord(0, 1);  if (clr)  m->colour(0,1,0);
	m->position( s,-s*asp, 0);  m->textureCoord(1, 1);  if (clr)  m->colour(0,0,0);
	m->position(-s, s*asp, 0);  m->textureCoord(0, 0);  if (clr)  m->colour(1,1,0);
	m->position( s, s*asp, 0);  m->textureCoord(1, 0);  if (clr)  m->colour(1,0,0);
	m->end();
 
	AxisAlignedBox aabInf;	aabInf.setInfinite();
	m->setBoundingBox(aabInf);  // always visible
	m->setRenderQueueGroup(RQG_Hud2);
	return m;
}

//  hud util
String App::GetTimeString(float time) const
{
	int min = (int) time / 60;
	float secs = time - min*60;

	if (time != 0.0)
	{
		String ss;
		ss = toStr(min)+":"+fToStr(secs,2,5,'0');
		return ss;
	}else
		return "-:--.---";
}

//  ghost filename
const String& App::GetGhostFile()
{
	static String file;
	file = PATHMANAGER::GetGhostsPath() + "/"
		+ pSet->game.track + (pSet->game.track_user ? "_u" : "") + (pSet->game.trackreverse ? "_r" : "")
		+ "_" + pSet->game.car[0] + ".rpl";
	return file;
}
