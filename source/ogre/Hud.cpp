#include "pch.h"
#include "Defines.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "SplitScreen.h"
#include "common/RenderConst.h"

#include <OgreRenderWindow.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>
#include <OgreManualObject.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreOverlayManager.h>
#include <OgreOverlayElement.h>
using namespace Ogre;

#include <MyGUI_PointerManager.h>
using namespace MyGUI;


///---------------------------------------------------------------------------------------------------------------
///  HUD create  rpm, vel
///---------------------------------------------------------------------------------------------------------------

void App::SizeHUD(bool full, Viewport* vp, int carId)
{
	// half size for 3 players top and 2 players in -horizontal-
	float mult = 1.f;
	if (mSplitMgr && !pSet->split_vertically &&
		mSplitMgr->mNumViewports == 2 || mSplitMgr->mNumViewports == 3 && carId == 2)
		mult = 0.5f;

	float fHudScale = pSet->size_gauges * mult;
	if (vp)
		asp = float(vp->getActualWidth())/float(vp->getActualHeight());
	else
		asp = float(mWindow->getWidth())/float(mWindow->getHeight());
	
	Real spx = fHudScale*1.1, spy = spx*asp;
	xcRpm = -1 + spx;  ycRpm = -1 + spy;
	xcVel =  1 - spx;  ycVel = -1 + spy;

	if (!full)
		return;
	if (nrpmB && nvelBk && nvelBm && nrpm &&nvel)
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
		float fHudSize = pSet->size_minimap * mult;
		ndMap->setScale(fHudSize,fHudSize*asp,1);

		const float marg = 1.f + 0.05f;  // from border
		fMiniX = 1 - fHudSize * marg, fMiniY = 1 - fHudSize*asp * marg;

		ndMap->setPosition(Vector3(fMiniX,fMiniY,0));
	}
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
		float size = std::max(fMapSizeX, fMapSizeY*asp);
		scX = 1.f / size;  scY = 1.f / size;

		//for (int c=0; c<2; ++c)
		String sMat = "circle_minimap";  //*/"road_minimap_inv";
		asp = 1.f;  //_temp
		ManualObject* m = Create2D(sMat,mSplitMgr->mHUDSceneMgr,1,true,true);  miniC = m;
		//asp = float(mWindow->getWidth())/float(mWindow->getHeight());
		m->setVisibilityFlags(RV_Hud);  m->setRenderQueueGroup(RQG_Hud1);
		
		///  change minimap image
		MaterialPtr mm = MaterialManager::getSingleton().getByName(sMat);
		Pass* pass = mm->getTechnique(0)->getPass(0);
		TextureUnitState* tus = pass->getTextureUnitState(0);
		if (tus)  tus->setTextureName(pSet->game.track + "_mini"+DEFAULT_TEXTURE_EXTENSION);
		tus = pass->getTextureUnitState(2);
		if (tus)  tus->setTextureName(pSet->game.track + "_ter"+SECONDARY_TEXTURE_EXTENSION);
		UpdMiniTer();
		

		float fHudSize = pSet->size_minimap;
		const float marg = 1.f + 0.05f;  // from border
		fMiniX = 1 - fHudSize * marg, fMiniY = 1 - fHudSize*asp * marg;

		ndMap = mSplitMgr->mHUDSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(fMiniX,fMiniY,0));
		ndMap->scale(fHudSize, fHudSize*asp, 1);
		ndMap->attachObject(m);
		
		//  car pos dot - for all carModels (ghost and remote too)
		vMoPos.clear();
		vNdPos.clear();
		for (int i=0; i < /*pSet->local_players*/carModels.size(); ++i)
		{	vMoPos.push_back(0);
			vMoPos[i] = Create2D("hud/CarPos", mSplitMgr->mHUDSceneMgr, 0.4f, true, true);
			vMoPos[i]->setVisibilityFlags(RV_Hud);  vMoPos[i]->setRenderQueueGroup(RQG_Hud3);
			vNdPos.push_back(0);
			vNdPos[i] = ndMap->createChildSceneNode();
			vNdPos[i]->scale(fHudSize*1.5f, fHudSize*1.5f, 1);
			vNdPos[i]->attachObject(vMoPos[i]);  /*vNdPos[i]->setVisible(false);  */}
		ndMap->setVisible(false/*pSet->trackmap*/);
	}

	
	//  backgr  gauges
	ManualObject* mrpmB = Create2D("hud/rpm",mSplitMgr->mHUDSceneMgr,1);	mrpmB->setVisibilityFlags(RV_Hud);
	mrpmB->setRenderQueueGroup(RQG_Hud1);
	nrpmB = mSplitMgr->mHUDSceneMgr->getRootSceneNode()->createChildSceneNode();
	nrpmB->attachObject(mrpmB);	nrpmB->setScale(0,0,0);  nrpmB->setVisible(false);

	ManualObject* mvelBk = Create2D("hud/kmh",mSplitMgr->mHUDSceneMgr,1);	mvelBk->setVisibilityFlags(RV_Hud);
	mvelBk->setRenderQueueGroup(RQG_Hud1);
	nvelBk = mSplitMgr->mHUDSceneMgr->getRootSceneNode()->createChildSceneNode();
	nvelBk->attachObject(mvelBk);	nvelBk->setScale(0,0,0);  mvelBk->setVisible(false);
		
	ManualObject* mvelBm = Create2D("hud/mph",mSplitMgr->mHUDSceneMgr,1);	mvelBm->setVisibilityFlags(RV_Hud);
	mvelBm->setRenderQueueGroup(RQG_Hud1);
	nvelBm = mSplitMgr->mHUDSceneMgr->getRootSceneNode()->createChildSceneNode();
	nvelBm->attachObject(mvelBm);	nvelBm->setScale(0,0,0);  mvelBm->setVisible(false);
		
	//  needles
	mrpm = Create2D("hud/needle",mSplitMgr->mHUDSceneMgr,1,true);  mrpm->setVisibilityFlags(RV_Hud);
	mrpm->setRenderQueueGroup(RQG_Hud3);
	nrpm = mSplitMgr->mHUDSceneMgr->getRootSceneNode()->createChildSceneNode();
	nrpm->attachObject(mrpm);	nrpm->setScale(0,0,0);	nrpm->setVisible(false);
	
	mvel = Create2D("hud/needle",mSplitMgr->mHUDSceneMgr,1,true);  mvel->setVisibilityFlags(RV_Hud);
	mvel->setRenderQueueGroup(RQG_Hud3);
	nvel = mSplitMgr->mHUDSceneMgr->getRootSceneNode()->createChildSceneNode();
	nvel->attachObject(mvel);	nvel->setScale(0,0,0);	nvel->setVisible(false);


	//  overlays
	OverlayManager& ovr = OverlayManager::getSingleton();
	ovCam = ovr.getByName("Car/CameraOverlay");

	ovGear = ovr.getByName("Hud/Gear");		hudGear = ovr.getOverlayElement("Hud/GearText");
	ovVel = ovr.getByName("Hud/Vel");		hudVel = ovr.getOverlayElement("Hud/VelText");
	ovBoost = ovr.getByName("Hud/Boost");	hudBoost = ovr.getOverlayElement("Hud/BoostText");
	ovAbsTcs = ovr.getByName("Hud/AbsTcs");	hudAbs = ovr.getOverlayElement("Hud/AbsText");
	ovCarDbg = ovr.getByName("Car/Stats");	hudTcs = ovr.getOverlayElement("Hud/TcsText");

	ovCountdown = ovr.getByName("Hud/Countdown");	hudCountdown = ovr.getOverlayElement("Hud/CountdownText");
	ovTimes = ovr.getByName("Hud/Times");	hudTimes = ovr.getOverlayElement("Hud/TimesText");
	ovOpp = ovr.getByName("Hud/Opponents"); hudOppB = ovr.getOverlayElement("Hud/OpponentsPanel");
	for (int o=0; o < 5; ++o)  for (int c=0; c < 3; ++c)  {
		hudOpp[o][c] = ovr.getOverlayElement("Hud/OppText"+toStr(o)+"_"+toStr(c));  hudOpp[o][c]->setCaption("");  }
	
	for (int o=0; o < carModels.size(); ++o)  // fill car names, not changed during play
	{
		const CarModel* cm = carModels[o];
		if (cm->eType != CarModel::CT_REPLAY)
		{
			hudOpp[o][2]->setCaption(cm->sDispName);
			hudOpp[o][2]->setColour(cm->color);
		}
	}

	ovWarnWin = ovr.getByName("Hud/WarnAndWin");
	hudWarnChk = ovr.getOverlayElement("Hud/Warning");
	hudWarnChk->setCaption(String(TR("#{WrongChk}")));
	hudWonPlace = ovr.getOverlayElement("Hud/WonPlace");

	//  dbg lines
	ovCarDbgTxt = ovr.getByName("Car/StatsTxt");  //ovCarDbgTxt->show();
	ovCarDbg = ovr.getByName("Car/Stats");  //ovCarDbg->show();  // bars
	for (int i=0; i < 5; ++i)
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
	{	if (nrpmB)  nrpmB->setVisible(false);
		if (nvelBk)	nvelBk->setVisible(false);	if (nvelBm)	nvelBm->setVisible(false);
		if (nrpm)	nrpm->setVisible(false);	if (nvel)	nvel->setVisible(false);
		if (ovGear)	  ovGear->hide();		if (ovVel)	  ovVel->hide();
		if (ovAbsTcs) ovAbsTcs->hide();
		if (ovBoost)  ovBoost->hide();		//if (hudBoost)  hudBoost->hide();
		if (ovCountdown)  ovCountdown->hide();
		if (hudGear)  hudGear->hide();		if (hudVel)   hudVel->hide();
		if (ovCarDbg)  ovCarDbg->hide();	if (ovCarDbgTxt)  ovCarDbgTxt->hide();

		if (ovCam)	 ovCam->hide();			if (ovTimes)  ovTimes->hide();
		if (ovWarnWin)  ovWarnWin->hide();	if (ovOpp)  ovOpp->hide();
		if (mFpsOverlay)  mFpsOverlay->hide();
		if (ndMap)  ndMap->setVisible(false);
		if (mGUI)	PointerManager::getInstance().setVisible(false);
		if (mWndRpl)  mWndRpl->setVisible(false);
	}else{
		bool show = pSet->show_gauges;
		if (nrpmB)  nrpmB->setVisible(show);
		if (nvelBk)	nvelBk->setVisible(show && !pSet->show_mph);
		if (nvelBm)	nvelBm->setVisible(show && pSet->show_mph);
		if (nrpm)	nrpm->setVisible(show);		if (nvel)	nvel->setVisible(show);
		if (ovGear)	{  if (1||show)  ovGear->show();  else  ovGear->hide();  }
		if (ovVel)	{  if (1||show)  ovVel->show();   else  ovVel->hide();   }
		if (ovBoost){  if (show && (pSet->game.boost_type == 1 || pSet->game.boost_type == 2))
									ovBoost->show();    else  ovBoost->hide();  }
		if (ovCountdown)  if (show)  ovCountdown->show();  else  ovCountdown->hide();
		if (ovAbsTcs){ if (show)  ovAbsTcs->show();   else  ovAbsTcs->hide(); }
		if (hudGear){  if (pSet->show_digits)  hudGear->show(); else  hudGear->hide();  }
		if (hudVel) {  if (pSet->show_digits)  hudVel->show();  else  hudVel->hide();  }

		show = pSet->car_dbgbars;
		if (ovCarDbg){  if (show)  ovCarDbg->show();  else  ovCarDbg->hide();   }
		show = pSet->car_dbgtxt || pSet->bltProfilerTxt;
		if (ovCarDbgTxt){  if (show)  ovCarDbgTxt->show();  else  ovCarDbgTxt->hide();   }
		//for (int i=0; i<5; ++i)
		//{	if (ovU[i])  if (show)  ovU[i]->show();  else  ovU[i]->hide();  }

		if (ovCam)	{  if (pSet->show_cam && !isFocGui)    ovCam->show();    else  ovCam->hide();     }
		if (ovTimes){  if (pSet->show_times)  ovTimes->show();  else  ovTimes->hide();   }
		if (ovOpp)  {  if (pSet->show_opponents && road && road->getNumPoints() > 0)  ovOpp->show();  else  ovOpp->hide();   }
		if (ovWarnWin){  if (pSet->show_times)  ovWarnWin->show();  else  ovWarnWin->hide();  }
		if (mFpsOverlay) { if (pSet->show_fps) mFpsOverlay->show(); else mFpsOverlay->hide(); }
		if (ndMap)  ndMap->setVisible(pSet->trackmap);
		if (mGUI)	PointerManager::getInstance().setVisible(isFocGuiOrRpl());
		if (mWndRpl && !bLoading)  mWndRpl->setVisible(bRplPlay && bRplWnd);  //
	}
}

