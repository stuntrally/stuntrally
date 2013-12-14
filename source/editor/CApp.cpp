#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/data/CData.h"
#include "../vdrift/pathmanager.h"
#include "CApp.h"
#include "CGui.h"
#include "../ogre/common/GuiCom.h"
#include "../road/Road.h"
#include "../paged-geom/PagedGeometry.h"
#include "../ogre/common/WaterRTT.h"
#include "../ogre/common/RenderBoxScene.h"

#include "../shiny/Main/Factory.hpp"
#include "../shiny/Platforms/Ogre/OgrePlatform.hpp"
#include <OgreTerrainPaging.h>
#include <OgreTerrainGroup.h>

#if OGRE_PLATFORM != OGRE_PLATFORM_WIN32
	// dir listing
	#include <dirent.h>
	#include <sys/types.h>
#endif
using namespace Ogre;


//  ctor
//----------------------------------------------------------------------------------------------------------------------
App::App(SETTINGS* pSet1)
	:mTerrainGroup(0), mTerrainPaging(0), mPageManager(0), mTerrainGlobals(0)
	,bTerUpd(0), curBr(0)
	,ndPos(0), mpos(0), asp(4.f/3.f)
	,ndCar(0),entCar(0), ndStBox(0),entStBox(0), ndFluidBox(0),entFluidBox(0), ndObjBox(0),entObjBox(0)
	,grass(0), trees(0), sun(0), pr(0),pr2(0)
	,eTrkEvent(TE_None), bNewHmap(0), bTrGrUpd(0)
	,iFlCur(0), bRecreateFluids(0)
	
	,bTerUpdBlend(1), track(0)
	,world(0), config(0), dispatcher(0), broadphase(0), solver(0)  //blt
	,trackObject(0), trackMesh(0)
	,mStaticGeom(0), mTimer(0.f), bUpdTerPrv(0)
	//  objs
	,iObjCur(-1), iObjTNew(0), iObjLast(0)
	,objSim(0), objEd(EO_Move)
	,inst(0)
{
	pSet = pSet1;
	
	mBrSize[0] = 16.f;	mBrSize[1] = 24.f;	mBrSize[2] = 16.f;	mBrSize[3] = 16.f;
	mBrIntens[0] = 20.f;mBrIntens[1] = 20.f;mBrIntens[2] = 20.f;mBrIntens[3] = 20.f;
	mBrPow[0] = 2.f;	mBrPow[1] = 2.f;	mBrPow[2] = 2.f;	mBrPow[3] = 2.f;
	mBrFq[0] = 1.f;		mBrFq[1] = 1.f;		mBrFq[2] = 1.f;		mBrFq[3] = 1.f;
	mBrNOf[0] = 0.f;	mBrNOf[1] = 0.f;	mBrNOf[2] = 0.f;	mBrNOf[3] = 0.f;
	mBrOct[0] = 5;		mBrOct[1] = 5;		mBrOct[2] = 5;		mBrOct[3] = 5;
	mBrShape[0] = BRS_Sinus;  mBrShape[1] = BRS_Sinus;
	mBrShape[2] = BRS_Sinus;  mBrShape[3] = BRS_Sinus;
	terSetH = 10.f;     mBrFilt = 2.f;  mBrFiltOld = 1.f;  pBrFmask = 0;
	mBrushData = new float[BrushMaxSize*BrushMaxSize];
	sBrushTest[0]=0;   updBrush();
	iSnap = 0;  angSnap = crAngSnaps[iSnap];

	///  new
	mWaterRTT = new WaterRTT();
	data = new CData();

	gcom = new CGuiCom(this);
	gcom->mGui = mGui;
	gui = new CGui(this);
	gui->viewBox = new wraps::RenderBoxScene();
	gui->gcom = gcom;
	
	track = new TRACK(std::cout, std::cerr);  //!
	sc = new Scene();
	gui->sc = sc;
}

const Ogre::String App::csBrShape[BRS_ALL] = { "Triangle", "Sinus", "Noise", "Noise2", "N-gon" };  // static


///  material factory setup
//---------------------------------------------------------------------------------------------------------------------------
void App::postInit()
{
	sh::OgrePlatform* platform = new sh::OgrePlatform("General", PATHMANAGER::Data() + "/" + "materials");
	platform->setCacheFolder(PATHMANAGER::ShaderDir());
	
	mFactory = new sh::Factory(platform);
	SetFactoryDefaults();
}


App::~App()
{
	gui->viewBox->destroy();
	delete gui->viewBox;

	BltWorldDestroy();
	
	delete track;  //!
	delete[] pBrFmask;  pBrFmask = 0;

	delete[] mBrushData;
	delete road;

	if (mTerrainPaging)
	{	OGRE_DELETE mTerrainPaging;
		OGRE_DELETE mPageManager;
	}else
		OGRE_DELETE mTerrainGroup;
	OGRE_DELETE mTerrainGlobals;

	delete mFactory;  //!

	delete sc;
	delete mWaterRTT;

	delete gcom;
	delete gui;
	delete data;
}

void App::destroyScene()
{
	mWaterRTT->destroy();

	//NewCommon(false);  //?

	if (road)
	{	road->DestroyRoad();  delete road;  road = 0;  }

	if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }

	DestroyWeather();

	delete[] sc->td.hfHeight;
	delete[] sc->td.hfAngle;
}
	

//  util
//---------------------------------------------------------------------------------------------------------------
ManualObject* App::Create2D(const String& mat, Real s, bool dyn)
{
	ManualObject* m = mSceneMgr->createManualObject();
	m->setDynamic(dyn);
	m->setUseIdentityProjection(true);
	m->setUseIdentityView(true);
	m->setCastShadows(false);
	m->estimateVertexCount(4);
	m->begin(mat, RenderOperation::OT_TRIANGLE_STRIP);
	m->position(-s,-s*asp, 0);  m->textureCoord(0, 1);
	m->position( s,-s*asp, 0);  m->textureCoord(1, 1);
	m->position(-s, s*asp, 0);  m->textureCoord(0, 0);
	m->position( s, s*asp, 0);  m->textureCoord(1, 0);
	m->end();
 
	AxisAlignedBox aabInf;	aabInf.setInfinite();
	m->setBoundingBox(aabInf);  // always visible
	m->setRenderQueueGroup(RQG_Hud2);
	return m;
}
