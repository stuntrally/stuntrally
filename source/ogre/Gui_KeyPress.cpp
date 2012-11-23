#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "OgreGame.h"
#include "FollowCamera.h"
#include "SplitScreen.h"
#include "common/Gui_Def.h"
#include "common/RenderConst.h"
#include "common/GraphView.h"
#include "common/Slider.h"
#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"

#include <MyGUI_PointerManager.h>
#include <OIS/OIS.h>
#include "../oisb/OISB.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <OgreRoot.h>
#include <OgreTerrain.h>
#include <OgreMaterialManager.h>
#include <OgreOverlay.h>
#include <OgreRenderWindow.h>
using namespace std;
using namespace Ogre;
using namespace MyGUI;


//-----------------------------------------------------------------------------------------------------------
//  Key pressed
//-----------------------------------------------------------------------------------------------------------

// util
bool App::actionIsActive(std::string name, std::string pressed)
{
	std::string actionKey = GetInputName(mOISBsys->lookupAction("General/" + name)->mBindings[0]->mBindables[0].second->getBindableName());
	boost::to_lower(actionKey);
	boost::to_lower(pressed);
	return actionKey == pressed;
}

bool App::keyPressed( const OIS::KeyEvent &arg )
{
	// update all keystates  (needed for all action("..") from oisb)
	if (mOISBsys)
		mOISBsys->process(0.000/*?0*/);
	
	// action key == pressed key
	#define action(s)  actionIsActive(s, mKeyboard->getAsString(arg.key))

	bool tweak = isTweak();

	if (!bAssignKey)
	{
		//  change gui tabs
		if (isFocGui && !pSet->isMain)
		{
			MyGUI::TabPtr tab = 0;  MyGUI::TabControl* sub = 0;
			switch (pSet->inMenu)
			{	case WND_Game:
				case WND_Champ:    tab = mWndTabsGame;  sub = vSubTabsGame[tab->getIndexSelected()];  break;
				case WND_Replays:  tab = mWndTabsRpl;  break;
				case WND_Help:     tab = mWndTabsHelp;  break;
				case WND_Options:  tab = mWndTabsOpts;  sub = vSubTabsOpts[tab->getIndexSelected()];  break;
			}
			if (tab)
			if (shift)
			{	if (action("PrevTab")) {  // prev gui subtab
					if (sub)  {  int num = sub->getItemCount();
						sub->setIndexSelected( (sub->getIndexSelected() - 1 + num) % num );  }	}
				else if (action("NextTab")) {  // next gui subtab
					if (sub)  {  int num = sub->getItemCount();
						sub->setIndexSelected( (sub->getIndexSelected() + 1) % num );  }  }
			}else
			{	int num = tab->getItemCount()-1, i = 0, n = 0;
				if (action("PrevTab")) {
					i = tab->getIndexSelected();
					do{  if (i==1)  i = num;  else  --i;  ++n;  }
					while (n < num && tab->getButtonWidthAt(i) == 1);
					tab->setIndexSelected(i);  MenuTabChg(tab,i);  return true;  }
				else if (action("NextTab")) {
					i = tab->getIndexSelected();
					do{  if (i==num)  i = 1;  else  ++i;  ++n;  }
					while (n < num && tab->getButtonWidthAt(i) == 1);
					tab->setIndexSelected(i);  MenuTabChg(tab,i);  return true;  }
			}
		}
		else if (!isFocGui && pSet->show_graphs)  // change graphs type
		{
			int& v = (int&)pSet->graphs_type;  int vo = v;
			if (action("PrevTab"))  v = (v-1 + Gh_ALL) % Gh_ALL;
			if (action("NextTab"))	v = (v+1) % Gh_ALL;
			if (vo != v)
			{	float fv = float(v) / Gh_ALL;
				slGraphsType(slGraphT, fv);  slGraphT->setValue(fv);
				if (v == 4)  iUpdTireGr = 1;  //upd now
			}
		}

		//  gui on/off  or close wnds
		if (action("ShowOptions") && !alt)
		{
			if (mWndNetEnd && mWndNetEnd->getVisible())  {  mWndNetEnd->setVisible(false);  // hide netw end
				return false;	}
			else
			{
				if (mWndChampEnd && mWndChampEnd->getVisible())  mWndChampEnd->setVisible(false);  // hide champs end
				toggleGui(true);  return false;
			}
		}

		//  new game - reload   not in multiplayer
		if (action("RestartGame") && !mClient)
		{	NewGame();  return false;	}

		//  new game - fast (same track & cars)
		if (action("ResetGame") && !mClient)
		{
			for (int c=0; c < carModels.size(); ++c)
			{
				CarModel* cm = carModels[c];
				if (cm->pCar)  cm->pCar->bResetPos = true;
				if (cm->fCam)  cm->fCam->first = true;
				cm->ResetChecks();
				cm->iWonPlace = 0;  cm->iWonPlaceOld = 0;
				cm->iWonMsgTime = 0.f;
			}
			pGame->timer.Reset(-1);
			pGame->timer.pretime = mClient ? 2.0f : pSet->game.pre_time;  // same for all multi players
			carIdWin = 1;  //
			ghost.Clear(); //
		}


		//  Screen shot
		//if (action("Screenshot"))  //isnt working
		const OISB::Action* act = OISB::System::getSingleton().lookupAction("General/Screenshot",false);
		if (act && act->isActive())
		{	mWindow->writeContentsToTimestampedFile(PATHMANAGER::GetScreenShotDir() + "/", ".jpg");
			return false;	}
		
		using namespace OIS;


		//  main menu keys
		if (pSet->isMain && isFocGui)
		{
			switch (arg.key)
			{
			case KC_UP:  case KC_NUMPAD8:
				pSet->inMenu = (pSet->inMenu-1+WND_ALL)%WND_ALL;
				toggleGui(false);  return true;

			case KC_DOWN:  case KC_NUMPAD2:
				pSet->inMenu = (pSet->inMenu+1)%WND_ALL;
				toggleGui(false);  return true;

			case KC_RETURN:
				pSet->isMain = false;
				toggleGui(false);  return true;
			}
		}

		//  esc
		if (arg.key == KC_ESCAPE)
		{
			if (pSet->escquit)
				mShutDown = true;	// quit
			else
				if (mWndChampStage->getVisible())  ///  close champ wnds
					btnChampStageStart(0);
				else
					toggleGui(true);	// gui on/off
			return true;
		}
		
		
		//  shortcut keys for gui access (alt-Q,C,S,G,V,.. )
		if (alt)
			switch (arg.key)
			{
				case KC_Z:  // alt-Z Tweak (alt-shift-Z save&reload)
					TweakToggle();	return true;
					
				case KC_Q:	GuiShortcut(WND_Game, 1);	return true;  // Q Track
				case KC_C:	GuiShortcut(WND_Game, 2);	return true;  // C Car
				case KC_W:	GuiShortcut(WND_Game, 4);	return true;  // W Setup
				case KC_U:	GuiShortcut(WND_Game, 5);	return true;  // U Multiplayer
				case KC_H:	GuiShortcut(WND_Champ,6);	return true;  // H Champs

				case KC_R:	GuiShortcut(WND_Replays, 1);	return true;  // R Replays

				case KC_S:	GuiShortcut(WND_Options, 1);	return true;  // S Screen
				 case KC_E:	GuiShortcut(WND_Options, 1,1);	return true;  // E -Effects
				case KC_G:	GuiShortcut(WND_Options, 2);	return true;  // G Graphics
				 case KC_N:	GuiShortcut(WND_Options, 2,2);	return true;  // N -Vegetation

				case KC_V:	GuiShortcut(WND_Options, 3);	return true;  // V View
				 case KC_M:	GuiShortcut(WND_Options, 3,1);	return true;  // M -Minimap
				 case KC_O:	GuiShortcut(WND_Options, 3,3);	return true;  // O -Other
				case KC_I:	GuiShortcut(WND_Options, 4);	return true;  // I Input
			}

		///* tire edit */
		if (pSet->graphs_type == Gh_TireEdit && !tweak)
		{
			int& iCL = iEdTire==1 ? iCurLong : (iEdTire==0 ? iCurLat : iCurAlign);
			int iCnt = iEdTire==1 ? 11 : (iEdTire==0 ? 15 : 18);
			switch (arg.key)
			{
				case KC_HOME: case KC_NUMPAD7:  // mode long/lat
					iEdTire = iEdTire==1 ? 0 : 1;  iUpdTireGr=1;  return true;

				case KC_END: case KC_NUMPAD1:	// mode align
					iEdTire = iEdTire==2 ? 0 : 2;  iUpdTireGr=1;  return true;

				/*case KC_1:*/ case KC_PGUP: case KC_NUMPAD9:    // prev val
					iCL = (iCL-1 +iCnt)%iCnt;  iUpdTireGr=1;  return true;

				/*case KC_2:*/ case KC_PGDOWN: case KC_NUMPAD3:  // next val
					iCL = (iCL+1)%iCnt;  iUpdTireGr=1;  return true;
			}
		}
		
		
		//  not main menus
		//if (/*&& !pSet->isMain*/)
		if (!tweak)
		{
			Widget* wf = MyGUI::InputManager::getInstance().getKeyFocusWidget();
			bool edFoc = wf && wf->getTypeName() == "EditBox";
			//if (wf)  LogO(wf->getTypeName()+" " +toStr(edFoc));
			switch (arg.key)
			{
				case KC_BACK:
					if (mWndChampStage->getVisible())	// back from champs stage wnd
					{	btnChampStageBack(0);  return true;  }

					if (pSet->isMain)  break;
					if (isFocGui)
					{	if (edFoc)  break;
						pSet->isMain = true;  toggleGui(false);  }
					else
						if (mWndRpl && !isFocGui)	bRplWnd = !bRplWnd;  // replay controls
					return true;

				case KC_P:		// replay play/pause
					if (bRplPlay && !isFocGui)
					{	bRplPause = !bRplPause;  UpdRplPlayBtn();
						return true;  }
					break;
					
				case KC_K:		// replay car ofs
					if (bRplPlay && !isFocGui)	{	--iRplCarOfs;  return true;  }
					break;
				case KC_L:		// replay car ofs
					if (bRplPlay && !isFocGui)	{	++iRplCarOfs;  return true;  }
					break;
					
				case KC_F:		// focus on find edit
					if (ctrl && edFind && isFocGui &&
						!pSet->isMain && pSet->inMenu == WND_Game && mWndTabsGame->getIndexSelected() == 1)
					{
						MyGUI::InputManager::getInstance().resetKeyFocusWidget();
						MyGUI::InputManager::getInstance().setKeyFocusWidget(edFind);
						return true;
					}	break;
					

				case KC_F7:		// Times
					if (shift)
					{	WP wp = chOpponents;  ChkEv(show_opponents);  ShowHUD();  }
					else if (!ctrl)
					{	WP wp = chTimes;  ChkEv(show_times);  ShowHUD();  }
					return false;
					
				case KC_F8:		// car debug bars
					if (ctrl)
					{	WP wp = chDbgB;  ChkEv(car_dbgbars);   ShowHUD();  }
					else		// Minimap
					if (!shift)
					{	WP wp = chMinimp;  ChkEv(trackmap);
						for (int c=0; c < 4; ++c)
							if (ndMap[c])  ndMap[c]->setVisible(pSet->trackmap);
					}	return false;

				case KC_F9:
					if (ctrl)	// car debug surfaces
					{	WP wp = chDbgS;  ChkEv(car_dbgsurf);  ShowHUD();  }
					else
					if (shift)	// car debug text
					{	WP wp = chDbgT;  ChkEv(car_dbgtxt);  ShowHUD();  }
					else		// graphs
					{	WP wp = chGraphs;  ChkEv(show_graphs);
						for (int i=0; i < graphs.size(); ++i)
							graphs[i]->SetVisible(pSet->show_graphs);
					}
					return true;

				case KC_F11:
					if (shift)	// profiler times
					{	WP wp = chProfTxt;  ChkEv(profilerTxt);  ShowHUD();  }
					else
					if (!ctrl)  // Fps
					{	WP wp = chFps;  ChkEv(show_fps); 
						if (pSet->show_fps)  mFpsOverlay->show();  else  mFpsOverlay->hide();
						return false;
					}	break;

				case KC_F10:	//  blt debug, txt
					if (shift)
					{	WP wp = chBltTxt;  ChkEv(bltProfilerTxt);  return false;  }
					else if (ctrl)
					{	WP wp = chBlt;  ChkEv(bltDebug);  return false;  }
					else		// wireframe
						toggleWireframe();
					return false;

				
				case KC_RETURN:		///  close champ wnds
					if (mWndChampStage->getVisible())
						btnChampStageStart(0);
					else			//  chng trk/car + new game  after up/dn
					if (isFocGui && !pSet->isMain)
						switch (pSet->inMenu)
						{
						case WND_Replays:	btnRplLoad(0);  break;
						case WND_Game:  case WND_Champ:
						{	switch (mWndTabsGame->getIndexSelected())
							{
							case 1:	changeTrack();	btnNewGame(0);  break;
							case 2:	changeCar();	btnNewGame(0);  break;
							case 5:	chatSendMsg();  break;
							case 6:	btnChampStart(0);  break;
						}	break;
					}	}
					else
					if (mClient && !isFocGui)  // show/hide players net wnd
					{	mWndNetEnd->setVisible(!mWndNetEnd->getVisible());  return true;  }
					break;
			}
		}
	}
	InputBind(arg.key);
	

	//  gui input
	if (mGUI && (isFocGui || tweak))
	{
		MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::Enum(arg.key), arg.text);
		return false;
	}

	return true;
}


