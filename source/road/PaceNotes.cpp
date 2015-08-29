#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/Axes.h"
#include "PaceNotes.h"
#include "../ogre/ReplayTrk.h"
#include "../vdrift/dbl.h"
#ifdef SR_EDITOR
	#include "../editor/CApp.h"
	#include "../editor/settings.h"
#else
	#include "../ogre/CGame.h"
	#include "../vdrift/settings.h"
#endif
#include "../vdrift/pathmanager.h"
#include "../road/SplineBase.h"
#include "../road/Road.h"
#include <OgreTimer.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreBillboardSet.h>
//#include <OgreTerrain.h>
using namespace std;
using namespace Ogre;


//  ctor  ---------
PaceNote::PaceNote()
	:nd(0), bb(0), pos(0,0,0)
	,type(P_1), dir(0), vel(0.f)
	,clr(0,0,0,0), ofs(0,0), uv(0,0)
{	}


//  Pace notes
//--------------------------------------------------------------------------------------
void PaceNotes::Rebuild(SplineRoad* road)
{
	Ogre::Timer ti;	

	Destroy();
return;
	
	///  trace road
	int ii = road->vPace.size();
	for (int i=0; i < ii; ++i)
	{
		const SplineRoad::PaceM& cur = road->vPace[i],
			prv = road->vPace[(i-1+ii)%ii], nxt = road->vPace[(i+1)%ii];

		Vector3 c1 = cur.pos - prv.pos, c2 = nxt.pos - cur.pos;
		c1.y = 0.f;  c2.y = 0.f;
		c1.normalise();  c2.normalise();

		Vector3 cross = c1.crossProduct(c2);
		Real dot = c1.dotProduct(c2);
		Real aa = acos(dot);  // road yaw angle
		//Real aa = asin(cross.length());

		Vector3 n(0,1,0);
		Real dn = n.dotProduct(cross);
		if (dn < 0.f)  aa = -aa;
		if (cur.loop)  aa = 0.f;  // loop zero
			
		// LogO(fToStr(aa*180.f/PI_d,1,5));//+" "+fToStr(dn));
		//LogO(fToStr(aa));
		if (fabs(aa) < 0.05f)  aa = 0.f;
			
		PaceNote o;  // add
		o.pos = cur.pos;
		o.size = Vector2(1.f, 6.f);
		o.clr = Vector4(1,1,1,1);
		o.uv = Vector2(aa < 0.f ? 0.25f : 0.f,  0.5f + fabs(aa)*0.5f);
		//o.uv = Vector2(i/3 * 0.25f, i%2 * 0.25f);
		Create(o);  vv.push_back(o);/**/
	}
	
return;

	///  trace Track's Ghost
	TrackGhost gho;
	
	//  foreach track
	bool rev = false;
	string sRev = rev ? "_r" : "";
	#ifdef SR_EDITOR
	string track = pApp->pSet->gui.track;
	#else
	string track = pApp->pSet->game.track;
	#endif
	//if (track.substr(0,4) == "Test" && track.substr(0,5) != "TestC")  continue;
	
	//  load
	string file = PATHMANAGER::TrkGhosts()+"/"+ track + sRev + ".gho";
	if (!PATHMANAGER::FileExists(file))
	{	LogO("NOT found: "+file);/**/  }
	else
	{	LogO("---------  "+track+"  ---------");
		gho.LoadFile(file);
		
		//  test
		Vector3 pos,oldPos;  float oldTime = 0.f;
		Quaternion rot;  float vel = 0.f;

		int num = gho.getNumFrames(), ijmp = 0;
		float* sta = new float[num];
		int i,ii,n;  float oy=0.f;
		for (ii=0; ii < 2; ++ii)
		for (i=0; i < num; ++i)
		{
			//  pos
			const TrackFrame& fr = gho.getFrame0(i);
			Axes::toOgre(pos, fr.pos);  // pos
			rot = Axes::toOgre(fr.rot);
			//float y = rot.getYaw().valueDegrees();
			Vector3 pp = pos - oldPos;
			
			//  yaw
			float yy = TerUtil::GetAngle(pp.x, pp.z) *180.f/PI_d;
			float y = yy-oy;  if (y > 180)  y -= 360;  if (y < -180)  y += 360;
			oy = yy;

			//  vel
			float dist = pp.length();
			float dt = fr.time - oldTime;  // 0.04
			if (i > 0 && i < num-1 && dt > 0.001f)
				vel = 3.6f * dist / dt;

			if (vel < 20)  y *= vel / 20.f;  // y sc

			//  sudden pos jumps-
			bool jmp = false;
			if (i > 10 && i < num-1)
			if (dist > 6.f)  //par
			{	jmp = true;  ++ijmp;  }


			//  avg steer
			float sa = 0.f;  const int nn = 10;  //par
			if (i < num-nn)
			for (n=i; n < i+nn; ++n)
			{
				const TrackFrame& f = gho.getFrame0(n);
				sa += fabs(f.steer/127.f);
			}
			sa /= float(nn);
			sta[i] = sa;
			
			
			//  log  ----
			if (ii==0)
			LogO("i:"+ iToStr(i,4) +" t:"+ fToStr(fr.time,2,6)//+" dt: "+fToStr(dt)
				+"  v:"+ fToStr(vel,0,4)
				+"  b:"+ toStr(fr.brake)
				+"  s:"+ (fr.steer==0 ? " ---" : fToStr(fr.steer/127.f,1,4))
				+"  a:"+ fToStr(sa,1,4)
				+"  y:"+ fToStr(y,1,4)
				//+"  p: "+ fToStr(fr.pos[0])+" "+fToStr(fr.pos[1])+" "+fToStr(fr.pos[2])
				+(jmp ? " !jd: "+fToStr(dist) : "")
			);
	
			
			///  add pace note
			if (ii==1)
			{	
				float sb = 0.f;  const int nn = 10;  //par
				if (i < num-nn)
				for (n=i; n < i+nn; ++n)
				{
					const TrackFrame& f = gho.getFrame0(n);
					if (sa > 0.3f)
					sb += sa;
				}
				sb /= float(nn);
				//LogO(fToStr(sb));
							
				//  create
				if (i%6==0 && sb > 0.4f)
				{
					PaceNote n;
					n.pos = pos;  //fr.brake  fr.steer
					Create(n);
					vv.push_back(n);
				}
			}
			
			oldPos = pos;  oldTime = fr.time;
		}
		if (ijmp > 0)
			LogO("!Jumps: "+toStr(ijmp));
		delete[] sta;
	}


	//UpdVis(fLodBias);

	LogO(String("::: Time PaceNotes Rebuild: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
//--------------------------------------------------------------------------------------


//  Create
void PaceNotes::Create(PaceNote& n)
{
	++ii;
	n.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	n.bb = mSceneMgr->createBillboardSet("P-"+toStr(ii),2);
	n.bb->setDefaultDimensions(n.size.x, n.size.y);

	n.bb->setRenderQueueGroup(RQG_CarTrails);
	n.bb->setVisibilityFlags(RV_Car);
	n.bb->setCustomParameter(0, Vector4(n.ofs.x, n.ofs.y, n.uv.x, n.uv.y));  // params, uv ofs

	n.bb->createBillboard(Vector3(0,0,0), ColourValue(n.clr.x, n.clr.y, n.clr.z, n.clr.w));
	//n.bb->setVisible(false);
	n.bb->setMaterialName("pacenote");
	n.nd->attachObject(n.bb);
	n.nd->setPosition(n.pos);
}

//  Destroy
void PaceNotes::Destroy(PaceNote& n)
{
	mSceneMgr->destroyBillboardSet(n.bb);  n.bb = 0;
	mSceneMgr->destroySceneNode(n.nd);  n.nd = 0;
}

void PaceNotes::Destroy()  // all
{
	for (size_t i=0; i < vv.size(); ++i)
		Destroy(vv[i]);
	vv.clear();
	ii = 0;
}


//  ctor  ---------
PaceNotes::PaceNotes(App* papp) :pApp(papp)
	,mSceneMgr(0),mCamera(0),mTerrain(0), ii(0)
{	}

//  setup
void PaceNotes::Setup(SceneManager* sceneMgr, Camera* camera, Terrain* terrain)
{
	mSceneMgr = sceneMgr;  mCamera = camera;  mTerrain = terrain;
}
