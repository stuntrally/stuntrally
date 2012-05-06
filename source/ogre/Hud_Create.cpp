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

	CreateGraphs();
	
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


//  HUD show/hide
//---------------------------------------------------------------------------------------------------------------
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


//  HUD utils
//---------------------------------------------------------------------------------------------------------------
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
