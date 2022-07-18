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
#include <OgreParticleSystem.h>
#include <OgreParticleEmitter.h>
#include <MyGUI_InputManager.h>
#include <MyGUI_Widget.h>
#include <MyGUI_TextBox.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_Window.h>
using namespace Ogre;
using namespace MyGUI;

#define isKey(a)  mInputWrapper->isKeyDown(SDL_SCANCODE_##a)


//  Road Point
//---------------------------------------------------------------------------------------------------------------
void App::KeyTxtRoad(Real q)
{
	SplineRoad* road = scn->road;
	int ic = road->iChosen;  bool bCur = ic >= 0, notEmpty = road->getNumPoints() > 1;
	bool ok = bCur && notEmpty;
	SplinePoint& sp = ok ? road->getPoint(ic) : road->newP;
	
	std::string s;
	Txt *rdTxt = gui->rdTxt, *rdVal = gui->rdVal, *rdKey = gui->rdKey,
		*rdTxtSt = gui->rdTxtSt, *rdValSt = gui->rdValSt;
	Img *rdImg = gui->rdImg;

	static bool first = true;
	if (first)  // once, static text
	{	first = false;
													rdKey[0]->setCaption("+ - Home");
		rdTxt[1]->setCaption(TR("#{Road_Width}"));	rdKey[1]->setCaption("/ *");
		rdTxt[2]->setCaption(TR("#{Road_Roll}"));	rdKey[2]->setCaption("1 2");
		rdTxt[3]->setCaption(TR("#{Road_Yaw}"));	rdKey[3]->setCaption("3 4");
													rdKey[4]->setCaption(TR("#{InputRoadAngType}"));
		rdTxt[5]->setCaption(TR("#{Road_Snap}"));	rdKey[5]->setCaption("5");
		rdTxt[6]->setCaption(TR("#{Road_Pipe}"));	rdKey[6]->setCaption("O P#E07030 8");//[ ]
		rdTxt[7]->setCaption(TR("#{Road_Column}"));	rdKey[7]->setCaption("End");
													rdKey[8]->setCaption("9 0");//- =
		rdTxt[9]->setCaption(TR("#{Road_ChkR}"));	rdKey[9]->setCaption("K L");
													rdKey[10]->setCaption("#80E0E0       7");
													//rdKey[11]->setCaption("#C060E0 sh-8");
		rdTxt[12]->setCaption(TR("#{Wall}"));		rdKey[12]->setCaption("ctrl 9 0");
	}

	bool ter = road->IsRiver() ? true : !ok ? false :
		!(!sp.onTer || !road->getPoint(road->getNext(ic)).onTer);

  	//  type
	const static String rdt[RD_ALL] = {"#{Road}", "#{River}", "#{Wall}", "#{RacingLine}"};
	const static Colour clr[RD_ALL] = {
		Colour(1.0,0.8,0.5), Colour(0.5,0.8,1.0), Colour(0.9,0.95,1.0), Colour(0.5,1.0,0.0),};
	int t = road->type;
	mWndRoadCur->setCaption(TR("R - " + rdt[t]));
	mWndRoadCur->setColour(clr[t]);  mWndRoadCur->setAlpha(t == 0 ? 0.5f : 0.8f);

	rdTxt[0]->setCaption(TR(sp.onTer ? "#{Road_OnTerrain}" : "#{Road_Height}"));
	rdVal[0]->setCaption(sp.onTer ? "" : fToStr(sp.pos.y,1,3));

	rdVal[1]->setCaption(fToStr(sp.width,2,4));  // angles
	rdVal[2]->setCaption(fToStr(sp.aRoll,1,3));
	rdVal[3]->setCaption(fToStr(sp.aYaw,1,3));
	rdTxt[4]->setCaption(toStr(sp.aType)+" "+TR("#{Road_Angle"+csAngType[sp.aType]+"}"));
	rdVal[5]->setCaption(fToStr(angSnap,0,1));
													// .old, mark
	rdImg[6]->setVisible(!sp.onPipe);  rdImg[7]->setVisible(sp.onPipe);
	rdTxt[6]->setCaption(sp.onPipe ? TR("#{Road_OnPipe}") : TR("#{Road_Pipe}"));
	rdTxt[6]->setTextColour(sp.onPipe ? MyGUI::Colour(1.0,0.45,0.2) : MyGUI::Colour(0.86,0.86,0));
	rdVal[6]->setCaption(sp.pipe==0.f ? "" : fToStr(sp.pipe,2,4));
	
	bool vis = !ter && !sp.isnt();
	rdTxt[7]->setVisible(vis);	rdVal[7]->setVisible(vis);  rdKey[7]->setVisible(vis);
	rdVal[7]->setCaption(ter ? "" : toStr(sp.cols));  // column

	rdTxt[12]->setVisible(vis);  rdKey[12]->setVisible(vis);  
	rdTxt[12]->setCaption(toStr(sp.idWall)+" "+road->getWallMtrStr(ic));  // wall mtr
	rdTxt[8]->setCaption(toStr(sp.idMtr)+" "+road->getMtrStr(ic));  // mtr

	rdVal[9]->setCaption( sp.chkR == 0.f ? "" : fToStr(sp.chkR,1,3)+"  "+ (sp.chk1st ? "#D0D0FF(1)":"") );

	const static String sLoop[LoopTypes]={"","Loop Straight","Side Loop","Barrel Loop",
		"Loop 2 in 1","Double Loops","Frenzy Loop","Ramp","View Only"};  //todo: transl?
	rdTxt[10]->setCaption(TR(sLoop[sp.loop]));
	rdVal[10]->setCaption(!sp.notReal ? "" : TR("#{Road_NotReal}"));
	rdImg[10]->setVisible(!sp.notReal);  rdImg[11]->setVisible(sp.notReal);

	//  status
	if (road->vSel.size() > 0)  s = TR("#{Road_sel}")+": "+toStr(road->vSel.size());
	else  s = fToStr(road->iChosen+1,0,2)+"/"+toStr(road->vSegs.size());
	rdVal[11]->setCaption(s);

	rdTxt[11]->setCaption(TR(bCur ? "#{Road_Cur}" : "#{Road_New}"));
	rdTxt[11]->setTextColour(bCur ? MyGUI::Colour(0.85,0.75,1) : MyGUI::Colour(0.3,1,0.1));

	rdKey[11]->setCaption(toStr(scn->rdCur+1)+"/"+toStr(scn->roads.size()));

	rdKey[13]->setCaption(road->bMerge ? TR("#{Road_Merged}"): "");
	rdTxt[13]->setCaption(road->isLooped ? "": TR("#{Road_NotLooped}"));


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
		rdValSt[0]->setCaption(fToStr(road->st.Length, 0,4));
		rdValSt[1]->setCaption(fToStr(road->st.WidthAvg, 2,5));
		rdValSt[2]->setCaption(fToStr(road->st.HeightDiff, 2,5));

		rdValSt[3]->setCaption(fToStr(road->st.OnTer, 1,4)+"%");
		rdValSt[4]->setCaption(fToStr(road->st.Pipes, 1,4)+"%");
		rdValSt[5]->setCaption(fToStr(road->st.OnPipe, 1,4)+"%");
		rdValSt[6]->setCaption(fToStr(road->st.bankAvg, 0,2)+" /"+fToStr(road->st.bankMax, 0,2));

		int lp = !bCur ? -1 : road->vSegs[road->iChosen].lpos.size();
		rdValSt[7]->setCaption(toStr(lp));
		rdValSt[8]->setCaption(fToStr(road->st.segsMrg+1, 0,2));//+" "+fToStr(road->st.segsMrgW+1, 0,2));
		rdValSt[9]->setCaption(fToStr(road->st.iVis, 0,2));
		rdValSt[10]->setCaption(fToStr(road->st.iTris/1000.f, 1,4)+"k");
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
void App::KeyTxtTerrain(Real q)
{
	Txt *brTxt = gui->brTxt, *brVal = gui->brVal, *brKey = gui->brKey;
	SplineRoad* road = scn->road;

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


//  Start box
//----------------------------------------------------------------
void App::KeyTxtStart(Real q)
{
	SplineRoad* road = scn->road;
	Txt *stTxt = gui->stTxt;
	Vector3 p;  if (ndCar)  p = ndCar->getPosition();
	stTxt[0]->setCaption("");
	stTxt[1]->setCaption(TR("#{Road_Width} ") +fToStr(road->vStBoxDim.z,1,4));
	stTxt[2]->setCaption(TR("#{Road_Height} ")+fToStr(road->vStBoxDim.y,1,4));
	stTxt[3]->setCaption(TR("#{Road_Dir}  ") +(road->iDir == 1 ? "+1" : "-1") );
	stTxt[4]->setCaption(TR(road->isLooped ? "" : iEnd ? "#{FogEnd}" : "#{FogStart}"));

	//  edit
	if (isKey(LEFTBRACKET) ||isKey(O)){  road->AddBoxH(-q*0.2);  UpdStartPos();  }
	if (isKey(RIGHTBRACKET)||isKey(P)){  road->AddBoxH( q*0.2);  UpdStartPos();  }
	if (isKey(SEMICOLON)   ||isKey(K)){  road->AddBoxW(-q*0.2);  UpdStartPos();  }
	if (isKey(APOSTROPHE)  ||isKey(L)){  road->AddBoxW( q*0.2);  UpdStartPos();  }
	//if (mz > 0 && bEdit())	// snap rot by 15 deg ..
}


///  Fluids
//----------------------------------------------------------------
void App::KeyTxtFluids(Real q)
{
	Txt *flTxt = gui->flTxt;
	if (scn->sc->fluids.empty())
	{
		if (flTxt[0])	flTxt[0]->setCaption(TR("#{None}"));
		for (int i=1; i < gui->FL_TXT; ++i)
			if (flTxt[i])  flTxt[i]->setCaption("");
	}else
	{	FluidBox& fb = scn->sc->fluids[iFlCur];
		flTxt[0]->setCaption(TR("#{Road_Cur}     ") +toStr(iFlCur+1)+" / "+toStr(scn->sc->fluids.size()));
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
void App::KeyTxtObjects()
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
	objTxt[5]->setTextColour(objSim ? MyGUI::Colour(1.0,0.9,1.0) : MyGUI::Colour(0.8,0.8,0.83));

	//  edit
	if (mz != 0 && bEdit())  // wheel prev/next
	{
		if (objs > 0)  {  iObjCur = (iObjCur-mz+objs)%objs;  UpdObjPick();  }
	}
}


///  Emitters
//----------------------------------------------------------------
void App::KeyTxtEmitters(Real q)
{
	Txt *emtTxt = gui->emtTxt;
	int emts = scn->sc->emitters.size();
	bool bNew = iEmtCur == -1;
	SEmitter& e = bNew || scn->sc->emitters.empty() ? emtNew : scn->sc->emitters[iEmtCur];
	
	UString s = bNew
		? UString("#80FF80")+TR("#{Road_New}")+"#B0D0B0     "
		: UString("#A0D0FF")+TR("#{Road_Cur}")+"#B0B0D0     ";
	s = s + (bNew ? "-" : toStr(iEmtCur+1)+" / "+toStr(emts));
	emtTxt[0]->setCaption(s);
	emtTxt[1]->setCaption(/*bNew ? vObjNames[iEmtTNew] :*/ e.name);
	emtTxt[2]->setCaption(String(emtEd==EO_Move  ?"#60FF60":"")+ TR("#{Obj_Pos}  ") +fToStr(e.pos.x,1,4)+" "+fToStr(e.pos.y,1,4)+" "+fToStr(e.pos.z,1,4));
	emtTxt[3]->setCaption(String(emtEd==EO_Rotate?"#FFA0A0":"")+ TR("#{Obj_Rot}  y ") +fToStr(e.rot/*e.up.x*/,0,3) );
	emtTxt[4]->setCaption(String(emtEd==EO_Scale ?"#60F0FF":"")+ TR("#{scale}  ") +fToStr(e.size.x,2,4)+" "+fToStr(e.size.y,2,4)+" "+fToStr(e.size.z,2,4));
	emtTxt[5]->setCaption(TR("#{Density}: ") +fToStr(e.rate,0,3) );
	emtTxt[6]->setCaption(TR("#{Count}: ")+ toStr(e.ps ? e.ps->getNumParticles() : 0) + (e.stat ? "  Static" : ""));

	if (!bEdit())  return;
	if (isKey(LEFTBRACKET) ||isKey(O)){  e.rate  *= 1.f - 0.04f*q;  e.ps->getEmitter(0)->setEmissionRate(e.rate);  }
	if (isKey(RIGHTBRACKET)||isKey(P)){  e.rate  *= 1.f + 0.04f*q;  e.ps->getEmitter(0)->setEmissionRate(e.rate);  }

	//  edit
	if (mz != 0)  // wheel prev/next
	{
		if (emts > 0)  {  iEmtCur = (iEmtCur-mz+emts)%emts;  UpdEmtBox();  }
	}
}
