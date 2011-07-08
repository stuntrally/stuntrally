#include "pch.h"
#include "Defines.h"
#include "OgreGame.h"
#include "FollowCamera.h"
#include "../road/Road.h"
#include "../vdrift/game.h"
#include "../paged-geom/PagedGeometry.h"

#include <OgreParticleSystem.h>
using namespace Ogre;


//---------------------------------------------------------------------------------------------------------------
//  Frame Start
//---------------------------------------------------------------------------------------------------------------

void App::UpdThr()
{
	while (!mShutDown)
	{
		///  step Game  **
		//  separate thread
		if (pSet->mult_thr == 1 && !bLoading)
		{
			bool ret = pGame->OneLoop();

			//if (!pGame->pause && mFCam)
			//	mFCam->update(pGame->framerate/**/);
			//if (ndSky)  ///o-
			//	ndSky->setPosition(GetCamera()->getPosition());

			if (!ret)
				mShutDown = true;
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			Sleep(0);  // par
#endif
		}
		/*else
		{	//  2nd test --
			if (road) {  //--
				if (grass)  grass->update();
				if (trees)  trees->update();  }
			Sleep(0);  // par
		}/**/
	}
}

//---------------------------------------------------------------------------------------------------------------

bool App::frameStart(Real time)
{	
	if (bGuiReinit)  // after language change from combo
	{	bGuiReinit = false;
		mGUI->destroyWidgets(vwGui);
		InitGui();
		bWindowResized = true;
		mWndTabs->setIndexSelected(6);  // switch back to view tab
	}

	if (bWindowResized)
	{	bWindowResized = false;

		//  resize Options wnd
		const int yN = 7;
		const Real yw[yN] = {400, 600, 720, 768, 960, 1024, 1200};
		const Real yf[yN] = {0.0, 0.0, 0.05, 0.1, 0.2, 0.3,  0.3};
		Real xm = 0.f, ym = 0.f;  // margin
		for (int i=0; i < yN; ++i)
			if (pSet->windowy >= yw[i]-10)  ym = yf[i];
		Real yo = (1.f - ym)*pSet->windowy, xo = 4.f/3.f * yo;  // opt wnd size in pix
		ym = (pSet->windowy - yo)*0.5f;  xm = (pSet->windowx - xo)*0.5f;
		
		if (mWndOpts)
			mWndOpts->setCoord(xm, ym, xo, yo);

		//  reposition Quit btn
		if (bnQuit)
			bnQuit->setCoord(pSet->windowx - 0.09*pSet->windowx, 0, 0.09*pSet->windowx, 0.03*pSet->windowy);
		bSizeHUD = true;
	}
		
	if (bLoading)
	{
		NewGameDoLoad();
		return true;
	}
	else 
	{
		//  keys dn/up - trklist, carlist
		#define isKey(a)  mKeyboard->isKeyDown(OIS::a)
		static float dirU = 0.f,dirD = 0.f;
		if (isFocGui)
		{	if (isKey(KC_UP)  ||isKey(KC_NUMPAD8))	dirD += time;  else
			if (isKey(KC_DOWN)||isKey(KC_NUMPAD2))	dirU += time;  else
			{	dirU = 0.f;  dirD = 0.f;  }
			int d = ctrl ? 4 : 1;
			if (dirU > 0.0f) {  carLNext( d);  trkLNext( d);  rplLNext( d);  dirU = -0.12f;  }
			if (dirD > 0.0f) {  carLNext(-d);  trkLNext(-d);  rplLNext(-d);  dirD = -0.12f;  }
		}
		
		//  replay forward,backward keys
		if (bRplPlay)
		{
			isFocRpl = ctrl;
			int ta = (isKey(KC_LBRACKET) ? -2 : 0) + (isKey(KC_RBRACKET) ? 2 : 0);
			if (ta)
			{	double tadd = ta;
				tadd *= (shift ? 0.2 : 1) * (ctrl ? 4 : 1) * (alt ? 8 : 1);  // multiplers
				if (!bRplPause)  tadd -= 1;  // play compensate
				double t = pGame->timer.GetReplayTime(), len = replay.GetTimeLength();
				t += tadd * time;  // add
				if (t < 0.0)  t += len;  // cycle
				if (t > len)  t -= len;
				pGame->timer.SetReplayTime(t);
			}
		}

		if (!pGame)
			return false;
		pGame->pause = bRplPlay ? (bRplPause || isFocGui) : isFocGui;


		///  step Game  *******
		//  single thread, sim on draw
		bool ret = true;
		if (pSet->mult_thr != 1)
		{
			ret = pGame->OneLoop();
			if (!ret)  mShutDown = true;
		}
		updatePoses(time);  //pGame->framerate

		//  multi thread
		if (pSet->mult_thr == 1)
		{
			/// ???? ---------
			static QTimer gtim;
			gtim.update();
			double dt = gtim.dt;
			
			// Update cameras for all cars
			for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
				if ((*it)->fCam)
					(*it)->fCam->update(pGame->framerate);
		}

		//  update all cube maps
		for (std::vector<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		if ((*it)->eType != CarModel::CT_GHOST && (*it)->pReflect)
			(*it)->pReflect->Update();

		//  trees
		//if (pSet->mult_thr != 2)
		if (road) {
			if (grass)  grass->update();
			if (trees)  trees->update();  }

		//  road upd lods
		if (road)
		{
			road->RebuildRoadInt();

			//  more than 1 in pre viewport, each frame
			if (pSet->local_players == 1)
			{
				if (roadUpCnt <= 0)
				{
					roadUpCnt = 15;  //par upd, time..
					road->UpdLodVis(pSet->road_dist);
				}
				roadUpCnt--;/**/
			}
		}

		//**  bullet bebug draw
		if (dbgdraw)  {
			dbgdraw->setDebugMode(pSet->bltDebug ? /*255*/1: 0);
			dbgdraw->step();  }

		///  terrain mtr from blend maps
		// now in CarModel::Update
		//UpdWhTerMtr(pCar);
		
		// stop rain/snow when paused
		if (pr && pr2 && pGame)
		{
			if (pGame->pause)
			{
				 pr->setSpeedFactor(0.f);
				 pr2->setSpeedFactor(0.f);
			}
			else
			{
				 pr->setSpeedFactor(1.f);
				 pr2->setSpeedFactor(1.f);
			}
		}
		
		return ret;
	}
}
bool App::frameEnd(Real time)
{
	return true;
}


//  newPoses - Get new car pos from game
//---------------------------------------------------------------------------------------------------------------
void App::newPoses()
{
	if (!pGame)  return;
	if (pGame->cars.size() == 0)  return;
	double rplTime = pGame->timer.GetReplayTime();
	double lapTime = pGame->timer.GetPlayerTime();

	// Iterate through all car models and get new pos info
	int iCarNum = 0;
	std::vector<CarModel*>::iterator carMIt = carModels.begin();
	std::list<PosInfo>::iterator newPosIt = newPosInfos.begin();
	while (carMIt != carModels.end())
	{
		CarModel* carM = *carMIt;
		CAR* pCar = carM->pCar;
		PosInfo posInfo;
		bool bGhost = carM->eType == CarModel::CT_GHOST;
		
		//  local data  car,wheels
		MATHVECTOR <float,3> pos, whPos[4];
		QUATERNION <float> rot, whRot[4];  //?double


		///-----------------------------------------------------------------------
		//  play  get data from replay
		///-----------------------------------------------------------------------
		if (bGhost)
		{
			ReplayFrame fr;
			bool ok = ghplay.GetFrame(lapTime, &fr, 0);
			//  car
			pos = fr.pos;  rot = fr.rot;
			//  wheels
			for (int w=0; w < 4; ++w)
			{
				whPos[w] = fr.whPos[w];  whRot[w] = fr.whRot[w];
				posInfo.whVel[w] = fr.whVel[w];
				posInfo.whSlide[w] = fr.slide[w];  posInfo.whSqueal[w] = fr.squeal[w];
				posInfo.whR[w] = replay.header.whR[iCarNum][w];//
				posInfo.whMtr[w] = fr.whMtr[w];
				posInfo.fboost = fr.fboost;
			}
		}
		else if (bRplPlay)
		{
			//  time  from start
			bool ok = replay.GetFrame(rplTime, &fr, iCarNum);
				if (!ok)	pGame->timer.RestartReplay();
			
			//  car
			pos = fr.pos;  rot = fr.rot;
			//  wheels
			for (int w=0; w < 4; ++w)
			{
				whPos[w] = fr.whPos[w];  whRot[w] = fr.whRot[w];
				posInfo.whVel[w] = fr.whVel[w];
				posInfo.whSlide[w] = fr.slide[w];  posInfo.whSqueal[w] = fr.squeal[w];
				posInfo.whR[w] = replay.header.whR[iCarNum][w];//
				posInfo.whMtr[w] = fr.whMtr[w];
				posInfo.fboost = fr.fboost;
			}
		}
		else
		//  get data from vdrift
		//-----------------------------------------------------------------------
		{
			pos = pCar->dynamics.GetPosition();
			rot = pCar->dynamics.GetOrientation();
			
			for (int w=0; w < 4; ++w)
			{	WHEEL_POSITION wp = WHEEL_POSITION(w);
				whPos[w] = pCar->dynamics.GetWheelPosition(wp);
				whRot[w] = pCar->dynamics.GetWheelOrientation(wp);
				//float wR = pCar->GetTireRadius(wp);
				posInfo.whVel[w] = pCar->dynamics.GetWheelVelocity(wp).Magnitude();
				posInfo.whSlide[w] = -1.f;  posInfo.whSqueal[w] = pCar->GetTireSquealAmount(wp, &posInfo.whSlide[w]);
				posInfo.whR[w] = pCar->GetTireRadius(wp);//
				posInfo.whMtr[w] = carM->whTerMtr[w];
				posInfo.fboost = pCar->dynamics.doBoost;
			}
		}
		

		//  transform axes, vdrift to ogre  car & wheels
		//-----------------------------------------------------------------------

		posInfo.pos = Vector3(pos[0],pos[2],-pos[1]);
		Quaternion q(rot[0],rot[1],rot[2],rot[3]), q1;
		Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);
		q1.FromAngleAxis(-rad,Vector3(axi.z,-axi.x,-axi.y));  posInfo.rot = q1 * qFixCar;
		Vector3 vcx,vcz;  q1.ToAxes(vcx,posInfo.carY,vcz);

		if (!isnan(whPos[0][0]))
		for (int w=0; w < 4; w++)
		{
			posInfo.whPos[w] = Vector3(whPos[w][0],whPos[w][2],-whPos[w][1]);
			Quaternion q(whRot[w][0],whRot[w][1],whRot[w][2],whRot[w][3]), q1;
			Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);
			q1.FromAngleAxis(-rad,Vector3(axi.z,-axi.x,-axi.y));  posInfo.whRot[w] = q1 * qFixWh;
		}
		posInfo.bNew = true;
		

		///  sound listener  - - - - -
		if (!bGhost)
		{
			if (pGame->sound.Enabled())  // todo: set from camera, for each player? ..
			{
				pGame->sound.SetListener(
					MATHVECTOR <float,3> (pos[0], pos[1], pos[2]),
					QUATERNION <float>(),
					//QUATERNION <float> (rot.x(), rot.y(), rot.z(), rot.w()),
					MATHVECTOR <float,3>());
			}
			bool incar = true;
			{
				std::list <SOUNDSOURCE *> soundlist;
				pCar->GetEngineSoundList(soundlist);
				for (std::list <SOUNDSOURCE *>::iterator s = soundlist.begin(); s != soundlist.end(); s++)
					(*s)->Set3DEffects(!incar);
			}
		}

		///-----------------------------------------------------------------------
		//  record  save data for replay
		///-----------------------------------------------------------------------
		if (pSet->rpl_rec && !pGame->pause && !bGhost)
		{
			//static int ii = 0;
			//if (ii++ >= 0)	// 1 half game framerate
			{	//ii = 0;
				ReplayFrame fr;
				fr.time = rplTime;  //  time  from start
				fr.pos = pos;  fr.rot = rot;  //  car
				//  wheels
				for (int w=0; w < 4; w++)
				{	fr.whPos[w] = whPos[w];  fr.whRot[w] = whRot[w];

					WHEEL_POSITION wp = WHEEL_POSITION(w);
					const TRACKSURFACE* surface = pCar->dynamics.GetWheelContact(wp).GetSurfacePtr();
					fr.surfType[w] = !surface ? TRACKSURFACE::NONE : surface->type;
					//  squeal
					fr.slide[w] = -1.f;  fr.squeal[w] = pCar->GetTireSquealAmount(wp, &fr.slide[w]);
					fr.whVel[w] = pCar->dynamics.GetWheelVelocity(wp).Magnitude();
					//  susp
					fr.suspVel[w] = pCar->dynamics.GetSuspension(wp).GetVelocity();
					fr.suspDisp[w] = pCar->dynamics.GetSuspension(wp).GetDisplacementPercent();
					//replay.header.whR[w] = pCar->GetTireRadius(wp);//
					fr.whMtr[w] = carM->whTerMtr[w];
				}
				//  hud
				fr.vel = pCar->GetSpeedometer();  fr.rpm = pCar->GetEngineRPM();
				fr.gear = pCar->GetGear();  fr.clutch = pCar->GetClutch();
				fr.throttle = pCar->dynamics.GetEngine().GetThrottle();
				fr.steer = pCar->GetLastSteer();
				fr.fboost = pCar->dynamics.doBoost;
				//  eng snd
				fr.posEngn = pCar->dynamics.GetEnginePosition();
				fr.speed = pCar->GetSpeed();
				fr.dynVel = pCar->dynamics.GetVelocity().Magnitude();
				
				replay.AddFrame(fr, iCarNum);  // rec replay
				if (iCarNum==0)  /// rec ghost lap
				{
					fr.time = lapTime;
					ghost.AddFrame(fr, 0);
				}
				
				if (valRplName2)  // recorded info
				{
					int size = replay.GetNumFrames() * sizeof(ReplayFrame);
					sprintf(s, "%5.2f", float(size)/1000000.f);
					String ss = String( TR("#{RplRecTime}: ")) + GetTimeString(replay.GetTimeLength()) + TR("   #{RplSize}: ") + s + TR(" #{UnitMB}");
					valRplName2->setCaption(ss);
				}
			}
		}
		if (bRplPlay && valRplName2)  valRplName2->setCaption("");
		///-----------------------------------------------------------------------
		

		//  chekpoints, lap start
		//-----------------------------------------------------------------------
		if (bRplPlay || bGhost)   // dont check when replay play
			carM->bWrongChk = false;
		else //if (iCarNum == 0)  // only works for 1st car?
		{
			if (carM->bGetStPos)  // first pos is at start
			{	carM->bGetStPos = false;
				carM->matStPos.makeInverseTransform(posInfo.pos, Vector3::UNIT_SCALE, posInfo.rot);
				carM->iCurChk = -1;  carM->iNextChk = -1;  carM->iNumChks = 1;  // reset lap
			}
			if (road && !carM->bGetStPos)
			{
				//  start/finish box dist
				Vector4 carP(posInfo.pos.x,posInfo.pos.y,posInfo.pos.z,1);
				carM->vStDist = carM->matStPos * carP;
				carM->bInSt = abs(carM->vStDist.x) < road->vStBoxDim.x && 
					abs(carM->vStDist.y) < road->vStBoxDim.y && 
					abs(carM->vStDist.z) < road->vStBoxDim.z;
			
				carM->iInChk = -1;  carM->bWrongChk = false;
				int ncs = road->mChks.size();
				if (ncs > 0)
				{	if (carM->bInSt && carM->iNumChks == ncs && carM->iCurChk != -1)  // finish
					{
						bool best = pGame->timer.Lap(iCarNum, 0,0, true, pSet->trackreverse);  //pGame->cartimerids[pCar] ?

						if (!pSet->rpl_bestonly || best)  ///  new best lap, save ghost
						if (iCarNum==0)  // for many, only 1st-
						{
							ghost.SaveFile(GetGhostFile());
							ghplay.CopyFrom(ghost);
						}
						ghost.Clear();
						
						carM->iCurChk = -1;  carM->iNumChks = 1;

						///  winner  for local players > 1
						if (carIdWin == -1 && pGame->timer.GetCurrentLap(iCarNum) >= pSet->num_laps)
							carIdWin = iCarNum;
					}
					for (int i=0; i < ncs; ++i)
					{
						const CheckSphere& cs = road->mChks[i];
						Real d2 = posInfo.pos.squaredDistance(cs.pos);
						if (d2 < cs.r2)  // car in checkpoint
						{
							carM->iInChk = i;
							if (carM->iCurChk == -1)  // first, any
							{	carM->iCurChk = i;  carM->iNumChks = 1;  }
							else if (carM->iNumChks < ncs)
							{
								int ii = (pSet->trackreverse ? -1 : 1) * road->iDir;
								carM->iNextChk = (carM->iCurChk + ii + ncs) % ncs;
									
								//  any if first, or next
								if (i == carM->iNextChk)
								{	carM->iCurChk = i;  carM->iNumChks++;  }
								else
								if (carM->iInChk != carM->iCurChk)
									carM->bWrongChk = true;
							}
							break;
						}
				}	}
		}	}

		(*newPosIt) = posInfo;
		carMIt++;  newPosIt++;  iCarNum++;  // next
	}
}


