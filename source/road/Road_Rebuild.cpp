#include "pch.h"
#include "../ogre/common/Defines.h"
#include "../ogre/common/RenderConst.h"
#include "Road.h"
#ifndef ROAD_EDITOR
#include "../vdrift/game.h"
#endif

#ifdef ROAD_EDITOR
#define LogR(a)  //LogO(String("~ Road  ") + a);
#else
#define LogR(a)
#endif

#include <OgreTerrain.h>
#include <OgreMeshManager.h>
#include <OgreEntity.h>
using namespace Ogre;
#include "../ogre/common/QTimer.h"


//---------------------------------------------------------------------------------------------------------------
Vector3 getNormalAtWorldPosition(Terrain* terrain, Real x, Real z, Real s)
{
	Real y0=0;
	Vector3 vx(x-s, y0, z), vz(x, y0, z-s);
	Vector3 vX(x+s, y0, z), vZ(x, y0, z+s);
	vx.y = terrain->getHeightAtWorldPosition(vx);  vX.y = terrain->getHeightAtWorldPosition(vX);
	vz.y = terrain->getHeightAtWorldPosition(vz);  vZ.y = terrain->getHeightAtWorldPosition(vZ);
	Vector3 v_x = vx-vX;  v_x.normalise();
	Vector3 v_z = vz-vZ;  v_z.normalise();
	Vector3 n = -v_x.crossProduct(v_z);  n.normalise();
	return n;
}

static float GetAngle(float x, float y)
{
	if (x == 0.f && y == 0.f)
		return 0.f;

	if (y == 0.f)
		return (x < 0.f) ? PI_d : 0.f;
	else
		return (y < 0.f) ? atan2f(-y, x) : (2*PI_d - atan2f(y, x));
}

///  Rebuild
//---------------------------------------------------------
void SplineRoad::RebuildRoad(bool full)
{
	rebuild = true;
	if (full)
		iDirtyId = -1;
	else
		iDirtyId = iChosen;
}

