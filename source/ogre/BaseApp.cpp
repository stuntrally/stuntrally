#include "stdafx.h"
#include "BaseApp.h"
#include "FollowCamera.h"

#include "../vdrift/pathmanager.h"


//  rendering
//-------------------------------------------------------------------------------------
bool BaseApp::frameRenderingQueued(const FrameEvent& evt)
{
	if (mWindow->isClosed())
		return false;

	if (mShutDown)
		return false;

	//  update each device
	mKeyboard->capture();
	mMouse->capture();

	using namespace OIS;
	   // key modifiers
	  alt = mKeyboard->isModifierDown(Keyboard::Alt),
	 ctrl = mKeyboard->isModifierDown(Keyboard::Ctrl),
	shift = mKeyboard->isModifierDown(Keyboard::Shift);

	updateStats();
	
	// dt-
	Real time = evt.timeSinceLastFrame;
	if (time > 0.2f)  time = 0.2f;
	
	frameStart(time);
	//* */if (!frameStart())
	//	return false;
	
	return true;
}

bool BaseApp::frameEnded(const FrameEvent& evt)
{
	// dt-
	Real time = evt.timeSinceLastFrame;
	if (time > 0.2f)  time = 0.2f;
	
	return frameEnd(time);
	//return true;
}


///-------------------------------------------------------------------------------------
//  Key press
///-------------------------------------------------------------------------------------
bool BaseApp::keyPressed( const OIS::KeyEvent &arg )
{
	using namespace OIS;
	if (!alt)
	switch (arg.key)  // global
	{
		///  Wireframe F10
		case KC_F10:
		if (!shift)
		{	mbWireFrame = !mbWireFrame;
			///  Set for all cameras
			PolygonMode mode = mbWireFrame ? PM_WIREFRAME : PM_SOLID;

			if (mSplitMgr)
			for (std::list<Camera*>::iterator it=mSplitMgr->mCameras.begin(); it!=mSplitMgr->mCameras.end(); it++)
				(*it)->setPolygonMode(mode);
			
			if (ndSky)	ndSky->setVisible(!mbWireFrame);  // hide sky
		}	return false;
		
		//  Screen Shot  PtrScr -slow
		case KC_SYSRQ:
			mWindow->writeContentsToTimestampedFile(PATHMANAGER::GetScreenShotDir() + "/", ".jpg");	return false;
	}


	if (isFocGui && mGUI)	// gui
	{
		mGUI->injectKeyPress(MyGUI::KeyCode::Enum(arg.key), arg.text);
		return false;
	}

	int visMask = 255;

	switch (arg.key)
	{
		///  Camera
		case KC_PGDOWN: case KC_NUMPAD3:
		case KC_C:		// Next
			roadUpCnt = 0;
			// for all cars
			for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
			{
				if ((*it)->fCam)
				{
					(*it)->fCam->Next(0, shift);
					if ((*it)->fCam->ca.mHideGlass)  visMask = 255-16;
					else        visMask = 255;
					
				}
			}
			// Set visibility mask for all viewports
			for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); it++)
				(*it)->setVisibilityMask(visMask);
			return false;

		case KC_PGUP: case KC_NUMPAD9:
		case KC_X:		// Prev
			roadUpCnt = 0;
			// for all cars
			for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
			{
				if ((*it)->fCam)
				{
					(*it)->fCam->Next(1, shift);
					if ((*it)->fCam->ca.mHideGlass)  visMask = 255-16;
					else        visMask = 255;
				}
			}
			// Set visibility mask for all viewports
			for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); it++)
				(*it)->setVisibilityMask(visMask);
			return false;
			
		//case KC_S:		// Save S
		//	if (pCar)	if (pCar->mFCam)
		//	pCar->mFCam->saveCamera();
		//	return false;

	
	//  Camera pos info
	case KC_O:
	{	mbShowCamPos = !mbShowCamPos;
		if (!mbShowCamPos)	mDebugText = "";
	}	return false;

	
	//case KC_F5:
	//	TextureManager::getSingleton().reloadAll();	break;
	}

	return true;
}


///  Fps stats
// ------------------------------------------------------------------------
void BaseApp::updateStats()
{
	// Print camera pos, rot
	
	static char s[128];
	// Only for 1 local player
	if (mSplitMgr && mSplitMgr->mNumPlayers == 1)
	{
		if (mbShowCamPos)
		{
			const Vector3& pos = (*mSplitMgr->mCameras.begin())->getDerivedPosition();
			const Quaternion& rot = (*mSplitMgr->mCameras.begin())->getDerivedOrientation();
			sprintf(s, "Pos: %5.1f %5.1f %5.1f", //  Rot: %6.3f %6.3f %6.3f %6.3f",
							pos.x, pos.y, pos.z,  rot.x, rot.y, rot.z, rot.w );
			mDebugText = String( s );
		}
	}
	try {
		const RenderTarget::FrameStats& stats = mWindow->getStatistics();
		//char s[20];
		sprintf(s, "%5.1f", stats.lastFPS );	mOvrFps->setCaption( s );
		sprintf(s, "%5.1fk", Real(stats.triangleCount)/1000.f );	mOvrTris->setCaption( s );
		sprintf(s, "%3lu", stats.batchCount );	mOvrBat->setCaption( s );

		mOvrDbg->setCaption( mFilText + "  " + mDebugText );
	}
	catch(...) {  /*ignore*/  }
}
