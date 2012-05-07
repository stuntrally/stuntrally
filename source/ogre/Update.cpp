#include "pch.h"
#include "common/Defines.h"
#include "OgreGame.h"
#include "FollowCamera.h"
#include "../road/Road.h"
#include "../vdrift/game.h"
#include "../vdrift/quickprof.h"
#include "../paged-geom/PagedGeometry.h"
#include "../ogre/common/MaterialGen/MaterialFactory.h"
#include "../oisb/OISBSystem.h"
#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"
#include "LinearMath/btDefaultMotionState.h"

#include <OgreParticleSystem.h>
#include <OgreManualObject.h>
#include <OgreMaterialManager.h>
#include "common/Gui_Def.h"
#include "common/MultiList2.h"
#include "common/Slider.h"
#include "SplitScreen.h"
#include <MyGUI.h>
using namespace Ogre;
using namespace MyGUI;


//  simulation (2nd) thread
//---------------------------------------------------------------------------------------------------------------

void App::UpdThr()
{
	while (!mShutDown)
	{
		///  step Game  **

		//  separate thread
		pGame->qtim.update();
		double dt = pGame->qtim.dt;

		///if (mOISBsys)  // input update  multi thread
		///	mOISBsys->process(dt);
		
		if (pSet->multi_thr == 1 && !bLoading)
		{
			bool ret = pGame->OneLoop(dt);
			if (!ret)
				mShutDown = true;

			DoNetworking();
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(pSet->thread_sleep));
	}
}

		
void App::DoNetworking()
{
	bool doNetworking = (mClient && mClient->getState() == P2PGameClient::GAME);
	// Note that there is no pause when in networked game
	pGame->pause = bRplPlay ? (bRplPause || isFocGui) : (isFocGui && !doNetworking);

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
	if (pSet->graphs_type == 4 && carModels.size() > 0)
	{
		int k = (isKey(OIS::KC_1) || isKey(OIS::KC_DIVIDE)  ? -1 : 0)
			  + (isKey(OIS::KC_2) || isKey(OIS::KC_MULTIPLY) ? 1 : 0);
		if (k)
		{
			double mul = shift ? 0.2 : (ctrl ? 4.0 : 1.0);
			mul *= 0.005;  // par
			typedef CARDYNAMICS::T T;

			CARDYNAMICS& cd = carModels[0]->pCar->dynamics;
			if (iEdTire == 1)  // longit |
			{
				T& val = cd.tire[0].longitudinal_parameters[iCurLong];  // modify 1st
				val += mul*k * (1 + abs(val));
				for (int i=1; i<4; ++i)
					cd.tire[i].longitudinal_parameters[iCurLong] = val;  // copy for rest
			}
			else if (iEdTire == 0)  // lateral --
			{
				T& val = cd.tire[0].transverse_parameters[iCurLat];
				val += mul*k * (1 + abs(val));
				for (int i=1; i<4; ++i)
					cd.tire[i].transverse_parameters[iCurLat] = val;
			}
			else  // align o
			{
				T& val = cd.tire[0].aligning_parameters[iCurAlign];
				val += mul*k * (1 + abs(val));
				for (int i=1; i<4; ++i)
					cd.tire[i].aligning_parameters[iCurAlign] = val;
			}

			//  update hat, 1st
			cd.tire[0].CalculateSigmaHatAlphaHat();
			for (int i=1; i<4; ++i)  // copy for rest
			{	cd.tire[i].sigma_hat = cd.tire[0].sigma_hat;
				cd.tire[i].alpha_hat = cd.tire[0].alpha_hat;
			}
			iUpdTireGr = 1;
		}
	}
	//...................................................................


	if (bGuiReinit)  // after language change from combo
	{	bGuiReinit = false;

		mGUI->destroyWidgets(vwGui);  bnQuit=0;mWndOpts=0;  //todo: rest too..
		InitGui();
		bWindowResized = true;
		mWndTabsOpts->setIndexSelected(3);  // switch back to view tab
	}

	if (bWindowResized)
	{	bWindowResized = false;
		ResizeOptWnd();
		SizeGUI();
		updTrkListDim();  updChampListDim();  // resize lists
		bSizeHUD = true;
	}
		
	///  sort trk list
	if (trkMList && trkMList->mSortColumnIndex != trkMList->mSortColumnIndexOld
		|| trkMList->mSortUp != trkMList->mSortUpOld)
	{
		trkMList->mSortColumnIndexOld = trkMList->mSortColumnIndex;
		trkMList->mSortUpOld = trkMList->mSortUp;

		pSet->tracks_sort = trkMList->mSortColumnIndex;  // to set
		pSet->tracks_sortup = trkMList->mSortUp;
		TrackListUpd(false);
	}

	if (bLoading)
	{
		NewGameDoLoad();
		PROFILER.endBlock(" frameSt");
		return true;
	}
	else 
	{
		bool bFirstFrame = (carModels.size()>0 && carModels.front()->bGetStPos) ? true : false;
		
		if (isFocGui && mWndTabsOpts->getIndexSelected() == 4 && pSet->inMenu == WND_Options && !pSet->isMain)
			UpdateInputBars();
		
		//  keys up/dn, for lists
		#define isKey(a)  mKeyboard->isKeyDown(OIS::a)
		static float dirU = 0.f,dirD = 0.f;
		if (isFocGui && !pSet->isMain)
		{
			if (isKey(KC_UP)  ||isKey(KC_NUMPAD8))	dirD += time;  else
			if (isKey(KC_DOWN)||isKey(KC_NUMPAD2))	dirU += time;  else
			{	dirU = 0.f;  dirD = 0.f;  }
			int d = ctrl ? 4 : 1;
			if (dirU > 0.0f) {  LNext( d);  dirU = -0.12f;  }
			if (dirD > 0.0f) {  LNext(-d);  dirD = -0.12f;  }
		}
		
		///  Gui updates from networking
		//  We do them here so that they are handled in the main thread as MyGUI is not thread-safe
		if (isFocGui)
		{
			if (mMasterClient) {
				std::string error = mMasterClient->getError();
				if (!error.empty())
					Message::createMessageBox("Message", TR("#{Error}"), error,
						MessageBoxStyle::IconError | MessageBoxStyle::Ok);
			}
			boost::mutex::scoped_lock lock(netGuiMutex);
			if (bRebuildGameList) {  rebuildGameList();  bRebuildGameList = false;  }
			if (bRebuildPlayerList) {  rebuildPlayerList();  bRebuildPlayerList = false;  }
			if (bUpdateGameInfo) {  updateGameInfo();  bUpdateGameInfo = false;  }
			if (bUpdChat)  {  edNetChat->setCaption(sChatBuffer);  bUpdChat = false;  }
			if (bStartGame)
			{
				mMasterClient.reset();
				mClient->startGame();
				btnNewGameStart(NULL);
				bStartGame = false;
			}
		}

		//  replay forward,backward keys
		if (bRplPlay)
		{
			isFocRpl = ctrl;
			bool le = isKey(KC_LBRACKET), ri = isKey(KC_RBRACKET), ctrlN = ctrl && (le || ri);
			int ta = ((le || bRplBack) ? -2 : 0) + ((ri || bRplFwd) ? 2 : 0);
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

		// input
		//PROFILER.beginBlock("input");  // below 0.0 ms
		//if (pSet->multi_thr == 0)  /// move to sim thread...
		if (mOISBsys)  // input update (old ver, in render)
			mOISBsys->process(time);
		//PROFILER.endBlock("input");


		if (pSet->multi_thr == 0)
			DoNetworking();


		//  single thread, sim on draw
		bool ret = true;
		if (pSet->multi_thr == 0)
		{
			ret = pGame->OneLoop(time);
			if (!ret)  mShutDown = true;
			updatePoses(time);
		}
		
		// align checkpoint arrow
		// move in front of camera
		if (pSet->check_arrow && arrowNode && !bRplPlay)
		{
			Vector3 camPos = carModels.front()->fCam->mCamera->getPosition();
			Vector3 dir = carModels.front()->fCam->mCamera->getDirection();
			dir.normalise();
			Vector3 up = carModels.front()->fCam->mCamera->getUp();
			up.normalise();
			Vector3 arrowPos = camPos + 10.0f * dir + 3.5f*up;
			arrowNode->setPosition(arrowPos);
			
			// animate
			if (bFirstFrame) // 1st frame: dont animate
				arrowAnimCur = arrowAnimEnd;
			else
				arrowAnimCur = Quaternion::Slerp(time*5, arrowAnimStart, arrowAnimEnd, true);
			arrowRotNode->setOrientation(arrowAnimCur);
			
			// look down -y a bit so we can see the arrow better
			arrowRotNode->pitch(Degree(-20), SceneNode::TS_LOCAL); 
		}

		for (std::vector<CarModel*>::iterator it=carModels.begin();
			it!=carModels.end(); ++it)
		{
			if ( (*it)->fCam)
				(*it)->fCam->updInfo(time);
		}

		//  update all cube maps
		PROFILER.beginBlock("g.refl");
		for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		if ((*it)->eType != CarModel::CT_GHOST && (*it)->pReflect)
			(*it)->pReflect->Update();
		PROFILER.endBlock("g.refl");

		//  trees
		PROFILER.beginBlock("g.veget");
		if (road) {
			if (grass)  grass->update();
			if (trees)  trees->update();  }
		PROFILER.endBlock("g.veget");

		//  road upd lods
		if (road)
		{
			//PROFILER.beginBlock("g.road");  // below 0.0 ms

			//  more than 1: in pre viewport, each frame
			if (mSplitMgr->mNumViewports == 1)
			{
				roadUpdTm += time;
				if (roadUpdTm > 0.1f)  // interval [sec]
				{
					roadUpdTm = 0.f;
					road->UpdLodVis(pSet->road_dist);
				}
			}
			//PROFILER.endBlock("g.road");
		}

		//**  bullet bebug draw
		if (dbgdraw)  {							// DBG_DrawWireframe
			dbgdraw->setDebugMode(pSet->bltDebug ? 1 /*+(1<<13) 255*/ : 0);
			dbgdraw->step();  }


		///  terrain mtr from blend maps
		// now in CarModel::Update
		//UpdWhTerMtr(pCar);
		
		// stop rain/snow when paused
		if (pr && pr2 && pGame)
		{
			if (pGame->pause)
				{	 pr->setSpeedFactor(0.f);	 pr2->setSpeedFactor(0.f);	}
			else{	 pr->setSpeedFactor(1.f);	 pr2->setSpeedFactor(1.f);	}
		}
		
		materialFactory->update();
		
		// We put this here, because first render frame is rather heavy
		if (mClient && bLoadingEnd)
		{
			bLoadingEnd = false;
			mClient->loadingFinished();  // Signal loading finished to the peers
		}
		
		PROFILER.endBlock(" frameSt");
		return ret;
	}
	PROFILER.endBlock(" frameSt");
}
bool App::frameEnd(Real time)
{
	return true;
}
