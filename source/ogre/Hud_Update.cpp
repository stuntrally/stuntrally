#include "pch.h"
#include "../vdrift/par.h"
#include "common/Def_Str.h"
#include "common/RenderConst.h"
#include "common/data/CData.h"
#include "common/data/SceneXml.h"
#include "common/data/TracksXml.h"
#include "common/CScene.h"
#include "../vdrift/game.h"
#include "../vdrift/quickprof.h"
#include "../road/Road.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "SplitScreen.h"
#include "FollowCamera.h"
#include "common/MultiList2.h"
#include "common/GraphView.h"

#include <OgreRenderWindow.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>
#include <OgreManualObject.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreOverlayManager.h>
#include <OgreOverlayElement.h>
using namespace Ogre;
using namespace MyGUI;


///---------------------------------------------------------------------------------------------------------------
//  Update HUD
///---------------------------------------------------------------------------------------------------------------
void CHud::Update(int carId, float time)
{
	PROFILER.beginBlock("g.hud");

	
	//  update HUD elements for all cars that have a viewport (local or replay)
	int cnt = std::min(6, (int)app->carModels.size());  // all cars
	int cntC = std::min(4, cnt - (app->isGhost2nd && !app->bRplPlay ? 1 : 0));  // all vis plr
	
	UpdPosElems(cnt, cntC, carId);


	//  track %  local, updated always
	for (int c = 0; c < cntC; ++c)
	{	CarModel* cm = app->carModels[c];
		if (cm->eType == CarModel::CT_LOCAL ||
			cm->eType == CarModel::CT_REPLAY)
			cm->UpdTrackPercent();
	}
	
	if (carId == -1 || app->carModels.empty())
	{
		PROFILER.endBlock("g.hud");
		return;
	}

	#ifdef DEBUG
	assert(carId >= 0);
	assert(carId < app->carModels.size());
	#endif


	//  var
	CarModel* pCarM = app->carModels[carId];
	CAR* pCar = pCarM ? pCarM->pCar : 0;
	Hud& h = hud[carId];


	///  multiplayer
	if (app->mClient)
		UpdMultiplayer(cnt, time);


	///  opponents list
	bool visOpp = h.txOpp[0] && pSet->show_opponents;
	if (visOpp && pCarM && pCarM->pMainNode)
		UpdOpponents(h, cnt, pCarM);


	//  motion blur intensity
	if (pSet->blur)
		UpdMotBlur(pCar, time);


	///  gear, vel texts
	UpdCarTexts(carId, h, time, pCar);

	
	///  Times, race pos
	if (pSet->show_times && pCar)
		UpdTimes(carId, h, time, pCar, pCarM);


	//  camera cur
	if (h.txCam)
	{	FollowCamera* cam = pCarM->fCam;
		if (cam && cam->updName)
		{	cam->updName = false;
			h.txCam->setCaption(cam->sName);
	}	}


	///  debug infos
	UpdDebug(pCar, pCarM);


	PROFILER.endBlock("g.hud");
}


//---------------------------------------------------------------------------------------------------------------
///  Update HUD minimap poses, man obj vertices, etc
//---------------------------------------------------------------------------------------------------------------
void CHud::UpdPosElems(int cnt, int cntC, int carId)
{
	int c;
	//  gui viewport - done once for all
	if (carId == -1)
	for (c = 0; c < cntC; ++c)
	if (app->carModels[c]->eType == CarModel::CT_LOCAL)
	{
		//  hud rpm,vel
		float vel=0.f, rpm=0.f, clutch=1.f;  int gear=1;
		GetCarVals(c,&vel,&rpm,&clutch,&gear);

		if (!app->carModels[c]->vtype == V_Car)
			rpm = -1.f;  // hide rpm gauge
		
		//  update all mini pos tri
		for (int i=0; i < cnt; ++i)
			UpdRotElems(c, i, vel, rpm);
	}

	///  all minimap car pos-es rot
	const static Real tc[4][2] = {{0,1}, {1,1}, {0,0}, {1,0}};
	const float z = pSet->size_minipos;  // tri size
	
	if (carId == -1 && moPos)
	{	moPos->beginUpdate(0);

		const int plr = app->mSplitMgr->mNumViewports;
		for (int v = 0; v < plr; ++v)  // all viewports
		{
			const Hud& h = hud[v];
			const float sc = pSet->size_minimap * app->mSplitMgr->mDims[v].avgsize;
			const Vector3& pos = h.ndMap->getPosition();
			
			for (c = 0; c < cntC; ++c)  // all mini pos for one car
			{
				const SMiniPos& sp = h.vMiniPos[c];
				const ColourValue& clr = app->carModels[c]->color;

				for (int p=0; p < 4; ++p)  // all 4 points
				{
					float x = pos.x + (sp.x + sp.px[p]*z)*sc;
					float y = pos.y + (sp.y + sp.py[p]*z)*sc*asp;
					moPos->position(x, y, 0);
					moPos->textureCoord(tc[p][0], tc[p][1]);
					moPos->colour(clr);
		}	}	}
		
		int ii = plr * cntC;
		for (int i=0; i < ii; ++i)
		{	int n = i*4;
			moPos->quad(n,n+1,n+3,n+2);
		}
		moPos->end();
	}
}

