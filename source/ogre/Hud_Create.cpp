#include "pch.h"
#include "common/Def_Str.h"
#include "common/data/SceneXml.h"
#include "common/CScene.h"
#include "common/RenderConst.h"
#include "common/GuiCom.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "SplitScreen.h"

#include <OgreRenderWindow.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreManualObject.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreOverlayManager.h>
#include <OgreOverlayElement.h>
using namespace Ogre;
using namespace MyGUI;


///---------------------------------------------------------------------------------------------------------------
///  HUD create
///---------------------------------------------------------------------------------------------------------------

void CHud::Create()
{
	//Destroy();  //
	if (app->carModels.size() == 0)  return;

	Ogre::Timer ti;

	SceneManager* scm = app->mSplitMgr->mGuiSceneMgr;
	if (hud[0].moMap || hud[0].txVel || hud[0].txTimes)
		LogO("Create Hud: exists !");

	app->CreateGraphs();
		
	//  minimap from road img
	int plr = app->mSplitMgr->mNumViewports;  // pSet->game.local_players;
	LogO("-- Create Hud  plrs="+toStr(plr));
	asp = 1.f;

	///  reload mini textures
	ResourceGroupManager& resMgr = ResourceGroupManager::getSingleton();
	Ogre::TextureManager& texMgr = Ogre::TextureManager::getSingleton();

	String path = app->bRplPlay ? 
		app->gcom->PathListTrkPrv(app->replay.header.track_user, app->replay.header.track) :
		app->gcom->PathListTrkPrv(pSet->game.track_user, pSet->game.track);
	const String sRoad = "road.png", sTer = "terrain.jpg", sGrp = "TrkMini";
	resMgr.addResourceLocation(path, "FileSystem", sGrp);  // add for this track
	resMgr.unloadResourceGroup(sGrp);
	resMgr.initialiseResourceGroup(sGrp);

	Scene* sc = app->scn->sc;
	if (sc->ter)
	{	try {  texMgr.unload(sRoad);  texMgr.load(sRoad, sGrp, TEX_TYPE_2D, MIP_UNLIMITED);  }  catch(...) {  }
		try {  texMgr.unload(sTer);   texMgr.load(sTer,  sGrp, TEX_TYPE_2D, MIP_UNLIMITED);  }  catch(...) {  }
	}

	//if (terrain)
	int cnt = std::min(6/**/, (int)app->carModels.size() );  // others
	#ifdef DEBUG
	assert(plr <= hud.size());
	//assert(cnt <= hud[0].moPos.size());
	#endif
	int y=1200; //off 0

	
	//  car pos tris (form all cars on all viewports)
	SceneNode* rt = scm->getRootSceneNode();
	asp = 1.f;  //_temp
	moPos = Create2D("hud/CarPos", scm, 0.f, true,true, 1.f,Vector2(1,1), RV_Hud,RQG_Hud3, plr * 6);
	ndPos = rt->createChildSceneNode();
	ndPos->attachObject(moPos);


	//  for each car
	for (int c=0; c < plr; ++c)
	{
		String s = toStr(c);
		Hud& h = hud[c];
		CarModel* cm = app->carModels[c];
		if (sc->ter)
		{	float t = sc->td.fTerWorldSize*0.5;
			minX = -t;  minY = -t;  maxX = t;  maxY = t;
		}
		float fMapSizeX = maxX - minX, fMapSizeY = maxY - minY;  // map size
		float size = std::max(fMapSizeX, fMapSizeY*asp);
		scX = 1.f / size;  scY = 1.f / size;
		
		//  change minimap image
		String sMat = "circle_minimap";
		MaterialPtr mm = MaterialManager::getSingleton().getByName(sMat);
		Pass* pass = mm->getTechnique(0)->getPass(0);
		TextureUnitState* tus = pass->getTextureUnitState(0);
		if (tus)  tus->setTextureName(sc->ter ? sRoad : "alpha.png");
		tus = pass->getTextureUnitState(2);
		if (tus)  tus->setTextureName(sc->ter ? sTer : "alpha.png");
		UpdMiniTer();
		
		float fHudSize = pSet->size_minimap * app->mSplitMgr->mDims[c].avgsize;
		h.ndMap = rt->createChildSceneNode();
		asp = 1.f;  //_temp
		ManualObject* m = Create2D(sMat,scm,1, true,true, 1.f,Vector2(1,1), RV_Hud,RQG_Hud1);  h.moMap = m;
		h.ndMap->attachObject(m);
		//asp = float(mWindow->getWidth())/float(mWindow->getHeight());
		h.ndMap->setVisible(false/*pSet->trackmap*/);

	
		//  gauges  backgr  -----------
		String st = toStr(pSet->gauges_type);
		h.moGauges = Create2D("hud/"+st,scm, 1.f, true,false, 0.f,Vector2(0.f,0.5f), RV_Hud,RQG_Hud1, 2);
		h.ndGauges = rt->createChildSceneNode();  h.ndGauges->attachObject(h.moGauges);  h.ndGauges->setVisible(false);

		//  gauges  needles
		h.moNeedles = Create2D("hud/"+st,scm, 1.f, true,false, 0.f,Vector2(0.5f,0.5f), RV_Hud,RQG_Hud3, 2);
		h.ndNeedles = rt->createChildSceneNode();  h.ndNeedles->attachObject(h.moNeedles);  h.ndNeedles->setVisible(false);


		///  GUI
		//  gear  text  -----------
		h.parent = app->mGui->createWidget<Widget>("",0,0,pSet->windowx,pSet->windowy,Align::Left,"Back","main"+s);

		if (cm->vtype == V_Car)
		{
			h.txGear = h.parent->createWidget<TextBox>("TextBox",
				0,y, 160,116, Align::Left, "Gear"+s);  h.txGear->setVisible(false);
			h.txGear->setFontName("DigGear");  h.txGear->setFontHeight(126);
			//h.txGear->setTextShadow(true);
		}
		
		//  vel
		/*h.bckVel = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 130,82, Align::Left, "IVel"+s);
		h.bckVel->setImageTexture("back_vel.png");
		h.bckVel->setAlpha(0.7f);*/
		
		//h.txVel = h.bckVel->createWidget<TextBox>("TextBox",
		//	10,5, 360,96, Align::Right, "Vel"+s);  h.txVel->setVisible(false);
		h.txVel = h.parent->createWidget<TextBox>("TextBox",
			0,y, 360,96, Align::Right, "Vel"+s);  h.txVel->setVisible(false);
		h.txVel->setFontName("DigGear");  //h.txVel->setFontHeight(64);
		//h.txVel->setInheritsAlpha(false);
		//h.txVel->setTextShadow(true);
		
		//  boost
		if (cm->vtype != V_Sphere)
		{
			h.txBFuel = h.parent->createWidget<TextBox>("TextBox",
				0,y, 120,80, Align::Right, "Fuel"+s);  h.txBFuel->setVisible(false);
			h.txBFuel->setTextAlign(Align::Right|Align::VCenter);
			h.txBFuel->setFontName("DigGear");  h.txBFuel->setFontHeight(72);
			h.txBFuel->setTextColour(Colour(0.6,0.8,1.0));  //h.txBFuel->setTextShadow(true);

			h.icoBFuel = h.parent->createWidget<ImageBox>("ImageBox",
				0,y, 40,40, Align::Left, "IFuel"+s);  //h.icoBFuel->setVisible(false);
			h.icoBFuel->setImageTexture("gui_icons.png");
			//if (pSet->game.boost_type == 3)
			h.icoBFuel->setImageCoord(IntCoord(512,0,128,128));

			//h.icoBInf = h.parent->createWidget<ImageBox>("ImageBox",
			//	0,y, 40,40, Align::Right, "IInf"+s);
			//h.icoBInf->setImageTexture("gui_icons.png");
			//h.icoBInf->setImageCoord(IntCoord(512,768,128,128));
		}

		//  damage %
		if (pSet->game.damage_type > 0)
		{
			h.imgDamage = h.parent->createWidget<ImageBox>("ImageBox",
				0,y, 130,46, Align::Left, "PDmg"+s);
			h.imgDamage->setImageTexture("menu2.png");
			
			h.txDamage = h.parent->createWidget<TextBox>("TextBox",
				0,y, 120,40, Align::Right, "Dmg"+s);  //h.txDamage->setVisible(false);
			h.txDamage->setTextAlign(Align::Right|Align::VCenter);
			h.txDamage->setFontName("hud.replay");  //h.txDamage->setFontHeight(64);
			h.txDamage->setTextColour(Colour(0.7,0.7,0.7));  h.txDamage->setTextShadow(true);

			h.icoDamage = h.parent->createWidget<ImageBox>("ImageBox",
				0,y, 40,40, Align::Left, "IDmg"+s);  //h.icoDamage->setVisible(false);
			h.icoDamage->setImageTexture("gui_icons.png");
			if (pSet->game.damage_type == 1)
				h.icoDamage->setImageCoord(IntCoord(512,256,128,128));
			else
				h.icoDamage->setImageCoord(IntCoord(640,384,128,128));
		}
		
		//  rewind <<
		#if 0
		h.txRewind = h.parent->createWidget<TextBox>("TextBox",
			0,y, 240,80, Align::Right, "Rew"+s);  //h.txRewind->setVisible(false);
		h.txRewind->setFontName("DigGear");  h.txRewind->setFontHeight(64);
		h.txRewind->setTextColour(Colour(0.9,0.7,1.0));
		h.txRewind->setCaption("3.0");

		h.icoRewind = h.parent->createWidget<ImageBox>("ImageBox",
			200,180, 40,40, Align::Left, "IRew"+s);  //h.icoRewind->setVisible(false);
		h.icoRewind->setImageTexture("gui_icons.png");
		h.icoRewind->setImageCoord(IntCoord(512,384,128,128));
		#endif


		///  times text  ----------------------
		/*h.bckTimes = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 356,260, Align::Left, "TimP"+s);  h.bckTimes->setVisible(false);
		h.bckTimes->setAlpha(0.f);
		h.bckTimes->setImageTexture("back_times.png");*/
		bool hasLaps = pSet->game.local_players > 1 || pSet->game.champ_num >= 0 || pSet->game.chall_num >= 0 || app->mClient;

		h.txTimTxt = h.parent->createWidget<TextBox>("TextBox",
			0,y, 120,260, Align::Left, "TimT"+s);
		h.txTimTxt->setFontName("hud.times");  h.txTimTxt->setTextShadow(true);
		h.txTimTxt->setInheritsAlpha(false);
		h.txTimTxt->setCaption(
			(hasLaps ? String("#90D0C0")+TR("#{TBLap}") : "")+
			"\n#A0E0E0"+TR("#{TBTime}") +
			"\n#70D070"+TR("#{Track}") +
			"\n#C0C030"+TR("#{TBPosition}") +
			"\n#F0C050"+TR("#{TBPoints}") +
			"\n#C8A898"+TR("#{Progress}") );

		h.txTimes = h.parent->createWidget<TextBox>("TextBox",
			0,y, 230,260, Align::Left, "Tim"+s);
		h.txTimes->setInheritsAlpha(false);
		h.txTimes->setFontName("hud.times");  h.txTimes->setTextShadow(true);


		///  lap results  ----------------------
		h.bckLap = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 320,210, Align::Left, "LapP"+s);  h.bckLap->setVisible(false);
		h.bckLap->setColour(Colour(0.4,0.4,0.4));
		h.bckLap->setAlpha(0.5f);
		h.bckLap->setImageTexture("back_times.png");

		h.txLapTxt = h.parent->createWidget<TextBox>("TextBox",
			0,y, 120,300, Align::Left, "LapT"+s);
		h.txLapTxt->setFontName("hud.times");  h.txLapTxt->setTextShadow(true);
		h.txLapTxt->setInheritsAlpha(false);
		h.txLapTxt->setCaption(//String("\n")+
			//(hasLaps ? String("#D0F8F0")+TR("#{TBLap}") : "")+
			"\n#80C0F0"+TR("#{TBLast}") +
			"\n#80E0E0"+TR("#{TBBest}") +
			"\n#70D070"+TR("#{Track}") +
			"\n#C0C030"+TR("#{TBPosition}") +
			"\n#F0C050"+TR("#{TBPoints}") );
		h.txLapTxt->setVisible(false);

		h.txLap = h.parent->createWidget<TextBox>("TextBox",
			0,y, 230,320, Align::Left, "Lap"+s);
		h.txLap->setInheritsAlpha(false);
		h.txLap->setFontName("hud.times");  h.txLap->setTextShadow(true);
		h.txLap->setVisible(false);


		//  opp list  -----------
		h.bckOpp = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 224,200, Align::Left, "OppB"+toStr(c));
		h.bckOpp->setAlpha(0.9f);  h.bckOpp->setVisible(false);
		h.bckOpp->setImageTexture("opp_rect.png");

		for (int n=0; n < 3; ++n)
		{
			h.txOpp[n] = h.parent->createWidget<TextBox>("TextBox",
				n*80+10,0, 90,180, n == 2 ? Align::Left : Align::Right, "Opp"+toStr(n)+s);
			h.txOpp[n]->setFontName("hud.text");  h.txOpp[n]->setVisible(false);
			if (n==0)  h.txOpp[n]->setTextShadow(true);
		}
		h.lastOppH = -1;  // upd size

		//  wrong chk warning  -----------
		h.bckWarn = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 400,60, Align::Left, "WarnB"+s);  h.bckWarn->setVisible(false);
		h.bckWarn->setImageTexture("back_times.png");

		h.txWarn = h.bckWarn->createWidget<TextBox>("TextBox",
			0,0, 400,60, Align::Left, "WarnT"+s);
		h.txWarn->setFontName("hud.text");  h.txWarn->setTextShadow(true);
		h.txWarn->setTextColour(Colour(1,0.2,0));  h.txWarn->setTextAlign(Align::Center);
		h.txWarn->setCaption(TR("#{WrongChk}"));

		//  win place  -----------
		h.bckPlace = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 400,60, Align::Left, "PlcB"+s);  h.bckPlace->setVisible(false);
		h.bckPlace->setImageTexture("back_times.png");

		h.txPlace = h.bckPlace->createWidget<TextBox>("TextBox",
			0,0, 400,60, Align::Left, "PlcT"+s);
		h.txPlace->setFontName("hud.text");  h.txPlace->setTextShadow(true);
		h.txPlace->setTextAlign(Align::Center);


		//  start countdown
		h.txCountdown = h.parent->createWidget<TextBox>("TextBox",
			0,y, 200,120, Align::Left, "CntT"+s);  h.txCountdown->setVisible(false);
		h.txCountdown->setFontName("DigGear");  h.txCountdown->setTextShadow(true);
		h.txCountdown->setTextColour(Colour(0.8,0.9,1));  h.txCountdown->setTextAlign(Align::Center);

		//  abs, tcs
		h.txAbs = h.parent->createWidget<TextBox>("TextBox",
			0,y, 120,60, Align::Left, "AbsT"+s);
		h.txAbs->setFontName("hud.text");  h.txAbs->setTextShadow(true);
		h.txAbs->setCaption("ABS");  h.txAbs->setTextColour(Colour(1,1,0.6));

		h.txTcs = h.parent->createWidget<TextBox>("TextBox",
			0,y, 120,60, Align::Left, "TcsT"+s);
		h.txTcs->setFontName("hud.text");  h.txTcs->setTextShadow(true);
		h.txTcs->setCaption("TCS");  h.txTcs->setTextColour(Colour(0.6,1,1));

		//  camera name
		h.txCam = h.parent->createWidget<TextBox>("TextBox",
			0,0, 300,30, Align::Left, "CamT"+s);
		h.txCam->setFontName("hud.text");  h.txCam->setTextShadow(true);
		h.txCam->setTextColour(Colour(0.65,0.85,0.85));

		//  input bars  --------------
		/*Img bar = tabitem->createWidget<ImageBox>("ImageBox",
			x2 + (twosided ? 0 : 64), y+4, twosided ? 128 : 64, 16, Align::Default,
			"bar_" + toStr(i) + "_" + sPlr);
		gcom->setOrigPos(bar, "OptionsWnd");
		bar->setUserData(*it);
		bar->setImageTexture(String("input_bar.png"));  bar->setImageCoord(IntCoord(0,0,128,16));
		*/
	}

	//  camera info
	txCamInfo = app->mGui->createWidget<TextBox>("TextBox",
		0,y, 900,100, Align::Left, "Back", "CamIT");  txCamInfo->setVisible(false);
	txCamInfo->setFontName("hud.text");  txCamInfo->setTextShadow(true);
	txCamInfo->setTextColour(Colour(0.8,0.9,0.9));


	//  chat msg  -----------
	bckMsg = app->mGui->createWidget<ImageBox>("ImageBox",
		0,y, 600,80, Align::Left, "Back", "MsgB");  bckMsg->setVisible(false);
	bckMsg->setAlpha(0.9f);
	bckMsg->setColour(Colour(0.5,0.5,0.5));
	bckMsg->setImageTexture("back_times.png");

	txMsg = bckMsg->createWidget<TextBox>("TextBox",
		16,10, 800,80, Align::Left, "PlcT");
	txMsg->setInheritsAlpha(false);
	txMsg->setFontName("hud.text");  txMsg->setTextShadow(true);
	txMsg->setTextColour(Colour(0.95,0.95,1.0));


	///  tex
	resMgr.removeResourceLocation(path, sGrp);
	
	//-  cars need update
	for (int i=0; i < app->carModels.size(); ++i)
	{	CarModel* cm = app->carModels[i];
		cm->updTimes = true;
		cm->updLap = true;  cm->fLapAlpha = 1.f;
	}

	
	///  tire vis circles  + + + +
	asp = float(app->mWindow->getWidth())/float(app->mWindow->getHeight());

	if (pSet->car_tirevis)
	{	SceneNode* rt = scm->getRootSceneNode();
		for (int i=0; i < 4; ++i)
		{
			ManualObject* m = app->mSceneMgr->createManualObject();
			m->setDynamic(true);
			m->setUseIdentityProjection(true);
			m->setUseIdentityView(true);
			m->setCastShadows(false);

			m->estimateVertexCount(32);
			m->begin("hud/line", RenderOperation::OT_LINE_LIST);
			m->position(-1,0, 0);  m->colour(1,1,1);
			m->position( 1,0, 0);  m->colour(1,1,1);
			m->end();
		 
			AxisAlignedBox aabInf;	aabInf.setInfinite();
			m->setBoundingBox(aabInf);  // always visible
			m->setVisibilityFlags(RV_Hud);
			m->setRenderQueueGroup(RQG_Hud1);

			moTireVis[i] = m;
			ndTireVis[i] = rt->createChildSceneNode();  ndTireVis[i]->attachObject(moTireVis[i]);
			ndTireVis[i]->setPosition((i%2 ? 1.f :-1.f) * 0.13f - 0.7f,
									  (i/2 ?-1.f : 1.f) * 0.22f - 0.6f, 0.f);
			const Real s = 0.06f;  // par
			ndTireVis[i]->setScale(s, s*asp, 1.f);
	}	}


	//  dbg texts
	OverlayManager& ovr = OverlayManager::getSingleton();
	ovCarDbg = ovr.getByName("Car/Stats");
	ovCarDbgTxt = ovr.getByName("Car/StatsTxt");
	ovCarDbgExt = ovr.getByName("Car/StatsExt");
	
	for (int i=0; i < ov.size(); ++i)
	{	String s = toStr(i+1);
		ov[i].oL = ovr.getOverlayElement("L_"+s);	ov[i].oR = ovr.getOverlayElement("R_"+s);
		ov[i].oS = ovr.getOverlayElement("S_"+s);	ov[i].oU = ovr.getOverlayElement("U_"+s);
		ov[i].oX = ovr.getOverlayElement("X_"+s);
	}

	Show();  //_
	app->bSizeHUD = true;
	//Size();
	
	LogO("::: Time Create Hud: "+fToStr(ti.getMilliseconds(),0,3)+" ms");
}



