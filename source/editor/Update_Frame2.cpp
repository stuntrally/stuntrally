#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../sdl4ogre/sdlinputwrapper.hpp"
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <OgreManualObject.h>
#include <MyGUI_InputManager.h>
#include <MyGUI_Widget.h>
using namespace Ogre;
using namespace MyGUI;


//  Update  input, info
//---------------------------------------------------------------------------------------------------------------
bool App::frameRenderingQueued(const FrameEvent& evt)
{
	if (!BaseApp::frameRenderingQueued(evt))
		return false;
	Real dt = evt.timeSinceLastFrame;

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
		if (mpos)
		{	mpos->beginUpdate(0);
			mpos->position(px[0],py[0], 0);  mpos->textureCoord(0, 1);	mpos->position(px[1],py[1], 0);  mpos->textureCoord(1, 1);
			mpos->position(px[3],py[3], 0);  mpos->textureCoord(0, 0);	mpos->position(px[2],py[2], 0);  mpos->textureCoord(1, 0);
			mpos->end();
	}	}
	
	//  status
	if (fStFade > 0.f)
	{	fStFade -= dt;
		
		gui->panStatus->setAlpha(std::min(1.f, fStFade / 1.5f));
		if (fStFade <= 0.f)
			gui->panStatus->setVisible(false);
	}

	#define isKey(a)  mInputWrapper->isKeyDown(SDL_SCANCODE_##a)
	const Real q = (shift ? 0.05 : ctrl ? 4.0 :1.0) * 20 * dt;


	//  key,mb info  ==================
	if (pSet->inputBar)
		UpdKeyBar(dt);

	//  keys up/dn - trklist

	WP wf = MyGUI::InputManager::getInstance().getKeyFocusWidget();
	static float dirU = 0.f,dirD = 0.f;
	if (bGuiFocus && wf != (WP)gcom->trkDesc[0])
	{	if (isKey(UP)  ||isKey(KP_8))  dirD += dt;  else
		if (isKey(DOWN)||isKey(KP_2))  dirU += dt;  else
		{	dirU = 0.f;  dirD = 0.f;  }
		int d = ctrl ? 4 : 1;
		if (dirU > 0.0f) {  gcom->trkListNext( d);  dirU = -0.2f;  }
		if (dirD > 0.0f) {  gcom->trkListNext(-d);  dirD = -0.2f;  }
	}

	
	///  Update Info texts  and edit by key continuous 
	///. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

	SplineRoad* road = scn->road;
	if (edMode == ED_Road && bEdit() && scn->road)
		KeyTxtRoad(q);

	else if (edMode < ED_Road)
		KeyTxtTerrain(q);  // Brush
	
	else if (edMode == ED_Start && road)
		KeyTxtStart(q);  // Start box, road dir

	else if (edMode == ED_Fluids)
		KeyTxtFluids(q);

	else if (edMode == ED_Objects)
		KeyTxtObjects();

	else if (edMode == ED_Particles)
		KeyTxtEmitters(q);

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

		Real dist = pSet->road_dist, dist2 = 200*200; //par
		bool prv = edMode == ED_PrvCam, ed = edMode == ED_Road && !bMoveCam;

		int i = 0;
		for (auto r : scn->roads)
		{
			r->UpdLodVis(dist, prv);
			r->UpdLodVisMarks(dist2, ed && i == scn->rdCur);
			++i;
		}
	}
	return true;
}
