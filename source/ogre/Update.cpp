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

			if (!pGame->pause && mFCam)
				mFCam->update(pGame->framerate/**/);
			if (ndSky)  ///o-
				ndSky->setPosition(GetCamera()->getPosition());

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
		if (pGame->settings->mult_thr != 1)
		{
			ret = pGame->OneLoop();
			if (!ret)  mShutDown = true;
		}
		updatePoses(time);  //pGame->framerate

		updateReflection();  //*

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
		UpdWhTerMtr(pCar);
		
		return ret;
	}
}
bool App::frameEnd(Real time)
{
	return true;
}


///  terrain mtr from blend maps
//-------------------------------------------------------------------------------------------------------
void App::UpdWhTerMtr(CAR* pCar)
{
	if (!pCar || !ndWh[0])  return;
	if (!terrain || !blendMtr)	// vdr trk
	{
		for (int i=0; i<4; ++i)  // for particles/trails only
			whTerMtr[i] = pCar->dynamics.bWhOnRoad[i] ? 0 : 1;
		return;
	}

	int t = blendMapSize;
	Real tws = sc.td.fTerWorldSize;

	//  wheels
	for (int i=0; i<4; ++i)
	{
		Vector3 w = ndWh[i]->getPosition();
		int mx = (w.x + 0.5*tws)/tws*t, my = (w.z + 0.5*tws)/tws*t;
		mx = max(0,min(t-1, mx)), my = max(0,min(t-1, my));
		
		int mtr = blendMtr[my*t + mx];
		if (pCar->dynamics.bWhOnRoad[i])
			mtr = 0;
		whTerMtr[i] = mtr;

		///  vdr set surface for wheel
		TRACKSURFACE* tsu = &pGame->track.tracksurfaces[mtr];
		pCar->dynamics.terSurf[i] = tsu;
		pCar->dynamics.bTerrain = true;
	}
}


//  update newPoses - get new car pos from game
//---------------------------------------------------------------------------------------------------------------
void App::newPoses()
{
	if (!pGame)  return;
	if (pGame->cars.size() == 0)  return;
	CAR* pCar = &(*pGame->cars.begin());


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
		if (ok)
		{	//  car
			pos = fr.pos;  rot = fr.rot;
			//  wheels
			for (int w=0; w < 4; ++w)
			{
				whPos[w] = fr.whPos[w];  whRot[w] = fr.whRot[w];
				newWhVel[w] = fr.whVel[w];
				newWhSlide[w] = fr.slide[w];  newWhSqueal[w] = fr.squeal[w];
				newWhR[w] = replay.header.whR[w];//
				newWhMtr[w] = fr.whMtr[w];
			}
		}
		else  //+ restart replay (repeat)
			pGame->timer.Lap(0, 0,0, true, pSet->trackreverse);
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
			newWhVel[w] = pCar->dynamics.GetWheelVelocity(wp).Magnitude();
			newWhSlide[w] = -1.f;  newWhSqueal[w] = pCar->GetTireSquealAmount(wp, &newWhSlide[w]);
			newWhR[w] = pCar->GetTireRadius(wp);//
			newWhMtr[w] = whTerMtr[w];
		}
	}
	

	//  transform axes, vdrift to ogre  car & wheels
	//-----------------------------------------------------------------------

	newPos = Vector3(pos[0],pos[2],-pos[1]);
	Quaternion q(rot[0],rot[1],rot[2],rot[3]), q1;
	Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);
	q1.FromAngleAxis(-rad,Vector3(axi.z,-axi.x,-axi.y));  newRot = q1 * qFixCar;
	Vector3 vcx,vcz;  q1.ToAxes(vcx,newCarY,vcz);

	for (int w=0; w < 4; w++)
	{
		newWhPos[w] = Vector3(whPos[w][0],whPos[w][2],-whPos[w][1]);
		Quaternion q(whRot[w][0],whRot[w][1],whRot[w][2],whRot[w][3]), q1;
		Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);
		q1.FromAngleAxis(-rad,Vector3(axi.z,-axi.x,-axi.y));  newWhRot[w] = q1 * qFixWh;
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
				fr.whMtr[w] = whTerMtr[w];
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

	if (bGetStPos)  // first pos is at start
	{	bGetStPos = false;
		matStPos.makeInverseTransform(newPos, Vector3::UNIT_SCALE, newRot);
		iCurChk = -1;  iNextChk = -1;  iNumChks = 1;  // reset lap
	}
	if (road && !bGetStPos)
	{
		//  start/finish box dist
		Vector4 carP(newPos.x,newPos.y,newPos.z,1);
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
				Real d2 = newPos.squaredDistance(cs.pos);
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
	}	}
}


