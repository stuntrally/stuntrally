#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../road/PaceNotes.h"
#include "../paged-geom/PagedGeometry.h"
#include "../ogre/common/MultiList2.h"
#include "../ogre/common/RenderBoxScene.h"
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
//#include <LinearMath/btDefaultMotionState.h>
//#include <BulletDynamics/Dynamics/btRigidBody.h>
#include "../shiny/Main/Factory.hpp"
#include "../sdl4ogre/sdlinputwrapper.hpp"
#include "../paged-geom/GrassLoader.h"
#include <MyGUI.h>
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreParticleEmitter.h>
#include <OgreParticleSystem.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
using namespace Ogre;


///  Mouse
//---------------------------------------------------------------------------------------------------------------
void App::processMouse(double fDT)
{
	//  static vars are smoothed
	static Radian sYaw(0), sPth(0);
	static Vector3 sMove(0,0,0);
	static double time = 0.0;
	time += fDT;
	
	const double ivDT = 0.004;  // const interval
	while (time > ivDT)
	{	time -= ivDT;
	
		Vector3 vInpC(0,0,0),vInp;
		Real fSmooth = (powf(1.0f - pSet->cam_inert, 2.2f) * 40.f + 0.1f) * ivDT;

		const Real sens = 0.13;
		if (bCam())
			vInpC = Vector3(mx, my, 0)*sens;
		vInp = Vector3(mx, my, 0)*sens;  mx = 0;  my = 0;
		vNew += (vInp-vNew) * fSmooth;

		if (mbMiddle){	mTrans.z += vInpC.y * 1.6f;  }  //zoom
		if (mbRight){	mTrans.x += vInpC.x;  mTrans.y -= vInpC.y;  }  //pan
		if (mbLeft){	mRotX -= vInpC.x;  mRotY -= vInpC.y;  }  //rot

		Real cs = pSet->cam_speed;  Degree cr(pSet->cam_speed);
		Real fMove = 100*cs;  //par speed
		Degree fRot = 300*cr, fkRot = 160*cr;

		Radian inYaw = rotMul * ivDT * (fRot* mRotX + fkRot* mRotKX);
		Radian inPth = rotMul * ivDT * (fRot* mRotY + fkRot* mRotKY);
		Vector3 inMove = moveMul * ivDT * (fMove * mTrans);

		sYaw += (inYaw - sYaw) * fSmooth;
		sPth += (inPth - sPth) * fSmooth;
		sMove += (inMove - sMove) * fSmooth;

		mCamera->yaw( sYaw );
		mCamera->pitch( sPth );
		mCamera->moveRelative( sMove );
	}
}


