#include "stdafx.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "FollowCamera.h"
#include "../road/Road.h"


//  ctors  -----------------------------------------------
App::App()
	:pGame(0), ndCar(0), ndPos(0), ndMap(0), ndLine(0)
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
	,mToolTip(0), mToolTipTxt(0), carList(0), trkList(0)
	,valAnisotropy(0), valViewDist(0), valTerDetail(0), valTerDist(0), valRoadDist(0)  // detail
	,valTrees(0), valGrass(0), valTreesDist(0), valGrassDist(0)  // paged
	,valReflSkip(0), valReflSize(0), valReflFaces(0), valReflDist(0)  // refl
	,valShaders(0), valShadowType(0), valShadowCount(0), valShadowSize(0), valShadowDist(0)  // shadow
	,valSizeGaug(0), valSizeMinmap(0)  // view
	,bRkmh(0),bRmph(0), chDbgT(0),chDbgB(0), chBlt(0),chFps(0), chTimes(0),chMinimp(0)
	,imgCar(0),imgPrv(0),imgMini(0),imgTer(0), valCar(0),valTrk(0),trkDesc(0), bSizeHUD(1)
	// game
	,bNew(false), blendMtr(0), iBlendMaps(0)
    ,miReflectCntr(0), miReflectCam(0), mReflAll1st(1)
	,dbgdraw(0),  bLoading(1), reflAct(0)
	,grass(0), trees(0), road(0)
	,pr(0),pr2(0), sun(0), 	vPofs(0,0,0)
{
	resCar = "";  resTrk = "";  resDrv = "";
	for (int w = 0; w < 4; ++w)
	{	ps[w] = 0;  pm[w] = 0;  pd[w] = 0;
		ndWh[w] = 0;  ndWhE[w] = 0; whTrl[w] = 0;
		ndRs[w] = 0;  ndRd[w] = 0;
		wht[w] = 0.f;  whTerMtr[w] = 0; }
	for (int i=0; i < 5; ++i)
	{	ovL[i]=0;  ovR[i]=0;  ovS[i]=0;  ovU[i]=0;  }
	for (int i=0; i < StTrk; ++i)  stTrk[i] = 0;

	for (int i=0; i < 6; ++i)
	{	mReflectCams[i] = 0;  mReflectRT[i] = 0;  }
}
String App::TrkDir()
{
	return PATHMANAGER::GetTrackPath() + "/" + pSet->track + "/";
}

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

void App::destroyScene()
{
	mToolTip = 0;  //?
    destroyReflectCams();

	//  destroy all
	//mSceneMgr->destroyAllManualObjects();
	//mSceneMgr->destroyAllEntities();
	//mSceneMgr->destroyAllStaticGeometry();

	////  par sys
	//if (pr)  {  mSceneMgr->destroyParticleSystem(pr);   pr=0;  }
	//if (pr2) {  mSceneMgr->destroyParticleSystem(pr2);  pr2=0;  }
	//for (int w=0; w < 4; w++)  {
	//	if (ps[w]) {  mSceneMgr->destroyParticleSystem(ps[w]);   ps[w]=0;  }
	//	if (pm[w]) {  mSceneMgr->destroyParticleSystem(pm[w]);   pm[w]=0;  }
	//	if (pd[w]) {  mSceneMgr->destroyParticleSystem(pd[w]);   pd[w]=0;  }  }

	//terrain = 0;
	//if (mTerrainGroup)
	//	mTerrainGroup->removeAllTerrains();
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


//  utility
//-------------------------------------------------------------------------------------
ManualObject* App::CreateModel(const String& mat, VERTEXARRAY* a, bool flip, bool track)
{
	int verts = a->vertices.size();
	if (verts == 0)  return NULL;
	int tcs   = a->texcoords[0].size(); //-
	int norms = a->normals.size();
	int faces = a->faces.size();
	// norms = verts, verts % 3 == 0

	ManualObject* m = mSceneMgr->createManualObject();
	m->begin(mat, RenderOperation::OT_TRIANGLE_LIST);

	int t = 0;
	if (track)
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(a->vertices[v+0], a->vertices[v+2], -a->vertices[v+1]);
			if (norms)
			m->normal(	a->normals [v+0], a->normals [v+2], -a->normals [v+1]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; ++f)
			m->index(a->faces[f]);
	}else
	if (flip)
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(a->vertices[v], a->vertices[v+1], a->vertices[v+2]);
			if (norms)
			m->normal(  a->normals [v], a->normals [v+1], a->normals [v+2]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; f += 3)
		{	m->index(a->faces[f+2]);  m->index(a->faces[f+1]);  m->index(a->faces[f]);	}
	}else
	{	for (int v = 0; v < verts; v += 3)
		{
			m->position(-a->vertices[v+1]+vPofs.x, -a->vertices[v+2]+vPofs.y, a->vertices[v]+vPofs.z);
			if (norms)
			m->normal(	-a->normals [v+1], -a->normals [v+2], a->normals [v]);
			if (t < tcs)
			{	m->textureCoord(a->texcoords[0][t], a->texcoords[0][t+1]);  t += 2;	}
		}
		for (int f = 0; f < faces; f += 3)
		{	m->index(a->faces[f+2]);  m->index(a->faces[f+1]);  m->index(a->faces[f]);	}
	}
	m->end();
	return m;
}

ManualObject* App::Create2D(const String& mat, Real s, bool dyn)
{
	ManualObject* m = mSceneMgr->createManualObject();
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
		sprintf(tempchar, "%d:%06.3f", min, secs);
		return tempchar;
	}
	else
		return "-:--.---";
}
