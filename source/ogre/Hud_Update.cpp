#include "pch.h"
#include "common/Defines.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "../vdrift/quickprof.h"
#include "../road/Road.h"
#include "SplitScreen.h"
#include "common/RenderConst.h"
#include "common/MultiList2.h"
#include "common/GraphView.h"

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
using namespace MyGUI;


//  HUD utils
//---------------------------------------------------------------------------------------------------------------

bool SortPerc(const CarModel* cm2, const CarModel* cm1)
{
	int l1 = cm1->pGame->timer.GetCurrentLap(cm1->iIndex);
	int l2 = cm2->pGame->timer.GetCurrentLap(cm2->iIndex);
	float p1 = cm1->trackPercent;
	float p2 = cm2->trackPercent;
	return (l1 < l2) || (l1 == l2 && p1 < p2);
}

bool SortWin(const CarModel* cm2, const CarModel* cm1)
{
	int l1 = cm1->pGame->timer.GetCurrentLap(cm1->iIndex);
	int l2 = cm2->pGame->timer.GetCurrentLap(cm2->iIndex);
	float t1 = cm1->pGame->timer.GetPlayerTimeTot(cm1->iIndex);
	float t2 = cm2->pGame->timer.GetPlayerTimeTot(cm2->iIndex);
	return (l1 < l2) || (l1 == l2 && t1 > t2);
}


void App::GetHUDVals(int id, float* vel, float* rpm, float* clutch, int* gear)
{
	const CarModel* pCarM = carModels[id];
	const CAR* pCar = pCarM ? pCarM->pCar : 0;

	if (pCar && !bRplPlay && pCarM->eType != CarModel::CT_GHOST)
	{	*vel = pCar->GetSpeedometer() * (pSet->show_mph ? 2.23693629f : 3.6f);
		*rpm = pCar->GetEngineRPM();  *gear = pCar->GetGear();
		//*clutch = pCar->GetClutch();  // todo: problems in multi thr1
	}
	if (bRplPlay)
	{
		*vel = frm[id].vel * (pSet->show_mph ? 2.23693629f : 3.6f);
		*rpm = frm[id].rpm;  *gear = frm[id].gear;
	}
}

