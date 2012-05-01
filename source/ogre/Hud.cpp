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


///  HUD resize
void App::SizeHUD(bool full, Viewport* vp, int carId)
{
	float wx = mWindow->getWidth(), wy = mWindow->getHeight();
	asp = wx/wy;
	//  for each car
	for (int c=0; c < pSet->game.local_players; ++c)
	{
		//  gauges
		Real xcRpm,ycRpm, xcVel,ycVel, yMax, xBFuel;  // -1..1
		if (ndRpmBk[c] && ndVelBk[c] && ndVelBm[c] && ndRpm[c] && ndVel[c])
		{
			Real fHudScale = pSet->size_gauges * mSplitMgr->mDims[c].avgsize;
			Real spx = fHudScale *1.1f, spy = spx*asp;
			xcRpm = mSplitMgr->mDims[c].left + spx;   ycRpm =-mSplitMgr->mDims[c].bottom + spy;
			xcVel = mSplitMgr->mDims[c].right - spx;  ycVel =-mSplitMgr->mDims[c].bottom + spy;
			yMax = ycVel - fHudScale;  xBFuel = xcVel - fHudScale;

			Vector3 sca(fHudScale,fHudScale*asp,1), sc(fHudScale,fHudScale,1);
			ndRpmBk[c]->setScale(sca);	ndVelBk[c]->setScale(sca);  ndVelBm[c]->setScale(sca);
			ndRpm[c]->setScale(sc); 	ndVel[c]->setScale(sc);

			Vector3 vr(xcRpm,ycRpm,0.f), vv(xcVel,ycVel,0.f);
			ndRpmBk[c]->setPosition(vr); ndVelBk[c]->setPosition(vv);  ndVelBm[c]->setPosition(vv);
			ndRpm[c]->setPosition(vr);	ndVel[c]->setPosition(vv);
			//LogO("-- Size  r "+toStr(vr)+"  v "+toStr(vv)+"  s "+toStr(sca));
		}
		//  minimap
		if (ndMap[c])
		{
			Real fHudSize = pSet->size_minimap * mSplitMgr->mDims[c].avgsize;
			ndMap[c]->setScale(fHudSize,fHudSize*asp,1);

			const Real marg = 1.f + 0.05f;  // from border
			Real fMiniX = mSplitMgr->mDims[c].right - fHudSize * marg;
			Real fMiniY =-mSplitMgr->mDims[c].top - fHudSize*asp * marg;

			ndMap[c]->setPosition(Vector3(fMiniX,fMiniY,0.f));
			//LogO("-- Size car:"+toStr(c)+"  x:"+fToStr(fMiniX,2,4)+" y:"+fToStr(fMiniY,2,4)+"  s:"+fToStr(fHudSize,2,4));
		}

		//  gear, vel texts
		if (txGear[c] && txVel[c] && txBFuel[c])
		{
			//  current viewport max x,y in pixels
			int iwx = (mSplitMgr->mDims[c].right +1.f)*0.5f*wx,
				iwy = (mSplitMgr->mDims[c].bottom+1.f)*0.5f*wy;
			int my = (1.f-yMax)*0.5f*wy;  // gauge bottom y

			//  positioning,  min iwy - dont go below viewport bottom
			int vv = pSet->gauges_type > 0 ? -45 : 40;
			int gx = (xcRpm+1.f)*0.5f*wx + 20, gy = std::min(iwy -48, my - 40);
			int vx = (xcVel+1.f)*0.5f*wx + vv, vy = std::min(iwy -48, my - 15);			
			int bx =(xBFuel+1.f)*0.5f*wx - 10, by = std::min(iwy -36, my + 5);
				vx = std::min(vx, iwx -100);
				bx = std::min(bx, iwx -180);  // not too near to vel
			txGear[c]->setPosition(gx,gy);
			txVel[c]->setPosition(vx,vy);
			txBFuel[c]->setPosition(bx,by);
		}
	}
}


///---------------------------------------------------------------------------------------------------------------
///  HUD create
///---------------------------------------------------------------------------------------------------------------

