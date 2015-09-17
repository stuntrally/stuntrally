#include "pch.h"
#include "common/Def_Str.h"
#include "common/Gui_Def.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "common/GuiCom.h"
#include "common/CScene.h"
#include "common/WaterRTT.h"
#include "common/data/SceneXml.h"
#include "common/data/FluidsXml.h"
#include "FollowCamera.h"
#include "../road/Road.h"
#include "../road/PaceNotes.h"
#include "../vdrift/game.h"
#include "../vdrift/quickprof.h"
#include "../paged-geom/PagedGeometry.h"
#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"
#include "LinearMath/btDefaultMotionState.h"
#include "SplitScreen.h"
#include "../shiny/Main/Factory.hpp"
#include "../sdl4ogre/sdlinputwrapper.hpp"

#include <OgreParticleSystem.h>
#include <OgreManualObject.h>
#include <OgreMaterialManager.h>
#include <OgreSceneNode.h>
#include <OgreViewport.h>
#include <OgreTimer.h>
#include "common/MultiList2.h"
#include "common/Slider.h"
#include <MyGUI.h>
using namespace Ogre;
using namespace MyGUI;


#define isKey(a)  mInputWrapper->isKeyDown(a)


//  simulation (2nd) thread
//---------------------------------------------------------------------------------------------------------------

void App::UpdThr()
{
	Ogre::Timer gtim;
	//#ifdef _WIN32
	//DWORD af = 2;
	//gtim.setOption("QueryAffinityMask", &af);
	//#endif
	gtim.reset();

	while (!mShutDown)
	{
		///  step Game  **

		double dt = double(gtim.getMicroseconds()) * 0.000001;
		gtim.reset();
		
		if (pSet->multi_thr == 1 && !bLoading && !mShutDown)
		{
			bSimulating = true;
			bool ret = pGame->OneLoop(dt);
			if (!ret)
				mShutDown = true;  //ShutDown();

			DoNetworking();
			bSimulating = false;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(pSet->thread_sleep));
	}
}

	
bool App::isTweakTab()
{
	int tt = !gui->tabTweak ? 0 : gui->tabTweak->getIndexSelected();
	return isTweak() && tt != 1 && tt != 2;
}


///  Newtork update  . . . .
void App::DoNetworking()
{
	bool doNetworking = (mClient && mClient->getState() == P2PGameClient::GAME);
	
	//  no pause in networked game
	int tt = !gui->tabTweak ? 0 : gui->tabTweak->getIndexSelected();
	bool gui = isFocGui || isTweakTab();
	pGame->pause = bRplPlay ? (bRplPause || gui) : (gui && !doNetworking);

	//  handle networking stuff
	if (doNetworking)
	{
		PROFILER.beginBlock("-network");

		//  update the local car's state to the client
		protocol::CarStatePackage cs;  // FIXME: Handles only one local car
		for (CarModels::const_iterator it = carModels.begin(); it != carModels.end(); ++it)
		{
			if ((*it)->eType == CarModel::CT_LOCAL)
			{
				cs = (*it)->pCar->GetCarStatePackage();
				cs.trackPercent = uint8_t( (*it)->trackPercent / 100.f * 255.f);  // pack to uint8
				break;
			}
		}
		mClient->setLocalCarState(cs);

		//  check for new car states
		protocol::CarStates states = mClient->getReceivedCarStates();
		for (protocol::CarStates::const_iterator it = states.begin(); it != states.end(); ++it)
		{
			int8_t id = it->first;  // Car number  // FIXME: Various places assume carModels[0] is local
			if (id == 0)  id = mClient->getId();
			
			CarModel* cm = carModels[id];
			if (cm && cm->pCar)
			{
				cm->pCar->UpdateCarState(it->second);
				cm->trackPercent = cm->pCar->trackPercentCopy;  // got from client
			}
		}
		PROFILER.endBlock("-network");
	}
}


//  Frame Start
//---------------------------------------------------------------------------------------------------------------

