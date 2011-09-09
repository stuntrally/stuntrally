#include "pch.h"
#include "Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include "../vdrift/pathmanager.h"
using namespace Ogre;

//  Update  input, info
//---------------------------------------------------------------------------------------------------------------
//  tool wnds show/hide
void App::UpdEditWnds()
{
	if (mWndBrush){
		if (edMode == ED_Deform)
		{	mWndBrush->setCaption("Terrain Deform");  
			mWndBrush->setColour(MyGUI::Colour(0.5f, 0.9f, 0.3f));
			mWndBrush->setVisible(true);  }
		else if (edMode == ED_Smooth)
		{	mWndBrush->setCaption("Terrain Smooth");
			mWndBrush->setColour(MyGUI::Colour(0.3f, 0.8f, 0.8f));
			mWndBrush->setVisible(true);  }
		else if (edMode == ED_Height)
		{	mWndBrush->setCaption("Terrain Height");
			mWndBrush->setColour(MyGUI::Colour(0.7f, 1.0f, 0.7f));
			mWndBrush->setVisible(true);  }
		else if (edMode == ED_Start)
		{	mWndBrush->setCaption("Car Start pos");
			mWndBrush->setColour(MyGUI::Colour(0.7f, 0.7f, 1.0f));
			mWndBrush->setVisible(true);  }
		else
			mWndBrush->setVisible(false);
	}
	bool bRoad = edMode == ED_Road;
	if (mWndRoadCur)  mWndRoadCur->setVisible(bRoad);
	if (mWndRoadStats)  mWndRoadStats->setVisible(bRoad);
	if (mWndCam)  mWndCam->setVisible(edMode == ED_PrvCam);
	UpdStartPos();  // StBox visible
	UpdVisGui();  //br prv..
}
void App::UpdVisGui()
{
	if (mWndOpts)  mWndOpts->setVisible(bGuiFocus);
	if (bnQuit)  bnQuit->setVisible(bGuiFocus);  //TODO: ?crash from lang change combo
	if (mGUI)  mGUI->setVisiblePointer(bGuiFocus || !bMoveCam);
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
		mViewport->setVisibilityMask(255);
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
		mViewport->setVisibilityMask(256);
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


//  Key Press
//---------------------------------------------------------------------------------------------------------------
void App::trkListNext(int rel)
{
	if (!(bGuiFocus && mWndTabs->getIndexSelected() == 0))  return;
	int i = std::max(0, std::min((int)trkList->getItemCount()-1, (int)trkList->getIndexSelected()+rel ));
	trkList->setIndexSelected(i);
	trkList->beginToItemAt(std::max(0, i-11));  // center
	listTrackChng(trkList,i);
}

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
				rt[RTs-1].rndTex->writeContentsToFile(pathTrkPrv[1] + pSet->track + ".jpg");  //U
				listTrackChng(trkList,0);  // upd gui img
				Status("Preview saved", 1,1,0);
				break;
		}
		return true;  //!
	}


	int num = mWndOpts ? mWndTabs->getItemCount() : 0;

	switch (arg.key)  //  global keys  ---------------------
	{
		//  Show Stats  I
   		case KC_I:  if (alt)
		{	mStatsOn = !mStatsOn;	
			if (mStatsOn)  mDebugOverlay->show();  else  mDebugOverlay->hide();
			return true;
		}	break;

		//  Wire Frame  F10
		case KC_F10:
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

		case KC_F8:  UpdateTrack();	return true;

   		case KC_F2:  // +-rt num
   			if (alt) {	pSet->num_mini = (pSet->num_mini - 1 + RTs+2) % (RTs+2);  UpdMiniVis();  }
   			else if (bGuiFocus)  // prev gui tab
   				mWndTabs->setIndexSelected( (mWndTabs->getIndexSelected() - 1 + num) % num );
   			break;
   		case KC_F3:
   			if (alt) {	pSet->num_mini = (pSet->num_mini + 1) % (RTs+2);  UpdMiniVis();  }
   			else if (bGuiFocus)  // next gui tab
   				mWndTabs->setIndexSelected( (mWndTabs->getIndexSelected() + 1) % num );
   			break;
   			
   		case KC_RETURN:  // load track
			if (bGuiFocus)
			if (mWndTabs->getIndexSelected() == 0)
				btnNewGame(0);
   			break;
	}

	if (bGuiFocus && mGUI)  //  GUI  ---------------------
	{
		MyGUI::Char text = arg.text;
		if (shift)	// shift-letters,numbers dont work ??
		{
			const static int num = 21;
			const static MyGUI::Char chT[num][2] = {
				{'0',')'},{'1','!'},{'2','@'},{'3','#'},{'4','$'},{'5','%'},{'6','^'},{'7','&'},{'8','*'},{'9','('},
				{'`','~'},{'-','_'},{'=','+'},{'[','{'},{']','}'},{'\\','|'},{';',':'},{'\'','\"'},{',','<'},{'.','>'},{'/','?'}};
			if (text >= 'a' && text <= 'z')
				text += 'A'-'a';
			else
			for (int i=0; i < num; ++i)
				if (text == chT[i][0])
				{	text = chT[i][1];  break;	}
		}
		if (arg.key == KC_ESCAPE || arg.key == KC_BACK)
			text = 0;
		mGUI->injectKeyPress(MyGUI::KeyCode::Enum(arg.key), text);
		return true;
	}


	///  Road keys  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
	if (edMode == ED_Road && road && bEdit())
	{
	if (iSnap > 0)
	switch (arg.key)
	{
		case KC_1:	road->AddYaw(-1,angSnap);	break;
		case KC_2:	road->AddYaw( 1,angSnap);	break;
		case KC_3:	road->AddRoll(-1,angSnap);	break;
		case KC_4:	road->AddRoll( 1,angSnap);	break;
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
	}
	}

	//  ter brush shape
	if (edMode < ED_Road)
	switch (arg.key)
	{
		case KC_K:	mBrShape[curBr] = (EBrShape)((mBrShape[curBr]-1 + BRS_ALL) % BRS_ALL);  updBrush();  break;
		case KC_L:	mBrShape[curBr] = (EBrShape)((mBrShape[curBr]+1) % BRS_ALL);            updBrush();  break;
		case KC_COMMA:	mBrOct[curBr] = std::max(1, mBrOct[curBr]-1);  updBrush();  break;
		case KC_PERIOD:	mBrOct[curBr] = std::min(7, mBrOct[curBr]+1);  updBrush();  break;
	}

	///  Common Keys  * * * * * * * * * * * * *
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
			pSet->bFog = !pSet->bFog;  chkFog->setStateCheck(pSet->bFog);  UpdFog();  }  break;

		//  trees
		case KC_V:	bTrGrUpd = true;  break;

		//  terrain
		case KC_D:	if (bEdit()){  edMode = ED_Deform;  curBr = 0;  updBrush();  UpdEditWnds();  }	break;
		case KC_S:	if (bEdit()){  edMode = ED_Smooth;  curBr = 1;  updBrush();  UpdEditWnds();  }	break;
		case KC_E:	if (bEdit()){  edMode = ED_Height;  curBr = 2;  updBrush();  UpdEditWnds();  }	break;
		//case KC_F: TODO: ter brush  filter, ramp ...

		//  road
		case KC_R:	if (bEdit()){  edMode = ED_Road;	UpdEditWnds();  }	break;
		case KC_B:  if (road)  road->RebuildRoad(true);  break;
		case KC_T:	if (mWndRoadStats)  mWndRoadStats->setVisible(!mWndRoadStats->isVisible());  break;
		case KC_M:  if (road)  road->ToggleMerge();  break;

		//  start pos
		case KC_Q:	if (bEdit()){  edMode = ED_Start;  UpdEditWnds();  }   break;
		//  fluids
		case KC_W:	if (bEdit()){  edMode = ED_Fluids;  UpdEditWnds();  }   break;
		
		case KC_SPACE:
			if (edMode == ED_Start && road)  road->iDir *= -1;  break;
			
		case KC_F7:  togPrvCam();  break;
	}
	return true;
}