//---------------------------------------------------------------------------------------------------------------
//  frame events
//---------------------------------------------------------------------------------------------------------------
bool App::frameEnded(const FrameEvent& evt)
{
	//  show when in gui on generator subtab
	if (ovTerPrv)
	if (bGuiFocus && mWndEdit &&
		mWndEdit->getVisible() && mWndTabsEdit->getIndexSelected() == TAB_Terrain &&
		gui->vSubTabsEdit.size() > TAB_Terrain && gui->vSubTabsEdit[TAB_Terrain]->getIndexSelected() == 2/**/)
		ovTerPrv->show();  else  ovTerPrv->hide();

	//  track events
	if (eTrkEvent != TE_None)
	{	switch (eTrkEvent)  {
			case TE_Load:	LoadTrackEv();  break;
			case TE_Save:	SaveTrackEv();  break;
			case TE_Update: UpdateTrackEv();  break;  }
		eTrkEvent = TE_None;
	}
	
	///  input
	mInputWrapper->capture(false);

	//  road pick
	SplineRoad* road = scn->road;
	if (road)
	{
		const MyGUI::IntPoint& mp = MyGUI::InputManager::getInstance().getMousePosition();
		Real mx = Real(mp.left)/mWindow->getWidth(), my = Real(mp.top)/mWindow->getHeight();
		bool setpos = edMode >= ED_Road || !brLockPos,
			hide = !(edMode == ED_Road && bEdit());
		road->Pick(mCamera, mx, my,  setpos, edMode == ED_Road, hide);
	}

	EditMouse();  // edit


	///<>  Ter upd	- - -
	static int tu = 0, bu = 0;
	if (tu >= pSet->ter_skip)
	if (bTerUpd)
	{	bTerUpd = false;  tu = 0;
		if (scn->mTerrainGroup)
			scn->mTerrainGroup->update();
	}	tu++;

	if (bu >= pSet->ter_skip)
	if (bTerUpdBlend)
	{	bTerUpdBlend = false;  bu = 0;
		//if (terrain)
			scn->UpdBlendmap();
	}	bu++;


	///<>  Edit Ter
	TerCircleUpd();
	bool def = false;
	static bool defOld = false;
	float gd = scn->sc->densGrass;
	static float gdOld = scn->sc->densGrass;

	if (scn->terrain && road && bEdit() && road->bHitTer)
	{
		float dt = evt.timeSinceLastFrame;
		Real s = shift ? 0.25 : ctrl ? 4.0 :1.0;
		switch (edMode)
		{
		case ED_Deform:
			if (mbLeft) {  def = true;  deform(road->posHit, dt, s);  }else
			if (mbRight){  def = true;  deform(road->posHit, dt,-s);  }
			break;
		case ED_Filter:
			if (mbLeft) {  def = true;  filter(road->posHit, dt, s);  }
			break;
		case ED_Smooth:
			if (mbLeft) {  def = true;  smooth(road->posHit, dt);  }
			break;
		case ED_Height:
			if (mbLeft) {  def = true;  height(road->posHit, dt, s);  }
			break;
		}
	}

#if 0
if (pSet->bTrees)
{
	///  upd grass
	if (gd != gdOld)
	{
		Real fGrass = pSet->grass * scn->sc->densGrass * 3.0f;
		for (int i=0; i < scn->sc->ciNumGrLay; ++i)
		{
			const SGrassLayer* gr = &scn->sc->grLayersAll[i];
			if (gr->on)
			{
				Forests::GrassLayer *l = gr->grl;
				if (l)
				{	l->setDensity(gr->dens * fGrass);
					scn->grass->reloadGeometry();
				}
			}
		}
	}

	if (!def && defOld)
	{
		scn->UpdGrassDens();
		//if (grd.rnd)
		//	grd.rnd->update();

		for (int i=0; i < scn->sc->ciNumGrLay; ++i)
		{
			const SGrassLayer* gr = &scn->sc->grLayersAll[i];
			if (gr->on)
			{
				Forests::GrassLayer *l = gr->grl;
				if (l)
				{	l->setDensityMap(scn->grassDensRTex, Forests::MapChannel(std::min(3,i)));  // l->chan);
					//l->applyShader();
					scn->grass->reloadGeometry();
				}
			}
		}
	}
	defOld = def;
	gdOld = gd;
}
#endif

	///  paged  * * *  ? frameStarted
	if (road)
	{	if (scn->grass)  scn->grass->update();
		if (scn->trees)  scn->trees->update();
	}


	///  paged  Upd  * * *
	if (bTrGrUpd)
	{	bTrGrUpd = false;
		pSet->bTrees = !pSet->bTrees;
		scn->RecreateTrees();
	}
	
	
	//  roads upd
	if (!scn->roads.empty())
	{
		SplineRoad* road1 = scn->roads[0];

		for (SplineRoad* road : scn->roads)
		{
			road->bCastShadow = pSet->shadow_type >= Sh_Depth;
			bool fu = road->RebuildRoadInt();
			
			bool full = road == road1 && fu;
			if (full && scn->pace)  // pace, only for 1st
			{
				scn->pace->SetupTer(scn->terrain);
				road->RebuildRoadPace();
				scn->pace->Rebuild(road, scn->sc, pSet->trk_reverse);
			}
		}
	}

	///**  Render Targets update
	if (edMode == ED_PrvCam)
	{
		scn->sc->camPos = mCamera->getPosition();
		scn->sc->camDir = mCamera->getDirection();
		if (rt[RT_View].tex)
			rt[RT_View].tex->update();
	}else{
		static int ri = 0;
		if (ri >= pSet->mini_skip)
		{	ri = 0;
			for (int i=0; i < RT_View; ++i)
				if (rt[i].tex)
					rt[i].tex->update();
		}	ri++;
	}
	//LogO(toStr(evt.timeSinceLastFrame));

	return true;
}