void App::CreateHUD(bool destroy)
{
	if (carModels.size() == 0)  return;

	SceneManager* scm = mSplitMgr->mGuiSceneMgr;

	///  graphs create  .-_/\._-
	if (graphs.size()==0)
	{
		for (int i=0; i < 4; ++i)
		{
			GraphView* gv = new GraphView(scm);
			gv->Create(512, "graph"+toStr(i%4+1), 0.13f);
			if (i >= 4)
				gv->SetSize(0.f, 0.24f, 0.5f, 0.25f);
			else
				gv->SetSize(0.f, 0.50f, 0.5f, 0.25f);
			graphs.push_back(gv);
		}
	}
	
	if (destroy)
	{
		for (int c=0; c < 4; ++c)
		{
			if (moMap[c]) {  scm->destroyManualObject(moMap[c]);  moMap[c]=0;  }
			if (ndMap[c]) {  scm->destroySceneNode(ndMap[c]);  ndMap[c]=0;  }

			for (int i=0; i < 4; ++i)
			{
				if (vMoPos[c][i]) {  scm->destroyManualObject(vMoPos[c][i]);  vMoPos[c][i]=0;  }
				if (vNdPos[c][i]) {  scm->destroySceneNode(vNdPos[c][i]);  vNdPos[c][i]=0;  }
			}
			if (moRpmBk[c])  {  scm->destroyManualObject(moRpmBk[c]);  moRpmBk[c]=0;  }
			if (ndRpmBk[c]) {  scm->destroySceneNode(ndRpmBk[c]);  ndRpmBk[c]=0;  }

			if (moVelBk[c]) {  scm->destroyManualObject(moVelBk[c]);  moVelBk[c]=0;  }
			if (ndVelBk[c]) {  scm->destroySceneNode(ndVelBk[c]);  ndVelBk[c]=0;  }
				
			if (moVelBm[c]) {  scm->destroyManualObject(moVelBm[c]);  moVelBm[c]=0;  }
			if (ndVelBm[c]) {  scm->destroySceneNode(ndVelBm[c]);  ndVelBm[c]=0;  }
				
			if (moRpm[c]) {  scm->destroyManualObject(moRpm[c]);  moRpm[c]=0;  }
			if (ndRpm[c]) {  scm->destroySceneNode(ndRpm[c]);  ndRpm[c]=0;  }
			
			if (moVel[c]) {  scm->destroyManualObject(moVel[c]);  moVel[c]=0;  }
			if (ndVel[c]) {  scm->destroySceneNode(ndVel[c]);  ndVel[c]=0;  }
		}
	}
	for (int c=0; c<4; ++c)
	{	if (txGear[c]) {  mGUI->destroyWidget(txGear[c]);  txGear[c] = 0;  }
		if (txVel[c])  {  mGUI->destroyWidget(txVel[c]);  txVel[c] = 0;  }
		if (txBFuel[c])  {  mGUI->destroyWidget(txBFuel[c]);  txBFuel[c] = 0;  }
	}
	
	//  minimap from road img
	int plr = mSplitMgr->mNumViewports;  // pSet->game.local_players;
	LogO("-- Create Hud  plrs="+toStr(plr));
	asp = 1.f;

	//if (terrain)
	for (int c=0; c < plr; ++c)  // for each car
	{
		float t = sc.td.fTerWorldSize*0.5;
		minX = -t;  minY = -t;  maxX = t;  maxY = t;

		float fMapSizeX = maxX - minX, fMapSizeY = maxY - minY;  // map size
		float size = std::max(fMapSizeX, fMapSizeY*asp);
		scX = 1.f / size;  scY = 1.f / size;

		String sMat = "circle_minimap";
		asp = 1.f;  //_temp
		ManualObject* m = Create2D(sMat,scm,1,true,true);  moMap[c] = m;
		//asp = float(mWindow->getWidth())/float(mWindow->getHeight());
		m->setVisibilityFlags(RV_Hud);  m->setRenderQueueGroup(RQG_Hud1);
		
		//  change minimap image
		MaterialPtr mm = MaterialManager::getSingleton().getByName(sMat);
		Pass* pass = mm->getTechnique(0)->getPass(0);
		TextureUnitState* tus = pass->getTextureUnitState(0);
		if (tus)  tus->setTextureName(pSet->game.track + "_mini.png");
		tus = pass->getTextureUnitState(2);
		if (tus)  tus->setTextureName(pSet->game.track + "_ter.jpg");
		UpdMiniTer();
		

		float fHudSize = pSet->size_minimap * mSplitMgr->mDims[c].avgsize;
		SceneNode* rt = scm->getRootSceneNode();
		ndMap[c] = rt->createChildSceneNode(Vector3(0,0,0));
		ndMap[c]->attachObject(m);
		
		//  car pos tri - for all carModels (ghost and remote too)
		for (int i=0; i < carModels.size(); ++i)
		{
			vMoPos[c][i] = Create2D("hud/CarPos", scm, 0.4f, true, true);
			vMoPos[c][i]->setVisibilityFlags(RV_Hud);  vMoPos[c][i]->setRenderQueueGroup(RQG_Hud3);
				  
			vNdPos[c][i] = ndMap[c]->createChildSceneNode();
			vNdPos[c][i]->scale(fHudSize*1.5f, fHudSize*1.5f, 1);
			vNdPos[c][i]->attachObject(vMoPos[c][i]);  //vNdPos[i]->setVisible(false);
		}
		ndMap[c]->setVisible(false/*pSet->trackmap*/);

	
		//  gauges  backgr
		String st = toStr(pSet->gauges_type);
		moRpmBk[c] = Create2D("hud/rpm"+st,scm,1);  moRpmBk[c]->setVisibilityFlags(RV_Hud);
		moRpmBk[c]->setRenderQueueGroup(RQG_Hud1);
		ndRpmBk[c] = rt->createChildSceneNode();
		ndRpmBk[c]->attachObject(moRpmBk[c]);	ndRpmBk[c]->setScale(0,0,0);  ndRpmBk[c]->setVisible(false);

		moVelBk[c] = Create2D("hud/kmh"+st,scm,1);  moVelBk[c]->setVisibilityFlags(RV_Hud);
		moVelBk[c]->setRenderQueueGroup(RQG_Hud1);
		ndVelBk[c] = rt->createChildSceneNode();
		ndVelBk[c]->attachObject(moVelBk[c]);	ndVelBk[c]->setScale(0,0,0);  moVelBk[c]->setVisible(false);
			
		moVelBm[c] = Create2D("hud/mph"+st,scm,1);  moVelBm[c]->setVisibilityFlags(RV_Hud);
		moVelBm[c]->setRenderQueueGroup(RQG_Hud1);
		ndVelBm[c] = rt->createChildSceneNode();
		ndVelBm[c]->attachObject(moVelBm[c]);	ndVelBm[c]->setScale(0,0,0);  moVelBm[c]->setVisible(false);
			
		//  gauges  needles
		moRpm[c] = Create2D("hud/needle"+st,scm,1,true);  moRpm[c]->setVisibilityFlags(RV_Hud);
		moRpm[c]->setRenderQueueGroup(RQG_Hud3);
		ndRpm[c] = rt->createChildSceneNode();
		ndRpm[c]->attachObject(moRpm[c]);	ndRpm[c]->setScale(0,0,0);	ndRpm[c]->setVisible(false);
		
		moVel[c] = Create2D("hud/needle"+st,scm,1,true);  moVel[c]->setVisibilityFlags(RV_Hud);
		moVel[c]->setRenderQueueGroup(RQG_Hud3);
		ndVel[c] = rt->createChildSceneNode();
		ndVel[c]->attachObject(moVel[c]);	ndVel[c]->setScale(0,0,0);	ndVel[c]->setVisible(false);


		//  gear, vel text
		txGear[c] = mGUI->createWidget<TextBox>("TextBox",
			0,1200, 160,116, Align::Left, "Back", "Gear"+toStr(c));
		txGear[c]->setVisible(false);
		txGear[c]->setFontName("DigGear");  txGear[c]->setFontHeight(126);

		txVel[c] = mGUI->createWidget<TextBox>("TextBox",
			0,1200, 360,96, Align::Right, "Back", "Vel"+toStr(c));
		txVel[c]->setVisible(false);
		txVel[c]->setFontName("DigGear");  //txVel[c]->setFontHeight(64);
		
		txBFuel[c] = mGUI->createWidget<TextBox>("TextBox",
			0,1200, 240,80, Align::Right, "Back", "BFuel"+toStr(c));
		txBFuel[c]->setVisible(false);
		txBFuel[c]->setFontName("DigGear");  txBFuel[c]->setFontHeight(64);
		txBFuel[c]->setTextColour(Colour(0.6,0.8,1.0));
	}


	//  overlays
	OverlayManager& ovr = OverlayManager::getSingleton();
	ovCam = ovr.getByName("Car/CameraOverlay");

	ovAbsTcs = ovr.getByName("Hud/AbsTcs");	hudAbs = ovr.getOverlayElement("Hud/AbsText");
	ovCarDbg = ovr.getByName("Car/Stats");	hudTcs = ovr.getOverlayElement("Hud/TcsText");

	ovCountdown = ovr.getByName("Hud/Countdown");	hudCountdown = ovr.getOverlayElement("Hud/CountdownText");
	ovNetMsg = ovr.getByName("Hud/NetMessages");	hudNetMsg = ovr.getOverlayElement("Hud/NetMessagesText");

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
		ovU[i] = ovr.getOverlayElement("U_"+toStr(i+1));
	}
	ShowHUD();  //_
	bSizeHUD = true;
	//SizeHUD(true);
}


