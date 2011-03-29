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
		if (pSet->mult_thr == 1)
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
		if (pGame->cars.size() == 0)
			return ret;

		CAR* pCar = &(*pGame->cars.begin());
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


//  update
//---------------------------------------------------------------------------------------------------------------
void App::newPoses()
{
	if (!pGame)  return;
	if (pGame->cars.size() == 0)  return;

	CAR* pCar = &(*pGame->cars.begin());
	const MATHVECTOR <double,3> pos = pCar->dynamics.GetPosition();
	const QUATERNION <double> rot = pCar->dynamics.GetOrientation();
	
	newPos = Vector3(pos[0],pos[2],-pos[1]);

	Quaternion q(rot[0],rot[1],rot[2],rot[3]);
	Radian rad;  Vector3 axi;  q.ToAngleAxis(rad, axi);

	Vector3 vrot(axi.z, -axi.x, -axi.y);
		QUATERNION <double> fix;  fix.Rotate(PI, 0, 1, 0);
		Quaternion qr;  qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();
	Quaternion q1;  q1.FromAngleAxis(-rad, vrot);  q1 = q1 * qr;
	Vector3 vcx,vcy,vcz;  q1.ToAxes(vcx,vcy,vcz);

	newRot = q1;  vCarY = vcy;
	bNew = true;


	///  sound listener  - - - - -
	if (pGame->sound.Enabled())
	{
		pGame->sound.SetListener(
			MATHVECTOR <float, 3> (pos[0], pos[1], pos[2]),
			QUATERNION <float> (),
			//QUATERNION <float> (rot.x(), rot.y(), rot.z(), rot.w()),
			MATHVECTOR <float, 3>());
	}
	bool incar = true;  //..(active_camera->GetName() == "hood" || active_camera->GetName() == "incar");
	{
		std::list <SOUNDSOURCE *> soundlist;
		pCar->GetEngineSoundList(soundlist);
		for (std::list <SOUNDSOURCE *>::iterator s = soundlist.begin(); s != soundlist.end(); s++)
			(*s)->Set3DEffects(!incar);
	}
	

	///  chekpoints, lap start
	///-----------------------------------------------------------------------

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

void App::updatePoses(float time)
{	
	///  ---  set car pos & rot ---
	if (!pGame)  return;
	if (pGame->cars.size() == 0)  return;
	if (!ndCar)  return;

	if (!bNew)  return;
	bNew = false;

	ndCar->setPosition(newPos);
	ndCar->setOrientation(newRot);

	CAR* pCar = &(*pGame->cars.begin());
	
	// on minimap  pos x,y = -1..1
	float xp =(-newPos[2] - minX)*scX*2-1,
		  yp =-(newPos[0] - minY)*scY*2+1;
	if (ndPos)
		ndPos->setPosition(xp,yp,0);
	
	
	//  wheels
	for (int w=0; w < 4; w++)
	{
		WHEEL_POSITION wp = WHEEL_POSITION(w);
		MATHVECTOR <double, 3> pos = pCar->dynamics.GetWheelPosition(wp);
		float wR = pCar->GetTireRadius(wp);
		Vector3 vpos(pos[0],pos[2],-pos[1]);
		ndWh[w]->setPosition(vpos);

			QUATERNION <double> fix;  fix.Rotate(Math::HALF_PI, 0, 1, 0);
			Quaternion qr;  qr.w = fix.w();  qr.x = fix.x();  qr.y = fix.y();  qr.z = fix.z();
		QUATERNION <double> rot = pCar->dynamics.GetWheelOrientation(wp);
		Quaternion q(rot[0],rot[1],rot[2],rot[3]);  //q = qr * q;//
		Radian rad;  Vector3 axi;	q.ToAngleAxis(rad, axi);
		Quaternion q1;  q1.FromAngleAxis(-rad, Vector3(axi.z, -axi.x, -axi.y));  q1 = q1 * qr;
		ndWh[w]->setOrientation(q1);
		
		
		///  Ray dbg  *----*
		//MATHVECTOR <float, 3>& rs = pCar->dynamics.vRayStarts[w], &rd = pCar->dynamics.vRayDirs[w];
		//if (ndRs[w])  ndRs[w]->setPosition(Vector3(rs[0],rs[2],-rs[1]));
		//if (ndRd[w])  ndRd[w]->setPosition(Vector3(rd[0],rd[2],-rd[1]));
		
	

		//  update particle emitters
		//-----------------------------------------------------------------------------
		MATHVECTOR <float,3> vwhVel = pCar->dynamics.GetWheelVelocity(wp);
		float whVel = vwhVel.Magnitude() * 3.6f;  //kmh
		float slide = -1.f, squeal = pCar->GetTireSquealAmount(wp, &slide);
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
		int mtr = min((int)(whTerMtr[w]-1), (int)(sc.td.layers.size()-1));
		TerLayer& lay = whTerMtr[w]==0 ? sc.td.layerRoad : sc.td.layersAll[sc.td.layers[mtr]];
		emitD *= lay.dust;  emitM *= lay.mud;  sizeD *= lay.dustS;  emitS *= lay.smoke;

		//  par emit
		if (pSet->particles)
		{
			if (ps[w] && sc.td.layerRoad.smoke > 0.f/*&& !sc.ter*/)  // only at vdr road
			{
				ParticleEmitter* pe = ps[w]->getEmitter(0);
				pe->setPosition(vpos + vCarY * wR*0.7f); // 0.218
				/**/ps[w]->getAffector(0)->setParameter("alpha", toStr(-0.4f - 0.07f/2.4f * whVel));
				/**/pe->setTimeToLive( max(0.1, 2 - whVel/2.4f * 0.04) );  // fade,live
				pe->setDirection(-vCarY);	pe->setEmissionRate(emitS);
			}
			if (pm[w])	//  mud
			{	ParticleEmitter* pe = pm[w]->getEmitter(0);
				//pe->setDimensions(sizeM,sizeM);
				pe->setPosition(vpos + vCarY * wR*0.7f); // 0.218
				pe->setDirection(-vCarY);	pe->setEmissionRate(emitM);
			}
			if (pd[w])	//  dust
			{	pd[w]->setDefaultDimensions(sizeD,sizeD);
				ParticleEmitter* pe = pd[w]->getEmitter(0);
				pe->setPosition(vpos + vCarY * wR*0.51f ); // 0.16
				pe->setDirection(-vCarY);	pe->setEmissionRate(emitD);
			}
		}

		//  update trails h+
		if (pSet->trails)  {
			if (ndWhE[w])
			{	Vector3 vp = vpos + vCarY * wR*0.72f;  // 0.22
				if (terrain && whTerMtr[w]>0)
					vp.y = terrain->getHeightAtWorldPosition(vp) + 0.05f;
					//if (/*whOnRoad[w]*/whTerMtr[w]>0 && road)  // on road, add ofs
					//	vp.y += road->fHeight;	}/**/
				ndWhE[w]->setPosition(vp);
			}
			float al = 0.5f * /*squeal*/ min(1.f, 0.7f * wht[w]) * onGr;  // par+
			if (whTrl[w])	whTrl[w]->setInitialColour(0,
				lay.tclr.r,lay.tclr.g,lay.tclr.b, lay.tclr.a * al/**/);
		}
	}


	//  par  rain
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
