#include "pch.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/GuiCom.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "CScene.h"
#include "../road/Road.h"
#include "../vdrift/pathmanager.h"
#include <OgreRenderTexture.h>
#include <MyGUI.h>
#include "../sdl4ogre/sdlinputwrapper.hpp"
using namespace MyGUI;
using namespace Ogre;


//---------------------------------------------------------------------------------------------------------------
//  Key Press
//---------------------------------------------------------------------------------------------------------------

bool App::keyPressed(const SDL_KeyboardEvent &arg)
{
	SDL_Scancode skey = arg.keysym.scancode;
	#define key(a)  SDL_SCANCODE_##a
	
	///  Preview camera  ---------------------
	if (edMode == ED_PrvCam)
	{
		switch (skey)
		{
			case key(ESCAPE):  case key(F7):  togPrvCam();  break;  // exit

			case key(KP_ENTER):  case key(RETURN):  // save screen
			{	int u = pSet->allow_save ? pSet->gui.track_user : 1;
				rt[RT_View].tex->writeContentsToFile(gcom->pathTrk[u] + pSet->gui.track + "/preview/view.jpg");
				gcom->listTrackChng(gcom->trkList,0);  // upd gui img
				gui->Status("#{Saved}", 1,1,0);
			}	break;

			case key(F12):  // screenshot
				mWindow->writeContentsToTimestampedFile(PATHMANAGER::Screenshots() + "/", ".jpg");
				return true;
		}
		return true;
	}

	//  main menu keys
	Widget* wf = InputManager::getInstance().getKeyFocusWidget();
	bool editFocus = wf && wf->getTypeName() == "EditBox";

	if (pSet->bMain && bGuiFocus)
	{
		switch (skey)
		{
		case key(UP):  case key(KP_8):
			pSet->inMenu = (pSet->inMenu-1+WND_ALL)%WND_ALL;
			gui->toggleGui(false);  return true;

		case key(DOWN):  case key(KP_2):
			pSet->inMenu = (pSet->inMenu+1)%WND_ALL;
			gui->toggleGui(false);  return true;

		case key(KP_ENTER):  case key(RETURN):
			pSet->bMain = false;
			gui->toggleGui(false);  return true;
		}
	}
	if (!pSet->bMain && bGuiFocus)
	{
		switch (skey)
		{
		case key(BACKSPACE):
			if (pSet->bMain)  break;
			if (bGuiFocus)
			{	if (editFocus)  break;
				pSet->bMain = true;  gui->toggleGui(false);  }
			return true;
		}
	}

	//  change gui tabs
	TabPtr tab = 0;  TabControl* sub = 0;  int iTab1 = 1;
	if (bGuiFocus && !pSet->bMain)
	switch (pSet->inMenu)
	{
		case WND_Track:   tab = mWndTabsTrack;  sub = gui->vSubTabsTrack[tab->getIndexSelected()];  break;
		case WND_Edit:    tab = mWndTabsEdit;   sub = gui->vSubTabsEdit[tab->getIndexSelected()];  break;
		case WND_Help:    tab = sub = gui->vSubTabsHelp[1];  iTab1 = 0;  break;
		case WND_Options: tab = mWndTabsOpts;   sub = gui->vSubTabsOpts[tab->getIndexSelected()];  break;
	}
	bool edit = bEdit();
	SplineRoad* road = scn->road;
	bool bRoad = edMode == ED_Road && road && edit;


	///  Pick open  ---------------------
	bool editGui = bGuiFocus && !pSet->bMain && pSet->inMenu==WND_Edit;
	if (skey==key(TAB) && editGui)
	{
		switch (tab->getIndexSelected())
		{	case TAB_Sun:  /*if (sub->getIndexSelected()==0)*/ {  gui->btnPickSky(0);  return true;  }  break;
			case TAB_Layers:  gui->btnPickTex(0);  return true;
			case TAB_Grass:  if (sub->getIndexSelected()==1) {  gui->btnPickGrass(0);  return true;  }  break;
			case TAB_Veget:  if (sub->getIndexSelected()==1) {  gui->btnPickVeget(0);  return true;  }  break;
			case TAB_Road:
				switch (sub->getIndexSelected())
				{
				case 0:  gui->btnPickRoad(0);   return true;
				case 1:  gui->btnPickPipe(0);   return true;
				case 2:  gui->btnPickRoadCol(0);   return true;
				}	break;
		}
		mWndPick->setVisible(false);
	}

	//  Global keys
	//------------------------------------------------------------------------------------------------------------------------------
	switch (skey)
	{
		case key(ESCAPE): //  quit
			if (pSet->escquit)
			{
				mShutDown = true;
			}	return true;

		case key(F1):  case key(GRAVE):
			if (ctrl)  // context help (show for cur mode)
			{
				if (bMoveCam)         gui->GuiShortcut(WND_Help, 1, 1);
				else switch (edMode)
				{	case ED_Smooth: case ED_Height: case ED_Filter:
					case ED_Deform:   gui->GuiShortcut(WND_Help, 1, 3);  break;
					case ED_Road:     gui->GuiShortcut(WND_Help, 1, 5);  break;
					case ED_Start:    gui->GuiShortcut(WND_Help, 1, 8);  break;
					case ED_Fluids:   gui->GuiShortcut(WND_Help, 1, 9);  break;
					case ED_Objects:  gui->GuiShortcut(WND_Help, 1, 10);  break;
					case ED_Particles:gui->GuiShortcut(WND_Help, 1, 11);  break;
					case ED_PrvCam:
					default:		  gui->GuiShortcut(WND_Help, 1, 0);  break;
			}	}
			else	//  Gui mode, Options
				gui->toggleGui(true);
			return true;

		case key(F12):  //  screenshot
			mWindow->writeContentsToTimestampedFile(PATHMANAGER::Screenshots() + "/",
				pSet->screen_png ? ".png" : ".jpg");
			return true;

		//  save, reload
		case key(F4):  if (!alt)  SaveTrack();  return true;
		case key(F5):  LoadTrack();  return true;
		
		case key(F8):  // update
			if (editGui)
			switch (tab->getIndexSelected())
			{	case TAB_Layers:  gui->btnUpdateLayers(0);  return true;
				case TAB_Grass:  gui->btnUpdateGrass(0);  return true;
				case TAB_Veget:  gui->btnUpdateVeget(0);  return true;
			}
			UpdateTrack();  return true;  // default full

		case key(F9):  // blendmap
			gui->ckDebugBlend.Invert();
			bTerUpdBlend = true;
			return true;


		//  prev num tab (layers,grasses,models)
		case key(1):  if (alt && !bRoad)  {  gui->NumTabNext(-1);  return true;  }  break;
		//  next num tab
		case key(2):  if (alt && !bRoad)  {  gui->NumTabNext( 1);  return true;  }  break;

		case key(F2):  // +-rt num
   			if (shift)
   			{	pSet->num_mini = (pSet->num_mini - 1 + RT_ALL) % RT_ALL;  UpdMiniVis();  }
   			else
   			if (bGuiFocus && tab && !pSet->bMain)
   				if (alt)  // prev gui subtab
   				{
   					if (sub)  {  int num = sub->getItemCount();
   						sub->setIndexSelected( (sub->getIndexSelected() - 1 + num) % num );  }
	   			}
   				else	// prev gui tab
   				{	int num = tab->getItemCount()-1, i = tab->getIndexSelected();
					if (i==iTab1)  i = num;  else  --i;
					tab->setIndexSelected(i);  if (iTab1==1)  gui->tabMainMenu(tab,i);
	   			}
   			break;

		case key(F3):  // tabs,sub
   			if (shift)
   			{	pSet->num_mini = (pSet->num_mini + 1) % RT_ALL;  UpdMiniVis();  }
   			else
   			if (bGuiFocus && tab && !pSet->bMain)
   				if (alt)  // next gui subtab
   				{
   					if (sub)  {  int num = sub->getItemCount();
   						sub->setIndexSelected( (sub->getIndexSelected() + 1) % num );  }
	   			}
	   			else	// next gui tab
	   			{	int num = tab->getItemCount()-1, i = tab->getIndexSelected();
					if (i==num)  i = iTab1;  else  ++i;
					tab->setIndexSelected(i);  if (iTab1==1)  gui->tabMainMenu(tab,i);
				}
   			break;
   			
   		case key(SPACE):  // subtabs
   			if (bGuiFocus && !editFocus && tab && !pSet->bMain)
				if (sub)  {  int num = sub->getItemCount();
					sub->setIndexSelected( (sub->getIndexSelected() + (shift ? -1 : 1) + num) % num );  }
			break;
   			
		case key(KP_ENTER):  case key(RETURN):  // load track
			if (bGuiFocus)
			if (mWndTabsTrack->getIndexSelected() == 1 && !pSet->bMain && pSet->inMenu == WND_Track)
				gui->btnNewGame(0);
   			break;


		//  Fps, WireFrame  F11
		case key(F11):
			if (ctrl)  gui->ckWireframe.Invert();  else  gui->ckFps.Invert();
			return true;

		//  Show Stats  ctrl-I
		case key(I):
   			if (ctrl) {  gui->ckInputBar.Invert();  return true;  }
			break;

		//  Top view  alt-Z
		case key(Z):
			if (alt)  {  gui->toggleTopView();  return true;  }
			break;

		//  load next track  F6
		case key(F6):
			if (pSet->check_load)
			{	gui->iLoadNext = shift ? -1 : 1;  return true;  }
			break;
	}

	//  GUI  keys in edits  ---------------------
	if (bGuiFocus && mGui && !alt && !ctrl)
	{
		InputManager::getInstance().injectKeyPress(KeyCode::Enum(mInputWrapper->sdl2OISKeyCode(arg.keysym.sym)), 0);
		return true;
	}


	///  Road keys  * * * * * * * * * * * * * * *
	if (bRoad)
		keyPressRoad(skey);


	//  ter brush shape
	if (edMode < ED_Road && !alt && edit)
	switch (skey)
	{
		case key(K):    if (ctrl)  {  mBrShape[curBr] = (EBrShape)((mBrShape[curBr]-1 + BRS_ALL) % BRS_ALL);  updBrush();  }  break;
		case key(L):    if (ctrl)  {  mBrShape[curBr] = (EBrShape)((mBrShape[curBr]+1) % BRS_ALL);            updBrush();  }  break;
		case key(N): case key(COMMA):   mBrOct[curBr] = std::max(1, mBrOct[curBr]-1);  updBrush();  break;
		case key(M): case key(PERIOD):  mBrOct[curBr] = std::min(7, mBrOct[curBr]+1);  updBrush();  break;

		case key(KP_ENTER):
		case key(RETURN):  brLockPos = !brLockPos;  break;
		case key(SPACE):  //  set brush height from terrain
			if (edMode == ED_Height)
			{	terSetH = road->posHit.y;  }
			break;
	}

	//  ter brush presets  ----
	if (edMode < ED_Road && alt && skey >= key(1) && skey <= key(0) && edit)
	{
		int id = skey - key(1);
		if (shift)  id += 10;
		SetBrushPreset(id);
	}


	//  Fluids, Objects, Emitters  * * * * * * * * * * *
	keyPressObjects(skey);


	///  Common Keys  ************************************************************************************************************
	if (alt)
	switch (skey)
	{
		case key(Q):  gui->GuiShortcut(WND_Track, 1);  return true;  // Q Track
		case key(O):  gui->GuiShortcut(WND_Track, 2);  return true;  // O Tools

		case key(W):  gui->GuiShortcut(WND_Track, 3);  return true;  // W Game
		case key(P):  gui->GuiShortcut(WND_Track, 4);  return true;  // P Pacenotes
		case key(J):  gui->GuiShortcut(WND_Track, 5);  return true;  // J Warnings

		case key(S):  gui->GuiShortcut(WND_Edit, TAB_Sun);       return true;  // S Sun
		case key(H):  gui->GuiShortcut(WND_Edit, TAB_Terrain);   return true;  // H Heightmap
		 case key(D): gui->GuiShortcut(WND_Edit, TAB_Terrain,1); return true;  //  D -Brushes

		case key(T):  gui->GuiShortcut(WND_Edit, TAB_Layers);    return true;  // T Layers (Terrain)
		 case key(B): gui->GuiShortcut(WND_Edit, TAB_Layers,0);  return true;  //  B -Blendmap

		case key(G):  gui->GuiShortcut(WND_Edit, TAB_Grass);     return true;  // G Grasses
		 case key(F): gui->GuiShortcut(WND_Edit, TAB_Grass,2);   return true;  //  F -Channels

		case key(V):  gui->GuiShortcut(WND_Edit, TAB_Veget);     return true;  // V Vegetation
		 case key(M): gui->GuiShortcut(WND_Edit, TAB_Veget,1);   return true;  //  M -Models

		case key(U):  gui->GuiShortcut(WND_Edit, TAB_Surface);   return true;  // U Surfaces
		case key(R):  gui->GuiShortcut(WND_Edit, TAB_Road);      return true;  // R Road
		case key(X):  gui->GuiShortcut(WND_Edit, TAB_Objects);   return true;  // X Objects

		case key(C):  gui->GuiShortcut(WND_Options, 1);	  return true;  // C Screen
		case key(A):  gui->GuiShortcut(WND_Options, 2);   return true;  // A Graphics

		case key(E):  gui->GuiShortcut(WND_Options, 3);   return true;  // E View /Settings
		case key(K):  gui->GuiShortcut(WND_Options, 4);   return true;  // K Tweak
		
		case key(I):  gui->GuiShortcut(WND_Help, 1);  return true;  // I Input/help
	}
	else
	switch (skey)
	{
		case key(TAB):	//  Camera / Edit mode
		if (!bGuiFocus && !alt)  {
			bMoveCam = !bMoveCam;  UpdVisGui();  UpdFluidBox();  UpdObjPick();
		}	break;

		//  toggle fog, veget, weather, particles
		case key(V):  bTrGrUpd = true;  break;
		case key(G):  gui->ckFog.Invert();  break;
		case key(I):  gui->ckWeather.Invert();  break;
		case key(P):  bParticles = !bParticles;  bRecreateEmitters = true;  break;

		//  terrain
		case key(D):  if (bEdit()){  SetEdMode(ED_Deform);  curBr = 0;  updBrush();  UpdEditWnds();  }	break;
		case key(S):  if (bEdit()){  SetEdMode(ED_Smooth);  curBr = 1;  updBrush();  UpdEditWnds();  }	break;
		case key(E):  if (bEdit()){  SetEdMode(ED_Height);  curBr = 2;  updBrush();  UpdEditWnds();  }	break;
		case key(F):  if (bEdit()){  SetEdMode(ED_Filter);  curBr = 3;  updBrush();  UpdEditWnds();  }
			else  //  focus on find edit  (global)
			if (ctrl && gcom->edTrkFind /*&& bGuiFocus &&
				!pSet->isMain && pSet->inMenu == WND_Edit && mWndTabsEdit->getIndexSelected() == 1*/)
			{
				if (wf == gcom->edTrkFind)  // ctrl-F  twice to toggle filtering
				{	gcom->ckTrkFilter.Invert();  return true;  }
				gui->GuiShortcut(WND_Track, 1);  // Track tab
				InputManager::getInstance().resetKeyFocusWidget();
				InputManager::getInstance().setKeyFocusWidget(gcom->edTrkFind);
				return true;
			}	break;

		//  road
		case key(R):  if (bEdit()){  SetEdMode(ED_Road);	UpdEditWnds();  }	break;
		case key(B):  if (road)  {  road->UpdPointsH();  road->Rebuild(true);  }  break;
		case key(T):  if (edMode == ED_Road && mWndRoadStats)
						mWndRoadStats->setVisible(!mWndRoadStats->getVisible());  break;
		case key(M):  if (edMode == ED_Road && road)  road->ToggleMerge();  break;

		//  start pos
		case key(Q):  if (bEdit()){  SetEdMode(ED_Start);  UpdEditWnds();  }   break;
		case key(SPACE):
			if (edMode == ED_Start && road)  road->iDir *= -1;  break;
		case key(KP_ENTER):  case key(RETURN):
			if (edMode == ED_Start)  iEnd = 1 - iEnd;  UpdStartPos();  break;
		
		//  prv cam
		case key(F7):  togPrvCam();  break;

		//  fluids
		case key(W):  if (bEdit()){  SetEdMode(ED_Fluids);  UpdEditWnds();  }   break;
		case key(F10):  SaveWaterDepth();   break;

		//  objects
		case key(C):  if (edMode == ED_Objects)  {  objSim = !objSim;  ToggleObjSim();  }  break;
		case key(X):  if (bEdit()){  SetEdMode(ED_Objects);  UpdEditWnds();  }   break;

		//  particles
		case key(A):  if (bEdit()){  SetEdMode(ED_Particles);  UpdEditWnds();  }   break;
	}

	return true;
}
