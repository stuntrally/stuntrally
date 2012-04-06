#include "pch.h"
#include "common/Defines.h"
#include "BaseApp.h"
#include "FollowCamera.h"
#include "../vdrift/pathmanager.h"
#include "SplitScreen.h"

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
	
	return kcMap[k];
}


///  Fps stats
// ------------------------------------------------------------------------
void BaseApp::updateStats()
{
	// Print camera pos, rot
	
	// Only for 1 local player
	/*if (mSplitMgr && mSplitMgr->mNumViewports == 1 && mbShowCamPos)
	{
		const Vector3& pos = (*mSplitMgr->mCameras.begin())->getDerivedPosition();
		const Quaternion& rot = (*mSplitMgr->mCameras.begin())->getDerivedOrientation();
		mDebugText = "Pos: "+fToStr(pos.x,1,5)+" "+fToStr(pos.y,1,5)+" "+fToStr(pos.z,1,5);
		mDebugText += "  Rot: "+fToStr(rot.x,3,6)+" "+fToStr(rot.y,3,6)+" "+fToStr(rot.z,3,6)+" "+fToStr(rot.w,3,6);
	}/**/

	try {  // TODO: tri & bat vals when compositor !...
		const RenderTarget::FrameStats& stats = mWindow->getStatistics();
		
		mOvrFps->setCaption( fToStr(stats.lastFPS, 1) );
		mOvrTris->setCaption( fToStr(Real(stats.triangleCount)/1000.f, 1)+"k");
		mOvrBat->setCaption( toStr(stats.batchCount) );

		mOvrDbg->setCaption( mFilText + "  " + mDebugText );
	}
	catch(...) {  /*ignore*/  }
}


