#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include "../paged-geom/PagedGeometry.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/MultiList2.h"
//#include "../ogre/common/MaterialGen/MaterialFactory.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
using namespace Ogre;


//  Update  input, info
//---------------------------------------------------------------------------------------------------------------
bool App::frameRenderingQueued(const FrameEvent& evt)
{
	if (!BaseApp::frameRenderingQueued(evt))
		return false;

	//  pos on minimap *
	if (ndPos)
	{
		Real x = (0.5 - mCamera->getPosition().z / sc.td.fTerWorldSize);
		Real y = (0.5 + mCamera->getPosition().x / sc.td.fTerWorldSize);
		ndPos->setPosition(xm1+(xm2-xm1)*x, ym1+(ym2-ym1)*y, 0);
		//--------------------------------
		float angrot = mCamera->getOrientation().getYaw().valueDegrees();
		float psx = 0.9f * pSet->size_minimap, psy = psx*asp;  // *par len

		const static float d2r = PI_d/180.f;
		static float px[4],py[4];
		for (int i=0; i<4; i++)
		{
			float ia = 135.f + float(i)*90.f;
			float p = -(angrot + ia) * d2r;
			px[i] = psx*cosf(p);  py[i] =-psy*sinf(p);
		}
		if (mpos)  {	mpos->beginUpdate(0);
			mpos->position(px[0],py[0], 0);  mpos->textureCoord(0, 1);	mpos->position(px[1],py[1], 0);  mpos->textureCoord(1, 1);
			mpos->position(px[3],py[3], 0);  mpos->textureCoord(0, 0);	mpos->position(px[2],py[2], 0);  mpos->textureCoord(1, 0);
			mpos->end();  }
	}
	
	//  status overlay
	if (fStFade > 0.f)
	{	fStFade -= evt.timeSinceLastFrame;
		//Real a = std::min(1.0f, fStFade*0.9f);
		//ColourValue cv(0.0,0.5,a, a );
		//ovStat->setColour(cv);	ovSt->setColour(cv);
		if (fStFade <= 0.f)
		{	ovSt->hide();	ovSt->setMaterialName("");  }
	}

	#define isKey(a)  mKeyboard->isKeyDown(OIS::KC_##a)
	const Real q = (shift ? 0.05 : ctrl ? 4.0 :1.0) * 20 * evt.timeSinceLastFrame;


	// key,mb info  ==================
	if (mStatsOn)
	{
		using namespace OIS;
		const int Kmax = KC_DELETE;  // last key
		static float tkey[Kmax+1] = {0.f,};  // key delay time
		int i;
		static bool first=true;
		if (first)
		{	first=false;
			for (i=Kmax; i > 0; --i)  tkey[i] = 0.f;
		}
		String ss = "";
		//  mouse buttons
		if (mbLeft)  ss += "LMB ";
		if (mbRight)  ss += "RMB ";
		if (mbMiddle)  ss += "MMB ";

		//  pressed
		for (i=Kmax; i > 0; --i)
			if (mKeyboard->isKeyDown( (KeyCode)i ))
				tkey[i] = 0.2f;  // min time to display

		//  modif
		if (tkey[KC_LCONTROL] > 0.f || tkey[KC_RCONTROL] > 0.f)	ss += "Ctrl ";
		if (tkey[KC_LMENU] > 0.f || tkey[KC_RMENU] > 0.f)		ss += "Alt ";
		if (tkey[KC_LSHIFT] > 0.f || tkey[KC_RSHIFT] > 0.f)		ss += "Shift ";
		//  all
		for (i=Kmax; i > 0; --i)
		{
			if (tkey[i] > 0.f)
			{	tkey[i] -= evt.timeSinceLastFrame;  //dec time
				KeyCode k = (KeyCode)i;
	
				 if (k == KC_LSHIFT || k == KC_RSHIFT ||
					 k == KC_LCONTROL || k == KC_RCONTROL ||
					 k == KC_LMENU || k == KC_RMENU)		{	}
			else if (k == KC_DIVIDE)	ss += "Num / ";		else if (k == KC_MULTIPLY)	ss += "Num * ";
			else if (k == KC_ADD)		ss += "Num + ";		else if (k == KC_SUBTRACT)	ss += "Num - ";
			else
			{		 if (k == KC_NUMPAD0)  k = KC_INSERT;	else if (k == KC_DECIMAL)  k = KC_DELETE;
				else if (k == KC_NUMPAD1)  k = KC_END;		else if (k == KC_NUMPAD2)  k = KC_DOWN;
				else if (k == KC_NUMPAD3)  k = KC_PGDOWN;	else if (k == KC_NUMPAD4)  k = KC_LEFT;
				else if (k == KC_NUMPAD5)  k = KC_DELETE;	else if (k == KC_NUMPAD6)  k = KC_RIGHT;
				else if (k == KC_NUMPAD7)  k = KC_HOME;		else if (k == KC_NUMPAD8)  k = KC_UP;
				else if (k == KC_NUMPAD9)  k = KC_PGUP;
					ss += mKeyboard->getAsString(k) + " ";
		}	}	}
		
		//  mouse wheel
		static int mzd = 0;
		if (mz > 0)  mzd = 30;
		if (mz < 0)  mzd = -30;
		if (mzd > 0)  {  ss += "Wheel up";  --mzd;  }
		if (mzd < 0)  {  ss += "Wheel dn";  ++mzd;  }
		//ovInfo->setCaption(ss);
		ovDbg->setCaption(ss);
	}


	//  keys up/dn - trklist
	static float dirU = 0.f,dirD = 0.f;
	if (bGuiFocus)
	{	if (isKey(UP)  ||isKey(NUMPAD8))	dirD += evt.timeSinceLastFrame;  else
		if (isKey(DOWN)||isKey(NUMPAD2))	dirU += evt.timeSinceLastFrame;  else
		{	dirU = 0.f;  dirD = 0.f;  }
		int d = ctrl ? 4 : 1;
		if (dirU > 0.0f) {  trkListNext( d);  dirU = -0.12f;  }
		if (dirD > 0.0f) {  trkListNext(-d);  dirD = -0.12f;  }
	}

	
	///  Update Info texts  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

	//  Road Point
	//----------------------------------------------------------------
	if (edMode == ED_Road && bEdit() && road)
	{
		int ic = road->iChosen;  bool bCur = ic >= 0;
		SplinePoint& sp = bCur ? road->getPoint(ic) : road->newP;
		std::string s;

		static bool first = true;
		if (first)  // once, static text
		{	first = false;
															rdKey[0]->setCaption("Home");
			rdTxt[1]->setCaption(TR("#{Road_Width}"));		rdKey[1]->setCaption("/ *");
			rdTxt[2]->setCaption(TR("#{Road_Yaw}"));		rdKey[2]->setCaption("1 2");
			rdTxt[3]->setCaption(TR("#{Road_Roll}"));		rdKey[3]->setCaption("3 4");
															rdKey[4]->setCaption("5 6");
			rdTxt[5]->setCaption(TR("#{Road_Snap}"));		rdKey[5]->setCaption("7 8");
			rdTxt[6]->setCaption(TR("#{Road_Pipe}"));		rdKey[6]->setCaption("[ ]");
			rdTxt[7]->setCaption(TR("#{Road_Column}"));		rdKey[7]->setCaption("End");
															rdKey[8]->setCaption("- =");
			rdTxt[9]->setCaption(TR("#{Road_ChkR}"));		rdKey[9]->setCaption("K L");
		}

		rdTxt[0]->setCaption(TR(sp.onTer ? "#{Road_OnTerrain}" : "#{Road_Height}"));
		rdVal[0]->setCaption(sp.onTer ? "" : fToStr(sp.pos.y,1,3));

		rdVal[1]->setCaption(fToStr(sp.width,2,4));
		rdVal[2]->setCaption(fToStr(sp.aYaw,1,3));
		rdVal[3]->setCaption(fToStr(sp.aRoll,1,3));
		rdTxt[4]->setCaption(toStr(sp.aType)+" "+TR("#{Road_Angle"+csAngType[sp.aType]+"}"));
		rdVal[5]->setCaption(fToStr(angSnap,0,1));
		rdVal[6]->setCaption(sp.pipe==0.f ? "" : fToStr(sp.pipe,2,4));
		
		rdTxt[7]->setVisible(!sp.onTer);	rdKey[7]->setVisible(!sp.onTer);
		rdVal[7]->setCaption(sp.onTer ? "" : toStr(sp.cols));
		
		rdTxt[8]->setCaption(toStr(sp.idMtr)+" "+road->getMtrStr(ic));
		rdVal[9]->setCaption( sp.chkR == 0.f ? "" : fToStr(sp.chkR,1,3)+"  "+ (road->iP1 == ic ? "#8080FF(1)":"") );

		if (road->vSel.size() > 0)  s = TR("#{Road_sel}")+": "+toStr(road->vSel.size());
		else  s = fToStr(road->iChosen+1,0,2)+"/"+toStr(road->vSegs.size());
		rdVal[10]->setCaption(s);

		rdTxt[10]->setCaption(TR(bCur ? "#{Road_Cur}" : "#{Road_New}"));
		rdTxt[10]->setTextColour(bCur ? MyGUI::Colour(0.85,0.75,1) : MyGUI::Colour(0.3,1,0.1));

		rdKey[10]->setCaption(road->bMerge ? "Mrg":"");


		//  road stats  --------------------------------
		if (mWndRoadStats && mWndRoadStats->getVisible())
		{
			static bool first = true;
			if (first)  // once, static text
			{	first = false;
				rdTxtSt[0]->setCaption(TR("#{Road_Length}"));
				rdTxtSt[1]->setCaption(TR("#{Road_Width}"));
				rdTxtSt[2]->setCaption(TR("#{Road_Height}"));

				rdTxtSt[3]->setCaption(TR("#{TrackInAir}"));
				rdTxtSt[4]->setCaption(TR("#{TrackPipes}"));

				rdTxtSt[5]->setCaption("lod pnt");
				rdTxtSt[6]->setCaption("segs Mrg");
				rdTxtSt[7]->setCaption("vis");
				rdTxtSt[8]->setCaption("tri");
			}
			
			rdValSt[0]->setCaption(fToStr(road->st.Length,0,4));
			rdValSt[1]->setCaption(fToStr(road->st.WidthAvg,2,5));
			rdValSt[2]->setCaption(fToStr(road->st.HeightDiff,2,5));

			rdValSt[3]->setCaption(fToStr(road->st.OnTer,1,4)+"%");
			rdValSt[4]->setCaption(fToStr(road->st.Pipes,1,4)+"%");

			int lp = !bCur ? -1 : road->vSegs[road->iChosen].lpos.size();
			rdValSt[5]->setCaption(toStr(lp));
			rdValSt[6]->setCaption(fToStr(road->segsMrg+1,0,2));
			rdValSt[7]->setCaption(fToStr(road->iVis,0,2));
			rdValSt[8]->setCaption(fToStr(road->iTris/1000.f,1,4)+"k");
		}

		//  edit  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
		Vector3 vx = mCamera->getRight();	   vx.y = 0;  vx.normalise();  // on xz
		Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
		Vector3 vy = Vector3::UNIT_Y;

		if (isKey(LEFT)||isKey(NUMPAD4))  road->Move(-vx*q);	if (isKey(RIGHT)||isKey(NUMPAD6))	road->Move( vx*q);
		if (isKey(UP)||isKey(NUMPAD8))	  road->Move( vz*q);	if (isKey(DOWN)||isKey(NUMPAD2))	road->Move(-vz*q);

		if (isKey(SUBTRACT))	road->Move(-vy*q);			if (isKey(ADD))		road->Move( vy*q);
		if (isKey(MULTIPLY))	road->AddWidth( q*0.4f);	if (isKey(DIVIDE))	road->AddWidth(-q*0.4f);

		if (iSnap == 0)
		{	if (isKey(1))		road->AddYaw(-q*3,0,alt);	if (isKey(3))		road->AddRoll(-q*3,0,alt);
			if (isKey(2))		road->AddYaw( q*3,0,alt);	if (isKey(4))		road->AddRoll( q*3,0,alt);
		}
		if (isKey(LBRACKET))	road->AddPipe(-q*0.2);	if (isKey(K ))	road->AddChkR(-q*0.2);  // chk
		if (isKey(RBRACKET))	road->AddPipe( q*0.2);	if (isKey(L))	road->AddChkR( q*0.2);

		if (mz > 0)			road->NextPoint();
		else if (mz < 0)	road->PrevPoint();
	}

	///  Terrain  Brush
	//--------------------------------------------------------------------------------------------------------------------------------
	else if (edMode < ED_Road /*&& bEdit()*/)
	{
		static bool first = true;
		if (first)  // once, static text
		{	first = false;

			brTxt[0]->setCaption(TR("#{Brush_Size}"));		brKey[0]->setCaption("- =");
			brTxt[1]->setCaption(TR("#{Brush_Force}"));		brKey[1]->setCaption("[ ]");
			brTxt[2]->setCaption(TR("#{Brush_Power}"));		brKey[2]->setCaption("; \'");
			brTxt[3]->setCaption(TR("#{Brush_Shape}"));		brKey[3]->setCaption("K L");

			brTxt[4]->setCaption(TR("#{Brush_Freq}"));		brKey[4]->setCaption("O P");
			brTxt[5]->setCaption(TR("#{Brush_Octaves}"));	brKey[5]->setCaption(", .");
			brTxt[6]->setCaption(TR("#{Brush_Offset}"));	brKey[6]->setCaption("9 0");
		}
		brVal[0]->setCaption(fToStr(mBrSize[curBr],1,4));
		brVal[1]->setCaption(fToStr(mBrIntens[curBr],1,4));
		brVal[2]->setCaption(fToStr(mBrPow[curBr],2,4));
		brVal[3]->setCaption(TR("#{Brush_Shape"+csBrShape[mBrShape[curBr]]+"}"));

		bool brN = mBrShape[curBr] == BRS_Noise;  int i;
		for (i=4; i<=6; ++i)
		{	brTxt[i]->setVisible(brN);  brVal[i]->setVisible(brN);  brKey[i]->setVisible(brN);	}

		if (brN)
		{	brVal[4]->setCaption(fToStr(mBrFq[curBr],2,4));
			brVal[5]->setCaption(toStr(mBrOct[curBr]));
			brVal[6]->setCaption(fToStr(mBrNOf[curBr],1,4));
		}

		bool edH = edMode != ED_Height;
		const static MyGUI::Colour clrEF[2] = {MyGUI::Colour(0.7,1.0,0.7), MyGUI::Colour(0.6,0.8,1.0)};
		const MyGUI::Colour& clr = clrEF[!edH ? 0 : 1];
		i=7;
		{	brTxt[i]->setTextColour(clr);	brVal[i]->setTextColour(clr);  brKey[i]->setTextColour(clr);  }

		if (edMode == ED_Filter)
		{
			brTxt[7]->setCaption(TR("#{Brush_Filter}"));	brKey[7]->setCaption("  1 2");
			brVal[7]->setCaption(fToStr(mBrFilt,1,3));
		}
		else if (!edH && road && road->bHitTer)
		{
			brTxt[7]->setCaption(TR("#{Brush_Height}"));	brKey[7]->setCaption("RMB");
			brVal[7]->setCaption(fToStr(terSetH,1,4));
		}else
		{	brTxt[7]->setCaption("");  brVal[7]->setCaption("");  brKey[7]->setCaption("");  }
		
		brTxt[8]->setCaption(edH ? "" : TR("#{Brush_CurrentH}"));
		brVal[8]->setCaption(edH ? "" : fToStr(road->posHit.y,1,4));
		brKey[8]->setCaption(edH ? "" : "");
		
		
		//  edit  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
		if (mz != 0)
			if (alt){			mBrPow[curBr]   *= 1.f - 0.4f*q*mz;  updBrush();  }
			else if (!shift){	mBrSize[curBr]  *= 1.f - 0.4f*q*mz;  updBrush();  }
			else				mBrIntens[curBr]*= 1.f - 0.4f*q*mz/0.05;

		if (isKey(MINUS)){		mBrSize[curBr]  *= 1.f - 0.04f*q;  updBrush();  }
		if (isKey(EQUALS)){		mBrSize[curBr]  *= 1.f + 0.04f*q;  updBrush();  }
		if (isKey(LBRACKET))	mBrIntens[curBr]*= 1.f - 0.04f*q;
		if (isKey(RBRACKET))	mBrIntens[curBr]*= 1.f + 0.04f*q;
		if (isKey(SEMICOLON )){ mBrPow[curBr]   *= 1.f - 0.04f*q;  updBrush();  }
		if (isKey(APOSTROPHE)){ mBrPow[curBr]   *= 1.f + 0.04f*q;  updBrush();  }

		if (isKey(O)){			mBrFq[curBr]    *= 1.f - 0.04f*q;  updBrush();  }
		if (isKey(P)){			mBrFq[curBr]    *= 1.f + 0.04f*q;  updBrush();  }
		if (isKey(9)){			mBrNOf[curBr]   -= 0.3f*q;		   updBrush();  }
		if (isKey(0)){			mBrNOf[curBr]   += 0.3f*q;		   updBrush();  }

		if (isKey(1)){			mBrFilt         *= 1.f - 0.04f*q;  updBrush();  }
		if (isKey(2)){			mBrFilt         *= 1.f + 0.04f*q;  updBrush();  }
		
		if (mBrIntens[curBr] < 0.1f)  mBrIntens[curBr] = 0.1;  // rest in updBrush
	}

	///  Start  box, road dir
	//----------------------------------------------------------------
	else if (edMode == ED_Start && road)
	{
		Vector3 p;  if (ndCar)  p = ndCar->getPosition();
		stTxt[0]->setCaption("");
		stTxt[1]->setCaption("width "+fToStr(road->vStBoxDim.z,1,4));
		stTxt[2]->setCaption("height "+fToStr(road->vStBoxDim.y,1,4));
		stTxt[3]->setCaption("road dir "+ (road->iDir == 1 ? String("+1") : String("-1")) );

		//  edit
		if (isKey(LBRACKET))	{  road->AddBoxH(-q*0.2);  UpdStartPos();  }
		if (isKey(SEMICOLON))	{  road->AddBoxW(-q*0.2);  UpdStartPos();  }
		if (isKey(RBRACKET))	{  road->AddBoxH( q*0.2);  UpdStartPos();  }
		if (isKey(APOSTROPHE))	{  road->AddBoxW( q*0.2);  UpdStartPos();  }
		//if (mz > 0)	// snap rot by 15 deg ..
	}
	///  Fluids
	//----------------------------------------------------------------
	else if (edMode == ED_Fluids)
	{
		if (sc.fluids.empty())
		{
			if (flTxt[0])	flTxt[0]->setCaption("None");
			for (int i=1; i < FL_TXT; ++i)
				if (flTxt[i])  flTxt[i]->setCaption("");
		}else
		{	FluidBox& fb = sc.fluids[iFlCur];
			flTxt[0]->setCaption("Cur/All:  "+toStr(iFlCur+1)+" / "+toStr(sc.fluids.size()));
			flTxt[1]->setCaption(fb.name);
			flTxt[2]->setCaption("Pos:  "+fToStr(fb.pos.x,1,4)+" "+fToStr(fb.pos.y,1,4)+" "+fToStr(fb.pos.z,1,4));
			flTxt[3]->setCaption(""/*"Rot:  "+fToStr(fb.rot.x,1,4)*/);
			flTxt[3]->setCaption("Size:  "+fToStr(fb.size.x,1,4)+" "+fToStr(fb.size.y,1,4)+" "+fToStr(fb.size.z,1,4));
			flTxt[4]->setCaption("Tile:  "+fToStr(fb.tile.x,3,5)+" "+fToStr(fb.tile.y,3,5));

			//  edit
			if (isKey(LBRACKET)){	fb.tile   *= 1.f - 0.04f*q;  bRecreateFluids = true;  }
			if (isKey(RBRACKET)){	fb.tile   *= 1.f + 0.04f*q;  bRecreateFluids = true;  }
			if (isKey(SEMICOLON )){	fb.tile.y *= 1.f - 0.04f*q;  bRecreateFluids = true;  }
			if (isKey(APOSTROPHE)){	fb.tile.y *= 1.f + 0.04f*q;  bRecreateFluids = true;  }

			if (mz != 0)  // wheel prev/next
			{	int fls = sc.fluids.size();
				if (fls > 0)  {  iFlCur = (iFlCur-mz+fls)%fls;  UpdFluidBox();  }
			}
		}
	}
	///  Objects
	//----------------------------------------------------------------
	else if (edMode == ED_Objects)
	{
		if (iObjCur == -1 || sc.objects.empty())
		{	//  none sel
			objTxt[0]->setCaption("#20FF20New#C0C0C0    "+toStr(iObjCur)+" / "+toStr(sc.objects.size()));
			objTxt[1]->setCaption(vObjNames[iObjTNew]);  // all new params ...
			objTxt[3]->setCaption("");
			objTxt[4]->setCaption("");
			objTxt[5]->setCaption("");
		}else
		{	const Object& o = sc.objects[iObjCur];
			if (vObjSel.empty())
				objTxt[0]->setCaption("#A0D0FFCur#C0C0C0     "+toStr(iObjCur+1)+" / "+toStr(sc.objects.size()));
			else
				objTxt[0]->setCaption("#00FFFFSel#C0C0C0     "+toStr(vObjSel.size())+" / "+toStr(sc.objects.size()));
			objTxt[1]->setCaption(o.name);
			objTxt[3]->setCaption("Pos:  "+fToStr(o.pos[0],1,4)+" "+fToStr(o.pos[2],1,4)+" "+fToStr(-o.pos[1],1,4));
			objTxt[4]->setCaption("Rot:  "+fToStr(o.nd->getOrientation().getYaw().valueDegrees(),1,4));
			objTxt[5]->setCaption("Scale:  "+fToStr(o.scale.x,2,4)+" "+fToStr(o.scale.y,2,4)+" "+fToStr(o.scale.z,2,4));
		}
		//  edit
		if (mz != 0)  // wheel prev/next
		{	int objs = sc.objects.size();
			if (objs > 0)  {  iObjCur = (iObjCur-mz+objs)%objs;  UpdObjPick();  }
		}
	}
	mz = 0;  // mouse wheel

	
	//  rebuild road after end of selection change
	static bool bSelChngOld = false;
	if (road)
	{
		road->fLodBias = pSet->road_dist;  // after rebuild

		if (bSelChngOld && !road->bSelChng)
			road->RebuildRoad(true);

		bSelChngOld = road->bSelChng;
		road->bSelChng = false;
	}

	///  upd road lods
	static int dti = 5, ti = dti-1;  ++ti;
	if (road && ti >= dti)
	{	ti = 0;
		road->UpdLodVis(pSet->road_dist, edMode == ED_PrvCam);
	}/**/
	return true;
}

