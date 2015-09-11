#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include "../sdl4ogre/sdlinputwrapper.hpp"
#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <OgreManualObject.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <MyGUI_InputManager.h>
#include <MyGUI_Widget.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_Window.h>
using namespace Ogre;
using namespace MyGUI;


//  Update  input, info
//---------------------------------------------------------------------------------------------------------------
bool App::frameRenderingQueued(const FrameEvent& evt)
{
	if (!BaseApp::frameRenderingQueued(evt))
		return false;

	//  pos on minimap *
	if (ndPos)
	{	Real w = scn->sc->td.fTerWorldSize;
		Real x = (0.5 - mCamera->getPosition().z / w);
		Real y = (0.5 + mCamera->getPosition().x / w);
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
	
	//  status
	if (fStFade > 0.f)
	{	fStFade -= evt.timeSinceLastFrame;
		
		gui->panStatus->setAlpha(std::min(1.f, fStFade / 1.5f));
		if (fStFade <= 0.f)
			gui->panStatus->setVisible(false);
	}

	#define isKey(a)  mInputWrapper->isKeyDown(SDL_SCANCODE_##a)
	const Real q = (shift ? 0.05 : ctrl ? 4.0 :1.0) * 20 * evt.timeSinceLastFrame;


	// key,mb info  ==================
	if (pSet->inputBar)
	{
		// TODO: This is definitely not bullet-proof.
		const int Kmax = SDL_SCANCODE_SLEEP;  // last key
		static float tkey[Kmax+1] = {0.f,};  // key delay time
		int i;
		static bool first=true;
		if (first)
		{	first=false;
			for (i=Kmax; i > 0; --i)  tkey[i] = 0.f;
		}
		String ss = "   ";
		//  pressed
		for (i=Kmax; i > 0; --i)
			if  (mInputWrapper->isKeyDown(SDL_Scancode(i)))
				tkey[i] = 0.2f;  // min time to display

		//  modif
		const static int
			lc = SDL_SCANCODE_LCTRL,  rc = SDL_SCANCODE_RCTRL,
			la = SDL_SCANCODE_LALT,   ra = SDL_SCANCODE_RALT,
			ls = SDL_SCANCODE_LSHIFT, rs = SDL_SCANCODE_RSHIFT;

		if (tkey[lc] > 0.f || tkey[rc] > 0.f)	ss += "#D0F0D0Ctrl  ";
		if (tkey[la] > 0.f || tkey[ra] > 0.f)	ss += "#D0F0D0Alt  ";
		if (tkey[ls] > 0.f || tkey[rs] > 0.f)	ss += "#D0F0D0Shift  ";

		//  mouse buttons
		if (mbLeft)  ss += "#C0FFFFLMB  ";
		if (mbRight)  ss += "#C0FFFFRMB  ";
		if (mbMiddle)  ss += "#C0FFFFMMB  ";

		//  all
		for (i=Kmax; i > 0; --i)
		{
			if (tkey[i] > 0.f)
			{	tkey[i] -= evt.timeSinceLastFrame;  //dec time
				if (i!=lc && i!=la && i!=ls && i!=rc && i!=ra && i!=rs)
				{
					String s = String(SDL_GetKeyName(SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(i))));
					s = StringUtil::replaceAll(s, "Keypad", "#FFFFC0Num ");
					ss += "#FFFFFF" + s + "  ";
				}
		}	}
		
		//  mouse wheel
		static int mzd = 0;
		if (mz > 0)  mzd = 30;
		if (mz < 0)  mzd = -30;
		if (mzd > 0)  {  ss += "#D0D8FFWheel up";  --mzd;  }
		if (mzd < 0)  {  ss += "#D0D8FFWheel down";  ++mzd;  }
		txInput->setCaption(ss);
	}

	//  keys up/dn - trklist

	WP wf = MyGUI::InputManager::getInstance().getKeyFocusWidget();
	static float dirU = 0.f,dirD = 0.f;
	if (bGuiFocus && wf != (WP)gcom->trkDesc[0])
	{	if (isKey(UP)  ||isKey(KP_8))  dirD += evt.timeSinceLastFrame;  else
		if (isKey(DOWN)||isKey(KP_2))  dirU += evt.timeSinceLastFrame;  else
		{	dirU = 0.f;  dirD = 0.f;  }
		int d = ctrl ? 4 : 1;
		if (dirU > 0.0f) {  gcom->trkListNext( d);  dirU = -0.2f;  }
		if (dirD > 0.0f) {  gcom->trkListNext(-d);  dirD = -0.2f;  }
	}

	
	///  Update Info texts  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

	//  Road Point
	//----------------------------------------------------------------
	SplineRoad* road = scn->road;
	if (edMode == ED_Road && bEdit() && scn->road)
	{
		int ic = road->iChosen;  bool bCur = ic >= 0;
		SplinePoint& sp = bCur ? road->getPoint(ic) : road->newP;
		std::string s;
		Txt *rdTxt = gui->rdTxt, *rdVal = gui->rdVal, *rdKey = gui->rdKey,
			*rdTxtSt = gui->rdTxtSt, *rdValSt = gui->rdValSt;

		static bool first = true;
		if (first)  // once, static text
		{	first = false;
															rdKey[0]->setCaption("Home");
			rdTxt[1]->setCaption(TR("#{Road_Width}"));		rdKey[1]->setCaption("/ *");
			rdTxt[2]->setCaption(TR("#{Road_Roll}"));		rdKey[2]->setCaption("1 2");
			rdTxt[3]->setCaption(TR("#{Road_Yaw}"));		rdKey[3]->setCaption("3 4");
															rdKey[4]->setCaption(TR("#{InputRoadAngType}"));
			rdTxt[5]->setCaption(TR("#{Road_Snap}"));		rdKey[5]->setCaption("5");
			rdTxt[6]->setCaption(TR("#{Road_Pipe}"));		rdKey[6]->setCaption("O P");//[ ]
			rdTxt[7]->setCaption(TR("#{Road_Column}"));		rdKey[7]->setCaption("End");
															rdKey[8]->setCaption("9 0");//- =
			rdTxt[9]->setCaption(TR("#{Road_ChkR}"));		rdKey[9]->setCaption("K L");
		}

		rdTxt[0]->setCaption(TR(sp.onTer ? "#{Road_OnTerrain}" : "#{Road_Height}"));
		rdVal[0]->setCaption(sp.onTer ? "" : fToStr(sp.pos.y,1,3));

		rdVal[1]->setCaption(fToStr(sp.width,2,4));  // angles
		rdVal[2]->setCaption(fToStr(sp.aRoll,1,3));
		rdVal[3]->setCaption(fToStr(sp.aYaw,1,3));
		rdTxt[4]->setCaption(toStr(sp.aType)+" "+TR("#{Road_Angle"+csAngType[sp.aType]+"}"));
		rdVal[5]->setCaption(fToStr(angSnap,0,1));
													  // .old, mark
		rdTxt[6]->setCaption(sp.onPipe ? (sp.onPipe==1 ?".":"")+ TR("#{Road_OnPipe}") : TR("#{Road_Pipe}"));
		rdTxt[6]->setTextColour(sp.onPipe ? MyGUI::Colour(1.0,0.45,0.2) : MyGUI::Colour(0.86,0.86,0));
		rdVal[6]->setCaption(sp.pipe==0.f ? "" : fToStr(sp.pipe,2,4));
		
		rdTxt[7]->setVisible(!sp.onTer);	rdKey[7]->setVisible(!sp.onTer);
		rdVal[7]->setCaption(sp.onTer ? "" : toStr(sp.cols));
		
		rdTxt[8]->setCaption(toStr(sp.idMtr)+" "+road->getMtrStr(ic));  // mtr

		rdVal[9]->setCaption( sp.chkR == 0.f ? "" : fToStr(sp.chkR,1,3)+"  "+ (sp.chk1st ? "#D0D0FF(1)":"") );

		const static String sLoop[LoopTypes]={"","Loop Straight","Side Loop","Barrel Loop",
			"Loop 2 in 1","Double Loops","Frenzy Loop","Ramp"};  //todo: transl..
		rdTxt[10]->setCaption(TR(sLoop[sp.loop]));
		rdVal[10]->setCaption(!sp.notReal ? "" : TR("#{Road_NotReal}"));

		//  status
		if (road->vSel.size() > 0)  s = TR("#{Road_sel}")+": "+toStr(road->vSel.size());
		else  s = fToStr(road->iChosen+1,0,2)+"/"+toStr(road->vSegs.size());
		rdVal[11]->setCaption(s);

		rdTxt[11]->setCaption(TR(bCur ? "#{Road_Cur}" : "#{Road_New}"));
		rdTxt[11]->setTextColour(bCur ? MyGUI::Colour(0.85,0.75,1) : MyGUI::Colour(0.3,1,0.1));

		rdKey[11]->setCaption(road->bMerge ? "Mrg":"");


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
				rdTxtSt[5]->setCaption(TR("#{Road_OnPipe}"));
				rdTxtSt[6]->setCaption("BnkAng");

				rdTxtSt[7]->setCaption("lod points");
				rdTxtSt[8]->setCaption("segs Merge");
				rdTxtSt[9]->setCaption("vis");
				rdTxtSt[10]->setCaption("tris");
			}			
			rdValSt[0]->setCaption(fToStr(road->st.Length,0,4));
			rdValSt[1]->setCaption(fToStr(road->st.WidthAvg,2,5));
			rdValSt[2]->setCaption(fToStr(road->st.HeightDiff,2,5));

			rdValSt[3]->setCaption(fToStr(road->st.OnTer,1,4)+"%");
			rdValSt[4]->setCaption(fToStr(road->st.Pipes,1,4)+"%");
			rdValSt[5]->setCaption(fToStr(road->st.OnPipe,1,4)+"%");
			rdValSt[6]->setCaption(fToStr(road->st.bankAvg,0,2)+" /"+fToStr(road->st.bankMax,0,2));

			int lp = !bCur ? -1 : road->vSegs[road->iChosen].lpos.size();
			rdValSt[7]->setCaption(toStr(lp));
			rdValSt[8]->setCaption(fToStr(road->st.segsMrg+1,0,2));
			rdValSt[9]->setCaption(fToStr(road->st.iVis,0,2));
			rdValSt[10]->setCaption(fToStr(road->st.iTris/1000.f,1,4)+"k");
		}

		//  edit  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
		Vector3 vx = mCamera->getRight();	   vx.y = 0;  vx.normalise();  // on xz
		Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
		Vector3 vy = Vector3::UNIT_Y;

		if (isKey(LEFT) ||isKey(KP_4))  road->Move(-vx*q);
		if (isKey(RIGHT)||isKey(KP_6))  road->Move( vx*q);
		if (isKey(DOWN) ||isKey(KP_2))  road->Move(-vz*q);
		if (isKey(UP)   ||isKey(KP_8))  road->Move( vz*q);

		if (isKey(KP_MINUS)) road->Move(-vy*q);  if (isKey(KP_MULTIPLY)) road->AddWidth( q*0.4f);
		if (isKey(KP_PLUS))  road->Move( vy*q);  if (isKey(KP_DIVIDE))   road->AddWidth(-q*0.4f);

		if (iSnap == 0 && !alt)
		{	if (isKey(3))	road->AddYaw(-q*3,0,alt);	if (isKey(1))	road->AddRoll(-q*3,0,alt);
			if (isKey(4))	road->AddYaw( q*3,0,alt);	if (isKey(2))	road->AddRoll( q*3,0,alt);
		}
		if (isKey(LEFTBRACKET) ||isKey(O))  road->AddPipe(-q*0.2);
		if (isKey(RIGHTBRACKET)||isKey(P))  road->AddPipe( q*0.2);

		Real qc = ctrl ? q*0.2f/4.f : q*0.2f;  // ctrl no effect
		if (isKey(K))  road->AddChkR(-qc, ctrl);  // chk
		if (isKey(L))  road->AddChkR( qc, ctrl);

		if (mz > 0)			road->NextPoint();
		else if (mz < 0)	road->PrevPoint();
	}

	///  Terrain  Brush
	//--------------------------------------------------------------------------------------------------------------------------------
	else if (edMode < ED_Road)
	{
		Txt *brTxt = gui->brTxt, *brVal = gui->brVal, *brKey = gui->brKey;

		static bool first = true;
		if (first)  // once, static text
		{	first = false;

			brTxt[0]->setCaption(TR("#{Brush_Size}"));		brKey[0]->setCaption("- =");
			brTxt[1]->setCaption(TR("#{Brush_Force}"));		brKey[1]->setCaption("[ ]");
			brTxt[2]->setCaption(TR("#{Brush_Power}"));		brKey[2]->setCaption("K L");//; \'
			brTxt[3]->setCaption(TR("#{Brush_Shape}"));		brKey[3]->setCaption("ctrl");//-K L

			brTxt[4]->setCaption(TR("#{Brush_Freq}"));		brKey[4]->setCaption("O P");
			brTxt[5]->setCaption(TR("#{Brush_Octaves}"));	brKey[5]->setCaption("N M");//, .
			brTxt[6]->setCaption(TR("#{Brush_Offset}"));	brKey[6]->setCaption("9 0");
		}
		brVal[0]->setCaption(fToStr(mBrSize[curBr],1,4));
		brVal[1]->setCaption(fToStr(mBrIntens[curBr],1,4));
		brVal[2]->setCaption(fToStr(mBrPow[curBr],2,4));
		brVal[3]->setCaption(TR("#{Brush_Shape"+csBrShape[mBrShape[curBr]]+"}"));

		bool brN = mBrShape[curBr] >= BRS_Noise;  int i;
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
		brKey[8]->setCaption(brLockPos ? TR("#{Lock}") : "");
		
		
		//  edit  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
		if (mz != 0 && bEdit())
			if (alt){			mBrPow[curBr]   *= 1.f - 0.4f*q*mz;  updBrush();  }
			else if (!shift){	mBrSize[curBr]  *= 1.f - 0.4f*q*mz;  updBrush();  }
			else				mBrIntens[curBr]*= 1.f - 0.4f*q*mz/0.05;

		if (isKey(MINUS)){   mBrSize[curBr]  *= 1.f - 0.04f*q;  updBrush();  }
		if (isKey(EQUALS)){  mBrSize[curBr]  *= 1.f + 0.04f*q;  updBrush();  }
		if (isKey(LEFTBRACKET))   mBrIntens[curBr]*= 1.f - 0.04f*q;
		if (isKey(RIGHTBRACKET))  mBrIntens[curBr]*= 1.f + 0.04f*q;
		if (isKey(SEMICOLON)  || (!ctrl && isKey(K)))
						{	mBrPow[curBr]   *= 1.f - 0.04f*q;  updBrush();  }
		if (isKey(APOSTROPHE) || (!ctrl && isKey(L)))
						{	mBrPow[curBr]   *= 1.f + 0.04f*q;  updBrush();  }

		if (isKey(O)){		mBrFq[curBr]    *= 1.f - 0.04f*q;  updBrush();  }
		if (isKey(P)){		mBrFq[curBr]    *= 1.f + 0.04f*q;  updBrush();  }
		if (isKey(9)){		mBrNOf[curBr]   -= 0.3f*q;		   updBrush();  }
		if (isKey(0)){		mBrNOf[curBr]   += 0.3f*q;		   updBrush();  }

		if (isKey(1)){		mBrFilt         *= 1.f - 0.04f*q;  updBrush();  }
		if (isKey(2)){		mBrFilt         *= 1.f + 0.04f*q;  updBrush();  }
		
		if (mBrIntens[curBr] < 0.1f)  mBrIntens[curBr] = 0.1;  // rest in updBrush
	}

	///  Start  box, road dir
	//----------------------------------------------------------------
	else if (edMode == ED_Start && road)
	{
		Txt *stTxt = gui->stTxt;
		Vector3 p;  if (ndCar)  p = ndCar->getPosition();
		stTxt[0]->setCaption("");
		stTxt[1]->setCaption(TR("#{Road_Width} ") +fToStr(road->vStBoxDim.z,1,4));
		stTxt[2]->setCaption(TR("#{Road_Height} ")+fToStr(road->vStBoxDim.y,1,4));
		stTxt[3]->setCaption(TR("#{Road_Dir}  ") +(road->iDir == 1 ? "+1" : "-1") );

		//  edit
		if (isKey(LEFTBRACKET) ||isKey(O)){  road->AddBoxH(-q*0.2);  UpdStartPos();  }
		if (isKey(RIGHTBRACKET)||isKey(P)){  road->AddBoxH( q*0.2);  UpdStartPos();  }
		if (isKey(SEMICOLON)   ||isKey(K)){  road->AddBoxW(-q*0.2);  UpdStartPos();  }
		if (isKey(APOSTROPHE)  ||isKey(L)){  road->AddBoxW( q*0.2);  UpdStartPos();  }
		//if (mz > 0 && bEdit())	// snap rot by 15 deg ..
	}
	///  Fluids
	//----------------------------------------------------------------
	else if (edMode == ED_Fluids)
	{
		Txt *flTxt = gui->flTxt;
		if (scn->sc->fluids.empty())
		{
			if (flTxt[0])	flTxt[0]->setCaption(TR("#{None}"));
			for (int i=1; i < gui->FL_TXT; ++i)
				if (flTxt[i])  flTxt[i]->setCaption("");
		}else
		{	FluidBox& fb = scn->sc->fluids[iFlCur];
			flTxt[0]->setCaption(TR("#{Road_Cur}") +"  "+toStr(iFlCur+1)+" / "+toStr(scn->sc->fluids.size()));
			flTxt[1]->setCaption(fb.name);
			flTxt[2]->setCaption(TR("#{Obj_Pos}  ") +fToStr(fb.pos.x,1,4)+" "+fToStr(fb.pos.y,1,4)+" "+fToStr(fb.pos.z,1,4));
			flTxt[3]->setCaption("");
			//flTxt[3]->setCaption(TR("#{Obj_Rot}:  ")+fToStr(fb.rot.x,1,4));
			flTxt[3]->setCaption(TR("#{scale}  ") +fToStr(fb.size.x,1,4)+" "+fToStr(fb.size.y,1,4)+" "+fToStr(fb.size.z,1,4));
			flTxt[4]->setCaption(TR("#{Tile}  ") +fToStr(fb.tile.x,3,5)+" "+fToStr(fb.tile.y,3,5));

			//  edit
			if (isKey(LEFTBRACKET) ||isKey(O)){  fb.tile   *= 1.f - 0.04f*q;  bRecreateFluids = true;  }
			if (isKey(RIGHTBRACKET)||isKey(P)){  fb.tile   *= 1.f + 0.04f*q;  bRecreateFluids = true;  }
			if (isKey(SEMICOLON)   ||isKey(K)){  fb.tile.y *= 1.f - 0.04f*q;  bRecreateFluids = true;  }
			if (isKey(APOSTROPHE)  ||isKey(L)){  fb.tile.y *= 1.f + 0.04f*q;  bRecreateFluids = true;  }

			if (mz != 0 && bEdit())  // wheel prev/next
			{	int fls = scn->sc->fluids.size();
				if (fls > 0)  {  iFlCur = (iFlCur-mz+fls)%fls;  UpdFluidBox();  }
			}
		}
	}
	///  Objects
	//----------------------------------------------------------------
	else if (edMode == ED_Objects)
	{
		Txt *objTxt = gui->objTxt;
		int objs = scn->sc->objects.size();
		bool bNew = iObjCur == -1;
		const Object& o = bNew || scn->sc->objects.empty() ? objNew : scn->sc->objects[iObjCur];
		const Quaternion& q = o.nd->getOrientation();
		//Quaternion q(o.rot.w(),o.rot.x(),o.rot.y(),o.rot.z());
		UString s = bNew
			? UString("#80FF80")+TR("#{Road_New}")+"#B0D0B0     "
			: UString("#A0D0FF")+TR("#{Road_Cur}")+"#B0B0D0     ";
		if (!vObjSel.empty())
			s = s + UString("#00FFFF")+TR("#{Road_sel}  ")+toStr(vObjSel.size());
		else
			s = s + (bNew ? "-" : toStr(iObjCur+1)+" / "+toStr(objs));
		objTxt[0]->setCaption(s);
		objTxt[1]->setCaption(bNew ? vObjNames[iObjTNew] : o.name);
		objTxt[2]->setCaption(String(objEd==EO_Move  ?"#60FF60":"")+ TR("#{Obj_Pos}  ") +fToStr(o.pos[0],1,4)+" "+fToStr(o.pos[2],1,4)+" "+fToStr(-o.pos[1],1,4));
		objTxt[3]->setCaption(String(objEd==EO_Rotate?"#FFA0A0":"")+ TR("#{Obj_Rot}  y ") +fToStr(q.getYaw().valueDegrees(),0,3)+" p "+fToStr(q.getPitch().valueDegrees(),0,3)+" r "+fToStr(q.getRoll().valueDegrees(),0,3));
		objTxt[4]->setCaption(String(objEd==EO_Scale ?"#60F0FF":"")+ TR("#{scale}  ") +fToStr(o.scale.x,2,4)+" "+fToStr(o.scale.y,2,4)+" "+fToStr(o.scale.z,2,4));

		objTxt[5]->setCaption(TR("#{Simulation}:  ") + TR(objSim?"#{Yes}":"#{No}")); // +"      "+toStr(world->getNumCollisionObjects()));
		objTxt[5]->setTextColour(objSim ? MyGUI::Colour(1.0,0.9,1.0) : MyGUI::Colour(0.6,0.6,0.63));

		//  edit
		if (mz != 0 && bEdit())  // wheel prev/next
		{
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
			road->Rebuild(true);

		bSelChngOld = road->bSelChng;
		road->bSelChng = false;
	}

	///  upd road lods
	static int dti = 5, ti = dti-1;  ++ti;
	if (road && ti >= dti)
	{	ti = 0;
		road->UpdLodVis(pSet->road_dist, edMode == ED_PrvCam);
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------------
///  Mouse
//---------------------------------------------------------------------------------------------------------------
void App::editMouse()
{
	if (!bEdit())  return;
	
	///  mouse edit Road  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	SplineRoad* road = scn->road;	
	if (road && edMode == ED_Road)
	{
		const Real fMove(5.0f), fRot(10.f);  //par speed

		if (!alt)
		{
			if (mbLeft)    // move on xz
			{	Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
				Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
				road->Move((vNew.x * vx - vNew.y * vz) * fMove * moveMul);
			}else
			if (mbRight)   // height
				road->Move(-vNew.y * Vector3::UNIT_Y * fMove * moveMul);
			else
			if (mbMiddle)  // width
				road->AddWidth(vNew.x * fMove * 0.2f * moveMul);
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
	if (edMode == ED_Start /*&&	vStartPos.size() >= 4 && vStartRot.size() >= 4*/)
	{
		const Real fMove(0.5f), fRot(0.05f);  //par speed
		if (!alt)
		{
			if (mbLeft)
			{
				if (ctrl)  // set pos from ter hit point
				{
					if (road && road->bHitTer)
					{
						Vector3 v = road->posHit;
						scn->sc->startPos[0] = v.x;
						scn->sc->startPos[1] =-v.z;
						scn->sc->startPos[2] = v.y+0.6f;  //car h above
				}	}
				else  // move
				{
					Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
					Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
					Vector3 vm = (-vNew.y * vx - vNew.x * vz) * fMove * moveMul;
					scn->sc->startPos[0] += vm.z;
					scn->sc->startPos[1] += vm.x;
				}
				UpdStartPos();
			}
			else
			if (mbRight)
			{
				Real ym = -vNew.y * fMove * moveMul;
				scn->sc->startPos[2] += ym;  UpdStartPos();
			}
		}else
		{	//  alt
			typedef QUATERNION<float> Qf;  Qf qr;
			if (mbLeft)    // rot yaw
			{
				qr.Rotate(vNew.x * fRot, 0,0,1);
				Qf& q = scn->sc->startRot;
				if (shift)  q = qr * q;  else  q = q * qr;
				UpdStartPos();
			}else
			if (mbRight)   // rot pitch, roll
			{
				if (shift)  qr.Rotate(vNew.x * fRot, 1,0,0);
				else        qr.Rotate(vNew.y *-fRot, 0,1,0);
				Qf& q = scn->sc->startRot;
				q = q * qr;  UpdStartPos();
			}else
			if (mbMiddle)  // rot reset
			{
				qr.Rotate(0, 0,0,1);
				scn->sc->startRot = qr;
				UpdStartPos();
			}
		}
	}

	///  edit fluids . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
	if (edMode == ED_Fluids && !scn->sc->fluids.empty())
	{
		FluidBox& fb = scn->sc->fluids[iFlCur];
		const Real fMove(0.5f), fRot(1.5f);  //par speed
		if (!alt)
		{
			if (mbLeft)	// move on xz
			{
				Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
				Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
				Vector3 vm = (-vNew.y * vz + vNew.x * vx) * fMove * moveMul;
				fb.pos += vm;
				scn->vFlNd[iFlCur]->setPosition(fb.pos);  UpdFluidBox();
			}else
			if (mbRight)  // move y
			{
				Real ym = -vNew.y * fMove * moveMul;
				fb.pos.y += ym;
				scn->vFlNd[iFlCur]->setPosition(fb.pos);  UpdFluidBox();
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
				Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();  vx.x = fabs(vx.x);  vx.z = fabs(vx.z);
				Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();  vz.x = fabs(vz.x);  vz.z = fabs(vz.z);
				Vector3 vm = (vNew.y * vz + vNew.x * vx) * fMove * moveMul;
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
	bool mbAny = mbLeft || mbMiddle || mbRight;
	if (edMode == ED_Objects && mbAny)
	{
		const Real fMove(0.5f), fRot(1.5f), fScale(0.02f);  //par speed
		bool upd = false, sel = !vObjSel.empty();

		//  rotate/scale selected
		Vector3 pos0;  Matrix3 m;
		if (sel && objEd != EO_Move)
		{	pos0 = GetObjPos0();
			if (objEd == EO_Rotate)
			{	Real relA = -vNew.x * fRot * moveMul;
				Quaternion q;  q.FromAngleAxis(Degree(relA), mbLeft ? Vector3::UNIT_Y : (mbRight ? -Vector3::UNIT_Z : Vector3::UNIT_X));
				q.ToRotationMatrix(m);
		}	}

		//  selection, picked or new
		std::set<int>::iterator it = vObjSel.begin();
		int i = sel ? *it : iObjCur;
		while (i == -1 || (i >= 0 && i < scn->sc->objects.size()))
		{
			Object& o = i == -1 ? objNew : scn->sc->objects[i];
			bool upd1 = false;

			switch (objEd)
			{
				case EO_Move:
				{
					if (mbLeft && i != -1)  // move on xz
					{
						Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
						Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
						Vector3 vm = (-vNew.y * vz + vNew.x * vx) * fMove * moveMul;
						o.pos[0] += vm.x;  o.pos[1] -= vm.z;  // todo: for selection ..
						o.SetFromBlt();	 upd1 = true;
					}else
					if (mbRight)  // move y
					{
						Real ym = -vNew.y * fMove * moveMul;
						o.pos[2] += ym;
						o.SetFromBlt();	 upd1 = true;
					}
				}	break;

				case EO_Rotate:
				{
					Real xm = -vNew.x * fRot * moveMul *PI_d/180.f;
					Quaternion q(o.rot.w(),o.rot.x(),o.rot.y(),o.rot.z());
					Radian r = Radian(xm);  Quaternion qr;

					if (sel)  // rotate selected
					{
						Vector3 p(o.pos[0],o.pos[2],-o.pos[1]);  p = p-pos0;
						p = m * p;
						o.pos = MATHVECTOR<float,3>(p.x+pos0.x, -p.z-pos0.z, p.y+pos0.y);
					}

					qr.FromAngleAxis(r, mbLeft ? Vector3::UNIT_Z : (mbRight ? Vector3::UNIT_Y : Vector3::UNIT_X));
					if (sel || alt)  q = qr * q;  else  q = q * qr;
					o.rot = QUATERNION<float>(q.x,q.y,q.z,q.w);

					o.SetFromBlt();	 upd1 = true;
				}	break;

				case EO_Scale:
				{
					float vm = (vNew.y - vNew.x) * fMove * moveMul;
					float sc = 1.f - vm * fScale;
			
					if (sel)  // scale selected
					{
						Vector3 p(o.pos[0],o.pos[2],-o.pos[1]);  p = p-pos0;
						p = p * sc + pos0;
						if (mbLeft)        o.pos = MATHVECTOR<float,3>(p.x, -p.z, p.y);
						else if (mbRight)  o.pos = MATHVECTOR<float,3>(o.pos[0], o.pos[1], p.y);
						else               o.pos = MATHVECTOR<float,3>(p.x, o.pos[1], o.pos[2]);  // todo: use rot for x,z ..
					}
					
					if (!o.dyn)  // static objs only
					{
						if (mbLeft)        o.scale *= sc;  // xyz
						else if (mbRight)  o.scale.y *= sc;  // y
						else               o.scale.z *= sc;  // z
						o.nd->setScale(o.scale);
					}
					o.SetFromBlt();	 upd1 = true;
				}	break;
			}
			if (upd1)
				upd = true;

			if (sel)
			{	++it;  // next sel
				if (it == vObjSel.end())  break;
				i = *it;
			}else  break;  // only 1
		}
		if (upd)
			UpdObjPick();
	}
}
