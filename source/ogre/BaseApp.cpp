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


//  get key name  /macro to strip away the Keyboard/
String BaseApp::StrFromKey(const String& skey)
{
	Ogre::vector<String>::type ss = StringUtil::split(skey, "/");
	if (ss.size() != 2)
		return String(TR("#{InputKeyUnassigned}"));
	
	String s = ss[1];
	OIS::KeyCode k = (OIS::KeyCode)s2i(s);
	
	return mKeyboard->getAsString(k);
}

///-------------------------------------------------------------------------------------
//  Key press
///-------------------------------------------------------------------------------------
bool BaseApp::keyPressed( const OIS::KeyEvent &arg )
{
	if (bAssignKey)
	{	bAssignKey = false;
		pressedKey = arg.key;
		
		//  cancel on backspace or escape
		bool cancel = pressedKey == OIS::KC_BACK || pressedKey == OIS::KC_ESCAPE;
		
		pressedKeySender->setCaption(mKeyboard->getAsString(pressedKey));
		//  show mouse again
		MyGUI::PointerManager::getInstance().setVisible(true);
		
		//  get action/schema/index from widget name
		Ogre::vector<String>::type ss = StringUtil::split(pressedKeySender->getName(), "_");
		std::string actionName = ss[1], schemaName = ss[2], index = ss[3];
		
		OISB::ActionSchema* schema = OISB::System::getSingleton().mActionSchemas[schemaName];
		OISB::Action* action = schema->mActions[actionName];
		if (action->mBindings.size() == 0)
			action->createBinding();
		OISB::Binding* binding = action->mBindings.front();
		
		//  save keys
		String decKey = "", incKey = "";
		size_t num = binding->getNumBindables();
		for (int i = 0; i < num; ++i)
		{
			OISB::Bindable* bind = binding->getBindable(i);
			String name = bind->getBindableName();
			String role = binding->getRole(bind);
			//if (StringUtil::startsWith(n,"Keyboard"));
			if (role == "dec")  decKey = name;
			if (role == "inc")  incKey = name;
		}
		//  clear, unbind
		for (int i = num-1; i >= 0; --i)
			binding->unbind(binding->getBindable(i));

		//  change
		String skey = cancel ? "" : "Keyboard/" + toStr(pressedKey);
			 if (index == "1")  incKey = skey;  // lower btn - inc
		else if (index == "2")  decKey = skey;  // upper btn - dec

		//  update, bind
		if (incKey != "")	binding->bind(incKey, "inc");
		if (decKey != "")	binding->bind(decKey, "dec");
		
				
		//  update button labels  . . . . . . . 
		MyGUI::ButtonPtr b1 = mGUI->findWidget<MyGUI::Button>("inputbutton_" + actionName + "_" + schemaName + "_" + "1", "", false);
		MyGUI::ButtonPtr b2 = mGUI->findWidget<MyGUI::Button>("inputbutton_" + actionName + "_" + schemaName + "_" + "2", "", false);
		if (b1)  b1->setCaption(StrFromKey(incKey));
		if (b2)  b2->setCaption(StrFromKey(decKey));
		
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

	#if 0
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
	#endif

	return true;
}


///  Fps stats
// ------------------------------------------------------------------------
void BaseApp::updateStats()
{
	// Print camera pos, rot
	
	static char s[128];
	// Only for 1 local player
	if (mSplitMgr && mSplitMgr->mNumViewports == 1)
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
