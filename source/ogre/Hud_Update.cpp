#include "pch.h"
#include "../vdrift/par.h"
#include "common/Def_Str.h"
#include "common/RenderConst.h"
#include "common/data/CData.h"
#include "common/data/SceneXml.h"
#include "common/data/TracksXml.h"
#include "common/CScene.h"
#include "../vdrift/game.h"
#include "../vdrift/quickprof.h"
#include "../road/Road.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "SplitScreen.h"
#include "FollowCamera.h"
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


void CHud::GetVals(int id, float* vel, float* rpm, float* clutch, int* gear)
{
	#ifdef DEBUG
	assert(id >= 0);
	assert(id < app->carModels.size());
	assert(id < app->frm.size());
	#endif
	const CarModel* pCarM = app->carModels[id];
	const CAR* pCar = pCarM ? pCarM->pCar : 0;

	if (pCar && !app->bRplPlay && !pCarM->isGhost())
	{	*vel = pCar->GetSpeedometer() * (pSet->show_mph ? 2.23693629f : 3.6f);
		*rpm = pCar->GetEngineRPM();  *gear = pCar->GetGear();
		//*clutch = pCar->GetClutch();  // todo: problems in multi thr1
	}
	if (app->bRplPlay)
	{
		*vel = app->frm[id].vel * (pSet->show_mph ? 2.23693629f : 3.6f);
		*rpm = app->frm[id].rpm;  *gear = app->frm[id].gear;
	}
}

