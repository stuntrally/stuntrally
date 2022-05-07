#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/data/CData.h"
#include "../ogre/common/data/SceneXml.h"
#include "../ogre/common/data/FluidsXml.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../vdrift/pathmanager.h"
#include "../ogre/common/MultiList2.h"
#include <OgreTerrain.h>
#include <OgreSceneNode.h>
#include <OgreRenderTexture.h>
#include <OgreEntity.h>
#include <MyGUI.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include "../sdl4ogre/sdlinputwrapper.hpp"
#include "../sdl4ogre/sdlcursormanager.hpp"
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
		case WND_Edit:    tab = mWndTabsEdit;  sub = gui->vSubTabsEdit[tab->getIndexSelected()];  break;
		case WND_Help:    tab = sub = gui->vSubTabsHelp[1];  iTab1 = 0;  break;
		case WND_Options: tab = mWndTabsOpts;  sub = gui->vSubTabsOpts[tab->getIndexSelected()];  break;
	}
	SplineRoad* road = scn->road;
	bool bRoad = edMode == ED_Road && road && bEdit();


	///  Pick open  ---------------------
	bool editGui = bGuiFocus && !pSet->bMain && pSet->inMenu==WND_Edit;
	if (skey==key(TAB) && editGui)
	{
		switch (tab->getIndexSelected())
		{	case TAB_Sun:    /*if (sub->getIndexSelected()==0)*/ {  gui->btnPickSky(0);  return true;  }  break;
			case TAB_Layers:  gui->btnPickTex(0);  return true;
			case TAB_Grass:  if (sub->getIndexSelected()==1) {  gui->btnPickGrass(0);  return true;  }  break;
			case TAB_Veget:  if (sub->getIndexSelected()==1) {  gui->btnPickVeget(0);  return true;  }  break;
			case TAB_Road:   if (sub->getIndexSelected()==0) {  gui->btnPickRoad(0);   return true;  }  break;
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


	///  Road keys  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
	if (bRoad)
	{
		if (iSnap > 0 && !alt)
		switch (skey)
		{
			case key(1):  road->AddRoll(-1,angSnap,alt);  break;
			case key(2):  road->AddRoll( 1,angSnap,alt);  break;
			case key(3):  road->AddYaw(-1,angSnap,alt);  break;
			case key(4):  road->AddYaw( 1,angSnap,alt);  break;
		}
		int snap = 0;
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
				else		road->ToggleColumn();  break;

			//  prev,next
			case key(PAGEUP):  case key(KP_9):
				if (shift)  road->SelAddPoint();	road->PrevPoint();  break;
			case key(PAGEDOWN):	case key(KP_3):
				if (shift)  road->SelAddPoint();	road->NextPoint();  break;
				
			//  del
			case key(DELETE):  case key(KP_PERIOD):
			case key(KP_5):
				if (ctrl)	road->DelSel();
				else		road->Delete();  break;

			//  ins
			case key(INSERT):  case key(KP_0):
				if (ctrl && !shift && !alt)
				{	if (road->CopySel())  gui->Status("#{Copy}", 0.6,0.8,1.0);  }
				else if (!ctrl && shift && !alt)	road->Paste();
				else if ( ctrl && shift && !alt)	road->Paste(true);
				else
				{	road->newP.pos.x = road->posHit.x;
					road->newP.pos.z = road->posHit.z;
					if (!scn->sc->ter)
						road->newP.pos.y = road->posHit.y;
					//road->newP.aType = AT_Both;
					road->Insert(shift ? INS_Begin : ctrl ? INS_End : alt ? INS_CurPre : INS_Cur);
				}	break;					  

			case key(0):
				if (ctrl)  {   road->Set1stChk();  break;  }
			case key(EQUALS):
				if (ctrl)  road->ChgWallId(1);
				else       road->ChgMtrId(1);  break;
			case key(9):
			case key(MINUS):
				if (ctrl)  road->ChgWallId(-1);
				else       road->ChgMtrId(-1);  break;

			case key(1):  if (alt)
						if (shift)	road->AngZero();
						else		road->ChgAngType(-1);  break;
			case key(2):  if (alt)	road->ChgAngType(1);  break;

			case key(7):  road->ChgLoopType(shift ? -1 : 1);  break;  
			case key(8): if (shift)	road->ToggleNotReal();
						 else		road->ToggleOnPipe(ctrl);  break;

			case key(5):  snap = shift ? 1 :-1;  break;
			
			case key(U):  AlignTerToRoad();  break;
			
			//  looped  todo: separate finish in Q start ..
			case key(N):  road->isLooped = !road->isLooped;
				road->recalcTangents();  road->Rebuild(true);  break;

			//  more roads  ------
			case key(KP_ENTER):  case key(RETURN):
				if (alt)
				{	//  river
					scn->road->river = !scn->road->river;
					scn->road->Rebuild(true);
				}
				else if (ctrl && shift)
				{	//  del
					auto& r = scn->roads;
					if (r.size() > 1)  // not last
					{
						r[scn->rdCur]->Destroy();
						r.erase(r.begin() + scn->rdCur);
						
						if (scn->rdCur >= r.size())
							scn->rdCur = r.size();
						scn->road = r[scn->rdCur];
				}	}
				else if (ctrl)
				{	//  add new
					SplineRoad* road = new SplineRoad(this);
					int id = scn->roads.size();
					
					road->Setup("sphere.mesh", pSet->road_sphr, scn->terrain, mSceneMgr, mCamera, id);
					road->Rebuild(true);  //road->RebuildRoadInt();
					
					scn->roads.push_back(road);
					scn->road = road;
					scn->rdCur = id;
				}else
				{	//  next
					int id = scn->roads.size();
					scn->rdCur = (scn->rdCur + (shift ? -1 : 1) + id) % id;
					scn->road = scn->roads[scn->rdCur];
					gui->SetGuiRoadFromXml();
				}	break;
		}
		if (snap)
		{	iSnap = (iSnap + (snap < 0 ? -1+ciAngSnapsNum : 1))%ciAngSnapsNum;  angSnap = crAngSnaps[iSnap];	}
	}

	//  ter brush shape
	if (edMode < ED_Road && !alt && !bGuiFocus)
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
	if (edMode < ED_Road && alt && skey >= key(1) && skey <= key(0) && !bMoveCam)
	{
		// TODO
		int id = skey - key(1);
		if (shift)  id += 10;
		SetBrushPreset(id);
	}

	
	//  Fluids  ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
	if (edMode == ED_Fluids)
	{	int fls = scn->sc->fluids.size();
		const std::vector<FluidParams>& dfl = scn->data->fluids->fls;
		switch (skey)
		{
			//  ins
			case key(INSERT):  case key(KP_0):
			if (road && road->bHitTer)
			{
				FluidBox fb;	fb.name = "water blue";
				fb.pos = road->posHit;	fb.rot = Vector3(0.f, 0.f, 0.f);
				fb.size = Vector3(50.f, 20.f, 50.f);	fb.tile = Vector2(0.01f, 0.01f);
				scn->sc->fluids.push_back(fb);
				scn->sc->UpdateFluidsId();
				iFlCur = scn->sc->fluids.size()-1;
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
				if (fls == 1)	scn->sc->fluids.clear();
				else			scn->sc->fluids.erase(scn->sc->fluids.begin() + iFlCur);
				iFlCur = std::max(0, std::min(iFlCur, (int)scn->sc->fluids.size()-1));
				bRecreateFluids = true;
				break;

			//  prev,next type
			case key(9):  case key(MINUS):
			{
				FluidBox& fb = scn->sc->fluids[iFlCur];
				fb.id = (fb.id-1 + dfl.size()) % dfl.size();
				fb.name = dfl[fb.id].name;
				bRecreateFluids = true;
			}	break;
			case key(0):  case key(EQUALS):
			{
				FluidBox& fb = scn->sc->fluids[iFlCur];
				fb.id = (fb.id+1) % dfl.size();
				fb.name = dfl[fb.id].name;
				bRecreateFluids = true;
			}	break;
		}
	}

	//  Objects  | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
	if (edMode == ED_Objects)
	{	int objs = scn->sc->objects.size(), objAll = vObjNames.size();
		switch (skey)
		{
			case key(SPACE):
				iObjCur = -1;  PickObject();  UpdObjPick();  break;
				
			//  prev,next type
			case key(9):  case key(MINUS):   SetObjNewType((iObjTNew-1 + objAll) % objAll);  break;
			case key(0):  case key(EQUALS):  SetObjNewType((iObjTNew+1) % objAll);  break;

				
			//  ins
			case key(INSERT):	case key(KP_0):
			if (ctrl)  // copy selected
			{
				if (!vObjSel.empty())
				{
					vObjCopy.clear();
					for (std::set<int>::iterator it = vObjSel.begin();
						it != vObjSel.end(); ++it)
					{
						vObjCopy.push_back(scn->sc->objects[*it]);
					}
					gui->Status("#{Copy}", 0.6,0.8,1.0);
			}	}
			else if (shift)  // paste copied
			{
				if (!vObjCopy.empty())
				{
					vObjSel.clear();  // unsel
					Object o = objNew;
					for (int i=0; i < vObjCopy.size(); ++i)
					{
						objNew = vObjCopy[i];
						AddNewObj(false);
						vObjSel.insert(scn->sc->objects.size()-1);  // add it to sel
					}
					objNew = o;
					UpdObjSel();  UpdObjPick();
			}	}
			else
			if (scn->road && scn->road->bHitTer)  // insert new
			{
				AddNewObj();
				//iObjCur = scn->sc->objects.size()-1;  // auto select inserted-
				UpdObjPick();
			}	break;

			
			//  sel
			case key(BACKSPACE):
				if (alt)  // select all
					for (int i=0; i < objs; ++i)
						vObjSel.insert(i);
				else
				if (ctrl)  vObjSel.clear();  // unsel all
				else
				if (iObjCur > -1)
					if (vObjSel.find(iObjCur) == vObjSel.end())
						vObjSel.insert(iObjCur);  // add to sel
					else
						vObjSel.erase(iObjCur);  // unselect				

				UpdObjSel();
				break;
		}
		::Object* o = iObjCur == -1 ? &objNew :
					((iObjCur >= 0 && objs > 0 && iObjCur < objs) ? &scn->sc->objects[iObjCur] : 0);
		switch (skey)
		{
			//  first, last
			case key(HOME):  case key(KP_7):
				iObjCur = 0;  UpdObjPick();  break;
			case key(END):  case key(KP_1):
				if (objs > 0)  iObjCur = objs-1;  UpdObjPick();  break;

			//  prev,next
			case key(PAGEUP):  case key(KP_9):
				if (objs > 0) {  iObjCur = (iObjCur-1+objs)%objs;  }  UpdObjPick();  break;
			case key(PAGEDOWN):	case key(KP_3):
				if (objs > 0) {  iObjCur = (iObjCur+1)%objs;       }  UpdObjPick();  break;

			//  del
			case key(DELETE):  case key(KP_PERIOD):
			case key(KP_5):
				if (iObjCur >= 0 && objs > 0)
				{	::Object& o = scn->sc->objects[iObjCur];
					mSceneMgr->destroyEntity(o.ent);
					mSceneMgr->destroySceneNode(o.nd);
					
					if (objs == 1)	scn->sc->objects.clear();
					else			scn->sc->objects.erase(scn->sc->objects.begin() + iObjCur);
					iObjCur = std::min(iObjCur, (int)scn->sc->objects.size()-1);
					UpdObjPick();
				}	break;

			//  move,rot,scale
			case key(1):
				if (!shift)  objEd = EO_Move;
				else if (o)
				{
					if (iObjCur == -1)  // reset h
					{
						o->pos[2] = 0.f;  o->SetFromBlt();  UpdObjPick();
					}
					else if (road)  // move to ter
					{
						const Vector3& v = road->posHit;
						o->pos[0] = v.x;  o->pos[1] =-v.z;  o->pos[2] = v.y + objNew.pos[2];
						o->SetFromBlt();  UpdObjPick();
					}
				}	break;

			case key(2):
				if (!shift)  objEd = EO_Rotate;
				else if (o)  // reset rot
				{
					o->rot = QUATERNION<float>(0,1,0,0);
					o->SetFromBlt();  UpdObjPick();
				}	break;

			case key(3):
				if (!shift)  objEd = EO_Scale;
				else if (o)  // reset scale
				{
					o->scale = Vector3::UNIT_SCALE * (ctrl ? 0.5f : 1.f);
					o->nd->setScale(o->scale);  UpdObjPick();
				}	break;
		}
	}

	//  Emitters  : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : :
	if (edMode == ED_Particles && !bGuiFocus)
	{	int emts = scn->sc->emitters.size();
		switch (skey)
		{
			//  prev,next
			case key(PAGEUP):  case key(KP_9):
				if (emts > 0) {  iEmtCur = (iEmtCur-1+emts)%emts;  }  UpdEmtBox();  break;
			case key(PAGEDOWN):	case key(KP_3):
				if (emts > 0) {  iEmtCur = (iEmtCur+1)%emts;       }  UpdEmtBox();  break;

			case key(1):  emtEd = EO_Move;    break;
			case key(2):  emtEd = EO_Rotate;  break;
			case key(3):  emtEd = EO_Scale;   break;

			//  prev,next type
			case key(9):  case key(MINUS):   SetEmtType(-1);  bRecreateEmitters = true;  break;
			case key(0):  case key(EQUALS):  SetEmtType( 1);  bRecreateEmitters = true;  break;

			case key(KP_ENTER):  case key(RETURN):
				scn->sc->emitters[iEmtCur].stat = !scn->sc->emitters[iEmtCur].stat;
				bRecreateEmitters = true;  break;

			//  ins
			case key(INSERT):  case key(KP_0):
			if (road && road->bHitTer)
			{
				SEmitter em;  em.name = "AcidVapor1";
				em.pos = road->posHit;  em.size = Vector3(2.f, 1.f, 2.f);  em.rate = 10.f;
				scn->sc->emitters.push_back(em);  iEmtCur = scn->sc->emitters.size()-1;
				bRecreateEmitters = true;
			}	break;

			//  del
			case key(DELETE):  case key(KP_PERIOD):
			case key(KP_5):
				if (emts == 1)	scn->sc->emitters.clear();
				else			scn->sc->emitters.erase(scn->sc->emitters.begin() + iEmtCur);
				iEmtCur = std::max(0, std::min(iEmtCur, (int)scn->sc->emitters.size()-1));
				bRecreateEmitters = true;
				break;
		}
	}

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

		case key(C):  gui->GuiShortcut(WND_Options, 1);		return true;  // C Screen
		case key(A):  gui->GuiShortcut(WND_Options, 2);		return true;  // A Graphics

		case key(E):  gui->GuiShortcut(WND_Options, 3);     return true;  // E View /Settings
		case key(K):  gui->GuiShortcut(WND_Options, 3,3);   return true;  // K -Tweak
		
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

		//  roads, rivers
		case key(R):  if (bEdit()){  SetEdMode(ED_Road);	UpdEditWnds();  }	break;
		case key(B):  if (road)  {  road->UpdPointsH();  road->Rebuild(true);  }  break;
		case key(T):  if (edMode == ED_Road && mWndRoadStats)
						mWndRoadStats->setVisible(!mWndRoadStats->getVisible());  break;
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
		case key(C):  if (edMode == ED_Objects)  {  objSim = !objSim;  ToggleObjSim();  }  break;
		case key(X):  if (bEdit()){  SetEdMode(ED_Objects);  UpdEditWnds();  }   break;

		//  particles
		case key(A):  if (bEdit()){  SetEdMode(ED_Particles);  UpdEditWnds();  }   break;
	}

	return true;
}