//---------------------------------------------------------------------------------------------------------------
///  Mouse
//---------------------------------------------------------------------------------------------------------------
void App::processMouse()  //! from Thread, cam vars only
{
	//double m_interval = timer.iv;
	//static double m_sumTime = 0.0;
	//m_sumTime += mDTime;  int num = 0;
	//while (m_sumTime > m_interval)
	//if (!alt)
	{
		//num++;
		//m_sumTime -= m_interval;
		Real fDT = timer.iv;  //mDTime;
		
		//  static vars are smoothed
		Vector3 vInpC(0,0,0),vInp;  //static Vector3 vNew(0,0,0);
		Real fSmooth = (powf(1.0f - pSet->cam_inert, 2.2f) * 40.f + 0.1f) * fDT;
		
		const Real sens = 0.13;
		if (bCam())
			vInpC = Vector3(mx, my, 0)*sens;
		vInp = Vector3(mx, my, 0)*sens;  mx = 0;  my = 0;
		vNew += (vInp-vNew) * fSmooth;
		//vNew = vInp;
		
		if (mbMiddle){	mTrans.z += vInpC.y * 1.6f;  }  //zoom
		if (mbRight){	mTrans.x += vInpC.x;  mTrans.y -= vInpC.y;  }  //pan
		if (mbLeft){	mRotX -= vInpC.x;  mRotY -= vInpC.y;  }  //rot
		//mTrans.z -= vInp.z;	//scroll

		//  move camera	//if (bCam())
		{
			Real cs = pSet->cam_speed;  Degree cr(pSet->cam_speed);
			Real fMove = 100*cs;  //par speed
			Degree fRot = 500*cr, fkRot = 80*cr;
		
			static Radian sYaw(0), sPth(0);
			static Vector3 sMove(0,0,0);

			Radian inYaw = rotMul * fDT * (fRot* mRotX + fkRot* mRotKX);
			Radian inPth = rotMul * fDT * (fRot* mRotY + fkRot* mRotKY);
			Vector3 inMove = moveMul * fDT * (fMove * mTrans);

			sYaw += (inYaw - sYaw) * fSmooth;
			//sYaw = inYaw;
			sPth += (inPth - sPth) * fSmooth;
			//sPth = inPth;
			sMove += (inMove - sMove) * fSmooth;
			//sMove = inMove;

			//if (abs(sYaw.valueRadians()) > 0.000001f)
				mCameraT->yaw( sYaw );
			//if (abs(sYaw.valueRadians()) > 0.000001f)
				mCameraT->pitch( sPth );
			//if (sMove.squaredLength() > 0.000001f)
				mCameraT->moveRelative( sMove );
		}
	}
	//LogO("dt: " + toStr((float)mDTime) + "  n.iv's: " + toStr(num));
}