///  Tweak
//-----------------------------------------------------------------------------------------------------------

void App::TweakCarSave()
{
	String text = edTweak->getCaption();
	if (text == "")  return;
	text = StringUtil::replaceAll(text, "##", "#");
	text = StringUtil::replaceAll(text, "#E5F4FF", "");  //!

	std::string path, pathUser, pathUserDir;
	bool user = GetCarPath(&path, &pathUser, &pathUserDir, pSet->game.car[0], sc->asphalt);
	
	PATHMANAGER::CreateDir(pathUserDir, pGame->error_output);
	std::ofstream fo(pathUser.c_str());
	fo << text.c_str();
	fo.close();
	
	NewGame();
}

void App::TweakCarLoad()
{
	std::string path, pathUser, pathUserDir;
	bool user = GetCarPath(&path, &pathUser, &pathUserDir, pSet->game.car[0], sc->asphalt);

	if (!PATHMANAGER::FileExists(path))
	{
		edTweak->setCaption("");
		txtTweakPath->setCaption("Not Found ! " + path);
		txtTweakPath->setColour(Colour(1,0,0));
	}else
	{
		std::ifstream fi(path.c_str());
		String text = "", s;
		while (getline(fi,s))
			text += s + "\n";
		fi.close();

		text = StringUtil::replaceAll(text, "#", "##");
		text = StringUtil::replaceAll(text, "#E5F4FF", "");  //!
		edTweak->setCaption(UString(text));
		//edTweak->setVScrollPosition(0);

		size_t p = path.find("cars");
		if (p != string::npos)
			path = path.substr(p+5, path.length());
		txtTweakPath->setCaption((user ? "User: " : "Original: ") + path);
		txtTweakPath->setTextColour(user ? Colour(1,1,0.5) : Colour(0.5,1,1));
		
		MyGUI::InputManager::getInstance().resetKeyFocusWidget();
		MyGUI::InputManager::getInstance().setKeyFocusWidget(edTweak);
	}
}

