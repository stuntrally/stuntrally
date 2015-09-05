#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "PaceNotes.h"
#ifdef SR_EDITOR
	#include "../editor/CApp.h"
	#include "../editor/settings.h"
#else
	#include "../ogre/CGame.h"
	#include "../vdrift/settings.h"
#endif
#include <OgreTimer.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreBillboard.h>
#include <OgreBillboardSet.h>
#include <MyGUI_Gui.h>
#include <MyGUI_TextBox.h>
#include "tinyxml.h"
#include "tinyxml2.h"
using namespace tinyxml2;
//#include <OgreTerrain.h>
using namespace std;
using namespace Ogre;


//  ctor  ---------
PaceNote::PaceNote()
	:nd(0), bb(0),bc(0)//, txt(0), text(0)
	,pos(0,0,0), use(1), id(0)
	,size(4.f,4.f), clr(0,0,0,0), ofs(0,0), uv(0,0)
	,start(0), jump(0), vel(0.f)
{	}
PaceNote::PaceNote(int i, int t, Vector3 p,  //id,use, pos
		float sx,float sy,  float r,float g,float b,float a,  //size, clr
		float ox,float oy, float u,float v)  //ofs:dir,bar width, tex uv
	:nd(0), bb(0),bc(0)//, txt(0), text(0)
	,pos(p), use(t), id(i)
	,size(sx,sy), clr(r,g,b,a), ofs(ox,oy), uv(u,v)
	,start(0), jump(0), vel(0.f)
{	}


//  Create
void PaceNotes::Create(PaceNote& n)
{
	//if (n.use == 1)
	//	LogO("PP "+toStr(n.id)+(n.start?" ST":""));
	++ii;
	n.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	n.bb = mSceneMgr->createBillboardSet("P-"+toStr(ii),2);
	n.bb->setDefaultDimensions(n.size.x, n.size.y);

	n.bb->setRenderQueueGroup(RQG_CarParticles);
	n.bb->setVisibilityFlags(RV_Car);

	n.bb->setCustomParameter(0, Vector4(n.ofs.x, n.ofs.y, n.uv.x, n.uv.y));  // params, uv ofs
	n.bc = n.bb->createBillboard(Vector3(0,0,0), ColourValue(n.clr.x, n.clr.y, n.clr.z, n.clr.w));

	n.bb->setMaterialName("pacenote");
	n.nd->attachObject(n.bb);
	n.nd->setPosition(n.pos);
	n.nd->setVisible(false);
	//if (n.text)
	//{
	//	n.txt = mGui->createWidget
	//}
}

void PaceNotes::Update(PaceNote& n)
{
	n.bc->setColour(ColourValue(n.clr.x, n.clr.y, n.clr.z, n.clr.w));
	n.bb->setCustomParameter(0, Vector4(n.ofs.x, n.ofs.y, n.uv.x, n.uv.y));  // params, uv ofs
}

//  Destroy
void PaceNotes::Destroy(PaceNote& n)
{
	//if (n.txt){  mGui->destroyWidget(n.txt);  n.txt = 0;  }
	mSceneMgr->destroyBillboardSet(n.bb);  n.bb = 0;
	mSceneMgr->destroySceneNode(n.nd);  n.nd = 0;
}

//  all
void PaceNotes::Destroy()
{
	for (size_t i=0; i < vPN.size(); ++i)
		Destroy(vPN[i]);
	vPN.clear();
	vPS.clear();
	ii = 0;
}


