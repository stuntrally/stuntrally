#include "pch.h"
#include "GraphView.h"
#include "RenderConst.h"
#include "Defines.h"

#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <MyGUI_Gui.h>
#include <MyGUI_TextBox.h>
using namespace Ogre;
using namespace MyGUI;


//  ctor
GraphView::GraphView(SceneManager* pSceneMgr, RenderWindow* pWindow, MyGUI::Gui* pGui)
	:mSceneMgr(pSceneMgr), mWindow(pWindow), mGui(pGui)
	,moLine(0),moBack(0),moGrid(0), node(0), iCurX(0)
	,txt(0), txPosX(0.f), txH(0), txAlignY(-2)
{  }

//  same as in graph1..5 materials
const Colour GraphView::graphClr[5+8+8] = {
	Colour(0.0, 1.0, 1.0),
	Colour(0.0, 1.0, 0.0),
	Colour(1.0, 1.0, 0.0),
	Colour(1.0, 0.5, 0.0),
	Colour(1.0, 0.0, 0.0),

	Colour(1.0, 1.0, 1.0),
	Colour(0.8, 1.0, 1.0),
	Colour(0.6, 1.0, 1.0),
	Colour(0.4, 1.0, 1.0),
	Colour(0.2, 0.8, 1.0),
	Colour(0.2, 0.6, 1.0),
	Colour(0.2, 0.4, 1.0),
	Colour(0.2, 0.2, 1.0),

	Colour(1.0, 1.0, 0.9),
	Colour(1.0, 1.0, 0.6),
	Colour(1.0, 1.0, 0.3),
	Colour(1.0, 1.0, 0.0),
	Colour(1.0, 0.8, 0.0),
	Colour(1.0, 0.6, 0.0),
	Colour(1.0, 0.4, 0.0),
	Colour(1.0, 0.2, 0.0) };


//  Create
//----------------------------------------------------------------------------------------
void GraphView::moSetup(ManualObject* mo, bool dynamic, Ogre::uint8 RQG)
{
	mo->setDynamic(dynamic);
	mo->setUseIdentityProjection(true);
	mo->setUseIdentityView(true);
	mo->setCastShadows(false);

	AxisAlignedBox aabInf;	aabInf.setInfinite();
	mo->setBoundingBox(aabInf);  // always visible
	mo->setRenderQueueGroup(RQG);  // on hud
	mo->setVisibilityFlags(RV_Hud);
}

void GraphView::Create(int length, String sMtr, float backAlpha)
{
	vals.resize(length);  iCurX = 0;  //size-1
	node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	
	//  graph line  ----------------------
	if (length > 1)  // at least 2, use 1 for text only
	{
		moLine = mSceneMgr->createManualObject();
		moSetup(moLine, true, RQG_Hud3);
		moLine->estimateVertexCount(length);

		moLine->begin(sMtr, RenderOperation::OT_LINE_STRIP);
		float s = 0.5f, asp = 1.f;
		moLine->position(0,0, 0);
		moLine->position(1,0, 0);
		moLine->end();
		node->attachObject(moLine);
	}

	//  backgr rect  ----------------------
	if (backAlpha > 0.01f)
	{
		moBack = mSceneMgr->createManualObject();
		moSetup(moBack, false, RQG_Hud1);
		moBack->estimateVertexCount(4);

		moBack->begin("graphBack", RenderOperation::OT_TRIANGLE_STRIP);
		ColourValue c(0,0,0, backAlpha);
		moBack->position(0,0, 0);  moBack->colour(c);
		moBack->position(1,0, 0);  moBack->colour(c);
		moBack->position(0,1, 0);  moBack->colour(c);
		moBack->position(1,1, 0);  moBack->colour(c);
		moBack->end();
		node->attachObject(moBack);
	}
}

//  Grid lines  == ||
//----------------------------------------------------------------------------------------
void GraphView::CreateGrid(int numH, int numV, /*char clr,*/ float clr, float alpha)
{
	if (!node)  return;
	if (numH <= 0 && numV <= 0)  return;
	
	moGrid = mSceneMgr->createManualObject();
	moSetup(moGrid, false, RQG_Hud2);
	//moGrid->estimateVertexCount((numH+numV)*2);

	//const Colour& gc = graphClr[clr];
	//ColourValue c(gc.red, gc.green, gc.blue, alpha);
	ColourValue c(clr, clr, clr, alpha);

	moGrid->begin("graphGrid", RenderOperation::OT_LINE_LIST);

	if (numH > 0)
	{
		float ya = 1.f / numH;
		//for (float y = ya; y <= 1.f-ya; y += ya)
		for (float y = 0.f; y <= 1.f; y += ya)
		{
			moGrid->position(0,y, 0);  moGrid->colour(c);
			moGrid->position(1,y, 0);  moGrid->colour(c);
		}
	}
	if (numV > 0)
	{
		float xa = 1.f / numV;
		//for (float x = xa; x <= 1.f-xa; y += xa)
		for (float x = 0.f; x <= 1.f; x += xa)
		{
			moGrid->position(x,0, 0);  moGrid->colour(c);
			moGrid->position(x,1, 0);  moGrid->colour(c);
		}
	}
	moGrid->end();
	node->attachObject(moGrid);
}

// todo: auto val range ?
// gui tab [big]= edit pos,size,value,text,range,etc. save in graphs.xml


//  Create title text
//----------------------------------------------------------------------------------------
void GraphView::CreateTitle(String title, char clr, float posX, char alignY, int fontHeight, int numLines, bool shadow)
{
	if (!mGui)  return;
	static int cntr = 0;  ++cntr;

	txPosX = posX;  txH = fontHeight;  txAlignY = alignY;

	txt = mGui->createWidget<TextBox>("TextBox",
		100,100, 360,txH*numLines, Align::Center, "Back", "GrTx"+toStr(cntr));

	if (shadow)
	{	txt->setTextShadow(true);
		txt->setTextShadowColour(Colour::Black);  }

	txt->setFontHeight(fontHeight);
	txt->setTextColour(graphClr[clr]);
	txt->setCaption(title);
	txt->setVisible(true);
}

void GraphView::UpdTitle(String title)
{
	if (!txt)  return;
	txt->setCaption(title);
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
	if (moGrid) {	mSceneMgr->destroyManualObject(moGrid);  moGrid = 0;  }
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
	if (!node || !moLine)  return;
	
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

