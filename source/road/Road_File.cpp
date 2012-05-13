#include "pch.h"
#include "../ogre/common/Defines.h"
#include "Road.h"
#include "tinyxml.h"

#include <OgreCamera.h>
#include <OgreMaterialManager.h>
#include <OgreEntity.h>
using namespace Ogre;


//  ctor
//---------------------------------------------------------------------------------------------------------------
#ifdef ROAD_EDITOR
SplineRoad::SplineRoad() :
#else
SplineRoad::SplineRoad(GAME* pgame) : pGame(pgame),
#endif
	mTerrain(0),mSceneMgr(0),mCamera(0),
	ndSel(0), ndChosen(0), ndRot(0), ndHit(0), ndChk(0),
	entSel(0), entChs(0), entRot(0), entHit(0), entChk(0),
	lastNdSel(0),lastNdChosen(0), iSelPoint(-1), iChosen(-1),
	posHit(Vector3::UNIT_SCALE), bHitTer(0), bSelChng(0),
	rebuild(false), iDirtyId(-1), idStr(0),
	fMarkerScale(1.f), fLodBias(1.f),
	bCastShadow(0), bRoadWFullCol(0),
	chksRoadLen(1.f)
{
	Defaults();  iTexSize = 1;
	iMrgSegs = 0;  segsMrg = 0;  iOldHide = -1;
	st.Length = 0.f;  st.WidthAvg = 0.f;  st.HeightDiff = 0.f;
	st.OnTer = 0.f;  st.Pipes = 0.f;
	st.Yaw = 0.f;  st.Pitch = 0.f;  st.Roll = 0.f;
}
void SplineRoad::Defaults()
{
	sTxtDesc = "";  fScRot = 1.8f;  fScHit = 0.8f;
	for (int i=0; i<MTRs; ++i)
	{	sMtrRoad[i] = "";  sMtrPipe[i] = "";  }
	sMtrWall = "road_wall";  sMtrCol = "road_col";  sMtrWallPipe = "pipe_wall";
	fHeight = 0.1f;  tcMul = 0.25f;  lenDiv0 = 1.f;  iw0 = 8;  iwPmul = 4;  ilPmul = 1;
	skLen = 1.f;  skH = 0.12f;
	setMrgLen = 180.f;  bMerge = false;  lposLen = 10.f;
	colN = 4; colR = 2.f;
	iDir = -1;  vStBoxDim = Vector3(1,3,5);  // /long |height -width
	iP1 = 0;  iChkId1 = 0;  iChkId1Rev = 0;
}

SplineRoad::~SplineRoad()
{	}

void SplineRoad::ToggleMerge()
{
	bMerge = !bMerge;
	RebuildRoad(true);
}


void SplineRoad::SetChecks()  // after xml load
{
	///  add checkpoints  * * *
	mChks.clear();  iChkId1 = 0;
	for (int i=0; i < mP.size(); ++i)  //=getNumPoints
	{
		if (mP[i].chkR > 0.f)
		{
			CheckSphere cs;
			cs.pos = mP[i].pos;  // +ofs_y ?-
			cs.r = mP[i].chkR * mP[i].width;
			cs.r2 = cs.r * cs.r;

			if (i == iP1)  //1st checkpoint
				iChkId1 = mChks.size();

			mChks.push_back(cs);
		}
	}
	int num = (int)mChks.size();
	if (num == 0)  return;

	//  1st checkpoint for reverse (= last chk)
	iChkId1Rev = (iChkId1 - iDir + num) % num;


	//  dist between checks
	if (num == 1)  {
		mChks[0].dist[0] = 10.f;  mChks[0].dist[1] = 10.f;  }

	//LogO("----  chks norm  ----");
	int i = iChkId1;  Real sum = 0.f;
	for (int n=0; n < num; ++n)
	{
		int i1 = (i + iDir + num) % num;
		Vector3 vd = mChks[i].pos - mChks[i1].pos;
		Real dist = (n == num-1) ? 0.f :  vd.length() + mChks[n].r;  // not last pair
		sum += dist;  mChks[n].dist[0] = sum;
		//LogO("Chk " + toStr(i) +"-"+ toStr(i1) + " dist:" + toStr(dist) + " sum:" + toStr(mChks[n].dist[0]));
		i = i1;
	}
	chksRoadLen = sum;

	//LogO("----  chks rev  ----");
	i = iChkId1Rev;  sum = 0.f;
	for (int n=0; n < num; ++n)
	{
		int i1 = (i - iDir + num) % num;
		Vector3 vd = mChks[i].pos - mChks[i1].pos;
		Real dist = (n == num-1) ? 0.f :  vd.length() + mChks[n].r;  // not last pair
		sum += dist;  mChks[n].dist[1] = sum;
		//LogO("Chk " + toStr(i) +"-"+ toStr(i1) + " dist:" + toStr(dist) + " sum:" + toStr(mChks[n].dist[1]));
		i = i1;
	}
	//LogO("----");
	//LogO("chksRoadLen: "+toStr(sum));
	//LogO("chk 1st: "+toStr(iChkId1) + " last: "+toStr(iChkId1Rev) + " dir: "+toStr(iDir));
	//LogO("----");
}

	

