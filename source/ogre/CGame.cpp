#include "pch.h"
#include "common/Defines.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "SplitScreen.h"
#include "../paged-geom/PagedGeometry.h"
#include "common/RenderConst.h"
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
	:pGame(game), sc(0), mThread(), mTimer(0)
	// ter
	,mTerrainGlobals(0), mTerrainGroup(0), terrain(0), mPaging(false)
	,mTerrainPaging(0), mPageManager(0)
	// game
	,blendMtr(0), iBlendMaps(0), dbgdraw(0), noBlendUpd(0), blendMapSize(513)
	,grass(0), trees(0), road(0)
	,pr(0),pr2(0), sun(0)
	,carIdWin(-1), iRplCarOfs(0)
	// other
	,newGameRpl(0)
	,iEdTire(0), iTireLoad(0), iCurLat(0),iCurLong(0),iCurAlign(0), iUpdTireGr(0)
	,mStaticGeom(0), fLastFrameDT(0.001f)
	,bPerfTest(0),iPerfTestStage(PT_StartWait), isGhost2nd(0)
{
	pSet = settings;
	pGame->collision.pApp = this;

	sc = new Scene();
	frm.resize(4);

	for (int p=0;p<4;++p)
	{
		for (int a=0;a<NumPlayerActions;++a)
			mPlayerInputState[p][a] = 0;
	}

	//  util for update rot
	Quaternion qr;  {
	QUATERNION<double> fix;  fix.Rotate(PI_d, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixCar = qr;  }
	QUATERNION<double> fix;  fix.Rotate(PI_d/2, 0, 1, 0);
	qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();  qFixWh = qr;

	resCar = "";  resTrk = "";  resDrv = "";
	
	///  new
	hud = new CHud(this);
	gui = new CGui(this);
	hud->gui = gui;
	mBindListner = gui;

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
}


void App::postInit()
{
	SetFactoryDefaults();

	mSplitMgr->pApp = this;

	mFactory->setMaterialListener(this);
}

void App::setTranslations()
{
	// loading states
	loadingStates.clear();
	loadingStates.insert(std::make_pair(LS_CLEANUP, String(TR("#{LS_CLEANUP}"))));
	loadingStates.insert(std::make_pair(LS_GAME, String(TR("#{LS_GAME}"))));
	loadingStates.insert(std::make_pair(LS_SCENE, String(TR("#{LS_SCENE}"))));
	loadingStates.insert(std::make_pair(LS_CAR, String(TR("#{LS_CAR}"))));

	loadingStates.insert(std::make_pair(LS_TERRAIN, String(TR("#{LS_TER}"))));
	loadingStates.insert(std::make_pair(LS_ROAD, String(TR("#{LS_ROAD}"))));
	loadingStates.insert(std::make_pair(LS_OBJECTS, String(TR("#{LS_OBJS}"))));
	loadingStates.insert(std::make_pair(LS_TREES, String(TR("#{LS_TREES}"))));

	loadingStates.insert(std::make_pair(LS_MISC, String(TR("#{LS_MISC}"))));
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
	
	gui->mToolTip = 0;  //?

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

	if (pSet->shadow_type <= 1)
	{
		t->setShadowCasterMaterial ("");
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


///  input events
//----------------------------------------------------------------------------------------------------------------------------------
void App::LoadInputDefaults()
{
	mInputActions.clear();
	mInputActions.push_back(InputAction(A_ShowOptions, "ShowOptions", SDLK_TAB, InputAction::Trigger));
	mInputActions.push_back(InputAction(A_PrevTab, "PrevTab", SDLK_F2, InputAction::Trigger));
	mInputActions.push_back(InputAction(A_NextTab, "NextTab", SDLK_F3, InputAction::Trigger));
	mInputActions.push_back(InputAction(A_RestartGame, "RestartGame", SDLK_F5, InputAction::Trigger));
	mInputActions.push_back(InputAction(A_ResetGame, "ResetGame", SDLK_F4, InputAction::Trigger));
	mInputActions.push_back(InputAction(A_Screenshot, "Screenshot", SDLK_F12, InputAction::Trigger));

	LoadInputDefaults(mInputActions, mInputCtrl);

	std::vector<InputAction>* a = mInputActionsPlayer;
	a[0].clear();
	a[0].push_back(InputAction(A_Throttle, "Throttle", SDLK_UP, InputAction::HalfAxis));
	a[0].push_back(InputAction(A_Brake, "Brake", SDLK_DOWN, InputAction::HalfAxis));
	a[0].push_back(InputAction(A_Steering, "Steering", SDLK_LEFT, SDLK_RIGHT, InputAction::Axis));
	a[0].push_back(InputAction(A_HandBrake, "HandBrake", SDLK_SPACE, InputAction::HalfAxis));
	a[0].push_back(InputAction(A_Boost, "Boost", SDLK_LCTRL, InputAction::HalfAxis));
	a[0].push_back(InputAction(A_Flip, "Flip", SDLK_q, SDLK_w, InputAction::Axis));
	a[0].push_back(InputAction(A_ShiftUp, "ShiftUp", SDLK_a, InputAction::Trigger));
	a[0].push_back(InputAction(A_ShiftDown, "ShiftDown", SDLK_z, InputAction::Trigger));
	a[0].push_back(InputAction(A_PrevCamera, "PrevCamera", SDLK_x, InputAction::Trigger));
	a[0].push_back(InputAction(A_NextCamera, "NextCamera", SDLK_c, InputAction::Trigger));
	a[0].push_back(InputAction(A_LastChk, "LastChk", SDLK_0, InputAction::Trigger));
	a[0].push_back(InputAction(A_Rewind, "Rewind", SDLK_BACKSPACE, InputAction::Trigger));

	a[1].clear();
	a[1].push_back(InputAction(A_Throttle, "Throttle", SDLK_u, InputAction::HalfAxis));
	a[1].push_back(InputAction(A_Brake, "Brake", SDLK_m, InputAction::HalfAxis));
	a[1].push_back(InputAction(A_Steering, "Steering", SDLK_h, SDLK_k, InputAction::Axis));
	a[1].push_back(InputAction(A_HandBrake, "HandBrake", SDLK_n, InputAction::HalfAxis));
	a[1].push_back(InputAction(A_Boost, "Boost", SDLK_j, InputAction::HalfAxis));
	a[1].push_back(InputAction(A_Flip, "Flip", SDLK_y, SDLK_i, InputAction::Axis));
	a[1].push_back(InputAction(A_ShiftUp, "ShiftUp", SDLK_UNKNOWN, InputAction::Trigger));
	a[1].push_back(InputAction(A_ShiftDown, "ShiftDown", SDLK_UNKNOWN, InputAction::Trigger));
	a[1].push_back(InputAction(A_PrevCamera, "PrevCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[1].push_back(InputAction(A_NextCamera, "NextCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[1].push_back(InputAction(A_LastChk, "LastChk", SDLK_UNKNOWN, InputAction::Trigger));
	a[1].push_back(InputAction(A_Rewind, "Rewind", SDLK_UNKNOWN, InputAction::Trigger));

	a[2].clear();
	a[2].push_back(InputAction(A_Throttle, "Throttle", SDLK_r, InputAction::HalfAxis));
	a[2].push_back(InputAction(A_Brake, "Brake", SDLK_v, InputAction::HalfAxis));
	a[2].push_back(InputAction(A_Steering, "Steering", SDLK_d, SDLK_g, InputAction::Axis));
	a[2].push_back(InputAction(A_HandBrake, "HandBrake", SDLK_b, InputAction::HalfAxis));
	a[2].push_back(InputAction(A_Boost, "Boost", SDLK_f, InputAction::HalfAxis));
	a[2].push_back(InputAction(A_Flip, "Flip", SDLK_e, SDLK_t, InputAction::Axis));
	a[2].push_back(InputAction(A_ShiftUp, "ShiftUp", SDLK_UNKNOWN, InputAction::Trigger));
	a[2].push_back(InputAction(A_ShiftDown, "ShiftDown", SDLK_UNKNOWN, InputAction::Trigger));
	a[2].push_back(InputAction(A_PrevCamera, "PrevCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[2].push_back(InputAction(A_NextCamera, "NextCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[2].push_back(InputAction(A_LastChk, "LastChk", SDLK_UNKNOWN, InputAction::Trigger));
	a[2].push_back(InputAction(A_Rewind, "Rewind", SDLK_UNKNOWN, InputAction::Trigger));

	a[3].clear();
	a[3].push_back(InputAction(A_Throttle, "Throttle", SDLK_p, InputAction::HalfAxis));
	a[3].push_back(InputAction(A_Brake, "Brake", SDLK_SLASH, InputAction::HalfAxis));
	a[3].push_back(InputAction(A_Steering, "Steering", SDLK_l, SDLK_QUOTE, InputAction::Axis));
	a[3].push_back(InputAction(A_HandBrake, "HandBrake", SDLK_PERIOD, InputAction::HalfAxis));
	a[3].push_back(InputAction(A_Boost, "Boost", SDLK_SEMICOLON, InputAction::HalfAxis));
	a[3].push_back(InputAction(A_Flip, "Flip", SDLK_o, SDLK_LEFTBRACKET, InputAction::Axis));
	a[3].push_back(InputAction(A_ShiftUp, "ShiftUp", SDLK_UNKNOWN, InputAction::Trigger));
	a[3].push_back(InputAction(A_ShiftDown, "ShiftDown", SDLK_UNKNOWN, InputAction::Trigger));
	a[3].push_back(InputAction(A_PrevCamera, "PrevCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[3].push_back(InputAction(A_NextCamera, "NextCamera", SDLK_UNKNOWN, InputAction::Trigger));
	a[3].push_back(InputAction(A_LastChk, "LastChk", SDLK_UNKNOWN, InputAction::Trigger));
	a[3].push_back(InputAction(A_Rewind, "Rewind", SDLK_UNKNOWN, InputAction::Trigger));

	for (int i=0; i<4; ++i)
		LoadInputDefaults(a[i], mInputCtrlPlayer[i]);
}

void App::LoadInputDefaults(std::vector<InputAction> &actions, ICS::InputControlSystem *pICS)
{
	for (std::vector<InputAction>::iterator it = actions.begin(); it != actions.end(); ++it)
	{
		ICS::Control* control;
		bool controlExists = (pICS->getChannel(it->mId)->getControlsCount() != 0);
		if (!controlExists)
		{
			if (it->mType == InputAction::Trigger)
				control = new ICS::Control(boost::lexical_cast<std::string>(it->mId), false, true, 0, ICS::ICS_MAX, ICS::ICS_MAX, false);
			else if (it->mType == InputAction::Axis)
				control = new ICS::Control(boost::lexical_cast<std::string>(it->mId), false, true, 0.5, 0.1, 30.0);
			else if (it->mType == InputAction::HalfAxis)
				control = new ICS::Control(boost::lexical_cast<std::string>(it->mId), false, true, 0.0, 0.1, 30.0);

			pICS->addControl(control);

			if (it->mKeyInc != SDLK_UNKNOWN)
				pICS->addKeyBinding(control, it->mKeyInc, ICS::Control::INCREASE);
			if (it->mKeyDec != SDLK_UNKNOWN)
				pICS->addKeyBinding(control, it->mKeyDec, ICS::Control::DECREASE);

			control->attachChannel(pICS->getChannel(it->mId), ICS::Channel::DIRECT);
			pICS->getChannel(it->mId)->update();
		}
		else
			control = pICS->getChannel(it->mId)->getAttachedControls().front().control;

		it->mICS = pICS;
		it->mControl = control;

		if (pICS == mInputCtrl)
			pICS->getChannel(it->mId)->addListener(this);
	}
}