void App::editMouse()
{
	if (!bEdit())  return;
	
	///  mouse edit Road  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	if (road && edMode == ED_Road)
	{
		const Real fMove(5.0f), fRot(40.f);  //par speed

		if (!alt)
		{
			if (mbLeft)    // move on xz
			{	Vector3 vx = mCameraT->getRight();	   vx.y = 0;  vx.normalise();
				Vector3 vz = mCameraT->getDirection();  vz.y = 0;  vz.normalise();
				road->Move((vNew.x * vx - vNew.y * vz) * fMove * moveMul);
			}else
			if (mbRight)   // height
				road->Move(-vNew.y * Vector3::UNIT_Y * fMove * moveMul);
			else
			if (mbMiddle)  // width
				road->AddWidth(vNew.x * fMove * moveMul);
		}else
		{	//  alt
			if (mbLeft)    // rot pitch
				road->AddYaw(   vNew.x * fRot * moveMul,0.f,false/*alt*/);
			if (mbRight)   // rot yaw
				road->AddRoll(  vNew.y *-fRot * moveMul,0.f,false/*alt*/);
		}
	}

	///  edit ter height val
	if (edMode == ED_Height)
	{
		if (mbRight)
		{	Real ym = -vNew.y * 0.5f * moveMul;
			terSetH += ym;
		}
	}

	///  edit start pos	 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	if (edMode == ED_Start /*&&
		vStartPos.size() >= 4 && vStartRot.size() >= 4*/)
	{
		const Real fMove(0.5f), fRot(0.05f);  //par speed
		const int n = 0;  // 1st entry - all same / edit 4..
		if (!alt)
		{
			if (mbLeft)
			{
				if (ctrl)  // set pos from ter hit point
				{
					if (road && road->bHitTer)
					{
						Vector3 v = road->posHit;
						vStartPos[n][0] = v.x;  vStartPos[n][1] =-v.z;
						vStartPos[n][2] = v.y+0.6f;  //car h above
					}
				}
				else  // move
				{
					Vector3 vx = mCameraT->getRight();	   vx.y = 0;  vx.normalise();
					Vector3 vz = mCameraT->getDirection();  vz.y = 0;  vz.normalise();
					Vector3 vm = (-vNew.y * vx - vNew.x * vz) * fMove * moveMul;
					vStartPos[n][0] += vm.z;
					vStartPos[n][1] += vm.x;
				}
				UpdStartPos();
			}
			else
			if (mbRight)
			{
				Real ym = -vNew.y * fMove * moveMul;
				vStartPos[n][2] += ym;  UpdStartPos();
			}
		}else
		{	//  alt
			typedef QUATERNION<float> Qf;
			if (mbLeft)    // rot pitch
			{
				Qf qr;  qr.Rotate(vNew.x * fRot * moveMul, 0,0,1);
				Qf& q = vStartRot[n];  // get yaw angle, add ..
				q = q * qr;  UpdStartPos();
			}else
			if (mbRight)   // rot yaw
			{
				Qf qr;  qr.Rotate(vNew.y *-fRot * moveMul, 0,1,0);
				Qf& q = vStartRot[n];
				q = q * qr;  UpdStartPos();
			}else
			if (mbMiddle)  // rot reset
			{
				Qf qr;  qr.Rotate(0, 0,0,1);
				Qf& q = vStartRot[n];	q = qr;  UpdStartPos();
			}
		}
	}

	///  edit fluids . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	if (edMode == ED_Fluids && !sc.fluids.empty())
	{
		FluidBox& fb = sc.fluids[iFlCur];
		const Real fMove(0.5f), fRot(1.5f);  //par speed
		if (!alt)
		{
			if (mbLeft)	// move on xz
			{
				Vector3 vx = mCameraT->getRight();	   vx.y = 0;  vx.normalise();
				Vector3 vz = mCameraT->getDirection();  vz.y = 0;  vz.normalise();
				Vector3 vm = (-vNew.y * vz + vNew.x * vx) * fMove * moveMul;
				fb.pos += vm;
				vFlNd[iFlCur]->setPosition(fb.pos);  UpdFluidBox();
			}else
			if (mbRight)  // move y
			{
				Real ym = -vNew.y * fMove * moveMul;
				fb.pos.y += ym;
				vFlNd[iFlCur]->setPosition(fb.pos);  UpdFluidBox();
			}
			// rot not supported (bullet trigger isnt working, trees check & waterDepth is a lot simpler)
			/*else
			if (mbMiddle)  // rot yaw
			{
				Real xm = vNew.x * fRot * moveMul;
				fb.rot.x += xm;
				vFlNd[iFlCur]->setOrientation(Quaternion(Degree(fb.rot.x),Vector3::UNIT_Y));
			}/**/
		}else
		{
			if (mbLeft)  // size xz
			{
				Vector3 vm = Vector3(vNew.y, 0, vNew.x) * fMove * moveMul;
				fb.size += vm;
				if (fb.size.x < 0.2f)  fb.size.x = 0.2f;
				if (fb.size.z < 0.2f)  fb.size.z = 0.2f;
				bRecreateFluids = true;  //
			}else
			if (mbRight)  // size y
			{
				float vm = -vNew.y * fMove * moveMul;
				fb.size.y += vm;
				if (fb.size.y < 0.2f)  fb.size.y = 0.2f;
				bRecreateFluids = true;  //
			}
		}
	}

	///  edit objects . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	if (edMode == ED_Objects && !sc.objects.empty() && (iObjCur >= 0 || !vObjSel.empty()))
	{
		const Real fMove(0.5f), fRot(1.5f), fScale(0.02f);  //par speed
		bool upd = false, sel = !vObjSel.empty();
		//  selection or picked
		std::set<int>::iterator it = vObjSel.begin();
		int i = sel ? *it : iObjCur;
		while (i >= 0 && i < sc.objects.size())
		{
			Object& o = sc.objects[i];
			if (!alt)
			{
				if (mbLeft)	// move on xz
				{
					Vector3 vx = mCameraT->getRight();	   vx.y = 0;  vx.normalise();
					Vector3 vz = mCameraT->getDirection();  vz.y = 0;  vz.normalise();
					Vector3 vm = (-vNew.y * vz + vNew.x * vx) * fMove * moveMul;
					o.pos[0] += vm.x;  o.pos[1] -= vm.z;  // todo: for selection ..
					o.SetFromBlt();	 upd = true;
				}else
				if (mbRight)  // move y
				{
					Real ym = -vNew.y * fMove * moveMul;
					o.pos[2] += ym;
					o.SetFromBlt();	 upd = true;
				}
				else
				if (mbMiddle)  // rot yaw,  ctrl pitch local-
				{
					Real xm = vNew.x * fRot * moveMul *PI_d/180.f;
					QUATERNION <float> qr;
					if (!ctrl)  qr.Rotate(-xm, 0, 0, 1);  else  qr.Rotate(-xm, 0, 1, 0);
					o.rot = o.rot * qr;
					o.SetFromBlt();	 upd = true;
				}
			}else
			{
				if (mbLeft)  // size xz
				{
					//Vector3 vm = Vector3(vNew.y, 0, vNew.x) * fMove * moveMul;
					float vm = (vNew.y - vNew.x) * fMove * moveMul;
					o.scale *= 1.f - vm * fScale;
					//if (o.scale.x < 0.02f)  o.scale.x = 0.02f;
					o.nd->setScale(o.scale);  upd = true;
				}else
				if (mbRight)  // scale y
				{
					float vm = (vNew.y - vNew.x) * fMove * moveMul;
					o.scale.y *= 1.f - vm * fScale;
					//if (o.scale.y < 0.02f)  o.scale.y = 0.02f;
					o.nd->setScale(o.scale);  upd = true;
				}
			}
			if (sel)
			{	++it;  // next sel
				if (it == vObjSel.end())  break;
				i = *it;
			}else  break;
		}
		if (upd)
			UpdObjPick();
	}
}