void BaseApp::InitKeyNamesMap()
{
	using namespace OIS;
	//std::map<OIS::KeyCode, Ogre::String> kcMap;

	kcMap[KC_UNASSIGNED  ] = "Unassigned";  //  0x00,
	kcMap[KC_ESCAPE      ] = "Esc";  //  0x01,
	kcMap[KC_1           ] = "1";  //  0x02,
	kcMap[KC_2           ] = "2";  //  0x03,
	kcMap[KC_3           ] = "3";  //  0x04,
	kcMap[KC_4           ] = "4";  //  0x05,
	kcMap[KC_5           ] = "5";  //  0x06,
	kcMap[KC_6           ] = "6";  //  0x07,
	kcMap[KC_7           ] = "7";  //  0x08,
	kcMap[KC_8           ] = "8";  //  0x09,
	kcMap[KC_9           ] = "9";  //  0x0A,
	kcMap[KC_0           ] = "0";  //  0x0B,
	kcMap[KC_MINUS       ] = "-";  //  0x0C,    // - on main keyboard
	kcMap[KC_EQUALS      ] = "=";  //  0x0D,
	kcMap[KC_BACK        ] = "Backspace";  //  0x0E,    // backspace
	kcMap[KC_TAB         ] = "Tab";  //  0x0F,
	kcMap[KC_Q           ] = "Q";  //  0x10,
	kcMap[KC_W           ] = "W";  //  0x11,
	kcMap[KC_E           ] = "E";  //  0x12,
	kcMap[KC_R           ] = "R";  //  0x13,
	kcMap[KC_T           ] = "T";  //  0x14,
	kcMap[KC_Y           ] = "Y";  //  0x15,
	kcMap[KC_U           ] = "U";  //  0x16,
	kcMap[KC_I           ] = "I";  //  0x17,
	kcMap[KC_O           ] = "O";  //  0x18,
	kcMap[KC_P           ] = "P";  //  0x19,
	kcMap[KC_LBRACKET    ] = "[";  //  0x1A,
	kcMap[KC_RBRACKET    ] = "]";  //  0x1B,
	kcMap[KC_RETURN      ] = "Enter";  //  0x1C,    // Enter on main keyboard
	kcMap[KC_LCONTROL    ] = "LCtrl";  //  0x1D,
	kcMap[KC_A           ] = "A";  //  0x1E,
	kcMap[KC_S           ] = "S";  //  0x1F,
	kcMap[KC_D           ] = "D";  //  0x20,
	kcMap[KC_F           ] = "F";  //  0x21,
	kcMap[KC_G           ] = "G";  //  0x22,
	kcMap[KC_H           ] = "H";  //  0x23,
	kcMap[KC_J           ] = "J";  //  0x24,
	kcMap[KC_K           ] = "K";  //  0x25,
	kcMap[KC_L           ] = "L";  //  0x26,
	kcMap[KC_SEMICOLON   ] = ";";  //  0x27,
	kcMap[KC_APOSTROPHE  ] = "\'";  //  0x28,
	kcMap[KC_GRAVE       ] = "`";  //  0x29,    // accent
	kcMap[KC_LSHIFT      ] = "LShift";  //  0x2A,
	kcMap[KC_BACKSLASH   ] = "\\";  //  0x2B,
	kcMap[KC_Z           ] = "Z";  //  0x2C,
	kcMap[KC_X           ] = "X";  //  0x2D,
	kcMap[KC_C           ] = "C";  //  0x2E,
	kcMap[KC_V           ] = "V";  //  0x2F,
	kcMap[KC_B           ] = "B";  //  0x30,
	kcMap[KC_N           ] = "N";  //  0x31,
	kcMap[KC_M           ] = "M";  //  0x32,
	kcMap[KC_COMMA       ] = ",";  //  0x33,
	kcMap[KC_PERIOD      ] = ".";  //  0x34,    // . on main keyboard
	kcMap[KC_SLASH       ] = "/";  //  0x35,    // / on main keyboard
	kcMap[KC_RSHIFT      ] = "RShift";  //  0x36,
	kcMap[KC_MULTIPLY    ] = "Num *";  //  0x37,    // * on numeric keypad
	kcMap[KC_LMENU       ] = "LAlt";  //  0x38,    // left Alt
	kcMap[KC_SPACE       ] = "Space";  //  0x39,
	kcMap[KC_CAPITAL     ] = "Caps Lock";  //  0x3A,

	kcMap[KC_F1          ] = "F1";  //  0x3B,
	kcMap[KC_F2          ] = "F2";  //  0x3C,
	kcMap[KC_F3          ] = "F3";  //  0x3D,
	kcMap[KC_F4          ] = "F4";  //  0x3E,
	kcMap[KC_F5          ] = "F5";  //  0x3F,
	kcMap[KC_F6          ] = "F6";  //  0x40,
	kcMap[KC_F7          ] = "F7";  //  0x41,
	kcMap[KC_F8          ] = "F8";  //  0x42,
	kcMap[KC_F9          ] = "F9";  //  0x43,
	kcMap[KC_F10         ] = "F10";  //  0x44,
	kcMap[KC_NUMLOCK     ] = "Num Lock";  //  0x45,
	kcMap[KC_SCROLL      ] = "Scroll Lock";  //  0x46,    // Scroll Lock
	kcMap[KC_NUMPAD7     ] = "Num 7";  //  0x47,
	kcMap[KC_NUMPAD8     ] = "Num 8";  //  0x48,
	kcMap[KC_NUMPAD9     ] = "Num 9";  //  0x49,
	kcMap[KC_SUBTRACT    ] = "Num -";  //  0x4A,    // - on numeric keypad
	kcMap[KC_NUMPAD4     ] = "Num 4";  //  0x4B,
	kcMap[KC_NUMPAD5     ] = "Num 5";  //  0x4C,
	kcMap[KC_NUMPAD6     ] = "Num 6";  //  0x4D,
	kcMap[KC_ADD         ] = "Num +";  //  0x4E,    // + on numeric keypad
	kcMap[KC_NUMPAD1     ] = "Num 1";  //  0x4F,
	kcMap[KC_NUMPAD2     ] = "Num 2";  //  0x50,
	kcMap[KC_NUMPAD3     ] = "Num 3";  //  0x51,
	kcMap[KC_NUMPAD0     ] = "Num 0";  //  0x52,
	kcMap[KC_DECIMAL     ] = "Num .";  //  0x53,    // . on numeric keypad
	kcMap[KC_OEM_102     ] = "OEM 102";  //  0x56,    // < > | on UK/Germany keyboards
	kcMap[KC_F11         ] = "F11";  //  0x57,
	kcMap[KC_F12         ] = "F12";  //  0x58,
	kcMap[KC_F13         ] = "F13";  //  0x64,    //                     (NEC PC98)
	kcMap[KC_F14         ] = "F14";  //  0x65,    //                     (NEC PC98)
	kcMap[KC_F15         ] = "F15";  //  0x66,    //                     (NEC PC98)

	kcMap[KC_KANA        ] = "KANA";  //  0x70,    // (Japanese keyboard)
	kcMap[KC_ABNT_C1     ] = "ABNT_C1";  //  0x73,    // / ? on Portugese (Brazilian) keyboards
	kcMap[KC_CONVERT     ] = "CONVERT";  //  0x79,    // (Japanese keyboard)
	kcMap[KC_NOCONVERT   ] = "NOCONVERT";  //  0x7B,    // (Japanese keyboard)
	kcMap[KC_YEN         ] = "YEN";  //  0x7D,    // (Japanese keyboard)
	kcMap[KC_ABNT_C2     ] = "ABNT_C2";  //  0x7E,    // Numpad . on Portugese (Brazilian) keyboards
	kcMap[KC_NUMPADEQUALS] = "NUMPADEQUALS";  //  0x8D,    // ] = "";  //  on numeric keypad (NEC PC98)
	kcMap[KC_PREVTRACK   ] = "PREV TRACK";  //  0x90,    // Previous Track (kcMap[KC_CIRCUMFLEX on Japanese keyboard)
	kcMap[KC_AT          ] = "AT";  //  0x91,    //                     (NEC PC98)
	kcMap[KC_COLON       ] = "COLON";  //  0x92,    //                     (NEC PC98)
	kcMap[KC_UNDERLINE   ] = "UNDERLINE";  //  0x93,    //                     (NEC PC98)
	kcMap[KC_KANJI       ] = "KANJI";  //  0x94,    // (Japanese keyboard)
	kcMap[KC_STOP        ] = "STOP";  //  0x95,    //                     (NEC PC98)
	kcMap[KC_AX          ] = "AX";  //  0x96,    //                     (Japan AX)
	kcMap[KC_UNLABELED   ] = "UNLABELED";  //  0x97,    //                        (J3100)
	kcMap[KC_NEXTTRACK   ] = "NEXT TRACK";  //  0x99,    // Next Track
	kcMap[KC_NUMPADENTER ] = "Num Enter";  //  0x9C,    // Enter on numeric keypad
	kcMap[KC_RCONTROL    ] = "RCtrl";  //  0x9D,
	kcMap[KC_MUTE        ] = "MUTE";  //  0xA0,    // Mute
	kcMap[KC_CALCULATOR  ] = "CALCULATOR";  //  0xA1,    // Calculator
	kcMap[KC_PLAYPAUSE   ] = "PLAYPAUSE";  //  0xA2,    // Play / Pause
	kcMap[KC_MEDIASTOP   ] = "MEDIA STOP";  //  0xA4,    // Media Stop
	kcMap[KC_VOLUMEDOWN  ] = "VOLUME DOWN";  //  0xAE,    // Volume -
	kcMap[KC_VOLUMEUP    ] = "VOLUME UP";  //  0xB0,    // Volume +
	kcMap[KC_WEBHOME     ] = "WEBHOME";  //  0xB2,    // Web home

	kcMap[KC_NUMPADCOMMA ] = "Num .";  //  0xB3,    // , on numeric keypad (NEC PC98)
	kcMap[KC_DIVIDE      ] = "Num /";  //  0xB5,    // / on numeric keypad
	kcMap[KC_SYSRQ       ] = "PtrScr";  //  0xB7,  // SysRq
	kcMap[KC_RMENU       ] = "RAlt";  //  0xB8,    // right Alt
	kcMap[KC_PAUSE       ] = "PAUSE";  //  0xC5,    // Pause
	kcMap[KC_HOME        ] = "Home";  //  0xC7,    // Home on arrow keypad
	kcMap[KC_UP          ] = "Up";  //  0xC8,    // UpArrow on arrow keypad
	kcMap[KC_PGUP        ] = "PgUp";  //  0xC9,    // PgUp on arrow keypad
	kcMap[KC_LEFT        ] = "Left";  //  0xCB,    // LeftArrow on arrow keypad
	kcMap[KC_RIGHT       ] = "Right";  //  0xCD,    // RightArrow on arrow keypad
	kcMap[KC_END         ] = "End";  //  0xCF,    // End on arrow keypad
	kcMap[KC_DOWN        ] = "Down";  //  0xD0,    // DownArrow on arrow keypad
	kcMap[KC_PGDOWN      ] = "PgDn";  //  0xD1,    // PgDn on arrow keypad
	kcMap[KC_INSERT      ] = "Insert";  //  0xD2,    // Insert on arrow keypad
	kcMap[KC_DELETE      ] = "Delete";  //  0xD3,    // Delete on arrow keypad
	kcMap[KC_LWIN        ] = "LWin";  //  0xDB,    // Left Windows key
	kcMap[KC_RWIN        ] = "RWin";  //  0xDC,    // Right Windows key
	kcMap[KC_APPS        ] = "APPS";  //  0xDD,    // AppMenu key
	kcMap[KC_POWER       ] = "POWER";  //  0xDE,    // System Power
	kcMap[KC_SLEEP       ] = "SLEEP";  //  0xDF,    // System Sleep
	kcMap[KC_WAKE        ] = "WAKE";  //  0xE3,    // System Wake
	kcMap[KC_WEBSEARCH   ] = "WEBSEARCH";  //  0xE5,    // Web Search
	kcMap[KC_WEBFAVORITES] = "WEBFAVORITES";  //  0xE6,    // Web Favorites
	kcMap[KC_WEBREFRESH  ] = "WEBREFRESH";  //  0xE7,    // Web Refresh
	kcMap[KC_WEBSTOP     ] = "WEBSTOP";  //  0xE8,    // Web Stop
	kcMap[KC_WEBFORWARD  ] = "WEBFORWARD";  //  0xE9,    // Web Forward
	kcMap[KC_WEBBACK     ] = "WEBBACK";  //  0xEA,    // Web Back
	kcMap[KC_MYCOMPUTER  ] = "MYCOMPUTER";  //  0xEB,    // My Computer
	kcMap[KC_MAIL        ] = "MAIL";  //  0xEC,    // Mail
	kcMap[KC_MEDIASELECT ] = "MEDIASELECT";  //  0xED     // Media Select
}

bool BaseApp::IsFocGui()
{
	return isFocGui || isFocRpl ||
		(mWndChampStage && mWndChampStage->getVisible()) ||
		(mWndChampEnd && mWndChampEnd->getVisible()) ||
		(mWndNetEnd && mWndNetEnd->getVisible());
}