void App::ShowHUD(bool hideAll)
{
	if (hideAll)
	{
		if (ovAbsTcs) ovAbsTcs->hide();
		if (ovNetMsg)  ovNetMsg->hide();	if (ovCountdown)  ovCountdown->hide();
		if (ovCarDbg)  ovCarDbg->hide();	if (ovCarDbgTxt)  ovCarDbgTxt->hide();
		if (ovCam)	 ovCam->hide();			if (ovTimes)  ovTimes->hide();
		if (ovWarnWin)  ovWarnWin->hide();	if (ovOpp)  ovOpp->hide();
		if (mFpsOverlay)  mFpsOverlay->hide();

		for (int c=0; c < 4; ++c)
		{
			if (txGear[c])  txGear[c]->setVisible(false);	if (txVel[c])  txVel[c]->setVisible(false);
			if (txBFuel[c])  txBFuel[c]->setVisible(false);
			if (ndRpmBk[c])  ndRpmBk[c]->setVisible(false);
			if (ndVelBk[c])	ndVelBk[c]->setVisible(false);	if (ndVelBm[c])	ndVelBm[c]->setVisible(false);
			if (ndRpm[c])	ndRpm[c]->setVisible(false);	if (ndVel[c])	ndVel[c]->setVisible(false);
			if (ndMap[c])  ndMap[c]->setVisible(false);
		}
		hideMouse();
		if (mWndRpl)  mWndRpl->setVisible(false);
	}
	else
	{	//this goes each frame..
		bool show = pSet->show_gauges;
		if (ovCountdown)  if (show)  ovCountdown->show();  else  ovCountdown->hide();
		if (ovNetMsg)	if (show)  ovNetMsg->show();  else  ovNetMsg->hide();
		if (ovAbsTcs){ if (show)  ovAbsTcs->show();   else  ovAbsTcs->hide(); }

		show = pSet->car_dbgbars;
		if (ovCarDbg){  if (show)  ovCarDbg->show();  else  ovCarDbg->hide();   }
		show = pSet->car_dbgtxt || pSet->bltProfilerTxt || pSet->profilerTxt;
		if (ovCarDbgTxt){  if (show)  ovCarDbgTxt->show();  else  ovCarDbgTxt->hide();   }
		//for (int i=0; i<5; ++i)
		//{	if (ovU[i])  if (show)  ovU[i]->show();  else  ovU[i]->hide();  }

		if (ovCam)	{  if (pSet->show_cam && !isFocGui)    ovCam->show();    else  ovCam->hide();     }
		if (ovTimes){  if (pSet->show_times)  ovTimes->show();  else  ovTimes->hide();   }
		if (ovOpp)  {  if (pSet->show_opponents && road && road->getNumPoints() > 0)  ovOpp->show();  else  ovOpp->hide();   }
		if (ovWarnWin){  if (pSet->show_times)  ovWarnWin->show();  else  ovWarnWin->hide();  }
		if (mFpsOverlay) { if (pSet->show_fps) mFpsOverlay->show(); else mFpsOverlay->hide(); }

		show = pSet->show_gauges;
		for (int c=0; c < 4; ++c)
		{
			if (txGear[c])  txGear[c]->setVisible(pSet->show_digits);	if (txVel[c])  txVel[c]->setVisible(pSet->show_digits);
			if (txBFuel[c])  txBFuel[c]->setVisible(show && (pSet->game.boost_type == 1 || pSet->game.boost_type == 2));
			if (ndRpmBk[c])  ndRpmBk[c]->setVisible(show);
			if (ndVelBk[c])	ndVelBk[c]->setVisible(show && !pSet->show_mph);
			if (ndVelBm[c])	ndVelBm[c]->setVisible(show && pSet->show_mph);
			if (ndRpm[c])	ndRpm[c]->setVisible(show);		if (ndVel[c])	ndVel[c]->setVisible(show);
			if (ndMap[c])  ndMap[c]->setVisible(pSet->trackmap);
		}
		updMouse();
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


//  HUD utils
//---------------------------------------------------------------------------------------------------------------
bool SortPerc(const CarModel* cm2, const CarModel* cm1)
{
	int l1 = cm1->pGame->timer.GetCurrentLap(cm1->iIndex);
	int l2 = cm2->pGame->timer.GetCurrentLap(cm2->iIndex);
	float p1 = cm1->trackPercent;
	float p2 = cm2->trackPercent;
	if (l1 == l2)  return p1 < p2;
	return l1 < l2;
}

bool SortWin(const CarModel* cm2, const CarModel* cm1)
{
	int l1 = cm1->pGame->timer.GetCurrentLap(cm1->iIndex);
	int l2 = cm2->pGame->timer.GetCurrentLap(cm2->iIndex);
	float t1 = cm1->pGame->timer.GetPlayerTimeTot(cm1->iIndex);
	float t2 = cm2->pGame->timer.GetPlayerTimeTot(cm2->iIndex);
	if (l1 == l2)  return t1 > t2;
	return l1 < l2;
}

	
void App::ShowHUDvp(bool vp)	// todo: use vis mask ..
{
	// show/hide for render viewport / gui viewport
	// first show everything
	ShowHUD(false);
	// now hide things we dont want
	if (!vp)
	{
		/// for gui viewport ----------------------
		if (ovOpp)  ovOpp->hide();
		if (ovTimes)  ovTimes->hide();		if (ovWarnWin)  ovWarnWin->hide();
		if (ovCarDbg)  ovCarDbg->hide();	if (ovCarDbgTxt)  ovCarDbgTxt->hide();
		if (ovCountdown)  ovCountdown->hide();  if (ovNetMsg)  ovNetMsg->hide();
		if (hudAbs)  hudAbs->hide();		if (hudTcs)  hudTcs->hide();
	}else{
		/// for render viewport ---------
		if (ovCam)  ovCam->hide();
		if (mFpsOverlay)  mFpsOverlay->hide();
	}
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


//  Update HUD
///---------------------------------------------------------------------------------------------------------------
void App::UpdateHUD(int carId, float time)
{
	PROFILER.beginBlock("g.hud");

	if (bSizeHUD)	// update sizes once after change
	{	bSizeHUD = false;
		SizeHUD(true);	}


	///  graphs update  -._/\_-.
	if (carId == -1)
	{
		if (carModels.size() > 0)
		{
			const CARDYNAMICS& cd = carModels[0]->pCar->dynamics;
			graphs[0]->AddVal(cd.fHitForce);
			graphs[1]->AddVal(cd.fHitForce2);
			graphs[2]->AddVal(cd.fHitForce3);
			graphs[3]->AddVal(cd.fHitForce4);
		}
		/**/
		for (int i=0; i < graphs.size(); ++i)
		{
			//static int t=0; ++t;
			//graphs[i]->AddVal(sinf(i*0.002f+t*0.01f)*0.5f+0.5f);
			graphs[i]->Update();
		}
	}
	
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
		
		String msg = "";  int place = 1;  // assing places
		for (int c = 0; c < carModels.size(); ++c)
		{
			CarModel* cm = carModels[c];
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
			for (int c = 0; c < carModels.size(); ++c)
			{
				CarModel* cm = carModels[c];
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
		for (int o = 0; o < carModels.size(); ++o)
		if (hudOpp[o][0])
		{
			CarModel* cm = carModels[o];
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
					bool end = pGame->timer.GetCurrentLap(cm->iIndex) >= pSet->game.num_laps;
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
		TIMER& tim = pGame->timer;
		
		if (pCarM->bWrongChk || pCarM->iWonPlace > 0 && (pSet->game.local_players > 1 || mClient))
			ovWarnWin->show();  else  ovWarnWin->hide();  //ov
			
		//  lap num (for many or champ)
		std::string times;
		if (pSet->game.local_players > 1 || pSet->game.champ_num >= 0 || mClient)
		{
			if (pCarM->iWonPlace > 0 && hudWonPlace)
			{	
				std::string s = String(TR("---  "+toStr(pCarM->iWonPlace)+" #{TBPlace}  ---"));
				hudWonPlace->setCaption(s);  hudWonPlace->show();
				const static ColourValue clrPlace[4] = {
					ColourValue(0.4,1,0.2), ColourValue(1,1,0.3), ColourValue(1,0.7,0.2), ColourValue(1,0.5,0.2) };
				hudWonPlace->setColour(clrPlace[pCarM->iWonPlace-1]);
			}
			times = String(TR("#{TBLap}  "+toStr(tim.GetCurrentLap(carId)+1)+"/"+toStr(pSet->game.num_laps)));
		}else
		{	if (!road)  // score on vdr track
			if (tim.GetIsDrifting(0))
				times += String(TR("#{TBScore}  "+fToStr(tim.GetDriftScore(0),0,3)+"+"+fToStr(tim.GetThisDriftScore(0),0,2)));
			else
				times += String(TR("#{TBScore}  "+fToStr(tim.GetDriftScore(0),0,3)));
		}		
		if (hudTimes) {
			hudTimes->setCaption(times +
				String(TR("\n#{TBTime} ")) + GetTimeString(tim.GetPlayerTime(carId))+
				String(TR("\n#{TBLast} ")) + GetTimeString(tim.GetLastLap(carId))+
				String(TR("\n#{TBBest} ")) + GetTimeString(tim.GetBestLap(carId, pSet->game.trackreverse)) );
		}
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
			//pCar->DebugPrint(s2, false, true, false, false);  ovU[1]->setCaption(s2.str());
			//pCar->DebugPrint(s3, false, false, true, false);  ovU[2]->setCaption(s3.str());
			//pCar->DebugPrint(s4, false, false, false, true);  ovU[3]->setCaption(s4.str());
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
		/* checks info *
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
	PROFILER.endBlock("g.hud");
}


///  Bullet profiling text
//--------------------------------------------------------------------------------------------------------------

void App::bltDumpRecursive(CProfileIterator* profileIterator, int spacing, std::stringstream& os)
{
	profileIterator->First();
	if (profileIterator->Is_Done())
		return;

	float accumulated_time=0,parent_time = profileIterator->Is_Root() ? CProfileManager::Get_Time_Since_Reset() : profileIterator->Get_Current_Parent_Total_Time();
	int i,j;
	int frames_since_reset = CProfileManager::Get_Frame_Count_Since_Reset();
	for (i=0;i<spacing;i++)	os << ".";
	os << "----------------------------------\n";
	for (i=0;i<spacing;i++)	os << ".";
	std::string s = "Profiling: "+String(profileIterator->Get_Current_Parent_Name())+" (total running time: "+fToStr(parent_time,3)+" ms) ---\n";
	os << s;
	//float totalTime = 0.f;

	int numChildren = 0;
	
	for (i = 0; !profileIterator->Is_Done(); i++,profileIterator->Next())
	{
		numChildren++;
		float current_total_time = profileIterator->Get_Current_Total_Time();
		accumulated_time += current_total_time;
		float fraction = parent_time > SIMD_EPSILON ? (current_total_time / parent_time) * 100 : 0.f;

		for (j=0;j<spacing;j++)	os << ".";
		double ms = (current_total_time / (double)frames_since_reset);
		s = toStr(i)+" -- "+profileIterator->Get_Current_Name()+" ("+fToStr(fraction,2)+" %) :: "+fToStr(ms,3)+" ms / frame ("+toStr(profileIterator->Get_Current_Total_Calls())+" calls)\n";
		os << s;
		//totalTime += current_total_time;
		//recurse into children
	}

	if (parent_time < accumulated_time)
	{
		os << "what's wrong\n";
	}
	for (i=0;i<spacing;i++)	os << ".";
	double unaccounted=  parent_time > SIMD_EPSILON ? ((parent_time - accumulated_time) / parent_time) * 100 : 0.f;
	s = "Unaccounted: ("+fToStr(unaccounted,3)+" %) :: "+fToStr(parent_time - accumulated_time,3)+" ms\n";
	os << s;
	
	for (i=0;i<numChildren;i++)
	{
		profileIterator->Enter_Child(i);
		bltDumpRecursive(profileIterator, spacing+3, os);
		profileIterator->Enter_Parent();
	}
}

void App::bltDumpAll(std::stringstream& os)
{
	CProfileIterator* profileIterator = 0;
	profileIterator = CProfileManager::Get_Iterator();

	bltDumpRecursive(profileIterator, 0, os);

	CProfileManager::Release_Iterator(profileIterator);
}