//---------------------------------------------------------------------------------------------------------------
//  frame events
//---------------------------------------------------------------------------------------------------------------
bool App::frameEnded(const FrameEvent& evt)
{
	//  track events
	if (eTrkEvent != TE_None)
	{	switch (eTrkEvent)  {
			case TE_Load:	LoadTrackEv();  break;
			case TE_Save:	SaveTrackEv();  break;
			case TE_Update: UpdateTrackEv();  break;  }
		eTrkEvent = TE_None;
	}
	
	///  input event queues  ------------------------------------
	for (int i=0; i < i_cmdKeyRel; ++i)
	{	const CmdKey& k = cmdKeyRel[i];
		MyGUI::InputManager::getInstance().injectKeyRelease(MyGUI::KeyCode::Enum(k.key));  }
	i_cmdKeyRel = 0;

	for (int i=0; i < i_cmdKeyPress; ++i)
	{	const CmdKey& k = cmdKeyPress[i];
		KeyPress(k);  }
	i_cmdKeyPress = 0;

	for (int i=0; i < i_cmdMouseMove; ++i)
	{	const CmdMouseMove& c = cmdMouseMove[i];
		MyGUI::InputManager::getInstance().injectMouseMove(c.ms.X.abs, c.ms.Y.abs, c.ms.Z.abs);  }
	i_cmdMouseMove = 0;

	for (int i=0; i < i_cmdMousePress; ++i)
	{	const CmdMouseBtn& b = cmdMousePress[i];
		MyGUI::InputManager::getInstance().injectMousePress(b.ms.X.abs, b.ms.Y.abs, MyGUI::MouseButton::Enum(b.btn));
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		SetCursor(0);  //?- cursor after alt-tab
		ShowCursor(0); 
		#endif
	}
	i_cmdMousePress = 0;

	for (int i=0; i < i_cmdMouseRel; ++i)
	{	const CmdMouseBtn& b = cmdMouseRel[i];
		MyGUI::InputManager::getInstance().injectMouseRelease(b.ms.X.abs, b.ms.Y.abs, MyGUI::MouseButton::Enum(b.btn));  }
	i_cmdMouseRel = 0;
	

	//  road pick
	if (road)
	{
		const MyGUI::IntPoint& mp = MyGUI::InputManager::getInstance().getMousePosition();
		Real mx = Real(mp.left)/mWindow->getWidth(), my = Real(mp.top)/mWindow->getHeight();
		road->Pick(mCamera, mx, my,
			edMode == ED_Road,  !(edMode == ED_Road && bEdit()));

		if (sc.vdr)  // blt ray hit
		{
			Ray ray = mCamera->getCameraToViewportRay(mx,my);
			const Vector3& pos = mCamera->getDerivedPosition(), dir = ray.getDirection();
			btVector3 from(pos.x,-pos.z,pos.y), to(dir.x,-dir.z,dir.y);  to = from + to*10000.f;
			btCollisionWorld::ClosestRayResultCallback rayRes(from, to);

			world->rayTest(from, to, rayRes);

			if (rayRes.hasHit())
				road->posHit = Vector3(rayRes.m_hitPointWorld.getX(),rayRes.m_hitPointWorld.getZ(),-rayRes.m_hitPointWorld.getY());
			else
				road->posHit = Vector3::ZERO;
			road->ndHit->setPosition(road->posHit);
		}
	}

	editMouse();  // edit


	///  paged  Upd  * * *
	if (bTrGrUpd)
	{	bTrGrUpd = false;
		pSet->bTrees = !pSet->bTrees;
		UpdTrees();
	}
	///  paged  * * *  ? frameStarted
	if (road)
	{	if (grass)  grass->update();
		if (trees)  trees->update();
	}
	
	///<>  Edit Ter
	TerCircleUpd();
	if (terrain && road && bEdit() && road->bHitTer)
	{
		float dt = evt.timeSinceLastFrame;
		Real s = shift ? 0.25 : ctrl ? 4.0 :1.0;
		switch (edMode)
		{
		case ED_Deform:
			if (mbLeft)   deform(road->posHit, dt, s);  else
			if (mbRight)  deform(road->posHit, dt,-s);
			break;
		case ED_Filter:
			if (mbLeft)   filter(road->posHit, dt, s);
			break;
		case ED_Smooth:
			if (mbLeft)   smooth(road->posHit, dt);
			break;
		case ED_Height:
			if (mbLeft)   height(road->posHit, dt, s);
			break;
		}
	}

	///<>  Ter upd
	static int tu = 0, bu = 0;
	if (tu >= pSet->ter_skip)
	if (bTerUpd)
	{	bTerUpd = false;  tu = 0;
		if (mTerrainGroup)
			mTerrainGroup->update();
	}	tu++;

	if (bu >= pSet->ter_skip)
	if (bTerUpdBlend)
	{	bTerUpdBlend = false;  bu = 0;
		if (terrain)
		{
			GetTerAngles();  // full
			initBlendMaps(terrain);
		}
	}	bu++;

	
	if (road)  // road
	{
		road->bCastShadow = pSet->shadow_type >= 2;
		road->RebuildRoadInt();
	}

	///**  Render Targets update
	if (edMode == ED_PrvCam)
	{
		sc.camPos = mCameraT->getPosition();
		sc.camDir = mCameraT->getDirection();
		if (rt[RTs-1].rndTex)
			rt[RTs-1].rndTex->update();
	}else{
		static int ri = 0;
		if (ri >= pSet->mini_skip)
		{	ri = 0;
			for (int i=0; i < RTs-1/**/; ++i)
				if (rt[i].rndTex)
					rt[i].rndTex->update();
		}	ri++;
	}
	//LogO(toStr(evt.timeSinceLastFrame));

	return true;
}