//---------------------------------------------------------------------------------------------------------------
bool App::frameStarted(const Ogre::FrameEvent& evt)
{
	BaseApp::frameStarted(evt);

	static Real time1 = 0.;
	mDTime = evt.timeSinceLastFrame;
	
	//  inc edit time
	time1 += mDTime;
	if (time1 > 1.)
	{	time1 -= 1.;  ++scn->sc->secEdited;

		if (bGuiFocus)	//  upd ed info txt
			gui->UpdEdInfo();
	}
	
	if (mDTime > 0.1f)  mDTime = 0.1f;  //min 5fps


	//  update input
	mRotX = 0; mRotY = 0;  mRotKX = 0; mRotKY = 0;  mTrans = Vector3::ZERO;
	#define  isKey(a)  mInputWrapper->isKeyDown(SDL_SCANCODE_##a)

	//  Move,Rot camera
	if (bCam())
	{
		if (isKey(A))  mTrans.x -= 1;	if (isKey(D))  mTrans.x += 1;
		if (isKey(W))  mTrans.z -= 1;	if (isKey(S))  mTrans.z += 1;
		if (isKey(Q))  mTrans.y -= 1;	if (isKey(E))  mTrans.y += 1;
		
		if (isKey(DOWN) ||isKey(KP_2))  mRotKY -= 1;
		if (isKey(UP)   ||isKey(KP_8))  mRotKY += 1;
		if (isKey(RIGHT)||isKey(KP_6))  mRotKX -= 1;
		if (isKey(LEFT) ||isKey(KP_4))  mRotKX += 1;
	}

	   // key modifiers
	  alt = mInputWrapper->isModifierHeld(SDL_Keymod(KMOD_ALT));
	 ctrl = mInputWrapper->isModifierHeld(SDL_Keymod(KMOD_CTRL));
	shift = mInputWrapper->isModifierHeld(SDL_Keymod(KMOD_SHIFT));

	 // speed multiplers
	moveMul = 1;  rotMul = 1;
	if(shift){	moveMul *= 0.2;	 rotMul *= 0.4;	}  // 16 8, 4 3, 0.5 0.5
	if(ctrl){	moveMul *= 4;	 rotMul *= 2.0;	}
	//if(alt)  {	moveMul *= 0.5;	 rotMul *= 0.5;	}
	//const Real s = (shift ? 0.05 : ctrl ? 4.0 :1.0)

	if (imgCur)  //-
	{
		const MyGUI::IntPoint& mp = MyGUI::InputManager::getInstance().getMousePosition();
		imgCur->setPosition(mp);
		imgCur->setVisible(bGuiFocus || !bMoveCam);
	}

	processMouse(mDTime);

	
	///  gui
	gui->GuiUpdate();
	
	
	if (bRecreateFluids)
	{	bRecreateFluids = false;

		scn->DestroyFluids();
		scn->CreateFluids();
		UpdFluidBox();
	}

	if (bRecreateEmitters)
	{	bRecreateEmitters = false;

		scn->DestroyEmitters(false);
		if (bParticles)
			scn->CreateEmitters();
		UpdEmtBox();
	}

	
	//--  3d view upd  (is global in window)
	static bool oldVis = false;
	int tab = mWndTabsEdit->getIndexSelected(), st5 = gui->vSubTabsEdit[TAB_Veget]->getIndexSelected();
	bool vis = mWndEdit && mWndEdit->getVisible() && (tab == TAB_Objects || tab == TAB_Veget && st5 == 1);

	if (oldVis != vis)
	{	oldVis = vis;
		gui->viewCanvas->setVisible(vis);
	}
	if (gui->tiViewUpd >= 0.f)
		gui->tiViewUpd += evt.timeSinceLastFrame;

	if (gui->tiViewUpd > 0.0f)  //par delay 0.1
	{	gui->tiViewUpd = -1.f;

		gui->viewBox->clearScene();
		if (!gui->viewMesh.empty() && ResourceGroupManager::getSingleton().resourceExistsInAnyGroup(gui->viewMesh))
		{	gui->viewSc = gui->viewBox->injectObject(gui->viewMesh);
			gui->updVegetInfo();
	}	}
	
	
	//  Update rain/snow - depends on camera
	scn->UpdateWeather(mCamera, pSet->bWeather ? 0.f : 1.f);

	// update shader time
	mTimer += evt.timeSinceLastFrame;
	mFactory->setSharedParameter("windTimer", sh::makeProperty <sh::FloatValue>(new sh::FloatValue(mTimer)));
	mFactory->setSharedParameter("waterTimer", sh::makeProperty <sh::FloatValue>(new sh::FloatValue(mTimer)));
	
	/*if (ndCar && road)  ///()  grass sphere test
	{
		const Vector3& p = ndCar->getPosition();  Real r = road->vStBoxDim.z/2;  r *= r;
		mFactory->setSharedParameter("posSph0", sh::makeProperty <sh::Vector4>(new sh::Vector4(p.x,p.y,p.z,r)));
		mFactory->setSharedParameter("posSph1", sh::makeProperty <sh::Vector4>(new sh::Vector4(p.x,p.y,p.z,r)));
	}/**/
	
	
	//  pace vis
	if (scn->pace)
		scn->pace->UpdVis(Vector3::ZERO, edMode == ED_PrvCam);

	
	//  upd terrain generator preview
	if (bUpdTerPrv)
	{	bUpdTerPrv = false;
		updateTerPrv();
	}

	
	///  simulate objects
	if (edMode == ED_Objects && objSim /*&& bEdit()*/)
		BltUpdate(evt.timeSinceLastFrame);
	
	UpdObjNewNode();


	bFirstRenderFrame = false;
	
	return true;
}
