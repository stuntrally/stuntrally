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

#include <OgreParticleSystem.h>
#include <OgreManualObject.h>
#include <OgreMaterialManager.h>
#include "common/Gui_Def.h"
#include "common/MultiList2.h"
#include "SplitScreen.h"
#include <MyGUI.h>
using namespace Ogre;
using namespace MyGUI;


//---------------------------------------------------------------------------------------------------------------
//  Frame Start
//---------------------------------------------------------------------------------------------------------------

void App::UpdThr()
{
	while (!mShutDown)
	{
		///  step Game  **
		//  separate thread
		if (pSet->multi_thr == 1 && !bLoading)
		{
			bool ret = pGame->OneLoop();

			//if (!pGame->pause && mFCam)
			//	mFCam->update(pGame->framerate/**/);
			//if (ndSky)  ///o-
			//	ndSky->setPosition(GetCamera()->getPosition());
			
			DoNetworking();

			if (!ret)
				mShutDown = true;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(5));  //par!?
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

//---------------------------------------------------------------------------------------------------------------

bool App::frameStart(Real time)
{
	PROFILER.beginBlock(" frameSt");
	//---------
	//static QTimer gtim;
	//gtim.update();
	//double dt = gtim.dt;

	//  multi thread
	if (pSet->multi_thr == 1 && pGame && !bLoading)
	{
		updatePoses(time);

		#if 0
		// Update cameras for all cars
		for (int i=0; i < carModels.size(); ++i)
		{
			CarModel* cm = carModels[i];
			//if (cm->fCam)
			//	cm->fCam->update(/*pGame->framerate*//*dt*/
			//		time, &newPosInfos[i]);
		}
		#endif

		#if 0
		//  camera jump test graph in log
		if (carModels.size()>0 && carModels[0]->pMainNode)
		{
			static Vector3 old(0,0,0);
			//Vector3 pos = carModels[0]->pMainNode->getPosition();
			//Vector3 pos = carModels[0]->mCamera->getPosition();
			//Vector3 pos = newPosInfos[0].pos;
			Vector3 pos = (carModels[0]->pMainNode->getPosition() - carModels[0]->mCamera->getPosition())*20;
			bool b = pos.x < -2.f;//pos.x < old.x;
			String s=" ";
			for (int i=0; i < (pos.x+2)*10; ++i)  s += "   ";
			s += "|";
			LogO("x= "+fToStr(pos.x,3,6)+/*"  z= "+fToStr(pos.z,3,6)+*/s+(b?" ################":""));
			old = pos;
		}
		#endif
	}
	
	#if 0
	///  graph test
	static Real ti = 0.f;  ti += time;
	if (mSplitMgr && !bLoading)
	{
		static int t=0;  ++t;
		static std::vector<Vector2> vpos,vpos2;
		if (t==1)
			for (int i=0; i < 1000; ++i)
			{
				vpos.push_back(Vector2(i+150,150+200));
				vpos2.push_back(Vector2(i+150,150+200));
			}

		for (int i=0; i < 1000-1; ++i)
			vpos[i].y = vpos[i+1].y;

		int i = 1000-1;
		if (carModels.size() > 0)
		vpos[i].y = //sin(i*0.1+t*0.1)*cos(i*0.11+t*0.11)*150+200;
			//350 - time * 10000.f;
			//350.f - newPosInfos[0].miniPos.x * 100.f;
			//350.f - newPosInfos[0].pos.x * 10.f;
			350.f - carModels[0]->pCar->dynamics.GetSuspension((WHEEL_POSITION )0).GetVelocity() * 100.f;

		//for (int i=90; i < 1000; ++i)
		//	vpos[i] = Vector2(i+150,
		//		sin(i*0.1+t*0.1)*cos(i*0.11+t*0.11)*150+200);

		for (int i=0; i < 1000; ++i)
			vpos2[i].y = vpos[i].y+1;

		mSplitMgr->mHUD->drawLines(false, vpos, ColourValue(0.2,0.1,0));
		mSplitMgr->mHUD->drawLines(false, vpos2, ColourValue(1.0,0.6,0.3));
	}
	#endif


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
		OISB::System::getSingleton().process(time);
		//PROFILER.endBlock("input");


		if (pSet->multi_thr == 0)
			DoNetworking();


		//  single thread, sim on draw
		bool ret = true;
		if (pSet->multi_thr == 0)
		{
			ret = pGame->OneLoop();
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

		//  update all cube maps
		PROFILER.beginBlock("g.refl");
		for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		if ((*it)->eType != CarModel::CT_GHOST && (*it)->pReflect)
			(*it)->pReflect->Update();
		PROFILER.endBlock("g.refl");

		//  trees
		PROFILER.beginBlock("g.veget");
		//if (pSet->mult_thr != 2)
		if (road) {
			if (grass)  grass->update();
			if (trees)  trees->update();  }
		PROFILER.endBlock("g.veget");

		//  road upd lods
		if (road)
		{
			//PROFILER.beginBlock("g.road");  // below 0.0 ms
			road->RebuildRoadInt();

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


//  newPoses - Get new car pos from game
//---------------------------------------------------------------------------------------------------------------
void App::newPoses(float time)  // time only for camera update
{
	if (!pGame || bLoading || pGame->cars.size() == 0 /*|| carPoses.empty() || iCurPoses.empty()*/)
		return;
	PROFILER.beginBlock(".newPos ");

	double rplTime = pGame->timer.GetReplayTime(0);
	double lapTime = pGame->timer.GetPlayerTime(0);

	// Iterate through all car models and set new pos info (from vdrift sim or replay)
	CarModel* carM0 = carModels[0];
	for (int c = 0; c < carModels.size(); ++c)
	{
		CarModel* carM = carModels[c];
		CAR* pCar = carM->pCar;
		PosInfo pi;  // new, to fill data
		bool bGhost = carM->eType == CarModel::CT_GHOST;
		
		//  local data  car,wheels
		MATHVECTOR <float,3> pos, whPos[4];
		QUATERNION <float> rot, whRot[4];


		///-----------------------------------------------------------------------
		//  play  get data from replay / ghost
		///-----------------------------------------------------------------------
		if (bGhost)
		{
			ReplayFrame rf;
			bool ok = ghplay.GetFrame(lapTime, &rf, 0);
			//  car
			pos = rf.pos;  rot = rf.rot;  pi.speed = rf.speed;
			pi.fboost = rf.fboost;  pi.steer = rf.steer;
			pi.percent = rf.percent;  pi.braking = rf.braking;
			pi.fHitTime = rf.fHitTime;	pi.fParIntens = rf.fParIntens;	pi.fParVel = rf.fParVel;
			pi.vHitPos = rf.vHitPos;	pi.vHitNorm = rf.vHitNorm;
			//  wheels
			for (int w=0; w < 4; ++w)
			{
				whPos[w] = rf.whPos[w];  whRot[w] = rf.whRot[w];
				pi.whVel[w] = rf.whVel[w];
				pi.whSlide[w] = rf.slide[w];  pi.whSqueal[w] = rf.squeal[w];
				pi.whR[w] = replay.header.whR[c][w];//
				pi.whTerMtr[w] = rf.whTerMtr[w];  pi.whRoadMtr[w] = rf.whRoadMtr[w];
				pi.whH[w] = rf.whH[w];  pi.whP[w] = rf.whP[w];
				pi.whAngVel[w] = rf.whAngVel[w];
				if (w < 2)  pi.whSteerAng[w] = rf.whSteerAng[w];
			}
		}
		else if (bRplPlay)  // class member frm - used for sounds in car.cpp
		{
			//  time  from start
			ReplayFrame& fr = frm[c];
			bool ok = replay.GetFrame(rplTime, &fr, c);
				if (!ok)	pGame->timer.RestartReplay(0);  //at end
			
			//  car
			pos = fr.pos;  rot = fr.rot;  pi.speed = fr.speed;
			pi.fboost = fr.fboost;  pi.steer = fr.steer;
			pi.percent = fr.percent;  pi.braking = fr.braking;
			pi.fHitTime = fr.fHitTime;	pi.fParIntens = fr.fParIntens;	pi.fParVel = fr.fParVel;
			pi.vHitPos = fr.vHitPos;	pi.vHitNorm = fr.vHitNorm;
			//  wheels
			for (int w=0; w < 4; ++w)
			{
				whPos[w] = fr.whPos[w];  whRot[w] = fr.whRot[w];
				pi.whVel[w] = fr.whVel[w];
				pi.whSlide[w] = fr.slide[w];  pi.whSqueal[w] = fr.squeal[w];
				pi.whR[w] = replay.header.whR[c][w];//
				pi.whTerMtr[w] = fr.whTerMtr[w];  pi.whRoadMtr[w] = fr.whRoadMtr[w];
				pi.whH[w] = fr.whH[w];  pi.whP[w] = fr.whP[w];
				pi.whAngVel[w] = fr.whAngVel[w];
				if (w < 2)  pi.whSteerAng[w] = fr.whSteerAng[w];
			}
		}
		else
		//  get data from vdrift
		//-----------------------------------------------------------------------
		if (pCar)
		{
			const CARDYNAMICS& cd = pCar->dynamics;
			pos = cd.GetPosition();  rot = cd.GetOrientation();
			//  car
			pi.fboost = cd.boostVal;	//posInfo.steer = cd.steer;
			pi.speed = pCar->GetSpeed();
			pi.percent = carM->trackPercent;	pi.braking = cd.IsBraking();
			pi.fHitTime = cd.fHitTime;	pi.fParIntens = cd.fParIntens;	pi.fParVel = cd.fParVel;
			pi.vHitPos = cd.vHitPos;	pi.vHitNorm = cd.vHitNorm;
			//  wheels
			for (int w=0; w < 4; ++w)
			{	WHEEL_POSITION wp = WHEEL_POSITION(w);
				whPos[w] = cd.GetWheelPosition(wp);  whRot[w] = cd.GetWheelOrientation(wp);
				//float wR = pCar->GetTireRadius(wp);
				pi.whVel[w] = cd.GetWheelVelocity(wp).Magnitude();
				pi.whSlide[w] = -1.f;  pi.whSqueal[w] = pCar->GetTireSquealAmount(wp, &pi.whSlide[w]);
				pi.whR[w] = pCar->GetTireRadius(wp);//
				pi.whTerMtr[w] = carM->whTerMtr[w];  pi.whRoadMtr[w] = carM->whRoadMtr[w];
				pi.whH[w] = cd.whH[w];  pi.whP[w] = cd.whP[w];
				pi.whAngVel[w] = cd.wheel[w].GetAngularVelocity();
				if (w < 2)  pi.whSteerAng[w] = cd.wheel[w].GetSteerAngle();
			}
		}
		

		//  transform axes, vdrift to ogre  car & wheels
		//-----------------------------------------------------------------------

		pi.pos = Vector3(pos[0],pos[2],-pos[1]);
		Quaternion q(rot[0],rot[1],rot[2],rot[3]), q1;
		Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);
		q1.FromAngleAxis(-rad,Vector3(axi.z,-axi.x,-axi.y));  pi.rot = q1 * qFixCar;
		Vector3 vcx,vcz;  q1.ToAxes(vcx,pi.carY,vcz);

		if (!isnan(whPos[0][0]))
		for (int w=0; w < 4; ++w)
		{
			pi.whPos[w] = Vector3(whPos[w][0],whPos[w][2],-whPos[w][1]);
			Quaternion q(whRot[w][0],whRot[w][1],whRot[w][2],whRot[w][3]), q1;
			Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);
			q1.FromAngleAxis(-rad,Vector3(axi.z,-axi.x,-axi.y));  pi.whRot[w] = q1 * qFixWh;
		}
		pi.bNew = true;
		

		///-----------------------------------------------------------------------
		//  record  save data for replay
		///-----------------------------------------------------------------------
		if (pSet->rpl_rec && !pGame->pause && !bGhost && pCar)
		{
			//static int ii = 0;
			//if (ii++ >= 0)	// 1 half game framerate
			{	//ii = 0;
				const CARDYNAMICS& cd = pCar->dynamics;
				ReplayFrame fr;
				fr.time = rplTime;  //  time  from start
				fr.pos = pos;  fr.rot = rot;  //  car
				//  wheels
				for (int w=0; w < 4; ++w)
				{	fr.whPos[w] = whPos[w];  fr.whRot[w] = whRot[w];

					WHEEL_POSITION wp = WHEEL_POSITION(w);
					const TRACKSURFACE* surface = cd.GetWheelContact(wp).GetSurfacePtr();
					fr.surfType[w] = !surface ? TRACKSURFACE::NONE : surface->type;
					//  squeal
					fr.slide[w] = -1.f;  fr.squeal[w] = pCar->GetTireSquealAmount(wp, &fr.slide[w]);
					fr.whVel[w] = cd.GetWheelVelocity(wp).Magnitude();
					//  susp
					fr.suspVel[w] = cd.GetSuspension(wp).GetVelocity();
					fr.suspDisp[w] = cd.GetSuspension(wp).GetDisplacementPercent();
					if (w < 2)
						fr.whSteerAng[w] = cd.wheel[w].GetSteerAngle();
					//replay.header.whR[w] = pCar->GetTireRadius(wp);//
					fr.whTerMtr[w] = carM->whTerMtr[w];  fr.whRoadMtr[w] = carM->whRoadMtr[w];
					//  fluids
					fr.whH[w] = cd.whH[w];  fr.whP[w] = cd.whP[w];
					fr.whAngVel[w] = cd.wheel[w].GetAngularVelocity();
					bool inFl = cd.inFluidsWh[w].size() > 0;
					int idPar = -1;
					if (inFl)
					{	const FluidBox* fb = *cd.inFluidsWh[w].begin();
						idPar = fb->idParticles;  }
					fr.whP[w] = idPar;
					if (w < 2)  pi.whSteerAng[w] = cd.wheel[w].GetSteerAngle();
				}
				//  hud
				fr.vel = pCar->GetSpeedometer();  fr.rpm = pCar->GetEngineRPM();
				fr.gear = pCar->GetGear();  fr.clutch = pCar->GetClutch();
				fr.throttle = cd.GetEngine().GetThrottle();
				fr.steer = pCar->GetLastSteer();
				fr.fboost = cd.doBoost;		fr.percent = carM->trackPercent;
				//  eng snd
				fr.posEngn = cd.GetEnginePosition();
				fr.speed = pCar->GetSpeed();
				fr.dynVel = cd.GetVelocity().Magnitude();
				fr.braking = cd.IsBraking();  //// from posInfo?, todo: simplify this code here ^^
				//  hit sparks
				fr.fHitTime = cd.fHitTime;	fr.fParIntens = cd.fParIntens;	fr.fParVel = cd.fParVel;
				fr.vHitPos = cd.vHitPos;	fr.vHitNorm = cd.vHitNorm;
				fr.whMudSpin = pCar->whMudSpin;
				
				replay.AddFrame(fr, c);  // rec replay
				if (c==0)  /// rec ghost lap
				{
					fr.time = lapTime;
					ghost.AddFrame(fr, 0);
				}
				
				if (valRplName2)  // recorded info
				{
					int size = replay.GetNumFrames() * sizeof(ReplayFrame);
					std::string s = fToStr( float(size)/1000000.f, 2,5);
					String ss = String( TR("#{RplRecTime}: ")) + GetTimeString(replay.GetTimeLength()) + TR("   #{RplSize}: ") + s + TR(" #{UnitMB}");
					valRplName2->setCaption(ss);
				}
			}
		}
		if (bRplPlay && valRplName2)  valRplName2->setCaption("");
		///-----------------------------------------------------------------------
		

		//  chekpoints, lap start
		//-----------------------------------------------------------------------
		if (bRplPlay || bGhost || !sc.ter)   // dont check when replay play
			carM->bWrongChk = false;
		else
		{
			// checkpoint arrow
			if (pSet->check_arrow && carM->eType == CarModel::CT_LOCAL
			  && !bRplPlay && arrowNode && road && road->mChks.size()>0)
			{
				// set animation start to old orientation
				arrowAnimStart = arrowAnimCur;
				
				// game start: no animation
				bool noAnim = carM->iNumChks == 0;
				
				// get vector from camera to checkpoint
				Vector3 chkPos = road->mChks[std::max(0, std::min((int)road->mChks.size()-1, carM->iNextChk))].pos;
					
				// workaround for last checkpoint
				if (carM->iNumChks == road->mChks.size())
				{
					// point arrow to start position
					chkPos = carM->vStartPos;
				}
				
				const Vector3& playerPos = pi.pos;
				Vector3 dir = chkPos - playerPos;
				dir[1] = 0; // only x and z rotation
				Quaternion quat = Vector3::UNIT_Z.getRotationTo(-dir); // convert to quaternion

				const bool valid = !quat.isNaN();
				if (valid)
				{	if (noAnim) arrowAnimStart = quat;
					arrowAnimEnd = quat;
				
					// set arrow color (wrong direction: red arrow)
					// calc angle towards cam
					Real angle = (arrowAnimCur.zAxis().dotProduct(carM->fCam->mCamera->getOrientation().zAxis())+1)/2.0f;
					// set color in material
					MaterialPtr arrowMat = MaterialManager::getSingleton().getByName("Arrow");
					if (!arrowMat.isNull())
					{	Technique* tech = arrowMat->getTechnique(0);
						if (tech && tech->getNumPasses() >= 2)
						if (tech->getPass(1)->hasFragmentProgram())
						{
							GpuProgramParametersSharedPtr fparams = arrowMat->getTechnique(0)->getPass(1)->getFragmentProgramParameters();
							// green: 0.0 1.0 0.0     0.0 0.4 0.0
							// red:   1.0 0.0 0.0     0.4 0.0 0.0
							Vector3 col1 = angle * Vector3(0.0, 1.0, 0.0) + (1-angle) * Vector3(1.0, 0.0, 0.0);
							Vector3 col2 = angle * Vector3(0.0, 0.4, 0.0) + (1-angle) * Vector3(0.4, 0.0, 0.0);
							fparams->setNamedConstant("color1", col1);
							fparams->setNamedConstant("color2", col2);
					}	}
				}
			}
			
			if (carM->bGetStPos)  // first pos is at start
			{	carM->bGetStPos = false;
				carM->matStPos.makeInverseTransform(pi.pos, Vector3::UNIT_SCALE, pi.rot);
				carM->ResetChecks();
			}
			if (road && !carM->bGetStPos)
			{
				//  start/finish box dist
				Vector4 carP(pi.pos.x,pi.pos.y,pi.pos.z,1);
				carM->vStDist = carM0->matStPos * carP;  // start pos from 1st car always
				carM->bInSt = abs(carM->vStDist.x) < road->vStBoxDim.x && 
					abs(carM->vStDist.y) < road->vStBoxDim.y && 
					abs(carM->vStDist.z) < road->vStBoxDim.z;
							
				carM->iInChk = -1;  carM->bWrongChk = false;
				int ncs = road->mChks.size();
				if (ncs > 0)
				{
					//  Finish  --------------------------------------
					if (carM->bInSt && carM->iNumChks == ncs && carM->iCurChk != -1
						&& carM->eType == CarModel::CT_LOCAL)  // only local car(s)
					{
						///  Lap
						bool finished = pGame->timer.GetCurrentLap(c) >= pSet->game.num_laps;
						bool best = finished ? false :  // dont inc laps when race over
							pGame->timer.Lap(c, 0,0, !finished, pSet->game.trackreverse);  //,boost_type?
						double timeCur = pGame->timer.GetPlayerTimeTot(c);

						//  Network notification, send: car id, lap time
						if (mClient && c == 0 && !finished)
							mClient->lap(pGame->timer.GetCurrentLap(c), pGame->timer.GetLastLap(c));

						///  new best lap, save ghost
						if (!pSet->rpl_bestonly || best)
						if (c==0 && pSet->rpl_rec)  // for many, only 1st-
						{
							ghost.SaveFile(GetGhostFile());  //,boost_type?
							ghplay.CopyFrom(ghost);
						}
						ghost.Clear();
						
						carM->ResetChecks();
						//  restore boost fuel, each lap
						if (pSet->game.boost_type == 1 && carM->pCar)
							carM->pCar->dynamics.boostFuel = gfBoostFuelStart;

						///  winner places  for local players > 1
						finished = pGame->timer.GetCurrentLap(c) >= pSet->game.num_laps;
						if (finished && !mClient)
						{
							if (pSet->game.champ_num < 0)
							{
								if (carM->iWonPlace == 0)	//  split screen winners
									carM->iWonPlace = carIdWin++;
							}else
								ChampionshipAdvance(timeCur);
						}
					}
					//  checkpoints  --------------------------------------
					for (int i=0; i < ncs; ++i)
					{
						const CheckSphere& cs = road->mChks[i];
						Real d2 = pi.pos.squaredDistance(cs.pos);
						if (d2 < cs.r2)  // car in checkpoint
						{
							carM->iInChk = i;
							//  next check
							if (i == carM->iNextChk && carM->iNumChks < ncs)
							{
								carM->iCurChk = i;  carM->iNumChks++;
								int ii = (pSet->game.trackreverse ? -1 : 1) * road->iDir;
								carM->iNextChk = (carM->iCurChk + ii + ncs) % ncs;
								//  save car pos and rot
								carM->pCar->SavePosAtCheck();
							}
							else
							if (carM->iInChk != carM->iCurChk)
								carM->bWrongChk = true;
							break;
						}
				}	}
		}	}

		
		///  store new pos info in queue  _________
		int qn = (iCurPoses[c] + 1) % CarPosCnt;  // next index in queue
		carPoses[qn][c] = pi;
		//  update camera
		if (carM->fCam)
			carM->fCam->update(time, pi, &carPoses[qn][c]);
		iCurPoses[c] = qn;  // atomic, set new index in queue

	}
	PROFILER.endBlock(".newPos ");
}


//  updatePoses - Set car pos for Ogre nodes, update particles, trails
//---------------------------------------------------------------------------------------------------------------
void App::updatePoses(float time)
{
	if (carModels.size() == 0)  return;
	PROFILER.beginBlock(".updPos ");
	
	//  Update all carmodels from their carPos
	const CarModel* playerCar = carModels.front();

	for (int c = 0; c < carModels.size(); ++c)
	{
		CarModel* carM = carModels[c];
		if (!carM)  {
			PROFILER.endBlock(".updPos ");
			return;  }
		
		//  hide ghost when empty
		bool bGhost = carM->eType == CarModel::CT_GHOST,
			bGhostVis = (ghplay.GetNumFrames() > 0) && pSet->rpl_ghost;
		if (bGhost)
		{
			carM->setVisible(bGhostVis);
			
			//  hide ghost car when close to player car (only when not transparent)
			if (!pSet->rpl_alpha)
			{
				float distance = carM->pMainNode->getPosition().squaredDistance(playerCar->pMainNode->getPosition());
				if (distance < 16.f)
					carM->setVisible(false);
			}
		}
		
		int q = iCurPoses[c];
		int cc = (c + iRplCarOfs) % carModels.size();  // offset, use camera from other car
		int qq = iCurPoses[cc];
		carM->Update(carPoses[q][c], carPoses[qq][cc], time);
		

		//  nick text pos upd
		if (carM->pNickTxt && carM->pMainNode)
		{
			Camera* cam = playerCar->fCam->mCamera;  //above car 1m
			Vector3 p = projectPoint(cam, carM->pMainNode->getPosition() + Vector3(0,1.f,0));
			p.x = p.x * mSplitMgr->mDims[0].width * 0.5f;  //1st viewport dims
			p.y = p.y * mSplitMgr->mDims[0].height * 0.5f;
			carM->pNickTxt->setPosition(p.x-40, p.y-16);  //center doesnt work
			carM->pNickTxt->setVisible(p.z > 0.f);
		}
	}
	
	///  Replay info
	if (bRplPlay && pGame->cars.size() > 0)
	{
		double pos = pGame->timer.GetPlayerTime(0);
		float len = replay.GetTimeLength();
		if (valRplPerc)  valRplPerc->setCaption(fToStr(pos/len*100.f, 1,4)+" %");
		if (valRplCur)  valRplCur->setCaption(GetTimeString(pos));
		if (valRplLen)  valRplLen->setCaption(GetTimeString(len));

		if (slRplPos)
		{	int v = pos/len * res;  slRplPos->setScrollPosition(v);  }
	}	
	PROFILER.endBlock(".updPos ");
}


//  Update HUD rotated elems - for carId, in baseCarId's space
//---------------------------------------------------------------------------------------------------------------
void App::UpdHUDRot(int baseCarId, int carId, float vel, float rpm)
{
	//if (!pCarM || carId == -1)  return;
	//CarModel* pCarM = carModels[b];
	int b = baseCarId, c = carId;
	bool main = b == c;
	float angBase = carModels[b]->angCarY;

	const float rsc = -180.f/6000.f, rmin = 0.f;  //rmp
	float angrmp = rpm*rsc + rmin;
	const float vsc = pSet->show_mph ? -180.f/100.f : -180.f/160.f, vmin = 0.f;  //vel
	float angvel = abs(vel)*vsc + vmin;
	float angrot = carModels[c]->angCarY;
	if (pSet->mini_rotated && pSet->mini_zoomed && !main)
		angrot -= angBase-180.f;

	float sx = 1.4f, sy = sx*asp;  // *par len
	float psx = 2.1f * pSet->size_minimap, psy = psx;  // *par len

	const static Real tc[4][2] = {{0,1}, {1,1}, {0,0}, {1,0}};  // defaults, no rot
	const static Real tp[4][2] = {{-1,-1}, {1,-1}, {-1,1}, {1,1}};
	const static float d2r = PI_d/180.f;
	const static Real ang[4] = {0.f,90.f,270.f,180.f};

	float rx[4],ry[4], vx[4],vy[4], px[4],py[4], cx[4],cy[4];  // rpm,vel, pos,crc
	for (int i=0; i<4; ++i)  // 4 verts, each +90deg
	{
		float ia = 45.f + ang[i];
		if (main)
		{	float r = -(angrmp + ia) * d2r;   rx[i] = sx*cosf(r);  ry[i] =-sy*sinf(r);
			float v = -(angvel + ia) * d2r;   vx[i] = sx*cosf(v);  vy[i] =-sy*sinf(v);
		}
		float p = -(angrot + ia) * d2r;	  float cp = cosf(p), sp = sinf(p);

		if (pSet->mini_rotated && pSet->mini_zoomed && main)
			{  px[i] = psx*tp[i][0];  py[i] = psy*tp[i][1];  }
		else{  px[i] = psx*cp*1.4f;   py[i] =-psy*sp*1.4f;   }

		float z = pSet->mini_rotated ? 0.70f/pSet->zoom_minimap : 0.5f/pSet->zoom_minimap;
		if (!pSet->mini_rotated)
			{  cx[i] = tp[i][0]*z;  cy[i] = tp[i][1]*z-1.f;  }
		else{  cx[i] =       cp*z;  cy[i] =      -sp*z-1.f;  }
	}
    
    //  rpm,vel needles
	if (main)
	{
		if (moRpm[b])  {	moRpm[b]->beginUpdate(0);
			for (int p=0;p<4;++p)  {  moRpm[b]->position(rx[p],ry[p], 0);
				moRpm[b]->textureCoord(tc[p][0], tc[p][1]);  }	moRpm[b]->end();  }
		if (moVel[b])  {	moVel[b]->beginUpdate(0);
			for (int p=0;p<4;++p)  {  moVel[b]->position(vx[p],vy[p], 0);
				moVel[b]->textureCoord(tc[p][0], tc[p][1]);  }	moVel[b]->end();  }
	}
		
	///  minimap car pos-es rot
	if (vMoPos[b][c])
	{	vMoPos[b][c]->beginUpdate(0);
		for (int p=0;p<4;++p)  {
			vMoPos[b][c]->position(px[p],py[p], 0);
			vMoPos[b][c]->textureCoord(tc[p][0], tc[p][1]);
			vMoPos[b][c]->colour(carModels[c]->color);  }
		vMoPos[b][c]->end();
	}
	
	//  minimap circle/rect rot
	int qb = iCurPoses[b], qc = iCurPoses[c];
	if (moMap[b] && pSet->trackmap && main)
	{
		moMap[b]->beginUpdate(0);
		if (!pSet->mini_zoomed)
			for (int p=0;p<4;++p)  {  moMap[b]->position(tp[p][0],tp[p][1], 0);
				moMap[b]->textureCoord(tc[p][0], tc[p][1]);  moMap[b]->colour(tc[p][0],tc[p][1], 0);  }
		else
		{	
			Vector2 mp(-carPoses[qb][b].pos[2],carPoses[qb][b].pos[0]);
			float xc =  (mp.x - minX)*scX,
				  yc = -(mp.y - minY)*scY+1.f;

			for (int p=0;p<4;++p)  {  moMap[b]->position(tp[p][0],tp[p][1], 0);
				moMap[b]->textureCoord(cx[p]+xc, -cy[p]-yc);  moMap[b]->colour(tc[p][0],tc[p][1], 1);  }
		}
		moMap[b]->end();
	}

	///  minimap car pos  x,y = -1..1
	Vector2 mp(-carPoses[qc][c].pos[2],carPoses[qc][c].pos[0]);

	//  other cars in player's car view space
	if (!main && pSet->mini_zoomed)
	{
		Vector2 plr(-carPoses[qb][b].pos[2],carPoses[qb][b].pos[0]);
		mp -= plr;  mp *= pSet->zoom_minimap;

		if (pSet->mini_rotated)
		{
			float a = angBase * PI_d/180.f;  Vector2 np;
			np.x = mp.x*cosf(a) - mp.y*sinf(a);  // rotate
			np.y = mp.x*sinf(a) + mp.y*cosf(a);  mp = -np;
		}
	}
	float xp = std::min(1.f, std::max(-1.f,  (mp.x - minX)*scX*2.f-1.f )),
		  yp = std::min(1.f, std::max(-1.f, -(mp.y - minY)*scY*2.f+1.f ));
	
	bool bGhost = carModels[c]->eType == CarModel::CT_GHOST,
		bGhostVis = (ghplay.GetNumFrames() > 0) && pSet->rpl_ghost;

	if (vNdPos[b][c])
		if (bGhost && !bGhostVis)  vNdPos[b][c]->setPosition(-100,0,0);  //hide
		else if (pSet->mini_zoomed && main)
			 vNdPos[b][c]->setPosition(0,0,0);
		else vNdPos[b][c]->setPosition(xp,yp,0);
}


//  util
Vector3 App::projectPoint(const Camera* cam, const Vector3& pos)
{
	Vector3 pos2D = cam->getProjectionMatrix() * (cam->getViewMatrix() * pos);

	//Real x = std::min(1.f, std::max(0.f,  pos2D.x * 0.5f + 0.5f ));  // leave on screen edges
	//Real y = std::min(1.f, std::max(0.f, -pos2D.y * 0.5f + 0.5f ));
	Real x =  pos2D.x * 0.5f + 0.5f;
	Real y = -pos2D.y * 0.5f + 0.5f;
	bool out = !cam->isVisible(pos);

	return Vector3(x * mWindow->getWidth(), y * mWindow->getHeight(), out ? -1.f : 1.f);
}

TextBox* App::CreateNickText(int carId, String text)
{
	TextBox* txt = mGUI->createWidget<TextBox>("TextBox",
		100,100, 360,32, Align::Center, "Back", "NickTxt"+toStr(carId));
	txt->setVisible(false);
	txt->setFontHeight(28);  //par 24..32
	txt->setTextShadow(true);  txt->setTextShadowColour(Colour::Black);
	txt->setCaption(text);
	return txt;
}
