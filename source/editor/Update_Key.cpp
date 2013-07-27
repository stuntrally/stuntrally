#include "pch.h"
#include "../ogre/common/Defines.h"
#include "../ogre/common/Gui_Def.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include "../vdrift/pathmanager.h"
#include "../ogre/common/MultiList2.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/SceneXml.h"
#include <OgreTerrain.h>
#include <MyGUI.h>
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
using namespace MyGUI;
using namespace Ogre;

#include "../sdl4ogre/sdlinputwrapper.hpp"
#include "../sdl4ogre/sdlcursormanager.hpp"


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
		SetObjNewType(iObjTNew);
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

	if (bnQuit)  bnQuit->setVisible(bGuiFocus);

	bool cursorVisible = bGuiFocus || !bMoveCam;
	mCursorManager->cursorVisibilityChange(cursorVisible);
	mInputWrapper->setMouseRelative(!cursorVisible);
	mInputWrapper->setGrabPointer(!cursorVisible);

	if (road)  road->SetTerHitVis(bEdit());
	if (!bGuiFocus && mToolTip)  mToolTip->setVisible(false);

	if (ovBrushPrv)
	if (edMode >= ED_Road || bMoveCam)
		ovBrushPrv->hide();  else  ovBrushPrv->show();

	for (int i=0; i < WND_ALL; ++i)
		mWndMainPanels[i]->setVisible(pSet->inMenu == i);
		
	if (txWarn)  txWarn->setVisible(false);
}

void App::toggleGui(bool toggle)
{
	if (edMode == ED_PrvCam)  return;
	if (toggle)
		bGuiFocus = !bGuiFocus;
	UpdVisGui();
}

