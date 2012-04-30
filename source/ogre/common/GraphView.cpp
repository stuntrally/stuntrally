#include "pch.h"

#include "GraphView.h"
#include "RenderConst.h"
using namespace Ogre;


//  ctor
GraphView::GraphView(SceneManager* pSceneMgr)
	:mSceneMgr(pSceneMgr), mo(0),mb(0), nd(0), iCurX(0)
{}

//  Create
void GraphView::Create(int size, String sMtr, bool background)
{
	vals.resize(size);  iCurX = 0;  //size-1
	
	//  graph  ----------------------
	mo = mSceneMgr->createManualObject();
	mo->setDynamic(true);
	mo->setUseIdentityProjection(true);
	mo->setUseIdentityView(true);
	mo->setCastShadows(false);

	mo->estimateVertexCount(size);
	mo->begin(sMtr, RenderOperation::OT_LINE_STRIP);
	float s = 0.5f, asp = 1.f;
	mo->position(0,0, 0);  //mo->colour(0,1,0);
	mo->position(1,0, 0);  //mo->colour(0,0,0);
	mo->end();

	AxisAlignedBox aabInf;	aabInf.setInfinite();
	mo->setBoundingBox(aabInf);  // always visible
	mo->setRenderQueueGroup(RQG_Hud3);  // on hud
	mo->setVisibilityFlags(RV_Hud);

	nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	nd->attachObject(mo);

	//  back rect  ----------------------
	if (background)
	{	const float al = 0.2f;
		mb = mSceneMgr->createManualObject();
		mb->setDynamic(true);
		mb->setUseIdentityProjection(true);
		mb->setUseIdentityView(true);
		mb->setCastShadows(false);

		mb->estimateVertexCount(4);
		mb->begin("graphBack", RenderOperation::OT_TRIANGLE_STRIP);
		mb->position(0,0, 0);  mb->colour(1,1,1,al);
		mb->position(1,0, 0);  mb->colour(1,1,1,al);
		mb->position(0,1, 0);  mb->colour(1,1,1,al);
		mb->position(1,1, 0);  mb->colour(1,1,1,al);
		mb->end();
	 
		mb->setBoundingBox(aabInf);
		mb->setRenderQueueGroup(RQG_Hud2);
		mb->setVisibilityFlags(RV_Hud);
		nd->attachObject(mb);
	}
	//nd->setPosition(-1.f, -1.f+0.5f, 0.f);
	//nd->setScale(0.8f, 0.5f, 1.f);
}

void GraphView::Destroy()
{
	if (mo) {	mSceneMgr->destroyManualObject(mo);  mo = 0;  }
	if (mb) {	mSceneMgr->destroyManualObject(mb);  mb = 0;  }
	if (nd) {	mSceneMgr->destroySceneNode(nd);  nd = 0;  }
}

//  Set Size
void GraphView::SetSize(float posX,float posY,float sizeX,float sizeY)  // [0..1]  0,0 is left bottom
{
	nd->setPosition(-1.f+posX*2.f, -1.f+posY*2.f, 0.f);
	nd->setScale(sizeX*2.f, sizeY*2.f, 1.f);
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

