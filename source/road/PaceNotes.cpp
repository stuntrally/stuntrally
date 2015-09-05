#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/Axes.h"
#include "../ogre/common/data/SceneXml.h"
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
#include "tinyxml.h"
#include "tinyxml2.h"
using namespace tinyxml2;
//#include <OgreTerrain.h>
using namespace std;
using namespace Ogre;


//  ctor  ---------
PaceNote::PaceNote()
	:nd(0), bb(0),bc(0), pos(0,0,0), use(1), id(0)
	,size(4.f,4.f), clr(0,0,0,0), ofs(0,0), uv(0,0)
	,start(0), jump(0), vel(0.f)
{	}
PaceNote::PaceNote(int i, int t, Vector3 p, float sx,float sy,
		float r,float g,float b,float a, float ox,float oy, float u,float v)
	:nd(0), bb(0),bc(0), pos(p), use(t), id(i)
	,size(sx,sy), clr(r,g,b,a), ofs(ox,oy), uv(u,v)
	,start(0), jump(0), vel(0.f)
{	}


//  Pace notes
//--------------------------------------------------------------------------------------
void PaceNotes::Rebuild(SplineRoad* road, Scene* sc, bool reversed)
{
	Ogre::Timer ti;	

	Destroy();

	//  const, vis
	const float u=0.125f,
		barX=1.f,barY=6.f, barA=0.6f,
		use1X=1.4f, use1A=1.f, useX=0.9f, useA=0.7f,

#ifdef SR_EDITOR
	#define USED
	#define BARS
	signX = 4.f;  // ed
#else
	signX = 2.f;  // game
#endif


	///  Simple Turns
	///  ~~~  params  ~~~
	const int nn = 8;  // levels +1, 0 fake        // turn sharpness,  max single angle in rad
	const float angN[nn] = {0.02f, 0.06f, 0.2f, 0.3f, 0.4f, 0.5f, 0.7f, 1.0f};
	const int Radd[nn]	 = {2,     2,     1,    1,    0,    0,    0,    0};  // needed for sign
	const int Rlen[nn]	 = {10,    10,    10,   8,    8,    7,    7,    6};  // road search range
	const float  ///par
		angStr = 0.01f,  /// 0.01 straight angle
		angMul = 0.7f,   /// 0.7 angN scale
		sustain = 0.34f, /// 0.3-0.4  sustain current turn, more splits if higher
		LenLong = 60.f;  /// 60  above this length turn is long

	struct PNote  // total angle sum, tex uv, color
	{	float ang;  int iu,iv;  float h,s,v;  };
	
	const static PNote arPN[nn][2] =
	{			// short						// long					
		{{150.f, 6,7, 0.00f, 1.f, 0.5f},{150.f, 6,6, 0.00f, 0.5f, 0.5f}},
		{{120.f, 6,7, 0.00f, 1.f, 1.f}, {120.f, 6,6, 0.05f, 0.5f, 1.f}},
		{{ 90.f, 5,7, 0.05f, 1.f, 1.f}, { 90.f, 5,6, 0.10f, 0.5f, 1.f}},
		{{ 60.f, 4,7, 0.10f, 1.f, 1.f}, { 60.f, 4,6, 0.15f, 0.5f, 1.f}},
		{{ 45.f, 3,7, 0.15f, 1.f, 1.f}, { 45.f, 3,6, 0.20f, 0.5f, 1.f}},
		{{ 30.f, 2,7, 0.20f, 1.f, 1.f}, { 30.f, 2,6, 0.25f, 0.5f, 1.f}},
		{{ 15.f, 1,7, 0.25f, 1.f, 1.f}, { 15.f, 1,6, 0.30f, 0.5f, 1.f}},
		{{  0.f, 0,7, 0.30f, 1.f, 1.f}, {  0.f, 0,6, 0.35f, 0.5f, 1.f}},
	};

	
	//  find start point
	Vector3 posSt = Axes::toOgre(sc->startPos), vSt = posSt;
	float stDist = FLT_MAX;
	int ist = -1;
	iStart = 0;  iAll = 1;
	iDir = reversed ? -road->iDir : road->iDir;


	///  Trace Road  |||  prepass
	int ii = road->vPace.size();
	for (int i=0; i < ii; ++i)
	{
		SplineRoad::PaceM& p = road->vPace[i],
			prv = road->vPace[(i-1+ii)%ii], nxt = road->vPace[(i+1)%ii];

		//  closest to trk start
		float dist = p.pos.squaredDistance(posSt);
		if (dist < stDist)
		{	stDist = dist;  ist = i;  vSt = p.pos;  }
		
		//  dir xz
		Vector3 c1 = p.pos - prv.pos, c2 = nxt.pos - p.pos;
		c1.y = 0.f;  c2.y = 0.f;
		c1.normalise();  c2.normalise();

		//  aa - road yaw angle
		Vector3 cross = c1.crossProduct(c2);
		Real dot = c1.dotProduct(c2);
		Real aa = acos(dot);
		//Real aa = asin(cross.length());

		//  sign
		Real neg = Vector3::UNIT_Y.dotProduct(cross);
		if (neg < 0.f)  aa = -aa;
		
		///  no loops  no hidden
		if (p.loop || !p.vis)  aa = 0.f;
			
		//LogO(fToStr(aa*180.f/PI_d,1,5));//+" "+fToStr(neg));

		///  no straights  //par 0.01
		if (fabs(aa) < 0.01f)  aa = 0.f;
		p.aa = aa;  // save

		#ifdef BARS  // add dbg bar |
		PaceNote o(i,4, p.pos,  // vis use, pos   // ADD dbg
			barX,barY, 1,1,1,barA,  // size, clr
			aa < 0.f ? 0.f : 1.f, 0.5f,  // dir, bar width
			aa < 0.f ? 7.5f*u : 7.f*u,  // u
			u + fabs(aa)*0.6f);  // v  //par scale 0.3-1.2
		Create(o);  vPN.push_back(o);
		#endif
	}


	//  start sign  ~~
	if (ist==-1)
		LogO("!! Pace start pos NOT found!");
	else
	{	//ist = road->vPace[ist].id;
		LogO("Pace start pos: "+toStr(ist)+"/"+toStr(ii)+"  dist: "+fToStr(sqrt(stDist)));
	}

	PaceNote o(ist,1, vSt, signX,signX, 1,1,1,use1A,  // ADD start
		0.f, 1.f,  1.f*u, 2.f*u);  o.start = 1;
	Create(o);  vPN.push_back(o);

	
	//  old prev vals
	bool loop1 = false, jump1 = false, jump1R = false, onpipe1 = false;
	bool dirR = road->iDir > 0;  // road dir
	if (reversed)  dirR = !dirR;  // track dir
	
	//  jump signs, set vel in trk gho
	std::vector<int> vJ,vJe;  // id for vPN


///  ~~~  Auto Gen. turn signs  ~~~
	int i,n, n1=1;
	for (n=nn-1; n >= n1; --n)  // all levels, 0 fake
	for (i=0; i < ii; ++i)     // all road points
	{
		const PNote& PD = arPN[nn-1-n][0];  // dbg
		ColourValue c;  c.setHSB(PD.h, PD.s, PD.v);
		
		SplineRoad::PaceM& p = road->vPace[i];
		if (fabs(p.aa) > angN[n]*angMul && p.used < 0)
		{
			///  Get Neighbors  ~~~
			//  staying in range, not below sustain * original angle
			const int ri = Rlen[n];
			float am = angN[n-1]*angMul * sustain;
			bool dir = dirR ? p.aa > 0.f : p.aa < 0.f;
			float Adir = dir ? 0.f : 1.f;

			Vector3 pos = p.pos, prv = pos;  // main sign pos
			float Asum = p.aa, Lsum = 0.f;

			#ifdef USED  // add used start
			///  type, pos | size, clr | dir, bar, u,v
			PaceNote o(i,2, p.pos2, use1X,use1X, c.r,c.g,c.b,use1A,  // ADD dbg
				Adir, 0.f,  PD.iu *u, PD.iv *u);
			Create(o);  vPN.push_back(o);
			#endif

			int r=1, rr=0, radd=0, rsub=0;
			bool ok = true;
			while (ok && rr < ri)  // search next  ++
			{
				SplineRoad::PaceM& pp = road->vPace[(i+r)%ii];
				ok = pp.used < 0 && (p.aa > 0.f && pp.aa > am || p.aa < 0.f && pp.aa <-am);
				if (ok)
				{
					pp.used = n;  ++radd;
					Asum += pp.aa;  Lsum += prv.distance(pp.pos);
					if (!dirR)  pos = pp.pos;  // back pos

					#ifdef USED  // add used
					PaceNote o(i,3, pp.pos2, useX,useX, c.r,c.g,c.b,useA,  // ADD dbg
						Adir, 0.f,  PD.iu *u, PD.iv *u);
					Create(o);  vPN.push_back(o);
					#endif
				}
				++rr;  ++r;  prv = pp.pos;
			}
			r=1;  rr=0;  ok = true;
			while (ok && rr < ri)  // search prev  --
			{
				SplineRoad::PaceM& pp = road->vPace[(i+r+ii)%ii];
				ok = pp.used < 0 && (p.aa > 0.f && pp.aa > am || p.aa < 0.f && pp.aa <-am);
				if (ok)
				{
					pp.used = n;  ++rsub;
					Asum += pp.aa;  Lsum += prv.distance(pp.pos);
					if (dirR)  pos = pp.pos;  // back pos

					#ifdef USED  // add used
					PaceNote o(i,3, pp.pos2, useX,useX, c.r,1-c.g,1-c.b,useA,  // ADD dbg
						Adir, 0.f,  PD.iu *u, PD.iv *u);
					Create(o);  vPN.push_back(o);
					#endif
				}
				++rr;  --r;  prv = pp.pos;
			}
			
			///  Add Turn  ~~~  ~ ~ ~
			int rsad = rsub + radd;
			if (rsad > Radd[n])
			{
				p.used = n;

				float ang = fabs(Asum) *180.f/PI_d;  // total turn yaw in degrees
				int iLong = Lsum > LenLong ? 1 : 0;  // long turn

				//  find sign by angle in table
				bool ff = true;  int ai = 0;
				while (ff  && ai < nn)
				{
					if (ang >= arPN[ai][iLong].ang)
					{	ff = false;  }
					else  ++ai;
				}
				const PNote& PN = arPN[ai][iLong];
				
				#if 0
				LogO("n "+toStr(n)+
					"  A "+fToStr(Asum*180.f/PI_d, 1,6)+
					"  L "+fToStr(Lsum, 1,5)+
					"  AL "+fToStr(Asum*180.f/PI_d / Lsum, 1,5)+
					"  u "+iToStr(PN.iu,1)+"  v "+iToStr(PN.iv,1)+
					"  h "+fToStr(PN.h)+"  s "+fToStr(PN.h)+"  v "+fToStr(PN.v));
				#endif

				ColourValue c;  c.setHSB(PN.h, PN.s, PN.v);
				
				PaceNote o(i,1, pos,  signX,signX, c.r,c.g,c.b,1,  // ADD
					Adir, 0.f,  PN.iu *u, PN.iv *u);  // uv
				Create(o);  vPN.push_back(o);
			}
		}

		if (n==n1)
		{
			///~~~  Loop
			bool lp = dirR ? p.loop && !loop1 :
							!p.loop && loop1;
			if (lp)
			{
				//  find loop end  //todo: loop type?
				#if 0
				int r=3, rr=0, radd=1, ri = 40;  // max loop len
				int rs = dirR ? 1 : -1;
				SplineRoad::PaceM* pe = &p;
				
				bool ok = true;
				while (ok && rr < ri)  // search next/prev
				{
					int id = (i+r*rs+ii)%ii;
					pe = &road->vPace[id];
					LogO("Loop: "+iToStr(id)+(pe->loop?"+":"-")+" "+fToStr(pe->aa));
					ok = pe->loop;
					if (ok)  ++radd;
					++rr;  ++r;
				}
				PaceNote q(j,2, pe->pos, signX,signX, 0.6,0.8,1,0.6,  // ADD dbg
					0.f, 0.f,  1.f*u, 5.f*u);
				Create(q);  vPN.push_back(q);
				LogO("Loop: "+iToStr(radd));
				#endif

				ColourValue c;  c.setHSB(0.55f, 0.7f, 1.f);
				PaceNote o(i,1, p.pos, signX,signX, c.r,c.g,c.b,1,  // ADD
					0.f, 0.f,  0.f*u, 5.f*u);
				Create(o);  vPN.push_back(o);
			}
			loop1 = p.loop;

			///~~~  Jump
			bool jmp1  = dirR ? p.jumpR : p.jump;
			bool jmp1R = dirR ? p.jump : p.jumpR;
			bool jump = jmp1  && !jump1;   // jump/start
			bool land = jmp1R && !jump1R;  // land/end
			if (jump || land)
			{
				ColourValue c;  c.setHSB(land? 0.57f: 0.56f, land? 0.5f: 1.f, land? 0.9f: 1.f);
				PaceNote o(i,1, p.pos, signX,signX, c.r,c.g,c.b,1,  // ADD
					land? 0.f: 1.f, 0.f,  1.f*u, 4.f*u);
				o.jump = land ? 2 : 1;
				Create(o);  vPN.push_back(o);
				(jump ? vJ : vJe).push_back(vPN.size()-1);
			}

			jump1 = jmp1;  jump1R = jmp1R;  

			///~~~  On Pipe
			bool onp = dirR ? p.onpipe && !onpipe1 :
							 !p.onpipe && onpipe1;
			if (onp)
			{	PaceNote o(i,1, p.pos, signX,signX, 1,1,1,1,  // ADD
					0.f, 0.f,  4.f*u, 2.f*u);
				Create(o);  vPN.push_back(o);
			}
			onpipe1 = p.onpipe;
		}
	}

	
	///:  only real signs
	#ifndef SR_EDITOR  // game
	vPS.clear();
	for (i=0; i < vPN.size(); ++i)
		if (vPN[i].use == 1)
			vPS.push_back(vPN[i]);
	
	std::sort(vPS.begin(), vPS.end(), PaceSort);
	
	///:  find start
	iAll = vPS.size();
	for (i=0; i < iAll; ++i)
	{	if (vPS[i].start)
			iStart = i;
		//LogO("SS "+toStr(vPS[i].id));
	}
	iCur = iStart;
	#endif

	if (vJ.size() != vJe.size())
		LogO("Pace Jumps != JumpEnds");  //j
	
	LogO(String("::: Time PaceNotes Rebuild1: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	


///  ~~~  trace Track's Ghost  ~~~

	//  util dist to jump points
	std::vector<float> vJd;
	vJd.resize(vJ.size(), FLT_MAX);
	
	bool rev = false;
	string sRev = rev ? "_r" : "";
	#ifdef SR_EDITOR
	string track = pApp->pSet->gui.track;
	#else
	string track = pApp->pSet->game.track;
	#endif
	
	//  load
	string file = PATHMANAGER::TrkGhosts()+"/"+ track + sRev + ".gho";
	if (!PATHMANAGER::FileExists(file))
	{	LogO("Pace trk gho not found: "+file);/**/  }
	else
	{	LogO("---------  "+track+"  ---------");
		TrackGhost gho;
		gho.LoadFile(file);
		
		//  test
		Vector3 pos,oldPos;  float oldTime = 0.f;
		Quaternion rot;  float vel = 0.f;
		int num = gho.getNumFrames();

		for (i=0; i < num; ++i)
		{
			//  pos
			const TrackFrame& fr = gho.getFrame0(i);
			Axes::toOgre(pos, fr.pos);  // pos
			Vector3 pp = pos - oldPos;

			//  vel
			float dist = pp.length();
			float dt = fr.time - oldTime;  // 0.04
			if (i > 0 && i < num-1 && dt > 0.001f)
				vel = 3.6f * dist / dt;

			//if (vel < 20)  y *= vel / 20.f;  // y sc

			//todo: ter jmp, bumps cast ray down to ter..
			//mTerrain->getHeightAtWorldPosition

			
			//  log  ----
			#if 0
			LogO("i:"+ iToStr(i,4) +" t:"+ fToStr(fr.time,2,6)//+" dt: "+fToStr(dt)
				+"  v:"+ fToStr(vel,0,4)
				+"  b:"+ toStr(fr.brake)
				+"  s:"+ (fr.steer==0 ? " ---" : fToStr(fr.steer/127.f,1,4))
				//+"  a:"+ fToStr(sa,1,4)+"  y:"+ fToStr(y,1,4)
				+(jmp ? " !jd: "+fToStr(dist) : "")
			);
			#endif
			
			//~~~  check all jumps for dist
			for (int j=0; j < vJ.size(); ++j)
			{
				PaceNote& p = vPN[vJ[j]];
				float d = pos.squaredDistance(p.pos);
				if (d < vJd[j])
				{	vJd[j] = d;
					p.vel = vel;
					//LogO("j "+toStr(j)+"  i "+toStr(vJ[j])+"  v "+fToStr(vel));  //j
				}
			}
			
			//  pos marks . .
			if (i%6==0)
			{
				//fr.brake  fr.steer
				PaceNote o(1000,5, pos, useX,useX, 1,1,1,1,  // ADD dbg
					0.f, 0.f,  1.f*u, 1.f*u);
				Create(o);  vPN.push_back(o);
			}
			
			oldPos = pos;  oldTime = fr.time;
		}
	}

	///  upd Jumps vel  ~~~
	LogO("== jump "+toStr(vJ.size())+" land "+toStr(vJe.size()));  //j
	size_t s = std::min(vJ.size(), vJe.size());
	for (i=0; i < s; ++i)
	{
		PaceNote& p = vPN[vJ[i]], pe = vPN[vJe[i]];
		float len = p.pos.distance(pe.pos);
		int l = std::min(2.f, len / 60.f);
		bool land = p.jump == 2;

		p.uv.x = l*u;  // UPD
		Update(p);
		//o.txt = 

		LogO("jump "+toStr(i)+"  vel "+fToStr(p.vel)+"  len "+fToStr(p.vel));  //j
	}

	LogO(String("::: Time PaceNotes Rebuild2: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
//--------------------------------------------------------------------------------------


//  Create
void PaceNotes::Create(PaceNote& n)
{
	//if (n.use == 1)
	//	LogO("PP "+toStr(n.id)+(n.start?" ST":""));
	++ii;
	n.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	n.bb = mSceneMgr->createBillboardSet("P-"+toStr(ii),2);
	n.bb->setDefaultDimensions(n.size.x, n.size.y);

	n.bb->setRenderQueueGroup(RQG_CarParticles);
	n.bb->setVisibilityFlags(RV_Car);

	n.bb->setCustomParameter(0, Vector4(n.ofs.x, n.ofs.y, n.uv.x, n.uv.y));  // params, uv ofs
	n.bc = n.bb->createBillboard(Vector3(0,0,0), ColourValue(n.clr.x, n.clr.y, n.clr.z, n.clr.w));

	n.bb->setMaterialName("pacenote");
	n.nd->attachObject(n.bb);
	n.nd->setPosition(n.pos);
	n.nd->setVisible(false);
}

void PaceNotes::Update(PaceNote& n)
{
	n.bc->setColour(ColourValue(n.clr.x, n.clr.y, n.clr.z, n.clr.w));
	n.bb->setCustomParameter(0, Vector4(n.ofs.x, n.ofs.y, n.uv.x, n.uv.y));  // params, uv ofs
}

//  Destroy
void PaceNotes::Destroy(PaceNote& n)
{
	//if (txt)  
	mSceneMgr->destroyBillboardSet(n.bb);  n.bb = 0;
	mSceneMgr->destroySceneNode(n.nd);  n.nd = 0;
}

//  all
void PaceNotes::Destroy()
{
	for (size_t i=0; i < vPN.size(); ++i)
		Destroy(vPN[i]);
	vPN.clear();
	vPS.clear();
	ii = 0;
}


//  update visibility  ---------
void PaceNotes::UpdVis(Vector3 carPos, bool hide)
{
	const Real dd = pApp->pSet->pace_dist, dd2 = dd*dd;

	const Vector3& c = mCamera->getPosition();
	int i,s;

	//static int xx=0;  ++xx;  // inc test
	//if (xx > 10) {  xx=0;  iCur++;  if (iCur>=iAll)  iCur-=iAll;  }
	//LogO(toStr(iCur));

	///todo: cd. jump vel, loop side-, onpipe key8
	///  strict jfw not: hid, onpipe, under ter
	///  reset car pos iCur, prev chk 0
	
#ifndef SR_EDITOR
	//  game  ----
	const int rng = pApp->pSet->pace_next;  // vis next count
	const float radiusA = 9.f*9.f;  //par pace sphere radius

	s = vPS.size();
	for (i=0; i < s; ++i)
	{
		PaceNote& p = vPS[i];
		bool vis = pApp->pSet->pace_show;

		//  Advance to next sign  ~ ~ ~
		//        a    iCur   s-1  s=7
		//  0  1  2  3  4  5  6   -id
		bool vrng = iDir > 0 ?  // inside cur..cur+range with cycle
			(i >= iCur && i <= iCur+rng || i < iCur+rng-iAll) :
			(i <= iCur && i >= iCur-rng || i > iCur-rng+iAll);
		vis &= vrng;

		if (vrng)
		{	float d = p.pos.squaredDistance(carPos);
			if (d < radiusA && i != iCur)  // close next only
			{
				//LogO("iCur "+iToStr(i,3)+"  d "+fToStr(sqrt(d))+"  <> "+iToStr(i-iCur));
				iCur = i;
		}	}
		p.nd->setVisible(vis);
	}
#else  //  ed  ----
	s = vPN.size();
	if (hide)
	{	for (i=0; i < s; ++i)
			vPN[i].nd->setVisible(false);
		return;
	}
	for (i=0; i < s; ++i)
	{
		PaceNote& p = vPN[i];
		bool vis = p.use <= pApp->pSet->pace_show;

		const Vector3& o = p.pos;
		Real dist = c.squaredDistance(o);
		bool vnear = dist < dd2;
		vis &= vnear;

		p.nd->setVisible(vis);
	}
#endif
}


//  ctor  ---------
PaceNotes::PaceNotes(App* papp) :pApp(papp)
	,mSceneMgr(0),mCamera(0),mTerrain(0)
	,ii(0), iStart(0),iAll(1), iDir(1), iCur(0)
{	}

//  setup
void PaceNotes::Setup(SceneManager* sceneMgr, Camera* camera, Terrain* terrain)
{
	mSceneMgr = sceneMgr;  mCamera = camera;  mTerrain = terrain;
}


///  Load
//------------------------------------------------------------------------------------------
bool PaceNotes::LoadFile(String fname)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(fname.c_str());
	if (e != XML_SUCCESS)  return false;
		
	XMLElement* root = doc.RootElement(), *n = NULL;
	if (!root)  return false;
	const char* a = NULL;
	
	vPN.clear();

	n = root->FirstChildElement("gen");	if (n)  {
		//a = n->Attribute("len");	if (a)  g_LenDim0 = s2r(a);
	}

	n = root->FirstChildElement("P");	//  points
	while (n)
	{
		PaceNote p;
		a = n->Attribute("p");  p.pos = s2v(a);
		a = n->Attribute("c");  p.clr = s2v4(a);
		a = n->Attribute("s");  p.size = s2v2(a);
		a = n->Attribute("o");  p.ofs = s2v2(a);
		a = n->Attribute("u");  p.uv = s2v2(a);

		//  Add point
		vPN.push_back(p);
		n = n->NextSiblingElement("P");
	}
	return true;
}

///  Save
//------------------------------------------------------------------------------------------
bool PaceNotes::SaveFile(String fname)
{
	TiXmlDocument xml;	TiXmlElement root("Pacenotes");
	
	int i, num = vPN.size();
	for (i=0; i < num; ++i)
	{
		TiXmlElement n("P");
		{
			const PaceNote& p = vPN[i];

			n.SetAttribute("p",	toStrC( p.pos ));
			n.SetAttribute("c",	toStrC( p.clr ));
			n.SetAttribute("s",	toStrC( p.size ));
			n.SetAttribute("o",	toStrC( p.ofs ));
			n.SetAttribute("u",	toStrC( p.uv ));
		}
		root.InsertEndChild(n);
	}
	
	xml.InsertEndChild(root);
	return xml.SaveFile(fname.c_str());
}