///---------------------------------------------------------------------------------------------------------------
//  Update HUD
///---------------------------------------------------------------------------------------------------------------
void CHud::Update(int carId, float time)
{
	PROFILER.beginBlock("g.hud");

	
	//  update HUD elements for all cars that have a viewport (local or replay)
	//-----------------------------------------------------------------------------------
	int cnt = std::min(6/**/, (int)app->carModels.size());  // all cars
	int cntC = std::min(4/**/, cnt - (app->isGhost2nd && !app->bRplPlay ? 1 : 0));  // all vis plr
	int c;
	
	//  gui viewport - done once for all
	if (carId == -1)
	for (c = 0; c < cntC; ++c)
	if (app->carModels[c]->eType == CarModel::CT_LOCAL)
	{
		//  hud rpm,vel
		float vel=0.f, rpm=0.f, clutch=1.f;  int gear=1;
		GetVals(c,&vel,&rpm,&clutch,&gear);

		if (!app->carModels[c]->vtype == V_Car)
			rpm = -1.f;  // hide rpm gauge
		
		//  update all mini pos tri
		for (int i=0; i < cnt; ++i)
			UpdRot(c, i, vel, rpm);
	}

	///  all minimap car pos-es rot
	const static Real tc[4][2] = {{0,1}, {1,1}, {0,0}, {1,0}};
	const float z = pSet->size_minipos;  // tri size
	
	if (carId == -1 && moPos)
	{	moPos->beginUpdate(0);

		const int plr = app->mSplitMgr->mNumViewports;
		for (int v = 0; v < plr; ++v)  // all viewports
		{
			const Hud& h = hud[v];
			const float sc = pSet->size_minimap * app->mSplitMgr->mDims[v].avgsize;
			const Vector3& pos = h.ndMap->getPosition();
			
			for (c = 0; c < cntC; ++c)  // all mini pos for one car
			{
				const SMiniPos& sp = h.vMiniPos[c];
				const ColourValue& clr = app->carModels[c]->color;

				for (int p=0; p < 4; ++p)  // all 4 points
				{
					float x = pos.x + (sp.x + sp.px[p]*z)*sc;
					float y = pos.y + (sp.y + sp.py[p]*z)*sc*asp;
					moPos->position(x, y, 0);
					moPos->textureCoord(tc[p][0], tc[p][1]);
					moPos->colour(clr);
		}	}	}
		
		int ii = plr * cntC;
		for (int i=0; i < ii; ++i)
		{	int n = i*4;
			moPos->quad(n,n+1,n+3,n+2);
		}
		moPos->end();
	}

	//  track% local, updated always
	for (c = 0; c < cntC; ++c)
	{	CarModel* cm = app->carModels[c];
		if (cm->eType == CarModel::CT_LOCAL ||
			cm->eType == CarModel::CT_REPLAY)
			cm->UpdTrackPercent();
	}
	
	if (carId == -1 || app->carModels.empty())
	{
		PROFILER.endBlock("g.hud");
		return;
	}

	#ifdef DEBUG
	assert(carId >= 0);
	assert(carId < app->carModels.size());
	#endif


	CarModel* pCarM = app->carModels[carId];
	CAR* pCar = pCarM ? pCarM->pCar : 0;

	float vel=0.f, rpm=0.f, clutch=1.f;  int gear=1;
	GetVals(carId,&vel,&rpm,&clutch,&gear);
	Hud& h = hud[carId];


	///  multiplayer
	// -----------------------------------------------------------------------------------
	static float tm = 0.f;  tm += time;
	if (tm > 0.2f && app->mClient/**/)  // not every frame, each 0.2s
	{
		//  sort winners
		std::list<CarModel*> cms;
		for (c=0; c < cnt; ++c)
			cms.push_back(app->carModels[c]);

		cms.sort(SortWin);
		//stable_sort(cms.begin(), cms.end(), SortWin);
		
		String msg = "";  int place = 1;  // assing places
		for (std::list<CarModel*>::iterator it = cms.begin(); it != cms.end(); ++it)
		{
			CarModel* cm = *it;
			bool end = app->pGame->timer.GetCurrentLap(cm->iIndex) >= pSet->game.num_laps;
			cm->iWonPlace = end ? place++ : 0;  // when ended race

			//  detect change (won),  can happen more than once, if time diff < ping delay
			if (cm->iWonPlace != cm->iWonPlaceOld)
			{	cm->iWonPlaceOld = cm->iWonPlace;
				cm->iWonMsgTime = gPar.timeWonMsg;
				if (cm->iIndex == 0)  // for local player, show end wnd
					app->mWndNetEnd->setVisible(true);
			}
			if (cm->iWonMsgTime > 0.f)
			{	cm->iWonMsgTime -= tm;
				if (cm->iWonPlace != 0)
					msg += cm->sDispName + " " + TR("#{FinishedCommaPlace}") + ": " + toStr(cm->iWonPlace) + "\n";
			}
		}
		if (app->mClient && /*ap->pGame->timer.pretime <= 0.f &&*/ app->pGame->timer.waiting)
			msg += TR("#{NetWaitingForOthers}")+"...\n";
			
		//  chat 2 last lines
		if (gui->sChatLast1 != "")	msg += gui->sChatLast1 + "\n";
		if (gui->sChatLast2 != "")	msg += gui->sChatLast2;
			
		++gui->iChatMove;
		if (gui->iChatMove >= 10)  //par 2sec
		{	gui->iChatMove = 0;
			gui->sChatLast1 = gui->sChatLast2;
			gui->sChatLast2 = "";
		}
		
		//  upd hud msgs
		if (txMsg)
		{	txMsg->setCaption(msg);
			bckMsg->setVisible(!msg.empty());
		}

		//  upd end list
		if (app->mWndNetEnd->getVisible())
		{
			MyGUI::MultiList2* li = gui->liNetEnd;
			li->removeAllItems();
			for (std::list<CarModel*>::iterator it = cms.begin(); it != cms.end(); ++it)
			{
				CarModel* cm = *it;
				String clr = StrClr(cm->color);

				li->addItem(""/*clr+ toStr(c+1)*/, 0);  int l = li->getItemCount()-1;
				li->setSubItemNameAt(1,l, clr+ (cm->iWonPlace == 0 ? "--" : toStr(cm->iWonPlace)));
				li->setSubItemNameAt(2,l, clr+ cm->sDispName);
				li->setSubItemNameAt(3,l, clr+ StrTime( cm->iWonPlace == 0 ? 0.f : app->pGame->timer.GetPlayerTimeTot(cm->iIndex) ));
				//li->setSubItemNameAt(4,l, clr+ fToStr(cm->iWonMsgTime,1,3));
				li->setSubItemNameAt(4,l, clr+ StrTime( app->pGame->timer.GetBestLapRace(cm->iIndex) ));
				li->setSubItemNameAt(5,l, clr+ toStr( app->pGame->timer.GetCurrentLap(cm->iIndex) ));
		}	}
		tm = 0.f;
	}


	///  opponents list
	// -----------------------------------------------------------------------------------
	bool visOpp = h.txOpp[0] && pSet->show_opponents;
	if (visOpp && pCarM && pCarM->pMainNode)
	{
		std::list<CarModel*> cms;  // sorted list
		for (c=0; c < cnt; ++c)
		{	//  cars only
			CarModel* cm = app->carModels[c];
			if (!cm->isGhost())
			{	if (app->bRplPlay)
					cm->trackPercent = app->carPoses[app->iCurPoses[c]][c].percent;
				cms.push_back(cm);
		}	}
		if (pSet->opplist_sort)
			cms.sort(SortPerc);
		
		for (c=0; c < cnt; ++c)
		{	//  add last, if visible, ghost1 (dont add 2nd) and track's ghost
			CarModel* cm = app->carModels[c];
			if (cm->eType == (app->isGhost2nd ? CarModel::CT_GHOST2 : CarModel::CT_GHOST) && pSet->rpl_ghost ||
				cm->isGhostTrk() && pSet->rpl_trackghost)
			{
				cm->trackPercent = app->carPoses[app->iCurPoses[c]][c].percent;  // ghost,rpl
				cms.push_back(cm);
		}	}

		bool bGhostEnd = app->pGame->timer.GetPlayerTime(0) > app->ghplay.GetTimeLength();
		String s0,s1,s2;  // Track% Dist Nick
		ColourValue clr;  c = 0;
		for (std::list<CarModel*>::iterator it = cms.begin(); it != cms.end(); ++it)
		{
			CarModel* cm = *it;
			if (cm->pMainNode)
			{
				bool bGhost = cm->isGhost() && !cm->isGhostTrk();
				bool bGhostVis = (app->ghplay.GetNumFrames() > 0) && pSet->rpl_ghost;
				bool bGhEmpty = bGhost && !bGhostVis;

				//  dist  -----------
				if (cm == pCarM || bGhEmpty)  // no dist to self or to empty ghost
					s1 += "\n";
				else
				{	Vector3 v = cm->pMainNode->getPosition() - pCarM->pMainNode->getPosition();
					float dist = v.length();  // meters
					Real h = std::min(60.f, dist) / 60.f;
					clr.setHSB(0.5f - h * 0.4f, 1, 1);
					s1 += StrClr(clr)+ fToStr(dist,0,3)+"m\n";
				}
					
				//  percent %  -----------
				if (bGhEmpty || cm->isGhostTrk())
					s0 += "\n";
				else
				{	float perc = bGhost && bGhostEnd ? 100.f : cm->trackPercent;
					clr.setHSB(perc*0.01f * 0.4f, 0.7f, 1);
					s0 += StrClr(clr)+ fToStr(perc,0,3)+"%\n";
				}
				
				//  nick name  -----------
				if (cm->eType != CarModel::CT_REPLAY)
				{
					s2 += StrClr(cm->color)+ cm->sDispName;
					bool end = app->pGame->timer.GetCurrentLap(cm->iIndex) >= pSet->game.num_laps
							&& (app->mClient || pSet->game.local_players > 1);  // multiplay or split
					if (end)  //  place (1)
						s2 += "  (" + toStr(cm->iWonPlace) + ")";
				}
				s2 += "\n";  ++c;
		}	}
		//  upd pos, size
		if (h.lastOppH != c)
		{	h.lastOppH = c;
			int y = c*25 +4, yo = h.yOpp - y-4;

			for (int n=0; n < 3; ++n)
			{	h.txOpp[n]->setPosition(h.xOpp + n*65+5, yo + 3);
				h.txOpp[n]->setSize(90,y);
			}
			h.bckOpp->setPosition(h.xOpp, yo);
			h.bckOpp->setSize(230,y);
		}
		h.txOpp[0]->setCaption(s0);  h.txOpp[1]->setCaption(s1);  h.txOpp[2]->setCaption(s2);
	}

	//  Set motion blur intensity for this viewport, depending on car's linear velocity
	//-----------------------------------------------------------------------------------
	if (pSet->blur)
	{
		// use velocity squared to achieve an exponential motion blur
		float speed = pCar->GetVelocity().MagnitudeSquared();
		
		// peak at 250 kmh (=69 m/s), 69² = 4761
		// motion blur slider: 1.0 = peak at 100 km/h   0.0 = peak at 400 km/h   -> 0.5 = peak at 250 km/h
		// lerp(100, 400, 1-motionBlurIntensity)
		float peakSpeed = 100 + (1-pSet->blur_int) * (400-100);
		float intens = fabs(speed) / pow((peakSpeed/3.6f), 2);
		
		// higher fps = less perceived motion blur time a frame will be still visible on screen:
		// each frame, 1-intens of the original image is lost
		// example (intens = 0.7):
		//	   frame 1: full img		   frame 2: 0.7  * image
		//	   frame 3: 0.7² * image	   frame 4: 0.7³ * image
		// portion of image visible after 'n' frames: pow(intens, n);
		//	   example 1: 60 fps	   0.7³ image after 4 frames: 0.066 sec
		//	   example 2: 120 fps	   0.7³ image after 4 frames: 0.033 sec
		// now: need to achieve *same* time for both fps values
		// to do this, adjust intens
		// (1.0/fps) * pow(intens, n) == (1.0/fps2) * pow(intens2, n)
		// set n=4  intens_new = sqrt(sqrt((intens^4 * fpsReal/desiredFps))
		intens = sqrt(sqrt( pow(intens, 4) * ((1.0f/time) / 120.0f) ));
			
		intens = std::min(intens, 0.9f);  // clamp to 0.9f
		app->motionBlurIntensity = intens;
	}


	///  gear, vel texts  -----------------------------
	if (h.txGear)
	{
		float cl = clutch*0.8f + 0.2f;
		if (gear == -1)
		{	h.txGear->setCaption("R");  h.txGear->setTextColour(Colour(0.3,1,1,cl));  }
		else if (gear == 0)
		{	h.txGear->setCaption("N");  h.txGear->setTextColour(Colour(0.3,1,0.3,cl));  }
		else if (gear > 0 && gear < 8)
		{	h.txGear->setCaption(toStr(gear));  h.txGear->setTextColour(Colour(1,1-gear*0.1,0.2,cl));  }
	}
	if (h.txVel && pCar)
	{
		h.txVel->setCaption(fToStr(fabs(vel),0,3));

		float k = pCar->GetSpeedometer() * 3.6f * 0.0025f;	// vel clr
		#define m01(x)  std::min(1.f, std::max(0.f, (float) (x) ))
		h.txVel->setTextColour(Colour(m01(k*2.f), m01(0.5f+k*1.5f-k*k*2.5f), m01(1+k*0.8f-k*k*3.5f)));
	}

	//  boost fuel (time)  ------
	if (h.txBFuel && pCar && h.txBFuel->getVisible())
	{
		float f = 0.1f * std::min(10.f, pCar->dynamics.boostFuel);
		//ColourValue c;  c.setHSB(0.6f - f*0.1f, 0.7f + f*0.3f, 0.8f + f*0.2f);
		//h.txBFuel->setTextColour(Colour(c.r,c.g,c.b));
		h.txBFuel->setTextColour(Colour(f*0.35f +0.25f, std::min(1.f, f*0.9f +0.5f), std::min(1.f, f*0.6f +0.75f)));
		h.txBFuel->setCaption(fToStr(pCar->dynamics.boostFuel,1,3));
	}

	//  damage %  ------
	if (h.txDamage && pCar && h.txDamage->getVisible())
	{
		float d = std::min(100.f, Math::Floor(pCar->dynamics.fDamage));
		h.txDamage->setCaption(fToStr(d,0,3)+" %");  d*=0.01f;
		float e = std::min(1.f, 0.8f + d*2.f);
		h.txDamage->setTextColour(Colour(e-d*d*0.4f, std::max(0.f, e-d), std::max(0.f, e-d*2.f) ));
	}
	
	//  abs, tcs on  ------
	if (h.txAbs && h.txTcs && pCar)
	{
		bool vis = pCar->GetABSEnabled();  h.txAbs->setVisible(vis);
		if (vis)  h.txAbs->setAlpha(pCar->GetABSActive() ? 1.f : 0.6f);
		
		vis = pCar->GetTCSEnabled();  h.txTcs->setVisible(vis);
		if (vis)  h.txTcs->setAlpha(pCar->GetTCSActive() ? 1.f : 0.6f);
	}
	
	
	///  times, race pos  -----------------------------
	if (pSet->show_times && pCar)
	{
		TIMER& tim = app->pGame->timer;
		bool hasLaps = pSet->game.local_players > 1 || pSet->game.champ_num >= 0 || pSet->game.chall_num >= 0 || app->mClient;
		if (hasLaps)
		{	//  place
			if (pCarM->iWonPlace > 0 && h.txPlace)
			{
				String s = TR("---  "+toStr(pCarM->iWonPlace)+" #{TBPlace}  ---");
				h.txPlace->setCaption(s);
				const static Colour clrPlace[4] = {
					Colour(0.4,1,0.2), Colour(1,1,0.3), Colour(1,0.7,0.2), Colour(1,0.5,0.2) };
				h.txPlace->setTextColour(clrPlace[pCarM->iWonPlace-1]);
				h.bckPlace->setVisible(true);
		}	}

		//  times  ------------------------------
		bool cur = pCarM->iCurChk >= 0 && !app->vTimeAtChks.empty();
		float ghTimeES = cur ? app->vTimeAtChks[pCarM->iCurChk] : 0.f;
		float part = ghTimeES / app->fLastTime;  // fraction which track ghost has driven

		bool coldStart = tim.GetCurrentLap(carId) == 1;  // was 0
		float carMul = app->GetCarTimeMul(pSet->game.car[carId], pSet->game.sim_mode);
		//| cur
		float ghTimeC = ghTimeES + (coldStart ? 0 : 1);
		float ghTime = ghTimeC * carMul;  // scaled
		float diffT = pCarM->timeAtCurChk - ghTime;  // cur car diff at chk
		float diff = 0.f;  // on hud
		
		//!- if (pCarM->updTimes || pCarM->updLap)
		{	pCarM->updTimes = false;

			//  track time, points
			float last = tim.GetLastLap(carId), best = tim.GetBestLap(carId, pSet->game.trackreverse);
			float timeCur = last < 0.1f ? best : last;
			float timeTrk = app->data->tracks->times[pSet->game.track];
			bool b = timeTrk > 0.f && timeCur > 0.f;

			//bool coldStart = tim.GetCurrentLap(carId) == 1;  // was 0
			float time = (/*place*/1 * app->data->cars->magic * timeTrk + timeTrk) / carMul;  // trk time (for 1st place)
			//float t1pl = data->carsXml.magic * timeTrk;

			float points = 0.f, curPoints = 0.f;
			int place = app->GetRacePos(timeCur, timeTrk, carMul, coldStart, &points);
			//| cur
			float timCC = timeTrk + (coldStart ? 0 : 1);
			float timCu = timCC * carMul;
			diff = pCarM->timeAtCurChk + /*(coldStart ? 1:0)*carMul*/ - time * part;  ///new

			float chkPoints = 0.f;  // cur, at chk, assume diff time later than track ghost
			int chkPlace = app->GetRacePos(timCu + diffT, timeTrk, carMul, coldStart, &chkPoints);
			bool any = cur || b;
	
			h.sTimes =
				"\n#80E080" + StrTime(time)+
				"\n#D0D040" + (cur ? toStr( chkPlace )     : "--")+
				"\n#F0A040" + (cur ? fToStr(chkPoints,1,3) : "--");
			float dlap = last - time;
			h.sLap =
				"#D0E8FF"+TR("#{TBLapResults}") +
				"\n#80C8FF" + StrTime(last)+
				(last > 0.f ? String("  ") + (dlap > 0.f ? "#80E0FF+" : "#60FF60-") + fToStr(fabs(dlap), 1,3) : "")+
				"\n#80E0E0" + StrTime(best)+
				"\n#80E080" + StrTime(time)+
				"\n#D0D040" + (b ? toStr(place)      : "--")+
				"\n#F0A040" + (b ? fToStr(points,1,3) : "--");
			if (h.txLap)
				h.txLap->setCaption(h.sLap);
		}
		if (h.txTimes)
			h.txTimes->setCaption(
				(hasLaps ? "#A0E0D0"+toStr(tim.GetCurrentLap(carId)+1)+" / "+toStr(pSet->game.num_laps) : "") +
				"\n#A0E0E0" + StrTime(tim.GetPlayerTime(carId))+
				(cur ? String("  ") + (diff > 0.f ? "#80E0FF+" : "#60FF60-")+
					fToStr(fabs(diff), 1,3) : "")+
				h.sTimes+
				"\n#E0B090" + fToStr(pCarM->trackPercent,0,1)+"%" );

		if (h.txLap)
		{
			//if (pCarM->updLap)
			//{	pCarM->updLap = false;
				//h.txLap->setCaption(h.sLap);
			//}
			float a = std::min(1.f, pCarM->fLapAlpha * 2.f);
			bool hasRoad = app->scn->road && app->scn->road->getNumPoints() > 2;
			bool vis = pSet->show_times && hasRoad && a > 0.f;
			if (vis)
			{	if (app->iLoad1stFrames == -2)  //bLoading)  //  fade out
				{	pCarM->fLapAlpha -= !hasRoad ? 1.f : time * gPar.fadeLapResults;
					if (pCarM->fLapAlpha < 0.f)  pCarM->fLapAlpha = 0.f;
				}
				h.bckLap->setAlpha(a);
				h.txLapTxt->setAlpha(a);  h.txLap->setAlpha(a);
			}
			h.bckLap->setVisible(vis);
			h.txLapTxt->setVisible(vis);  h.txLap->setVisible(vis);
		}
	}


	//  checkpoint warning  --------
	if (app->scn->road && h.bckWarn && pCarM)
	{
		/* checks debug *
		if (ov[0].oU)  {
			//"ghost:  "  + StrTime(ghost.GetTimeLength()) + "  "  + toStr(ghost.GetNumFrames()) + "\n" +
			//"ghplay: " + StrTime(ghplay.GetTimeLength()) + "  " + toStr(ghplay.GetNumFrames()) + "\n" +
			ov[0].oU->setCaption(String("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n") +
				"         st " + toStr(pCarM->bInSt ? 1:0) + " in" + toStr(pCarM->iInChk) +
				"  |  cur" + toStr(pCarM->iCurChk) + " > next " + toStr(pCarM->iNextChk) +
				"  |  Num " + toStr(pCarM->iNumChks) + " / All " + toStr(road->mChks.size()));
		}	/**/

		if (pCarM->bWrongChk)
			pCarM->fChkTime = gPar.timeShowChkWarn;
			
		bool show = pCarM->fChkTime > 0.f;
		if (show)  pCarM->fChkTime -= time;
		h.bckWarn->setVisible(show && pSet->show_times);
	}

	//  race countdown  ------
	if (h.txCountdown)
	{
		bool vis = app->pGame->timer.pretime > 0.f && !app->pGame->timer.waiting;
		if (vis)
			h.txCountdown->setCaption(fToStr(app->pGame->timer.pretime,1,3));
		h.txCountdown->setVisible(vis);
	}

	//  camera cur  ------
	if (h.txCam)
	{	FollowCamera* cam = pCarM->fCam;
		if (cam && cam->updName)
		{	cam->updName = false;
			h.txCam->setCaption(cam->sName);
	}	}


	//-------------------------------------------------------------------------------------------------------------------
	///  debug infos
	//-------------------------------------------------------------------------------------------------------------------
	
	static int oldTxtClr = -2;  // clr upd
	if (oldTxtClr != pSet->car_dbgtxtclr)
	{	oldTxtClr = pSet->car_dbgtxtclr;
		UpdDbgTxtClr();
	}

	//  car debug text  --------
	static bool oldCarTxt = false;
	if (pCar && ov[0].oU)
	{
		if (pSet->car_dbgtxt)
		{	std::stringstream s1,s2,s3,s4;
			pCar->DebugPrint(s1, true, false, false, false);  ov[0].oU->setCaption(s1.str());
			pCar->DebugPrint(s2, false, true, false, false);  ov[1].oU->setCaption(s2.str());
			pCar->DebugPrint(s3, false, false, true, false);  ov[2].oU->setCaption(s3.str());
			pCar->DebugPrint(s4, false, false, false, true);  ov[3].oU->setCaption(s4.str());
		}else
		if (pSet->car_dbgtxt != oldCarTxt)
		{	ov[0].oU->setCaption(""); /*ovU[1]->setCaption(""); ovU[2]->setCaption(""); ovU[3]->setCaption("");*/	}
	}
	oldCarTxt = pSet->car_dbgtxt;
	

	//  profiling times --------
	if (pSet->profilerTxt && ov[1].oU)
	{
		PROFILER.endCycle();
		static int frame=0;  ++frame;

		if (frame > 10)  //par
		{	frame = 0;
			std::string sProf = PROFILER.getAvgSummary(quickprof::MILLISECONDS);
			//sProf = "PROF "+fToStr(1000.f/PROFILER.getAvgDuration(" frameSt",quickprof::MILLISECONDS), 2,4);
			ov[1].oU->setCaption(sProf);
		}
		//if (newPosInfos.size() > 0)
		//ov[3].oU->setCaption("carm: " + toStr(ap->carModels.size()) + " newp: " + toStr((*newPosInfos.begin()).pos));
	}


	//  bullet profiling text  --------
	static bool oldBltTxt = false;
	if (ov[1].oU)
	{
		if (pSet->bltProfilerTxt)
		{
			static int cc = 0;  ++cc;
			if (cc > 40)
			{	cc = 0;
				std::stringstream os;
				bltDumpAll(os);
				ov[1].oU->setCaption(os.str());
			}
		}
		else
		if (pSet->bltProfilerTxt != oldBltTxt)
			ov[1].oU->setCaption("");
	}
	oldBltTxt = pSet->bltProfilerTxt;

	
	//  wheels slide, susp bars  --------
	if (pSet->car_dbgbars && pCar)
	{
		const Real xp = 80, yp = -530, ln = 20, y4 = 104;
		//const static char swh[4][6] = {"F^L<","F^R>","RvL<","RvR>"};
		for (int w=0; w < 4; ++w)
		if (ov[3-w].oL && ov[3-w].oR && ov[3-w].oS)
		{	
			float slide = /*-1.f*/0.f, sLong = 0.f, sLat = 0.f;
			float squeal = pCar->GetTireSquealAmount((WHEEL_POSITION)w, &slide, &sLong, &sLat);

			//MATHVECTOR<float,3> vwhVel = pCar->dynamics.GetWheelVelocity((WHEEL_POSITION)w);
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
			sLong = fabs(sLong);
			float slng = sLong / sLong * powf(sLong, 0.3f);  // slide*20.f

			ov[3-w].oR->setPosition(slng * 14.f +xp, yp + w*ln);
			ov[3-w].oL->setPosition(sLat * 14.f +xp, yp + w*ln +y4);
			ov[3-w].oS->setPosition(susp * 70.f +xp, yp + w*ln -y4);
		}
		if (ov[4].oL)  ov[4].oL->setPosition(xp, yp + -20 +y4+3);
		if (ov[4].oS)  ov[4].oS->setPosition(xp + 70, yp + -20 -104-3);

		//ov[3-w].oR->setCaption("|");  ov[3-w].oR->setColour(ColourValue(0.6,1.0,0.7));
	}


	//  input values
	/*if (pCar && ap->pGame && ap->pGame->profilingmode)
	{	const std::vector<float>& inp = pCar->dynamics.inputsCopy;
	if (ov[2].oU && inp.size() == CARINPUT::ALL)
	{	sprintf(s, 
		" Throttle %5.2f\n Brake %5.2f\n Steer %5.2f\n"
		" Handbrake %5.2f\n Boost %5.2f\n Flip %5.2f\n"
		,inp[CARINPUT::THROTTLE], inp[CARINPUT::BRAKE], -inp[CARINPUT::STEER_LEFT]+inp[CARINPUT::STEER_RIGHT]
		,inp[CARINPUT::HANDBRAKE],inp[CARINPUT::BOOST], inp[CARINPUT::FLIP] );
		ov[2].oU->setCaption(String(s));
	}	}/**/


	//  wheels ter mtr, surface info  ---------
	if (pSet->car_dbgsurf && pCar)
	{
		String ss = pCarM->txtDbgSurf;

		//  surfaces  info
		/*ss += "\n";
		for (int i=0; i < ap->pGame->track.tracksurfaces.size(); ++i)
			ss += String(ap->pGame->track.tracksurfaces[i].name.c_str()) + "\n";/**/

		//ovCarDbg->show();
		if (ov[4].oX)  {  //ov[4].oL->setTop(400);
			ov[4].oX->setCaption(ss);  }
	}
	

	///  tire vis circles  + + + +
	if (pCar && moTireVis[0] && pSet->car_tirevis)
	{
		const Real z = 6000.f / pSet->tc_r, zy = pSet->tc_xr,
			m_z = 2.f * z;  // scale, max factor
		const int na = 32;  // circle quality
		const Real ad = 2.f*PI_d/na, u = 0.02f;  // u line thickness
		const ColourValue cb(0.8,0.8,0.8),cl(0.2,1,0.2),cr(0.9,0.4,0),cc(1,1,0);
		
		for (int i=0; i < 4; ++i)
		{
			const CARDYNAMICS& cd = pCar->dynamics;
			const CARWHEEL::SlideSlip& t = cd.wheel[i].slips;
			float d = cd.wheel_contact[i].GetDepth() - 2*cd.wheel[i].GetRadius();
			bool off = !(d > -0.1f && d <= -0.01f);  // not in air
			//bool on = cd.wheel_contact[i].GetColObj();
			
			ManualObject* m = moTireVis[i];
			m->beginUpdate(0);
			//  back +
			m->position(-1,0,0);  m->colour(cb);
			m->position( 1,0,0);  m->colour(cb);
			m->position(0,-1,0);  m->colour(cb);
			m->position(0, 1,0);  m->colour(cb);
			
			//  tire, before combine
			Real lx = off ? 0.f : -t.preFy/z*zy,  ly = off ? 0.f : t.preFx/z;
			for (int y=-1; y<=1; ++y)
			for (int x=-1; x<=1; ++x)  {
				m->position(0  +x*u, 0  +y*u, 0);  m->colour(cr);
				m->position(lx +x*u, ly +y*u, 0);  m->colour(cr);  }

			//  tire line /
			lx = off ? 0.f : -t.Fy/z*zy;  ly = off ? 0.f : t.Fx/z;
			for (int y=-2; y<=2; ++y)
			for (int x=-2; x<=2; ++x)  {
				m->position(0  +x*u, 0  +y*u, 0);  m->colour(cl);
				m->position(lx +x*u, ly +y*u, 0);  m->colour(cl);  }

			//  max circle o
			Real rx = off || t.Fym > m_z ? 0.f : t.Fym/z,
			     ry = off || t.Fxm > m_z ? 0.f : t.Fxm/z, a = 0.f;
			Vector3 p(0,0,0),po(0,0,0);
			
			for (int n=0; n <= na; ++n)
			{
				p.x = rx*cosf(a)*zy;  p.y =-ry*sinf(a);
				if (n > 0)  {
					m->position(po);  m->colour(cc);
					m->position(p);   m->colour(cc);  }
				a += ad;  po = p;
			}
			m->end();
		}
	}

	PROFILER.endBlock("g.hud");
}


