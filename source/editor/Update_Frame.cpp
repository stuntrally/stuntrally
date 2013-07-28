#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include "../paged-geom/PagedGeometry.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/MultiList2.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
//#include "LinearMath/btDefaultMotionState.h"
//#include "BulletDynamics/Dynamics/btRigidBody.h"
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include "../shiny/Main/Factory.hpp"
using namespace Ogre;

#include "../sdl4ogre/sdlinputwrapper.hpp"


///  Mouse
//---------------------------------------------------------------------------------------------------------------
void App::processMouse(double fDT)
{
	//  static vars are smoothed
	Vector3 vInpC(0,0,0),vInp;
	Real fSmooth = (powf(1.0f - pSet->cam_inert, 2.2f) * 40.f + 0.1f) * fDT;

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
	Degree fRot = 500*cr, fkRot = 160*cr;

	static Radian sYaw(0), sPth(0);
	static Vector3 sMove(0,0,0);

	Radian inYaw = rotMul * fDT * (fRot* mRotX + fkRot* mRotKX);
	Radian inPth = rotMul * fDT * (fRot* mRotY + fkRot* mRotKY);
	Vector3 inMove = moveMul * fDT * (fMove * mTrans);

	sYaw += (inYaw - sYaw) * fSmooth;
	sPth += (inPth - sPth) * fSmooth;
	sMove += (inMove - sMove) * fSmooth;

	mCamera->yaw( sYaw );
	mCamera->pitch( sPth );
	mCamera->moveRelative( sMove );
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
		vSubTabsEdit.size() > 3 && vSubTabsEdit[3]->getIndexSelected() == 1)
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
	if (bTerUpdBlend)
	{	bTerUpdBlend = false;  bu = 0;
		if (terrain)
		{
			GetTerAngles();  // full
			initBlendMaps(terrain);
		}
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
	#define  key(a)  (mInputWrapper->isKeyDown(SDL_GetScancodeFromKey(a)))

	//  Move,Rot camera
	if (bCam())
	{
		if(key(SDLK_a))	mTrans.x -= 1;	if(key(SDLK_d))	mTrans.x += 1;
		if(key(SDLK_w))	mTrans.z -= 1;	if(key(SDLK_s))	mTrans.z += 1;
		if(key(SDLK_q))	mTrans.y -= 1;	if(key(SDLK_e))	mTrans.y += 1;

		if(key(SDLK_DOWN)||key(SDLK_KP_2))   mRotKY -= 1;
		if(key(SDLK_UP)  ||key(SDLK_KP_8))   mRotKY += 1;
		if(key(SDLK_RIGHT)||key(SDLK_KP_6))  mRotKX -= 1;
		if(key(SDLK_LEFT) ||key(SDLK_KP_4))  mRotKX += 1;
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

	processMouse(mDTime);

	
	UnfocusLists();
	
	if (iLoadNext)  // load next/prev track
	{	size_t cnt = trkList->getItemCount();
		if (cnt > 0)  
		{	int i = std::max(0, std::min((int)cnt-1, (int)trkList->getIndexSelected() + iLoadNext ));
			iLoadNext = 0;
			trkList->setIndexSelected(i);
			trkList->beginToItemAt(std::max(0, i-11));  // center
			listTrackChng(trkList,i);
			btnNewGame(0);
	}	}
	
	if (bGuiReinit)  // after language change from combo
	{	bGuiReinit = false;
		mGUI->destroyWidgets(vwGui);  bnQuit=0;mWndOpts=0;trkList=0; //todo: rest too..
		InitGui();
		SetGuiFromXmls();
		bWindowResized = true;
		//mWndTabs->setIndexSelected(10);  // switch back to view tab
	}

	if (bWindowResized)
	{	bWindowResized = false;

		ResizeOptWnd();
		//bSizeHUD = true;
		SizeGUI();
		updTrkListDim();
		viewCanvas->setCoord(GetViewSize());
		//LoadTrack();  // shouldnt be needed ...
	}
	
	if (bRecreateFluids)
	{	bRecreateFluids = false;
		DestroyFluids();
		CreateFluids();
		UpdFluidBox();
	}
	
	///  sort trk list
	if (trkList && (trkList->mSortColumnIndex != trkList->mSortColumnIndexOld
		|| trkList->mSortUp != trkList->mSortUpOld))
	{
		trkList->mSortColumnIndexOld = trkList->mSortColumnIndex;
		trkList->mSortUpOld = trkList->mSortUp;

		pSet->tracks_sort = trkList->mSortColumnIndex;  // to set
		pSet->tracks_sortup = trkList->mSortUp;
		TrackListUpd(false);
	}

	
	//--  3d view upd  (is global in window)
	static bool oldVis = false;
	int tab = mWndTabsEdit->getIndexSelected(), st5 = vSubTabsEdit[5]->getIndexSelected();
	bool vis = mWndEdit && mWndEdit->getVisible() && (tab == 7 || tab == 5 && st5 == 2);
	if (oldVis != vis)
	{	oldVis = vis;
		viewCanvas->setVisible(vis);
	}
	if (tiViewUpd >= 0.f)
		tiViewUpd += evt.timeSinceLastFrame;
	if (tiViewUpd > 0.0f)  //par delay 0.1
	{	tiViewUpd = -1.f;
		viewBox.clearScene();
		if (viewMesh != "" && ResourceGroupManager::getSingleton().resourceExistsInAnyGroup(viewMesh))
			viewBox.injectObject(viewMesh);
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
	if (edMode == ED_Objects && objSim /*&& bEdit()*/)
		BltUpdate(evt.timeSinceLastFrame);
	
	UpdObjNewNode();

	bFirstRenderFrame = false;
	
	return true;
}
