#include "pch.h"
#include "Road.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"

#include <OgreSceneManager.h>
#include <OgreTerrain.h>
#include <OgreSceneNode.h>
#include <OgreEntity.h>
using namespace Ogre;


//  Setup
//---------------------------------------------------------------------------------------------------------------
void SplineMarkEd::createMarker(String name, String mat, Entity*& ent, SceneNode*& nd)
{
	ent = mSceneMgr->createEntity(name, sMarkerMesh);
	ent->setMaterialName(mat);  ent->setCastShadows(false);  ent->setVisibilityFlags(RV_Hud);
	nd = mSceneMgr->getRootSceneNode()->createChildSceneNode(name);
	nd->attachObject(ent);  nd->setVisible(false);
}

void SplineMarkEd::Setup(
	String sMarkerMeshFile, Real scale,
	Terrain* terrain, SceneManager* sceneMgr, Camera* camera, int idx)
{
	idRd = idx;
	sMarkerMesh = sMarkerMeshFile;
	fMarkerScale = scale;
	mTerrain = terrain;  mSceneMgr = sceneMgr;  mCamera = camera;
	
	if (sMarkerMesh == "")  return;

	String name, si = toStr(idRd);
	createMarker(si+"sphereSel",   "sphere_sel",   entSel, ndSel);
	createMarker(si+"sphereChosen","sphere_chosen",entChs, ndChosen);
	createMarker(si+"sphereHit",   "sphere_hit",   entHit, ndHit);
	createMarker(si+"sphereRot",   "sphere_rot",   entRot, ndRot);
	createMarker(si+"sphereCheck", "sphere_check", entChk, ndChk);

	ndHit->setScale(/*fMarkerScale **/ fScHit * Vector3::UNIT_SCALE);
	ndRot->setScale(fMarkerScale * fScRot * Vector3::UNIT_SCALE);
	entChk->setRenderQueueGroup(RQG_RoadMarkers);  // after road
}


void SplineEdit::Mark::setPos(Vector3 pos)
{
	nd->setPosition(pos);
	//ndC->setPosition(pos);
}
void SplineEdit::Mark::setVis(bool vis)
{
	nd->setVisible(vis);
	//ndC->setVisible(vis);
}


//  add marker
//-------------------------------------------------------------------------------------
void SplineMarkEd::AddMarker(Vector3 pos)
{
	if (sMarkerMesh == "")  return;
	Entity* ent;//, *entC;
	SceneNode* nd;//, *ndC;

	ent = mSceneMgr->createEntity(sMarkerMesh);
	ent->setMaterialName("sphere_norm");  ent->setCastShadows(false);
	ent->setVisibilityFlags(RV_Hud);
	
	nd = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos);
	nd->attachObject(ent);  nd->scale(fMarkerScale * Vector3::UNIT_SCALE);

	//entC = mSceneMgr->createEntity(sMarkerMesh);
	//entC->setMaterialName("sphere_norm");  entC->setCastShadows(false);
	//entC->setVisibilityFlags(RV_Hud/*RQG_RoadMarkers*/);
	
	//ndC = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos);
	//ndC->attachObject(entC);  //ndC->setVisible(true);

	Mark m;
	m.nd = nd;  m.ent = ent;
	//m.ndC = ndC;  m.entC = entC;
	vMarks.push_back(m);
}

void SplineMarkEd::DestroyMarker(int id)
{
	Mark& m = vMarks[id];
	mSceneMgr->destroyEntity(m.ent);
	//mSceneMgr->destroyEntity(m.entC);
	mSceneMgr->destroySceneNode(m.nd);
	//mSceneMgr->destroySceneNode(m.ndC);
}

//  del last marker
void SplineMarkEd::DelLastMarker()
{
	if (sMarkerMesh == "")  return;
	int last = vMarks.size()-1;
	if (lastNdChosen == last)
		lastNdChosen = -2;
	if (lastNdSel == last)
		lastNdSel = -2;

	DestroyMarker(getNumPoints()-1);
	vMarks.pop_back();
}

//  destroy all
void SplineMarkEd::DestroyMarkers()
{
	if (sMarkerMesh == "")  return;
	for (size_t i=0; i < vMarks.size(); ++i)
		DestroyMarker(i);

	vMarks.clear();
	lastNdChosen = -2;
	lastNdSel = -2;
}


//  Select
//-------------------------------------------------------------------------------------
void SplineRoad::SelectMarker(bool bHide)  // Mr Melect Sarker
{
	if (vMarks.empty())
	{
		ndChosen->setVisible(false);
		ndRot->setVisible(false);
		ndChk->setVisible(false);
		ndSel->setVisible(false);
		return;
	}

	if (lastNdSel >= 0)
		vMarks[lastNdSel].setVis(true);
	if (lastNdChosen >= 0)
		vMarks[lastNdChosen].setVis(true);

	int i = iChosen;
	if (i == -1 || bHide)
	{	ndChosen->setVisible(false);
		ndRot->setVisible(false);
		ndChk->setVisible(false);
	}else  // chosen
	{
		Mark& m = vMarks[i];
		m.setVis(false);
		ndChosen->setPosition(m.nd->getPosition());
		ndChosen->setScale(m.nd->getScale());
		ndChosen->setVisible(true);
		ndRot->setVisible(true);
		lastNdChosen = i;

		Vector3 pc = m.nd->getPosition();
		//  move checks in pipe to center (half normal up)
		if (mP[i].pipe > 0.f && mP[i].onPipe==0)
			pc += 0.5f * mP[i].width * DL0.v0_N[i];
		ndChk->setPosition(pc);
		
		ndChk->setScale(mP[i].chkR * 2.f * mP[i].width * Vector3::UNIT_SCALE);
		ndChk->setVisible(true);
	}	

	if (iSelPoint == -1 || bHide)
		ndSel->setVisible(false);
	else  // sel
	{
		Mark& m = vMarks[iSelPoint];
		m.setVis(false);
		ndSel->setPosition(m.nd->getPosition());
		ndSel->setScale(m.nd->getScale());
		ndSel->setVisible(true);
		lastNdSel = iSelPoint;
	}
	UpdRot();
}

