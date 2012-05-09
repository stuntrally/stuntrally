#include "pch.h"
#include "common/Defines.h"
#include "OgreGame.h"
#include "FollowCamera.h"
#include "../road/Road.h"

#include "../vdrift/game.h"
#include "../vdrift/quickprof.h"
#include "../network/gameclient.hpp"

#include "common/Slider.h"
#include "SplitScreen.h"
using namespace Ogre;



//  newPoses - Get new car pos from game
//---------------------------------------------------------------------------------------------------------------
void App::newPoses(float time)  // time only for camera update
{
	if (!pGame || bLoading || pGame->cars.size() == 0 /*|| carPoses.empty() || iCurPoses.empty()*/)
		return;
	PROFILER.beginBlock(".newPos ");

	double rplTime = pGame->timer.GetReplayTime(0);
	double lapTime = pGame->timer.GetPlayerTime(0);

	//  iterate through all car models and set new pos info (from vdrift sim or replay)
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
				fr.fHitForce = cd.fHitForce;
				
				replay.AddFrame(fr, c);  // rec replay
				if (c==0)  /// rec ghost lap
				{
					fr.time = lapTime;
					ghost.AddFrame(fr, 0);
				}
				
				if (valRplName2)  // recorded info ..not here, in update
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
						bool finished = (pGame->timer.GetCurrentLap(c) >= pSet->game.num_laps)
										&& (mClient || pSet->game.local_players > 1);
						bool best = finished ? false :  // dont inc laps when race over (in multiplayer or splitscreen mode)
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
		if (!isFocGui || mClient)  // dont if gui, but network always
		{
			int qn = (iCurPoses[c] + 1) % CarPosCnt;  // next index in queue
			carPoses[qn][c] = pi;
			//  update camera
			if (carM->fCam)
				carM->fCam->update(time, pi, &carPoses[qn][c], &pGame->collision);
			iCurPoses[c] = qn;  // atomic, set new index in queue
		}
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
			bool curVisible = carM->mbVisible;
			bool newVisible = bGhostVis;
			
			//  hide ghost car when close to player car (only when not transparent)
			if (!pSet->rpl_alpha)
			{
				float distance = carM->pMainNode->getPosition().squaredDistance(playerCar->pMainNode->getPosition());
				if (distance < 16.f)
					newVisible = false;
			}
			if (curVisible == newVisible)
				carM->hideTime = 0.f;
			else
			{	carM->hideTime += time;  // change vis after delay
				if (carM->hideTime > 0.2f)  // par sec
					carM->setVisible(newVisible);
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
		{	float v = pos/len;  slRplPos->setValue(v);  }
	}	
	
	
	///  objects - dynamic (props)  -------------------------------------------------------------
	for (int i=0; i < sc.objects.size(); ++i)
	{
		Object& o = sc.objects[i];
		if (o.ms)
		{
			btTransform tr, ofs;
			o.ms->getWorldTransform(tr);
			const btVector3& p = tr.getOrigin();
			const btQuaternion& q = tr.getRotation();
			o.pos[0] = p.x();  o.pos[1] = p.y();  o.pos[2] = p.z();
			o.rot[0] = q.x();  o.rot[1] = q.y();  o.rot[2] = q.z();  o.rot[3] = q.w();
			o.SetFromBlt();
		}
	}

	PROFILER.endBlock(".updPos ");
}
