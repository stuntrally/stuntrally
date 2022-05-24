#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "../ogre/common/Axes.h"
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


//---------------------------------------------------------------------------------------------------------------
///  Mouse
//---------------------------------------------------------------------------------------------------------------
void App::EditMouse()
{
	if (!bEdit())  return;
	
	if (scn->road && edMode == ED_Road)
		MouseRoad();

	if (edMode == ED_Height)  ///  edit ter height val
	{
		if (mbRight)
		{	Real ym = -vNew.y * 0.5f * moveMul;
			terSetH += ym;
	}	}
	
	if (edMode == ED_Start /*&&	vStartPos.size() >= 4 && vStartRot.size() >= 4*/)
		MouseStart();

	if (edMode == ED_Particles && !scn->sc->emitters.empty())
		MouseEmitters();
	
	if (edMode == ED_Fluids && !scn->sc->fluids.empty())
		MouseFluids();

	bool mbAny = mbLeft || mbMiddle || mbRight;
	if (edMode == ED_Objects && mbAny)
		MouseObjects();

	// if (edMode == ED_Particles && mbAny)
	// 	MouseEmitters();
}


///  edit Road  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseRoad()
{
	const Real fMove(0.03f), fRot(10.f);  //par speed
	SplineRoad* road = scn->road;
	const Real d = road->iChosen == -1 ? 30.f * fMove :
		mCamera->getRealPosition().distance(road->getPos(road->iChosen)) * fMove;

	if (!alt)
	{
		if (mbLeft)    // move on xz
		{	Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
			Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
			road->Move((vNew.x * vx - vNew.y * vz) * d * moveMul);
		}else
		if (mbRight)   // height
			road->Move(-vNew.y * Vector3::UNIT_Y * d * moveMul);
		else
		if (mbMiddle)  // width
			road->AddWidth(vNew.x * 1.f * moveMul);
	}else
	{	//  alt
		if (mbLeft)    // rot pitch
			road->AddYaw(   vNew.x * fRot * moveMul,0.f,false/*alt*/);
		if (mbRight)   // rot yaw
			road->AddRoll(  vNew.y *-fRot * moveMul,0.f,false/*alt*/);
	}
}


///  edit Start pos	 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseStart()
{
	const Real fMove(0.02f), fRot(0.05f);  //par speed
	const Real d = mCamera->getRealPosition().distance(Axes::toOgre(scn->sc->startPos[iEnd])) * fMove;
	if (!alt)
	{
		if (mbLeft)
		{
			if (ctrl)  // set pos, from ter hit point
			{	if (scn->road && scn->road->bHitTer)
				{
					Vector3 v = scn->road->posHit;
					scn->sc->startPos[iEnd][0] = v.x;
					scn->sc->startPos[iEnd][1] =-v.z;
					scn->sc->startPos[iEnd][2] = v.y+0.6f;  //car h above
			}	}
			else  // move
			{
				Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
				Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
				Vector3 vm = (-vNew.y * vx - vNew.x * vz) * d * moveMul;
				scn->sc->startPos[iEnd][0] += vm.z;
				scn->sc->startPos[iEnd][1] += vm.x;
			}
			UpdStartPos();
		}
		else if (mbRight)
		{
			Real ym = -vNew.y * d * moveMul;
			scn->sc->startPos[iEnd][2] += ym;  UpdStartPos();
		}
	}else  //  alt
	{
		typedef QUATERNION<float> Qf;  Qf qr;
		if (mbLeft)    // rot yaw
		{
			qr.Rotate(vNew.x * fRot, 0,0,1);
			Qf& q = scn->sc->startRot[iEnd];
			if (shift)  q = qr * q;  else  q = q * qr;
			UpdStartPos();
		}
		else if (mbRight)   // rot pitch, roll
		{
			if (shift)  qr.Rotate(vNew.x * fRot, 1,0,0);
			else        qr.Rotate(vNew.y *-fRot, 0,1,0);
			Qf& q = scn->sc->startRot[iEnd];
			q = q * qr;  UpdStartPos();
		}
		else if (mbMiddle)  // rot reset
		{
			qr.Rotate(0, 0,0,1);
			scn->sc->startRot[iEnd] = qr;
			UpdStartPos();
		}
	}
}


