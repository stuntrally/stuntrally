#include "stdafx.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "../road/Road.h"



///---------------------------------------------------------------------------------------------------------------
///  HUD create  rpm, vel
///---------------------------------------------------------------------------------------------------------------

void App::SizeHUD(bool full)
{
	float fHudScale = pSet->size_gauges;
	asp = float(mWindow->getWidth())/float(mWindow->getHeight());

	Real spx = fHudScale*1.1, spy = spx*asp;
	xcRpm = -1 + spx;  ycRpm = -1 + spy;
	xcVel =  1 - spx;  ycVel = -1 + spy;

	if (full &&	nrpmB && nvelBk && nvelBm && nrpm &&nvel)
	{
		Vector3 sca(fHudScale,fHudScale*asp,1), sc(fHudScale,fHudScale,1);
		nrpmB->setScale(sca);	nvelBk->setScale(sca);  nvelBm->setScale(sca);
		nrpm->setScale(sc); 	nvel->setScale(sc);

		Vector3 vr(xcRpm,ycRpm,0), vv(xcVel,ycVel,0);
		nrpmB->setPosition(vr);	nvelBk->setPosition(vv);  nvelBm->setPosition(vv);
		nrpm->setPosition(vr);	nvel->setPosition(vv);
	}

	if (ndMap)
	{
		float fHudSize = pSet->size_minimap;
		ndMap->setScale(fHudSize, fHudSize*asp, 1);

		const float marg = 1.f + 0.05f;  // from border
		fMiniX = 1 - fHudSize * marg, fMiniY = 1 - fHudSize*asp * marg;

		ndMap->setPosition(Vector3(fMiniX,fMiniY,0));
	}/**/
}