//---------------------------------------------------------------------------------------------------------------
bool App::frameStarted(const Ogre::FrameEvent& evt)
{
	BaseApp::frameStarted(evt);
	
	if (bGuiReinit)  // after language change from combo
	{	bGuiReinit = false;
		mGUI->destroyWidgets(vwGui);  bnQuit=0;mWndOpts=0;trkMList=0; //todo: rest too..
		InitGui();
		SetGuiFromXmls();
		bWindowResized = true;
		//mWndTabs->setIndexSelected(10);  // switch back to view tab
	}

	if (bWindowResized)
	{	bWindowResized = false;

		ResizeOptWnd();
		//bSizeHUD = true;
		SizeGUI();
		updTrkListDim();
		
		LoadTrack();  // shouldnt be needed but ...
	}
	
	if (bRecreateFluids)
	{	bRecreateFluids = false;
		DestroyFluids();
		CreateFluids();
		UpdFluidBox();
	}
	
	///  sort trk list
	if (trkMList && (trkMList->mSortColumnIndex != trkMList->mSortColumnIndexOld
		|| trkMList->mSortUp != trkMList->mSortUpOld))
	{
		trkMList->mSortColumnIndexOld = trkMList->mSortColumnIndex;
		trkMList->mSortUpOld = trkMList->mSortUp;

		pSet->tracks_sort = trkMList->mSortColumnIndex;  // to set
		pSet->tracks_sortup = trkMList->mSortUp;
		TrackListUpd(false);
	}
	
	//materialFactory->update();
	
	bFirstRenderFrame = false;
	
	return true;
}