///---------------------------------------------------------------------------------------------------------------
//  Update HUD
///---------------------------------------------------------------------------------------------------------------
void App::UpdateHUD(int carId, float time)
{
	PROFILER.beginBlock("g.hud");

	if (bSizeHUD)	// update sizes once after change
	{	bSizeHUD = false;
		SizeHUD(true);	}

	
	//  update HUD elements for all cars that have a viewport (local or replay)
	//-----------------------------------------------------------------------------------
	if (carId == -1)  // gui vp - done once for all
	for (int c = 0; c < carModels.size(); ++c)
	if (carModels[c]->eType == CarModel::CT_LOCAL)
	{
		//  hud rpm,vel
		float vel=0.f, rpm=0.f, clutch=1.f;  int gear=1;
		GetHUDVals(c,&vel,&rpm,&clutch,&gear);
		
		//  update pos tri on minimap  (all)
		for (int i=0; i < carModels.size(); ++i)
			UpdHUDRot(c, i, vel, rpm);
	}

	if (carId == -1 || carModels.size()==0)
	{
		PROFILER.endBlock("g.hud");
		return;
	}

	CarModel* pCarM = carModels[carId];
	CAR* pCar = pCarM ? pCarM->pCar : 0;

	float vel=0.f, rpm=0.f, clutch=1.f;  int gear=1;
	GetHUDVals(carId,&vel,&rpm,&clutch,&gear);


	///  multiplayer
	// -----------------------------------------------------------------------------------
	static float tm = 0.f;  tm += time;
	if (tm > 0.2f /**/&& mClient/**/)  // not every frame, each 0.2s
	// if (pSet->game.isNetw) ..
	{
		//  sort winners
		std::list<CarModel*> cms;
		for (int o=0; o < carModels.size(); ++o)
			cms.push_back(carModels[o]);

		cms.sort(SortWin);
		//stable_sort(cms.begin(), cms.end(), SortWin);
		
		String msg = "";  int place = 1;  // assing places
		for (std::list<CarModel*>::iterator it = cms.begin(); it != cms.end(); ++it)
		{
			CarModel* cm = *it;
			bool end = pGame->timer.GetCurrentLap(cm->iIndex) >= pSet->game.num_laps;
			cm->iWonPlace = end ? place++ : 0;  // when ended race

			//  detect change (won),  can happen more than once, if time diff < ping delay
			if (cm->iWonPlace != cm->iWonPlaceOld)
			{	cm->iWonPlaceOld = cm->iWonPlace;
				cm->iWonMsgTime = 4.f;  //par in sec
				if (cm->iIndex == 0)  // for local player, show end wnd
					mWndNetEnd->setVisible(true);
			}
			if (cm->iWonMsgTime > 0.f)
			{	cm->iWonMsgTime -= tm;
				if (cm->iWonPlace != 0)
					msg += cm->sDispName + " " + TR("#{FinishedCommaPlace}") + ": " + toStr(cm->iWonPlace) + "\n";
			}
		}
		if (mClient && /*pGame->timer.pretime <= 0.f &&*/ pGame->timer.waiting)
			msg += TR("#{NetWaitingForOthers}")+"...\n";
			
		//  chat 2 last lines
		if (sChatLast1 != "")	msg += sChatLast1 + "\n";
		if (sChatLast2 != "")	msg += sChatLast2;
			
		++iChatMove;
		if (iChatMove >= 10)  //par 2sec
		{	iChatMove = 0;
			sChatLast1 = sChatLast2;
			sChatLast2 = "";
		}
		
		//  upd hud msgs
		if (hudNetMsg)
		{	
			hudNetMsg->setCaption(msg);
			ovNetMsg->show();
		}

		//  upd end list
		if (mWndNetEnd->getVisible())
		{	liNetEnd->removeAllItems();
			for (std::list<CarModel*>::iterator it = cms.begin(); it != cms.end(); ++it)
			{
				CarModel* cm = *it;
				//String clr = "#E0F0FF";
				std::stringstream ss;  // car color to hex str
				ss << std::hex << std::setfill('0');
				ss << (cm->color.getAsARGB() & 0xFFFFFF);
				String clr = "#"+ss.str();

				liNetEnd->addItem(""/*clr+ toStr(c+1)*/, 0);  int l = liNetEnd->getItemCount()-1;
				liNetEnd->setSubItemNameAt(1,l, clr+ (cm->iWonPlace == 0 ? "--" : toStr(cm->iWonPlace)));
				liNetEnd->setSubItemNameAt(2,l, clr+ cm->sDispName);
				liNetEnd->setSubItemNameAt(3,l, clr+ GetTimeString( cm->iWonPlace == 0 ? 0.f : pGame->timer.GetPlayerTimeTot(cm->iIndex) ));
				//liNetEnd->setSubItemNameAt(4,l, clr+ fToStr(cm->iWonMsgTime,1,3));
				liNetEnd->setSubItemNameAt(4,l, clr+ GetTimeString( pGame->timer.GetBestLapRace(cm->iIndex) ));
				liNetEnd->setSubItemNameAt(5,l, clr+ toStr( pGame->timer.GetCurrentLap(cm->iIndex) ));
		}	}
		tm = 0.f;
	}


	///  opponents list
	// -----------------------------------------------------------------------------------
	if (ovOpp->isVisible() && pCarM && pCarM->pMainNode)
	{
		std::list<CarModel*> cms;  // sorted list
		for (int o=0; o < carModels.size(); ++o)
		{	// cars only
			if (carModels[o]->eType != CarModel::CT_GHOST)
			{	if (bRplPlay)
					carModels[o]->trackPercent = carPoses[iCurPoses[o]][o].percent;
				cms.push_back(carModels[o]);	}
		}
		if (pSet->opplist_sort)
			cms.sort(SortPerc);
		
		for (int o=0; o < carModels.size(); ++o)
		{	// add ghost last
			if (carModels[o]->eType == CarModel::CT_GHOST)
			{	carModels[o]->trackPercent = carPoses[iCurPoses[o]][o].percent;  // ghost,rpl
				cms.push_back(carModels[o]);	}
		}

		ColourValue clr;
		//if (carModels.size() == carPoses.size())  //-
		int o = 0;
		for (std::list<CarModel*>::iterator it = cms.begin(); it != cms.end(); ++it,++o)
		if (hudOpp[o][0])
		{
			CarModel* cm = *it;
			if (cm->pMainNode)
			{
				bool bGhost = cm->eType == CarModel::CT_GHOST;
				bool bGhostVis = (ghplay.GetNumFrames() > 0) && pSet->rpl_ghost;
				bool bGhEmpty = bGhost && !bGhostVis;

				if (!bGhost && cm->eType != CarModel::CT_REMOTE)
					cm->UpdTrackPercent();

				if (cm == pCarM || bGhEmpty)  // no dist to self or to empty ghost
					hudOpp[o][1]->setCaption("");
				else
				{	Vector3 v = cm->pMainNode->getPosition() - pCarM->pMainNode->getPosition();
					float dist = v.length();  // meters, mph:feet?
					//  dist m
					hudOpp[o][1]->setCaption(fToStr(dist,0,3)+"m");
					Real h = std::min(60.f, dist) / 60.f;
					clr.setHSB(0.5f - h * 0.4f, 1,1);		hudOpp[o][1]->setColour(clr);
				}
					
				if (bGhEmpty)
					hudOpp[o][0]->setCaption("");
				else
				{	//  percent % val
					float perc = cm->trackPercent;
					if (bGhost && pGame->timer.GetPlayerTime(0) > ghplay.GetTimeLength())
						perc = 100.f;  // force 100 at ghost end
					hudOpp[o][0]->setCaption(fToStr(perc,0,3)+"%");
					clr.setHSB(perc*0.01f*0.4f, 0.7f,1);	hudOpp[o][0]->setColour(clr);
				}
				
				///  Lap Time  pos (1)
				//if (mClient)
				{
					float t = 0.f;  int lap = -1;
					if (!bGhost)
					{
						TIMER& tim = pGame->timer;
						t = pGame->timer.GetLastLap(cm->iIndex);  // GetPlayerTimeTot
						lap = pGame->timer.GetPlayerCurrentLap(cm->iIndex);  // not o, sorted index
					}
					bool end = pGame->timer.GetCurrentLap(cm->iIndex) >= pSet->game.num_laps
							&& (mClient || pSet->game.local_players > 1);  // multiplay or split
					hudOpp[o][2]->setCaption(
						//+ "   " + toStr(lap) + " " + GetTimeString(t)
						+ end ? cm->sDispName + "  (" + toStr(cm->iWonPlace) + ")" : cm->sDispName);
				}
				//else
				//	hudOpp[o][2]->setCaption(cm->sDispName);
				hudOpp[o][2]->setColour(cm->color);
			}
		}
	}

	//Commenting this code out for now as it is not needed for pixel motion blur
	//   Set motion blur intensity for this viewport, depending on car's linear velocity
	// -----------------------------------------------------------------------------------
	/*if (pSet->motionblur)
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
	*/

	///  gear, vel texts  -----------------------------
	if (txVel[carId] && txGear[carId] && pCar)
	{
		float cl = clutch*0.8f + 0.2f;
		if (gear == -1)
		{	txGear[carId]->setCaption("R");  txGear[carId]->setTextColour(Colour(0.3,1,1,cl));	}
		else if (gear == 0)
		{	txGear[carId]->setCaption("N");  txGear[carId]->setTextColour(Colour(0.3,1,0.3,cl));	}
		else if (gear > 0 && gear < 8)
		{	txGear[carId]->setCaption(toStr(gear));  txGear[carId]->setTextColour(Colour(1,1-gear*0.1,0.2,cl));	}

		txVel[carId]->setCaption(fToStr(std::abs(vel),0,3));

		float k = pCar->GetSpeedometer() * 3.6f * 0.0025f;	// vel clr
		#define m01(x)  std::min(1.0f, std::max(0.0f, (float) (x) ))
		txVel[carId]->setTextColour(Colour(m01(k*2), m01(0.5+k*1.5-k*k*2.5), m01(1+k*0.8-k*k*3.5)));
	}

	//  boost fuel (time)  -----------------------------
	if (txBFuel[carId] && pCar && txBFuel[carId]->getVisible())
	{
		txBFuel[carId]->setCaption(fToStr(pCar->dynamics.boostFuel,1,3));
	}

	//  race countdown  -----------------------------
	if (hudCountdown)
	{
		if (pGame->timer.pretime > 0.f && !pGame->timer.waiting)
		{
			hudCountdown->setCaption(fToStr(pGame->timer.pretime,1,3));
			hudCountdown->show();
		}else
			hudCountdown->hide();
	}
	
	//  abs, tcs on  -----------------------------
	if (hudAbs && hudTcs)
	{
		if (pCar)
		{
			if (pCar->GetABSEnabled())
			{	hudAbs->show();
				hudAbs->setColour(ColourValue(1,0.8,0.6, pCar->GetABSActive() ? 1 : 0.4));
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
		TIMER& tim = pGame->timer;
		
		if (pCarM->bWrongChk || pCarM->iWonPlace > 0 && (pSet->game.local_players > 1 || mClient))
			ovWarnWin->show();  else  ovWarnWin->hide();  //ov
			
		//  lap num (for many or champ)
		bool hasLaps = pSet->game.local_players > 1 || pSet->game.champ_num >= 0 || mClient;
		if (hasLaps)
		{
			if (pCarM->iWonPlace > 0 && hudWonPlace)
			{	
				std::string s = String(TR("---  "+toStr(pCarM->iWonPlace)+" #{TBPlace}  ---"));
				hudWonPlace->setCaption(s);  hudWonPlace->show();
				const static ColourValue clrPlace[4] = {
					ColourValue(0.4,1,0.2), ColourValue(1,1,0.3), ColourValue(1,0.7,0.2), ColourValue(1,0.5,0.2) };
				hudWonPlace->setColour(clrPlace[pCarM->iWonPlace-1]);
			}
		}
		if (txTimes[carId])
			txTimes[carId]->setCaption(
				(hasLaps ? "#D0E8F0"+toStr(tim.GetCurrentLap(carId)+1)+"/"+toStr(pSet->game.num_laps) : "") +
				"\n#E8F4FF" + GetTimeString(tim.GetPlayerTime(carId))+
				"\n#C0E0F0" + GetTimeString(tim.GetLastLap(carId))+
				"\n#D8E0F8" + GetTimeString(tim.GetBestLap(carId, pSet->game.trackreverse)) );
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
			pCar->DebugPrint(s1, true, false, false, false);  ovU[0]->setCaption(s1.str());  ovU[0]->setColour(ColourValue::Black);
			pCar->DebugPrint(s2, false, true, false, false);  ovU[1]->setCaption(s2.str());
			pCar->DebugPrint(s3, false, false, true, false);  ovU[2]->setCaption(s3.str());
			pCar->DebugPrint(s4, false, false, false, true);  ovU[3]->setCaption(s4.str());
		}else
		if (pSet->car_dbgtxt != oldCarTxt)
		{	ovU[0]->setCaption(""); /*ovU[1]->setCaption(""); ovU[2]->setCaption(""); ovU[3]->setCaption("");*/	}
	}
	oldCarTxt = pSet->car_dbgtxt;
	

	//  profiling times --------
	if (pSet->profilerTxt && ovU[1])
	{
		PROFILER.endCycle();
		static int frame=0;  ++frame;

		if (frame > 10)  //par
		{	frame = 0;
			std::string sProf = PROFILER.getAvgSummary(quickprof::MILLISECONDS);
			//sProf = "PROF "+fToStr(1000.f/PROFILER.getAvgDuration(" frameSt",quickprof::MILLISECONDS), 2,4);
			ovU[1]->setCaption(sProf);
		}
		//if (newPosInfos.size() > 0)
		//ovU[3]->setCaption("carm: " + toStr(carModels.size()) + " newp: " + toStr((*newPosInfos.begin()).pos));
	}


	//  bullet profiling text  --------
	static bool oldBltTxt = false;
	if (ovU[1])
	{
		if (pSet->bltProfilerTxt)
		{
			static int cc = 0;  cc++;
			if (cc > 40)
			{	cc = 0;
				std::stringstream os;
				bltDumpAll(os);
				ovU[1]->setCaption(os.str());
			}
		}
		else
		if (pSet->bltProfilerTxt != oldBltTxt)
			ovU[1]->setCaption("");
	}
	oldBltTxt = pSet->bltProfilerTxt;

	
	//  wheels slide, susp bars  --------
	if (pSet->car_dbgbars && pCar)
	{
		const Real xp = 80, yp = -530, ln = 20, y4 = 104;
		//const static char swh[4][6] = {"F^L<","F^R>","RvL<","RvR>"};
		for (int w=0; w < 4; ++w)
		if (ovL[3-w] && ovR[3-w] && ovS[3-w])
		{	
			float slide = /*-1.f*/0.f, sLong = 0.f, sLat = 0.f;
			float squeal = pCar->GetTireSquealAmount((WHEEL_POSITION)w, &slide, &sLong, &sLat);

			//MATHVECTOR <float,3> vwhVel = pCar->dynamics.GetWheelVelocity((WHEEL_POSITION)w);
			//float whVel = vwhVel.Magnitude() * 3.6f;

			/**  //  info
			static char ss[256];
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
	{
		/* checks debug *
		if (ovU[0])  {
			//"ghost:  "  + GetTimeString(ghost.GetTimeLength()) + "  "  + toStr(ghost.GetNumFrames()) + "\n" +
			//"ghplay: " + GetTimeString(ghplay.GetTimeLength()) + "  " + toStr(ghplay.GetNumFrames()) + "\n" +
			ovU[0]->setCaption("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" +
				"         st " + toStr(pCarM->bInSt ? 1:0) + " in" + toStr(pCarM->iInChk) +
				"  |  cur" + toStr(pCarM->iCurChk) + " > next " + toStr(pCarM->iNextChk) +
				"  |  Num " + toStr(pCarM->iNumChks) + " / All " + toStr(road->mChks.size()));
		}	/**/

		if (pCarM->bWrongChk)
			pCarM->fChkTime = 2.f;  //par sec
		int show = pCarM->fChkTime > 0.f ? 1 : 0;
		if (show)  pCarM->fChkTime -= time;
		//if (show != pCarM->iChkWrong)  //-
		bool place = pSet->game.local_players > 1 || mClient, won = pCarM->iWonPlace > 0;
			if (show)  {  hudWarnChk->show();  if (place && !won)  hudWonPlace->hide();  }
			else  {       hudWarnChk->hide();  if (place && won)  hudWonPlace->show();  }
		pCarM->iChkWrong = show;
	}


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


	//  wheels ter mtr, surface info  ---------
	if (pSet->car_dbgsurf && pCar)
	{
		String ss = "";
		ss = pCarM->txtDbgSurf;

		//  surfaces  info
		/*ss += "\n";
		for (int i=0; i < pGame->track.tracksurfaces.size(); ++i)
			ss += String(pGame->track.tracksurfaces[i].name.c_str()) + "\n";/**/

		//ovCarDbg->show();
		if (ovX[4])  {  //ovL[4]->setTop(400);
			ovX[4]->setColour(ColourValue::Black);
			ovX[4]->setCaption(ss);  }
	}

	PROFILER.endBlock("g.hud");
}


//---------------------------------------------------------------------------------------------------------------
//  Update HUD rotated elems - for carId, in baseCarId's space
//---------------------------------------------------------------------------------------------------------------
void App::UpdHUDRot(int baseCarId, int carId, float vel, float rpm)
{
	//if (carId == -1)  return;
	int b = baseCarId, c = carId;
	bool main = b == c;
	float angBase = carModels[b]->angCarY;
	
	bool bZoom = pSet->mini_zoomed && sc->ter, bRot = pSet->mini_rotated && sc->ter;

	const float vmin[2] = {0.f,-45.f}, rmin[2] = {0.f,-45.f},
		vsc_mph[2] = {-180.f/100.f, -(180.f+vmin[1])/90.f},
		vsc_kmh[2] = {-180.f/160.f, -(180.f+vmin[1])/120.f},
		sc_rpm[2] = {-180.f/6000.f, -(180.f+rmin[1])/5000.f};
	const int ig = pSet->gauges_type > 0 ? 1 : 0;

	//  angles
	float angrmp = rpm*sc_rpm[ig] + rmin[ig];
	float vsc = pSet->show_mph ? vsc_mph[ig] : vsc_kmh[ig];
	float angvel = abs(vel)*vsc + vmin[ig];
	float angrot = carModels[c]->angCarY;
	if (bRot && bZoom && !main)
		angrot -= angBase-180.f;

	float sx = 1.4f, sy = sx*asp;  // *par len
	float psx = 2.1f * pSet->size_minimap, psy = psx;  // *par len

	//  4 points, 2d pos
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

		if (bRot && bZoom && main)
			{  px[i] = psx*tp[i][0];  py[i] = psy*tp[i][1];  }
		else{  px[i] = psx*cp*1.4f;   py[i] =-psy*sp*1.4f;   }

		float z = bRot ? 0.70f/pSet->zoom_minimap : 0.5f/pSet->zoom_minimap;
		if (!bRot)
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
		if (!bZoom)
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
	if (!main && bZoom)
	{
		Vector2 plr(-carPoses[qb][b].pos[2],carPoses[qb][b].pos[0]);
		mp -= plr;  mp *= pSet->zoom_minimap;

		if (bRot)
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
		else if (bZoom && main)
			 vNdPos[b][c]->setPosition(0,0,0);
		else vNdPos[b][c]->setPosition(xp,yp,0);
}