void App::CreateHUD()
{	
	//  minimap from road img
	asp = 1.f;
	if (terrain)
	{
		ofsX=0; ofsY=0;
		float t = sc.td.fTerWorldSize*0.5;
		minX = -t;  minY = -t;  maxX = t;  maxY = t;

		float fMapSizeX = maxX - minX, fMapSizeY = maxY - minY;  // map size
		float size = max(fMapSizeX, fMapSizeY*asp);
		scX = 1.f / size;  scY = 1.f / size;

		asp = 1.f;  //_temp
		ManualObject* m = Create2D("road_minimap_inv",1);
		//asp = float(mWindow->getWidth())/float(mWindow->getHeight());
		m->setVisibilityFlags(2);
		m->setRenderQueueGroup(RENDER_QUEUE_OVERLAY-5);
		
		///  change minimap image
		MaterialPtr mm = MaterialManager::getSingleton().getByName("road_minimap_inv");
		Pass* pass = mm->getTechnique(0)->getPass(0);
		TextureUnitState* tus = pass->getTextureUnitState(0);
		if (tus)
			tus->setTextureName(pSet->track + "_mini.png");

		float fHudSize = pSet->size_minimap;
		const float marg = 1.f + 0.05f;  // from border
		fMiniX = 1 - fHudSize * marg, fMiniY = 1 - fHudSize*asp * marg;

		ndMap = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(fMiniX,fMiniY,0));
		ndMap->scale(fHudSize, fHudSize*asp, 1);
		ndMap->attachObject(m);
		
		//  car pos dot
		mpos = Create2D("hud/CarPos", 0.2f, true);  // dot size  -par
		mpos->setVisibilityFlags(2);
		mpos->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);
		ndPos = ndMap->createChildSceneNode();
		ndPos->scale(fHudSize, fHudSize, 1);
		ndPos->attachObject(mpos);
		ndMap->setVisible(pSet->trackmap);
	}

	
	//  backgr  gauges
	ManualObject* mrpmB = Create2D("hud/rpm",1);	mrpmB->setVisibilityFlags(2);
	mrpmB->setRenderQueueGroup(RENDER_QUEUE_OVERLAY-5);
	nrpmB = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	nrpmB->attachObject(mrpmB);	nrpmB->setScale(0,0,0);  nrpmB->setVisible(false);

	ManualObject* mvelBk = Create2D("hud/kmh",1);	mvelBk->setVisibilityFlags(2);
	mvelBk->setRenderQueueGroup(RENDER_QUEUE_OVERLAY-5);
	nvelBk = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	nvelBk->attachObject(mvelBk);	nvelBk->setScale(0,0,0);  mvelBk->setVisible(false);
		
	ManualObject* mvelBm = Create2D("hud/mph",1);	mvelBm->setVisibilityFlags(2);
	mvelBm->setRenderQueueGroup(RENDER_QUEUE_OVERLAY-5);
	nvelBm = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	nvelBm->attachObject(mvelBm);	nvelBm->setScale(0,0,0);  mvelBm->setVisible(false);
		
	//  needles
	mrpm = Create2D("hud/needle",1,true);	mrpm->setVisibilityFlags(2);
	mrpm->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);
	nrpm = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	nrpm->attachObject(mrpm);	nrpm->setScale(0,0,0);	nrpm->setVisible(false);
	
	mvel = Create2D("hud/needle",1,true);	mvel->setVisibilityFlags(2);
	mvel->setRenderQueueGroup(RENDER_QUEUE_OVERLAY);
	nvel = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	nvel->attachObject(mvel);	nvel->setScale(0,0,0);	nvel->setVisible(false);


	//  overlays
	OverlayManager& ovr = OverlayManager::getSingleton();
	ovCam = ovr.getByName("Car/CameraOverlay");

	ovGear = ovr.getByName("Hud/Gear");
	ovVel = ovr.getByName("Hud/Vel");
	hudGear = ovr.getOverlayElement("Hud/GearText");
	hudVel = ovr.getOverlayElement("Hud/VelText");

	ovAbsTcs = ovr.getByName("Hud/AbsTcs");
	ovCarDbg = ovr.getByName("Car/Stats");
	hudAbs = ovr.getOverlayElement("Hud/AbsText");
	hudTcs = ovr.getOverlayElement("Hud/TcsText");

	ovTimes = ovr.getByName("Hud/Times");
	hudTimes = ovr.getOverlayElement("Hud/TimesText");
	hudCheck = ovr.getOverlayElement("Hud/TimesCheck");

	//  dbg lines
	ovCarDbgTxt = ovr.getByName("Car/StatsTxt");  //ovCarDbgTxt->show();
	ovCarDbg = ovr.getByName("Car/Stats");  //ovCarDbg->show();  // bars
	for (int i=0; i < 5; i++)
	{	ovL[i] = ovr.getOverlayElement("L_"+toStr(i+1));
		ovR[i] = ovr.getOverlayElement("R_"+toStr(i+1));
		ovS[i] = ovr.getOverlayElement("S_"+toStr(i+1));
		ovU[i] = ovr.getOverlayElement("U_"+toStr(i+1));	}

	ShowHUD();  //_
	bSizeHUD = true;
	//SizeHUD(true);
}