//  updatePoses - set car pos for Ogre meshes, update particles, trails
//---------------------------------------------------------------------------------------------------------------
void App::updatePoses(float time)
{	
	if (!pGame || !ndCar)  return;
	if (pGame->cars.size() == 0)  return;
	if (!bNew)  return;  // new only
	bNew = false;

	//  car pos and rot
	ndCar->setPosition(newPos);
	ndCar->setOrientation(newRot);

	// on minimap  pos x,y = -1..1
	float xp =(-newPos[2] - minX)*scX*2-1,
		  yp =-(newPos[0] - minY)*scY*2+1;
	if (ndPos)
		ndPos->setPosition(xp,yp,0);
	
	
	//  wheels
	for (int w=0; w < 4; w++)
	{
		float wR = newWhR[w];
		ndWh[w]->setPosition(newWhPos[w]);
		ndWh[w]->setOrientation(newWhRot[w]);
		int whMtr = newWhMtr[w];  //whTerMtr[w];
		
		
		//  update particle emitters
		//-----------------------------------------------------------------------------
		float whVel = newWhVel[w] * 3.6f;  //kmh
		float slide = newWhSlide[w], squeal = newWhSqueal[w];
		float onGr = slide < 0.f ? 0.f : 1.f;

		//  wheel temp
		wht[w] += squeal * time * 7;
		wht[w] -= time*6;  if (wht[w] < 0.f)  wht[w] = 0.f;

		///  emit rates +
		Real emitS = 0.f, emitM = 0.f, emitD = 0.f;  //paused
		if (!isFocGui)
		{	 Real sq = squeal* min(1.f, wht[w]), l = pSet->particles_len * onGr;
			 emitS = sq * (whVel * 30) * l *0.3f;  //..
			 emitM = slide < 1.4f ? 0.f :  (8.f * sq * min(5.f, slide) * l);
			 emitD = (min(140.f, whVel) / 3.5f + slide * 1.f ) * l;  }
		Real sizeD = (0.3f + 1.1f * min(140.f, whVel) / 140.f) * (w < 2 ? 0.5f : 1.f);
		//  ter mtr factors
		int mtr = min((int)(whMtr-1), (int)(sc.td.layers.size()-1));
		TerLayer& lay = whMtr==0 ? sc.td.layerRoad : sc.td.layersAll[sc.td.layers[mtr]];
		emitD *= lay.dust;  emitM *= lay.mud;  sizeD *= lay.dustS;  emitS *= lay.smoke;

		//  par emit
		Vector3 vpos = newWhPos[w];
		if (pSet->particles)
		{
			if (ps[w] && sc.td.layerRoad.smoke > 0.f/*&& !sc.ter*/)  // only at vdr road
			{
				ParticleEmitter* pe = ps[w]->getEmitter(0);
				pe->setPosition(vpos + newCarY * wR*0.7f); // 0.218
				/**/ps[w]->getAffector(0)->setParameter("alpha", toStr(-0.4f - 0.07f/2.4f * whVel));
				/**/pe->setTimeToLive( max(0.1, 2 - whVel/2.4f * 0.04) );  // fade,live
				pe->setDirection(-newCarY);	pe->setEmissionRate(emitS);
			}
			if (pm[w])	//  mud
			{	ParticleEmitter* pe = pm[w]->getEmitter(0);
				//pe->setDimensions(sizeM,sizeM);
				pe->setPosition(vpos + newCarY * wR*0.7f); // 0.218
				pe->setDirection(-newCarY);	pe->setEmissionRate(emitM);
			}
			if (pd[w])	//  dust
			{	pd[w]->setDefaultDimensions(sizeD,sizeD);
				ParticleEmitter* pe = pd[w]->getEmitter(0);
				pe->setPosition(vpos + newCarY * wR*0.51f ); // 0.16
				pe->setDirection(-newCarY);	pe->setEmissionRate(emitD);
			}
		}

		//  update trails h+
		if (pSet->trails)  {
			if (ndWhE[w])
			{	Vector3 vp = vpos + newCarY * wR*0.72f;  // 0.22
				if (terrain && whMtr > 0)
					vp.y = terrain->getHeightAtWorldPosition(vp) + 0.05f;
					//if (/*whOnRoad[w]*/whMtr > 0 && road)  // on road, add ofs
					//	vp.y += road->fHeight;	}/**/
				ndWhE[w]->setPosition(vp);
			}
			float al = 0.5f * /*squeal*/ min(1.f, 0.7f * wht[w]) * onGr;  // par+
			if (whTrl[w])	whTrl[w]->setInitialColour(0,
				lay.tclr.r,lay.tclr.g,lay.tclr.b, lay.tclr.a * al/**/);
		}
	}


	//  par  rain cam  . . . .
	if (pSet->particles)
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

}
