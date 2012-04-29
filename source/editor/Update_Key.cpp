#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include "../vdrift/pathmanager.h"
#include "../ogre/common/MultiList2.h"
#include "../ogre/common/RenderConst.h"
#include <MyGUI.h>
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

	UpdStartPos();  // StBox visible
	UpdVisGui();  //br prv..
}

void App::UpdVisGui()
{
	bool notMain = bGuiFocus && !pSet->isMain;
	if (mWndMain)	mWndMain->setVisible(bGuiFocus && pSet->isMain);
	if (mWndEdit)	mWndEdit->setVisible(notMain && pSet->inMenu == WND_Edit);
	if (mWndHelp)	mWndHelp->setVisible(notMain && pSet->inMenu == WND_Help);
	if (mWndOpts)	mWndOpts->setVisible(notMain && pSet->inMenu == WND_Options);

	if (bnQuit)  bnQuit->setVisible(bGuiFocus);
	if (mGUI)  PointerManager::getInstance().setVisible(bGuiFocus || !bMoveCam);
	if (road)  road->SetTerHitVis(bEdit());
	if (!bGuiFocus && mToolTip)  mToolTip->setVisible(false);

	if (ovBrushPrv)
	if (edMode >= ED_Road || !bEdit())
		ovBrushPrv->hide();  else  ovBrushPrv->show();

	for (int i=0; i < WND_ALL; ++i)
		mWndMainPanels[i]->setVisible(pSet->inMenu == i);
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
	static bool oldV = false;
	if (edMode == ED_PrvCam)  // leave
	{
		edMode = edModeOld;
		mViewport->setVisibilityMask(RV_MaskAll);
		rt[RTs].ndMini->setVisible(false);
		ndCar->setVisible(true);

		UpdFog();  // restore fog, veget
		if (oldV)  {  bTrGrUpd = true;  oldV = false;  }
		mTerrainGlobals->setMaxPixelError(pSet->terdetail);

		sc.camPos = mCameraT->getPosition();
		sc.camDir = mCameraT->getDirection();
		mCameraT->setPosition( mCamPosOld);
		mCameraT->setDirection(mCamDirOld);
	}else  // enter
	{
		edModeOld = edMode;
		edMode = ED_PrvCam;
		bMoveCam = true;  UpdVisGui();
		mViewport->setVisibilityMask(RV_MaskPrvCam);
		rt[RTs].ndMini->setVisible(true);
		ndCar->setVisible(false);

		UpdFog(true);  // on fog, veget
		if (!pSet->bTrees)  {  bTrGrUpd = true;  oldV = true;  }
		mTerrainGlobals->setMaxPixelError(0.5f);  //hq ter

		mCamPosOld = mCameraT->getPosition();
		mCamDirOld = mCameraT->getDirection();
		mCameraT->setPosition( sc.camPos);
		mCameraT->setDirection(sc.camDir);
	}
	UpdEditWnds();
}


//  key util
void App::trkListNext(int rel)
{
	bool b = bGuiFocus && (mWndTabsEdit->getIndexSelected() == 0)
		&& !pSet->isMain && pSet->inMenu == WND_Edit;
	if (!b)  return;
	
	int i = std::max(0, std::min((int)trkMList->getItemCount()-1, (int)trkMList->getIndexSelected()+rel ));
	trkMList->setIndexSelected(i);
	trkMList->beginToItemAt(std::max(0, i-11));  // center
	listTrackChng(trkMList,i);
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
	if (!bGuiFocus)
	if (edMode != ED_PrvCam)  {
		bGuiFocus = !bGuiFocus;  UpdVisGui();  /*subtab = -2;*/  }

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
	MyGUI::TabControl* tc = (*subt)[t];  if (!tc)  return;
	int  cnt = tc->getItemCount();

	if (t == tab && subtab == -1)  // cycle subpages if same tab
	{	if (shift)
			tc->setIndexSelected( (tc->getIndexSelected()-1+cnt) % cnt );
		else
			tc->setIndexSelected( (tc->getIndexSelected()+1) % cnt );
	}else
	if (subtab > -1)
		tc->setIndexSelected( std::min(cnt-1, subtab) );
}


