#include "pch.h"
#include "../ogre/common/Defines.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/SceneXml.h"
#include "../ogre/common/CData.h"
#include "../ogre/common/FluidsXml.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../vdrift/pathmanager.h"
#include "../ogre/common/MultiList2.h"
#include <OgreTerrain.h>
#include <MyGUI.h>
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "../sdl4ogre/sdlinputwrapper.hpp"
#include "../sdl4ogre/sdlcursormanager.hpp"
using namespace MyGUI;
using namespace Ogre;


//  Update  input, info
//---------------------------------------------------------------------------------------------------------------
//  tool wnds show/hide
void App::UpdEditWnds()
{
	if (mWndBrush)
	{	if (edMode == ED_Deform)
		{	static_cast<StaticTextPtr>(mWndBrush)->setCaption("Terrain Deform");  
			mWndBrush->setColour(MyGUI::Colour(0.5f, 0.9f, 0.3f));
			mWndBrush->setVisible(true);  }
		else if (edMode == ED_Filter)
		{	static_cast<StaticTextPtr>(mWndBrush)->setCaption("Terrain Filter");
			mWndBrush->setColour(MyGUI::Colour(0.5f, 0.75f, 1.0f));
			mWndBrush->setVisible(true);  }
		else if (edMode == ED_Smooth)
		{	static_cast<StaticTextPtr>(mWndBrush)->setCaption("Terrain Smooth");
			mWndBrush->setColour(MyGUI::Colour(0.3f, 0.8f, 0.8f));
			mWndBrush->setVisible(true);  }
		else if (edMode == ED_Height)
		{	static_cast<StaticTextPtr>(mWndBrush)->setCaption("Terrain Height");
			mWndBrush->setColour(MyGUI::Colour(0.7f, 1.0f, 0.7f));
			mWndBrush->setVisible(true);  }
		else
			mWndBrush->setVisible(false);
	}
	if (mWndRoadCur) mWndRoadCur->setVisible(edMode == ED_Road);
	if (mWndCam)     mWndCam->setVisible(edMode == ED_PrvCam);
	
	if (mWndStart)   mWndStart->setVisible(edMode == ED_Start);

	if (mWndFluids)  mWndFluids->setVisible(edMode == ED_Fluids);
	UpdFluidBox();

	if (mWndObjects) mWndObjects->setVisible(edMode == ED_Objects);

	if (mWndRivers)  mWndRivers->setVisible(edMode == ED_Rivers);

	UpdStartPos();  // StBox visible
	UpdVisGui();  //br prv..

	UpdMtrWaterDepth();
}

void App::SetEdMode(ED_MODE newMode)
{
	static bool first = true;
	if (newMode == ED_Objects && first)
	{
		gui->SetObjNewType(gui->iObjTNew);
		first = false;
	}

	//if (pSet->autoWaterDepth)  //..?
	if (edMode == ED_Fluids && newMode != ED_Fluids)
		SaveWaterDepth();  // update, on exit from Fluids editing

	edMode = newMode;
}


void App::UpdVisGui()
{
	bool notMain = bGuiFocus && !pSet->isMain;
	if (mWndMain)	mWndMain->setVisible(bGuiFocus && pSet->isMain);
	if (mWndEdit)	mWndEdit->setVisible(notMain && pSet->inMenu == WND_Edit);
	if (mWndHelp)	mWndHelp->setVisible(notMain && pSet->inMenu == WND_Help);
	if (mWndOpts)	mWndOpts->setVisible(notMain && pSet->inMenu == WND_Options);

	if (gui->bnQuit)  gui->bnQuit->setVisible(bGuiFocus);

	bool vis = bGuiFocus || !bMoveCam;
	mCursorManager->cursorVisibilityChange(vis);
	mInputWrapper->setMouseRelative(!vis);
	mInputWrapper->setGrabPointer(!vis);

	if (road)  road->SetTerHitVis(bEdit());
	if (!bGuiFocus && gui->mToolTip)  gui->mToolTip->setVisible(false);

	if (ovBrushPrv)
	if (edMode >= ED_Road || bMoveCam)
		ovBrushPrv->hide();  else  ovBrushPrv->show();

	for (int i=0; i < WND_ALL; ++i)
		mWndMainPanels[i]->setVisible(pSet->inMenu == i);
		
	if (gui->txWarn)  gui->txWarn->setVisible(false);

	//  1st center mouse
	static bool first = true;
	if (bGuiFocus && first)
	{	first = false;
		gui->GuiCenterMouse();
	}
}

