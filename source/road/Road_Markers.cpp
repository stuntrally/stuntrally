//#include "Defines.h"
#include "Road.h"

#include <OgreSceneManager.h>
#include <OgreTerrain.h>
#include <OgreSceneNode.h>
#include <OgreEntity.h>
using namespace Ogre;


//  Setup
//---------------------------------------------------------------------------------------------------------------
void SplineRoad::Setup(String sMarkerMeshFile, Real scale,
	Terrain* terrain, SceneManager* sceneMgr, Camera* camera)
{
	sMarkerMesh = sMarkerMeshFile;
	fMarkerScale = scale;
	mTerrain = terrain;  mSceneMgr = sceneMgr;  mCamera = camera;
	
	if (sMarkerMesh == "")  return;

	#define createSphere(mat, ent, nd)  \
		ent = mSceneMgr->createEntity(name, sMarkerMeshFile);  \
		ent->setMaterialName(mat);  ent->setCastShadows(false);  ent->setVisibilityFlags(2);  \
		nd = mSceneMgr->getRootSceneNode()->createChildSceneNode(name);  \
		nd->attachObject(ent);  nd->setVisible(false);
	
	String name;
	name = "sphereSel";		createSphere("sphere_sel", entSel, ndSel);
	name = "sphereChosen";  createSphere("sphere_chosen", entChs, ndChosen);
	name = "sphereHit";		createSphere("sphere_hit", entHit, ndHit);
		ndHit->setScale(/*fMarkerScale **/ fScHit * Vector3::UNIT_SCALE);
	name = "sphereRot";		createSphere("sphere_rot", entRot, ndRot);
		ndRot->setScale(fMarkerScale * fScRot * Vector3::UNIT_SCALE);
	name = "sphereCheck";	createSphere("sphere_check", entChk, ndChk);
		entChk->setRenderQueueGroup(80);  // after road
}


//  add marker
//-------------------------------------------------------------------------------------
void SplineRoad::AddMarker(Vector3 pos)
{
	if (sMarkerMesh == "")  return;
	Entity* ent;  SceneNode* nod;
	String name = "sphere"+StringConverter::toString(getNumPoints());
	ent = mSceneMgr->createEntity(sMarkerMesh/*,name*/);
	ent->setMaterialName("sphere_norm");  ent->setCastShadows(false);  ent->setVisibilityFlags(2);
	nod = mSceneMgr->getRootSceneNode()->createChildSceneNode(/*name,*/pos);
	nod->attachObject(ent);  nod->scale(fMarkerScale * Vector3::UNIT_SCALE);
	vMarkNodes.push_back(nod);
}

//  del last marker
void SplineRoad::DelLastMarker()
{
	if (sMarkerMesh == "")  return;
	SceneNode* last = vMarkNodes[vMarkNodes.size()-1];
	if (lastNdChosen == last)
		lastNdChosen = 0;
	if (lastNdSel == last)
		lastNdSel = 0;
	String name = "sphere"+StringConverter::toString(getNumPoints()-1);
	mSceneMgr->destroyEntity(name);
	mSceneMgr->destroySceneNode(vMarkNodes[getNumPoints()-1]);
	vMarkNodes.pop_back();
}

//  destroy all
void SplineRoad::DestroyMarkers()
{
	if (sMarkerMesh == "")  return;
	for (size_t i=0; i < vMarkNodes.size(); ++i)
	{
		String name = "sphere"+StringConverter::toString(i);
		mSceneMgr->destroyEntity(name);
		mSceneMgr->destroySceneNode(vMarkNodes[i]);
	}
	vMarkNodes.clear();
	lastNdChosen = 0;
	lastNdSel = 0;
}

//  select
//-------------------------------------------------------------------------------------
void SplineRoad::SelectMarker(bool bHide)
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

void SplineRoad::UpdAllMarkers()
{
	if (sMarkerMesh == "")  return;
	for (int i=0; i < getNumPoints(); ++i)
		Move1(i, Vector3::ZERO);  //-

	for (int i=0; i < (int)vMarkNodes.size(); i++)
	if (i < getNumPoints())
	{
		Vector3* pos = &mP[i].pos;  //- update on ter pos (move 0)
		if (mP[i].onTer)
			pos->y = mTerrain->getHeightAtWorldPosition(pos->x, 0, pos->z) + fHeight;

		SceneNode* nd = vMarkNodes[i];
		nd->setPosition(*pos/*getPos(i)*/);
		nd->setScale(fMarkerScale * Vector3::UNIT_SCALE);
	}
	UpdRot();
}

//  util
//------------------------------------------------
void SplineRoad::UpdRot()
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