void App::ShowHUD(bool hideAll)
{
	if (hideAll)
	{
		if (nrpmB)  nrpmB->setVisible(false);
		if (nvelBk)	nvelBk->setVisible(false);
		if (nvelBm)	nvelBm->setVisible(false);
		if (nrpm)	nrpm->setVisible(false);
		if (nvel)	nvel->setVisible(false);
		if (ovGear)	{ ovGear->hide();  }
		if (ovVel)	{ ovVel->hide();   }
		if (ovAbsTcs){ ovAbsTcs->hide(); }
		if (hudGear){ hudGear->hide(); }
		if (hudVel) { hudVel->hide(); }
		if (ovCarDbg){ ovCarDbg->hide();   }
		if (ovCarDbgTxt){ ovCarDbgTxt->hide();   }

		if (ovCam)	{ ovCam->hide();     }
		if (ovTimes){ ovTimes->hide();   }
	}
	else
	{
		bool show = pSet->show_gauges;
		if (nrpmB)  nrpmB->setVisible(show);
		if (nvelBk)	nvelBk->setVisible(show && !pSet->show_mph);
		if (nvelBm)	nvelBm->setVisible(show && pSet->show_mph);
		if (nrpm)	nrpm->setVisible(show);
		if (nvel)	nvel->setVisible(show);
		if (ovGear)	{  if (1||show)  ovGear->show();  else  ovGear->hide();  }
		if (ovVel)	{  if (1||show)  ovVel->show();   else  ovVel->hide();   }
		if (ovAbsTcs){ if (show)  ovAbsTcs->show();   else  ovAbsTcs->hide(); }
		if (hudGear) {if (show) hudGear->show(); else hudGear->hide(); }
		if (hudVel) {if (show) hudVel->show(); else hudVel->hide(); }

		show = pSet->car_dbgbars;
		if (ovCarDbg){  if (show)  ovCarDbg->show();  else  ovCarDbg->hide();   }
		show = pSet->car_dbgtxt;
		if (ovCarDbgTxt){  if (show)  ovCarDbgTxt->show();  else  ovCarDbgTxt->hide();   }
		//for (int i=0; i<5; ++i)
		//{	if (ovU[i])  if (show)  ovU[i]->show();  else  ovU[i]->hide();  }

		if (ovCam)	{  if (pSet->show_cam)    ovCam->show();    else  ovCam->hide();     }
		if (ovTimes){  if (pSet->show_times)  ovTimes->show();  else  ovTimes->hide();   }
	}
}