//---------------------------------------------------------------------------------------------------------------
///  Update HUD rotated elems - for carId, in baseCarId's space
//---------------------------------------------------------------------------------------------------------------
void CHud::UpdRot(int baseCarId, int carId, float vel, float rpm)
{
	//if (carId == -1)  return;
	int b = baseCarId, c = carId;
	bool main = b == c;
	#ifdef DEBUG
	assert(c >= 0);
	assert(b >= 0);
	assert(b < hud.size());  // only b
	assert(c < app->carModels.size());
	assert(b < app->carModels.size());
	assert(c < hud[b].vMiniPos.size());
	#endif
	float angBase = app->carModels[b]->angCarY;
	
	bool bZoom = pSet->mini_zoomed && app->scn->sc->ter,
		bRot = pSet->mini_rotated && app->scn->sc->ter;

	const float vmin[2] = {0.f,-45.f}, rmin[2] = {0.f,-45.f},
		vsc_mph[2] = {-180.f/100.f, -(180.f+vmin[1])/90.f},
		vsc_kmh[2] = {-180.f/160.f, -(180.f+vmin[1])/120.f},
		sc_rpm[2] = {-180.f/6000.f, -(180.f+rmin[1])/5000.f};
	const int ig = pSet->gauges_type > 0 ? 1 : 0;

	//  angles
	float angrmp = rpm*sc_rpm[ig] + rmin[ig];
	float vsc = pSet->show_mph ? vsc_mph[ig] : vsc_kmh[ig];
	float angvel = fabs(vel)*vsc + vmin[ig];
	float angrot = app->carModels[c]->angCarY;
	if (bRot && bZoom && !main)
		angrot -= angBase-180.f;

	Hud& h = hud[b];  int p;
	float sx = 1.4f * h.fScale, sy = sx*asp;  // *par len

	//  4 points, 2d pos
	const static Real tc[4][2] = {{0,1}, {1,1}, {0,0}, {1,0}};  // defaults, no rot
	const static Real tn[4][2] = {{0.5f,1.f}, {1.f,1.f}, {0.5,0.5f}, {1.f,0.5f}};  // tc needle
	const static Real tp[4][2] = {{-1,-1}, {1,-1}, {-1,1}, {1,1}};
	const static float d2r = PI_d/180.f;
	const static Real ang[4] = {0.f,90.f,270.f,180.f};

	float rx[4],ry[4], vx[4],vy[4], px[4],py[4], cx[4],cy[4];  // rpm,vel, pos,crc
	for (int i=0; i<4; ++i)  // 4 verts, each +90deg
	{
		//  needles
		float ia = 45.f + ang[i];
		if (main)
		{	float r = -(angrmp + ia) * d2r;   rx[i] = sx*cosf(r);  ry[i] =-sy*sinf(r);
			float v = -(angvel + ia) * d2r;   vx[i] = sx*cosf(v);  vy[i] =-sy*sinf(v);
		}
		float p = -(angrot + ia) * d2r;	  float cp = cosf(p), sp = sinf(p);

		//  mini
		if (bRot && bZoom && main)
			{  px[i] = tp[i][0];  py[i] = tp[i][1];  }
		else{  px[i] = cp*1.4f;   py[i] =-sp*1.4f;   }

		float z = bRot ? 0.70f/pSet->zoom_minimap : 0.5f/pSet->zoom_minimap;
		if (!bRot)
			{  cx[i] = tp[i][0]*z;  cy[i] = tp[i][1]*z-1.f;  }
		else{  cx[i] =       cp*z;  cy[i] =      -sp*z-1.f;  }
	}
	    
    //  rpm,vel needles
    float r = 0.55f, v = 0.85f;
    bool bRpm = rpm >= 0.f;
	if (main && h.moNeedles)
	{
		h.moNeedles->beginUpdate(0);
		if (bRpm)
		for (p=0; p<4; ++p)  {
			h.moNeedles->position(
				h.vcRpm.x + rx[p]*r,
				h.vcRpm.y + ry[p]*r, 0);  h.moNeedles->textureCoord(tn[p][0], tn[p][1]);  }
		for (p=0; p<4; ++p)  {
			h.moNeedles->position(
				h.vcVel.x + vx[p]*v,
				h.vcVel.y + vy[p]*v, 0);  h.moNeedles->textureCoord(tn[p][0], tn[p][1]);  }
		h.moNeedles->quad(0,1,3,2);
		if (bRpm)
			h.moNeedles->quad(4,5,7,6);
 		h.moNeedles->end();
	}
	//  rpm,vel gauges backgr
	if (main && h.updGauges && h.moGauges)
	{	h.updGauges=false;
		Real o = pSet->show_mph ? 0.5f : 0.f;
	
		h.moGauges->beginUpdate(0);
		if (bRpm)
		for (p=0; p<4; ++p)  {
			h.moGauges->position(
				h.vcRpm.x + tp[p][0]*h.fScale*r,
				h.vcRpm.y + tp[p][1]*h.fScale*asp*r, 0);  h.moGauges->textureCoord(tc[p][0]*0.5f, tc[p][1]*0.5f+0.5f);  }
		for (p=0; p<4; ++p)  {
			h.moGauges->position(
				h.vcVel.x + tp[p][0]*h.fScale*v,
				h.vcVel.y + tp[p][1]*h.fScale*asp*v, 0);  h.moGauges->textureCoord(tc[p][0]*0.5f+o, tc[p][1]*0.5f);  }
		h.moGauges->quad(0,1,3,2);
		if (bRpm)
			h.moGauges->quad(4,5,7,6);
		h.moGauges->end();
	}

		
	///  minimap car pos-es rot
	for (p=0; p<4; ++p)
	{	
		h.vMiniPos[c].px[p] = px[p];
		h.vMiniPos[c].py[p] = py[p];
	}
	
	//  minimap circle/rect rot
	int qb = app->iCurPoses[b], qc = app->iCurPoses[c];
	if (h.moMap && pSet->trackmap && main)
	{
		h.moMap->beginUpdate(0);
		if (!bZoom)
			for (p=0; p<4; ++p)  {
				h.moMap->position(tp[p][0],tp[p][1], 0);  h.moMap->textureCoord(tc[p][0], tc[p][1]);
				h.moMap->colour(tc[p][0],tc[p][1], 0);  }
		else
		{	Vector2 mp(-app->carPoses[qb][b].pos[2], app->carPoses[qb][b].pos[0]);
			float xc =  (mp.x - minX)*scX,
				  yc = -(mp.y - minY)*scY+1.f;
			for (p=0; p<4; ++p)  {
				h.moMap->position(tp[p][0],tp[p][1], 0);  h.moMap->textureCoord(cx[p]+xc, -cy[p]-yc);
				h.moMap->colour(tc[p][0],tc[p][1], 1);  }
		}
		h.moMap->end();
	}

	///  minimap car pos  x,y = -1..1
	Vector2 mp(-app->carPoses[qc][c].pos[2], app->carPoses[qc][c].pos[0]);

	//  other cars in player's car view space
	if (!main && bZoom)
	{
		Vector2 plr(-app->carPoses[qb][b].pos[2], app->carPoses[qb][b].pos[0]);
		mp -= plr;  mp *= pSet->zoom_minimap;

		if (bRot)
		{
			float a = angBase * PI_d/180.f;  Vector2 np;
			np.x = mp.x*cosf(a) - mp.y*sinf(a);  // rotate
			np.y = mp.x*sinf(a) + mp.y*cosf(a);  mp = -np;
		}
	}
	float xp =  (mp.x - minX)*scX*2.f-1.f,
		  yp = -(mp.y - minY)*scY*2.f+1.f;

	//  clamp to circle
	if (bZoom /*&& bRot*/)
	{
		float d = xp*xp + yp*yp;
		const float dd = pSet->mini_border ? 0.95f : 0.85f;
		if (d > dd*dd)
		{	d = dd/sqrt(d);
			xp *= d;  yp *= d;
		}
	}else
	{	// clamp to square
		xp = std::min(1.f, std::max(-1.f, xp));
		yp = std::min(1.f, std::max(-1.f, yp));
	}
	
	//  visible
	int cg = app->isGhost2nd && !app->bRplPlay &&
		app->carModels[c]->eType == CarModel::CT_GHOST &&
		c < app->carModels.size()-1 ? 1 : 0;

	bool hide = !app->carModels[c+cg]->mbVisible;
	if (hide)
	{	h.vMiniPos[c].x = -100.f;
		h.vMiniPos[c].y = 0.f;  }
	else if (bZoom && main)
	{	h.vMiniPos[c].x = 0.f;
		h.vMiniPos[c].y = 0.f;  }
	else
	{	h.vMiniPos[c].x = xp;
		h.vMiniPos[c].y = yp;  }
}
