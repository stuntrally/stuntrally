#include "pch.h"
#include "../ogre/common/Def_Str.h"
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
#include "../road/SplineBase.h"
#include "../road/Road.h"
#include "../vdrift/pathmanager.h"
#include <OgreTimer.h>
#include <OgreSceneNode.h>
#include <OgreTerrain.h>
using namespace std;
using namespace Ogre;


//  Pace notes
//--------------------------------------------------------------------------------------
void PaceNotes::Rebuild(SplineRoad* road, Scene* sc, bool reversed)
{
	Ogre::Timer ti;	

	Destroy();
	
	if (road->getNumPoints() < 2 || !mTerrain)
		return;

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
		{{150.f, 6,7, 0.00f, 1.f, 0.5f},{150.f, 6,6, 0.00f, 0.7f, 0.7f}},
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
		
		///  no loops  only visible and real
		if (p.loop || !p.vis || p.notReal)  aa = 0.f;
			
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


	///  start sign  ~~
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
	int loop1 = 0;  bool jump1 = false, jump1R = false;
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
			#if 0	//  find loop end
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

				int t = p.loop>0 ? p.loop : loop1;  --t;
				ColourValue c;  c.setHSB(0.55f, 0.7f, 1.f);
				PaceNote o(i,1, p.pos, signX,signX, c.r,c.g,c.b,1,  // ADD
					0.f, 0.f,  t*u, 5.f*u);
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
				o.jump = land ? 2 : 1;  o.text = true;
				Create(o);  vPN.push_back(o);
				(jump ? vJ : vJe).push_back(vPN.size()-1);
			}

			jump1 = jmp1;  jump1R = jmp1R;  

			///~~~  On Pipe
			if (p.onPipe || p.onPipeE)
			{	bool st = reversed ? p.onPipe : p.onPipeE;
				int n = st ? 0 : 2;
				PaceNote o(i,1, p.pos, signX,signX, 0.7,0.5,1,1,  // ADD
					0.f, 0.f,  n *u, 2.f*u);
				Create(o);  vPN.push_back(o);
			}
		}
	}

	LogO(String("::: Time PaceNotes Rebuild1: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
	

///  ~~~  trace Track's Ghost  ~~~

	//  util dist to jump points
	std::vector<float> vJd;
	vJd.resize(vJ.size(), FLT_MAX);
	
	bool rev = false;
	string sRev = rev ? "_r" : "";
	#ifdef SR_EDITOR
	string track = pSet->gui.track;
	#else
	string track = pSet->game.track;
	#endif
	
	//  load
	TrackGhost gho;
	int num = 0;

	string file = PATHMANAGER::TrkGhosts()+"/"+ track + sRev + ".gho";
	if (!PATHMANAGER::FileExists(file))
		LogO("Pace trk gho not found: "+file);/**/
	else
	{	gho.LoadFile(file, false);
		num = gho.getNumFrames();
	}
	
	if (num > 0 && mTerrain)
	{
		Vector3 pos;
		//  terrain height
		float* terH = new float[num];
		for (i=0; i < num; ++i)
		{
			const TrackFrame& fr = gho.getFrame0(i);
			Axes::toOgre(pos, fr.pos);  // pos
			Real yTer = mTerrain->getHeightAtWorldPosition(Vector3(pos.x,0.f,pos.z));
			Real dTer = pos.y - yTer - 0.73f;  //par-
			terH[i] = dTer;
		}
		
		///~~~  terrain jump or bump
		const float g = 0.7f;  //par gray
		const float yH = 0.3f;  //par min ter h diff
		const int ri = 30;  // max len

		for (i=0; i < num; ++i)
		{	int ip = (i-1+num)%num;  //prev

			const TrackFrame& fr = gho.getFrame0(i);
			Axes::toOgre(pos, fr.pos);  // pos
			float aTer = TerUtil::GetAngleAt(mTerrain, pos.x,pos.z, 1.f);
			//LogO(fToStr(aTer));
			
			if (terH[i] > yH && terH[ip] <= yH && aTer < 20.f)  // bump
			{
				//  find closest vPace
				int id = -1, nn = road->vPace.size();
				float dn = FLT_MAX;
				for (int n=0; n < nn; ++n)
				{
					SplineRoad::PaceM& p = road->vPace[n];
					float d = p.pos.squaredDistance(pos);
					if (d < dn)
					{	dn = d;  id = n;  }
				}
				bool onTer = true;
				if (id==-1)  LogO("Pace ter jmp not found closest!");
				else  onTer = road->vPace[id].onTer &&  // close too
						road->vPace[(id+1)%nn].onTer && road->vPace[(id-1+nn)%nn].onTer;
				
				if (!onTer)  continue;  // only on terrain
				
				//  find how long and high
				int r=1, rr=0, radd=0;  float hMax = 0.f;
				bool ok = true;
				while (ok && rr < ri)  // search next  ++
				{
					int ir = (i+r)%num;
					ok = terH[ir] > yH;
					if (ok)
					{	++radd;
						if (yH > hMax)  hMax = yH;
						//Lsum += prv.distance(pp.pos);
						//if (!dirR)  pos = pp.pos;  // back pos

						#ifdef USED  // add used
						if (ir%4==0)
						{
						const TrackFrame& fr = gho.getFrame0(ir);
						Vector3 pp;  Axes::toOgre(pp, fr.pos);  // pos
						//Real yTer = mTerrain->getHeightAtWorldPosition(Vector3(pos.x,0.f,pos.z));
						PaceNote o(id,3, pp, useX,useX, g,g,g,useA,  // ADD dbg
							0.f, 0.f,  0.f*u, 3.f*u);
						Create(o);  vPN.push_back(o);
						}
						#endif
					}
					++rr;  ++r;
				}
				//LogO("TerJmp "+toStr(radd));
				if (radd > 7)  //par min len
				{
					const TrackFrame& fr = gho.getFrame0(i);
					Axes::toOgre(pos, fr.pos);  // pos

					//  check if in mud, allow only water
					float fa = 0.f;  // depth
					const float up = 0.5f;
					int fs = sc->fluids.size();
					for (int fi=0; fi < fs; ++fi)
					{
						const FluidBox& fb = sc->fluids[fi];
						if (fb.pos.y+up - pos.y > 0.f)  // dont check above
						{
							const float sizex = fb.size.x*0.5f, sizez = fb.size.z*0.5f;
							//  check outside rect 2d
							if (pos.x > fb.pos.x - sizex && pos.x < fb.pos.x + sizex &&
								pos.z > fb.pos.z - sizez && pos.z < fb.pos.z + sizez)
							{
								float f = fb.pos.y+up - pos.y;
								if (!fb.deep)  // only waters
								if (f > fa)  fa = f;
							}
						}
					}
					if (fa == 0.f)
					{
						Real yTer = mTerrain->getHeightAtWorldPosition(Vector3(pos.x,0.f,pos.z));
						pos.y = yTer + 3.f;
						int n = radd > 15 ? 1 : 0;
						PaceNote o(id,1, pos, signX,signX, g,g,g,1,  // ADD
							0.f, 0.f,  n*u, 3.f*u);
						Create(o);  vPN.push_back(o);
					}
				}
			}
		}

		///~~~  vel for jumps
		Vector3 oldPos;  float oldTime = 0.f;
		Quaternion rot;  float vel = 0.f;

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
				vel = dist / dt;  // *3.6f

			//  log  ----
			/*LogO("i:"+ iToStr(i,4) +" t:"+ fToStr(fr.time,2,6)//+" dt: "+fToStr(dt)
				+"  v:"+ fToStr(vel,0,4) +"  b:"+ toStr(fr.brake)
				+"  s:"+ (fr.steer==0 ? " ---" : fToStr(fr.steer/127.f,1,4))
			);/**/
			
			//~~~  check all jumps for dist
			for (int j=0; j < vJ.size(); ++j)
			{
				PaceNote& p = vPN[vJ[j]];
				float d = pos.squaredDistance(p.pos);
				if (d < vJd[j])
				{	vJd[j] = d;  p.vel = vel;  }
			}
			
			//  pos marks . .
			#ifdef SR_EDITOR  // ed
			if (i%6==0)  //par
			{
				PaceNote o(9000+i,5, pos, useX,useX, 0.5,1,1,1,  // ADD dbg
					0.f, 0.f,  1.f*u, 1.f*u);
				Create(o);  vPN.push_back(o);
			}
			#endif
			oldPos = pos;  oldTime = fr.time;
		}
		delete[] terH;
	}

	///  upd Jumps vel  ~~~
	if (vJ.size() != vJe.size())
		LogO("Pace jumps "+toStr(vJ.size())+" != "+toStr(vJe.size())+" lands");  //j
	
	int s = std::min(vJ.size(), vJe.size());
	for (i=0; i < s; ++i)
	{
		PaceNote& p = vPN[vJ[i]], pe = vPN[vJe[i]];
		float len = p.pos.distance(pe.pos);
		int l = std::max(0.f, std::min(2.f, (len-60.f) / 40.f));  //par
		bool land = p.jump == 2;

		pe.vel = p.vel;
		pe.uv.x = p.uv.x = l*u;  // UPD
		Update(p);

		LogO((land?"land ":"jump ")+toStr(i)+" id "+toStr(p.id)+"-"+toStr(pe.id)+
			" vel "+fToStr(p.vel*3.6f,0,3)+" len "+fToStr(len,0,3));  //j
	}

	//  move above fluids ~~
	s = vPN.size();
	for (i=0; i < s; ++i)
	{
		PaceNote& p = vPN[i];
		float fa = 0.f;  // depth
		const float up = 3.f;
		for (int fi=0; fi < sc->fluids.size(); ++fi)
		{
			const FluidBox& fb = sc->fluids[fi];
			if (fb.pos.y+up - p.pos.y > 0.f)  // dont check above
			{
				const float sizex = fb.size.x*0.5f, sizez = fb.size.z*0.5f;
				//  check outside rect 2d
				if (p.pos.x > fb.pos.x - sizex && p.pos.x < fb.pos.x + sizex &&
					p.pos.z > fb.pos.z - sizez && p.pos.z < fb.pos.z + sizez)
				{
					float f = fb.pos.y+up - p.pos.y;
					if (f > fa)  fa = f;
				}
			}
		}
		if (fa > 0.f)
		{	p.pos.y += fa;  p.nd->setPosition(p.pos);  }
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
	Reset();
	#endif

	LogO(String("::: Time PaceNotes Rebuild2: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}

void PaceNotes::Reset()
{
	iCur = iStart;
	//iCur = (iStart +iDir+iAll)%iAll;
}
