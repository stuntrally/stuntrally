#include "pch.h"
#include "Defines.h"
#include "BaseApp.h"
#include "FollowCamera.h"
#include "../vdrift/pathmanager.h"
#include "SplitScreenManager.h"

#include "MyGUI_Prerequest.h"
#include "MyGUI_PointerManager.h"

#include <OIS/OIS.h>
#include "../oisb/OISB.h"

#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>
using namespace Ogre;


//  rendering
//-------------------------------------------------------------------------------------
bool BaseApp::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	if (mWindow->isClosed())
		return false;

	if (mShutDown)
		return false;

	//  update each device
	mKeyboard->capture();
	mMouse->capture();

	   // key modifiers
	  alt = mKeyboard->isModifierDown(OIS::Keyboard::Alt),
	 ctrl = mKeyboard->isModifierDown(OIS::Keyboard::Ctrl),
	shift = mKeyboard->isModifierDown(OIS::Keyboard::Shift);

	updateStats();
	
	// dt-
	Real time = evt.timeSinceLastFrame;
	if (time > 0.2f)  time = 0.2f;
	
	frameStart(time);
	//* */if (!frameStart())
	//	return false;
	
	return true;
}

bool BaseApp::frameEnded(const Ogre::FrameEvent& evt)
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
		
		// cancel on backspace or escape
		bool cancel = false;
		if (pressedKey == OIS::KC_BACK || pressedKey == OIS::KC_ESCAPE)
			cancel = true;
		
		pressedKeySender->setCaption(mKeyboard->getAsString(pressedKey));
		// show mouse again
		MyGUI::PointerManager::getInstance().setVisible(true);
		
		// get action/schema/index from widget name
		std::string actionName = Ogre::StringUtil::split(pressedKeySender->getName(), "_")[1];
		std::string schemaName = Ogre::StringUtil::split(pressedKeySender->getName(), "_")[2];
		std::string index = Ogre::StringUtil::split(pressedKeySender->getName(), "_")[3];
		
		OISB::ActionSchema* schema = OISB::System::getSingleton().mActionSchemas[schemaName];
		OISB::Action* action = schema->mActions[actionName];
		if (action->mBindings.size() == 0)
			action->createBinding();
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
		
		// cancel: delete canceled bind
		if (cancel)
		{
			if (index == "1" && bind1) binding->unbind(bind1);
			if (index == "2" && bind2) binding->unbind(bind2);
			if (index == "1") bind1 = NULL;
			if (index == "2") bind2 = NULL;
		}
			
		// delete all binds
		if (!cancel) {
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
							binding->bind(bind2, bind2_role);	
				}
				else if (index == "2")
				{
					// only bind 1st if keys are not the same (will throw exception)
					if (bind1)
						if ("Keyboard/" + mKeyboard->getAsString(pressedKey) != bind1->getBindableName())
							binding->bind(bind1, bind1_role);

					binding->bind("Keyboard/" + mKeyboard->getAsString(pressedKey), bind2_role);
				}
			}
			catch (OIS::Exception) 
			{
				// invalid key?
				// restore old
				LogO("WARNING: binding->bind failed, restoring old binds...");
				
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
		}
		
		// macro to strip away the Keyboard/
		#define stripk(s) Ogre::StringUtil::split(s, "/")[1]
		
		// update button labels
		MyGUI::ButtonPtr b1 = mGUI->findWidget<MyGUI::Button>("inputbutton_" + actionName + "_" + schemaName + "_" + "1", "", false);
		MyGUI::ButtonPtr b2 = mGUI->findWidget<MyGUI::Button>("inputbutton_" + actionName + "_" + schemaName + "_" + "2", "", false);
		if (binding->getNumBindables() == 0)
		{
			if (b1) b1->setCaption( TR("#{InputKeyUnassigned}") ); 
			if (b2) b2->setCaption( TR("#{InputKeyUnassigned}") );
		}
		else if (binding->getNumBindables() == 1)
		{
			// increase first
			if (binding->getRole(binding->getBindable(0)) == "decrease")
			{
				if (b1) b2->setCaption( stripk(binding->getBindable(0)->getBindableName()) );
				if (b2) b1->setCaption( TR("#{InputKeyUnassigned}") );
			}else{
				if (b1) b1->setCaption( stripk(binding->getBindable(0)->getBindableName()) );
				if (b2) b2->setCaption( TR("#{InputKeyUnassigned}") );
			}
		}
		else if (binding->getNumBindables() == 2)
		{
			// increase first
			if (binding->getRole(binding->getBindable(0)) == "increase")
			{
				if (b1) b1->setCaption( stripk(binding->getBindable(0)->getBindableName()) );
				if (b2) b2->setCaption( stripk(binding->getBindable(1)->getBindableName()) );
			}else{
				if (b2) b2->setCaption( stripk(binding->getBindable(0)->getBindableName()) );
				if (b1) b1->setCaption( stripk(binding->getBindable(1)->getBindableName()) );
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
		//case KC_S:		// Save S
		//	if (pCar)	if (pCar->mFCam)
		//	pCar->mFCam->saveCamera();
		//	return false;

	
		//  Camera pos info
		/*case KC_O:
		{	mbShowCamPos = !mbShowCamPos;
			if (!mbShowCamPos)	mDebugText = "";
		}	return false;/**/

		
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
		/*if (mbShowCamPos)
		{
			const Vector3& pos = (*mSplitMgr->mCameras.begin())->getDerivedPosition();
			const Quaternion& rot = (*mSplitMgr->mCameras.begin())->getDerivedOrientation();
			sprintf(s, "Pos: %5.1f %5.1f %5.1f", //  Rot: %6.3f %6.3f %6.3f %6.3f",
							pos.x, pos.y, pos.z,  rot.x, rot.y, rot.z, rot.w );
			mDebugText = String( s );
		}/**/
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