void App::CmbTweakCarSet(CMB)
{
}
void App::CmbTweakTireSet(CMB)
{
}

void App::CmbEdTweakCarSet(EditPtr ed)
{
}
void App::CmbEdTweakTireSet(EditPtr ed)
{
}


//  tweak save car and reload game
void App::TweakTireSave()
{
	///TODO: save tires, ed name, game reload all..
	// ed car setup name, chk exist, load
	// jump to section,  help on current line
	// ed find text? syntax clr?=
/*	name = asphalt
	version = 1

	[ params ]
	#--------	Lateral force
	a0=0.776		#	Shape factor												A0
	a1=-0          #	Load infl. on lat. friction coeff (*1000)  (1/kN)			A1
	a2=3087         #	Lateral friction coefficient at load = 0 (*1000)			A2
	a3=2165          #	Maximum stiffness   (N/deg)									A3
	a4=18.0          #	Load at maximum stiffness   (kN)							A4
	a5=0.013        #	Camber influence on stiffness   (%/deg/100)					A5
	a6=-0.14        #	Curvature change with load									A6
	a7=0.14         #	Curvature at load = 0										A7
	a8=0.019        #	Horizontal shift because of camber  (deg/deg)				A8
	a9=-0.019       #	Load influence on horizontal shift  (deg/kN)				A9
	a10=-0.18       #	Horizontal shift at load = 0  (deg)							A10
	a111=0          #	Camber influence on vertical shift  (N/deg/kN)				A11.1
	a112=0.0       #	Camber influence on vertical shift  (N/deg/kN**2)			A11.2
	a12=0.0        #	Load influence on vertical shift  (N/kN)					A12
	a13=0.0        #	Vertical shift at load = 0  (N)								A13
	#--------	Longitudinal force
	b0=1.55         #	Shape factor   B0
	b1=-0.49        #	Load infl. on long. friction coeff (*1000)  (1/kN)   B1
	b2=3439         #	Longitudinal friction coefficient at load = 0 (*1000)  B2
	b3=85.5         #	Curvature factor of stiffness   (N/%/kN**2) . B3
	b4=470.0        #	Change of stiffness with load at load = 0 (N/%/kN)   B4
	b5=0.0          #	Change of progressivity of stiffness/load (1/kN)   B5
	b6=0.0008       #	Curvature change with load   B6
	b7=0.005        #	Curvature change with load   B7
	b8=-0.024       #	Curvature at load = 0   B8
	b9=0.00        #	Load influence on horizontal shift   (%/kN)   B9
	b10=0.0         #	Horizontal shift at load = 0   (%)   B10
	b11=0           #	Load influence on vertical shift   (N/kN)   B11
	b12=0           #	Vertical shift at load = 0   (N)   B12
	#---------	Aligning moment
	c0=2.10         #	Shape factor   C0
	c1=-3.9         #	Load influence of peak value   (Nm/kN**2)   C1
	c2=-3.9         #	Load influence of peak value   (Nm/kN)   C2
	c3=-1.26        #	Curvature factor of stiffness   (Nm/deg/kN**2) C3		
	c4=-8.20        #	Change of stiffness with load at load = 0 (Nm/deg/kN)   C4
	c5=0.025        #	Change of progressivity of stiffness/load (1/kN)   C5
	c6=0.0          #	Camber influence on stiffness   (%/deg/100)   C6
	c7=0.044        #	Curvature change with load   C7
	c8=-0.58        #	Curvature change with load   C8
	c9=0.18         #	Curvature at load = 0   C9
	c10=0.043       #	Camber influence of stiffness   C10
	c11=0.048       #	Camber influence on horizontal shift (deg/deg)  C11
	c12=-0.0035     #	Load influence on horizontal shift (deg/kN)  C1
	c13=-0.18       #	Horizontal shift at load = 0 (deg)  C13
	c14=0.14        #	Camber influence on vertical shift (Nm/deg/kN**2) C14
	c15=-1.029      #	Camber influence on vertical shift (Nm/deg/kN)  C15
	c16=0.27        #	Load influence on vertical shift (Nm/kN)  C16
	c17=-1.1        #	Vertical shift at load = 0 (Nm)  C17c0=2.2							
	#---------
*/
}