//  updatePoses - Set car pos for Ogre nodes, update particles, trails
//---------------------------------------------------------------------------------------------------------------
void App::updatePoses(float time)
{	
	//  Update all carmodels with their newPosInfo
	int i=0;
	std::vector<CarModel*>::iterator carIt = carModels.begin();
	std::list<PosInfo>::iterator newPosIt = newPosInfos.begin();
	while (carIt != carModels.end())
	{
		CarModel* carM = *carIt;
		if (!carM)  return;
		PosInfo newPosInfo = *newPosIt;
		
		//  hide ghost when empty
		bool bGhost = carM->eType == CarModel::CT_GHOST;
		if (bGhost)
			carM->setVisible((ghplay.GetNumFrames() > 0) && pSet->rpl_ghost);
		
		carM->Update(newPosInfo, time);
		
		//  pos on minimap  x,y = -1..1
		if (!bGhost)
		{	float xp =(-newPosInfo.pos[2] - minX)*scX*2-1,
				  yp =-(newPosInfo.pos[0] - minY)*scY*2+1;
			if (ndPos[i])
				ndPos[i]->setPosition(xp,yp,0);
		}
		carIt++;  newPosIt++;  i++;
	}
	
	///  Replay info
	if (bRplPlay && pGame->cars.size() > 0)
	{
		double pos = pGame->timer.GetPlayerTime();
		float len = replay.GetTimeLength();
		if (valRplPerc){  sprintf(s, "%4.1f %%", pos/len * 100.f);  valRplPerc->setCaption(s);  }
		if (valRplCur)  valRplCur->setCaption(GetTimeString(pos));
		if (valRplLen)  valRplLen->setCaption(GetTimeString(len));

		if (slRplPos)
		{
			#define res  1000000.f
			int v = pos/len * res;  slRplPos->setScrollPosition(v);
		}
	}	
}
