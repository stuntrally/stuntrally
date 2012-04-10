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
	if (mWndBrush){
		if (edMode == ED_Deform)
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
		else if (edMode == ED_Start)
		{	static_cast<StaticTextPtr>(mWndBrush)->setCaption("Car Start pos");
			mWndBrush->setColour(MyGUI::Colour(0.7f, 0.7f, 1.0f));
			mWndBrush->setVisible(true);  }
		else
			mWndBrush->setVisible(false);
	}
	bool bRoad = edMode == ED_Road;
	if (mWndRoadCur)  mWndRoadCur->setVisible(bRoad);
	//if (mWndRoadStats)  mWndRoadStats->setVisible(bRoad);
	if (mWndCam)  mWndCam->setVisible(edMode == ED_PrvCam);
	if (mWndFluids)  mWndFluids->setVisible(edMode == ED_Fluids);
	UpdStartPos();  // StBox visible
	UpdVisGui();  //br prv..
}
void App::UpdVisGui()
{
	if (mWndOpts)  mWndOpts->setVisible(bGuiFocus);
	if (bnQuit)  bnQuit->setVisible(bGuiFocus);
	if (mGUI)  PointerManager::getInstance().setVisible(bGuiFocus || !bMoveCam);
	if (road)  road->SetTerHitVis(bEdit());
	if (!bGuiFocus && mToolTip)  mToolTip->setVisible(false);

	if (ovBrushPrv)
	if (edMode >= ED_Road || !bEdit())
		ovBrushPrv->hide();  else  ovBrushPrv->show();
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
	if (!(bGuiFocus && mWndTabs->getIndexSelected() == 0))  return;
	int i = std::max(0, std::min((int)trkMList->getItemCount()-1, (int)trkMList->getIndexSelected()+rel ));
	trkMList->setIndexSelected(i);
	trkMList->beginToItemAt(std::max(0, i-11));  // center
	listTrackChng(trkMList,i);
}

void App::GuiShortcut(int tab, int subtab)
{
	if (!bGuiFocus)
	if (edMode != ED_PrvCam)  {
		bGuiFocus = !bGuiFocus;  UpdVisGui();  subtab = -2;  }

	size_t t = mWndTabs->getIndexSelected();
	mWndTabs->setIndexSelected(tab);

	MyGUI::TabControl* tc = vSubTabs[t];  if (!tc)  return;
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
			{	int u = pSet->allow_save ? 0 : 1;
				rt[RTs-1].rndTex->writeContentsToFile(pathTrkPrv[u] + pSet->gui.track + ".jpg");
				listTrackChng(trkMList,0);  // upd gui img
				Status("Preview saved", 1,1,0);
			}	break;
		}
		return true;  //!
	}


	int num = mWndOpts ? mWndTabs->getItemCount() : 0;

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
		if (edMode != ED_PrvCam)  {
			bGuiFocus = !bGuiFocus;  UpdVisGui();  }  return true;

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
   			if (bGuiFocus)
   				if (shift)  // prev gui subtab
   				{
   					MyGUI::TabControl* sub = vSubTabs[mWndTabs->getIndexSelected()];
   					if (sub)  {  int num = sub->getItemCount();
   						sub->setIndexSelected( (sub->getIndexSelected() - 1 + num) % num );  }
	   			}
   				else	// prev gui tab
	   				mWndTabs->setIndexSelected( (mWndTabs->getIndexSelected() - 1 + num) % num );
   			break;
   		case KC_F3:
   			if (alt)
   			{	pSet->num_mini = (pSet->num_mini + 1) % (RTs+2);  UpdMiniVis();  }
   			else
   			if (bGuiFocus)
   				if (shift)  // next gui subtab
   				{
   					MyGUI::TabControl* sub = vSubTabs[mWndTabs->getIndexSelected()];
   					if (sub)  {  int num = sub->getItemCount();
   						sub->setIndexSelected( (sub->getIndexSelected() + 1) % num );  }
	   			}
	   			else	// next gui tab
   					mWndTabs->setIndexSelected( (mWndTabs->getIndexSelected() + 1) % num );
   			break;
   			
   		case KC_RETURN:  // load track
			if (bGuiFocus)
			if (mWndTabs->getIndexSelected() == 0)
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
				iFlCur = 0;  break;
			case KC_END:  case KC_NUMPAD1:
				if (fls > 0)  iFlCur = fls-1;  break;

			//  prev,next
			case KC_PGUP:	case KC_NUMPAD9:
				if (fls > 0) {  iFlCur = (iFlCur-1+fls)%fls;	}	break;
			case KC_PGDOWN:	case KC_NUMPAD3:
				if (fls > 0) {  iFlCur = (iFlCur+1)%fls;		}	break;

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

	///  Common Keys  * * * * * * * * * * * * *
	if (alt)
	switch (arg.key)
	{
		case KC_Q:	GuiShortcut(0);  return true;  // Q Track
		case KC_S:	GuiShortcut(1);  return true;  // S Sun

		case KC_H:	GuiShortcut(2);  return true;  // H Terrain (Heightmap)
		case KC_T:	GuiShortcut(3);  return true;  // T Layers (Terrain)
		 case KC_B:	GuiShortcut(3,0);  return true;  //  B -Blendmap
		 case KC_P:	GuiShortcut(3,1);  return true;  //  P -Particles
		 case KC_U:	GuiShortcut(3,2);  return true;  //  U -Surfaces

		case KC_V:	GuiShortcut(4);  return true;  // V Vegetation
		 case KC_M:	GuiShortcut(4,1);  return true;  //  M -Models

		case KC_R:	GuiShortcut(5);  return true;  // R Road
		case KC_O:	GuiShortcut(6);  return true;  // O Tools

		case KC_C:	GuiShortcut(7);  return true;  // S Screen
		case KC_G:	GuiShortcut(8);  return true;  // G Graphics
		 case KC_N:	GuiShortcut(8,2);  return true;  // N -Vegetation

		case KC_I:	GuiShortcut(9);  return true;  // I Input/help
		case KC_E:	GuiShortcut(10);  return true;  // E Settings
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
			bMoveCam = !bMoveCam;  UpdVisGui();  }	break;

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
			if (ctrl && edFind && bGuiFocus && mWndTabs->getIndexSelected() == 0)
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
		//  fluids
		case KC_W:	if (bEdit()){  edMode = ED_Fluids;  UpdEditWnds();  }   break;
		case KC_F10:	SaveWaterDepth();   break;
		
		case KC_SPACE:
			if (edMode == ED_Start && road)  road->iDir *= -1;  break;
			
		case KC_F7:  togPrvCam();  break;
	}

	return true;
}