void App::btnTweakCarSave(WP){	TweakCarSave();  }
void App::btnTweakCarLoad(WP){	TweakCarLoad();  }
void App::btnTweakTireSave(WP){	TweakTireSave();  }


///  Tweak read / save file
//-----------------------------------------------------------------------------------------
void App::TweakToggle()
{
	//  window
	bool vis = !mWndTweak->getVisible();
	mWndTweak->setVisible(vis);

	std::string path, pathUser, pathUserDir;
	bool user = GetCarPath(&path, &pathUser, &pathUserDir, pSet->game.car[0], sc->asphalt);
	
	//  load  if car changed
	static string lastPath = "";
	if (lastPath != path || ctrl)  // force reload  ctrl-alt-Z
	{	lastPath = path;
		TweakCarLoad();
	}
	
	//  save and reload  shift-alt-Z
	if (!vis && shift)
		TweakCarSave();
}


//  Get car file path
bool App::GetCarPath(std::string* pathCar, std::string* pathSave, std::string* pathSaveDir,
	std::string carname, bool asphalt, std::string tweakSetup, bool forceOrig)
{
	std::string file = carname + (asphalt ? "_a":"") + ".car",
		pathOrig  = PATHMANAGER::GetCarPath()     + "/"+carname+"/"+ file,
		pathUserD = PATHMANAGER::GetCarPathUser() + "/"+carname+"/"+ (tweakSetup != "" ? tweakSetup+"/" : ""),
		pathUser  = pathUserD + file;

	if (pathSave)  *pathSave = pathUser;
	if (pathSaveDir)  *pathSaveDir = pathUserD;
	
	if (!forceOrig && PATHMANAGER::FileExists(pathUser))
	{
		*pathCar = pathUser;
		return true;
	}
	*pathCar = pathOrig;
	return false;
}