///  update road lods visibility
//--------------------------------------------------------------------------------------------------------
void SplineRoad::UpdLodVis(/*Camera* pCam,*/ float fBias, bool bFull)
{
	iVis = 0;  iTris = 0;
	const Real fDist[LODs+1] = {-800/*!temp -120*/, 40, 80, 140, 1000};
	
	const Plane& pl = mCamera->getFrustumPlane(FRUSTUM_PLANE_NEAR);
	for (size_t seg = 0; seg < vSegs.size(); ++seg)
	{
		#ifdef ROAD_EDITOR
		bool bSel = !bFull && ((vSel.size() == 0 && seg == iChosen || vSel.find(seg) != vSel.end()));
		#endif
		
		RoadSeg& rs = vSegs[seg];
		if (rs.empty)  continue;
		
		//  get dist
		Real d = FLT_MAX;
		for (size_t p=0; p < rs.lpos.size(); ++p)
			d = std::min(d, pl.getDistance( rs.lpos[p] ));
		
		//  set 1 mesh visible
		for (int i=0; i < LODs; ++i)
		{
			bool vis;
			if (bFull)  vis = i==0;  else  // all in 1st lod for preview
			vis = d >= fDist[i] * fBias && d < fDist[i+1] * fBias;  // normal
			/*if (bMerge)  vis = rs.mrgLod == i;  // vis mrg test-
			else  vis = i == 3;  /**/// check lod 0
			
			#ifdef ROAD_EDITOR
			if (vis)
				rs.road[i].ent->setMaterialName(
					rs.sMtrRd + (bSel ? "_sel" : "") );
			#endif
			
			rs.road[i].ent->setVisible(vis);
			if (rs.wall[i].ent)
				rs.wall[i].ent->setVisible(vis);
			//if (rs.col.ent && i==0)
			//	rs.col.ent->setVisible(vis);
			if (vis) {  ++iVis;  iTris += rs.nTri[i];  }
		}
	}
}

///  set lod0 for  render to tex   * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
void SplineRoad::SetForRnd(String sMtr)
{
	MaterialPtr mat = MaterialManager::getSingleton().getByName(sMtr);
	for (size_t seg = 0; seg < vSegs.size(); ++seg)
	{
		RoadSeg& rs = vSegs[seg];
		if (rs.empty)  continue;
		
		for (int i=0; i < LODs; ++i)
		{
			rs.road[i].ent->setVisible(i==0);
			rs.road[i].ent->setMaterial(mat);
			if (rs.wall[i].ent)
				rs.wall[i].ent->setVisible(false);
		}
		if (rs.col.ent)
			rs.col.ent->setVisible(false);
	}
}
void SplineRoad::UnsetForRnd()
{
	for (size_t seg = 0; seg < vSegs.size(); ++seg)
	{
		RoadSeg& rs = vSegs[seg];
		if (rs.empty)  continue;
		
		MaterialPtr mat = MaterialManager::getSingleton().getByName(  //^opt?..
			rs.sMtrRd);
		for (int i=0; i < LODs; ++i)
		{
			rs.road[i].ent->setMaterial(mat);
			// wall auto in updLodVis
		}
		if (rs.col.ent)
			rs.col.ent->setVisible(true);
	}
}