bool App::frameStart(Real time)
{
	PROFILER.beginBlock(" frameSt");
	fLastFrameDT = time;


	//  input
	for (int i=0; i<4; ++i)
	{
		boost::lock_guard<boost::mutex> lock(input->mPlayerInputStateMutex);
		for (int a = 0; a < NumPlayerActions; ++a)
			input->mPlayerInputState[i][a] = mInputCtrlPlayer[i]->getChannel(a)->getValue();
	}

	if (imgBack && pGame)  // show/hide background image
	{
		bool backImgVis = !bLoading && pGame->cars.empty();
		imgBack->setVisible(backImgVis);
	}


	//  multi thread
	if (pSet->multi_thr == 1 && pGame && !bLoading)
	{
		updatePoses(time);
	}

	///  graphs update  -._/\_-.
	if (pSet->show_graphs && graphs.size() > 0)
	{
		GraphsNewVals();
		UpdateGraphs();
	}

	//...................................................................
	///* tire edit */
	if ((pSet->graphs_type == Gh_TireEdit || pSet->graphs_type == Gh_Tires4Edit) &&
		carModels.size() > 0 && !mWndTweak->getVisible())
	{
		int k = (isKey(SDL_SCANCODE_1) || isKey(SDL_SCANCODE_KP_DIVIDE)  ? -1 : 0)
			  + (isKey(SDL_SCANCODE_2) || isKey(SDL_SCANCODE_KP_MULTIPLY) ? 1 : 0);
		if (k)
		{
			double mul = shift ? 0.2 : (ctrl ? 4.0 : 1.0);
			mul *= 0.005;  // par

			CARDYNAMICS& cd = carModels[0]->pCar->dynamics;
			CARTIRE* tire = cd.GetTire(FRONT_LEFT);
			if (iEdTire == 1)  // longit |
			{
				Dbl& val = tire->longitudinal[iCurLong];  // modify 1st
				val += mul*k * (1 + abs(val));
				for (int i=1; i<4; ++i)
					cd.GetTire(WHEEL_POSITION(i))->longitudinal[iCurLong] = val;  // copy for rest
			}
			else if (iEdTire == 0)  // lateral --
			{
				Dbl& val = tire->lateral[iCurLat];
				val += mul*k * (1 + abs(val));
				for (int i=1; i<4; ++i)
					cd.GetTire(WHEEL_POSITION(i))->lateral[iCurLat] = val;
			}
			else  // align o
			{
				Dbl& val = tire->aligning[iCurAlign];
				val += mul*k * (1 + abs(val));
				for (int i=1; i<4; ++i)
					cd.GetTire(WHEEL_POSITION(i))->aligning[iCurAlign] = val;
			}

			//  update hat, 1st
			tire->CalculateSigmaHatAlphaHat();
			for (int i=1; i<4; ++i)  // copy for rest
			{	cd.GetTire(WHEEL_POSITION(i))->sigma_hat = tire->sigma_hat;
				cd.GetTire(WHEEL_POSITION(i))->alpha_hat = tire->alpha_hat;
			}
			iUpdTireGr = 1;
		}
	}
	//...................................................................


	///  gui
	gui->GuiUpdate();

	
	if (bWindowResized)
	{	bWindowResized = false;

		gcom->ResizeOptWnd();
		gcom->SizeGUI();
		gcom->updTrkListDim();
		gui->updChampListDim();  // resize lists
		gui->slSSS(0);
		gui->listCarChng(gui->carList,0);  // had wrong size
		bRecreateHUD = true;
		
		if (mSplitMgr)  //  reassign car cameras from new viewports
		{	std::list<Camera*>::iterator it = mSplitMgr->mCameras.begin();
			for (int i=0; i < carModels.size(); ++i)
				if (carModels[i]->fCam && it != mSplitMgr->mCameras.end())
				{	carModels[i]->fCam->mCamera = *it;  ++it;  }
		}
		if (!mSplitMgr->mCameras.empty())
		{
			Camera* cam1 = *mSplitMgr->mCameras.begin();
			scn->mWaterRTT->setViewerCamera(cam1);
			if (scn->grass)  scn->grass->setCamera(cam1);
			if (scn->trees)  scn->trees->setCamera(cam1);
		}

		///gui->InitCarPrv();
	}

	//  hud update sizes, after res change
	if (bRecreateHUD)
	{	bRecreateHUD = false;
		
		hud->Destroy();  hud->Create();
	}
	if (bSizeHUD)
	{	bSizeHUD = false;

		hud->Size();
	}


	if (bLoading)
	{
		NewGameDoLoad();
		PROFILER.endBlock(" frameSt");
		return true;
	}
	else 
	{
		///  loading end  ------
		const int iFr = 3;
		if (iLoad1stFrames >= 0)
		{	++iLoad1stFrames;
			if (iLoad1stFrames == iFr)
			{
				LoadingOff();  // hide loading overlay
				mSplitMgr->mGuiViewport->setClearEveryFrame(true, FBT_DEPTH);
				gui->Ch_LoadEnd();
				bLoadingEnd = true;
				iLoad1stFrames = -1;  // for refl
			}
		}else if (iLoad1stFrames >= -1)
		{
			--iLoad1stFrames;  // -2 end

			imgLoad->setVisible(false);  // hide back imgs
			if (imgBack)
				imgBack->setVisible(false);
		}

		
		//  input
		if (isFocGui && pSet->inMenu == MNU_Options && !pSet->isMain &&
			mWndTabsOpts->getIndexSelected() == TABo_Input)
			gui->UpdateInputBars();
		
		
		//  keys up/dn, for lists
		static float dirU = 0.f,dirD = 0.f;
		if (isFocGui && !pSet->isMain && !isTweak())
		{
			if (isKey(SDL_SCANCODE_UP)  ||isKey(SDL_SCANCODE_KP_8))	dirD += time;  else
			if (isKey(SDL_SCANCODE_DOWN)||isKey(SDL_SCANCODE_KP_2))	dirU += time;  else
			{	dirU = 0.f;  dirD = 0.f;  }
			int d = ctrl ? 4 : 1;
			if (dirU > 0.0f) {  gui->LNext( d);  dirU = -0.2f;  }
			if (dirD > 0.0f) {  gui->LNext(-d);  dirD = -0.2f;  }
		}
		
		///  Gui updates from Networking
		gui->UpdGuiNetw();


		//  replay forward,backward keys
		if (bRplPlay)
		{
			isFocRpl = ctrl;
			bool le = isKey(SDL_SCANCODE_LEFTBRACKET), ri = isKey(SDL_SCANCODE_RIGHTBRACKET), ctrlN = ctrl && (le || ri);
			int ta = ((le || gui->bRplBack) ? -2 : 0) + ((ri || gui->bRplFwd) ? 2 : 0);
			if (ta)
			{	double tadd = ta;
				tadd *= (shift ? 0.2 : 1) * (ctrlN ? 4 : 1) * (alt ? 8 : 1);  // multipliers
				if (!bRplPause)  tadd -= 1;  // play compensate
				double t = pGame->timer.GetReplayTime(0), len = replay.GetTimeLength();
				t += tadd * time;  // add
				if (t < 0.0)  t += len;  // cycle
				if (t > len)  t -= len;
				pGame->timer.SetReplayTime(0, t);
			}
		}

		if (!pGame)
		{
			PROFILER.endBlock(" frameSt");
			return false;
		}


		if (pSet->multi_thr == 0)
			DoNetworking();


		//  single thread, sim on draw
		bool ret = true;
		if (pSet->multi_thr == 0)
		{
			ret = pGame->OneLoop(time);
			if (!ret)
				ShutDown();
			updatePoses(time);
		}
		
		// align checkpoint arrow
		// move in front of camera
		if (pSet->check_arrow && hud->arrow.node && !bRplPlay && !carModels.empty())
		{
			FollowCamera* cam = carModels[0]->fCam;
		
			Vector3 pos = cam->mCamera->getPosition();
			Vector3 dir = cam->mCamera->getDirection();  dir.normalise();
			Vector3 up = cam->mCamera->getUp();  up.normalise();
			Vector3 arrowPos = pos + 10.0f * dir + 3.5f*up;
			hud->arrow.node->setPosition(arrowPos);
			
			// animate
			bool bFirstFrame = carModels.front()->bGetStPos;
			if (bFirstFrame) // 1st frame: dont animate
				hud->arrow.qCur = hud->arrow.qEnd;
			else
				hud->arrow.qCur = Quaternion::Slerp(time*5, hud->arrow.qStart, hud->arrow.qEnd, true);
			hud->arrow.nodeRot->setOrientation(hud->arrow.qCur);
			
			// look down -y a bit so we can see the arrow better
			hud->arrow.nodeRot->pitch(Degree(-20), SceneNode::TS_LOCAL); 
		}

		//  cam info text
		if (pSet->show_cam && !carModels.empty() && hud->txCamInfo)
		{	FollowCamera* cam = carModels[0]->fCam;
			if (cam)
			{	bool vis = cam->updInfo(time) && !isFocGui;
				if (vis)
					hud->txCamInfo->setCaption(String(cam->ss));
				hud->txCamInfo->setVisible(vis);
		}	}
		

		//  update all cube maps
		PROFILER.beginBlock("g.refl");
		for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		if (!(*it)->isGhost() && (*it)->pReflect)
			(*it)->pReflect->Update(iLoad1stFrames == -1);
		PROFILER.endBlock("g.refl");


		//  trees
		PROFILER.beginBlock("g.veget");
		if (scn->road) {
			if (scn->grass)  scn->grass->update();
			if (scn->trees)  scn->trees->update();  }
		PROFILER.endBlock("g.veget");


		//  road upd lods
		if (scn->road)
		{
			//PROFILER.beginBlock("g.road");  // below 0.0 ms

			//  more than 1: in pre viewport, each frame
			if (mSplitMgr->mNumViewports == 1)
			{
				roadUpdTm += time;
				if (roadUpdTm > 0.1f)  // interval [sec]
				{
					roadUpdTm = 0.f;
					scn->road->UpdLodVis(pSet->road_dist);
				}
			}
			//PROFILER.endBlock("g.road");
		}

		//[]()  pace upd vis  ~ ~ ~
		if (scn->pace)
		{	
			const CarModel* cm = *carModels.begin();
			Vector3 p = cm->pMainNode->getPosition();
			float vel = cm->pCar->GetSpeedometer();
			scn->pace->carVel = vel;
			scn->pace->rewind = cm->pCar->bRewind;
			scn->pace->UpdVis(p);
		}
		

		//**  bullet bebug draw
		if (dbgdraw)  {							// DBG_DrawWireframe
			dbgdraw->setDebugMode(pSet->bltDebug ? 1 /*+(1<<13) 255*/ : 0);
			dbgdraw->step();  }


		///  terrain mtr from blend maps
		// now in CarModel::Update
		//UpdWhTerMtr(pCar);
		
		// stop rain/snow when paused
		if (scn->pr && scn->pr2 && pGame)
		{
			if (pGame->pause)
				{	 scn->pr->setSpeedFactor(0.f);  scn->pr2->setSpeedFactor(0.f);  }
			else{	 scn->pr->setSpeedFactor(1.f);  scn->pr2->setSpeedFactor(1.f);  }
		}

		
		//  update shader time
		mTimer += time;
		mFactory->setSharedParameter("windTimer",  sh::makeProperty <sh::FloatValue>(new sh::FloatValue(mTimer)));
		mFactory->setSharedParameter("waterTimer", sh::makeProperty <sh::FloatValue>(new sh::FloatValue(mTimer)));


		///()  grass sphere pos
		bool hasCars = !carModels.empty();
		if (hasCars)
		{
			Real r = 1.7;  r *= r;  //par
			const Vector3* p = &carModels[0]->posSph[0];
			mFactory->setSharedParameter("posSph0", sh::makeProperty <sh::Vector4>(new sh::Vector4(p->x,p->y,p->z,r)));
			p = &carModels[0]->posSph[1];
			mFactory->setSharedParameter("posSph1", sh::makeProperty <sh::Vector4>(new sh::Vector4(p->x,p->y,p->z,r)));
		}else
		{	mFactory->setSharedParameter("posSph0", sh::makeProperty <sh::Vector4>(new sh::Vector4(0,0,500,-1)));
			mFactory->setSharedParameter("posSph1", sh::makeProperty <sh::Vector4>(new sh::Vector4(0,0,500,-1)));
		}
		
		///~~  fluid fog, send params to shaders
		if (hasCars  && pSet->game.local_players == 1)
		{
			int fi = carModels[0]->iCamFluid;
			float p = carModels[0]->fCamFl;
			//? if (fi != idFlOld)  {
			if (fi >= 0)
			{	const FluidBox* fb = &scn->sc->fluids[fi];
				const FluidParams& fp = scn->sc->pFluidsXml->fls[fb->id];

				mFactory->setSharedParameter("fogFluidH", sh::makeProperty <sh::Vector4>(new sh::Vector4(
					fb->pos.y +p /*+0.5f par? ofsH..*/, 1.f / fp.fog.dens, fp.fog.densH +p*0.5f, 0)));

				mFactory->setSharedParameter("fogFluidClr", sh::makeProperty <sh::Vector4>(new sh::Vector4(
					fp.fog.r, fp.fog.g, fp.fog.b, fp.fog.a)));
			}else
				mFactory->setSharedParameter("fogFluidH", sh::makeProperty <sh::Vector4>(new sh::Vector4(
					-900.f, 1.f/17.f, 0.15f, 0)));

		}// no else, set in setFog default


		//  Signal loading finished to the peers
		if (mClient && bLoadingEnd)
		{
			bLoadingEnd = false;
			mClient->loadingFinished();
		}
		
		PROFILER.endBlock(" frameSt");

		return ret;
	}
	PROFILER.endBlock(" frameSt");
}

bool App::frameEnd(Real time)
{
	//  sleep when in Gui
	if (isFocGui && pSet->gui_sleep >= 0)  // && gui && gui->bGI)
		//!pSet->isMain && pSet->inMenu == MNU_Single && mWndTabsGame->getIndexSelected() == TAB_Multi)
		boost::this_thread::sleep(boost::posix_time::milliseconds(pSet->gui_sleep));

	return true;
}
