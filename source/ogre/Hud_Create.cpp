#include "pch.h"
#include "common/Defines.h"
#include "OgreGame.h"
#include "../vdrift/game.h"
#include "../vdrift/quickprof.h"
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
using namespace MyGUI;


///  HUD resize
//---------------------------------------------------------------------------------------------------------------
void App::SizeHUD(bool full, Viewport* vp)
{
	float wx = mWindow->getWidth(), wy = mWindow->getHeight();
	asp = wx/wy;
	bool vdrSpl = sc->vdr && pSet->game.local_players > 1;
	int cnt = pSet->game.local_players;
	#ifdef DEBUG
	assert(cnt <= hud.size());
	#endif
	//  for each car
	for (int c=0; c < cnt; ++c)
	{
		const SplitScreenManager::VPDims& dim = mSplitMgr->mDims[c];
		//  gauges
		Real xcRpm,ycRpm, xcVel,ycVel, yMax, xBFuel;  // -1..1
		if (hud[c].ndRpmBk && hud[c].ndVelBk && hud[c].ndVelBm && hud[c].ndRpm && hud[c].ndVel)
		{
			Real fHudScale = pSet->size_gauges * dim.avgsize;
			Real spx = fHudScale *1.1f, spy = spx*asp;
			xcRpm = dim.left + spx;   ycRpm =-dim.bottom + spy;
			xcVel = dim.right - spx;  ycVel =-dim.bottom + spy;
			yMax = ycVel - fHudScale;  xBFuel = xcVel - fHudScale;

			Vector3 sca(fHudScale,fHudScale*asp,1), sc(fHudScale,fHudScale,1);
			hud[c].ndRpmBk->setScale(sca);	hud[c].ndVelBk->setScale(sca);  hud[c].ndVelBm->setScale(sca);
			hud[c].ndRpm->setScale(sc); 	hud[c].ndVel->setScale(sc);

			Vector3 vr(xcRpm,ycRpm,0.f), vv(xcVel,ycVel,0.f);
			hud[c].ndRpmBk->setPosition(vr); hud[c].ndVelBk->setPosition(vv);  hud[c].ndVelBm->setPosition(vv);
			hud[c].ndRpm->setPosition(vr);	 hud[c].ndVel->setPosition(vv);
			//LogO("-- Size  r "+toStr(vr)+"  v "+toStr(vv)+"  s "+toStr(sca));
		}
		//  minimap
		if (hud[c].ndMap)
		{
			Real fHudSize = pSet->size_minimap * dim.avgsize;
			hud[c].ndMap->setScale((vdrSpl ? 2 : 1)*fHudSize,fHudSize*asp,1);

			const Real marg = 1.f + 0.05f;  // from border
			//Real fMiniX = dim.right - fHudSize * marg;
			Real fMiniX = vdrSpl ? (1.f - 2.f*fHudSize * marg) : (dim.right - fHudSize * marg);
			Real fMiniY =-dim.top - fHudSize*asp * marg;

			hud[c].ndMap->setPosition(Vector3(fMiniX,fMiniY,0.f));
			//LogO("-- Size car:"+toStr(c)+"  x:"+fToStr(fMiniX,2,4)+" y:"+fToStr(fMiniY,2,4)+"  s:"+fToStr(fHudSize,2,4));
		}

		//  gear, vel texts
		{
			//  current viewport max x,y in pixels
			int iwx = (dim.right +1.f)*0.5f*wx,
				iwy = (dim.bottom+1.f)*0.5f*wy;
			int my = (1.f-yMax)*0.5f*wy;  // gauge bottom y

			//  positioning,  min iwy - dont go below viewport bottom
			int vv = pSet->gauges_type > 0 ? -45 : 40;
			int gx = (xcRpm+1.f)*0.5f*wx + 20, gy = std::min(iwy -48, my - 40);
			int vx = (xcVel+1.f)*0.5f*wx + vv, vy = std::min(iwy -48, my - 15);			
			int bx =(xBFuel+1.f)*0.5f*wx - 10, by = std::min(iwy -36, my + 5);
				vx = std::min(vx, iwx -100);
				bx = std::min(bx, iwx -180);  // not too near to vel
			if (hud[c].txGear)  hud[c].txGear->setPosition(gx,gy);
			if (hud[c].txVel)  hud[c].txVel->setPosition(vx,vy);
			if (hud[c].txBFuel)  hud[c].txBFuel->setPosition(bx,by);
			if (hud[c].txDamage)  hud[c].txDamage->setPosition(gx+140,gy+10);
			
			bool hasLaps = pSet->game.local_players > 1 || pSet->game.champ_num >= 0 || mClient;
			int itx = (dim.left+1.f)*0.5f*wx,
				ity = (dim.top +1.f)*0.5f*wy;
			int tx = itx + 0, ty = ity + 32;
			if (hud[c].bckTimes)  hud[c].bckTimes->setPosition(tx,ty);
			tx += 16;  ty += (hasLaps ? 16 : 4);
			if (hud[c].txTimTxt)  hud[c].txTimTxt->setPosition(tx,ty);
			if (hud[c].txTimes)   hud[c].txTimes->setPosition(tx+96,ty);
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

	CreateGraphs();
	
	if (destroy)
	{
		for (int c=0; c < hud.size(); ++c)
		{
			if (hud[c].moMap) {  scm->destroyManualObject(hud[c].moMap);  hud[c].moMap=0;  }
			if (hud[c].ndMap) {  scm->destroySceneNode(hud[c].ndMap);  hud[c].ndMap=0;  }

			for (int i=0; i < 6; ++i)
			{
				if (hud[c].vMoPos[i]) {  scm->destroyManualObject(hud[c].vMoPos[i]);  hud[c].vMoPos[i]=0;  }
				if (hud[c].vNdPos[i]) {  scm->destroySceneNode(hud[c].vNdPos[i]);  hud[c].vNdPos[i]=0;  }
			}
			if (hud[c].moRpmBk) {  scm->destroyManualObject(hud[c].moRpmBk);  hud[c].moRpmBk=0;  }
			if (hud[c].ndRpmBk) {  scm->destroySceneNode(hud[c].ndRpmBk);  hud[c].ndRpmBk=0;  }

			if (hud[c].moVelBk) {  scm->destroyManualObject(hud[c].moVelBk);  hud[c].moVelBk=0;  }
			if (hud[c].ndVelBk) {  scm->destroySceneNode(hud[c].ndVelBk);  hud[c].ndVelBk=0;  }
				
			if (hud[c].moVelBm) {  scm->destroyManualObject(hud[c].moVelBm);  hud[c].moVelBm=0;  }
			if (hud[c].ndVelBm) {  scm->destroySceneNode(hud[c].ndVelBm);  hud[c].ndVelBm=0;  }
				
			if (hud[c].moRpm) {  scm->destroyManualObject(hud[c].moRpm);  hud[c].moRpm=0;  }
			if (hud[c].ndRpm) {  scm->destroySceneNode(hud[c].ndRpm);  hud[c].ndRpm=0;  }
			
			if (hud[c].moVel) {  scm->destroyManualObject(hud[c].moVel);  hud[c].moVel=0;  }
			if (hud[c].ndVel) {  scm->destroySceneNode(hud[c].ndVel);  hud[c].ndVel=0;  }
		}
	}
	for (int c=0; c < hud.size(); ++c)
	{	if (hud[c].txGear) {  mGUI->destroyWidget(hud[c].txGear);  hud[c].txGear = 0;  }
		if (hud[c].txVel)  {  mGUI->destroyWidget(hud[c].txVel);   hud[c].txVel = 0;  }
		if (hud[c].txBFuel){  mGUI->destroyWidget(hud[c].txBFuel);  hud[c].txBFuel = 0;  }
		if (hud[c].txDamage){  mGUI->destroyWidget(hud[c].txDamage);  hud[c].txDamage = 0;  }

		if (hud[c].txTimTxt) {  mGUI->destroyWidget(hud[c].txTimTxt);  hud[c].txTimTxt = 0;  }
		if (hud[c].txTimes)  {  mGUI->destroyWidget(hud[c].txTimes);   hud[c].txTimes = 0;  }
		if (hud[c].bckTimes) {  mGUI->destroyWidget(hud[c].bckTimes);  hud[c].bckTimes = 0;  }
		hud[c].sTimes = "";
	}
	
	//  minimap from road img
	int plr = mSplitMgr->mNumViewports;  // pSet->game.local_players;
	LogO("-- Create Hud  plrs="+toStr(plr));
	asp = 1.f;

	///  reload mini textures
	ResourceGroupManager& resMgr = ResourceGroupManager::getSingleton();
	Ogre::TextureManager& texMgr = Ogre::TextureManager::getSingleton();

	String path = bRplPlay ? 
		PathListTrkPrv(replay.header.track_user, replay.header.track) :
		PathListTrkPrv(pSet->game.track_user, pSet->game.track);
	const String sRoad = "road.png", sTer = "terrain.jpg", sGrp = "TrkMini";
	resMgr.addResourceLocation(path, "FileSystem", sGrp);  // add for this track
	resMgr.unloadResourceGroup(sGrp);
	resMgr.initialiseResourceGroup(sGrp);

	if (sc->ter)
	{	try {  texMgr.unload(sRoad);  texMgr.load(sRoad, sGrp, TEX_TYPE_2D, MIP_UNLIMITED);  }  catch(...) {  }
		try {  texMgr.unload(sTer);   texMgr.load(sTer,  sGrp, TEX_TYPE_2D, MIP_UNLIMITED);  }  catch(...) {  }
	}

	//if (terrain)
	int cnt = std::min(6/**/, (int)carModels.size() -(isGhost2nd?1:0) );  // others
	#ifdef DEBUG
	assert(plr <= hud.size());
	assert(cnt <= hud[0].vMoPos.size());
	#endif
	for (int c=0; c < plr; ++c)  // for each car
	{
		if (sc->ter)
		{	float t = sc->td.fTerWorldSize*0.5;
			minX = -t;  minY = -t;  maxX = t;  maxY = t;  }

		float fMapSizeX = maxX - minX, fMapSizeY = maxY - minY;  // map size
		float size = std::max(fMapSizeX, fMapSizeY*asp);
		scX = 1.f / size;  scY = 1.f / size;

		String sMat = "circle_minimap";
		asp = 1.f;  //_temp
		ManualObject* m = Create2D(sMat,scm,1,true,true);  hud[c].moMap = m;
		//asp = float(mWindow->getWidth())/float(mWindow->getHeight());
		m->setVisibilityFlags(RV_Hud);  m->setRenderQueueGroup(RQG_Hud1);
		
		//  change minimap image
		MaterialPtr mm = MaterialManager::getSingleton().getByName(sMat);
		Pass* pass = mm->getTechnique(0)->getPass(0);
		TextureUnitState* tus = pass->getTextureUnitState(0);
		if (tus)  tus->setTextureName(sc->ter ? sRoad : "alpha.png");
		tus = pass->getTextureUnitState(2);
		if (tus)  tus->setTextureName(sc->ter ? sTer : "alpha.png");
		UpdMiniTer();
		

		float fHudSize = pSet->size_minimap * mSplitMgr->mDims[c].avgsize;
		SceneNode* rt = scm->getRootSceneNode();
		if (!sc->vdr)
		{	hud[c].ndMap = rt->createChildSceneNode(Vector3(0,0,0));
			hud[c].ndMap->attachObject(m);
		}
		//  car pos tri - for all carModels (ghost and remote too)
		for (int i=0; i < cnt; ++i)
		{
			hud[c].vMoPos[i] = Create2D("hud/CarPos", scm, 0.4f, true, true);
			hud[c].vMoPos[i]->setVisibilityFlags(RV_Hud);  hud[c].vMoPos[i]->setRenderQueueGroup(RQG_Hud3);
				  
			hud[c].vNdPos[i] = hud[c].ndMap ? hud[c].ndMap->createChildSceneNode() : hud[0].ndMap->createChildSceneNode();
			hud[c].vNdPos[i]->scale(fHudSize*1.5f, fHudSize*1.5f, 1);
			hud[c].vNdPos[i]->attachObject(hud[c].vMoPos[i]);
		}
		if (hud[c].ndMap)
			hud[c].ndMap->setVisible(false/*pSet->trackmap*/);

	
		//  gauges  backgr  -----------
		String st = toStr(pSet->gauges_type);
		hud[c].moRpmBk = Create2D("hud/rpm"+st,scm,1);	hud[c].moRpmBk->setVisibilityFlags(RV_Hud);
		hud[c].moRpmBk->setRenderQueueGroup(RQG_Hud1);	hud[c].ndRpmBk = rt->createChildSceneNode();
		hud[c].ndRpmBk->attachObject(hud[c].moRpmBk);	hud[c].ndRpmBk->setScale(0,0,0);  hud[c].ndRpmBk->setVisible(false);

		hud[c].moVelBk = Create2D("hud/kmh"+st,scm,1);	hud[c].moVelBk->setVisibilityFlags(RV_Hud);
		hud[c].moVelBk->setRenderQueueGroup(RQG_Hud1);	hud[c].ndVelBk = rt->createChildSceneNode();
		hud[c].ndVelBk->attachObject(hud[c].moVelBk);	hud[c].ndVelBk->setScale(0,0,0);  hud[c].moVelBk->setVisible(false);
			
		hud[c].moVelBm = Create2D("hud/mph"+st,scm,1);	hud[c].moVelBm->setVisibilityFlags(RV_Hud);
		hud[c].moVelBm->setRenderQueueGroup(RQG_Hud1);	hud[c].ndVelBm = rt->createChildSceneNode();
		hud[c].ndVelBm->attachObject(hud[c].moVelBm);	hud[c].ndVelBm->setScale(0,0,0);  hud[c].moVelBm->setVisible(false);
			
		//  gauges  needles
		hud[c].moRpm = Create2D("hud/needle"+st,scm,1,true);  hud[c].moRpm->setVisibilityFlags(RV_Hud);
		hud[c].moRpm->setRenderQueueGroup(RQG_Hud3);	hud[c].ndRpm = rt->createChildSceneNode();
		hud[c].ndRpm->attachObject(hud[c].moRpm);		hud[c].ndRpm->setScale(0,0,0);	hud[c].ndRpm->setVisible(false);
		
		hud[c].moVel = Create2D("hud/needle"+st,scm,1,true);  hud[c].moVel->setVisibilityFlags(RV_Hud);
		hud[c].moVel->setRenderQueueGroup(RQG_Hud3);	hud[c].ndVel = rt->createChildSceneNode();
		hud[c].ndVel->attachObject(hud[c].moVel);		hud[c].ndVel->setScale(0,0,0);	hud[c].ndVel->setVisible(false);


		//  gear, vel text  -----------
		hud[c].txGear = mGUI->createWidget<TextBox>("TextBox",
			0,1200, 160,116, Align::Left, "Back", "Gear"+toStr(c));  hud[c].txGear->setVisible(false);
		hud[c].txGear->setFontName("DigGear");  hud[c].txGear->setFontHeight(126);

		hud[c].txVel = mGUI->createWidget<TextBox>("TextBox",
			0,1200, 360,96, Align::Right, "Back", "Vel"+toStr(c));  hud[c].txVel->setVisible(false);
		hud[c].txVel->setFontName("DigGear");  //hud[c].txVel->setFontHeight(64);
		
		//  boost
		hud[c].txBFuel = mGUI->createWidget<TextBox>("TextBox",
			0,1200, 240,80, Align::Right, "Back", "BFuel"+toStr(c));  hud[c].txBFuel->setVisible(false);
		hud[c].txBFuel->setFontName("DigGear");  hud[c].txBFuel->setFontHeight(64);
		hud[c].txBFuel->setTextColour(Colour(0.6,0.8,1.0));

		hud[c].txDamage = mGUI->createWidget<TextBox>("TextBox",
			0,1200, 240,80, Align::Right, "Back", "Dmg"+toStr(c));  hud[c].txDamage->setVisible(false);
		hud[c].txDamage->setFontName("font.20");  //hud[c].txDamage->setFontHeight(64);
		hud[c].txDamage->setTextColour(Colour(0.7,0.7,0.7));	hud[c].txDamage->setTextShadow(true);
		

		//  times text    -----------
		hud[c].bckTimes = mGUI->createWidget<ImageBox>("ImageBox",
			0,1200, 208,200, Align::Left, "Back", "TimP"+toStr(c));  hud[c].bckTimes->setVisible(false);
		//bckTimes[c]->setAlpha(0.9f);		
		hud[c].bckTimes->setImageTexture("back_times.png");

		hud[c].txTimTxt = mGUI->createWidget<TextBox>("TextBox",
			0,1200, 90,180, Align::Left, "Back", "TimT"+toStr(c));  hud[c].txTimTxt->setVisible(false);
		hud[c].txTimTxt->setFontName("font.20");
		hud[c].txTimTxt->setTextShadow(true);
		bool hasLaps = pSet->game.local_players > 1 || pSet->game.champ_num >= 0 || mClient;
		hud[c].txTimTxt->setCaption(
			(hasLaps ? String("#D0E8F0")+TR("#{TBLap}") : "")+
			"\n#C0E0F0"+TR("#{TBTime}") + 
			"\n#80C0F0"+TR("#{TBLast}") + 
			"\n#80E0E0"+TR("#{TBBest}") +
			"\n#70D070"+TR("#{Track}") +
			"\n#C0C030"+TR("#{TBPosition}") +
			"\n#F0C050"+TR("#{TBPoints}") );

		hud[c].txTimes = mGUI->createWidget<TextBox>("TextBox",
			0,1200, /*80*/100,/*160*/200, Align::Left, "Back", "Tim"+toStr(c));
		hud[c].txTimes->setVisible(false);
		hud[c].txTimes->setFontName("font.20");  hud[c].txTimes->setTextShadow(true);
	}
	///  tex
	resMgr.removeResourceLocation(path, sGrp);


	//  overlays
	OverlayManager& ovr = OverlayManager::getSingleton();
	ovCam = ovr.getByName("Car/CameraOverlay");

	ovAbsTcs = ovr.getByName("Hud/AbsTcs");
	hudAbs = ovr.getOverlayElement("Hud/AbsText");	hudTcs = ovr.getOverlayElement("Hud/TcsText");

	ovCountdown = ovr.getByName("Hud/Countdown");	hudCountdown = ovr.getOverlayElement("Hud/CountdownText");
	ovNetMsg = ovr.getByName("Hud/NetMessages");	hudNetMsg = ovr.getOverlayElement("Hud/NetMessagesText");

	ovOpp = ovr.getByName("Hud/Opponents");		hudOppB = ovr.getOverlayElement("Hud/OpponentsPanel");

	for (int o=0; o < 6; ++o)  for (int c=0; c < 3; ++c)
	{
		hudOpp[o][c] = ovr.getOverlayElement("Hud/OppText"+toStr(o)+"_"+toStr(c));  hudOpp[o][c]->setCaption("");
	}
	for (int o=0; o < cnt; ++o)  // fill car names, not changed during play
	{
		CarModel* cm = carModels[o];
		if (cm->eType != CarModel::CT_REPLAY)
		{
			hudOpp[o][2]->setCaption(cm->sDispName);
			hudOpp[o][2]->setColour(cm->color);
		}
		cm->updTimes = true;
	}

	ovWarnWin = ovr.getByName("Hud/WarnAndWin");
	hudWarnChk = ovr.getOverlayElement("Hud/Warning");	hudWarnChk->setCaption(String(TR("#{WrongChk}")));
	hudWonPlace = ovr.getOverlayElement("Hud/WonPlace");

	//  dbg texts
	ovCarDbg = ovr.getByName("Car/Stats");
	ovCarDbgTxt = ovr.getByName("Car/StatsTxt");
	ovCarDbgExt = ovr.getByName("Car/StatsExt");
	
	for (int i=0; i < ov.size(); ++i)
	{	ov[i].oL = ovr.getOverlayElement("L_"+toStr(i+1));
		ov[i].oR = ovr.getOverlayElement("R_"+toStr(i+1));
		ov[i].oS = ovr.getOverlayElement("S_"+toStr(i+1));
		ov[i].oU = ovr.getOverlayElement("U_"+toStr(i+1));
		ov[i].oX = ovr.getOverlayElement("X_"+toStr(i+1));
	}
	ShowHUD();  //_
	bSizeHUD = true;
	//SizeHUD(true);
}


//  HUD show/hide
//---------------------------------------------------------------------------------------------------------------
void App::ShowHUD(bool hideAll)
{
	if (hideAll || iLoad1stFrames >= 0)  // still loading
	{
		if (ovAbsTcs) ovAbsTcs->hide();		if (ovCam)	 ovCam->hide();
		if (ovNetMsg)  ovNetMsg->hide();	if (ovCountdown)  ovCountdown->hide();
		if (ovCarDbg)  ovCarDbg->hide();	if (ovCarDbgTxt)  ovCarDbgTxt->hide();
		if (ovWarnWin)  ovWarnWin->hide();	if (ovOpp)  ovOpp->hide();
		if (mFpsOverlay)  mFpsOverlay->hide();

		for (int c=0; c < hud.size(); ++c)
		{
			if (hud[c].txGear)  hud[c].txGear->setVisible(false);
			if (hud[c].txVel)   hud[c].txVel->setVisible(false);
			if (hud[c].txBFuel)   hud[c].txBFuel->setVisible(false);
			if (hud[c].txDamage)  hud[c].txDamage->setVisible(false);

			if (hud[c].ndRpmBk)  hud[c].ndRpmBk->setVisible(false);
			if (hud[c].ndVelBk)  hud[c].ndVelBk->setVisible(false);
			if (hud[c].ndVelBm)  hud[c].ndVelBm->setVisible(false);

			if (hud[c].ndRpm)  hud[c].ndRpm->setVisible(false);
			if (hud[c].ndVel)  hud[c].ndVel->setVisible(false);
			if (hud[c].ndMap)  hud[c].ndMap->setVisible(false);

			if (hud[c].bckTimes)  hud[c].bckTimes->setVisible(false);
			if (hud[c].txTimTxt)  hud[c].txTimTxt->setVisible(false);
			if (hud[c].txTimes)   hud[c].txTimes->setVisible(false);
		}
		hideMouse();
		if (mWndRpl)  mWndRpl->setVisible(false);
	}
	else
	{	//this goes each frame..
		bool show = pSet->show_gauges, times = pSet->show_times;
		bool bfuel = pSet->game.boost_type == 1 || pSet->game.boost_type == 2;
		bool bdmg = pSet->game.damage_type > 0;
		
		if (ovCountdown)  if (show)  ovCountdown->show();  else  ovCountdown->hide();
		if (ovNetMsg)	if (show)  ovNetMsg->show();  else  ovNetMsg->hide();
		if (ovAbsTcs){  if (show)  ovAbsTcs->show();  else  ovAbsTcs->hide();  }

		show = pSet->car_dbgbars;
		if (ovCarDbg){  if (show)  ovCarDbg->show();  else  ovCarDbg->hide();  }
		show = pSet->car_dbgtxt || pSet->bltProfilerTxt || pSet->profilerTxt;
		if (ovCarDbgTxt){  if (show)  ovCarDbgTxt->show();  else  ovCarDbgTxt->hide();  }
		show = pSet->car_dbgsurf;
		if (ovCarDbgExt){  if (show)  ovCarDbgExt->show();  else  ovCarDbgExt->hide();  }

		if (ovCam)	{  if (pSet->show_cam && !isFocGui)  ovCam->show();  else  ovCam->hide();  }
		if (ovOpp)  {  if (pSet->show_opponents && (!sc->ter || road && road->getNumPoints() > 0))  ovOpp->show();  else  ovOpp->hide();  }
		if (ovWarnWin){  if (pSet->show_times)  ovWarnWin->show();  else  ovWarnWin->hide();  }
		if (mFpsOverlay){  if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();  }

		show = pSet->show_gauges;
		for (int c=0; c < hud.size(); ++c)
		{
			if (hud[c].txGear)  hud[c].txGear->setVisible(pSet->show_digits);
			if (hud[c].txVel)   hud[c].txVel->setVisible(pSet->show_digits);
			if (hud[c].txBFuel)   hud[c].txBFuel->setVisible(show && bfuel);
			if (hud[c].txDamage)  hud[c].txDamage->setVisible(show && bdmg);

			if (hud[c].ndRpmBk)  hud[c].ndRpmBk->setVisible(show);
			if (hud[c].ndVelBk)  hud[c].ndVelBk->setVisible(show && !pSet->show_mph);
			if (hud[c].ndVelBm)  hud[c].ndVelBm->setVisible(show && pSet->show_mph);

			if (hud[c].ndRpm)  hud[c].ndRpm->setVisible(show);
			if (hud[c].ndVel)  hud[c].ndVel->setVisible(show);
			if (hud[c].ndMap)  hud[c].ndMap->setVisible(pSet->trackmap);
			
			if (hud[c].bckTimes)  hud[c].bckTimes->setVisible(times);
			if (hud[c].txTimTxt)  hud[c].txTimTxt->setVisible(times);
			if (hud[c].txTimes)   hud[c].txTimes->setVisible(times);
		}
		updMouse();
		if (mWndRpl && !bLoading)  mWndRpl->setVisible(bRplPlay && bRplWnd);  //
	}
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
		if (ovOpp)  ovOpp->hide();			if (ovWarnWin)  ovWarnWin->hide();
		if (ovCarDbg)  ovCarDbg->hide();	if (ovCarDbgTxt)  ovCarDbgTxt->hide();
		if (ovCountdown)  ovCountdown->hide();  if (ovNetMsg)  ovNetMsg->hide();
		if (hudAbs)  hudAbs->hide();		if (hudTcs)  hudTcs->hide();
	}else{
		/// for render viewport ---------
		if (ovCam)  ovCam->hide();
		if (mFpsOverlay)  mFpsOverlay->hide();
	}
}


//  HUD utils
//---------------------------------------------------------------------------------------------------------------
void App::UpdMiniTer()
{
	MaterialPtr mm = MaterialManager::getSingleton().getByName("circle_minimap");
	Pass* pass = mm->getTechnique(0)->getPass(0);
	if (!pass)  return;
	try
	{	Ogre::GpuProgramParametersSharedPtr par = pass->getFragmentProgramParameters();
		if (par->_findNamedConstantDefinition("showTerrain",false))
			par->setNamedConstant("showTerrain", pSet->mini_terrain && sc->ter ? 1.f : 0.f);
		if (par->_findNamedConstantDefinition("showBorder",false))
			par->setNamedConstant("showBorder", pSet->mini_border && sc->ter ? 1.f : 0.f);
		if (par->_findNamedConstantDefinition("square",false))
			par->setNamedConstant("square", pSet->mini_zoomed && sc->ter ? 0.f : 1.f);
	}
	catch(...){  }
}


Vector3 App::projectPoint(const Camera* cam, const Vector3& pos)
{
	Vector3 pos2D = cam->getProjectionMatrix() * (cam->getViewMatrix() * pos);

	//Real x = std::min(1.f, std::max(0.f,  pos2D.x * 0.5f + 0.5f ));  // leave on screen edges
	//Real y = std::min(1.f, std::max(0.f, -pos2D.y * 0.5f + 0.5f ));
	Real x =  pos2D.x * 0.5f + 0.5f;
	Real y = -pos2D.y * 0.5f + 0.5f;
	bool out = !cam->isVisible(pos);

	return Vector3(x * mWindow->getWidth(), y * mWindow->getHeight(), out ? -1.f : 1.f);
}

TextBox* App::CreateNickText(int carId, String text)
{
	TextBox* txt = mGUI->createWidget<TextBox>("TextBox",
		100,100, 360,32, Align::Center, "Back", "NickTxt"+toStr(carId));
	txt->setVisible(false);
	txt->setFontHeight(28);  //par 24..32
	txt->setTextShadow(true);  txt->setTextShadowColour(Colour::Black);
	txt->setCaption(text);
	return txt;
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
