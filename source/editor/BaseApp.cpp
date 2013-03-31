#include "pch.h"
#include "../ogre/common/Defines.h"
#include "BaseApp.h"
#include "../road/Road.h"
using namespace Ogre;


///  Fps stats
//------------------------------------------------------------------------
void BaseApp::updateStats()
{
	//  Focus  * * *
	static int fcOld = -2;
	int fc = bGuiFocus ? 2 : bMoveCam ? 0 : 1;
	if (ovFocus && fc != fcOld)
	{	if (fcOld < 0)  ++fcOld;
		else  fcOld = fc;
		
		const char* sFoc[3] = {"Cam", " Edit", "  Gui"};
		ColourValue cFoc[3] = {ColourValue(0.7,0.85,1.0), ColourValue(0.7,1.0,0.5), ColourValue(1.0,1.0,0.4)};
		const char* mFoc[3] = {"Cam", "Edit", "Gui"};

		ovFocus->setCaption(sFoc[fc]);
		ovFocus->setColour(cFoc[fc]);  if (ovFocBck)
		ovFocBck->setMaterialName(String("Menu/Focus") + mFoc[fc]);
	}

	//  camera pos, rot
	if (pSet->camPos)
	{
		const Vector3& pos = /*road ? road->posHit :*/ mCamera->getDerivedPosition();
		//const Quaternion& rot = mCamera->getDerivedOrientation();
		String s = "Pos: "+fToStr(pos.x,1)+" " + fToStr(pos.y,1) + " " + fToStr(pos.z,1);
					//+", // | Rot: " +fToStr(rot.x,3) + " "+fToStr(rot.y,3)+" "+fToStr(rot.z,3)+" "+fToStr(rot.w,3);
		ovPos->setCaption(s);
	}

	//  Fps, Tri, Bat
	{	const RenderTarget::FrameStats& stats = mWindow->getStatistics();
		size_t mem = TextureManager::getSingleton().getMemoryUsage() + MeshManager::getSingleton().getMemoryUsage();

		ovFps->setCaption(fToStr(stats.lastFPS,1,5) );
		ovTri->setCaption(iToStr(int(stats.triangleCount/1000.f),4)+"k");
		ovBat->setCaption(iToStr(stats.batchCount,3) );
		ovMem->setCaption(iToStr(mem/1024/1024,3)+"M" );
	}
}


//  rendering
//-------------------------------------------------------------------------------------
bool BaseApp::frameStarted(const FrameEvent& evt)
{
	//OnTimer(evt.timeSinceLastFrame);
	
	updateStats();

	if (ndSky)  ///o-
		ndSky->setPosition(mCamera->getPosition());

	return true;
}

///  timer thread  - update input and camera
void BaseApp::OnTimer(double dtime)
{
	mDTime = dtime;
	if (mDTime > 0.1f)  mDTime = 0.1f;  //min 5fps

	//  update input
	if (!mKeyboard || !mMouse)  return;
	mKeyboard->capture();  mMouse->capture();

	mRotX = 0; mRotY = 0;  mRotKX = 0; mRotKY = 0;  mTrans = Vector3::ZERO;
	#define  key(a)  (mKeyboard->isKeyDown(OIS::KC_##a))
	
	//  Move,Rot camera
	if (bCam())
	{
		if(key(A))	mTrans.x -= 1;	if(key(D))	mTrans.x += 1;
		if(key(W))	mTrans.z -= 1;	if(key(S))	mTrans.z += 1;
		if(key(Q))	mTrans.y -= 1;	if(key(E))	mTrans.y += 1;
			
		if(key(DOWN)||key(NUMPAD2))   mRotKY -= 1;
		if(key(UP)  ||key(NUMPAD8))   mRotKY += 1;
		if(key(RIGHT)||key(NUMPAD6))  mRotKX -= 1;
		if(key(LEFT) ||key(NUMPAD4))  mRotKX += 1;
	}
 	//using namespace OIS;

	   // key modifiers
	  alt = mKeyboard->isModifierDown(OIS::Keyboard::Alt),
	 ctrl = mKeyboard->isModifierDown(OIS::Keyboard::Ctrl),
	shift = mKeyboard->isModifierDown(OIS::Keyboard::Shift);
	
	 // speed multiplers
	moveMul = 1;  rotMul = 1;
	if(shift){	moveMul *= 0.2;	 rotMul *= 0.4;	}  // 16 8, 4 3, 0.5 0.5
	if(ctrl){	moveMul *= 4;	 rotMul *= 2.0;	}
	//if(alt)  {	moveMul *= 0.5;	 rotMul *= 0.5;	}
	//const Real s = (shift ? 0.05 : ctrl ? 4.0 :1.0) 

	processMouse();
}

bool BaseApp::frameRenderingQueued(const FrameEvent& evt)
{
	//mDTime = evt.timeSinceLastFrame;
	//if (mDTime > 0.1f)  mDTime = 0.1f;  //min 5fps

	mCamera->setPosition(mCameraT->getPosition());  // copy from thread
	mCamera->setDirection(mCameraT->getDirection());

	if (mWindow->isClosed())
		return false;

	if (mShutDown)
		return false;

	return true;
}

bool BaseApp::frameEnded(const FrameEvent& evt)
{
	//(void)evt;
	//updateStats();
	return true;
}
