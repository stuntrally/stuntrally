#include "pch.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "common/Def_Str.h"
#include "common/RenderConst.h"
#include "common/GuiCom.h"
#include "common/data/CData.h"
#include "common/data/SceneXml.h"
#include "common/CScene.h"
#include "../vdrift/cardefs.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "SplitScreen.h"
#include "../paged-geom/PagedGeometry.h"
#include "common/WaterRTT.h"
#include "common/MultiList2.h"
#include "common/Gui_Popup.h"
#include "common/Axes.h"
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreTerrainPaging.h>
#include <OgrePageManager.h>
#include <OgreManualObject.h>
#include <OgreTechnique.h>
#include "../shiny/Platforms/Ogre/OgreMaterial.hpp"
#include "../ogre/common/RenderBoxScene.h"
using namespace Ogre;


//  ctors  -----------------------------------------------
App::App(SETTINGS *settings, GAME *game)
	:pGame(game)
	,hud(0), gui(0), gcom(0), input(0)
	,mThread(), mTimer(0.f)
	// game
	,blendMtr(0), blendMapSize(513), dbgdraw(0)
	,carIdWin(-1), iRplCarOfs(0)
	// other
	,newGameRpl(0), curLoadState(0), dstTrk(1)
	,bHideHudArr(0), bHideHudBeam(0), bHideHudPace(0), bHideHudTrail(0)
	,bRplPlay(0),bRplPause(0), bRplRec(0), bRplWnd(1), iRplSkip(0)
	,iEdTire(0), iTireLoad(0), iCurLat(0),iCurLong(0),iCurAlign(0), iUpdTireGr(0)
	,fLastFrameDT(0.001f)
	,bPerfTest(0),iPerfTestStage(PT_StartWait)
	,isGhost2nd(0),fLastTime(1.f)
{
	pSet = settings;
	pGame->collision.pApp = this;

	frm.resize(MAX_CARS);
	for (int i=0; i < MAX_CARS; ++i)
		iCurPoses[i] = 0;

	Axes::Init();

	resCar = "";  resTrk = "";  resDrv = "";
	oldTrack = "";  oldTrkUser = false;
	
	///  new
	scn = new CScene(this);
	data = scn->data;
	hud = new CHud(this);

	gcom = new CGuiCom(this);
	gui = new CGui(this);
	gui->gcom = gcom;
	hud->gui = gui;
	gui->popup = new GuiPopup();
	gui->viewBox = new wraps::RenderBoxScene();

	mBindListner = gui;
	input = new CInput(this);

	if (pSet->multi_thr)
		mThread = boost::thread(boost::bind(&App::UpdThr, boost::ref(*this)));
}

void App::ShutDown()
{
	mShutDown = true;
	if (mThread.joinable())
		mThread.join();
}

App::~App()
{
	ShutDown();

	gui->viewBox->destroy();
	delete gui->viewBox;

	delete gui->popup;
	delete gcom;
	delete gui;
	delete scn;

	delete hud;
	delete input;
}


void App::postInit()
{
	SetFactoryDefaults();

	mSplitMgr->pApp = this;

	mFactory->setMaterialListener(this);
}


void App::destroyScene()
{
	delete dbgdraw;  dbgdraw = 0;
	
	scn->mWaterRTT->destroy();
	
	DestroyObjects(true);
	
	for (int i=0; i < graphs.size(); ++i)
		delete graphs[i];

	for (int i=0; i<4; ++i)
		pSet->cam_view[i] = carsCamNum[i];

	// Delete all cars
	for (auto& car : carModels)
		delete car;

	carModels.clear();
	//carPoses.clear();
	
	gcom->mToolTip = 0;  //?

	scn->DestroyRoads();

	scn->DestroyTrees();

	if (pGame)
		pGame->End();
	
	delete[] scn->sc->td.hfHeight;
	delete[] blendMtr;  blendMtr = 0;

	BaseApp::destroyScene();
}


void App::materialCreated(sh::MaterialInstance* m, const std::string& configuration, unsigned short lodIndex)
{
	Technique* t = static_cast<sh::OgreMaterial*>(m->getMaterial())->getOgreTechniqueForConfiguration (configuration, lodIndex);

	if (pSet->shadow_type == Sh_None)
	{
		t->setShadowCasterMaterial("");
		return;
	}

	/*if (m->hasProperty("transparent") && m->hasProperty("cull_hardware") &&
		sh::retrieveValue<sh::StringValue>(m->getProperty("cull_hardware"), 0).get() == "none")
	{
		// Crash !?
		assert(!MaterialManager::getSingleton().getByName("PSSM/shadow_caster_nocull").isNull ());
		t->setShadowCasterMaterial("shadowcaster_nocull");
	}*/

	if (m->hasProperty("instancing") && sh::retrieveValue<sh::StringValue>(m->getProperty("instancing"), 0).get() == "true")
	{
		t->setShadowCasterMaterial("shadowcaster_instancing");
	}

	if (!m->hasProperty("transparent") || !sh::retrieveValue<sh::BooleanValue>(m->getProperty("transparent"), 0).get())
	{
		t->setShadowCasterMaterial("shadowcaster_noalpha");
	}
}
