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
	:nd(0), bb(0), pos(0,0,0), use(1)
	//,type(P_1), dir(0), vel(0.f)
	,size(4.f,4.f), clr(0,0,0,0), ofs(0,0), uv(0,0)
{	}
PaceNote::PaceNote(int t, Vector3 p, float sx,float sy,
		float r,float g,float b,float a, float ox,float oy, float u,float v)
	:nd(0), bb(0), pos(p), use(t)
	//,type(P_1), dir(0), vel(0.f)
	,size(sx,sy), clr(r,g,b,a), ofs(ox,oy), uv(u,v)
{	}


//  Pace notes
//--------------------------------------------------------------------------------------
void PaceNotes::Rebuild(SplineRoad* road)
{
	Ogre::Timer ti;	

	Destroy();

//return;

	const float u=0.125f,
		barX=1.f,barY=6.f, barA=0.6f, useX=2.f, useA=0.6f,
	#ifdef SR_EDITOR
	#define USED
	#define BARS
	signX = 4.f;  // ed
	#else
	signX = 2.f;  // game
	#endif
	
	///  trace Road  |||
	int ii = road->vPace.size();
	for (int i=0; i < ii; ++i)
	{
		SplineRoad::PaceM& cur = road->vPace[i],
			prv = road->vPace[(i-1+ii)%ii], nxt = road->vPace[(i+1)%ii];

		//  dir xz
		Vector3 c1 = cur.pos - prv.pos, c2 = nxt.pos - cur.pos;
		c1.y = 0.f;  c2.y = 0.f;
		c1.normalise();  c2.normalise();

		//  yaw ang
		Vector3 cross = c1.crossProduct(c2);
		Real dot = c1.dotProduct(c2);
		Real aa = acos(dot);  // road yaw angle
		//Real aa = asin(cross.length());

		//  sign
		Real dn = Vector3::UNIT_Y.dotProduct(cross);
		if (dn < 0.f)  aa = -aa;
		
		//  no loops
		if (cur.loop)  aa = 0.f;
			
		//LogO(fToStr(aa*180.f/PI_d,1,5));//+" "+fToStr(dn));
		//LogO(fToStr(aa));

		//  no straights  //par 0.05
		if (fabs(aa) < 0.01f)  aa = 0.f;
		cur.aa = aa;  // save

		#ifdef BARS  // add dbg bar |
		PaceNote o(4, cur.pos, barX,barY, 1,1,1,barA,  // size,clr
			aa < 0.f ? 0.f : 1.f, 0.5f,  // dir, width   //par_ 0.3-1.2
			aa < 0.f ? 7.5f*u : 7.f*u, u + fabs(aa)*0.6f);  // uv
		Create(o);  vPN.push_back(o);
		#endif
	}
	
	
	#if 1
	///  simple turns  ~ ~ ~
	const int nn = 7;  // levels								//par turn sharpness
	const float angN[nn] = {0.06f, 0.2f, 0.3f, 0.4f, 0.5f, 0.7f, 1.0f}, aNm = 0.7f;
	const int Radd[nn]	 = {2,     1,    1,    0,    0,    0,    0};  // needed for sign
	const int Rlen[nn]	 = {10,    10,   8,    8,    7,    7,    6};  // road search range
	bool dirR = road->iDir > 0, loop1 = false;

	//for (int n=2; n >= 0; --n)
	for (int n=nn-1; n >= 0; --n)  // all levels
	for (int i=0; i < ii; ++i)  // all road points
	{
		SplineRoad::PaceM& p = road->vPace[i];
		if (fabs(p.aa) > angN[n]*aNm && p.used < 0)
		{
			//p.used = n;
			
			//  get neighbors too
			//  staying in range not below amul of original angle
			const int ri = Rlen[n];
			//float amul = 0.4f,  //par
			//	am = angN[n]*aNm * amul;
			float am = n==0 ? 0.05f: angN[n-1]*aNm * 0.4f;  //par sustain
			bool dir = dirR ? p.aa > 0.f : p.aa < 0.f;
			float Adir = dir ? 0.f : 1.f;

			Vector3 pos = p.pos;  // main sign pos
			float Asum = p.aa;

			#ifdef USED  // add used
			PaceNote o(2, p.pos2, useX,useX, 1,1,1,1,  // size, clr
				Adir, 0.f,  n*u, 0.f);  // dir, uv
			Create(o);  vPN.push_back(o);
			#endif

			int r=1, rr=0, radd=0, rsub=0;
			bool ok = true;
			while (ok && rr < ri)
			{
				SplineRoad::PaceM& pp = road->vPace[(i+r)%ii];
				ok = pp.used < 0 && (p.aa > 0.f && pp.aa > am || p.aa < 0.f && pp.aa <-am);
				if (ok)
				{
					pp.used = n;  Asum += pp.aa;  ++radd;
					if (!dirR)  pos = pp.pos;  // back pos

					#ifdef USED  // add used
					PaceNote o(3, pp.pos2, useX,useX, 0.9,0.95,1,useA,  // size, clr
						Adir, 0.f,  n*u, 0.f);  // dir, uv
					Create(o);  vPN.push_back(o);
					#endif
				}
				++rr;  ++r;
			}
			r=1;  rr=0;  ok = true;
			while (ok && rr < ri)
			{
				SplineRoad::PaceM& pp = road->vPace[(i+r+ii)%ii];
				ok = pp.used < 0 && (p.aa > 0.f && pp.aa > am || p.aa < 0.f && pp.aa <-am);
				if (ok)
				{
					pp.used = n;  Asum += pp.aa;  ++rsub;
					if (dirR)  pos = pp.pos;  // back pos

					#ifdef USED  // add used
					PaceNote o(3, pp.pos2, useX,useX, 1,0.95,0.9,useA,  // size, clr
						Adir, 0.f,  n*u, 0.f);  // dir, uv
					Create(o);  vPN.push_back(o);
					#endif
				}
				++rr;  --r;
			}
			
			#if 1  // add turn
			int rsad = rsub + radd;
			if (rsad > Radd[n])
			{
				p.used = n;
				//  long turn, total angle  ...
				float s = signX * (0.5f + 0.5f * (1+rsad) / (1+Radd[n]));
				float l = std::min(2.f, std::max(0.5f,
					Asum / (rsad * angN[n]*aNm) ));
				LogO("n "+toStr(n)+"  Asum:"+fToStr(Asum,2,5)+"  s: "+fToStr(s)+"  l: "+fToStr(l));
				s = signX;  l = 1.f;//

				PaceNote o(1, pos, s,s*l, 1,1,1,1,  // size, clr
					Adir, 0.f,  n*u, 0.f);  // dir, uv
				Create(o);  vPN.push_back(o);
			}
			#endif
		}
		//  loop signs
		if (n==0)
		{
			bool lp = dirR ? p.loop && !loop1 :
							!p.loop && loop1;
			if (lp)
			{	PaceNote o(1, p.pos, signX,signX, 1,1,1,1,  // size, clr
					0.f/**/, 0.f,  0.f*u, u);  // dir, uv
				Create(o);  vPN.push_back(o);
			}
			loop1 = p.loop;		
		}
	}
	#endif
	
	LogO(String("::: Time PaceNotes Rebuild: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
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
			#if 0
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
			#endif
	
			
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
					vPN.push_back(n);
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

	n.bb->setRenderQueueGroup(RQG_CarParticles);
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

//  all
void PaceNotes::Destroy()
{
	for (size_t i=0; i < vPN.size(); ++i)
		Destroy(vPN[i]);
	vPN.clear();
	ii = 0;
}


//  update visibility
void PaceNotes::UpdVis()
{
	const Real dd = pApp->pSet->pace_dist, dd2 = dd*dd;
	#ifdef SR_EDITOR
	const int uu = pApp->pSet->pace_show;
	#else
	const int uu = 0;
	#endif

	const Vector3& c = mCamera->getPosition();
	for (size_t i=0; i < vPN.size(); ++i)
	if (vPN[i].use > uu)
		vPN[i].nd->setVisible(false);
	else
	{	const Vector3& p = vPN[i].pos;
		Real dist = c.squaredDistance(p);
		bool vis = dist < dd2;
		vPN[i].nd->setVisible(vis);
	}
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