//---------------------------------------------------------------------------------------------------------------
///  Update HUD rotated elems - for carId, in baseCarId's space
///  rpm,vel gauges  and minimap triangles
//---------------------------------------------------------------------------------------------------------------
void CHud::UpdRotElems(int baseCarId, int carId, float vel, float rpm)
{
	//if (carId == -1)  return;
	int b = baseCarId, c = carId;
	bool main = b == c;
	#ifdef DEBUG
	assert(c >= 0);
	assert(b >= 0);
	assert(b < hud.size());  // only b
	assert(c < app->carModels.size());
	assert(b < app->carModels.size());
	assert(c < hud[b].vMiniPos.size());
	#endif
	float angBase = app->carModels[b]->angCarY;
	
	bool bZoom = pSet->mini_zoomed && app->scn->sc->ter,
		bRot = pSet->mini_rotated && app->scn->sc->ter;

	const float vmin[2] = {0.f,-45.f}, rmin[2] = {0.f,-45.f},
		vsc_mph[2] = {-180.f/100.f, -(180.f+vmin[1])/90.f},
		vsc_kmh[2] = {-180.f/160.f, -(180.f+vmin[1])/120.f},
		sc_rpm[2] = {-180.f/6000.f, -(180.f+rmin[1])/5000.f};
	const int ig = pSet->gauges_type > 0 ? 1 : 0;

	//  angles
	float angrmp = rpm*sc_rpm[ig] + rmin[ig];
	float vsc = pSet->show_mph ? vsc_mph[ig] : vsc_kmh[ig];
	float angvel = fabs(vel)*vsc + vmin[ig];
	float angrot = app->carModels[c]->angCarY;
	if (bRot && bZoom && !main)
		angrot -= angBase-180.f;

	Hud& h = hud[b];  int p;
	float sx = 1.4f * h.fScale, sy = sx*asp;  // *par len

	//  4 points, 2d pos
	const static Real tc[4][2] = {{0,1}, {1,1}, {0,0}, {1,0}};  // defaults, no rot
	const static Real tn[4][2] = {{0.5f,1.f}, {1.f,1.f}, {0.5,0.5f}, {1.f,0.5f}};  // tc needle
	const static Real tp[4][2] = {{-1,-1}, {1,-1}, {-1,1}, {1,1}};
	const static float d2r = PI_d/180.f;
	const static Real ang[4] = {0.f,90.f,270.f,180.f};

	float rx[4],ry[4], vx[4],vy[4], px[4],py[4], cx[4],cy[4];  // rpm,vel, pos,crc
	for (int i=0; i < 4; ++i)  // 4 verts, each +90deg
	{
		//  needles
		float ia = 45.f + ang[i];
		if (main)
		{	float r = -(angrmp + ia) * d2r;   rx[i] = sx*cosf(r);  ry[i] =-sy*sinf(r);
			float v = -(angvel + ia) * d2r;   vx[i] = sx*cosf(v);  vy[i] =-sy*sinf(v);
		}
		float p = -(angrot + ia) * d2r;	  float cp = cosf(p), sp = sinf(p);

		//  mini
		if (bRot && bZoom && main)
			{  px[i] = tp[i][0];  py[i] = tp[i][1];  }
		else{  px[i] = cp*1.4f;   py[i] =-sp*1.4f;   }

		float z = bRot ? 0.70f/pSet->zoom_minimap : 0.5f/pSet->zoom_minimap;
		if (!bRot)
			{  cx[i] = tp[i][0]*z;  cy[i] = tp[i][1]*z-1.f;  }
		else{  cx[i] =       cp*z;  cy[i] =      -sp*z-1.f;  }
	}
	    
    //  rpm,vel needles
    float r = 0.55f, v = 0.85f;
    bool bRpm = rpm >= 0.f;
	if (main && h.moNeedles)
	{
		h.moNeedles->beginUpdate(0);
		if (bRpm)
		for (p=0; p < 4; ++p)
		{	h.moNeedles->position(
				h.vcRpm.x + rx[p]*r,
				h.vcRpm.y + ry[p]*r, 0);
			h.moNeedles->textureCoord(tn[p][0], tn[p][1]);
		}
		for (p=0; p < 4; ++p)
		{	h.moNeedles->position(
				h.vcVel.x + vx[p]*v,
				h.vcVel.y + vy[p]*v, 0);
			h.moNeedles->textureCoord(tn[p][0], tn[p][1]);
		}
		h.moNeedles->quad(0,1,3,2);
		if (bRpm)
			h.moNeedles->quad(4,5,7,6);
 		h.moNeedles->end();
	}
	//  rpm,vel gauges backgr
	if (main && h.updGauges && h.moGauges)
	{	h.updGauges=false;
		Real o = pSet->show_mph ? 0.5f : 0.f;
	
		h.moGauges->beginUpdate(0);
		if (bRpm)
		for (p=0; p < 4; ++p)
		{	h.moGauges->position(
				h.vcRpm.x + tp[p][0]*h.fScale*r,
				h.vcRpm.y + tp[p][1]*h.fScale*asp*r, 0);
			h.moGauges->textureCoord(tc[p][0]*0.5f, tc[p][1]*0.5f + 0.5f);
		}
		for (p=0; p < 4; ++p)
		{	h.moGauges->position(
				h.vcVel.x + tp[p][0]*h.fScale*v,
				h.vcVel.y + tp[p][1]*h.fScale*asp*v, 0);
			h.moGauges->textureCoord(tc[p][0]*0.5f+o, tc[p][1]*0.5f);
		}
		h.moGauges->quad(0,1,3,2);
		if (bRpm)
			h.moGauges->quad(4,5,7,6);
		h.moGauges->end();
	}

		
	//---------------------------------------------------------------------------------------------------------------
	///  minimap car pos-es rot
	for (p=0; p < 4; ++p)
	{	
		h.vMiniPos[c].px[p] = px[p];
		h.vMiniPos[c].py[p] = py[p];
	}
	
	//  minimap circle/rect rot
	int qb = app->iCurPoses[b], qc = app->iCurPoses[c];
	if (h.moMap && pSet->trackmap && main)
	{
		h.moMap->beginUpdate(0);
		if (!bZoom)
			for (p=0; p < 4; ++p)
			{	h.moMap->position(tp[p][0],tp[p][1], 0);
				h.moMap->textureCoord(tc[p][0], tc[p][1]);
				h.moMap->colour(tc[p][0],tc[p][1], 0);  }
		else
		{	Vector2 mp(-app->carPoses[qb][b].pos[2], app->carPoses[qb][b].pos[0]);
			float xc =  (mp.x - minX)*scX,
				  yc = -(mp.y - minY)*scY+1.f;
			for (p=0; p < 4; ++p)
			{	h.moMap->position(tp[p][0],tp[p][1], 0);
				h.moMap->textureCoord(cx[p]+xc, -cy[p]-yc);
				h.moMap->colour(tc[p][0],tc[p][1], 1);  }
		}
		h.moMap->end();
	}

	///  minimap car pos  x,y = -1..1
	Vector2 mp(-app->carPoses[qc][c].pos[2], app->carPoses[qc][c].pos[0]);

	//  other cars in player's car view space
	if (!main && bZoom)
	{
		Vector2 plr(-app->carPoses[qb][b].pos[2], app->carPoses[qb][b].pos[0]);
		mp -= plr;  mp *= pSet->zoom_minimap;

		if (bRot)
		{
			float a = angBase * PI_d/180.f;  Vector2 np;
			np.x = mp.x*cosf(a) - mp.y*sinf(a);  // rotate
			np.y = mp.x*sinf(a) + mp.y*cosf(a);  mp = -np;
		}
	}
	float xp =  (mp.x - minX)*scX*2.f-1.f,
		  yp = -(mp.y - minY)*scY*2.f+1.f;

	//  clamp to circle
	if (bZoom /*&& bRot*/)
	{
		float d = xp*xp + yp*yp;
		const float dd = pSet->mini_border ? 0.95f : 0.85f;
		if (d > dd*dd)
		{	d = dd/sqrt(d);
			xp *= d;  yp *= d;
		}
	}else
	{	// clamp to square
		xp = std::min(1.f, std::max(-1.f, xp));
		yp = std::min(1.f, std::max(-1.f, yp));
	}
	
	//  visible
	int cg = app->isGhost2nd && !app->bRplPlay &&
		app->carModels[c]->eType == CarModel::CT_GHOST &&
		c < app->carModels.size()-1 ? 1 : 0;

	bool hide = !app->carModels[c+cg]->mbVisible;
	if (hide)
	{	h.vMiniPos[c].x = -100.f;
		h.vMiniPos[c].y = 0.f;
	}
	else if (bZoom && main)
	{	h.vMiniPos[c].x = 0.f;
		h.vMiniPos[c].y = 0.f;
	}else
	{	h.vMiniPos[c].x = xp;
		h.vMiniPos[c].y = yp;
	}
}
