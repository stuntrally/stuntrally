#include "stdafx.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "FollowCamera.h"
#include "../road/Road.h"



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
	if (bWindowResized)
	{
		//comboResolution(NULL, 0);
		btnResChng(0);
		bWindowResized = false;
	}
	
	if (bLoading)
	{
		NewGameDoLoad();
		return true;
	}
	else 
	{
		//  keys dn/up - trklist, carlist
		#define isKey(a)  mKeyboard->isKeyDown(OIS::KC_##a)
		static float dirU = 0.f,dirD = 0.f;
		if (isFocGui)
		{	if (isKey(UP)  ||isKey(NUMPAD8))	dirD += time;  else
			if (isKey(DOWN)||isKey(NUMPAD2))	dirU += time;  else
			{	dirU = 0.f;  dirD = 0.f;  }
			int d = ctrl ? 4 : 1;
			if (dirU > 0.0f) {  carListNext( d);  trkListNext( d);  dirU = -0.12f;  }
			if (dirD > 0.0f) {  carListNext(-d);  trkListNext(-d);  dirD = -0.12f;  }
		}

		if (!pGame)
			return false;
		pGame->pause = isFocGui;

		///  step Game  **
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
			for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
			{
				if ((*it)->fCam)
					(*it)->fCam->update(pGame->framerate);
			}
			
			if (ndSky)  ///o-
				ndSky->setPosition(GetCamera()->getPosition());
		}

		// Update all cube maps
		for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
		{
			(*it)->pReflect->Update();
		}

		//  trees
		//if (pSet->mult_thr != 2)
		if (road) {
			if (grass)  grass->update();
			if (trees)  trees->update();  }

		//  road upd lods
		if (road)
		{
			road->RebuildRoadInt();
			if (roadUpCnt <= 0)
			{
				roadUpCnt = 20;  //par upd, time..
				road->UpdLodVis(pSet->road_dist);
			}
			roadUpCnt--;
		}

		//**  bullet bebug draw
		if (dbgdraw)  {
			dbgdraw->setDebugMode(pSet->bltDebug ? /*255*/1: 0);
			dbgdraw->step();  }

		//  hud
		CAR* pCar = pGame->cars.size() == 0 ? NULL : &(*pGame->cars.begin());
		UpdateHUD(pCar, time);

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
		
		//  par  rain cam  . . . .
		if (pSet->particles && time != 0)
		{	const Vector3& pos = mCamera->getPosition();
				static Vector3 oldPos = Vector3::ZERO;
				Vector3 vel = (pos-oldPos)/time;  oldPos = pos;
			Vector3 dir = mCamera->getDirection();//, up = mCamera->getUp();
			Vector3 par = pos + dir * 12 + vel * 0.4;
			if (pr && sc.rainEmit > 0)
			{
				ParticleEmitter* pe = pr->getEmitter(0);
				pe->setPosition(par);
				pe->setEmissionRate(sc.rainEmit);
			}
			if (pr2 && sc.rain2Emit > 0)
			{
				ParticleEmitter* pe = pr2->getEmitter(0);
				pe->setPosition(par);	//pe->setDirection(-up);
				pe->setEmissionRate(sc.rain2Emit);
			}
		}
		
		return ret;
	}
}
bool App::frameEnd(Real time)
{
	return true;
}