///  edit Fluids . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseFluids()
{
	FluidBox& fb = scn->sc->fluids[iFlCur];
	const Real fMove(0.02f), fRot(1.5f);  //par speed
	const Real d = mCamera->getRealPosition().distance(fb.pos) * fMove;
	if (!alt)
	{
		if (mbLeft)	// move on xz
		{
			Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
			Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
			Vector3 vm = (-vNew.y * vz + vNew.x * vx) * d * moveMul;
			fb.pos += vm;
			scn->vFlNd[iFlCur]->setPosition(fb.pos);  UpdFluidBox();
		}
		else if (mbRight)  // move y
		{
			Real ym = -vNew.y * d * moveMul;
			fb.pos.y += ym;
			scn->vFlNd[iFlCur]->setPosition(fb.pos);  UpdFluidBox();
		}
		// rot not supported (bullet trigger isnt working, trees check & waterDepth is a lot simpler)
		/*else if (mbMiddle)  // rot yaw
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
			Vector3 vm = (vNew.y * vz + vNew.x * vx) * d * moveMul;
			fb.size += vm;
			if (fb.size.x < 0.2f)  fb.size.x = 0.2f;
			if (fb.size.z < 0.2f)  fb.size.z = 0.2f;
			bRecreateFluids = true;  //
		}
		else if (mbRight)  // size y
		{
			float vm = -vNew.y * d * moveMul;
			fb.size.y += vm;
			if (fb.size.y < 0.2f)  fb.size.y = 0.2f;
			bRecreateFluids = true;  //
		}
	}
}


///  edit Emitters . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseEmitters()
{
	SEmitter& em = scn->sc->emitters[iEmtCur];
	const Real fMove(0.02f), fRot(1.5f);  //par speed
	const Real d = mCamera->getRealPosition().distance(em.pos) * fMove;
	if (emtEd == EO_Move)
	{
		if (mbLeft)	// move on xz
		{
			Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
			Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
			Vector3 vm = (-vNew.y * vz + vNew.x * vx) * d * moveMul;
			em.pos += vm;
			em.nd->setPosition(em.pos);  UpdEmtBox();
		}
		else if (mbRight)  // move y
		{
			Real ym = -vNew.y * d * moveMul;
			em.pos.y += ym;
			em.nd->setPosition(em.pos);  UpdEmtBox();
		}
		else if (mbMiddle)  // rot yaw
		{
			Real xm = vNew.x * fRot * moveMul;
			em.rot += xm;
			//em.nd->setOrientation(Quaternion(Degree(em.rot.x), em.up));
		}
	}else if(emtEd == EO_Scale)
	{
		if (mbLeft)  // size xz
		{
			Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();  vx.x = fabs(vx.x);  vx.z = fabs(vx.z);
			Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();  vz.x = fabs(vz.x);  vz.z = fabs(vz.z);
			Vector3 vm = (vNew.y * vz + vNew.x * vx) * d * moveMul;
			em.size += vm;
			if (em.size.x < 0.f)  em.size.x = 0.f;
			if (em.size.z < 0.f)  em.size.z = 0.f;
			em.UpdEmitter();  UpdEmtBox();
		}
		else if (mbRight)  // size y
		{
			float vm = -vNew.y * d * moveMul;
			em.size.y += vm;
			if (em.size.y < 0.f)  em.size.y = 0.f;
			em.UpdEmitter();  UpdEmtBox();
		}
	}
	//else if(emtEd == EO_Rotate)
}


///  edit Objects . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
void App::MouseObjects()
{
	const Real fMove(0.02f), fRot(1.5f), fScale(0.02f);  //par speed
	bool upd = false, sel = !vObjSel.empty();

	//  rotate/scale selected
	Vector3 pos0;  Matrix3 m;
	if (sel && objEd != EO_Move)
	{
		pos0 = GetObjPos0();
		if (objEd == EO_Rotate)
		{
			Real relA = -vNew.x * fRot * moveMul;
			Quaternion q;  q.FromAngleAxis(Degree(relA),
				mbLeft ? Vector3::UNIT_Y : (mbRight ? -Vector3::UNIT_Z : Vector3::UNIT_X));
			q.ToRotationMatrix(m);
	}	}

	//  selection, picked or new
	auto it = vObjSel.begin();
	int i = sel ? *it : iObjCur;
	while (i == -1 || (i >= 0 && i < scn->sc->objects.size()))
	{
		Object& o = i == -1 ? objNew : scn->sc->objects[i];
		bool upd1 = false;
		const Real d = mCamera->getRealPosition().distance(Axes::toOgre(o.pos)) * fMove;

		switch (objEd)
		{
			case EO_Move:
			{
				if (mbLeft && i != -1)  // move on xz
				{
					Vector3 vx = mCamera->getRight();      vx.y = 0;  vx.normalise();
					Vector3 vz = mCamera->getDirection();  vz.y = 0;  vz.normalise();
					Vector3 vm = (-vNew.y * vz + vNew.x * vx) * d * moveMul;
					o.pos[0] += vm.x;  o.pos[1] -= vm.z;  // todo: for selection ..
					o.SetFromBlt();	 upd1 = true;
				}
				else if (mbRight)  // move y
				{
					Real ym = -vNew.y * d * moveMul;
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
				float vm = (vNew.y - vNew.x) * d * moveMul;
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
