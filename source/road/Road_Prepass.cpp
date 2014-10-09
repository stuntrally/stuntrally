#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/ShapeData.h"
#include "../vdrift/dbl.h"
#include "Road.h"
#ifndef SR_EDITOR
	#include "../vdrift/game.h"
#else
	#include "../editor/CApp.h"
	#include "../bullet/BulletCollision/CollisionShapes/btTriangleMesh.h"
	#include "../bullet/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
	#include "../bullet/BulletCollision/CollisionDispatch/btCollisionObject.h"
	#include "../bullet/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#endif
#include <OgreTimer.h>
#include <OgreTerrain.h>
#include <OgreMeshManager.h>
#include <OgreEntity.h>
using namespace Ogre;
using std::vector;  using std::min;  using std::max;



///  Rebuild geometry
//--------------------------------------------------------------------------------------------------------------------------

void SplineRoad::RebuildRoadInt(bool editorAlign, bool bulletFull)
{

	if (!rebuild && !(editorAlign || bulletFull))  return;
	rebuild = false;

	UpdRot(); //

	Ogre::Timer ti;	


	DataRoad DR(editorAlign,bulletFull);
	
	PrepassRange(DR);
	
		
	//  full rebuild
	if (iDirtyId == -1)
	{
		DestroyRoad();
		for (int seg=0; seg < DR.segs; ++seg)
		{
			RoadSeg rs;  rs.empty = true;
			vSegs.push_back(rs);
		}
	}


	//  Auto angles
	PrepassAngles(DR);


	///  LOD
	//-----------------------------

	DataLod0 DL0;

	for (int lod = 0; lod < LODs; ++lod)
	{
		LogR("LOD: "+toStr(lod)+" ---");

		DataLod DL;
		StatsLod ST;

		PrepassLod(DR,DL0,DL,ST, lod, editorAlign);
		

		///  Segment
		//-----------------------------------

		DataLodMesh DLM;
		DL.tcLen = 0.f;

		int sNum = DR.sMax - DR.sMin,
			segM = DR.sMin;

		while (sNum > 0)
		{
			BuildSeg(DR,DL0,DL,ST,DLM, segM);
			
			--sNum;  ++segM;  // next
		}
		
		EndStats(DR,ST);
	}
	//-----------------------------
	
	
	UpdLodVis(fLodBias);
	if (iDirtyId == -1)
		iOldHide = -1;


	if (iDirtyId == -1)  //if (segs <= 4 || sMax - sMin > 4)
		LogO(String("::: Time Road Rebuild: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


//  Prepass Range
//---------------------------------------------------------------------------------------
void SplineRoad::PrepassRange(DataRoad& DR)
{
	//  segments range
	DR.segs = getNumPoints();
	if (DR.segs == 0 || DR.segs == 1)  return;

	
	if (vSegs.size() != DR.segs || DR.editorAlign || DR.bulletFull)
		iDirtyId = -1;  // force full
		
	DR.sMin = 0;  DR.sMax = DR.segs;
	//  update 4 segs only (fast)
	if (iDirtyId != -1 && DR.segs >= 4)
	{
		DR.sMin = iDirtyId-2;
		DR.sMax = iDirtyId+2;
		if (!isLooped)
			DR.sMin = std::max(0, DR.sMin);
	}
	if (!isLooped)  // 1 seg less
		DR.sMax = std::min(DR.segs-1, DR.sMax);
}

//  Auto Angles Prepass  ~~~
//---------------------------------------------------------------------------------------
void SplineRoad::PrepassAngles(DataRoad& DR)
{
	if (DR.segs > 2)
	for (int seg=0; seg < DR.segs; ++seg)
	{
		int seg0 = getPrev(seg);
				
		if (mP[seg].aType == AT_Manual)
		{	mP[seg].aYaw = mP[seg].mYaw;  mP[seg].aRoll = mP[seg].mRoll;  }
		else
		{	mP[seg].aRoll = 0.f;
			/// ... roll getangle?, +180 loops?, len
			const Real dist = 0.1f;
			Vector3 vl = GetLenDir(seg, 0.f, dist) + GetLenDir(seg0, 1.f-dist, 1.f);  //vl.normalise();
			Vector3 vw = Vector3(vl.z, 0.f, -vl.x);  //vw.normalise();
			mP[seg].aYaw = TerUtil::GetAngle(vw.x, vw.z) *180.f/PI_d;

			if (mP[seg].aType == AT_Both)
			{	mP[seg].aYaw += mP[seg].mYaw;  mP[seg].aRoll += mP[seg].mRoll;  }	
		}
	}
}

///  Prepass LOD,  data for segments
//---------------------------------------------------------------------------------------
const int ciLodDivs[LODs] = {1,2,4,8};

void SplineRoad::PrepassLod(
	const DataRoad& DR,
	DataLod0& DL0, DataLod& DL, StatsLod& ST,
	int lod, bool editorAlign)
{
	DL.lod = lod;
	int iLodDiv = ciLodDivs[lod];
	DL.isLod0 = lod == 0;
	DL.fLenDim = fLenDim0 * iLodDiv;
		
	//if (isLod0)?
	LogR("--- Lod segs prepass ---");
	for (int seg=0; seg < DR.segs; ++seg)
	{
		int seg1 = getNext(seg), seg0 = getPrev(seg);

		//  width steps  --
		Real sp = mP[seg].pipe, sp1 = mP[seg1].pipe, sp0 = mP[seg0].pipe;
		Real p = sp * iwPmul, pl = max(sp, sp1)* iwPmul/4;
		if (p < 0.f)  p = 1.f;  else  p = 1.f + p;
		if (pl< 0.f)  pl= 1.f;  else  pl= 1.f + pl;
		bool pipe = sp > 0.f || sp1 > 0.f;
		int wmin = pipe ? 5 : 1;  // min w steps  //par

		int iw = max(1/*wmin*/, (int)(p * iWidthDiv0 / iLodDiv));  //* wid/widDiv..
		DL.viW.push_back(iw);
		int iwl = max(1, (int)(pl * iWidthDiv0 / iLodDiv));

		//  length steps  |
		Real len = GetSegLen(seg);
		int  il = int(len / DL.fLenDim) / iwl * iwl + iwl;
		Real lenAdd = 1.f / il;
		
		DL.viL.push_back(il);
		DL.vSegLen.push_back(len);

		ST.roadLen += len;  //#
		if (pipe)
		{	ST.rdPipe += len; //#
			if (mP[seg].onPipe)  ST.rdOnPipe += len;  //#
		}

		///-  Merge conditions
		DL.sumLenMrg += len;
		//  mtr changes
		int hid = mP[seg].idMtr, hid1 = mP[seg1].idMtr, hid0 = mP[seg0].idMtr;
		LogR(toStr(sp0) + "  " + toStr(sp) + "  " + toStr(sp1));

		//  merge road and pipe segs, don't merge transitions
		if (sp != sp1 || sp != sp0  ||  hid != hid1 || hid != hid0)
		{	DL.sumLenMrg = 0.f;  ++DL.mrgCnt;
			DL.vbSegMrg.push_back(1);
		}
		else  //  onTer change
		if (mP[seg].onTer != mP[seg1].onTer || mP[seg].onTer != mP[seg0].onTer)
		{	DL.sumLenMrg = 0.f;  ++DL.mrgCnt;
			DL.vbSegMrg.push_back(1);
		}
		else  if (DL.sumLenMrg >= setMrgLen)
		{	DL.sumLenMrg -= setMrgLen;  ++DL.mrgCnt;
			DL.vbSegMrg.push_back(1);  // bNew
		}else
			DL.vbSegMrg.push_back(0);  // merged
		
		LogR("seg "+toStr(seg)+"  iw "+toStr(iw)+"  il "+toStr(il)+"  pp "+toStr(sp));
		
		if (DL.isLod0)
			DL0.viLSteps0.push_back(il);


		///  length <dir>  |
		Vector3 vl = GetLenDir(seg, 0, lenAdd), vw;  vl.normalise();
		Real ay = mP[seg].aYaw, ar = mP[seg].aRoll;
		
		///  width <dir>   ---
		if (mP[seg].onTer && mP[seg1].onTer)  //  perpendicular on xz
		{	vw = Vector3(vl.z, 0, -vl.x);  vw.normalise(); 
			//mP[seg].angle = atan2(vl.z, -vl.x)*180.f/PI_d+90.f;  // set yaw..
		}else
			vw = GetRot(ay,ar);  // from angles
			
		///  normal <dir>  /
		if (DL.isLod0)
		{	Vector3 vn = vl.crossProduct(vw);  vn.normalise();
			//if (vn.y < 0.f)  vn = -vn;  // always up y+
			DL0.vnSeg0.push_back(vn);
		}

		{
		Real wiMul = mP[seg].width;
		if (editorAlign)  // wider road for align terrain tool
			wiMul = wiMul*edWmul + edWadd;
		vw *= wiMul;
		DL.vwSeg.push_back(vw);
		}

		ST.avgWidth += mP[seg].width * len;  //#
		if (!mP[seg].onTer || !mP[seg1].onTer)
			ST.rdOnT += len;  //#


		//  tc  seg il * len
		Real l = 0.f;
		for (int i = 0; i < il; ++i)  // length +1
		{
			//  length dir
			Vector3 vl = GetLenDir(seg, l, l+lenAdd);
			l += lenAdd;  DL.tcLen += vl.length();
		}
		DL.vSegTc.push_back(DL.tcLen);
		if (DL.isLod0)
			DL0.vSegTc0.push_back(DL.tcLen);
	}

	
	LogR("--- seg prepass2  viwLS  ---");
	for (int seg=0; seg < DR.segs; ++seg)
	{
		int seg1 = getNext(seg);
		int il = DL.viL[seg];
		std::vector<int> viwL;

		//  width steps per lenght point in cur seg
		int iw0 = DL.viW[seg], iw1 = DL.viW[seg1];
		//String ss="";
		for (int i = -1; i <= il+1; ++i)  // length +1  +2-gap
		{
			int ii = max(0, min(il, i));
			int iw = iw0 + (int)( Real(ii)/Real(il) * (iw1-iw0) );
			//if (i==0 || i == il)
			//	ss += toStr(iw)+" ";
			viwL.push_back(iw);
		}
		int eq = iw1==iw0 ? 1 : 0;

		DL.viwEq.push_back(eq);
		DL.viwLS.push_back(viwL);
		//if (!eq)  vbSegMrg[seg] = 1;
		//LogR("seg "+toStr(seg)+"  >> "+ss);
	}

	ST.stats = DL.isLod0 && iDirtyId == -1;
	End0Stats(DL,ST);
}


//---------------------------------------------------------------------------------------
void SplineRoad::DataLodMesh::Clear()
{	iLmrg = 0;	iLmrgW = 0;  iLmrgC = 0;  iLmrgB = 0;

	pos.clear();  norm.clear();  tcs.clear();  clr.clear();
	posW.clear(); normW.clear(); tcsW.clear();
	posC.clear(); normC.clear(); tcsC.clear();
	posB.clear(); normB.clear(); tcsB.clear(); clrB.clear();
}


void SplineRoad::End0Stats(const DataLod& DL, const StatsLod& ST)
{
	//#  stats  at lod0, whole road
	if (!ST.stats)  return;

	st.Length = ST.roadLen;  st.WidthAvg = ST.avgWidth / ST.roadLen;
	st.OnTer = ST.rdOnT / ST.roadLen * 100.f;
	st.Pipes = ST.rdPipe / ST.roadLen * 100.f;
	st.OnPipe = ST.rdOnPipe / ST.roadLen * 100.f;
	st.segsMrg = DL.mrgCnt;
}

void SplineRoad::EndStats(const DataRoad& DR, const StatsLod& ST)
{	//#  stats
	if (!ST.stats)  return;

	st.HeightDiff = max(0.f, ST.stMaxH - ST.stMinH);
	st.bankAvg = ST.bankAvg / DR.segs;
	st.bankMax = ST.bankMax;
	//LogO("RD bank angle:  avg "+fToStr(st.bankAvg,1,3)+"  max "+fToStr(st.bankMax,1,3));
}