//  HUD destroy
//---------------------------------------------------------------------------------------------------------------
CHud::OvrDbg::OvrDbg() :
	oL(0),oR(0),oS(0), oU(0),oX(0)
{	}

CHud::Hud::Hud()
{
	vMiniPos.resize(6);
}

void CHud::Destroy()
{
	SceneManager* scm = app->mSplitMgr->mGuiSceneMgr;
	int i,c;
	for (c=0; c < hud.size(); ++c)
	{	Hud& h = hud[c];

		#define Dest2(mo,nd)  {  \
			if (mo) {  scm->destroyManualObject(mo);  mo=0;  } \
			if (nd) {  scm->destroySceneNode(nd);  nd=0;  }  }
		
		Dest2(h.moMap,h.ndMap)
		Dest2(h.moGauges,h.ndGauges)
		Dest2(h.moNeedles,h.ndNeedles)

		#define Dest(w)  \
			if (w) {  app->mGui->destroyWidget(w);  w = 0;  }
			
		Dest(h.txGear)  Dest(h.txVel)  Dest(h.bckVel)
		Dest(h.txAbs)  Dest(h.txTcs)  Dest(h.txCam)
		
		Dest(h.txBFuel)  Dest(h.txDamage)  Dest(h.txRewind)  Dest(h.imgDamage)
		Dest(h.icoBFuel)  Dest(h.icoBInf)  Dest(h.icoDamage)  Dest(h.icoRewind)

		for (i=0; i < 3; ++i)  Dest(h.txOpp[i])
		Dest(h.bckOpp)
		Dest(h.txTimTxt)  Dest(h.txTimes)  //Dest(h.bckTimes)
		Dest(h.txLapTxt)  Dest(h.txLap)  Dest(h.bckLap)
		h.sTimes = "";  h.sLap = "";
		
		Dest(h.txWarn)  Dest(h.bckWarn)
		Dest(h.txPlace)  Dest(h.bckPlace)
		Dest(h.txCountdown)
	}
	Dest2(moPos, ndPos)
	Dest(txMsg)  Dest(bckMsg)
	Dest(txCamInfo)
	
	for (i=0; i < 4; ++i)
		Dest2(moTireVis[i],ndTireVis[i])
}


void CHud::Arrow::Create(SceneManager* mSceneMgr, SETTINGS* pSet)
{
	if (!node)  node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	if (ent)  return;
	ent = mSceneMgr->createEntity("CheckpointArrow", "arrow.mesh");
	ent->setRenderQueueGroup(RQG_Hud3);
	ent->setCastShadows(false);
	nodeRot = node->createChildSceneNode();
	nodeRot->attachObject(ent);
	nodeRot->setScale(pSet->size_arrow/2.f * Vector3::UNIT_SCALE);
	ent->setVisibilityFlags(RV_Hud);
	nodeRot->setVisible(pSet->check_arrow);
}
