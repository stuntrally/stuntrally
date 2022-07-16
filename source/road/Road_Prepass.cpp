#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../vdrift/dbl.h"
#include "Road.h"
#include <OgreTimer.h>
using namespace Ogre;
using std::vector;  using std::min;  using std::max;



///  rebuild for Pace  ~ ~
void SplineRoad::RebuildRoadPace()
{
	Ogre::Timer ti;	
	vPace.clear();

	DataRoad DR(false, true);
	PrepassRange(DR);
	
	LogR("");
	LogR("LOD: -1 pace ---");
	// lod -1 is only for pacenotes data

	DataLod DL;
	StatsLod ST;

	PrepassLod(DR,DL0,DL,ST, -1, false);
	
	DataLodMesh DLM;

	///  Segment
	int sNum = DR.sMax - DR.sMin,
		segM = DR.sMin;

	while (sNum > 0)
	{
		DataSeg DS;
		
		BuildSeg(DR,DL0,DL,ST,DLM, DS, segM, true);
		
		--sNum;  ++segM;  // next
	}

	LogO(String("::: Time Road Pace Rebuild: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


///  Rebuild geometry
//--------------------------------------------------------------------------------------------------------------------------

bool SplineRoad::RebuildRoadInt(bool editorAlign, bool bulletFull)
{

	if (!rebuild && !(editorAlign || bulletFull))  return false;
	rebuild = false;

	updMtrRoadTer();
	UpdRot(); //

	Ogre::Timer ti;	


	DataRoad DR(editorAlign, bulletFull);
	
	PrepassRange(DR);
	
		
	//  full rebuild
	bool full = iDirtyId == -1;
	if (full)
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
	DL0.Clear();
	
	for (int lod = 0; lod < LODs; ++lod)
	{
		LogR("");
		LogR("LOD: "+toStr(lod)+" ---  " +
			(trail ? "trail" : river ? "river" : ""));

		DataLod DL;
		StatsLod ST;

		PrepassLod(DR,DL0,DL,ST, lod, editorAlign);
		
		DataLodMesh DLM;

		///  Segment
		//-----------------------------------
		int sNum = DR.sMax - DR.sMin,
			segM = DR.sMin;

		while (sNum > 0)
		{
			DataSeg DS;
			
			BuildSeg(DR,DL0,DL,ST,DLM, DS, segM, full);
			
			--sNum;  ++segM;  // next
		}
		
		EndStats(DR,ST);
	}
	//-----------------------------
	
	
	UpdLodVis(fLodBias);
	if (full)
		iOldHide = -1;


	if (full)
		LogO(String("::: Time Road Rebuild: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");

	return full;
}


//  Prepass Range
//---------------------------------------------------------------------------------------
void SplineRoad::PrepassRange(DataRoad& DR)
{
	//  segments range
	DR.segs = getNumPoints();
	DR.sMin = 0;  DR.sMax = DR.segs;
	if (DR.segs == 0 || DR.segs == 1)  return;

	
	if (vSegs.size() != DR.segs || DR.editorAlign || DR.bulletFull)
		iDirtyId = -1;  // force full
		
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

///par []()  pacenotes prepass
const int pace_iDiv = 4, pace_iW = 2;
const float pace_fLen = 12.f;


void SplineRoad::PrepassLod(
	const DataRoad& DR,
	DataLod0& DL0, DataLod& DL, StatsLod& ST,
	int lod, bool editorAlign)
{
	DL.lod = lod;
	DL.isLod0 = lod == 0;
	DL.isPace = lod == -1;

	int iLodDiv = DL.isPace ? pace_iDiv :  ciLodDivs[lod];
	DL.fLenDim =  DL.isPace ? pace_fLen :  g_LenDim0 * iLodDiv;
	DL.tcLen = 0.f;
	int inLoop = 0;
	int oldCk = -1;
		
	//if (isLod0)?
	LogR("--- Lod segs prepass ---");
	for (int seg=0; seg < DR.segs; ++seg)
	{
		int seg1 = getNext(seg), seg0 = getPrev(seg);

		//  width steps  pipe  --
		Real sp = mP[seg].pipe, sp1 = mP[seg1].pipe, sp0 = mP[seg0].pipe;
		Real p = sp * g_P_iw_mul, pl = max(sp, sp1)* g_P_iw_mul/4;
		if (p < 0.f)  p = 1.f;  else  p = 1.f + p;
		if (pl< 0.f)  pl= 1.f;  else  pl= 1.f + pl;
		bool pipe = sp > 0.f || sp1 > 0.f;
		//int wmin = pipe ? 5 : 1;  // min w steps  //par
		
		//  road  --
		int iw = DL.isPace ? pace_iW :
			max(1/*wmin*/, (int)(p * g_iWidthDiv0 / iLodDiv));  //* wid/widDiv..
		DL.v_iW.push_back(iw);
		int iwl = max(1, (int)(pl * g_iWidthDiv0 / iLodDiv));

		//  length steps  |
		Real len = GetSegLen(seg);
		int  il = int(len / DL.fLenDim) / iwl * iwl + iwl;
		Real lenAdd = 1.f / il;
		
		DL.v_iL.push_back(il);
		DL.v_len.push_back(len);

		if (!mP[seg].notReal)
		{	//  add len
			ST.roadLen += len;  //#
			if (pipe)
			{	ST.rdPipe += len; //#
				if (mP[seg].onPipe)  ST.rdOnPipe += len;  //#
		}	}

		///-  Merge conditions
		DL.sumLenMrg += len;
		//  mtr changes
		int hid = mP[seg].idMtr, hid1 = mP[seg1].idMtr, hid0 = mP[seg0].idMtr;

		if (IsTrail())
		{
			int ck = mP[seg].nCk;
			bool mrg = oldCk != ck;  oldCk = ck;
			
			DL.sumLenMrg = 0.f;  ++DL.mrgCnt;
			DL.v_bMerge.push_back(mrg ? 1 : 0);
		}
		//  merge road and pipe segs, don't merge transitions
		else if (sp != sp1 || sp != sp0  ||  hid != hid1 || hid != hid0)
		{	DL.sumLenMrg = 0.f;  ++DL.mrgCnt;
			DL.v_bMerge.push_back(1);
		}
		else  //  onTer change
		if (mP[seg].onTer != mP[seg1].onTer || mP[seg].onTer != mP[seg0].onTer)
		{	DL.sumLenMrg = 0.f;  ++DL.mrgCnt;
			DL.v_bMerge.push_back(1);
		}
		else  if (DL.sumLenMrg >= g_MergeLen)
		{	DL.sumLenMrg -= g_MergeLen;  ++DL.mrgCnt;
			DL.v_bMerge.push_back(1);  // bNew
		}else
			DL.v_bMerge.push_back(0);  // merged
		
		LogR("seg " + iToStr(seg,3) + "  iw " + iToStr(iw,3) + "  il " + iToStr(il,3) +
			"  pipe prv" + toStr(sp0) + "  cur " + toStr(sp) + "  nxt" + toStr(sp1) +
			(trail ? "  ck " + toStr(mP[seg].nCk) : "") +
			(DL.v_bMerge[DL.v_bMerge.size()-1] ? "  mrg" : ""));
		
		if (DL.isLod0)
		{	
			int l = mP[seg].loop;  // type
			if (l > 0)	///[]()
			if (inLoop == 0)  inLoop = l;
			else  inLoop = 0;

			DL0.v0_iL.push_back(il);
			DL0.v0_Loop.push_back(inLoop);
		}

		///  length <dir>  |
		Vector3 vl = GetLenDir(seg, 0, lenAdd), vw;  vl.normalise();
		Real ay = mP[seg].aYaw, ar = mP[seg].aRoll;
		
		///  width <dir>   ---
		if (mP[seg].onTer && mP[seg1].onTer)  //  perpendicular on xz
		{	vw = Vector3(vl.z, 0, -vl.x);  vw.normalise(); 
		}else
			vw = GetRot(ay,ar);  // from angles
			
		///  normal <dir>  /
		if (DL.isLod0)
		{	Vector3 vn = vl.crossProduct(vw);  vn.normalise();
			//  on pipe inv
			if (mP[seg].onPipe==2)
				vn = -vn;
			DL0.v0_N.push_back(vn);
		}

		//  width  -
		{
		Real wiMul = mP[seg].width;
		if (editorAlign)  // wider road for align terrain tool
			wiMul = wiMul*ed_Wmul + ed_Wadd;
		vw *= wiMul;
		DL.v_W.push_back(vw);
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
		DL.v_tc.push_back(DL.tcLen);
		if (DL.isLod0)
			DL0.v0_tc.push_back(DL.tcLen);
	}

	
	LogR("--- seg prepass2  viwLS  ---");
	for (int seg=0; seg < DR.segs; ++seg)
	{
		int seg1 = getNext(seg);
		int il = DL.v_iL[seg];
		std::vector<int> viwL;

		//  width steps per length point in cur seg
		int iw0 = DL.v_iW[seg], iw1 = DL.v_iW[seg1];
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

		DL.v_iwEq.push_back(eq);
		DL.v_iWL.push_back(viwL);
		//if (!eq)  vbSegMrg[seg] = 1;
		//LogR("seg "+toStr(seg)+"  >> "+ss);
	}

	//  stats done at lod 0 and full rebuild
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