void CGui::toggleGui(bool toggle)
{
	if (app->edMode == ED_PrvCam)  return;
	if (toggle)
		app->bGuiFocus = !app->bGuiFocus;
	app->UpdVisGui();
}

void CGui::Status(String s, float r,float g,float b)
{
	app->ovStat->setColour(ColourValue(r,g,b));
	app->ovStat->setCaption(s);
	app->ovSt->setMaterialName("hud/Times");
	app->ovSt->show();
	app->fStFade = 1.5f;
}


///  Preview Camera mode  - - - - - - - - - - - - - - - - - - - - - - - -
void App::togPrvCam()
{
	static bool oldV = false, oldI = false;
	if (edMode == ED_PrvCam)  // leave
	{
		SetEdMode(edModeOld);
		mViewport->setVisibilityMask(RV_MaskAll);
		rt[RTs].ndMini->setVisible(false);
		ndCar->setVisible(true);

		UpdateWaterRTT(mCamera);
		UpdFog();  // restore fog, veget
		if (oldV)  {  bTrGrUpd = true;  oldV = false;  }
		pSet->bWeather = oldI;
		UpdTerErr();

		sc->camPos = mCamera->getPosition();
		sc->camDir = mCamera->getDirection();
		mCamera->setPosition( mCamPosOld);
		mCamera->setDirection(mCamDirOld);
	}else  // enter
	{
		edModeOld = edMode;
		SetEdMode(ED_PrvCam);
		bMoveCam = true;  UpdVisGui();
		mViewport->setVisibilityMask(RV_MaskPrvCam);
		rt[RTs].ndMini->setVisible(true);
		ndCar->setVisible(false);

		UpdateWaterRTT(rt[3].rndCam);
		UpdFog(true);  // on fog, veget, weather
		if (!pSet->bTrees)  {  bTrGrUpd = true;  oldV = true;  }
		oldI = pSet->bWeather;  pSet->bWeather = false;
		mTerrainGlobals->setMaxPixelError(0.5f);  //hq ter

		mCamPosOld = mCamera->getPosition();
		mCamDirOld = mCamera->getDirection();
		mCamera->setPosition( sc->camPos);
		mCamera->setDirection(sc->camDir);
	}
	UpdEditWnds();
}


//  key util
void CGui::trkListNext(int rel)  //Gui..
{
	bool b = app->bGuiFocus && (app->mWndTabsEdit->getIndexSelected() == 1)
		&& !pSet->isMain && pSet->inMenu == WND_Edit;
	if (!b)  return;
	
	size_t cnt = trkList->getItemCount();
	if (cnt == 0)  return;
	int i = std::max(0, std::min((int)cnt-1, (int)trkList->getIndexSelected()+rel ));
	trkList->setIndexSelected(i);
	trkList->beginToItemAt(std::max(0, i-11));  // center
	listTrackChng(trkList,i);
}

void CGui::MainMenuBtn(MyGUI::WidgetPtr wp)
{
	for (int i=0; i < WND_ALL; ++i)
		if (wp == app->mWndMainBtns[i])
		{
			pSet->isMain = false;
			pSet->inMenu = i;
			toggleGui(false);
			return;
		}
}

void CGui::MenuTabChg(MyGUI::TabPtr tab, size_t id)
{
	if (id != 0)  return;
	tab->setIndexSelected(1);  // dont switch to 0
	pSet->isMain = true;
	toggleGui(false);  // back to main
}

