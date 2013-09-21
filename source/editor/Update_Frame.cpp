#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/GuiCom.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../paged-geom/PagedGeometry.h"
#include "../ogre/common/MultiList2.h"
#include "../ogre/common/RenderBoxScene.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
//#include "LinearMath/btDefaultMotionState.h"
//#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "../shiny/Main/Factory.hpp"
#include "../sdl4ogre/sdlinputwrapper.hpp"
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
	if (bGuiFocus &&
		mWndEdit && mWndEdit->getVisible() && mWndTabsEdit->getIndexSelected()==3 &&
		gui->vSubTabsEdit.size() > 3 && gui->vSubTabsEdit[3]->getIndexSelected() == 1)
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
	mInputWrapper->capture();

	//  road pick
	if (road)
	{
		const MyGUI::IntPoint& mp = MyGUI::InputManager::getInstance().getMousePosition();
		Real mx = Real(mp.left)/mWindow->getWidth(), my = Real(mp.top)/mWindow->getHeight();
		road->Pick(mCamera, mx, my,
			edMode == ED_Road,  !(edMode == ED_Road && bEdit()));

		if (sc->vdr)  // blt ray hit
		{
			Ray ray = mCamera->getCameraToViewportRay(mx,my);
			const Vector3& pos = mCamera->getDerivedPosition(), dir = ray.getDirection();
			btVector3 from(pos.x,-pos.z,pos.y), to(dir.x,-dir.z,dir.y);  to = from + to*10000.f;
			btCollisionWorld::ClosestRayResultCallback rayRes(from, to);

			world->rayTest(from, to, rayRes);

			if (rayRes.hasHit())
				road->posHit = Vector3(rayRes.m_hitPointWorld.getX(),rayRes.m_hitPointWorld.getZ(),-rayRes.m_hitPointWorld.getY());
			else
				road->posHit = Vector3::ZERO;
			road->ndHit->setPosition(road->posHit);
		}
	}

	editMouse();  // edit


	///  paged  Upd  * * *
	if (bTrGrUpd)
	{	bTrGrUpd = false;
		pSet->bTrees = !pSet->bTrees;
		UpdTrees();
	}
	///  paged  * * *  ? frameStarted
	if (road)
	{	if (grass)  grass->update();
		if (trees)  trees->update();
	}
	
	///<>  Edit Ter
	TerCircleUpd();
	if (terrain && road && bEdit() && road->bHitTer)
	{
		float dt = evt.timeSinceLastFrame;
		Real s = shift ? 0.25 : ctrl ? 4.0 :1.0;
		switch (edMode)
		{
		case ED_Deform:
			if (mbLeft)   deform(road->posHit, dt, s);  else
			if (mbRight)  deform(road->posHit, dt,-s);
			break;
		case ED_Filter:
			if (mbLeft)   filter(road->posHit, dt, s);
			break;
		case ED_Smooth:
			if (mbLeft)   smooth(road->posHit, dt);
			break;
		case ED_Height:
			if (mbLeft)   height(road->posHit, dt, s);
			break;
		}
	}

	///<>  Ter upd
	static int tu = 0, bu = 0;
	if (tu >= pSet->ter_skip)
	if (bTerUpd)
	{	bTerUpd = false;  tu = 0;
		if (mTerrainGroup)
			mTerrainGroup->update();
	}	tu++;

	if (bu >= pSet->ter_skip)
	{	if (gui->noBlendUpd)
			bTerUpdBlend = false;
		else
		if (bTerUpdBlend)
		{	bTerUpdBlend = false;  bu = 0;
			if (terrain)
			{
				GetTerAngles();  // full
				initBlendMaps(terrain);
		}	}
	}	bu++;

	
	if (road)  // road
	{
		road->bCastShadow = pSet->shadow_type >= 2;
		road->RebuildRoadInt();
	}

	///**  Render Targets update
	if (edMode == ED_PrvCam)
	{
		sc->camPos = mCamera->getPosition();
		sc->camDir = mCamera->getDirection();
		if (rt[RTs-1].rndTex)
			rt[RTs-1].rndTex->update();
	}else{
		static int ri = 0;
		if (ri >= pSet->mini_skip)
		{	ri = 0;
			for (int i=0; i < RTs-1/**/; ++i)
				if (rt[i].rndTex)
					rt[i].rndTex->update();
		}	ri++;
	}
	//LogO(toStr(evt.timeSinceLastFrame));

	return true;
}


