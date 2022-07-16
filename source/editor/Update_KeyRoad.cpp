#include "pch.h"
#include "../ogre/common/CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
using namespace Ogre;


//  Key Press Road
//---------------------------------------------------------------------------------------------------------------

void App::keyPressRoad(SDL_Scancode skey)
{
	#define key(a)  SDL_SCANCODE_##a
	SplineRoad* road = scn->road;
	
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
			{	//  road, river, decor cycle  not RD_ALL
				scn->road->type = (RoadType)((scn->road->type + (shift ? -1 : 1) + RD_Trail) % RD_Trail);
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
						scn->rdCur = r.size()-1;
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
				scn->road->HideMarks();
				int id = scn->roads.size();
				scn->rdCur = (scn->rdCur + (shift ? -1 : 1) + id) % id;
				scn->road = scn->roads[scn->rdCur];
				gui->SetGuiRoadFromXml();
			}	break;
	}
	if (snap)
	{	iSnap = (iSnap + (snap < 0 ? -1 + ciAngSnapsNum : 1)) % ciAngSnapsNum;  angSnap = crAngSnaps[iSnap];	}
}