void CGui::GuiShortcut(WND_Types wnd, int tab, int subtab)
{
	if (subtab == -1 && (!app->bGuiFocus || pSet->inMenu != wnd))  subtab = -2;  // cancel subtab cycling

	if (!app->bGuiFocus)
	if (app->edMode != ED_PrvCam)  {
		app->bGuiFocus = !app->bGuiFocus;  app->UpdVisGui();  }

	//isFocGui = true;
	pSet->isMain = false;  pSet->inMenu = wnd;
	
	MyGUI::TabPtr mWndTabs = 0;
	std::vector<MyGUI::TabControl*>* subt = 0;
	
	switch (wnd)
	{	case WND_Edit:		mWndTabs = app->mWndTabsEdit;  subt = &vSubTabsEdit;  break;
		case WND_Help:		mWndTabs = app->mWndTabsHelp;  subt = &vSubTabsHelp;  break;
		case WND_Options:	mWndTabs = app->mWndTabsOpts;  subt = &vSubTabsOpts;  break;
	}
	toggleGui(false);


	size_t t = mWndTabs->getIndexSelected();
	mWndTabs->setIndexSelected(tab);

	if (!subt)  return;
	MyGUI::TabControl* tc = (*subt)[tab];  if (!tc)  return;
	int  cnt = tc->getItemCount();

	if (t == tab && subtab == -1)  // cycle subpages if same tab
	{	if (app->shift)
			tc->setIndexSelected( (tc->getIndexSelected()-1+cnt) % cnt );
		else
			tc->setIndexSelected( (tc->getIndexSelected()+1) % cnt );
	}
	if (subtab > -1)
		tc->setIndexSelected( std::min(cnt-1, subtab) );
}

