#include "pch.h"
#include "common/Def_Str.h"
#include "common/RenderConst.h"
#include "common/GuiCom.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "common/data/CData.h"
#include "common/data/SceneXml.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "SplitScreen.h"
#include "../paged-geom/PagedGeometry.h"
#include "common/WaterRTT.h"
#include "common/MultiList2.h"
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreTerrainPaging.h>
#include <OgrePageManager.h>
#include <OgreManualObject.h>
#include "../shiny/Platforms/Ogre/OgreMaterial.hpp"
using namespace Ogre;


//  ctors  -----------------------------------------------
App::App(SETTINGS *settings, GAME *game)
	:pGame(game), pSet(settings),
	,sc(0), data(0), hud(0), gui(0), gcom(0), input(0), mBindListner(0)
	,mThread(), mTimer(0.f)
	// ter
	,mTerrainGlobals(0), mTerrainGroup(0), terrain(0), mPaging(false)
	,mTerrainPaging(0), mPageManager(0)
	// game
	,blendMtr(0), iBlendMaps(0), dbgdraw(0), noBlendUpd(0), blendMapSize(513)
	,grass(0), trees(0), road(0)
	,pr(0),pr2(0), sun(0), mStaticGeom(0)
	,carIdWin(-1), iRplCarOfs(0)
	// other
	,newGameRpl(0), curLoadState(0)
	,bRplPlay(0),bRplPause(0), bRplRec(0), bRplWnd(0)
	,iEdTire(0), iTireLoad(0), iCurLat(0),iCurLong(0),iCurAlign(0), iUpdTireGr(0)
	,mStaticGeom(0), fLastFrameDT(0.001f)
	,bPerfTest(0),iPerfTestStage(PT_StartWait), isGhost2nd(0)
{
	pGame->collision.pApp = this;

	sc = new Scene();
	mWaterRTT = new WaterRTT();
	frm.resize(4);
	
	for (int i=0; i < 8; ++i)
		iCurPoses = 0;

	//  util for update rot
	Quaternion qr;  {
	QUATERNION<double> fix;  fix.Rotate(PI_d, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixCar = qr;  }
	QUATERNION<double> fix;  fix.Rotate(PI_d/2, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixWh = qr;

	resCar = "";  resTrk = "";  resDrv = "";
	
	///  new
	data = new CData();
	hud = new CHud(this);
	gcom = new CGuiCom(this);
	gui = new CGui(this);
	gui->gcom = gcom;
	hud->gui = gui;
	mBindListner = gui;
	input = new CInput(this);

	if (pSet->multi_thr)
		mThread = boost::thread(boost::bind(&App::UpdThr, boost::ref(*this)));
}


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
	delete sc;
	delete data;
	delete gcom;
	delete gui;
	delete hud;
	delete input;
	//delete mWaterRTT;  // todo: crash at exit..
}


void App::postInit()
{
	SetFactoryDefaults();

	mSplitMgr->pApp = this;

	mFactory->setMaterialListener(this);
}


void App::destroyScene()
{
	DestroyObjects(true);
	
	for (int i=0; i < graphs.size(); ++i)
		delete graphs[i];

	for (int i=0; i<4; ++i)
		pSet->cam_view[i] = carsCamNum[i];

	// Delete all cars
	for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); ++it)
		delete (*it);

	carModels.clear();
	//carPoses.clear();
	
	gcom->mToolTip = 0;  //?

	if (road)
	{	road->DestroyRoad();  delete road;  road = 0;  }

	if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }

	if (pGame)
		pGame->End();
	delete[] sc->td.hfHeight;
	delete[] sc->td.hfAngle;
	delete[] blendMtr;  blendMtr = 0;

	BaseApp::destroyScene();
}


void App::materialCreated(sh::MaterialInstance* m, const std::string& configuration, unsigned short lodIndex)
{
	Ogre::Technique* t = static_cast<sh::OgreMaterial*>(m->getMaterial())->getOgreTechniqueForConfiguration (configuration, lodIndex);

	if (pSet->shadow_type == Sh_None)
	{
		t->setShadowCasterMaterial("");
		return;
	}

	// this is just here to set the correct shadow caster
	if (m->hasProperty ("transparent") && m->hasProperty ("cull_hardware") && sh::retrieveValue<sh::StringValue>(m->getProperty ("cull_hardware"), 0).get() == "none")
	{
		// Crash !?
		///assert(!MaterialManager::getSingleton().getByName("PSSM/shadow_caster_nocull").isNull ());
		//t->setShadowCasterMaterial("PSSM/shadow_caster_nocull");
	}

	if (!m->hasProperty ("transparent") || !sh::retrieveValue<sh::BooleanValue>(m->getProperty ("transparent"), 0).get())
	{
		t->setShadowCasterMaterial("PSSM/shadow_caster_noalpha");
	}
}