//  update newPoses - get new car pos from game
//---------------------------------------------------------------------------------------------------------------
void App::newPoses()
{
	if (!pGame)  return;
	if (pGame->cars.size() == 0)  return;
	// Iterate through all car models and get new pos info
	std::list<CAR>::iterator carIt = pGame->cars.begin();
	std::list<CarModel*>::iterator carMIt = carModels.begin();
	std::list<PosInfo>::iterator newPosInfoIt = newPosInfos.begin();
	///TODO -how to handle CarModels that don't have a vdrift car?
	while (carMIt != carModels.end() )
	{
		CAR* pCar = &(*carIt);
		CarModel* carM = *carMIt;
		PosInfo newPosInfo;
		
		//  local data  car,wheels
		MATHVECTOR <float,3> pos, whPos[4];
		QUATERNION <float> rot, whRot[4];  //?double


		///-----------------------------------------------------------------------
		//  play  get data from replay
		///-----------------------------------------------------------------------
		if (pSet->rpl_play)
		{
			//  time  from start
			double rtime = pGame->timer.GetReplayTime();
			bool ok = replay.GetFrame(rtime, &fr);
			if (!ok)	pGame->timer.RestartReplay();
			
			//  car
			pos = fr.pos;  rot = fr.rot;
			//  wheels
			for (int w=0; w < 4; ++w)
			{
				whPos[w] = fr.whPos[w];  whRot[w] = fr.whRot[w];
				newPosInfo.newWhVel[w] = fr.whVel[w];
				newPosInfo.newWhSlide[w] = fr.slide[w];  newPosInfo.newWhSqueal[w] = fr.squeal[w];
				newPosInfo.newWhR[w] = replay.header.whR[w];//
				newPosInfo.newWhMtr[w] = fr.whMtr[w];
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
				newPosInfo.newWhVel[w] = pCar->dynamics.GetWheelVelocity(wp).Magnitude();
				newPosInfo.newWhSlide[w] = -1.f;  newPosInfo.newWhSqueal[w] = pCar->GetTireSquealAmount(wp, &newPosInfo.newWhSlide[w]);
				newPosInfo.newWhR[w] = pCar->GetTireRadius(wp);//
				newPosInfo.newWhMtr[w] = carM->whTerMtr[w];
			}
		}
		

		//  transform axes, vdrift to ogre  car & wheels
		//-----------------------------------------------------------------------

		newPosInfo.newPos = Vector3(pos[0],pos[2],-pos[1]);
		Quaternion q(rot[0],rot[1],rot[2],rot[3]), q1;
		Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);
		q1.FromAngleAxis(-rad,Vector3(axi.z,-axi.x,-axi.y));  newPosInfo.newRot = q1 * qFixCar;
		Vector3 vcx,vcz;  q1.ToAxes(vcx,newPosInfo.newCarY,vcz);

		for (int w=0; w < 4; w++)
		{
			newPosInfo.newWhPos[w] = Vector3(whPos[w][0],whPos[w][2],-whPos[w][1]);
			Quaternion q(whRot[w][0],whRot[w][1],whRot[w][2],whRot[w][3]), q1;
			Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);
			q1.FromAngleAxis(-rad,Vector3(axi.z,-axi.x,-axi.y));  newPosInfo.newWhRot[w] = q1 * qFixWh;
		}
		bNew = true;
		

		///  sound listener  - - - - -
		if (pGame->sound.Enabled())  // todo: set from camera ..
		{
			pGame->sound.SetListener(
				MATHVECTOR <float,3> (pos[0], pos[1], pos[2]),
				QUATERNION <float>(),
				//QUATERNION <float> (rot.x(), rot.y(), rot.z(), rot.w()),
				MATHVECTOR <float,3>());
		}
		bool incar = true;  //..(active_camera->GetName() == "hood" || active_camera->GetName() == "incar");
		{
			std::list <SOUNDSOURCE *> soundlist;
			pCar->GetEngineSoundList(soundlist);
			for (std::list <SOUNDSOURCE *>::iterator s = soundlist.begin(); s != soundlist.end(); s++)
				(*s)->Set3DEffects(!incar);
		}

		///-----------------------------------------------------------------------
		//  record  save data for replay
		///-----------------------------------------------------------------------
		if (pSet->rpl_rec)
		{
			static int ii = 0;
			if (ii++ >= 0)	// 1 half game framerate
			{	ii = 0;
				ReplayFrame fr;
				fr.time = pGame->timer.GetReplayTime();  //  time  from start
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
				//  eng snd
				fr.posEngn = pCar->dynamics.GetEnginePosition();
				fr.speed = pCar->GetSpeed();
				fr.dynVel = pCar->dynamics.GetVelocity().Magnitude();
				replay.AddFrame(fr);
			}
		}
		///-----------------------------------------------------------------------
		

		//  chekpoints, lap start
		//-----------------------------------------------------------------------
		if (pSet->rpl_play)
		{	// dont check when replay play...
			bWrongChk = false;
		}
		else
		{
			if (bGetStPos)  // first pos is at start
			{	bGetStPos = false;
				matStPos.makeInverseTransform(newPosInfo.newPos, Vector3::UNIT_SCALE, newPosInfo.newRot);
				iCurChk = -1;  iNextChk = -1;  iNumChks = 1;  // reset lap
			}
			if (road && !bGetStPos)
			{
				//  start/finish box dist
				Vector4 carP(newPosInfo.newPos.x,newPosInfo.newPos.y,newPosInfo.newPos.z,1);
				vStDist = matStPos * carP;
				bInSt = abs(vStDist.x) < road->vStBoxDim.x && 
					abs(vStDist.y) < road->vStBoxDim.y && 
					abs(vStDist.z) < road->vStBoxDim.z;
			
				iInChk = -1;  bWrongChk = false;
				int ncs = road->mChks.size();
				if (ncs > 0)
				{
					if (bInSt && iNumChks == ncs && iCurChk != -1)  // finish
					{
						pGame->timer.Lap(0, 0,0, true, pSet->trackreverse);
						iCurChk = -1;  iNumChks = 1;
					}
					for (int i=0; i < ncs; ++i)
					{
						const CheckSphere& cs = road->mChks[i];
						Real d2 = newPosInfo.newPos.squaredDistance(cs.pos);
						if (d2 < cs.r2)  // car in checkpoint
						{
							iInChk = i;
							if (iCurChk == -1)  // first, any
							{	iCurChk = i;  iNumChks = 1;  }
							else if (iNumChks < ncs)
							{
								int ii = (pSet->trackreverse ? -1 : 1) * road->iDir;
								iNextChk = (iCurChk + ii + ncs) % ncs;
									
								//  any if first, or next
								if (i == iNextChk)
								{	iCurChk = i;  iNumChks++;  }
								else
								if (iInChk != iCurChk)
									bWrongChk = true;
							}
							break;
						}
					}
				}	
			}
		}
		(*newPosInfoIt) = newPosInfo;
		
		carIt++;
		carMIt++;
		newPosInfoIt++;
	}
}