void App::UpdMiniTer()
{
	MaterialPtr mm = MaterialManager::getSingleton().getByName("circle_minimap");
	Pass* pass = mm->getTechnique(0)->getPass(0);
	if (!pass)  return;
	try
	{	Ogre::GpuProgramParametersSharedPtr fparams = pass->getFragmentProgramParameters();
		if(fparams->_findNamedConstantDefinition("showTerrain",false))
		{
			fparams->setNamedConstant("showTerrain", pSet->mini_terrain ? 1.f : 0.f);
		}
	}catch(...){  }
}


//  Update HUD
///---------------------------------------------------------------------------------------------------------------
bool SortPerc(const CarModel* cm2, const CarModel* cm1)
{
	return cm1->trackPercent < cm2->trackPercent;
}

void App::UpdateHUD(int carId, CarModel* pCarM, CAR* pCar, float time, Viewport* vp)
{
	if (bSizeHUD)
	{	bSizeHUD = false;  // split x num plr ?..
		SizeHUD(true);	}
		
	// show/hide for render viewport / gui viewport
	// first show everything
	ShowHUD(false);
	// now hide things we dont want
	if (!vp)
	{
		/// for gui viewport ----------------------
		if (hudGear)  hudGear->hide();		if (hudVel)  hudVel->hide();		if (ovBoost)  ovBoost->hide();
		if (ovTimes)  ovTimes->hide();		if (ovWarnWin)  ovWarnWin->hide();	if (ovOpp)  ovOpp->hide();
		if (ovCarDbg)  ovCarDbg->hide();	if (ovCarDbgTxt)  ovCarDbgTxt->hide();
		if (ovCountdown)  ovCountdown->hide();
	}else{
		/// for render viewport ---------
		if (ovCam)  ovCam->hide();
		if (mFpsOverlay)  mFpsOverlay->hide();
	}
	//*H*/LogO(String("car: ") + toStr(carId) +" "+ (!pCarM ? toStr(pCarM) : pCarM->sDirname) +" "+ (vp?"v+":"v-") +" "+ (pCar?"c+":"c-") +" "+ (pCarM?"m+":"m-"));
	if (!pCar)  return;  //-1 for ^above hiding in gui vp

			
	///  hud rpm,vel  --------------------------------
	if (pCar && !bRplPlay)  // for local cars only..
	{	fr.vel = pCar->GetSpeedometer();
		fr.rpm = pCar->GetEngineRPM();
		fr.gear = pCar->GetGear();
		fr.clutch = pCar->GetClutch();
		//fr.throttle = pCar->dynamics.GetEngine().GetThrottle();  // not on hud
	}

    float vel = fr.vel * (pSet->show_mph ? 2.23693629f : 3.6f);
    float rpm = fr.rpm;
	//*H*/LogO(String("car: ") + toStr(carId) + "  vel: "+ toStr(vel) +"  rpm: "+ toStr(rpm));
	UpdHUDRot(carId, pCarM, vel, rpm);


	///  update remote players  -----------
	for (int i=0; i < carModels.size(); ++i)
	{
		if (carModels[i]->eType == CarModel::CT_GHOST)
			UpdHUDRot(i, carModels[i], 0.f, 0.f, true);
		else
		if (carId == 0 && !pSet->mini_rotated && !pSet->mini_zoomed)  // the only fall that works
		if (carModels[i]->eType != CarModel::CT_LOCAL && carModels[i]->eType != CarModel::CT_REPLAY)
			UpdHUDRot(i, carModels[i], 0.f, 0.f, true);
		
	}
			

	///  opponents list
	//------------------------------------------------------------------------
	if (ovOpp->isVisible() && pCarM && pCarM->pMainNode)
	{
		std::list<CarModel*> cms;  // sorted list
		for (int o=0; o < carModels.size(); ++o)
			cms.push_back(carModels[o]);
		cms.sort(SortPerc);

		ColourValue c;  char ss[128];
		int o = 0;
		for (std::list<CarModel*>::const_iterator it = cms.begin(); it != cms.end(); ++it, ++o)
		if (hudOpp[o][0])
		{
			CarModel* cm = *it;
			if (cm->eType != CarModel::CT_REPLAY && cm->pMainNode)
			{
				cm->UpdTrackPercent();

				bool bGhost = cm->eType == CarModel::CT_GHOST;
				bool bGhostVis = (ghplay.GetNumFrames() > 0) && pSet->rpl_ghost;

				if (cm == pCarM || bGhost && !bGhostVis)  // no dist to self or to empty ghost
					hudOpp[o][1]->setCaption("");
				else
				{	Vector3 v = cm->pMainNode->getPosition() - pCarM->pMainNode->getPosition();
					float dist = v.length();  // meters, mph:feet?
					//  dist m
					sprintf(ss, "%3.0fm", dist);		hudOpp[o][1]->setCaption(ss);
					Real h = std::min(60.f, dist) / 60.f;
					c.setHSB(0.5f - h * 0.4f, 1,1);		hudOpp[o][1]->setColour(c);
				}
					
				if (!bGhost)  // todo: save perc for ghost/replay or upd it as for regular car..
				{	//  percent %
					sprintf(ss, "%3.0f%%", cm->trackPercent);		hudOpp[o][0]->setCaption(ss);
					c.setHSB(cm->trackPercent*0.01f*0.4f, 0.7f,1);	hudOpp[o][0]->setColour(c);
				}
				else  hudOpp[o][0]->setCaption("");
				
				hudOpp[o][2]->setCaption(cm->sDispName);  // cant sort if-, name once in CreateHUD
				hudOpp[o][2]->setColour(cm->color);
			}
		}
	}


	//   Set motion blur intensity for this viewport, depending on car's linear velocity
	// -----------------------------------------------------------------------------------
	if (pSet->motionblur)
	{
		// use velocity squared to achieve an exponential motion blur - and its faster too - wow :)
		float speed = pCar->GetVelocity().MagnitudeSquared();
		
		// peak at 250 kmh (=69 m/s), 69² = 4761
		// motion blur slider: 1.0 = peak at 100 km/h   0.0 = peak at 400 km/h   -> 0.5 = peak at 250 km/h
		// lerp(100, 400, 1-motionBlurIntensity)
		float peakSpeed = 100 + (1-pSet->motionblurintensity) * (400-100);
		float motionBlurAmount = std::abs(speed) / pow((peakSpeed/3.6f), 2);
		
		// higher fps = less perceived motion blur time a frame will be still visible on screen:
		// each frame, 1-motionBlurAmount of the original image is lost
		// example (motionBlurAmount = 0.7):
		//	   frame 1: full img		   frame 2: 0.7  * image
		//	   frame 3: 0.7² * image	   frame 4: 0.7³ * image
		// portion of image visible after 'n' frames: pow(motionBlurAmount, n);
		//	   example 1: 60 fps	   0.7³ image after 4 frames: 0.066 sec
		//	   example 2: 120 fps	   0.7³ image after 4 frames: 0.033 sec
		// now: need to achieve *same* time for both fps values
		// to do this, adjust motionBlurAmount
		// (1.0/fps) * pow(motionBlurAmount, n) == (1.0/fps2) * pow(motionBlurAmount2, n)
		// set n=4  motionBlurAmount_new = sqrt(sqrt((motionBlurAmount^4 * fpsReal/desiredFps))
		motionBlurAmount = sqrt(sqrt( pow(motionBlurAmount, 4) * ((1.0f/time) / 120.0f) ));
			
		// clamp to 0.9f
		motionBlurAmount = std::min(motionBlurAmount, 0.9f);
		
		motionBlurIntensity = motionBlurAmount;
	}


	///  gear, vel texts  -----------------------------
	if (hudGear && hudVel && pCar)
	{
		char cg[132],sv[132];  cg[0]='1'; cg[1]=0; sv[1]=0;
		float cl = fr.clutch*0.8f + 0.2f;
		if (fr.gear == -1)
		{	cg[0]='R';  hudGear->setColour(ColourValue(0.3,1,1,cl));	}
		else if (fr.gear == 0)
		{	cg[0]='N';  hudGear->setColour(ColourValue(0.3,1,0.3,cl));	}
		else if (fr.gear > 0 && fr.gear < 8)
		{	cg[0]='0'+fr.gear;  hudGear->setColour(ColourValue(1,1-fr.gear*0.1,0.2,cl));	}

		sprintf(sv, "%3.0f", std::abs(vel));
		hudGear->setCaption(String(cg));
		hudVel->setCaption(String(sv));  int w = mWindow->getWidth();
		hudVel->setPosition(-0.055 + w/1600.f*0.045,-0.01);
		//hudVel->setPosition(-0.1 + (w-1024.f)/1600.f*0.07/*0.11*/,-0.01);

		float k = pCar->GetSpeedometer() * 3.6f * 0.0025f;	// vel clr
		#define m01(x)  std::min(1.0, std::max(0.0, (double) x))
		hudVel->setColour(ColourValue(m01(k*2), m01(0.5+k*1.5-k*k*2.5), m01(1+k*0.8-k*k*3.5)));
	}

	//  boost fuel (time)  -----------------------------
	char sb[132];
	if (hudBoost && pCar && hudBoost->isVisible())
	{
		sprintf(sb, "%3.1f", pCar->dynamics.boostFuel);
		hudBoost->setCaption(String(sb));
	}

	//  race countdown  -----------------------------
	if (hudCountdown)
	{
		if (pGame->timer.pretime > 0.f && !pGame->timer.waiting)
		{
			sprintf(sb, "%3.1f", pGame->timer.pretime);
			hudCountdown->setCaption(String(sb));
			hudCountdown->show();
		}else
			hudCountdown->hide();
	}
	
	//  abs, tcs on  -----------------------------
	if (hudAbs && hudTcs)
	{
		// hide on gui vp
		if (!vp)
		{	hudAbs->hide();  hudTcs->hide();  }
		else
		if (pCar)
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
	}
	
	///  times, score  -----------------------------
	if (pSet->show_times && pCar)
	{
		TIMER& tim = pGame->timer;	//car[playercarindex].
		tim.SetPlayerCarID(carId);
		s[0]=0;
		
		if (pCarM->bWrongChk || pSet->game.local_players > 1 && pCarM->iWonPlace > 0)
			ovWarnWin->show();  else  ovWarnWin->hide();  //ov
			
		//  lap num (for many or champ)
		if (pSet->game.local_players > 1 || pSet->game.champ_num >= 0)
		{
			if (pCarM->iWonPlace > 0 && hudWonPlace)
			{	sprintf(s, String(TR("---  %d #{TBPlace}  ---")).c_str(), pCarM->iWonPlace );
				hudWonPlace->setCaption(s);  hudWonPlace->show();
				const static ColourValue clrPlace[4] = {
					ColourValue(0.4,1,0.2), ColourValue(1,1,0.3), ColourValue(1,0.7,0.2), ColourValue(1,0.5,0.2) };
				hudWonPlace->setColour(clrPlace[pCarM->iWonPlace-1]);
			}
			sprintf(s, String(TR("#{TBLap}  %d/%d")).c_str(), tim.GetCurrentLap(carId)+1, pSet->game.num_laps );
		}else
		{	if (!road)  // score on vdr track
			if (tim.GetIsDrifting(0))
				sprintf(s, String(TR("#{TBScore}  %3.0f+%2.0f")).c_str(), tim.GetDriftScore(0), tim.GetThisDriftScore(0) );
			else
				sprintf(s, String(TR("#{TBScore}  %3.0f")).c_str(), tim.GetDriftScore(0) );
		}		
		if (hudTimes)
			hudTimes->setCaption(String(s) +
				String(TR("\n#{TBTime} ")) + GetTimeString(tim.GetPlayerTime())+
				String(TR("\n#{TBLast} ")) + GetTimeString(tim.GetLastLap())+
				String(TR("\n#{TBBest} ")) + GetTimeString(tim.GetBestLap(pSet->game.trackreverse)) );
	}

	//-------------------------------------------------------------------------------------------------------------------
	///  debug infos
	//-------------------------------------------------------------------------------------------------------------------

	//  car debug text  --------
	static bool oldCarTxt = false;
	if (pCar && ovU[0])
	{
		if (pSet->car_dbgtxt)
		{	std::stringstream s1,s2,s3,s4;
			pCar->DebugPrint(s1, true, false, false, false);  ovU[0]->setCaption(s1.str());
			pCar->DebugPrint(s2, false, true, false, false);  ovU[1]->setCaption(s2.str());
			pCar->DebugPrint(s3, false, false, true, false);  ovU[2]->setCaption(s3.str());
			pCar->DebugPrint(s4, false, false, false, true);  ovU[3]->setCaption(s4.str());
		}else
		if (pSet->car_dbgtxt != oldCarTxt)
		{	ovU[0]->setCaption(""); ovU[1]->setCaption(""); ovU[2]->setCaption(""); ovU[3]->setCaption("");		}
	}
	oldCarTxt = pSet->car_dbgtxt;
	

	//  profiling times -
	if (pGame && pGame->profilingmode && ovU[3])
	{
		if (pSet->multi_thr != 1) //! this one is causing a crash sometimes with multithreading.. maybe need a mutex? i have no idea..
			ovU[3]->setCaption(pGame->strProfInfo);
		//if (newPosInfos.size() > 0)
		//ovU[3]->setCaption("carm: " + toStr(carModels.size()) + " newp: " + toStr((*newPosInfos.begin()).pos));
	}

	//  bullet profiling text  --------
	static bool oldBltTxt = false;
	if (ovU[1])
	{
		if (pSet->bltProfilerTxt)
			ovU[1]->setCaption(pGame->collision.bltProfiling);
		else
		if (pSet->bltProfilerTxt != oldBltTxt)
			ovU[1]->setCaption("");
	}
	oldBltTxt = pSet->bltProfilerTxt;

	
	//  wheels slide, susp bars  --------
	if (pSet->car_dbgbars && pCar)
	{
		const Real xp = 80, yp = -530, ln = 20, y4 = 104;
		static char ss[256];
		//const static char swh[4][6] = {"F^L<","F^R>","RvL<","RvR>"};
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
	if (road && hudWarnChk && pCarM)
	{	/* checks info *
		if (ovU[0])
		//	"ghost:  "  + GetTimeString(ghost.GetTimeLength()) + "  "  + toStr(ghost.GetNumFrames()) + "\n" +
		//	"ghplay: " + GetTimeString(ghplay.GetTimeLength()) + "  " + toStr(ghplay.GetNumFrames()) + "\n" +
		{	sprintf(s, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
				"         st %d in%2d  |  cur%2d > next %d  |  Num %d / All %d"
			,pCarM->bInSt ? 1:0, pCarM->iInChk, pCarM->iCurChk, pCarM->iNextChk
			,pCarM->iNumChks, road->mChks.size());
			ovU[0]->setCaption(s);
		}	/**/

		if (pCarM->bWrongChk)
			pCarM->fChkTime = 2.f;  //par sec
		int show = pCarM->fChkTime > 0.f ? 1 : 0;
		if (show)  pCarM->fChkTime -= time;
		//if (show != pCarM->iChkWrong)  //-
		bool place = pSet->game.local_players > 1, won = pCarM->iWonPlace > 0;
			if (show)  {  hudWarnChk->show();  if (place && !won)  hudWonPlace->hide();  }
			else  {       hudWarnChk->hide();  if (place && won)  hudWonPlace->show();  }
		pCarM->iChkWrong = show;
	}


	//  tire params  --------
	#if 0
	if (ovU[0] && pCar)
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

	//  input values
	/*if (pCar && pGame && pGame->profilingmode)
	{	const std::vector<float>& inp = pCar->dynamics.inputsCopy;
	if (ovU[2] && inp.size() == CARINPUT::ALL)
	{	sprintf(s, 
		" Throttle %5.2f\n Brake %5.2f\n Steer %5.2f\n"
		" Handbrake %5.2f\n Boost %5.2f\n Flip %5.2f\n"
		,inp[CARINPUT::THROTTLE], inp[CARINPUT::BRAKE], -inp[CARINPUT::STEER_LEFT]+inp[CARINPUT::STEER_RIGHT]
		,inp[CARINPUT::HANDBRAKE],inp[CARINPUT::BOOST], inp[CARINPUT::FLIP] );
		ovU[2]->setCaption(String(s));
	}	}/**/

    //update lap, place, race
	/*timer.GetStagingTimeLeft(), timer.GetPlayerCurrentLap(), race_laps, curplace.first, curplace.second,
		car.GetEngineRedline(), car.GetEngineRPMLimit(), car.GetSpeedometer(), settings->GetMPH(),
		debug_info1.str(), debug_info2.str(), debug_info3.str(), debug_info4.str(),/**/


	///  debug text  --------
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
			mtr = std::max(0, std::min( (int)(sc.td.layers.size())-1, mtr-1));
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