//---------------------------------------------------------------------------------------------------------------
bool App::frameStarted(const Ogre::FrameEvent& evt)
{
	BaseApp::frameStarted(evt);

	mDTime = evt.timeSinceLastFrame;
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

	
	gcom->UnfocusLists();
	
	//if (gui->iLoadNext)  // load next/prev track
	//{	size_t cnt = gui->trkList->getItemCount();
	//	if (cnt > 0)  
	//	{	int i = std::max(0, std::min((int)cnt-1, (int)gui->trkList->getIndexSelected() + gui->iLoadNext ));
	//		gui->iLoadNext = 0;
	//		gui->trkList->setIndexSelected(i);
	//		gui->trkList->beginToItemAt(std::max(0, i-11));  // center
	//		gui->listTrackChng(gui->trkList,i);
	//		gui->btnNewGame(0);
	//}	}
	
	if (gcom->bGuiReinit)  // after language change from combo
	{	gcom->bGuiReinit = false;

		gui->mGui->destroyWidgets(vwGui);
		gcom->bnQuit=0; mWndOpts=0; gcom->trkList=0; //todo: rest too..

		gui->InitGui();
		gui->SetGuiFromXmls();
		bWindowResized = true;
	}

	if (bWindowResized)
	{	bWindowResized = false;

		gcom->ResizeOptWnd();
		//bSizeHUD = true;
		gcom->SizeGUI();
		gcom->updTrkListDim();
		gui->viewCanvas->setCoord(gui->GetViewSize());
		//LoadTrack();  // shouldnt be needed ...
	}
	
	if (bRecreateFluids)
	{	bRecreateFluids = false;
		DestroyFluids();
		CreateFluids();
		UpdFluidBox();
	}
	
	//  sort trk list
	gcom->SortTrkList();

	
	//--  3d view upd  (is global in window)
	static bool oldVis = false;
	int tab = mWndTabsEdit->getIndexSelected(), st5 = gui->vSubTabsEdit[5]->getIndexSelected();
	bool vis = mWndEdit && mWndEdit->getVisible() && (tab == 7 || tab == 5 && st5 == 2);
	if (oldVis != vis)
	{	oldVis = vis;
		gui->viewCanvas->setVisible(vis);
	}
	if (gui->tiViewUpd >= 0.f)
		gui->tiViewUpd += evt.timeSinceLastFrame;
	if (gui->tiViewUpd > 0.0f)  //par delay 0.1
	{	gui->tiViewUpd = -1.f;
		gui->viewBox->clearScene();
		if (gui->viewMesh != "" && ResourceGroupManager::getSingleton().resourceExistsInAnyGroup(gui->viewMesh))
			gui->viewBox->injectObject(gui->viewMesh);
	}
	
	
	//  Update rain/snow - depends on camera
	const Vector3& pos = mCamera->getPosition(), dir = mCamera->getDirection();
	static Vector3 oldPos = Vector3::ZERO;
	Vector3 vel = (pos-oldPos)/ (1.0f / mWindow->getLastFPS());  oldPos = pos;
	Vector3 par = pos + dir * 12 + vel * 0.4;
	float f = pSet->bWeather ? 0.f : 1.f;
	if (pr)
	{
		ParticleEmitter* pe = pr->getEmitter(0);
		pe->setPosition(par);
		pe->setEmissionRate(f * sc->rainEmit);
	}
	if (pr2)
	{
		ParticleEmitter* pe = pr2->getEmitter(0);
		pe->setPosition(par);
		pe->setEmissionRate(f * sc->rain2Emit);
	}

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
	
	
	//  upd ter gen prv tex
	if (bUpdTerPrv)
	{	bUpdTerPrv = false;
		updateTerPrv();
	}

	
	///  simulate
	if (edMode == ED_Objects && gui->objSim /*&& bEdit()*/)
		BltUpdate(evt.timeSinceLastFrame);
	
	gui->UpdObjNewNode();

	bFirstRenderFrame = false;
	
	return true;
}