//  Update all
void SplineMarkEd::UpdAllMarkers()
{
	if (sMarkerMesh == "")  return;
	for (int i=0; i < getNumPoints(); ++i)
		Move1(i, Vector3::ZERO);  //-

	int si = std::min(getNumPoints(), (int)vMarks.size());  //=
	for (int i=0; i < si; ++i)
	{
		Vector3& pos = mP[i].pos;  //- update on ter pos (move 0)
		if (mP[i].onTer)
			pos.y = getTerH(pos) + g_Height;

		Mark& m = vMarks[i];
		m.setPos(pos/*getPos(i)*/);
		m.nd->setScale(fMarkerScale * Vector3::UNIT_SCALE);
		//m.ndC->setScale(mP[i].chkR * 2.f * mP[i].width * Vector3::UNIT_SCALE);
	}
	UpdRot();
}


//  util
//------------------------------------------------
void SplineMarkEd::UpdRot()
{
	if (!ndRot)  return;
	int i = iChosen;	//ndRot->setVisible(mP[i].onTer);
	if (i == -1)  return;

	Vector3 vr = GetRot(mP[i].aYaw, mP[i].aRoll);
	Vector3 rpos = mP[i].pos + vr * mP[i].width * 0.54f;
	ndRot->setPosition(rpos);  //vr.normalise();
	ndRot->setScale(fMarkerScale * fScRot * Vector3::UNIT_SCALE);

	//Quaternion q;  q.FromAngleAxis(Degree(0), vr);
	//ndRot->setOrientation(q);  //.. box
}


void SplineMarkEd::SetTerHitVis(bool visible)
{
	if (ndHit)
		ndHit->setVisible(visible);
}



//  Checkpoints
//--------------------------------------------------------------------------------------
void SplineEditChk::AddChkR(Real relR, bool dontCheckR)    ///  ChkR
{
	int seg = iChosen;
	if (seg == -1)  return;
	mP[seg].chkR = std::max(0.f, mP[seg].chkR + relR);
	if (dontCheckR)  return;

	//  disallow between 0..1
	if (relR < 0.f && mP[seg].chkR < 1.f)  mP[seg].chkR = 0.f;  else
	if (relR > 0.f && mP[seg].chkR < 1.f)  mP[seg].chkR = 1.f;
	
	//  max radius  (or const on bridges or pipes)
	int all = getNumPoints();
	if (all < 2)  return;
	
	int next = (seg+1) % all, prev = (seg-1+all) % all;
	bool bridge = !mP[seg].onTer || !mP[next].onTer || !mP[prev].onTer;
	bool pipe = mP[seg].pipe > 0.5f;

	Real maxR = pipe || bridge ? 1.f : 2.5f;
	if (bridge || pipe)
	{	if (relR > 0.f)  mP[seg].chkR = maxR;  else
		if (relR < 0.f)  mP[seg].chkR = 0.f;
	}
	else if (relR > 0.f && mP[seg].chkR > maxR)
		mP[seg].chkR = maxR;
}


void SplineEditChk::AddBoxW(Real rel)
{
	vStBoxDim.z = std::max(6.f, vStBoxDim.z + rel);
}
void SplineEditChk::AddBoxH(Real rel)
{
	vStBoxDim.y = std::max(5.f, vStBoxDim.y + rel);
}

void SplineEditChk::Set1stChk()
{
	if (iChosen < 0 || iChosen >= getNumPoints())  return;
	if (mP[iChosen].chkR < 0.5f)  return;

	for (int i=0; i < getNumPoints(); ++i)  // clear from all
		mP[i].chk1st = false;
	mP[iChosen].chk1st = true;  // set this
}


//  Set Checkpoints
//--------------------------------------------------------------------------------------
void SplineRoad::SetChecks()
{
	///  add checkpoints  * * *
	//LogO("mChks.clear");
	mChks.clear();  iChkId1 = 0;
	for (int i=0; i < mP.size(); ++i)  //=getNumPoints
	{
		if (mP[i].chkR > 0.f)
		{
			CheckSphere cs;
			cs.pos = mP[i].pos;
			cs.r = mP[i].chkR * mP[i].width;
			
			//  move checks in pipe to center (half normal up)
			if (!DL0.v0_N.empty())
			if (mP[i].pipe > 0.f && mP[i].onPipe==0)
			{	cs.pos += 0.5f * mP[i].width * DL0.v0_N[i];
				cs.r *= 0.5f;  }  // exact-
			
			cs.r2 = cs.r * cs.r;
			cs.loop = mP[i].loop > 0;

			if (mP[i].chk1st)  //1st checkpoint
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
