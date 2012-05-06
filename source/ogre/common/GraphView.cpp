#include "pch.h"
#include "GraphView.h"
#include "RenderConst.h"
#include "Defines.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <MyGUI_Gui.h>
#include <MyGUI_TextBox.h>
using namespace Ogre;
using namespace MyGUI;


//  ctor
GraphView::GraphView(SceneManager* pSceneMgr, RenderWindow* pWindow, MyGUI::Gui* pGui)
	:mSceneMgr(pSceneMgr), mWindow(pWindow), mGui(pGui)
	,moLine(0),moBack(0), node(0), iCurX(0)
	,txt(0), txPosX(0.f), txH(0), txAlignY(-2)
{  }

//  same as in graph1..5 materials
const Colour GraphView::graphClr[5] = {
	Colour(0.0 ,1.0, 1.0),
	Colour(0.0, 1.0, 0.0),
	Colour(1.0, 1.0, 0.0),
	Colour(1.0, 0.5, 0.0),
	Colour(1.0, 0.0, 0.0)};


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
}

//  grid lines  ----------------------
//void GraphView::CreateGrid(int numH, int numV, r,g,b,a
//for () ..  // == ||?

// todo: ..
// auto val range ?
// gui tab [big]= edit pos,size,value,text,range,etc. save in graphs.xml


//  Create title text
//----------------------------------------------------------------------------------------
void GraphView::CreateTitle(String title, char clr, float posX, char alignY, int fontHeight, bool shadow)
{
	if (!mGui)  return;
	static int cntr = 0;  ++cntr;

	txt = mGui->createWidget<TextBox>("TextBox",
		100,100, 360,32, Align::Center, "Back", "GrTx"+toStr(cntr));

	if (shadow)
	{	txt->setTextShadow(true);
		txt->setTextShadowColour(Colour::Black);  }

	txt->setFontHeight(fontHeight);
	txt->setTextColour(graphClr[clr]);
	txt->setCaption(title);
	txt->setVisible(true);

	txPosX = posX;  txH = fontHeight;  txAlignY = alignY;
}

//  Set Size
//----------------------------------------------------------------------------------------
void GraphView::SetSize(float posX,float posY,float sizeX,float sizeY)  // [0..1]  0,0 is left bottom
{
	if (!node)  return;
	float px = -1.f+posX*2.f, py = -1.f+posY*2.f;
	node->setPosition(px, py, 0.f);
	node->setScale(sizeX*2.f, sizeY*2.f, 1.f);

	//  set title text pos
	if (!txt || !mWindow)  return;
	int wx = mWindow->getWidth(), wy = mWindow->getHeight();
	int x = (posX + txPosX*sizeX) * wx;  float pszY = posY + sizeY;
	switch (txAlignY)
	{
		case -1:  txt->setPosition(x, wy - pszY * wy - txH );  break;  // above
		case -2:  txt->setPosition(x, wy - pszY * wy       );  break;  // 1 inside  top
		case -3:  txt->setPosition(x, wy - pszY * wy + txH );  break;  // 2 below 1
		case  0:  txt->setPosition(x, wy - pszY * wy       );  break;  // center
		case  3:  txt->setPosition(x, wy - posY * wy-2*txH );  break;  // 2 above 1
		case  2:  txt->setPosition(x, wy - posY * wy - txH );  break;  // 1 inside  bottom
		case  1:  txt->setPosition(x, wy - posY * wy       );  break;  // below
	}
}
//----------------------------------------------------------------------------------------

//  Destroy
void GraphView::Destroy()
{
	if (mGui && txt)  {  mGui->destroyWidget(txt);  txt = 0;  }
	if (moLine) {	mSceneMgr->destroyManualObject(moLine);  moLine = 0;  }
	if (moBack) {	mSceneMgr->destroyManualObject(moBack);  moBack = 0;  }
	if (node) {		mSceneMgr->destroySceneNode(node);  node = 0;  }
}

//  show/hide
void GraphView::SetVisible(bool visible)
{
	if (node)  node->setVisible(visible);
	if (txt)   txt->setVisible(visible);
}


//  Add value  (into buffer)
//------------------------------------------------------------------
void GraphView::AddVal(float val)
{
	//if (!node)  return;
	if (iCurX >= vals.size())  return;

	//assert vals empty
	vals[iCurX] = val;
	++iCurX;  if (iCurX >= vals.size())  iCurX = 0;
}

//  Update  (on screen)
//------------------------------------------------------------------
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

