#include "stdafx.h"
#include "BaseApp.h"
#include "FollowCamera.h"
#include "../vdrift/pathmanager.h"
#include "../oisb/OISB.h"
#include "MyGUI_Prerequest.h"
#include "MyGUI_PointerManager.h"


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
	if (bAssignKey)
	{
		bAssignKey = false;
		pressedKey = arg.key;
		pressedKeySender->setCaption(mKeyboard->getAsString(pressedKey));
		Log(mKeyboard->getAsString(pressedKey));
		// show mouse again
		MyGUI::PointerManager::getInstance().setVisible(true);
		
		// get action/schema/index from widget name
		std::string actionName = Ogre::StringUtil::split(pressedKeySender->getName(), "_")[1];
		std::string schemaName = Ogre::StringUtil::split(pressedKeySender->getName(), "_")[2];
		std::string index = Ogre::StringUtil::split(pressedKeySender->getName(), "_")[3];
		
		OISB::ActionSchema* schema = OISB::System::getSingleton().mActionSchemas[schemaName];
		OISB::Action* action = schema->mActions[actionName];
		OISB::Binding* binding = action->mBindings.front();
		
		// get old binds
		OISB::Bindable* bind1 = NULL;
		OISB::Bindable* bind2 = NULL;
		std::string bind1_role = "";
		std::string bind2_role = "";
		if (binding->getNumBindables() == 1)
		{
			bind1 = binding->getBindable(0);
			bind1_role = binding->getRole(bind1);
		}
		else if (binding->getNumBindables() == 2)
		{
			bind1 = binding->getBindable(0);
			bind1_role = binding->getRole(bind1);
			bind2 = binding->getBindable(1);
			bind2_role = binding->getRole(bind2);
		}
		// delete all binds
		if (bind1) binding->unbind(bind1);
		if (bind2) binding->unbind(bind2);
		
		// for analog axis actions, make sure the binds have a role
		if (action->getActionType() == OISB::AT_ANALOG_AXIS)
		{
			if (bind1_role == "" || bind2_role == "")
			{
				bind1_role = "increase";
				bind2_role = "decrease";
			}
		}
		
		try
		{
			// rebind
			if (index == "1")
			{
				binding->bind("Keyboard/" + mKeyboard->getAsString(pressedKey), bind1_role);
				// only bind 2nd if keys are not the same (will throw exception)
				if (bind2)
					if ("Keyboard/" + mKeyboard->getAsString(pressedKey) != bind2->getBindableName())
					{
						binding->bind(bind2, bind2_role);	
					}
			}
			else if (index == "2")
			{
				// only bind 1st if keys are not the same (will throw exception)
				if (bind1)
					if ("Keyboard/" + mKeyboard->getAsString(pressedKey) != bind1->getBindableName())
					{
						binding->bind(bind1, bind1_role);
					}
				binding->bind("Keyboard/" + mKeyboard->getAsString(pressedKey), bind2_role);
			}
		}
		catch (OIS::Exception) 
		{
			// invalid key?
			// restore old
			Log("WARNING: binding->bind failed, restoring old binds...");
			
			// this is nasty, but since some very weird stuff can happen here, we have to individually try/catch
			try {
				binding->unbind("Keyboard/" + mKeyboard->getAsString(pressedKey));
			}
			catch (OIS::Exception) {}
			try {
				if (bind1) binding->unbind(bind1);
			}
			catch (OIS::Exception) {}
			try {
				if (bind2) binding->unbind(bind2);
			}
			catch (OIS::Exception) {}
			
			if (bind1) binding->bind(bind1, bind1_role);
			if (bind2) binding->bind(bind2, bind2_role);
			
			return true;
		}
		
		// macro to strip away the Keyboard/
		#define stripk(s) Ogre::StringUtil::split(s, "/")[1]
		
		// update button labels
		MyGUI::ButtonPtr b1 = mGUI->findWidget<MyGUI::Button>("inputbutton_" + actionName + "_" + schemaName + "_" + "1");
		MyGUI::ButtonPtr b2 = mGUI->findWidget<MyGUI::Button>("inputbutton_" + actionName + "_" + schemaName + "_" + "2");
		if (binding->getNumBindables() == 0)
		{
			b1->setCaption( TR("#{InputKeyUnassigned}") ); 
			b2->setCaption( TR("#{InputKeyUnassigned}") );
		}
		else if (binding->getNumBindables() == 1)
		{
			// increase first
			if (binding->getRole(binding->getBindable(0)) == "decrease")
			{
				b2->setCaption( stripk(binding->getBindable(0)->getBindableName()) );
				b1->setCaption( TR("#{InputKeyUnassigned}") );
			}
			else
			{
				b1->setCaption( stripk(binding->getBindable(0)->getBindableName()) );
				b2->setCaption( TR("#{InputKeyUnassigned}") );
			}
		}
		else if (binding->getNumBindables() == 2)
		{
			// increase first
			if (binding->getRole(binding->getBindable(0)) == "increase")
			{
				b1->setCaption( stripk(binding->getBindable(0)->getBindableName()) );
				b2->setCaption( stripk(binding->getBindable(1)->getBindableName()) );
			}
			else
			{
				b2->setCaption( stripk(binding->getBindable(0)->getBindableName()) );
				b1->setCaption( stripk(binding->getBindable(1)->getBindableName()) );

			}
		}
		return true;
	}
	
	using namespace OIS;
	if (!alt)
	switch (arg.key)  // global
	{
		///  Wireframe F10
		case KC_F10:
		if (!shift && !ctrl)
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

	switch (arg.key)
	{
		///  Camera
		case KC_PGDOWN: case KC_NUMPAD3:
		case KC_C:		// Next
		{	int visMask = 255;
			roadUpCnt = 0;
			//  for all cars
			for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
			{
				if ((*it)->fCam)
				{
					(*it)->fCam->Next(0, shift);
					if ((*it)->fCam->ca.mHideGlass)  visMask = 255-16;
					else        visMask = 255;
				}
			}
			//  set visibility mask for all viewports
			for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); it++)
				(*it)->setVisibilityMask(visMask);
		}	return false;

		case KC_PGUP: case KC_NUMPAD9:
		case KC_X:		// Prev
		{	int visMask = 255;
			roadUpCnt = 0;
			//  for all cars
			for (std::list<CarModel*>::iterator it=carModels.begin(); it!=carModels.end(); it++)
			{
				if ((*it)->fCam)
				{
					(*it)->fCam->Next(1, shift);
					if ((*it)->fCam->ca.mHideGlass)  visMask = 255-16;
					else        visMask = 255;
				}
			}
			//  set visibility mask for all viewports
			for (std::list<Viewport*>::iterator it=mSplitMgr->mViewports.begin(); it!=mSplitMgr->mViewports.end(); it++)
				(*it)->setVisibilityMask(visMask);
		}	return false;
			
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