void App::Status(String s, float r,float g,float b)
{
	ovStat->setColour(ColourValue(r,g,b));
	ovStat->setCaption(s);
	ovSt->setMaterialName("hud/Times");
	ovSt->show();
	fStFade = 1.5f;
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
void App::trkListNext(int rel)
{
	bool b = bGuiFocus && (mWndTabsEdit->getIndexSelected() == 1)
		&& !pSet->isMain && pSet->inMenu == WND_Edit;
	if (!b)  return;
	
	size_t cnt = trkList->getItemCount();
	if (cnt == 0)  return;
	int i = std::max(0, std::min((int)cnt-1, (int)trkList->getIndexSelected()+rel ));
	trkList->setIndexSelected(i);
	trkList->beginToItemAt(std::max(0, i-11));  // center
	listTrackChng(trkList,i);
}

void App::MainMenuBtn(MyGUI::WidgetPtr wp)
{
	for (int i=0; i < WND_ALL; ++i)
		if (wp == mWndMainBtns[i])
		{
			pSet->isMain = false;
			pSet->inMenu = i;
			toggleGui(false);
			return;
		}
}

void App::MenuTabChg(MyGUI::TabPtr tab, size_t id)
{
	if (id != 0)  return;
	tab->setIndexSelected(1);  // dont switch to 0
	pSet->isMain = true;
	toggleGui(false);  // back to main
}

void App::GuiShortcut(WND_Types wnd, int tab, int subtab)
{
	if (subtab == -1 && (!bGuiFocus || pSet->inMenu != wnd))  subtab = -2;  // cancel subtab cycling

	if (!bGuiFocus)
	if (edMode != ED_PrvCam)  {
		bGuiFocus = !bGuiFocus;  UpdVisGui();  }

	//isFocGui = true;
	pSet->isMain = false;  pSet->inMenu = wnd;
	
	MyGUI::TabPtr mWndTabs = 0;
	std::vector<MyGUI::TabControl*>* subt = 0;
	
	switch (wnd)
	{	case WND_Edit:		mWndTabs = mWndTabsEdit;  subt = &vSubTabsEdit;  break;
		case WND_Help:		mWndTabs = mWndTabsHelp;  subt = &vSubTabsHelp;  break;
		case WND_Options:	mWndTabs = mWndTabsOpts;  subt = &vSubTabsOpts;  break;
	}
	toggleGui(false);


	size_t t = mWndTabs->getIndexSelected();
	mWndTabs->setIndexSelected(tab);

	if (!subt)  return;
	MyGUI::TabControl* tc = (*subt)[tab];  if (!tc)  return;
	int  cnt = tc->getItemCount();

	if (t == tab && subtab == -1)  // cycle subpages if same tab
	{	if (shift)
			tc->setIndexSelected( (tc->getIndexSelected()-1+cnt) % cnt );
		else
			tc->setIndexSelected( (tc->getIndexSelected()+1) % cnt );
	}
	if (subtab > -1)
		tc->setIndexSelected( std::min(cnt-1, subtab) );
}

void App::NumTabNext(int rel)
{
	if (!bGuiFocus || pSet->isMain || pSet->inMenu != WND_Edit)  return;

	MyGUI::TabPtr tab = 0;

	#define tabNum(event)  {  \
		int cnt = tab->getItemCount();  \
		tab->setIndexSelected( (tab->getIndexSelected()+rel+cnt) % cnt );  \
		event(tab, tab->getIndexSelected());  }

	int id = mWndTabsEdit->getIndexSelected();
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
	///  Preview camera  ---------------------
	if (edMode == ED_PrvCam)
	{
		switch (arg.keysym.sym)
		{
			case SDLK_ESCAPE:  // exit
			case SDLK_F7:  togPrvCam();  break;

			case SDLK_RETURN:  // save screen
			{	int u = pSet->allow_save ? pSet->gui.track_user : 1;
				rt[RTs-1].rndTex->writeContentsToFile(pathTrk[u] + pSet->gui.track + "/preview/view.jpg");
				listTrackChng(trkList,0);  // upd gui img
				Status("Preview saved", 1,1,0);
			}	break;

			case SDLK_F12:  // screenshot
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
		switch (arg.keysym.sym)
		{
		case SDLK_UP:  case SDLK_KP_8:
			pSet->inMenu = (pSet->inMenu-1+WND_ALL)%WND_ALL;
			toggleGui(false);  return true;

		case SDLK_DOWN:  case SDLK_KP_2:
			pSet->inMenu = (pSet->inMenu+1)%WND_ALL;
			toggleGui(false);  return true;

		case SDLK_RETURN:
			pSet->isMain = false;
			toggleGui(false);  return true;
		}
	}
	if (!pSet->isMain && bGuiFocus)
	{
		switch (arg.keysym.sym)
		{
		case SDLK_BACKSPACE:
			if (pSet->isMain)  break;
			if (bGuiFocus)
			{	if (edFoc)  break;
				pSet->isMain = true;  toggleGui(false);  }
			return true;
		}
	}

	//  change gui tabs
	TabPtr tab = 0;  MyGUI::TabControl* sub = 0;  int iTab1 = 1;
	if (bGuiFocus && !pSet->isMain)
	switch (pSet->inMenu)
	{
		case WND_Edit:  tab = mWndTabsEdit;  sub = vSubTabsEdit[tab->getIndexSelected()];  break;
		case WND_Help:   tab = sub = vSubTabsHelp[1];  iTab1 = 0;  break;
		case WND_Options:  tab = mWndTabsOpts;  sub = vSubTabsOpts[tab->getIndexSelected()];  break;
	}

	//  global keys
	//------------------------------------------------------------------------------------------------------------------------------
	switch (arg.keysym.sym)
	{
		case SDLK_ESCAPE: //  quit
			if (pSet->escquit)
			{
				mShutDown = true;
			}	return true;

		case SDLK_F1:
		case SDLK_CARET:
			if (ctrl)  // context help (show for cur mode)
			{
				if (bMoveCam)		 GuiShortcut(WND_Help, 1, 0);
				else switch (edMode)
				{	case ED_Smooth: case ED_Height: case ED_Filter:
					case ED_Deform:  GuiShortcut(WND_Help, 1, 1);  break;
					case ED_Road:    GuiShortcut(WND_Help, 1, 2);  break;
					case ED_Start:   GuiShortcut(WND_Help, 1, 4);  break;
					case ED_Fluids:  GuiShortcut(WND_Help, 1, 5);  break;
					case ED_Objects: GuiShortcut(WND_Help, 1, 6);  break;
					default:		 GuiShortcut(WND_Help, 1, 0);  break;
			}	}
			else	//  Gui mode, Options
				toggleGui(true);
			return true;

		case SDLK_F12: //  screenshot
			mWindow->writeContentsToTimestampedFile(PATHMANAGER::Screenshots() + "/", ".jpg");
			return true;

		//  save, reload, update
		case SDLK_F4:  SaveTrack();	return true;
		case SDLK_F5:  LoadTrack();	return true;
		case SDLK_F8:  UpdateTrack();  return true;

		case SDLK_F9:  // blendmap
			if (alt)
			{	WP wp = chAutoBlendmap;  ChkEv(autoBlendmap);  }
			else	bTerUpdBlend = true;  return true;

		//  prev num tab (layers,grasses,models)
		case SDLK_1:
   			if (alt)  {  NumTabNext(-1);  return true;  }
			break;
		//  next num tab
		case SDLK_2:
   			if (alt)  {  NumTabNext(1);  return true;  }
			break;

		case SDLK_F2:  // +-rt num
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
					tab->setIndexSelected(i);  if (iTab1==1)  MenuTabChg(tab,i);
	   			}
   			break;

		case SDLK_F3:  // tabs,sub
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
					tab->setIndexSelected(i);  if (iTab1==1)  MenuTabChg(tab,i);
				}
   			break;
   			
		case SDLK_RETURN:  // load track
			if (bGuiFocus)
			if (mWndTabsEdit->getIndexSelected() == 1 && !pSet->isMain && pSet->inMenu == WND_Edit)
				btnNewGame(0);
   			break;

		//  Wire Frame  F11
		case SDLK_F11:
		{	mbWireFrame = !mbWireFrame;
			mCamera->setPolygonMode(mbWireFrame ? PM_WIREFRAME : PM_SOLID);
			if (ndSky)	ndSky->setVisible(!mbWireFrame);  // hide sky
			return true;
		}	break;

		//  Show Stats  I
		case SDLK_i:
   			if (ctrl)  {  chkInputBar(chInputBar);  return true;  }
			break;
			
		case SDLK_z:
			if (alt)  {  toggleTopView();  return true;  }
			break;

		//  load next track  F6
		case SDLK_F6:
			if (pSet->check_load)
			{	iLoadNext = shift ? -1 : 1;  return true;  }
			break;
	}

	//  GUI  keys in edits  ---------------------
	if (bGuiFocus && mGUI && !alt && !ctrl)
	{
		MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::Enum(mInputWrapper->sdl2OISKeyCode(arg.keysym.sym)), 0);
		return true;
	}


	///  Road keys  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
	if (edMode == ED_Road && road && bEdit())
	{
		if (iSnap > 0)
		switch (arg.keysym.sym)
		{
			case SDLK_1:	road->AddYaw(-1,angSnap,alt);	break;
			case SDLK_2:	road->AddYaw( 1,angSnap,alt);	break;
			case SDLK_3:	road->AddRoll(-1,angSnap,alt);	break;
			case SDLK_4:	road->AddRoll( 1,angSnap,alt);	break;
		}
		switch (arg.keysym.sym)
		{
			//  choose 1
			case SDLK_SPACE:
				if (ctrl)	road->CopyNewPoint();
				else		road->ChoosePoint();  break;
				
			//  multi sel
			case SDLK_BACKSPACE:
				if (alt)		road->SelAll();
				else if (ctrl)	road->SelClear();
				else			road->SelAddPoint();  break;
				
			//  ter on  first,last
			case SDLK_HOME:  case SDLK_KP_7:
				if (ctrl)	road->FirstPoint();
				else		road->ToggleOnTerrain();  break;
				
			//  cols
			case SDLK_END:  case SDLK_KP_1:
				if (ctrl)	road->LastPoint();
				else		road->ToggleColums();  break;

			//  prev,next
			case SDLK_PAGEUP:	case SDLK_KP_9:
				road->PrevPoint();  break;
			case SDLK_PAGEDOWN:	case SDLK_KP_3:
				road->NextPoint();  break;

			//  del
			case SDLK_DELETE:	case SDLK_KP_PERIOD:
			case SDLK_KP_5:
				if (ctrl)	road->DelSel();
				else		road->Delete();  break;

			//  ins
			case SDLK_INSERT:	case SDLK_KP_0:
				if (ctrl && !shift && !alt)	{	if (road->CopySel())  Status("Copy",0.6,0.8,1.0);  }
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

			case SDLK_0:  if (ctrl)  {  road->Set1stChk();  break;  }
			case SDLK_EQUALS:  road->ChgMtrId(1);		break;
			case SDLK_9:
			case SDLK_MINUS:   road->ChgMtrId(-1);	break;

			case SDLK_5:	road->ChgAngType(-1);	break;
			case SDLK_6:	if (shift)  road->AngZero();  else
						road->ChgAngType(1);	break;

			case SDLK_7:  iSnap = (iSnap-1+ciAngSnapsNum)%ciAngSnapsNum;  angSnap = crAngSnaps[iSnap];  break;
			case SDLK_8:  iSnap = (iSnap+1)%ciAngSnapsNum;                angSnap = crAngSnaps[iSnap];  break;
			
			case SDLK_u:  AlignTerToRoad();  break;
			
			//  looped  todo: finish set..
			case SDLK_n:  road->isLooped = !road->isLooped;
				road->recalcTangents();  road->RebuildRoad(true);  break;
		}
	}

	//  ter brush shape
	if (edMode < ED_Road && !alt)
	switch (arg.keysym.sym)
	{
		case SDLK_k:	if (ctrl)  {  mBrShape[curBr] = (EBrShape)((mBrShape[curBr]-1 + BRS_ALL) % BRS_ALL);  updBrush();  }  break;
		case SDLK_l:	if (ctrl)  {  mBrShape[curBr] = (EBrShape)((mBrShape[curBr]+1) % BRS_ALL);            updBrush();  }  break;
		case SDLK_n: case SDLK_COMMA:	mBrOct[curBr] = std::max(1, mBrOct[curBr]-1);  updBrush();  break;
		case SDLK_m: case SDLK_PERIOD:	mBrOct[curBr] = std::min(7, mBrOct[curBr]+1);  updBrush();  break;
	}

	//  ter brush presets  ----
	if (edMode < ED_Road && alt && arg.keysym.sym >= SDLK_1 && arg.keysym.sym <= SDLK_0 && !bMoveCam)
	{
		// TODO
		int id = arg.keysym.sym - SDLK_1;
		if (shift)  id += 10;
		SetBrushPreset(id);
	}

	
	//  Fluids  ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
	if (edMode == ED_Fluids)
	{	int fls = sc->fluids.size();
		switch (arg.keysym.sym)
		{
			//  ins
			case SDLK_INSERT:	case SDLK_KP_0:
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
		switch (arg.keysym.sym)
		{
			//  first, last
			case SDLK_HOME:  case SDLK_KP_7:
				iFlCur = 0;  UpdFluidBox();  break;
			case SDLK_END:  case SDLK_KP_1:
				if (fls > 0)  iFlCur = fls-1;  UpdFluidBox();  break;

			//  prev,next
			case SDLK_PAGEUP:	case SDLK_KP_9:
				if (fls > 0) {  iFlCur = (iFlCur-1+fls)%fls;  }  UpdFluidBox();  break;
			case SDLK_PAGEDOWN:	case SDLK_KP_3:
				if (fls > 0) {  iFlCur = (iFlCur+1)%fls;	  }  UpdFluidBox();  break;

			//  del
			case SDLK_DELETE:	case SDLK_KP_PERIOD:
			case SDLK_KP_5:
				if (fls == 1)	sc->fluids.clear();
				else			sc->fluids.erase(sc->fluids.begin() + iFlCur);
				iFlCur = std::max(0, std::min(iFlCur, (int)sc->fluids.size()-1));
				bRecreateFluids = true;
				break;

			//  prev,next type
			case SDLK_9:  case SDLK_MINUS:
			{	FluidBox& fb = sc->fluids[iFlCur];
				fb.id = (fb.id-1 + fluidsXml.fls.size()) % fluidsXml.fls.size();
				fb.name = fluidsXml.fls[fb.id].name;
				bRecreateFluids = true;  }	break;
			case SDLK_0:  case SDLK_EQUALS:
			{	FluidBox& fb = sc->fluids[iFlCur];
				fb.id = (fb.id+1) % fluidsXml.fls.size();
				fb.name = fluidsXml.fls[fb.id].name;
				bRecreateFluids = true;  }	break;
		}
	}

	//  Objects  | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
	if (edMode == ED_Objects)
	{	int objs = sc->objects.size(), objAll = vObjNames.size();
		switch (arg.keysym.sym)
		{
			case SDLK_SPACE:
				iObjCur = -1;  PickObject();  UpdObjPick();  break;
				
			//  prev,next type
			case SDLK_9:  case SDLK_MINUS:   SetObjNewType((iObjTNew-1 + objAll) % objAll);  break;
			case SDLK_0:  case SDLK_EQUALS:  SetObjNewType((iObjTNew+1) % objAll);  break;
				
			//  ins
			case SDLK_INSERT:	case SDLK_KP_0:
			if (road && road->bHitTer)
			{
				AddNewObj();
				//iObjCur = sc->objects.size()-1;  // auto select inserted-
				UpdObjPick();
			}	break;
			
			//  sel
			case SDLK_BACKSPACE:
				if (ctrl)  vObjSel.clear();  // unsel all
				else
				if (iObjCur > -1)
					if (vObjSel.find(iObjCur) == vObjSel.end())
						vObjSel.insert(iObjCur);  // add to sel
					else
						vObjSel.erase(iObjCur);  // unselect
				break;
		}
		::Object* o = iObjCur == -1 ? &objNew :
					((iObjCur >= 0 && objs > 0 && iObjCur < objs) ? &sc->objects[iObjCur] : 0);
		switch (arg.keysym.sym)
		{
			//  first, last
			case SDLK_HOME:  case SDLK_KP_7:
				iObjCur = 0;  UpdObjPick();  break;
			case SDLK_END:  case SDLK_KP_1:
				if (objs > 0)  iObjCur = objs-1;  UpdObjPick();  break;

			//  prev,next
			case SDLK_PAGEUP:	case SDLK_KP_9:
				if (objs > 0) {  iObjCur = (iObjCur-1+objs)%objs;  }  UpdObjPick();  break;
			case SDLK_PAGEDOWN:	case SDLK_KP_3:
				if (objs > 0) {  iObjCur = (iObjCur+1)%objs;	  }  UpdObjPick();  break;

			//  del
			case SDLK_DELETE:	case SDLK_KP_PERIOD:
			case SDLK_KP_5:
				if (iObjCur >= 0 && objs > 0)
				{	::Object& o = sc->objects[iObjCur];
					mSceneMgr->destroyEntity(o.ent);
					mSceneMgr->destroySceneNode(o.nd);
					
					if (objs == 1)	sc->objects.clear();
					else			sc->objects.erase(sc->objects.begin() + iObjCur);
					iObjCur = std::min(iObjCur, (int)sc->objects.size()-1);
					UpdObjPick();
				}	break;

			//  move,rot,scale
			case SDLK_1:
				if (!shift)  objEd = EO_Move;
				else if (o)
				{
					if (iObjCur == -1)  // reset h
					{
						o->pos[2] = 0.f;  o->SetFromBlt();  UpdObjPick();
					}
					else if (road)  // move to ter
					{
						const Ogre::Vector3& v = road->posHit;
						o->pos[0] = v.x;  o->pos[1] =-v.z;  o->pos[2] = v.y + objNew.pos[2];
						o->SetFromBlt();  UpdObjPick();
					}
				}	break;

			case SDLK_2:
				if (!shift)  objEd = EO_Rotate;
				else if (o)  // reset rot
				{
					o->rot = QUATERNION<float>(0,1,0,0);
					o->SetFromBlt();  UpdObjPick();
				}	break;

			case SDLK_3:
				if (!shift)  objEd = EO_Scale;
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
	switch (arg.keysym.sym)
	{
		case SDLK_q:	GuiShortcut(WND_Edit, 1);  return true;  // Q Track
		case SDLK_s:	GuiShortcut(WND_Edit, 2);  return true;  // S Sun

		case SDLK_h:	GuiShortcut(WND_Edit, 3);  return true;  // H Heightmap
		 case SDLK_d:	GuiShortcut(WND_Edit, 3,0);  return true;  //  D -Brushes

		case SDLK_t:	GuiShortcut(WND_Edit, 4);  return true;  // T Layers (Terrain)
		 case SDLK_b:	GuiShortcut(WND_Edit, 4,0);  return true;  //  B -Blendmap
		 case SDLK_p:	GuiShortcut(WND_Edit, 4,1);  return true;  //  P -Particles
		 case SDLK_u:	GuiShortcut(WND_Edit, 4,2);  return true;  //  U -Surfaces

		case SDLK_v:	GuiShortcut(WND_Edit, 5);  return true;  // V Vegetation
		 case SDLK_m:	GuiShortcut(WND_Edit, 5,2);  return true;  //  M -Models

		case SDLK_r:	GuiShortcut(WND_Edit, 6);  return true;  // R Road
		case SDLK_x:	GuiShortcut(WND_Edit, 7);  return true;  // X Objects
		case SDLK_o:	GuiShortcut(WND_Edit, 8);  return true;  // O Tools

		case SDLK_c:	GuiShortcut(WND_Options, 1);  return true;  // C Screen
		case SDLK_g:	GuiShortcut(WND_Options, 2);  return true;  // G Graphics
		 case SDLK_n:	GuiShortcut(WND_Options, 2,2);  return true;  // N -Vegetation
		case SDLK_e:	GuiShortcut(WND_Options, 3);  return true;  // E Settings
		case SDLK_k:	GuiShortcut(WND_Options, 4);  return true;  // K Tweak

		case SDLK_i:	GuiShortcut(WND_Help, 1);  return true;  // I Input/help
		case SDLK_j:	GuiShortcut(WND_Edit, 9);  return true;  // J Warnings
	}
	else
	switch (arg.keysym.sym)
	{
		case SDLK_TAB:	//  Camera / Edit mode
		if (!bGuiFocus && !alt)  {
			bMoveCam = !bMoveCam;  UpdVisGui();  UpdFluidBox();  UpdObjPick();
		}	break;

		//  fog
		case SDLK_g:  {
			pSet->bFog = !pSet->bFog;  chkFog->setStateSelected(pSet->bFog);  UpdFog();  }  break;
		//  trees
		case SDLK_v:	bTrGrUpd = true;  break;
		//  weather
		case SDLK_i:  {
			pSet->bWeather = !pSet->bWeather;  chkWeather->setStateSelected(pSet->bWeather);  }  break;

		//  terrain
		case SDLK_d:	if (bEdit()){  SetEdMode(ED_Deform);  curBr = 0;  updBrush();  UpdEditWnds();  }	break;
		case SDLK_s:	if (bEdit()){  SetEdMode(ED_Smooth);  curBr = 1;  updBrush();  UpdEditWnds();  }	break;
		case SDLK_e:	if (bEdit()){  SetEdMode(ED_Height);  curBr = 2;  updBrush();  UpdEditWnds();  }	break;
		case SDLK_f:  if (bEdit()){  SetEdMode(ED_Filter);  curBr = 3;  updBrush();  UpdEditWnds();  }
			else  //  focus on find edit  (global)
			if (ctrl && edFind //&& bGuiFocus &&
						  //!pSet->isMain && pSet->inMenu == WND_Edit && mWndTabsEdit->getIndexSelected() == 1
				)
			{
				GuiShortcut(WND_Edit, 1);  // Track tab
				MyGUI::InputManager::getInstance().resetKeyFocusWidget();
				MyGUI::InputManager::getInstance().setKeyFocusWidget(edFind);
				return true;
			}	break;

		//  road
		case SDLK_r:	if (bEdit()){  SetEdMode(ED_Road);	UpdEditWnds();  }	break;
		case SDLK_b:  if (road)  {  road->UpdPointsH();  road->RebuildRoad(true);  }  break;
		case SDLK_t:	if (mWndRoadStats)  mWndRoadStats->setVisible(!mWndRoadStats->getVisible());  break;
		case SDLK_m:  if (edMode == ED_Road && road)  road->ToggleMerge();  break;

		//  start pos
		case SDLK_q:	if (bEdit()){  SetEdMode(ED_Start);  UpdEditWnds();  }   break;
		case SDLK_SPACE:
			if (edMode == ED_Start && road)  road->iDir *= -1;  break;
		//  prv cam
		case SDLK_F7:  togPrvCam();  break;

		//  fluids
		case SDLK_w:	if (bEdit()){  SetEdMode(ED_Fluids);  UpdEditWnds();  }   break;
		case SDLK_F10:	SaveWaterDepth();   break;

		//  objects
		case SDLK_c:	if (edMode == ED_Objects)  {  objSim = !objSim;  ToggleObjSim();  }  break;
		case SDLK_x:	if (bEdit()){  SetEdMode(ED_Objects);  UpdEditWnds();  }   break;
		
		//  rivers
		///case SDLK_a:	if (bEdit()){  SetEdMode(ED_Rivers);  UpdEditWnds();  }	break;
	}

	return true;
}
