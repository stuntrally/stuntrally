#include "Defines.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "SplitScreenManager.h"
#include "../paged-geom/PagedGeometry.h"


//  ctors  -----------------------------------------------
App::App()
	:pGame(0), ndPos(0), ndMap(0), ndLine(0)
	,nrpmB(0),nvelBk(0),nvelBm(0), nrpm(0),nvel(0), mrpm(0),mvel(0),mpos(0)
	,hudGear(0),hudVel(0), hudAbs(0),hudTcs(0), hudTimes(0), hudCheck(0)
	,ovGear(0),ovVel(0), ovAbsTcs(0), ovCarDbg(0),ovCarDbgTxt(0), ovCam(0), ovTimes(0)
	// hud
	,asp(1),  xcRpm(0), ycRpm(0), xcVel(0), ycVel(0)
	,fMiniX(0),fMiniY(0), scX(1),scY(1), ofsX(0),ofsY(0), minX(0),maxX(0), minY(0),maxY(0)
	// ter
	,mTerrainGlobals(0), mTerrainGroup(0), mPaging(false)
	,mTerrainPaging(0), mPageManager(0)
	// gui
	,mToolTip(0), mToolTipTxt(0), carList(0), trkList(0), resList(0)
	,valAnisotropy(0), valViewDist(0), valTerDetail(0), valTerDist(0), valRoadDist(0)  // detail
	,valTrees(0), valGrass(0), valTreesDist(0), valGrassDist(0)  // paged
	,valReflSkip(0), valReflSize(0), valReflFaces(0), valReflDist(0)  // refl
	,valShaders(0), valShadowType(0), valShadowCount(0), valShadowSize(0), valShadowDist(0)  // shadow
	,valSizeGaug(0), valSizeMinmap(0)  // view
	,bRkmh(0),bRmph(0), chDbgT(0),chDbgB(0), chBlt(0),chBltTxt(0), chFps(0), chTimes(0),chMinimp(0), bnQuit(0)
	,imgCar(0),imgPrv(0),imgMini(0),imgTer(0), valCar(0),valTrk(0),trkDesc(0)
	,valRplPerc(0), valRplCur(0), valRplLen(0), slRplPos(0), rplList(0)
	,valRplName(0),valRplInfo(0),valRplName2(0),valRplInfo2(0), edRplName(0), edRplDesc(0)
	,bRplPlay(0), bRplPause(0), bRplRec(0)
	// game
	,blendMtr(0), iBlendMaps(0)
	,dbgdraw(0)
	,grass(0), trees(0), road(0)
	,pr(0),pr2(0), sun(0)
{
	pathTrk[0] = PATHMANAGER::GetTrackPath() + "/";
	pathTrk[1] = PATHMANAGER::GetTrackPathUser() + "/";

	resCar = "";  resTrk = "";  resDrv = "";
		
	for (int i=0; i < 5; ++i)
	{	ovL[i]=0;  ovR[i]=0;  ovS[i]=0;  ovU[i]=0;  }
	for (int i=0; i < StTrk; ++i)  stTrk[i] = 0;
	
	//  util for update rot
	Quaternion qr;  {
	QUATERNION <double> fix;  fix.Rotate(PI_d, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixCar = qr;  }
	QUATERNION <double> fix;  fix.Rotate(PI_d/2, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixWh = qr;
}

String App::TrkDir() {
	int u = pSet->track_user ? 1 : 0;			return pathTrk[u] + pSet->track + "/";  }

String App::PathListTrk(int user) {
	int u = user == -1 ? bListTrackU : user;	return pathTrk[u] + sListTrack;  }

App::~App()
{
	delete road;
	if (mTerrainPaging)
	{
		OGRE_DELETE mTerrainPaging;
		OGRE_DELETE mPageManager;
	}else
		OGRE_DELETE mTerrainGroup;
	OGRE_DELETE mTerrainGlobals;

	OGRE_DELETE dbgdraw;
}

void App::setTranslations()
{	
	// loading states
	loadingStates.insert(std::make_pair(LS_CLEANUP, String(TR("#{LS_CLEANUP}"))));
	loadingStates.insert(std::make_pair(LS_GAME, String(TR("#{LS_GAME}"))));
	loadingStates.insert(std::make_pair(LS_SCENE, String(TR("#{LS_SCENE}"))));
	loadingStates.insert(std::make_pair(LS_CAR, String(TR("#{LS_CAR}"))));
	loadingStates.insert(std::make_pair(LS_TER, String(TR("#{LS_TER}"))));
	loadingStates.insert(std::make_pair(LS_TRACK, String(TR("#{LS_TRACK}"))));
	loadingStates.insert(std::make_pair(LS_MISC, String(TR("#{LS_MISC}"))));
	
	// Kind of nasty to have it here, but this has to be called after configure()...
	mSplitMgr->pApp = this;
}

void App::destroyScene()
{
	// Delete all cars
	for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
	{
		delete (*it);
	}
	carModels.clear();
	newPosInfos.clear();
	
	///  save replay - manual save, button
	/*if (pSet->rpl_rec)
	{
		string file = PATHMANAGER::GetReplayPath() + "/" + pSet->track + ".rpl";
		if (replay.GetTimeLength() > 4.f)
			replay.SaveFile(file);
	}
	/**/

	mToolTip = 0;  //?

	if (road)
	{	road->DestroyRoad();  delete road;  road = 0;  }

	if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }

	if (pGame)
		pGame->End();
	delete[] sc.td.hfData;
	delete[] blendMtr;  blendMtr = 0;

	BaseApp::destroyScene();
}

ManualObject* App::Create2D(const String& mat, SceneManager* sceneMgr, Real s, bool dyn)
{
	ManualObject* m = sceneMgr->createManualObject();
	m->setDynamic(dyn);
	m->setUseIdentityProjection(true);
	m->setUseIdentityView(true);
	m->setCastShadows(false);

	m->estimateVertexCount(4);
	m->begin(mat, RenderOperation::OT_TRIANGLE_FAN);

	m->position(-s,-s*asp, 0);  m->textureCoord(0, 1);
	m->position( s,-s*asp, 0);  m->textureCoord(1, 1);
	m->position( s, s*asp, 0);  m->textureCoord(1, 0);
	m->position(-s, s*asp, 0);  m->textureCoord(0, 0);
	m->end();
 
	AxisAlignedBox aabInf;	aabInf.setInfinite();
	m->setBoundingBox(aabInf);  // always visible
	m->setRenderQueueGroup(RENDER_QUEUE_OVERLAY - 1);
	return m;
}

//  hud util
String App::GetTimeString(float time) const
{
	int min = (int) time / 60;
	float secs = time - min*60;

	if (time != 0.0)
	{
		char tempchar[128];
		sprintf(tempchar, "%d:%05.2f", min, secs);  //"%d:%06.3f"
		return tempchar;
	}
	else
		return "-:--.---";
}