//---------------------------------------------------------------------------------------------------------------
//  Key Press
//---------------------------------------------------------------------------------------------------------------
bool App::KeyPress(const CmdKey &arg)
{	
	using namespace OIS;

	///  Preview camera  ---------------------
	if (edMode == ED_PrvCam)
	{
		switch (arg.key)
		{
			case KC_ESCAPE:  // exit
			case KC_F7:  togPrvCam();  break;

			case KC_RETURN:  // save screen
			{	int u = pSet->allow_save ? pSet->gui.track_user : 1;
				rt[RTs-1].rndTex->writeContentsToFile(pathTrkPrv[u] + pSet->gui.track + ".jpg");
				listTrackChng(trkMList,0);  // upd gui img
				Status("Preview saved", 1,1,0);
			}	break;
		}
		return true;  //!
	}

	//  main menu keys
	Widget* wf = MyGUI::InputManager::getInstance().getKeyFocusWidget();
	bool edFoc = wf && wf->getTypeName() == "EditBox";

	if (pSet->isMain && bGuiFocus)
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
	if (!pSet->isMain && bGuiFocus)
	{
		switch (arg.key)
		{
		case KC_BACK:
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
		case WND_Help:  /*tab = mWndTabsHelp;*/ tab = sub = vSubTabsHelp[1];  iTab1 = 0;  break;
		case WND_Options:  tab = mWndTabsOpts;  sub = vSubTabsOpts[tab->getIndexSelected()];  break;
	}

	switch (arg.key)  //  global keys  ---------------------
	{
		//  Show Stats  I
   		case KC_I:  if (ctrl)
		{	mStatsOn = !mStatsOn;	
			if (mStatsOn)  mDebugOverlay->show();  else  mDebugOverlay->hide();
			return true;
		}	break;

		//  Wire Frame  F11
		case KC_F11:
		{	mbWireFrame = !mbWireFrame;
			mCamera->setPolygonMode(mbWireFrame ? PM_WIREFRAME : PM_SOLID);
			if (ndSky)	ndSky->setVisible(!mbWireFrame);  // hide sky
			return true;
		}	break;

		case KC_ESCAPE: //  quit
			if (pSet->escquit)
			{	inputThreadRunning = false;
				mShutDown = true;
			}	return true;

		case KC_F1:
		case KC_GRAVE:	//  Gui mode Options
			toggleGui(true);  return true;

		case KC_SYSRQ:
			mWindow->writeContentsToTimestampedFile(PATHMANAGER::GetScreenShotDir() + "/", ".jpg");
			return true;

		//  save, reload, update
		case KC_F4:  SaveTrack();	return true;
		case KC_F5:  LoadTrack();	return true;

		case KC_F8:  UpdateTrack();  return true;
		case KC_F9:  bTerUpdBlend = true;  return true;

   		case KC_F2:  // +-rt num
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
   		case KC_F3:
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
   			
   		case KC_RETURN:  // load track
			if (bGuiFocus)
			if (mWndTabsEdit->getIndexSelected() == 0 && !pSet->isMain && pSet->inMenu == WND_Edit)
				btnNewGame(0);
   			break;
	}

	//  keys in edits
	if (bGuiFocus && mGUI && !alt && !ctrl)  //  GUI  ---------------------
	{
		MyGUI::Char text = arg.text;  bool found = false;
		if (shift)	// shift-letters,numbers dont work ??
		{
			const static int num = 21;
			const static MyGUI::Char chT[num][2] = {
				{'0',')'},{'1','!'},{'2','@'},{'3','#'},{'4','$'},{'5','%'},{'6','^'},{'7','&'},{'8','*'},{'9','('},
				{'`','~'},{'-','_'},{'=','+'},{'[','{'},{']','}'},{'\\','|'},{';',':'},{'\'','\"'},{',','<'},{'.','>'},{'/','?'}};
			if (text >= 'a' && text <= 'z')
			{	text += 'A'-'a';  found = true;  }
			else
			for (int i=0; i < num; ++i)
				if (text == chT[i][0])
				{	text = chT[i][1];  found = true;  break;	}
		}
		if (arg.key == KC_ESCAPE || arg.key == KC_BACK)
		{	text = 0;  found = true;  }
		//if (found)
			MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::Enum(arg.key), text);
		return true;
	}


	///  Road keys  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
	if (edMode == ED_Road && road && bEdit())
	{
	if (iSnap > 0)
	switch (arg.key)
	{
		case KC_1:	road->AddYaw(-1,angSnap,alt);	break;
		case KC_2:	road->AddYaw( 1,angSnap,alt);	break;
		case KC_3:	road->AddRoll(-1,angSnap,alt);	break;
		case KC_4:	road->AddRoll( 1,angSnap,alt);	break;
	}
	switch (arg.key)
	{
		//  choose 1
		case KC_SPACE:
			if (ctrl)	road->CopyNewPoint();
			else		road->ChoosePoint();  break;
			
		//  multi sel
		case KC_BACK:
			if (alt)		road->SelAll();
			else if (ctrl)	road->SelClear();
			else			road->SelAddPoint();  break;
			
		//  ter on  first,last
		case KC_HOME:  case KC_NUMPAD7:
			if (ctrl)	road->FirstPoint();
			else		road->ToggleOnTerrain();  break;
			
		//  cols
		case KC_END:  case KC_NUMPAD1:
			if (ctrl)	road->LastPoint();
			else		road->ToggleColums();  break;

		//  prev,next
		case KC_PGUP:	case KC_NUMPAD9:
			road->PrevPoint();  break;
		case KC_PGDOWN:	case KC_NUMPAD3:
			road->NextPoint();  break;

		//  del
		case KC_DELETE:	case KC_DECIMAL:
		case KC_NUMPAD5:
			if (ctrl)	road->DelSel();
			else		road->Delete();  break;

		//  ins
		case KC_INSERT:	case KC_NUMPAD0:
		if (ctrl && !shift && !alt)	{	if (road->CopySel())  Status("Copy",0.6,0.8,1.0);  }
		else if (!ctrl && shift && !alt)	road->Paste();
		else if ( ctrl && shift && !alt)	road->Paste(true);
		else
		{	road->newP.pos.x = road->posHit.x;
			road->newP.pos.z = road->posHit.z;
			road->newP.aType = AT_Both;
			road->Insert(shift ? INS_Begin : ctrl ? INS_End : alt ? INS_CurPre : INS_Cur);
		}	break;					  

		case KC_MINUS:   road->ChgMtrId(-1);	break;
		case KC_EQUALS:  road->ChgMtrId(1);		break;

		case KC_5:	road->ChgAngType(-1);	break;
		case KC_6:	if (shift)  road->AngZero();  else
					road->ChgAngType(1);	break;

		case KC_7:  iSnap = (iSnap-1+ciAngSnapsNum)%ciAngSnapsNum;  angSnap = crAngSnaps[iSnap];  break;
		case KC_8:  iSnap = (iSnap+1)%ciAngSnapsNum;                angSnap = crAngSnaps[iSnap];  break;
		
		case KC_0:  road->Set1stChk();  break;
	}
	}

	//  ter brush shape
	if (edMode < ED_Road && !alt)
	switch (arg.key)
	{
		case KC_K:	mBrShape[curBr] = (EBrShape)((mBrShape[curBr]-1 + BRS_ALL) % BRS_ALL);  updBrush();  break;
		case KC_L:	mBrShape[curBr] = (EBrShape)((mBrShape[curBr]+1) % BRS_ALL);            updBrush();  break;
		case KC_COMMA:	mBrOct[curBr] = std::max(1, mBrOct[curBr]-1);  updBrush();  break;
		case KC_PERIOD:	mBrOct[curBr] = std::min(7, mBrOct[curBr]+1);  updBrush();  break;
	}
	
	//  Fluids ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
	if (edMode == ED_Fluids)
	{	int fls = sc.fluids.size();
		switch (arg.key)
		{
			//  ins
			case KC_INSERT:	case KC_NUMPAD0:
			if (road && road->bHitTer)
			{
				FluidBox fb;	fb.name = "water blue";
				fb.pos = road->posHit;	fb.rot = Ogre::Vector3(0.f, 0.f, 0.f);
				fb.size = Ogre::Vector3(50.f, 20.f, 50.f);	fb.tile = Vector2(0.01f, 0.01f);
				sc.fluids.push_back(fb);
				sc.UpdateFluidsId();
				iFlCur = sc.fluids.size()-1;
				bRecreateFluids = true;
			}	break;
		}
		if (fls > 0)
		switch (arg.key)
		{
			//  first, last
			case KC_HOME:  case KC_NUMPAD7:
				iFlCur = 0;  UpdFluidBox();  break;
			case KC_END:  case KC_NUMPAD1:
				if (fls > 0)  iFlCur = fls-1;  UpdFluidBox();  break;

			//  prev,next
			case KC_PGUP:	case KC_NUMPAD9:
				if (fls > 0) {  iFlCur = (iFlCur-1+fls)%fls;  }  UpdFluidBox();  break;
			case KC_PGDOWN:	case KC_NUMPAD3:
				if (fls > 0) {  iFlCur = (iFlCur+1)%fls;	  }  UpdFluidBox();  break;

			//  del
			case KC_DELETE:	case KC_DECIMAL:
			case KC_NUMPAD5:
				if (fls == 1)	sc.fluids.clear();
				else			sc.fluids.erase(sc.fluids.begin() + iFlCur);
				iFlCur = std::min(iFlCur, (int)sc.fluids.size()-1);
				bRecreateFluids = true;
				break;

			//  prev,next type
			case KC_MINUS:
			{	FluidBox& fb = sc.fluids[iFlCur];
				fb.id = (fb.id-1 + fluidsXml.fls.size()) % fluidsXml.fls.size();
				fb.name = fluidsXml.fls[fb.id].name;
				bRecreateFluids = true;  }	break;
			case KC_EQUALS:
			{	FluidBox& fb = sc.fluids[iFlCur];
				fb.id = (fb.id+1) % fluidsXml.fls.size();
				fb.name = fluidsXml.fls[fb.id].name;
				bRecreateFluids = true;  }	break;
		}
	}

	//  Objects  | | | | | | | | | | | | | | | | |
	if (edMode == ED_Objects)
	{	int objs = sc.objects.size(), objAll = vObjNames.size();
		switch (arg.key)
		{
			case KC_SPACE:
				iObjCur = -1;  break;  // unselect
				
			//  prev,next type
			case KC_LBRACKET:
				iObjNew = (iObjNew-1 + objAll)%objAll;  break;
			case KC_RBRACKET:
				iObjNew = (iObjNew+1)%objAll;  break;
				
			//  ins
			case KC_INSERT:	case KC_NUMPAD0:
			if (road && road->bHitTer)
			{
				::Object o;  o.name = vObjNames[iObjNew];
				const Ogre::Vector3& v = road->posHit;
				o.pos[0] = v.x;  o.pos[1] =-v.z;  o.pos[2] = v.y;  //o.pos.y += 0.5f;
				//todo: ?dyn objs size, get center,size, rmb height..
				String s = toStr(sc.objects.size()+1);  // counter for names

				//  create object
				o.ent = mSceneMgr->createEntity("oE"+s, o.name + ".mesh");
				o.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode("oN"+s);
				o.SetFromBlt();
				o.nd->setScale(o.scale);
				o.nd->attachObject(o.ent);  o.ent->setVisibilityFlags(RV_Vegetation);

				sc.objects.push_back(o);
				iObjCur = sc.objects.size()-1;
				UpdObjPick();
			}	break;
		}
		if (objs > 0)
		switch (arg.key)
		{
			//  first, last
			case KC_HOME:  case KC_NUMPAD7:
				iObjCur = 0;  UpdObjPick();  break;
			case KC_END:  case KC_NUMPAD1:
				if (objs > 0)  iObjCur = objs-1;  UpdObjPick();  break;

			//  prev,next
			case KC_PGUP:	case KC_NUMPAD9:
				if (objs > 0) {  iObjCur = (iObjCur-1+objs)%objs;  }  UpdObjPick();  break;
			case KC_PGDOWN:	case KC_NUMPAD3:
				if (objs > 0) {  iObjCur = (iObjCur+1)%objs;	  }  UpdObjPick();  break;

			//  del
			case KC_DELETE:	case KC_DECIMAL:
			case KC_NUMPAD5:
				if (iObjCur >= 0 && sc.objects.size() > 0)
				{	::Object& o = sc.objects[iObjCur];
					mSceneMgr->destroyEntity(o.ent);
					mSceneMgr->destroySceneNode(o.nd);
					
					if (objs == 1)	sc.objects.clear();
					else			sc.objects.erase(sc.objects.begin() + iObjCur);
					iObjCur = std::min(iObjCur, (int)sc.objects.size()-1);
					UpdObjPick();
				}	break;

			//  prev,next type
			case KC_1:  // reset rot
				if (iObjCur >= 0 && sc.objects.size() > 0)
				{	::Object& o = sc.objects[iObjCur];
					o.rot = QUATERNION <float>(0,1,0,0);
					o.SetFromBlt();
					UpdObjPick();
				}	break;

			case KC_2:  // reset scale
				if (iObjCur >= 0 && sc.objects.size() > 0)
				{	::Object& o = sc.objects[iObjCur];
					o.scale = Ogre::Vector3::UNIT_SCALE;
					o.nd->setScale(o.scale);
					UpdObjPick();
				}	break;
		}
	}

	///  Common Keys  * * * * * * * * * * * * *
	if (alt)
	switch (arg.key)
	{
		case KC_Q:	GuiShortcut(WND_Edit, 1);  return true;  // Q Track
		case KC_S:	GuiShortcut(WND_Edit, 2);  return true;  // S Sun

		case KC_H:	GuiShortcut(WND_Edit, 3);  return true;  // H Terrain (Heightmap)
		case KC_T:	GuiShortcut(WND_Edit, 4);  return true;  // T Layers (Terrain)
		 case KC_B:	GuiShortcut(WND_Edit, 4,0);  return true;  //  B -Blendmap
		 case KC_P:	GuiShortcut(WND_Edit, 4,1);  return true;  //  P -Particles
		 case KC_U:	GuiShortcut(WND_Edit, 4,2);  return true;  //  U -Surfaces

		case KC_V:	GuiShortcut(WND_Edit, 5);  return true;  // V Vegetation
		 case KC_M:	GuiShortcut(WND_Edit, 5,1);  return true;  //  M -Models

		case KC_R:	GuiShortcut(WND_Edit, 6);  return true;  // R Road
		case KC_O:	GuiShortcut(WND_Edit, 7);  return true;  // O Tools

		case KC_C:	GuiShortcut(WND_Options, 1);  return true;  // S Screen
		case KC_G:	GuiShortcut(WND_Options, 2);  return true;  // G Graphics
		 case KC_N:	GuiShortcut(WND_Options, 2,2);  return true;  // N -Vegetation
		case KC_E:	GuiShortcut(WND_Options, 3);  return true;  // E Settings

		case KC_I:	GuiShortcut(WND_Help, 1);  return true;  // I Input/help
	}
	else
	switch (arg.key)
	{
		case KC_TAB:	//  Camera / Edit mode
		if (!bGuiFocus && !alt)  {
			#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
				SetCursor(0);
				ShowCursor(0);  //?- cursor after alt-tab
			#endif
			bMoveCam = !bMoveCam;  UpdVisGui();  UpdFluidBox();  UpdObjPick();
		}	break;

		//  fog
		case KC_G:  {
			pSet->bFog = !pSet->bFog;  chkFog->setStateSelected(pSet->bFog);  UpdFog();  }  break;
		//  trees
		case KC_V:	bTrGrUpd = true;  break;

		//  terrain
		case KC_D:	if (bEdit()){  edMode = ED_Deform;  curBr = 0;  updBrush();  UpdEditWnds();  }	break;
		case KC_S:	if (bEdit()){  edMode = ED_Smooth;  curBr = 1;  updBrush();  UpdEditWnds();  }	break;
		case KC_E:	if (bEdit()){  edMode = ED_Height;  curBr = 2;  updBrush();  UpdEditWnds();  }	break;
		case KC_F:  if (bEdit()){  edMode = ED_Filter;  curBr = 3;  updBrush();  UpdEditWnds();  }
			else  //  focus on find edit
			if (ctrl && edFind && bGuiFocus &&
				!pSet->isMain && pSet->inMenu == WND_Edit && mWndTabsEdit->getIndexSelected() == 0)
			{
				MyGUI::InputManager::getInstance().resetKeyFocusWidget();
				MyGUI::InputManager::getInstance().setKeyFocusWidget(edFind);
				return true;
			}	break;

		//  road
		case KC_R:	if (bEdit()){  edMode = ED_Road;	UpdEditWnds();  }	break;
		case KC_B:  if (road)  road->RebuildRoad(true);  break;
		case KC_T:	if (mWndRoadStats)  mWndRoadStats->setVisible(!mWndRoadStats->getVisible());  break;
		case KC_M:  if (road)  road->ToggleMerge();  break;

		//  start pos
		case KC_Q:	if (bEdit()){  edMode = ED_Start;  UpdEditWnds();  }   break;
		case KC_SPACE:
			if (edMode == ED_Start && road)  road->iDir *= -1;  break;
		//  prv cam
		case KC_F7:  togPrvCam();  break;

		//  fluids
		case KC_W:	if (bEdit()){  edMode = ED_Fluids;  UpdEditWnds();  }   break;
		case KC_F10:	SaveWaterDepth();   break;

		//  objects
		case KC_X:	if (bEdit()){  edMode = ED_Objects;  UpdEditWnds();  }   break;
	}

	return true;
}
