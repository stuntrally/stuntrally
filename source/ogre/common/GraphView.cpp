#include "pch.h"

#include "GraphView.h"
#include "RenderConst.h"
using namespace Ogre;


//  ctor
GraphView::GraphView(SceneManager* pSceneMgr)
	:mSceneMgr(pSceneMgr), moLine(0),moBack(0), node(0), iCurX(0)
{}

//  Create
//----------------------------------------------------------------------------------------
void GraphView::Create(int length, String sMtr, float backAlpha)
{
	vals.resize(length);  iCurX = 0;  //size-1
	
	//  graph line  ----------------------
	moLine = mSceneMgr->createManualObject();
	moLine->setDynamic(true);
	moLine->setUseIdentityProjection(true);
	moLine->setUseIdentityView(true);
	moLine->setCastShadows(false);

	moLine->estimateVertexCount(length);
	moLine->begin(sMtr, RenderOperation::OT_LINE_STRIP);
	float s = 0.5f, asp = 1.f;
	moLine->position(0,0, 0);  //mo->colour(0,1,0);
	moLine->position(1,0, 0);  //mo->colour(0,0,0);
	moLine->end();

	AxisAlignedBox aabInf;	aabInf.setInfinite();
	moLine->setBoundingBox(aabInf);  // always visible
	moLine->setRenderQueueGroup(RQG_Hud3);  // on hud
	moLine->setVisibilityFlags(RV_Hud);

	node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	node->attachObject(moLine);

	//  backgr rect  ----------------------
	if (backAlpha > 0.01f)
	{
		moBack = mSceneMgr->createManualObject();
		moBack->setDynamic(true);
		moBack->setUseIdentityProjection(true);
		moBack->setUseIdentityView(true);
		moBack->setCastShadows(false);

		moBack->estimateVertexCount(4);
		moBack->begin("graphBack", RenderOperation::OT_TRIANGLE_STRIP);
		moBack->position(0,0, 0);  moBack->colour(1,1,1,backAlpha);
		moBack->position(1,0, 0);  moBack->colour(1,1,1,backAlpha);
		moBack->position(0,1, 0);  moBack->colour(1,1,1,backAlpha);
		moBack->position(1,1, 0);  moBack->colour(1,1,1,backAlpha);
		moBack->end();
	 
		moBack->setBoundingBox(aabInf);
		moBack->setRenderQueueGroup(RQG_Hud2);
		moBack->setVisibilityFlags(RV_Hud);
		node->attachObject(moBack);
	}
	/// todo: ..
	// helper grid lines =
	// digits text for grid values (mygui)
	// auto val range ?
	// save in pSet
	// gui tab= properties,size, etc
}

void GraphView::Destroy()
{
	if (moLine) {	mSceneMgr->destroyManualObject(moLine);  moLine = 0;  }
	if (moBack) {	mSceneMgr->destroyManualObject(moBack);  moBack = 0;  }
	if (node) {	mSceneMgr->destroySceneNode(node);  node = 0;  }
}

//  Set Size
void GraphView::SetSize(float posX,float posY,float sizeX,float sizeY)  // [0..1]  0,0 is left bottom
{
	if (!node)  return;
	node->setPosition(-1.f+posX*2.f, -1.f+posY*2.f, 0.f);
	node->setScale(sizeX*2.f, sizeY*2.f, 1.f);
}

//  show/hide
void GraphView::SetVisible(bool visible)
{
	if (!node)  return;
	node->setVisible(visible);
}


//  Update
//------------------------------------------------------------------
void GraphView::AddVal(float val)
{
	//if (!node)  return;
	if (iCurX >= vals.size())  return;

	//assert vals empty
	vals[iCurX] = val;
	++iCurX;  if (iCurX >= vals.size())  iCurX = 0;
}

void GraphView::Update()
{
	if (!node)  return;
	
	size_t size = vals.size();
	int i = iCurX % size;  // vals id
	float fx = 0.f, fAdd = 1.f / size;  // screen x

	moLine->beginUpdate(0);
	moLine->position(fx, vals[i], 0.f);
	for (size_t n=0; n < vals.size(); ++n)
	{
		moLine->position(fx, vals[i], 0.f);
		//mo->colour(ColourValue(1,1,0));

		++i;  if (i >= vals.size())  i = 0;
		fx += fAdd;
	}
	moLine->end();
}