void CGui::NumTabNext(int rel)
{
	if (!app->bGuiFocus || pSet->isMain || pSet->inMenu != WND_Edit)  return;

	MyGUI::TabPtr tab = 0;

	#define tabNum(event)  {  \
		int cnt = tab->getItemCount();  \
		tab->setIndexSelected( (tab->getIndexSelected()+rel+cnt) % cnt );  \
		event(tab, tab->getIndexSelected());  }

	int id = app->mWndTabsEdit->getIndexSelected();
	switch (id)
	{
		case 4:  // Layers
		{	tab = tabsTerLayers;  tabNum(tabTerLayer);
		}	break;
		case 5:  // Vegetation
		{
			int sid = vSubTabsEdit[id]->getIndexSelected();
			switch (sid)
			{
			case 1:  tab = tabsGrLayers;  tabNum(tabGrLayers);  break;
			case 2:  tab = tabsPgLayers;  tabNum(tabPgLayers);  break;
			}
		}	break;
	}
	//Tab(tabsTerLayers, "TabTerLay", tabTerLayer);
	//Tab(tabsGrLayers, "LGrLayTab", tabGrLayers);
	//Tab(tabsPgLayers, "LTrNumTab", tabPgLayers);
}


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
			case key(ESCAPE):  // exit
			case key(F7):  togPrvCam();  break;

			case key(RETURN):  // save screen
			{	int u = pSet->allow_save ? pSet->gui.track_user : 1;
				rt[RTs-1].rndTex->writeContentsToFile(gui->pathTrk[u] + pSet->gui.track + "/preview/view.jpg");
				gui->listTrackChng(gui->trkList,0);  // upd gui img
				gui->Status("Preview saved", 1,1,0);
			}	break;

			case key(F12):  // screenshot
				mWindow->writeContentsToTimestampedFile(PATHMANAGER::Screenshots() + "/", ".jpg");
				return true;
		}
		return true;  //!
	}

	//  main menu keys
	Widget* wf = MyGUI::InputManager::getInstance().getKeyFocusWidget();
	bool edFoc = wf && wf->getTypeName() == "EditBox";

	if (pSet->isMain && bGuiFocus)
	{
		switch (skey)
		{
		case key(UP):  case key(KP_8):
			pSet->inMenu = (pSet->inMenu-1+WND_ALL)%WND_ALL;
			gui->toggleGui(false);  return true;

		case key(DOWN):  case key(KP_2):
			pSet->inMenu = (pSet->inMenu+1)%WND_ALL;
			gui->toggleGui(false);  return true;

		case key(RETURN):
			pSet->isMain = false;
			gui->toggleGui(false);  return true;
		}
	}
	if (!pSet->isMain && bGuiFocus)
	{
		switch (skey)
		{
		case key(BACKSPACE):
			if (pSet->isMain)  break;
			if (bGuiFocus)
			{	if (edFoc)  break;
				pSet->isMain = true;  gui->toggleGui(false);  }
			return true;
		}
	}

	//  change gui tabs
	TabPtr tab = 0;  MyGUI::TabControl* sub = 0;  int iTab1 = 1;
	if (bGuiFocus && !pSet->isMain)
	switch (pSet->inMenu)
	{
		case WND_Edit:    tab = mWndTabsEdit;  sub = gui->vSubTabsEdit[tab->getIndexSelected()];  break;
		case WND_Help:    tab = sub = gui->vSubTabsHelp[1];  iTab1 = 0;  break;
		case WND_Options: tab = mWndTabsOpts;  sub = gui->vSubTabsOpts[tab->getIndexSelected()];  break;
	}

	//  global keys
	//------------------------------------------------------------------------------------------------------------------------------
	switch (skey)
	{
		case key(ESCAPE): //  quit
			if (pSet->escquit)
			{
				mShutDown = true;
			}	return true;

		case key(F1):
		case key(GRAVE):
			if (ctrl)  // context help (show for cur mode)
			{
				if (bMoveCam)		 gui->GuiShortcut(WND_Help, 1, 0);
				else switch (edMode)
				{	case ED_Smooth: case ED_Height: case ED_Filter:
					case ED_Deform:  gui->GuiShortcut(WND_Help, 1, 1);  break;
					case ED_Road:    gui->GuiShortcut(WND_Help, 1, 2);  break;
					case ED_Start:   gui->GuiShortcut(WND_Help, 1, 4);  break;
					case ED_Fluids:  gui->GuiShortcut(WND_Help, 1, 5);  break;
					case ED_Objects: gui->GuiShortcut(WND_Help, 1, 6);  break;
					default:		 gui->GuiShortcut(WND_Help, 1, 0);  break;
			}	}
			else	//  Gui mode, Options
				gui->toggleGui(true);
			return true;

		case key(F12): //  screenshot
			mWindow->writeContentsToTimestampedFile(PATHMANAGER::Screenshots() + "/", ".jpg");
			return true;

		//  save, reload, update
		case key(F4):  SaveTrack();	return true;
		case key(F5):  LoadTrack();	return true;
		case key(F8):  UpdateTrack();  return true;

		case key(F9):  // blendmap
			if (alt)
			{	/*BGui::*/WP wp = gui->chAutoBlendmap;  ChkEv(autoBlendmap);  }
			else	bTerUpdBlend = true;  return true;

		//  prev num tab (layers,grasses,models)
		case key(1):
   			if (alt)  {  gui->NumTabNext(-1);  return true;  }
			break;
		//  next num tab
		case key(2):
   			if (alt)  {  gui->NumTabNext(1);  return true;  }
			break;

		case key(F2):  // +-rt num
   			if (alt)
   			{	pSet->num_mini = (pSet->num_mini - 1 + RTs+2) % (RTs+2);  UpdMiniVis();  }
   			else
   			if (bGuiFocus && tab && !pSet->isMain)
   				if (shift)  // prev gui subtab
   				{
   					if (sub)  {  int num = sub->getItemCount();
   						sub->setIndexSelected( (sub->getIndexSelected() - 1 + num) % num );  }
	   			}
   				else	// prev gui tab
   				{	int num = tab->getItemCount()-1, i = tab->getIndexSelected();
					if (i==iTab1)  i = num;  else  --i;
					tab->setIndexSelected(i);  if (iTab1==1)  gui->MenuTabChg(tab,i);
	   			}
   			break;

		case key(F3):  // tabs,sub
   			if (alt)
   			{	pSet->num_mini = (pSet->num_mini + 1) % (RTs+2);  UpdMiniVis();  }
   			else
   			if (bGuiFocus && tab && !pSet->isMain)
   				if (shift)  // next gui subtab
   				{
   					if (sub)  {  int num = sub->getItemCount();
   						sub->setIndexSelected( (sub->getIndexSelected() + 1) % num );  }
	   			}
	   			else	// next gui tab
	   			{	int num = tab->getItemCount()-1, i = tab->getIndexSelected();
					if (i==num)  i = iTab1;  else  ++i;
					tab->setIndexSelected(i);  if (iTab1==1)  gui->MenuTabChg(tab,i);
				}
   			break;
   			
		case key(RETURN):  // load track
			if (bGuiFocus)
			if (mWndTabsEdit->getIndexSelected() == 1 && !pSet->isMain && pSet->inMenu == WND_Edit)
				gui->btnNewGame(0);
   			break;

		//  Wire Frame  F11
		case key(F11):
		{	mbWireFrame = !mbWireFrame;
			mCamera->setPolygonMode(mbWireFrame ? PM_WIREFRAME : PM_SOLID);
			if (ndSky)	ndSky->setVisible(!mbWireFrame);  // hide sky
			return true;
		}	break;

		//  Show Stats  ctrl-I
		case key(I):
   			if (ctrl)  {  gui->chkInputBar(gui->chInputBar);  return true;  }
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
		MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::Enum(mInputWrapper->sdl2OISKeyCode(arg.keysym.sym)), 0);
		return true;
	}


	///  Road keys  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
	if (edMode == ED_Road && road && bEdit())
	{
		if (iSnap > 0)
		switch (skey)
		{
			case key(1):  road->AddYaw(-1,angSnap,alt);  break;
			case key(2):  road->AddYaw( 1,angSnap,alt);  break;
			case key(3):  road->AddRoll(-1,angSnap,alt);  break;
			case key(4):  road->AddRoll( 1,angSnap,alt);  break;
		}
		switch (skey)
		{
			//  choose 1
			case key(SPACE):
				if (ctrl)	road->CopyNewPoint();
				else		road->ChoosePoint();  break;
				
			//  multi sel
			case key(BACKSPACE):
				if (alt)		road->SelAll();
				else if (ctrl)	road->SelClear();
				else			road->SelAddPoint();  break;
				
			//  ter on  first,last
			case key(HOME):  case key(KP_7):
				if (alt)	road->MirrorSel(shift);  else
				if (ctrl)	road->FirstPoint();
				else		road->ToggleOnTerrain();  break;
				
			//  cols
			case key(END):  case key(KP_1):
				if (ctrl)	road->LastPoint();
				else		road->ToggleColums();  break;

			//  prev,next
			case key(PAGEUP):  case key(KP_9):
				road->PrevPoint();  break;
			case key(PAGEDOWN):	case key(KP_3):
				road->NextPoint();  break;

			//  del
			case key(DELETE):  case key(KP_PERIOD):
			case key(KP_5):
				if (ctrl)	road->DelSel();
				else		road->Delete();  break;

			//  ins
			case key(INSERT):  case key(KP_0):
				if (ctrl && !shift && !alt)	{	if (road->CopySel())  gui->Status("Copy",0.6,0.8,1.0);  }
				else if (!ctrl && shift && !alt)	road->Paste();
				else if ( ctrl && shift && !alt)	road->Paste(true);
				else
				{	road->newP.pos.x = road->posHit.x;
					road->newP.pos.z = road->posHit.z;
					if (!sc->ter)
						road->newP.pos.y = road->posHit.y;
					//road->newP.aType = AT_Both;
					road->Insert(shift ? INS_Begin : ctrl ? INS_End : alt ? INS_CurPre : INS_Cur);
				}	break;					  

			case key(0):
				if (ctrl)  {   road->Set1stChk();  break;  }
			case key(EQUALS):  road->ChgMtrId(1);  break;
			case key(9):
			case key(MINUS):   road->ChgMtrId(-1);  break;

			case key(5):  road->ChgAngType(-1);  break;
			case key(6):  if (shift)  road->AngZero();  else
						road->ChgAngType(1);  break;

			case key(7):  iSnap = (iSnap-1+ciAngSnapsNum)%ciAngSnapsNum;  angSnap = crAngSnaps[iSnap];  break;
			case key(8):  iSnap = (iSnap+1)%ciAngSnapsNum;                angSnap = crAngSnaps[iSnap];  break;
			
			case key(U):  AlignTerToRoad();  break;
			
			//  looped  todo: finish set..
			case key(N):  road->isLooped = !road->isLooped;
				road->recalcTangents();  road->RebuildRoad(true);  break;
		}
	}

	//  ter brush shape
	if (edMode < ED_Road && !alt)
	switch (skey)
	{
		case key(K):	if (ctrl)  {  mBrShape[curBr] = (EBrShape)((mBrShape[curBr]-1 + BRS_ALL) % BRS_ALL);  updBrush();  }  break;
		case key(L):	if (ctrl)  {  mBrShape[curBr] = (EBrShape)((mBrShape[curBr]+1) % BRS_ALL);            updBrush();  }  break;
		case key(N): case key(COMMA):	mBrOct[curBr] = std::max(1, mBrOct[curBr]-1);  updBrush();  break;
		case key(M): case key(PERIOD):	mBrOct[curBr] = std::min(7, mBrOct[curBr]+1);  updBrush();  break;
	}

	//  ter brush presets  ----
	if (edMode < ED_Road && alt && skey >= key(1) && skey <= key(0) && !bMoveCam)
	{
		// TODO
		int id = skey - key(1);
		if (shift)  id += 10;
		SetBrushPreset(id);
	}

	
	//  Fluids  ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
	if (edMode == ED_Fluids)
	{	int fls = sc->fluids.size();
		switch (skey)
		{
			//  ins
			case key(INSERT):  case key(KP_0):
			if (road && road->bHitTer)
			{
				FluidBox fb;	fb.name = "water blue";
				fb.pos = road->posHit;	fb.rot = Ogre::Vector3(0.f, 0.f, 0.f);
				fb.size = Ogre::Vector3(50.f, 20.f, 50.f);	fb.tile = Vector2(0.01f, 0.01f);
				sc->fluids.push_back(fb);
				sc->UpdateFluidsId();
				iFlCur = sc->fluids.size()-1;
				bRecreateFluids = true;
			}	break;
		}
		if (fls > 0)
		switch (skey)
		{
			//  first, last
			case key(HOME):  case key(KP_7):
				iFlCur = 0;  UpdFluidBox();  break;
			case key(END):  case key(KP_1):
				if (fls > 0)  iFlCur = fls-1;  UpdFluidBox();  break;

			//  prev,next
			case key(PAGEUP):  case key(KP_9):
				if (fls > 0) {  iFlCur = (iFlCur-1+fls)%fls;  }  UpdFluidBox();  break;
			case key(PAGEDOWN):	case key(KP_3):
				if (fls > 0) {  iFlCur = (iFlCur+1)%fls;	  }  UpdFluidBox();  break;

			//  del
			case key(DELETE):  case key(KP_PERIOD):
			case key(KP_5):
				if (fls == 1)	sc->fluids.clear();
				else			sc->fluids.erase(sc->fluids.begin() + iFlCur);
				iFlCur = std::max(0, std::min(iFlCur, (int)sc->fluids.size()-1));
				bRecreateFluids = true;
				break;

			//  prev,next type
			case key(9):  case key(MINUS):
			{	FluidBox& fb = sc->fluids[iFlCur];
				fb.id = (fb.id-1 + data->fluids->fls.size()) % data->fluids->fls.size();
				fb.name = data->fluids->fls[fb.id].name;
				bRecreateFluids = true;  }	break;
			case key(0):  case key(EQUALS):
			{	FluidBox& fb = sc->fluids[iFlCur];
				fb.id = (fb.id+1) % data->fluids->fls.size();
				fb.name = data->fluids->fls[fb.id].name;
				bRecreateFluids = true;  }	break;
		}
	}

	//  Objects  | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
	if (edMode == ED_Objects)
	{	int objs = sc->objects.size(), objAll = gui->vObjNames.size();
		switch (skey)
		{
			case key(SPACE):
				gui->iObjCur = -1;  PickObject();  UpdObjPick();  break;
				
			//  prev,next type
			case key(9):  case key(MINUS):   gui->SetObjNewType((gui->iObjTNew-1 + objAll) % objAll);  break;
			case key(0):  case key(EQUALS):  gui->SetObjNewType((gui->iObjTNew+1) % objAll);  break;
				
			//  ins
			case key(INSERT):	case key(KP_0):
			if (road && road->bHitTer)
			{
				gui->AddNewObj();
				//iObjCur = sc->objects.size()-1;  // auto select inserted-
				UpdObjPick();
			}	break;
			
			//  sel
			case key(BACKSPACE):
				if (ctrl)  gui->vObjSel.clear();  // unsel all
				else
				if (gui->iObjCur > -1)
					if (gui->vObjSel.find(gui->iObjCur) == gui->vObjSel.end())
						gui->vObjSel.insert(gui->iObjCur);  // add to sel
					else
						gui->vObjSel.erase(gui->iObjCur);  // unselect
				break;
		}
		::Object* o = gui->iObjCur == -1 ? &gui->objNew :
					((gui->iObjCur >= 0 && objs > 0 && gui->iObjCur < objs) ? &sc->objects[gui->iObjCur] : 0);
		switch (skey)
		{
			//  first, last
			case key(HOME):  case key(KP_7):
				gui->iObjCur = 0;  UpdObjPick();  break;
			case key(END):  case key(KP_1):
				if (objs > 0)  gui->iObjCur = objs-1;  UpdObjPick();  break;

			//  prev,next
			case key(PAGEUP):  case key(KP_9):
				if (objs > 0) {  gui->iObjCur = (gui->iObjCur-1+objs)%objs;  }  UpdObjPick();  break;
			case key(PAGEDOWN):	case key(KP_3):
				if (objs > 0) {  gui->iObjCur = (gui->iObjCur+1)%objs;	  }  UpdObjPick();  break;

			//  del
			case key(DELETE):  case key(KP_PERIOD):
			case key(KP_5):
				if (gui->iObjCur >= 0 && objs > 0)
				{	::Object& o = sc->objects[gui->iObjCur];
					mSceneMgr->destroyEntity(o.ent);
					mSceneMgr->destroySceneNode(o.nd);
					
					if (objs == 1)	sc->objects.clear();
					else			sc->objects.erase(sc->objects.begin() + gui->iObjCur);
					gui->iObjCur = std::min(gui->iObjCur, (int)sc->objects.size()-1);
					UpdObjPick();
				}	break;

			//  move,rot,scale
			case key(1):
				if (!shift)  gui->objEd = EO_Move;
				else if (o)
				{
					if (gui->iObjCur == -1)  // reset h
					{
						o->pos[2] = 0.f;  o->SetFromBlt();  UpdObjPick();
					}
					else if (road)  // move to ter
					{
						const Ogre::Vector3& v = road->posHit;
						o->pos[0] = v.x;  o->pos[1] =-v.z;  o->pos[2] = v.y + gui->objNew.pos[2];
						o->SetFromBlt();  UpdObjPick();
					}
				}	break;

			case key(2):
				if (!shift)  gui->objEd = EO_Rotate;
				else if (o)  // reset rot
				{
					o->rot = QUATERNION<float>(0,1,0,0);
					o->SetFromBlt();  UpdObjPick();
				}	break;

			case key(3):
				if (!shift)  gui->objEd = EO_Scale;
				else if (o)  // reset scale
				{
					o->scale = Ogre::Vector3::UNIT_SCALE * (ctrl ? 0.5f : 1.f);
					o->nd->setScale(o->scale);  UpdObjPick();
				}	break;
		}
	}

	//  Rivers  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (edMode == ED_Rivers)
	{
		// todo
	}

	///  Common Keys  ************************************************************************************************************
	if (alt)
	switch (skey)
	{
		case key(Q):  gui->GuiShortcut(WND_Edit, 1);  return true;  // Q Track
		case key(S):  gui->GuiShortcut(WND_Edit, 2);  return true;  // S Sun

		case key(H):  gui->GuiShortcut(WND_Edit, 3);  return true;  // H Heightmap
		 case key(D): gui->GuiShortcut(WND_Edit, 3,0);  return true;  //  D -Brushes

		case key(T):  gui->GuiShortcut(WND_Edit, 4);  return true;  // T Layers (Terrain)
		 case key(B): gui->GuiShortcut(WND_Edit, 4,0);  return true;  //  B -Blendmap
		 case key(P): gui->GuiShortcut(WND_Edit, 4,1);  return true;  //  P -Particles
		 case key(U): gui->GuiShortcut(WND_Edit, 4,2);  return true;  //  U -Surfaces

		case key(V):  gui->GuiShortcut(WND_Edit, 5);  return true;  // V Vegetation
		 case key(M): gui->GuiShortcut(WND_Edit, 5,2);  return true;  //  M -Models

		case key(R):  gui->GuiShortcut(WND_Edit, 6);  return true;  // R Road
		case key(X):  gui->GuiShortcut(WND_Edit, 7);  return true;  // X Objects
		case key(O):  gui->GuiShortcut(WND_Edit, 8);  return true;  // O Tools

		case key(C):  gui->GuiShortcut(WND_Options, 1);  return true;  // C Screen
		case key(G):  gui->GuiShortcut(WND_Options, 2);  return true;  // G Graphics
		 case key(N): gui->GuiShortcut(WND_Options, 2,3);  return true;  // N -Vegetation
		case key(E):  gui->GuiShortcut(WND_Options, 3);  return true;  // E Settings
		case key(K):  gui->GuiShortcut(WND_Options, 4);  return true;  // K Tweak

		case key(I):  gui->GuiShortcut(WND_Help, 1);  return true;  // I Input/help
		case key(J):  gui->GuiShortcut(WND_Edit, 9);  return true;  // J Warnings
	}
	else
	switch (skey)
	{
		case key(TAB):	//  Camera / Edit mode
		if (!bGuiFocus && !alt)  {
			bMoveCam = !bMoveCam;  UpdVisGui();  UpdFluidBox();  UpdObjPick();
		}	break;

		//  fog
		case key(G):  {
			pSet->bFog = !pSet->bFog;  gui->chkFog->setStateSelected(pSet->bFog);  UpdFog();  }  break;
		//  trees
		case key(V):  bTrGrUpd = true;  break;
		//  weather
		case key(I):  {
			pSet->bWeather = !pSet->bWeather;  gui->chkWeather->setStateSelected(pSet->bWeather);  }  break;

		//  terrain
		case key(D):  if (bEdit()){  SetEdMode(ED_Deform);  curBr = 0;  updBrush();  UpdEditWnds();  }	break;
		case key(S):  if (bEdit()){  SetEdMode(ED_Smooth);  curBr = 1;  updBrush();  UpdEditWnds();  }	break;
		case key(E):  if (bEdit()){  SetEdMode(ED_Height);  curBr = 2;  updBrush();  UpdEditWnds();  }	break;
		case key(F):  if (bEdit()){  SetEdMode(ED_Filter);  curBr = 3;  updBrush();  UpdEditWnds();  }
			else  //  focus on find edit  (global)
			if (ctrl && gui->edFind /*&& bGuiFocus &&
				!pSet->isMain && pSet->inMenu == WND_Edit && mWndTabsEdit->getIndexSelected() == 1*/)
			{
				gui->GuiShortcut(WND_Edit, 1);  // Track tab
				MyGUI::InputManager::getInstance().resetKeyFocusWidget();
				MyGUI::InputManager::getInstance().setKeyFocusWidget(gui->edFind);
				return true;
			}	break;

		//  road
		case key(R):  if (bEdit()){  SetEdMode(ED_Road);	UpdEditWnds();  }	break;
		case key(B):  if (road)  {  road->UpdPointsH();  road->RebuildRoad(true);  }  break;
		case key(T):  if (mWndRoadStats)  mWndRoadStats->setVisible(!mWndRoadStats->getVisible());  break;
		case key(M):  if (edMode == ED_Road && road)  road->ToggleMerge();  break;

		//  start pos
		case key(Q):  if (bEdit()){  SetEdMode(ED_Start);  UpdEditWnds();  }   break;
		case key(SPACE):
			if (edMode == ED_Start && road)  road->iDir *= -1;  break;
		//  prv cam
		case key(F7):  togPrvCam();  break;

		//  fluids
		case key(W):  if (bEdit()){  SetEdMode(ED_Fluids);  UpdEditWnds();  }   break;
		case key(F10):  SaveWaterDepth();   break;

		//  objects
		case key(C):  if (edMode == ED_Objects)  {  gui->objSim = !gui->objSim;  ToggleObjSim();  }  break;
		case key(X):  if (bEdit()){  SetEdMode(ED_Objects);  UpdEditWnds();  }   break;
		
		//  rivers
		///case key(A):	if (bEdit()){  SetEdMode(ED_Rivers);  UpdEditWnds();  }	break;
	}

	return true;
}