void SplineRoad::RebuildRoadInt()
{
	if (!rebuild)  return;
	rebuild = false;

	int segs = getNumPoints();
	if (segs == 0 || segs == 1)  return;
	using std::vector;  using std::min;  using std::max;

	UpdRot(); //
	
	if (vSegs.size() != segs)
		iDirtyId = -1;  // sth wrong, force full
		
	int sMin = 0, sMax = segs;
	//  update 4 segs only (fast)
	if (iDirtyId != -1 && segs >= 4)
	{
		// else !isLooped case ...
		sMin =/* max(0,*/ iDirtyId-2;
		sMax =/* min(segs,*/ iDirtyId+2;
	}
	
	//  full rebuild
	QTimer ti;  ti.update();  /// time	
	
	if (iDirtyId == -1)
	{
		DestroyRoad();
		for (int seg=0; seg < segs; ++seg)
		{
			RoadSeg rs;  rs.empty = true;
			vSegs.push_back(rs);
		}
	}


	///  Auto angles prepass ...
	if (segs > 2)
	for (int seg=0; seg < segs; ++seg)
	{
		//int seg1 = (seg+1) % segs;  // next
		int seg0 = (seg-1+segs) % segs;  // prev
				
		if (mP[seg].aType == AT_Manual)
		{	mP[seg].aYaw = mP[seg].mYaw;  mP[seg].aRoll = mP[seg].mRoll;  }
		else
		{	mP[seg].aRoll = 0.f;
			/// ... roll getangle?, +180 loops?, len
			const Real dist = 0.1f;
			Vector3 vl = GetLenDir(seg, 0.f, dist) + GetLenDir(seg0, 1.f-dist, 1.f);  //vl.normalise();
			Vector3 vw = Vector3(vl.z, 0.f, -vl.x);  //vw.normalise();
			mP[seg].aYaw  = GetAngle(vw.x, vw.z) *180.f/PI_d;

			if (mP[seg].aType == AT_Both)
			{	mP[seg].aYaw += mP[seg].mYaw;  mP[seg].aRoll += mP[seg].mRoll;  }	
		}
	}


	///--------------------------------------------------------------------------------------------------------------------------
	///  LOD
	///--------------------------------------------------------------------------------------------------------------------------

	//>  data at lod 0
	vector<int> viLSteps0;
	vector<Real> vSegTc0;
	vector<Vector3> vnSeg0;  // normals

	const int lodDivs[LODs] = {1,2,4,8};

	for (int lod = 0; lod < LODs; ++lod)
	{
		int lodDiv = lodDivs[lod];
		Real lenDiv = lenDiv0 * lodDiv;
		LogR("LOD: "+toStr(lod)+" ---");


		///  segment data pre pass
		//---------------------------------------------------------------------------------------

		//>  data at cur lod
		vector<int> viL, viW;  // vi num steps for seg Length/Width
		vector<int> vbSegMrg;  // bool 0 if seg merged, 1 if new
		vector<Real> vSegTc, vSegLen;
		vector<Vector3> vwSeg;
		vector<vector <int> > viwLS;  //  width steps per length point, for each seg
		vector<int> viwEq;			// 1 if equal width steps at whole length, in seg

		Real sumLenMrg = 0.f, ltc = 0.f;  int mrgGrp = 0;  //#  stats
		Real roadLen = 0.f, rdOnT = 0.f, rdPipe = 0.f,
			avgWidth = 0.f, stMaxH = FLT_MIN, stMinH = FLT_MAX;
				
		
		//if (lod == 0)?
		LogR("--- seg prepass ---");
		for (int seg=0; seg < segs; ++seg)
		{
			int seg1 = (seg+1) % segs;  // next
			int seg0 = (seg-1+segs) % segs;  // prev

			//  width steps  <->
			Real sp = mP[seg].pipe, sp1 = mP[seg1].pipe, sp0 = mP[seg0].pipe;
			Real p = sp * iwPmul, pl = max(sp, sp1)* iwPmul/4;
			if (p < 0.f)  p = 1.f;  else  p = 1.f + p;
			if (pl< 0.f)  pl= 1.f;  else  pl= 1.f + pl;

			int iw = max(1, (int)(p * iw0 / lodDiv));  //* wid/widDiv..
			viW.push_back(iw);
			int iwl = max(1, (int)(pl * iw0 / lodDiv));

			//  length steps
			Real len = GetSegLen(seg);
			int  il = int(len / lenDiv) / iwl * iwl + iwl;
			Real la = 1.f / il;
			viL.push_back(il);
			vSegLen.push_back(len);
			roadLen += len;  //#
			if (sp > 0.f || sp1 > 0.f)
				rdPipe += len; //#

			//-  Merge length
			sumLenMrg += len;
			//  mtr changes
			int hid = mP[seg].idMtr, hid1 = mP[seg1].idMtr, hid0 = mP[seg0].idMtr;

			//  merge road and pipe segs, don't merge transitions
			LogR(toStr(sp0) + "  " + toStr(sp) + "  " + toStr(sp1));
			if (sp != sp1 || sp != sp0  ||  hid != hid1 || hid != hid0)
			{	sumLenMrg = 0.f;  ++mrgGrp;
				vbSegMrg.push_back(1);
			}
			else  //  onTer change
			if (mP[seg].onTer != mP[seg1].onTer || mP[seg].onTer != mP[seg0].onTer)
			{	sumLenMrg = 0.f;  ++mrgGrp;
				vbSegMrg.push_back(1);
			}
			else  if (sumLenMrg >= setMrgLen)
			{	sumLenMrg -= setMrgLen;  ++mrgGrp;
				vbSegMrg.push_back(1);  // bNew
			}else
				vbSegMrg.push_back(0);  // merged
			
			LogR("seg "+toStr(seg)+"  iw "+toStr(iw)+"  il "+toStr(il)+"  pp "+toStr(sp));
			
			if (lod==0)
				viLSteps0.push_back(il);


			///  length <dir>  |
			Vector3 vl = GetLenDir(seg, 0, la), vw;  vl.normalise();
			Real ay = mP[seg].aYaw, ar = mP[seg].aRoll;
			
			///  width <dir>   ---
			if (mP[seg].onTer && mP[seg1].onTer)  //  perpendicular on xz
			{	vw = Vector3(vl.z, 0, -vl.x);  vw.normalise(); 
				//mP[seg].angle = atan2(vl.z, -vl.x)*180.f/PI_d+90.f;  // set yaw..
			}else
				vw = GetRot(ay,ar);  // from angles
				
			///  normal <dir>  /
			if (lod == 0)
			{	Vector3 vn = vl.crossProduct(vw);  vn.normalise();
				//if (vn.y < 0.f)  vn = -vn;  // always up y+
				vnSeg0.push_back(vn);  }

			vw *= mP[seg].width;
			vwSeg.push_back(vw);

			avgWidth += mP[seg].width * len;  //#
			if (!mP[seg].onTer || !mP[seg1].onTer)
				rdOnT += len;  //#


			//  tcs  seg il* len
			Real l = 0.f;
			for (int i = 0; i < il; ++i)  // length +1
			{
				//  length dir
				Vector3 vl = GetLenDir(seg, l, l+la);
				l += la;  ltc += vl.length();
			}
			vSegTc.push_back(ltc);
			if (lod == 0)  vSegTc0.push_back(ltc);
		}

		
		LogR("--- seg prepass2  viwLS  ---");
		for (int seg=0; seg < segs; ++seg)
		{
			int seg1 = (seg+1) % segs;  // next
			int il = viL[seg];
			vector<int> viwL;

			//  width steps per lenght point in cur seg
			int iw0 = viW[seg], iw1 = viW[seg1];
			String ss="";
			for (int i = -1; i <= il+1; ++i)  // length +1  +2-gap
			{
				int ii = max(0, min(il, i));
				int iw = iw0 + (int)( Real(ii)/Real(il) * (iw1-iw0) );
				if (i==0 || i == il)
					ss += toStr(iw)+" ";
				viwL.push_back(iw);
			}
			int eq = iw1==iw0 ? 1 : 0;
			viwEq.push_back(eq);
			viwLS.push_back(viwL);
			//if (!eq)  vbSegMrg[seg] = 1;
			LogR("seg "+toStr(seg)+"  >> "+ss);
		}

		//#  stats  at lod0, whole road
		bool stats = lod == 0 && iDirtyId == -1;
		if (stats)  {
			st.Length = roadLen;  st.WidthAvg = avgWidth / roadLen;
			st.OnTer = rdOnT / roadLen * 100.f;  st.Pipes = rdPipe / roadLen * 100.f;
			segsMrg = mrgGrp;  }



		//--------------------------------------------------------------------------------------------------------------------------
		///  segment
		//--------------------------------------------------------------------------------------------------------------------------

		//>  mesh data  W-wall  C-column
		vector<Vector4> clr0/*empty*/, clr;
		vector<Vector3> pos,norm, posW,normW, posC,normC, posLod;
		vector<Vector2> tcs, tcsW, tcsC;  Real tc1 = 0;  ltc = 0;
		int iLmrg = 0, iLmrgW = 0, iLmrgC = 0;
		Vector3 vlOld;

		int sNum = sMax - sMin, segM = sMin;//, sNumO = sNum;
		while (sNum > 0)
		{
			int seg = (segM + segs) % segs;  // iterator
			int seg1 = (seg+1) % segs;  // next
			
			//if (lod == 0)
			//LogR("[Seg]  cur: " + toStr(seg) + "/" + toStr(sNumO) + "  all:" + toStr(segs));/**/

			//  on terrain  (whole seg)
			bool onTer = mP[seg].onTer && mP[seg1].onTer;
			
			// on merging segs only for game in whole road rebuild
			// off for editor (partial, 4segs rebuild)
			bool bNew = true, bNxt = true;
			if (bMerge)
			{	bNew = (segM == sMin/*1st*/)	|| vbSegMrg[seg];
				bNxt = (segM+1 == sMax/*last*/) || vbSegMrg[seg1];  // next is new
			}
			
			if (bNew)  //> new seg data
			{	iLmrg = 0;	iLmrgW = 0;  iLmrgC = 0;
				pos.clear();  norm.clear();  tcs.clear();  clr.clear();
				posW.clear(); normW.clear(); tcsW.clear();
				posC.clear(); normC.clear(); tcsC.clear();
			}
			

			///  destroy old
			RoadSeg& rs = vSegs[seg];
			if (!rs.empty && lod == 0)
				DestroySeg(seg);

			
			const int iwW = 7;  // wall  width steps - types..
			const int iwC = colN;  // column  polygon steps
						
			//  steps len
			int il = viL[seg];   Real la = 1.f / il;
			int il0= viLSteps0[seg];  Real la0= 1.f / il0 * skLen;
			Real l = -la0;

			//  width
			//Real wi1 = abs(mP[seg].width), wi2 = abs(mP[seg1].width), wi12 = wi2-wi1;

			///  angles ()__
			Real ay1 = mP[seg].aYaw, ay2 = mP[seg1].aYaw, ay21 = ay2-ay1;
			Real ar1 = mP[seg].aRoll,ar2 = mP[seg1].aRoll,ar21 = ar2-ar1;
			const Real asw = 180;	// more than 180 swirl - wrong at start/end
			while (ay21 > asw)  ay21 -= 2*asw;  while (ay21 <-asw)  ay21 += 2*asw;
			while (ar21 > asw)  ar21 -= 2*asw;	while (ar21 <-asw)  ar21 += 2*asw;

			//  tc begin,range
			Real tcBeg = (seg > 0) ? vSegTc[seg-1] : 0.f,  tcEnd  = vSegTc[seg],  tcRng  = tcEnd - tcBeg;
			Real tcBeg0= (seg > 0) ? vSegTc0[seg-1]: 0.f,  tcEnd0 = vSegTc0[seg], tcRng0 = tcEnd0 - tcBeg0;
			Real tcRmul = tcRng0 / tcRng;


			//------------------------------------------------------------------------------------
			//  Length  vertices
			//------------------------------------------------------------------------------------
			//LogR( " __len");
			if (mP[seg].idMtr >= 0)  // -1 hides segment
			for (int i = -1; i <= il+1; ++i)  // length +1  +2-gap
			{
				++iLmrg;
				///  length <dir>  |
				Vector3 vL0 = interpolate(seg, l);
				Vector3 vl = GetLenDir(seg, l, l+la), vw;
				Real len = vl.length();  vl.normalise();
				
				//  len tc
				if (i <= 0)  ltc = 0;
				Real tc = ltc * tcRmul + tcBeg0;
				//  skirt tc
				if (i == -1)	tc =-skLen* tcRmul + tcBeg0;
				if (i == il+1)  tc = skLen* tcRmul + tcEnd0;
				
				///  width <dir>   --
				if (mP[seg].onTer && mP[seg1].onTer)
				{	vw = Vector3(vl.z, 0, -vl.x);  }
				else  {		/// angles ()__
					Real ay = ay1 + ay21 * l;  // linear-
					Real ar = ar1 + ar21 * l;
					//Real ay = interpAYaw(seg,l);  // spline~
					//Real ar = interpARoll(seg,l);  // err swirl..
					vw = GetRot(ay,ar);  // from angles
				}
				vw.normalise();
				Vector3 vwn = vw;

				//Real wiMul = wi1 + wi12 * l;  // linear
				Real wiMul = interpWidth(seg, l);  // spline~
				vw *= wiMul;

				//  last vw = 1st form next seg		
				if (i==il && seg < segs-1)
					vw = vwSeg[seg+1];
				
				//  on terrain ~~
				bool onTer1 = onTer || mP[seg].onTer && i==0 || mP[seg1].onTer && i==il;

				///  normal <dir>  /
				Vector3 vn = vl.crossProduct(vw);  vn.normalise();
				if (i==0)	vn = vnSeg0[seg];  // seg start=end
				if (i==il)	vn = vnSeg0[seg1];
				//Vector3 vnu = vn;  if (vnu.y < 0)  vnu = -vnu;  // always up y+


				//  width steps <->
				//int iw = viW[seg];
				int iw = viwLS[seg][i+1];  //i = -1 .. il+1

				//  pipe width
				Real l01 = max(0.f, min(1.f, Real(i)/Real(il) ));
				Real p1 = mP[seg].pipe, p2 = mP[seg1].pipe;
				Real pipe = p1 + (p2-p1)*l01;
				bool trans = (p1 == 0.f || p2 == 0.f) && !viwEq[seg];
				Real trp = (p1 == 0.f) ? 1.f - l01 : l01;
				//LogR("   il="+toStr(i)+"/"+toStr(il)+"   iw="+toStr(iw)
				//	/*+(bNew?"  New ":"") +(bNxt?"  Nxt ":"")/**/);
				
				///  road ~    Width  vertices
				//-----------------------------------------------------------------------------------------------------------------
				for (int w=0; w <= iw; ++w)  // width +1
				{
					//  pos create
					Vector3 vP,vN;	Real tcw = Real(w)/Real(iw);

					Real yTer = 0.f;
					if (pipe == 0.f)
					{	//  flat --
						vP = vL0 + vw * (tcw - 0.5);
						vN = vn;
						yTer = mTerrain->getHeightAtWorldPosition(vP.x, 0, vP.z);
						if (onTer1)  //  onTerrain
						{
							vP.y = yTer + fHeight * ((w==0 || w==iw) ? 0.15f : 1.f);
							vN = getNormalAtWorldPosition(mTerrain, vP.x, vP.z, lenDiv*0.5f /*0.5f*/);
						}
					}else
					{	///  pipe (_)
						Real oo = (tcw - 0.5)/0.5 * PI_d * pipe;
						vP = vL0 + vw  * 0.5 * sinf(oo) +
								 + vn * (0.5 - 0.5 * cosf(oo)) * wiMul;
						vN = vn * cosf(oo) + vwn * sinf(oo);
						if (vN.y < 0.f)  vN.y = -vN.y;
						if (trans) {  //  transition from flat to pipe
							vP += vw * (tcw - 0.5) * trp;  }
						yTer = mTerrain->getHeightAtWorldPosition(vP.x, 0, vP.z);
					}
					
					//  skirt, gap patch_
					if (i == -1 || i == il+1)
						vP -= vn * skH;

					//  color - for minimap preview
					//  ---~~~====~~~---
					Real brdg = min(1.f, abs(vP.y - yTer) * 0.4f);  //par ] height diff mul
					Real h = max(0.f, 1.f - abs(vP.y - yTer) / 30.f);  // for grass dens tex
					Real blend = 0.f;  //rand()%1000/1000.f; // TODO: blend 2materials...?
					Vector4 c(brdg,pipe, blend, h);

					//>  data road
					pos.push_back(vP);	norm.push_back(vN);
					tcs.push_back(Vector2(tcw * 1.f /**2p..*/, tc * tcMul));
					clr.push_back(c);
					//#
					if (vP.y < stMinH)  stMinH = vP.y;
					if (vP.y > stMaxH)  stMaxH = vP.y;
				}
				

				///  wall ]
				//------------------------------------------------------------------------------------
				struct stWiPntW {  Real x,y, uv, nx,ny;  };  // wall width points
				const static stWiPntW wiPntW[iwW+1][2] = {  // section shape
					//  normal road                     //  pipe wall
					{{-0.5f, -0.1f, 0.0f,  1.0f, 0.0f}, {-0.28f, 0.7f, 0.0f, -1.0f, 0.0f}},
					{{-0.5f,  1.2f, 0.5f,  0.5f, 0.5f}, {-0.28f, 0.5f, 0.2f, -0.5f, 0.5f}},
					{{-0.56f, 1.2f, 0.2f, -0.5f, 0.5f}, {-0.28f, 0.0f, 0.2f, -0.5f, 0.0f}},
					{{-0.56f,-0.9f, 1.6f, -0.5f,-0.5f}, {-0.2f, -0.9f, 0.5f, -0.1f,-0.5f}},
					{{ 0.56f,-0.9f, 3.0f,  0.5f,-0.5f}, { 0.2f, -0.9f, 0.5f,  0.1f,-0.5f}},
					{{ 0.56f, 1.2f, 1.6f,  0.5f, 0.5f}, { 0.28f, 0.0f, 0.2f,  0.5f, 0.0f}},
					{{ 0.5f,  1.2f, 0.2f, -0.5f, 0.5f}, { 0.28f, 0.5f, 0.2f,  0.5f, 0.5f}},
					{{ 0.5f, -0.1f, 0.5f, -1.0f, 0.0f}, { 0.28f, 0.7f, 0.2f,  1.0f, 0.0f}}};
				Real uv = 0.f;  // tc long
				if (!onTer)
				if (i >= 0 && i <= il)  // length +1
				{	++iLmrgW;
				for (int w=0; w <= iwW; ++w)  // width +1
				{
					int pp = (p1 > 0.f || p2 > 0.f) ? 1 : 0;  //  pipe wall
					stWiPntW wP = wiPntW[w][pp];
					if (trans /*&& (w <= 3 || w >= iwW-3)*/)
					{	wP.x *= 1 + trp;  wP.y *= 1 - trp;  }
					uv += wP.uv;

					Vector3 vP = vL0 + vw * wP.x + vn * wP.y;
					Vector3 vN =     vwn * wP.nx + vn * wP.ny;  vN.normalise();

					//>  data Wall
					posW.push_back(vP);  normW.push_back(vN);
					tcsW.push_back(0.25f * Vector2(uv, tc * tcMul));  //pars
				}	}
				
				
				///  columns |
				//------------------------------------------------------------------------------------
				if (!onTer && mP[seg].cols > 0)
				if (i == il/2)  // middle-
				{	++iLmrgC;
					const Real r = colR;  // column radius
				for (int h=0; h <= 1; ++h)  // height
				for (int w=0; w <= iwC; ++w)  // width +1
				{
					Real ht = (h==0) ? 0.f : vL0.y - mTerrain->getHeightAtWorldPosition(vL0);
					Real a = Real(w)/iwC *2*PI_d,  //+PI_d/4.f
						x = r*cosf(a), y = r*sinf(a);

					Vector3 vlXZ(vl.x,0,vl.z);	Real fl = 1.f/max(0.01f, vlXZ.length());
					Vector3 vP = vL0 + fl * vl * x + vwn * y;
					Real yy;
					if (h==0)  // top below road
					{	yy = vn.y * -0.8f;  //pars
						vP.y += yy;  ht += yy;  }
					else  // bottom below ground
					{	yy = mTerrain->getHeightAtWorldPosition(vP) - 0.3f;
						vP.y = yy;	}
					ht += yy;

					Vector3 vN(vP.x-vL0.x,0,vP.z-vL0.z);  vN.normalise();

					//>  data Col
					posC.push_back(vP);  normC.push_back(vN);
					tcsC.push_back(Vector2( Real(w)/iwC * 4, ht * 0.2f ));  //pars
				}	}
				
				
				if (i == -1 || i == il)  // add len
				{	l += la0;  ltc += len;  }
				else
				{	l += la;  ltc += len;  }
			}
			//  Length  vertices
			//------------------------------------------------------------------------------------
			

			//  lod vis points
			if (lod == 0)
			{	int lps = max(2, (int)(vSegLen[seg] / lposLen));
				for (int p=0; p <= lps; ++p)
				{
					Vector3 vp = interpolate(seg, Real(p)/Real(lps));
					posLod.push_back(vp);
			}	}


			//---------------------------------------------------------------------------------------------------------
			///  create mesh  indices
			//---------------------------------------------------------------------------------------------------------
			if (bNxt && !pos.empty())  /*Merging*/
			{
				String sEnd = toStr(idStr);  ++idStr;
				String sMesh = "rd.mesh." + sEnd, sMeshW = sMesh + "W", sMeshC = sMesh + "C";

				#ifndef ROAD_EDITOR
				posBt.clear();
				#endif
				idx.clear();  // set for addTri
				at_pos = &pos;  at_size = pos.size();  at_ilBt = iLmrg-2;
				
				///  road ~
				int iiw = 0;  //LogR( " __idx");

				//  equal width steps
				if (viwEq[seg]==1)
					for (int i = 0; i < iLmrg-1; ++i)  // length-1 +2gap
					{
						int iw = viW[seg];  // grid  w-1 x l-1 x2 tris
						for (int w=0; w < iw; ++w)  // width-1
						{
							//LogR( "   il="+toStr(i)+"/"+toStr(il)+"   iw="+toStr(iw));
							int f = iiw + w, f1 = f + (iw+1);
							addTri(f+0,f1+1,f+1,i);
							addTri(f+0,f1+0,f1+1,i);
						}
						iiw += iw+1;
					}
				else
				//  pipe, diff width_
				for (int i = 0; i < iLmrg-1; ++i)  // length-1 +2gap
				{
					int iw = viwLS[seg][i], iw1 = viwLS[seg][i+1];
					int sw = iw1 < iw ? 1 : 0;
					//LogR( "   il="+toStr(i)+"/"+toStr(il)+"   iw="+toStr(iw));
					
					for (int w=0; w < iw -sw; ++w)  // width-1
					//int w=0;  // test fans
					{
						int f0 = iiw + w, f1 = f0 + (iw+1);
						//  |\ |  f0+0  f0+1
						//  | \|  f1+0  f1+1
						if (sw==0)  {
							addTri(f0+0,f1+1,f0+1,i);
							addTri(f0+0,f1+0,f1+1,i);
						}else{  // |/|
							addTri(f0+0,f1+0,f0+1,i);
							addTri(f0+1,f1+0,f1+1,i);
						}
					}
					///>>>  fix gaps when iw changes - fan tris
					int ma = iw1 - iw, ms = -ma, m;
					for (m=0; m < ma; ++m)
					{
						int f0 = iiw + iw-1, f1 = f0 + (iw+2)+m;
						addTri(f0+1,f1+0,f1+1,i);
					}
					for (m=0; m < ms; ++m)
					{
						int f0 = iiw + iw-sw -m, f1 = f0 + (iw+1);
						addTri(f0+0,f1+0,f0+1,i);
					}
					iiw += iw + 1;
				}
				vSegs[seg].nTri[lod] = idx.size()/3;


				//  create Ogre Mesh
				//-----------------------------------------
				MeshPtr meshOld = MeshManager::getSingleton().getByName(sMesh);
				if (!meshOld.isNull())  LogR("Mesh exists !!!" + sMesh);

				AxisAlignedBox aabox;
				MeshPtr mesh = MeshManager::getSingleton().createManual(sMesh,"General");
				SubMesh* sm = mesh->createSubMesh();
				
				int id = max(0,mP[seg].idMtr);
				if (isPipe(seg))
					rs.sMtrRd = sMtrPipe[id];
				else
					rs.sMtrRd = sMtrRoad[id] + (onTer ? "_ter" :"");

				CreateMesh(sm, aabox, pos,norm,clr,tcs, idx, rs.sMtrRd);

				MeshPtr meshW, meshC;  // ] |
				bool wall = !posW.empty();
				if (wall)
				{
					meshW = MeshManager::getSingleton().createManual(sMeshW,"General");
					meshW->createSubMesh();
				}
				bool cols = !posC.empty() && lod == 0;  // cols have no lods
				if (cols)
				{
					meshC = MeshManager::getSingleton().createManual(sMeshC,"General");
					meshC->createSubMesh();
				}
				//*=*/wall = 0;  cols = 0;  // test


				///  wall ]
				//------------------------------------------------------------------------------------
				// wall pipe glass mtr
				bool wPglass = isPipe(seg) && mP[seg].idMtr >= 1;  // wall pipe glass mtr
				//bool wPglass = isPipe(seg) && StringUtil::match(sMtrPipe[mP[seg].idMtr], "*lass*");
				if (wall)
				{
					idx.clear();
					for (int i = 0; i < iLmrgW-1; ++i)  // length
					{	int iiW = (i+0)*(iwW+1);
					for (int w=0; w < iwW; ++w)  // width
					{
						int f = iiW + w, f1 = f + (iwW+1);
						idx.push_back(f+1);  idx.push_back(f1+1);  idx.push_back(f+0);
						idx.push_back(f1+1);  idx.push_back(f1+0);  idx.push_back(f+0);
					}	}
					
					//  front plates start,end
					const int Wid[4/*6*/][3] = {{2,1,0},{3,2,0},{5,4,7},{6,5,7}/*,{7,3,0},{4,3,7}*/};
					int i,f, b = posW.size()-iwW-1;
					if (!isPipe(seg))  //  no fronts in pipes
					for (f=0; f < 4; ++f)
					{
						for (i=0; i<=2; ++i)  idx.push_back( Wid[f][i] );
						for (i=0; i<=2; ++i)  idx.push_back( Wid[f][2-i]+b );
					}
					vSegs[seg].nTri[lod] += idx.size()/3;

					sm = meshW->getSubMesh(0);   // for glass only..
					rs.sMtrWall = !wPglass ? sMtrWall : sMtrWallPipe;
					if (!posW.empty())
						CreateMesh(sm, aabox, posW,normW,clr0,tcsW, idx, rs.sMtrWall);
				}
				
				
				///  columns |
				//------------------------------------------------------------------------------------
				if (cols)
				{
					idx.clear();
					at_pos = &posC;
					for (int l=0; l < iLmrgC; ++l)
					for (int w=0; w < iwC; ++w)
					{
						int f = w + l*(iwC+1)*2, f1 = f + iwC+1;
						//idx.push_back(f+0);  idx.push_back(f1+1);  idx.push_back(f+1);
						//idx.push_back(f+0);  idx.push_back(f1+0);  idx.push_back(f1+1);
						addTri(f+0, f1+1, f+1 ,1);
						addTri(f+0, f1+0, f1+1,1);
					}					
					vSegs[seg].nTri[lod] += idx.size()/3;

					sm = meshC->getSubMesh(0);
					//if (!posC.empty())
					CreateMesh(sm, aabox, posC,normC,clr0,tcsC, idx, sMtrCol);
				}
				
								
				//  add Mesh to Scene  -----------------------------------------
				Entity* ent = 0, *entW = 0, *entC = 0;
				SceneNode* node = 0, *nodeW = 0, *nodeC = 0;
					AddMesh(mesh, sMesh, aabox, &ent, &node, "."+sEnd);
				if (wPglass)
				{	ent->setRenderQueueGroup(RQG_PipeGlass);
					//ent->setCastShadows(true);
				}
				if (wall /*&& !posW.empty()*/)
				{	AddMesh(meshW, sMeshW, aabox, &entW, &nodeW, "W."+sEnd);
					entW->setCastShadows(true);  // only cast
				}
				if (cols /*&& !posC.empty()*/)
				{	AddMesh(meshC, sMeshC, aabox, &entC, &nodeC, "C."+sEnd);
					entC->setVisible(true);  
					if (bCastShadow)
						entC->setCastShadows(true);
				}
				if (bCastShadow && !onTer)
					ent->setCastShadows(true);

				
				/**/
				
				//>>  store ogre data  ------------
				rs.road[lod].node = node;	rs.wall[lod].node = nodeW;
				rs.road[lod].ent = ent;		rs.wall[lod].ent = entW;
				rs.road[lod].mesh = mesh;	rs.wall[lod].mesh = meshW;
				rs.road[lod].smesh = sMesh; rs.wall[lod].smesh = sMeshW;
				if (lod==0)  {
					rs.col.node = nodeC;
					rs.col.ent = entC;
					rs.col.mesh = meshC;
					rs.col.smesh = sMeshC;  }
				rs.empty = false;  // new

				//  copy lod points
				if (lod == 0)  {
					for (size_t p=0; p < posLod.size(); ++p)
						rs.lpos.push_back(posLod[p]);
					posLod.clear();  }
			
				//#  stats--
				if (stats)
				{
					rs.mrgLod = (iMrgSegs % 2)*2+1;  //-
					iMrgSegs++;	 // count, full
				}


				///  bullet trimesh  at lod 0
				///------------------------------------------------------------------------------------
				#ifndef ROAD_EDITOR  // in Game
				if (lod == 0)
				{
					btTriangleMesh* trimesh = new btTriangleMesh();  vbtTriMesh.push_back(trimesh);
					#define vToBlt(v)  btVector3(v.x, -v.z, v.y)
					#define addTriB(a,b,c)  trimesh->addTriangle(vToBlt(a), vToBlt(b), vToBlt(c));

					size_t si = posBt.size(), a=0;;  // %3!
					for (size_t i=0; i < si/3; ++i,a+=3)
						addTriB(posBt[a], posBt[a+1], posBt[a+2]);

					// if (cols)  // add columns^..
					
					//  Road  ~
					btCollisionShape* shape = new btBvhTriangleMeshShape(trimesh, true);
					shape->setUserPointer(isPipe(seg) ? (void*)7788 : (void*)7777);  // mark as road,  + mtrId..
					
					//btRigidBody::btRigidBodyConstructionInfo infoT(0.f, 0, shape);
					//infoT.m_restitution = 0.0f;
					//infoT.m_friction = 0.8f;  // 1 like terrain
					//pGame->collision.AddRigidBody(infoT);  // old

					btCollisionObject* bco = new btCollisionObject();
					btTransform tr;  tr.setIdentity();  //tr.setOrigin(pc);
					bco->setActivationState(DISABLE_SIMULATION);
					bco->setCollisionShape(shape);	bco->setWorldTransform(tr);
					bco->setFriction(0.8f);  bco->setRestitution(0.f);  //`
					bco->setCollisionFlags(bco->getCollisionFlags() |
						btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
					pGame->collision.world->addCollisionObject(bco);
					pGame->collision.shapes.push_back(shape);
					
					//  Wall  ]
					if (wall)
					{	trimesh = new btTriangleMesh();  vbtTriMesh.push_back(trimesh);
						
						for (int i = 0; i < iLmrgW-1; ++i)  // length
						{	int iiW = i* (iwW+1);
						for (int w=0; w < iwW; ++w)  // width
						if (bRoadWFullCol || w==0 || w == iwW-1)  // only 2 sides|_| optym+
						{
							int f = iiW + w, f1 = f + (iwW+1);
							addTriB(posW[f+0], posW[f1+1], posW[f+1]);
							addTriB(posW[f+0], posW[f1+0], posW[f1+1]);
						}	}
						
						btCollisionShape* shape = new btBvhTriangleMeshShape(trimesh, true);
						shape->setUserPointer((void*)7777);  //-  + road mtr id todo...
						
						//btRigidBody::btRigidBodyConstructionInfo infoW(0.f, 0, shape);
						//infoW.m_restitution = 0.0f;
						//infoW.m_friction = 0.1f;  // 0 for wall
						//pGame->collision.AddRigidBody(infoW);  // old

						btCollisionObject* bco = new btCollisionObject();
						bco->setActivationState(DISABLE_SIMULATION);
						bco->setCollisionShape(shape);	bco->setWorldTransform(tr);
						bco->setFriction(0.1f);  bco->setRestitution(0.f);  //`
						bco->setCollisionFlags(bco->getCollisionFlags() |
							btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
						pGame->collision.world->addCollisionObject(bco);
						pGame->collision.shapes.push_back(shape);
					}
				}
				#endif

			}/*bNxt Merging*/

			sNum--;  segM++;  // next
		}
		///  segment end
		//--------------------------------------------------------------------------------------------------------------------------
	
		//#  stats
		if (stats)
			st.HeightDiff = max(0.f, stMaxH - stMinH);
	}
	//  lod end

	
	UpdLodVis(fLodBias);
	if (iDirtyId == -1)
		iOldHide = -1;


	if (iDirtyId == -1)
	//if (segs <= 4 || sMax - sMin > 4)
	{
		ti.update();	/// time
		float dt = ti.dt * 1000.f;
		LogO(String("::: Time Road Rebuild: ") + toStr(dt) + " ms");
	}
}


//  add triangle
void SplineRoad::addTri(int f1, int f2, int f3, int i)
{
	/*bool ok = true;  const int fmax = 65530; //16bit
	if (f1 >= at_size || f1 > fmax)  {  LogRE("idx too big: "+toStr(f1)+" >= "+toStr(at_size));  ok = 0;  }
	if (f2 >= at_size || f2 > fmax)  {  LogRE("idx too big: "+toStr(f2)+" >= "+toStr(at_size));  ok = 0;  }
	if (f3 >= at_size || f3 > fmax)  {  LogRE("idx too big: "+toStr(f3)+" >= "+toStr(at_size));  ok = 0;  }
	if (!ok)  return;/**/
	idx.push_back(f1);	idx.push_back(f2);	idx.push_back(f3);
	#ifndef ROAD_EDITOR  // GAME
	if (i > 0 && i < at_ilBt)
	{	posBt.push_back((*at_pos)[f1]);
		posBt.push_back((*at_pos)[f2]);
		posBt.push_back((*at_pos)[f3]);  }
	#endif
}