//  Update HUD
///---------------------------------------------------------------------------------------------------------------
void App::UpdateHUD(CAR* pCar, float time)
{
	if (bSizeHUD)
	{	bSizeHUD = false;
		SizeHUD(true);	}
	
	///  hud rpm,vel  --------------------------------
	if (pCar && !pSet->rpl_play)
	{	fr.vel = pCar->GetSpeedometer();
		fr.rpm = pCar->GetEngineRPM();
		fr.gear = pCar->GetGear();
		fr.clutch = pCar->GetClutch();
		//fr.throttle = pCar->dynamics.GetEngine().GetThrottle();  // not on hud
	}	
    float vel = fr.vel * (pSet->show_mph ? 2.23693629f : 3.6f);
    const float rsc = -180.f/6000.f, rmin = 0.f;  //rmp
    float angrmp = fr.rpm*rsc + rmin;
    float vsc = pSet->show_mph ? -180.f/100.f : -180.f/160.f, vmin = 0.f;  //vel
    float angvel = abs(vel)*vsc + vmin;
    float angrot = ndCar ? ndCar->getOrientation().getYaw().valueDegrees() : 0.f;
    float sx = 1.4f, sy = sx*asp;  // *par len
    float psx = 2.1f * pSet->size_minimap, psy = psx;  // *par len

    const static float d2r = PI/180.f;
    static float rx[4],ry[4], vx[4],vy[4], px[4],py[4];
    for (int i=0; i<4; i++)
    {
		float ia = 45.f + float(i)*90.f;
		float r = -(angrmp + ia) * d2r;
		float v = -(angvel + ia) * d2r;
		float p = -(angrot + ia) * d2r;
		rx[i] = sx*cosf(r);  ry[i] =-sy*sinf(r);
		vx[i] = sx*cosf(v);  vy[i] =-sy*sinf(v);
		px[i] = psx*cosf(p);  py[i] =-psy*sinf(p);
    }
    
    if (mrpm)  {	mrpm->beginUpdate(0);
		mrpm->position(rx[0],ry[0], 0);  mrpm->textureCoord(0, 1);	mrpm->position(rx[1],ry[1], 0);  mrpm->textureCoord(1, 1);
		mrpm->position(rx[2],ry[2], 0);  mrpm->textureCoord(1, 0);	mrpm->position(rx[3],ry[3], 0);  mrpm->textureCoord(0, 0);
		mrpm->end();  }

	if (mvel)  {	mvel->beginUpdate(0);
		mvel->position(vx[0],vy[0], 0);  mvel->textureCoord(0, 1);	mvel->position(vx[1],vy[1], 0);  mvel->textureCoord(1, 1);
		mvel->position(vx[2],vy[2], 0);  mvel->textureCoord(1, 0);	mvel->position(vx[3],vy[3], 0);  mvel->textureCoord(0, 0);
		mvel->end();  }
		
	if (mpos)  {	mpos->beginUpdate(0);
		mpos->position(px[0],py[0], 0);  mpos->textureCoord(0, 1);	mpos->position(px[1],py[1], 0);  mpos->textureCoord(1, 1);
		mpos->position(px[2],py[2], 0);  mpos->textureCoord(1, 0);	mpos->position(px[3],py[3], 0);  mpos->textureCoord(0, 0);
		mpos->end();  }


	//  gear, vel texts  -----------------------------
	if (hudGear && hudVel)
	{
		char cg[2],sv[8];  cg[1]=0;
		float cl = fr.clutch*0.8f + 0.2f;
		if (fr.gear == -1)
		{	cg[0]='R';  hudGear->setColour(ColourValue(0.3,1,1,cl));	}
		else if (fr.gear == 0)
		{	cg[0]='N';  hudGear->setColour(ColourValue(0.3,1,0.3,cl));	}
		else
		{	cg[0]='0'+fr.gear;  hudGear->setColour(ColourValue(1,1-fr.gear*0.1,0.2,cl));	}

		sprintf(sv, "%3.0f", abs(vel));
		hudGear->setCaption(String(cg));
		hudVel->setCaption(String(sv));  int w = mWindow->getWidth();
		hudVel->setPosition(-0.055 + w/1600.f*0.045,-0.01);
		//hudVel->setPosition(-0.1 + (w-1024.f)/1600.f*0.07/*0.11*/,-0.01);

		float k = pCar->GetSpeedometer() * 3.6f * 0.0025f;	// vel clr
		#define m01(x)  min(1.0, max(0.0, (double) x))
		hudVel->setColour(ColourValue(m01(k*2), m01(0.5+k*1.5-k*k*2.5), m01(1+k*0.8-k*k*3.5)));
	}
	
	//  abs, tcs on  --------
	if (hudAbs && hudTcs)
	{
		if (pCar->GetABSEnabled())
		{	hudAbs->show();
			hudAbs->setColour(ColourValue(1,0.8,0.6, pCar->GetABSActive() ? 1 : 0.5));
		}else
			hudAbs->hide();

		if (pCar->GetTCSEnabled())
		{	hudTcs->show();
			hudTcs->setColour(ColourValue(0.7,0.9,1, pCar->GetTCSActive() ? 1 : 0.4));
		}else
			hudTcs->hide();
	}
	
	//  times, score  --------
	if (pSet->show_times)
	{
		TIMER& tim = pGame->timer;	//car[playercarindex].
		s[0]=0;
		if (!road)  // no score on terrain
		if (tim.GetIsDrifting(0))
			sprintf(s, "score  %3.0f+%2.0f", tim.GetDriftScore(0), tim.GetThisDriftScore(0) );
		else
			sprintf(s, "score  %3.0f", tim.GetDriftScore(0) );

		hudTimes->setCaption(String(s) +
			String("\nTime ") + GetTimeString(tim.GetPlayerTime())+
			String("\nLast ") + GetTimeString(tim.GetLastLap())+
			String("\nBest ") + GetTimeString(tim.GetBestLap(pSet->trackreverse)) );
	}
	
	//-----------------------------------------------------------------------------------------------
	///  debug infos
	//-----------------------------------------------------------------------------------------------

	//  car debug text  --------
	/*if (pSet->car_dbgtxt)
	{
		std::stringstream s1,s2,s3,s4;
		pCar->DebugPrint(s1, true, false, false, false);  ovU[0]->setCaption(s1.str());
		pCar->DebugPrint(s2, false, true, false, false);  ovU[1]->setCaption(s2.str());
		pCar->DebugPrint(s3, false, false, true, false);  ovU[2]->setCaption(s3.str());
		pCar->DebugPrint(s4, false, false, false, true);  ovU[3]->setCaption(s4.str());
	}/**/

	//  profiling times
	if (pGame->profilingmode && ovU[3])
	{
		//ovU[3]->setCaption(pGame->strProfInfo);
	}

	//  wheels slide, susp bars  --------
	if (pSet->car_dbgbars)
	{
		const Real xp = 80, yp = -530, ln = 20, y4 = 104;
		static char ss[256];
		const static char swh[4][6] = {"F^L<","F^R>","RvL<","RvR>"};
		for (int w=0; w < 4; ++w)
		if (ovL[3-w] && ovR[3-w] && ovS[3-w])
		{	
			float slide = /*-1.f*/0.f, sLong = 0.f, sLat = 0.f;
			float squeal = pCar->GetTireSquealAmount((WHEEL_POSITION)w, &slide, &sLong, &sLat);

			//MATHVECTOR <float,3> vwhVel = pCar->dynamics.GetWheelVelocity((WHEEL_POSITION)w);
			//float whVel = vwhVel.Magnitude() * 3.6f;

			/**  //  info
			sprintf(ss, "%s %6.3f %6.3f  %6.3f %6.3f\n", swh[w],
				sLong/4.f, sLat/3.f, slide, squeal);
			ColourValue clr;  clr.setHSB( slide/20.f, 0.8f, 1.f );  //clr.a = min(0.f, slide/2.f);
			ovL[3-w]->setCaption(String(ss));
			ovL[3-w]->setColour(clr);
			//ovL[3-w]->setPosition(0.f, 230 + w*22);
			/**/

			//  bar meters |
			float susp = pCar->dynamics.GetSuspension(WHEEL_POSITION(w)).GetDisplacementPercent();
			float slng = sLong/abs(sLong)*powf(abs(sLong),0.3f);  // slide*20.f

			ovR[3-w]->setPosition(slng * 14.f +xp, yp + w*ln);
			ovL[3-w]->setPosition(sLat * 14.f +xp, yp + w*ln +y4);
			ovS[3-w]->setPosition(susp * 70.f +xp, yp + w*ln -y4);
		}
		if (ovL[4])  ovL[4]->setPosition(xp, yp + -20 +y4+3);
		if (ovS[4])  ovS[4]->setPosition(xp + 70, yp + -20 -104-3);

		//ovR[3-w]->setCaption("|");  ovR[3-w]->setColour(ColourValue(0.6,1.0,0.7));
	}

	//  checkpoint warning  --------
	if (road && hudCheck)
	{	/* chks info *
		sprintf(s, "st %d in%2d cur%2d nxt %d  num %d / all %d  %s" //"st-d %6.2f %6.2f %6.2f"
			,bInSt ? 1:0, iInChk, iCurChk, iNextChk,  iNumChks, road->mChks.size()
			,bWrongChk ? "Wrong Checkpoint" : ""
			);//,vStDist.x, vStDist.y, vStDist.z);
		hudCheck->setCaption(s);/**/

		static int showO = -1;  static float fChkTime = 0.f;
		if (bWrongChk)  fChkTime = 2.f;  //par sec
		int show = fChkTime > 0.f ? 1 : 0;
		if (show)  fChkTime -= time;
		if (show != showO)
			hudCheck->setCaption(show ? "Wrong Checkpoint" : "");
		showO = show;
	}


	//  tire params  --------
	#if 0
	if (ovU[0])
	{
		String ss = "";
		ss += "--Lateral--\n";
		for (int a=0; a < pCar->dynamics.tire[0].transverse_parameters.size(); ++a)
			ss += "a" + toStr(a) + " " + toStr( (Real)pCar->dynamics.tire[0].transverse_parameters[a] ) + "\n";
			
		//s += "a" + toStr(pCar->dynamics.tire[0].sigma_hat alpha_hat) + "\n";
		//pCar->dynamics.tire[0].

		//std::vector <T> longitudinal_parameters;
		//std::vector <T> transverse_parameters;
		//std::vector <T> aligning_parameters;
		
		ovU[0]->setCaption(ss);
	}
	#endif

    //update lap, place, race
	/*timer.GetStagingTimeLeft(), timer.GetPlayerCurrentLap(), race_laps, curplace.first, curplace.second,
		car.GetEngineRedline(), car.GetEngineRPMLimit(), car.GetSpeedometer(), settings->GetMPH(),
		debug_info1.str(), debug_info2.str(), debug_info3.str(), debug_info4.str(),/**/


	///  debug text  --------
	/*const MATHVECTOR <double,3> pos = pCar->dynamics.body.GetPosition();
	char ss[202];
	sprintf(ss,
		//"kmh:%5.1f rpm:%5d gear:%d  "
		"Pos: %5.2f %5.2f %5.2f"
		//,vel, pCar->GetEngineRPM(), gear
		,pos[0], pos[1], pos[2]
		);
	mDebugText = String(ss);	/**/

	/** char s[256];  // wheel ray
	for (int i=0; i < 4; i++)
	{
		sprintf(s, "c %6.3f",
			pCar->GetWheelContact(WHEEL_POSITION(i)).GetDepth() - pCar->GetTireRadius(WHEEL_POSITION(i)));
		ovL[i]->setCaption(String(s));
	}
	/**/
	
	//  wheels ter mtr info
	#if 0
	//if (iBlendMaps > 0)
	{
		String ss = "";
		static char s_[512];

		for (int i=0; i<4; ++i)
		{
			int mtr = whTerMtr[i];
			TRACKSURFACE* tsu = pCar->dynamics.terSurf[mtr];
			mtr = max(0, min( (int)(sc.td.layers.size())-1, mtr-1));
			TerLayer& lay = mtr == 0 ? sc.td.layerRoad : sc.td.layersAll[sc.td.layers[mtr]];

			sprintf(s_,  //"c %6.2f  "
				"R%d t%d  %s  [%s]  %s  \n"
				"  %4.0f  fr %4.2f / %4.2f  ba %4.2f  bw %4.2f \n"
				//"  d %4.2f m %4.2f ds %3.1f"	//". r%4.2f g%4.2f b%4.2f a%3.1f \n"
				//,pCar->dynamics.GetWheelContact(WHEEL_POSITION(i)).GetDepth() - 2*pCar->GetTireRadius(WHEEL_POSITION(i))
				,pCar->dynamics.bWhOnRoad[i], mtr, /*whOnRoad[i]?1:0,*/ lay.texFile.c_str()
				,tsu ? tsu->name.c_str() : "-", tsu ? csTRKsurf[tsu->type] : "-"
				,tsu ? tsu->rollingDrag : 0
				,tsu ? tsu->frictionTread : 0	,tsu ? tsu->frictionNonTread : 0
				,tsu ? tsu->bumpAmplitude : 0	,tsu ? tsu->bumpWaveLength : 0
				//,lay.dust, lay.mud, lay.dustS	//,lay.tclr.r, lay.tclr.g, lay.tclr.b, lay.tclr.a
				//,pCar->dynamics.wheel_contact[i].depth, pCar->dynamics.wheel_contact[i].col
				);
			ss += String(s_);
		}

		//  surfaces  info
		/*ss += "\n";
		for (int i=0; i < pGame->track.tracksurfaces.size(); ++i)
			ss += String(pGame->track.tracksurfaces[i].name.c_str()) + "\n";/**/

		//ovCarDbg->show();
		if (ovS[4])  {  //ovL[4]->setTop(400);
			ovS[4]->setColour(ColourValue::Black);
			ovS[4]->setCaption(ss);  }
	}
	#endif
}