//---------------------------------------------------------------------------------------------------------------
///  Load
//---------------------------------------------------------------------------------------------------------------
bool SplineRoad::LoadFile(String fname, bool build)
{
	DestroyMarkers();
	if (build)
		clear();
	
	TiXmlDocument xml;  xml.LoadFile(fname.c_str());
	TiXmlElement* root = xml.RootElement(), *n = NULL;
	if (!root)  return false;
	const char* a = NULL;
	
	Defaults();
	n = root->FirstChildElement("mtr");	if (n)  {
		for (int i=0; i<MTRs; ++i)  {	String si = i==0 ? "" : toStr(i+1);
			a = n->Attribute(String("road"+si).c_str());	if (a)  sMtrRoad[i] = String(a);
			a = n->Attribute(String("pipe"+si).c_str());	if (a)  sMtrPipe[i] = String(a);	}
		a = n->Attribute("wall");	if (a)  sMtrWall = String(a);
		a = n->Attribute("pipeW");	if (a)  sMtrWallPipe = String(a);
		a = n->Attribute("col");	if (a)  sMtrCol  = String(a);
	}
	n = root->FirstChildElement("dim");	if (n)  {
		a = n->Attribute("tcMul");		if (a)  tcMul = s2r(a);
		a = n->Attribute("lenDim");		if (a)  lenDiv0 = s2r(a);
		a = n->Attribute("widthSteps");	if (a)  iw0   = s2i(a);
		a = n->Attribute("heightOfs");	if (a)  fHeight = s2r(a);
	}
	n = root->FirstChildElement("mrg");	if (n)  {
		a = n->Attribute("skirtLen");	if (a)  skLen = s2r(a);
		a = n->Attribute("skirtH");		if (a)  skH   = s2r(a);

		a = n->Attribute("merge");		if (a)  bMerge  = s2i(a) > 0;
		a = n->Attribute("mergeLen");	if (a)  setMrgLen = s2r(a);
		a = n->Attribute("lodPntLen");	if (a)  lposLen = s2r(a);
	}
	n = root->FirstChildElement("geom");	if (n)  {
		a = n->Attribute("colN");	if (a)  colN = s2i(a);
		a = n->Attribute("colR");	if (a)  colR = s2r(a);
		a = n->Attribute("wsPm");	if (a)  iwPmul = s2r(a);
		a = n->Attribute("lsPm");	if (a)  ilPmul = s2r(a);
		a = n->Attribute("stBox");	if (a)  vStBoxDim = s2v(a);
		a = n->Attribute("iDir");	if (a)  iDir = s2i(a);
		a = n->Attribute("iChk1");	if (a)  iP1 = s2i(a);
	}
	
	n = root->FirstChildElement("stats");	if (n)  {
		a = n->Attribute("length");		if (a)  st.Length = s2r(a);
		a = n->Attribute("width");		if (a)  st.WidthAvg = s2r(a);
		a = n->Attribute("height");		if (a)  st.HeightDiff = s2r(a);
		  a = n->Attribute("onTer");	if (a)  st.OnTer = s2r(a);
		  a = n->Attribute("pipes");	if (a)  st.Pipes = s2r(a);
		a = n->Attribute("yaw");		if (a)  st.Yaw = s2r(a);
		a = n->Attribute("pitch");		if (a)  st.Pitch = s2r(a);
		a = n->Attribute("roll");		if (a)  st.Roll = s2r(a);	}
	
	n = root->FirstChildElement("txt");	if (n)  {
		a = n->Attribute("desc");		if (a)  sTxtDesc = String(a);
	}

	n = root->FirstChildElement("P");	//  points
	while (n)
	{
		a = n->Attribute("pos");	newP.pos = s2v(a);
		a = n->Attribute("w");		newP.width = !a ? 6.f : s2r(a);

		a = n->Attribute("a");		if (a)  newP.mYaw = s2r(a);  else  newP.mYaw = 0.f;
		a = n->Attribute("ay");		if (a)  newP.mRoll = s2r(a);  else  {
		  a = n->Attribute("ar");	if (a)  newP.mRoll = s2r(a);  else  newP.mRoll = 0.f;  }
		  
		a = n->Attribute("aT");		if (a)  newP.aType = (AngType)s2i(a);

		a = n->Attribute("onTer");	newP.onTer = (a && a[0]=='0') ? false : true;
		a = n->Attribute("col");	newP.cols = !a ? 1 : s2i(a);

		a = n->Attribute("pipe");	newP.pipe = !a ? 0.f : std::max(0.f, std::min(1.f, s2r(a)));
		a = n->Attribute("mtr");	newP.idMtr = !a ? 0 : std::max(-1, std::min(MTRs-1, s2i(a)));
		
		a = n->Attribute("chkR");	newP.chkR = !a ? 0.f : s2r(a);
				
		//  Add point
		//newP.pos *= 0.7f;  //scale
		if (build)  Insert(INS_End);

		n = n->NextSiblingElement("P");
	}
	/// create loop-
	/*const int q = 8;
	for (int i=0; i < q*3; ++i)
	{
		Real a = Real(i%q)/q *PI_d*2.f, ri = Real(i)/q;
		Vector3 pos(cosf(a), 0.f, -sinf(a));
		pos *= 12.f + ri*5.f;  // r
		pos.z -= 30.f;
		pos.y += 1 + i * 1.5f;  // h
		Insert(INS_End, pos, 7.f, ri*360.f, 10.f +ri*5.f, 0);
	}*/
	
	#ifdef ROAD_EDITOR
	bMerge = false;  // editor off merge
	#endif
	newP.SetDefault();
	iChosen = -1;  //std::max(0, std::min((int)(mP.size()-1), iChosen));
	
	SetChecks();
	if (build)  RebuildRoad(true);
	return true;
}

