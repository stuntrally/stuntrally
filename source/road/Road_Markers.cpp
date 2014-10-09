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
SplineMarkEd::SplineMarkEd()
	:mSceneMgr(0),mCamera(0)
	,ndSel(0), ndChosen(0), ndRot(0), ndHit(0), ndChk(0)
	,entSel(0), entChs(0), entRot(0), entHit(0), entChk(0)
	,lastNdSel(0),lastNdChosen(0)
	,fMarkerScale(1.f), fScRot(1.8f),fScHit(0.8f)
{	}

void SplineMarkEd::createMarker(Ogre::String name, Ogre::String mat, Ogre::Entity*& ent, Ogre::SceneNode*& nd)
{
	ent = mSceneMgr->createEntity(name, sMarkerMesh);
	ent->setMaterialName(mat);  ent->setCastShadows(false);  ent->setVisibilityFlags(RV_Hud);
	nd = mSceneMgr->getRootSceneNode()->createChildSceneNode(name);
	nd->attachObject(ent);  nd->setVisible(false);
}

void SplineMarkEd::Setup(
	String sMarkerMeshFile, Real scale,
	Terrain* terrain, SceneManager* sceneMgr, Camera* camera)
{
	sMarkerMesh = sMarkerMeshFile;
	fMarkerScale = scale;
	mTerrain = terrain;  mSceneMgr = sceneMgr;  mCamera = camera;
	
	if (sMarkerMesh == "")  return;

	String name;
	createMarker("sphereSel",   "sphere_sel",   entSel, ndSel);
	createMarker("sphereChosen","sphere_chosen",entChs, ndChosen);
	createMarker("sphereHit",   "sphere_hit",   entHit, ndHit);
	createMarker("sphereRot",   "sphere_rot",   entRot, ndRot);
	createMarker("sphereCheck", "sphere_check", entChk, ndChk);

	ndHit->setScale(/*fMarkerScale **/ fScHit * Vector3::UNIT_SCALE);
	ndRot->setScale(fMarkerScale * fScRot * Vector3::UNIT_SCALE);
	entChk->setRenderQueueGroup(RQG_RoadMarkers);  // after road
}


//  add marker
//-------------------------------------------------------------------------------------
void SplineMarkEd::AddMarker(Vector3 pos)
{
	if (sMarkerMesh == "")  return;
	Entity* ent;  SceneNode* nod;
	String name = "sphere"+toStr(getNumPoints());
	ent = mSceneMgr->createEntity(sMarkerMesh/*,name*/);
	ent->setMaterialName("sphere_norm");  ent->setCastShadows(false);  ent->setVisibilityFlags(RV_Hud);
	nod = mSceneMgr->getRootSceneNode()->createChildSceneNode(/*name,*/pos);
	nod->attachObject(ent);  nod->scale(fMarkerScale * Vector3::UNIT_SCALE);
	vMarkNodes.push_back(nod);
}

//  del last marker
void SplineMarkEd::DelLastMarker()
{
	if (sMarkerMesh == "")  return;
	SceneNode* last = vMarkNodes[vMarkNodes.size()-1];
	if (lastNdChosen == last)
		lastNdChosen = 0;
	if (lastNdSel == last)
		lastNdSel = 0;
	String name = "sphere"+toStr(getNumPoints()-1);
	mSceneMgr->destroyEntity(name);
	mSceneMgr->destroySceneNode(vMarkNodes[getNumPoints()-1]);
	vMarkNodes.pop_back();
}

//  destroy all
void SplineMarkEd::DestroyMarkers()
{
	if (sMarkerMesh == "")  return;
	for (size_t i=0; i < vMarkNodes.size(); ++i)
	{
		String name = "sphere"+toStr(i);
		mSceneMgr->destroyEntity(name);
		mSceneMgr->destroySceneNode(vMarkNodes[i]);
	}
	vMarkNodes.clear();
	lastNdChosen = 0;
	lastNdSel = 0;
}


//  Select
//-------------------------------------------------------------------------------------
void SplineMarkEd::SelectMarker(bool bHide)  // Mr Melect Sarker
{
	if (lastNdSel)
		lastNdSel->setVisible(true);
	if (lastNdChosen)
		lastNdChosen->setVisible(true);

	if (iChosen == -1 || bHide)
	{	ndChosen->setVisible(false);
		ndRot->setVisible(false);
		ndChk->setVisible(false);
	}else  // chosen
	{
		SceneNode* nd = vMarkNodes[iChosen];
		nd->setVisible(false);
		ndChosen->setPosition(nd->getPosition());
		ndChosen->setScale(nd->getScale()*1.0f);
		ndChosen->setVisible(true);
		ndRot->setVisible(true);
		lastNdChosen = nd;

		ndChk->setPosition(nd->getPosition());
		ndChk->setScale(mP[iChosen].chkR * 2 * mP[iChosen].width * Vector3::UNIT_SCALE);
		ndChk->setVisible(true);
	}	

	if (iSelPoint == -1 || bHide)
		ndSel->setVisible(false);
	else  // sel
	{
		SceneNode* nd = vMarkNodes[iSelPoint];
		nd->setVisible(false);
		ndSel->setPosition(nd->getPosition());
		ndSel->setScale(nd->getScale()*1.0f);
		ndSel->setVisible(true);
		lastNdSel = nd;
	}
	UpdRot();
}

//  Update all
void SplineMarkEd::UpdAllMarkers()
{
	if (sMarkerMesh == "")  return;
	for (int i=0; i < getNumPoints(); ++i)
		Move1(i, Vector3::ZERO);  //-

	for (int i=0; i < (int)vMarkNodes.size(); ++i)
	if (i < getNumPoints())
	{
		Vector3& pos = mP[i].pos;  //- update on ter pos (move 0)
		if (mP[i].onTer)
			pos.y = getTerH(pos) + fHeight;

		SceneNode* nd = vMarkNodes[i];
		nd->setPosition(pos/*getPos(i)*/);
		nd->setScale(fMarkerScale * Vector3::UNIT_SCALE);
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
	if (relR < 0.f && mP[seg].chkR < 1.f)
		mP[seg].chkR = 0.f;
	else
	if (relR > 0.f && mP[seg].chkR < 1.f)
		mP[seg].chkR = 1.f;
	
	//  max radius  (or const on bridges or pipes)
	int all = getNumPoints();
	if (all < 2)  return;
	
	int next = (seg+1) % all, prev = (seg-1+all) % all;
	bool bridge = !mP[seg].onTer || !mP[next].onTer || !mP[prev].onTer;
	bool pipe = mP[seg].pipe > 0.5f;
		// || mP[next].pipe > 0.5f || mP[prev].pipe > 0.5f;

	Real maxR = pipe ? 1.7f : bridge ? 1.f : 2.5f;
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


//  Set Checkpoints  after xml load, todo: move after rebuild ..
//--------------------------------------------------------------------------------------
void SplineEditChk::SetChecks()
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
			cs.loop = mP[i].loopChk > 0;

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
