#include "pch.h"

#include "GraphView.h"
#include "RenderConst.h"
using namespace Ogre;


//  ctor
GraphView::GraphView(SceneManager* pSceneMgr)
	:mSceneMgr(pSceneMgr),mo(0),nd(0), iCurX(0)
{}

//  Create
void GraphView::Create(int size)
{
	vals.resize(size);  iCurX = 0;  //size-1
	
	mo = mSceneMgr->createManualObject();
	mo->setDynamic(true);
	mo->setUseIdentityProjection(true);
	mo->setUseIdentityView(true);
	mo->setCastShadows(false);

	mo->estimateVertexCount(size);
	mo->begin("graph1", RenderOperation::OT_LINE_STRIP);
	float s = 0.5f, asp = 1.f;
	mo->position(-s,-s*asp, 0);  //mo->colour(0,1,0);
	mo->position( s,-s*asp, 0);  //mo->colour(0,0,0);
	mo->position(-s, s*asp, 0);  //mo->colour(1,1,0);
	mo->position( s, s*asp, 0);  //mo->colour(1,0,0);
	mo->end();
 
	AxisAlignedBox aabInf;	aabInf.setInfinite();
	mo->setBoundingBox(aabInf);  // always visible
	mo->setRenderQueueGroup(RQG_Hud3);  // on hud
	mo->setVisibilityFlags(RV_Hud);

	nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	nd->attachObject(mo);
}

void GraphView::Destroy()
{
	if (mo) {	mSceneMgr->destroyManualObject(mo);  mo = 0;  }
	if (nd) {	mSceneMgr->destroySceneNode(nd);  nd = 0;  }
}


//  Update
void GraphView::AddVal(float val)
{
	if (!mo)  return;

	//assert vals empty
	vals[iCurX] = val;
	++iCurX;  if (iCurX >= vals.size())  iCurX = 0;
}

void GraphView::Update()
{
	if (!mo)  return;
	
	size_t size = vals.size();
	int i = iCurX % size;  // vals id
	float fx = 0.f, fAdd = 1.f / size;  // screen x

	mo->beginUpdate(0);
	mo->position(fx, vals[i], 0.f);
	for (size_t n=0; n < vals.size(); ++n)
	{
		mo->position(fx, vals[i], 0.f);
		//mo->colour(ColourValue(1.f,1.f,0.f));

		++i;  if (i >= vals.size())  i = 0;
		fx += fAdd;
	}
	mo->end();
}