///  Save
//---------------------------------------------------------------------------------------------------------------
bool SplineRoad::SaveFile(String fname)
{
	TiXmlDocument xml;	TiXmlElement root("SplineRoad");

	TiXmlElement mtr("mtr");
		for (int i=0; i<MTRs; ++i)  {	String si = i==0 ? "" : toStr(i+1);
			if (sMtrRoad[i] != "")	mtr.SetAttribute(String("road"+si).c_str(),	sMtrRoad[i].c_str());
			if (sMtrPipe[i] != "")	mtr.SetAttribute(String("pipe"+si).c_str(),	sMtrPipe[i].c_str());  }

		if (sMtrWall != "road_wall")	mtr.SetAttribute("wall",	sMtrWall.c_str());
	if (sMtrWallPipe != "pipe_wall")	mtr.SetAttribute("pipeW",	sMtrWallPipe.c_str());
		if (sMtrCol  != "road_col")		mtr.SetAttribute("col",		sMtrCol.c_str());
	root.InsertEndChild(mtr);
	
	TiXmlElement dim("dim");
		dim.SetAttribute("tcMul",		toStr( tcMul ).c_str());
		dim.SetAttribute("lenDim",		toStr( lenDiv0 ).c_str());
		dim.SetAttribute("widthSteps", toStr( iw0  ).c_str());
		dim.SetAttribute("heightOfs",	toStr( fHeight ).c_str());
	root.InsertEndChild(dim);

	TiXmlElement mrg("mrg");
		mrg.SetAttribute("skirtLen",	toStr( skLen ).c_str());
		mrg.SetAttribute("skirtH",		toStr( skH ).c_str());

		mrg.SetAttribute("merge",		"1");  // always 1 for game, 0 set in editor
		mrg.SetAttribute("mergeLen",	toStr( setMrgLen ).c_str());
		mrg.SetAttribute("lodPntLen",	toStr( lposLen ).c_str());
	root.InsertEndChild(mrg);

	TiXmlElement geo("geom");
		geo.SetAttribute("colN",	toStr( colN ).c_str());
		geo.SetAttribute("colR",	toStr( colR ).c_str());
		geo.SetAttribute("wsPm",	toStr( iwPmul ).c_str());
		geo.SetAttribute("lsPm",	toStr( ilPmul ).c_str());
		geo.SetAttribute("stBox",	toStr( vStBoxDim ).c_str());
		geo.SetAttribute("iDir",	toStr( iDir ).c_str());
		geo.SetAttribute("iChk1",	toStr( iP1 ).c_str());
	root.InsertEndChild(geo);

	TiXmlElement ste("stats");
		ste.SetAttribute("length",	toStr( st.Length ).c_str());
		ste.SetAttribute("width",	toStr( st.WidthAvg ).c_str());
		ste.SetAttribute("height",	toStr( st.HeightDiff ).c_str());
		  ste.SetAttribute("onTer",	toStr( st.OnTer ).c_str());
		  ste.SetAttribute("pipes",	toStr( st.Pipes ).c_str());
		ste.SetAttribute("yaw",		toStr( st.Yaw ).c_str());
		ste.SetAttribute("pitch",	toStr( st.Pitch ).c_str());
		ste.SetAttribute("roll",	toStr( st.Roll ).c_str());
	root.InsertEndChild(ste);
		
	TiXmlElement txt("txt");
		txt.SetAttribute("desc",	sTxtDesc.c_str());
	root.InsertEndChild(txt);

	int num = getNumPoints();		//  points
	for (int i=0; i < num; ++i)
	{
		bool onTer = mP[i].onTer, onTer1 = mP[(i+1)%num].onTer, onTer_1 = mP[(i-1+num)%num].onTer;
		TiXmlElement p("P");
		{
			Vector3 pos = getPos(i);  if (onTer)  pos.y = 0.f;  // no need to save
			p.SetAttribute("pos",	toStr( pos ).c_str());
			p.SetAttribute("w",		toStr( mP[i].width ).c_str());
		
			if (!onTer)
				p.SetAttribute("onTer",	"0");

			if (!onTer || !onTer1 || !onTer_1)
			{	p.SetAttribute("a",  toStr( mP[i].mYaw ).c_str());
				p.SetAttribute("ar", toStr( mP[i].mRoll ).c_str());
				p.SetAttribute("aT", toStr( (int)mP[i].aType ).c_str());
			}

			if (mP[i].cols != 1)
				p.SetAttribute("col", toStr( mP[i].cols ).c_str());
			if (mP[i].pipe > 0.f)
				p.SetAttribute("pipe", toStr( mP[i].pipe ).c_str());

			if (mP[i].idMtr != 0)
				p.SetAttribute("mtr", toStr( mP[i].idMtr ).c_str());

			if (mP[i].chkR > 0.f)
				p.SetAttribute("chkR", toStr( mP[i].chkR ).c_str());
		}
		root.InsertEndChild(p);
	}
	
	xml.InsertEndChild(root);
	return xml.SaveFile(fname.c_str());
}