//  update visibility  ---------
void PaceNotes::UpdVis(Vector3 carPos, bool hide)
{
	const Real dd = pApp->pSet->pace_dist, dd2 = dd*dd;

	const Vector3& c = mCamera->getPosition();
	int i,s;

	//static int xx=0;  ++xx;  // inc test
	//if (xx > 10) {  xx=0;  iCur++;  if (iCur>=iAll)  iCur-=iAll;  }
	//LogO(toStr(iCur));

	///todo: cd. jump vel, loop side-, onpipe key8
	///  strict jfw not: hid, onpipe, under ter
	///  reset car pos iCur, prev chk 0
	
#ifndef SR_EDITOR
	//  game  ----
	const int rng = pApp->pSet->pace_next;  // vis next count
	const float radiusA = 9.f*9.f;  //par pace sphere radius

	s = vPS.size();
	for (i=0; i < s; ++i)
	{
		PaceNote& p = vPS[i];
		bool vis = pApp->pSet->pace_show;

		//  Advance to next sign  ~ ~ ~
		//        a    iCur   s-1  s=7
		//  0  1  2  3  4  5  6   -id
		bool vrng = iDir > 0 ?  // inside cur..cur+range with cycle
			(i >= iCur && i <= iCur+rng || i < iCur+rng-iAll) :
			(i <= iCur && i >= iCur-rng || i > iCur-rng+iAll);
		vis &= vrng;

		if (vrng)
		{	float d = p.pos.squaredDistance(carPos);
			if (d < radiusA && i != iCur)  // close next only
			{
				//LogO("iCur "+iToStr(i,3)+"  d "+fToStr(sqrt(d))+"  <> "+iToStr(i-iCur));
				iCur = i;
		}	}
		//if (vis && p.nd->txt)  updTxt(p);
		p.nd->setVisible(vis);
	}
#else  //  ed  ----
	s = vPN.size();
	if (hide)
	{	for (i=0; i < s; ++i)
			vPN[i].nd->setVisible(false);
		return;
	}
	for (i=0; i < s; ++i)
	{
		PaceNote& p = vPN[i];
		bool vis = p.use <= pApp->pSet->pace_show;

		const Vector3& o = p.pos;
		Real dist = c.squaredDistance(o);
		bool vnear = dist < dd2;
		vis &= vnear;

		p.nd->setVisible(vis);
	}
#endif
}


//  ctor  ---------
PaceNotes::PaceNotes(App* papp) :pApp(papp)
	,mSceneMgr(0),mCamera(0),mTerrain(0)
	,ii(0), iStart(0),iAll(1), iDir(1), iCur(0)
{	}

//  setup
void PaceNotes::Setup(SceneManager* sceneMgr, Camera* camera,
	Terrain* terrain, MyGUI::Gui* gui)
{
	mSceneMgr = sceneMgr;  mCamera = camera;
	mTerrain = terrain;  //mGui = gui;
}


///  Load
//------------------------------------------------------------------------------------------
bool PaceNotes::LoadFile(String fname)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(fname.c_str());
	if (e != XML_SUCCESS)  return false;
		
	XMLElement* root = doc.RootElement(), *n = NULL;
	if (!root)  return false;
	const char* a = NULL;
	
	vPN.clear();

	n = root->FirstChildElement("gen");	if (n)  {
		//a = n->Attribute("len");	if (a)  g_LenDim0 = s2r(a);
	}

	n = root->FirstChildElement("P");	//  points
	while (n)
	{
		PaceNote p;
		a = n->Attribute("p");  p.pos = s2v(a);
		a = n->Attribute("c");  p.clr = s2v4(a);
		a = n->Attribute("s");  p.size = s2v2(a);
		a = n->Attribute("o");  p.ofs = s2v2(a);
		a = n->Attribute("u");  p.uv = s2v2(a);

		//  Add point
		vPN.push_back(p);
		n = n->NextSiblingElement("P");
	}
	return true;
}

///  Save
//------------------------------------------------------------------------------------------
bool PaceNotes::SaveFile(String fname)
{
	TiXmlDocument xml;	TiXmlElement root("Pacenotes");
	
	int i, num = vPN.size();
	for (i=0; i < num; ++i)
	{
		TiXmlElement n("P");
		{
			const PaceNote& p = vPN[i];

			n.SetAttribute("p",	toStrC( p.pos ));
			n.SetAttribute("c",	toStrC( p.clr ));
			n.SetAttribute("s",	toStrC( p.size ));
			n.SetAttribute("o",	toStrC( p.ofs ));
			n.SetAttribute("u",	toStrC( p.uv ));
		}
		root.InsertEndChild(n);
	}
	
	xml.InsertEndChild(root);
	return xml.SaveFile(fname.c_str());
}