//  updatePoses - set car pos for Ogre nodes, update particles, trails
//---------------------------------------------------------------------------------------------------------------
void App::updatePoses(float time)
{	
	// Update all carmodels with their newPosInfo
	std::list<CarModel*>::iterator carIt = carModels.begin();
	std::list<PosInfo>::iterator newPosIt = newPosInfos.begin();
	while (carIt != carModels.end())
	{
		CarModel* carM = *carIt;
		if (!carM) return;
		PosInfo newPosInfo = *newPosIt;
		
		carM->Update(newPosInfo, time);
		
		/// TODO multiple dots on minimap
		//  pos on minimap  x,y = -1..1
		float xp =(-newPosInfo.newPos[2] - minX)*scX*2-1,
			  yp =-(newPosInfo.newPos[0] - minY)*scY*2+1;
		if (ndPos)
			ndPos->setPosition(xp,yp,0);
			
		carIt++;
		newPosIt++;
	}
	
	///  Replay info
	if (pSet->rpl_play)
	{
		double pos = pGame->timer.GetPlayerTime();
		float len = replay.GetTimeLength();
		if (valRplPerc){  sprintf(s, "%4.1f %%", pos/len * 100.f);  valRplPerc->setCaption(s);  }
		if (valRplCur){  valRplCur->setCaption( GetTimeString( pos ) );  }
		if (valRplLen){  valRplLen->setCaption( GetTimeString( len ) );  }
		
		if (slRplPos)
		{
			#define res  1000000.f
			int v = pos/len * res;  slRplPos->setScrollPosition(v);
		}
	}	
}
