#include "pch.h"
#include "common/Def_Str.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "common/data/SceneXml.h"
#include "common/CScene.h"
#include "../vdrift/settings.h"
#include <LinearMath/btQuickprof.h>
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreRenderWindow.h>
#include <OgreCamera.h>
#include <OgreOverlayElement.h>
#include <OgreTechnique.h>
#include <MyGUI_TextBox.h>
using namespace Ogre;


CHud::CHud(App* ap1)
	:app(ap1)
	
	,asp(1)//,  xcRpm(0), ycRpm(0), xcVel(0), ycVel(0)
	,scX(1),scY(1), minX(0),maxX(0), minY(0),maxY(0)
	,ndLine(0)

	,moPos(0) ,ndPos(0)
	
	,txMsg(0), bckMsg(0)
	,txCamInfo(0)

	,txDbgCar(0),txDbgTxt(0),txDbgExt(0)

	,ovCarDbg(0),ovCarDbgTxt(0),ovCarDbgExt(0)
{
	hud.resize(4);  ov.resize(5);
	
	for (int i=0; i<4; ++i)
	{	ndTireVis[i]=0;  moTireVis[i]=0;  }
	
	pSet = ap1->pSet;
	gui = ap1->gui;
}

CHud::Arrow::Arrow()
	:ent(0),node(0),nodeRot(0)
{	}


///  HUD utilities

ManualObject* CHud::Create2D(const String& mat, SceneManager* sceneMgr,
	Real s,  // scale pos
	bool dyn, bool clr,
	Real mul, Vector2 ofs,
	uint32 vis, uint8 rndQue,
	int cnt)
{
	ManualObject* m = sceneMgr->createManualObject();
	m->setDynamic(dyn);
	m->setUseIdentityProjection(true);
	m->setUseIdentityView(true);
	m->setCastShadows(false);

	m->estimateVertexCount(cnt * 4);
	m->begin(mat, cnt > 1 ? RenderOperation::OT_TRIANGLE_LIST : RenderOperation::OT_TRIANGLE_STRIP);
	const static Vector2 uv[4] = { Vector2(0.f,1.f),Vector2(1.f,1.f),Vector2(0.f,0.f),Vector2(1.f,0.f) };

	for (int i=0; i < cnt; ++i)
	{	m->position(-s,-s*asp, 0);  m->textureCoord(uv[0]*mul + ofs);  if (clr)  m->colour(0,1,0);
		m->position( s,-s*asp, 0);  m->textureCoord(uv[1]*mul + ofs);  if (clr)  m->colour(0,0,0);
		m->position(-s, s*asp, 0);  m->textureCoord(uv[2]*mul + ofs);  if (clr)  m->colour(1,1,0);
		m->position( s, s*asp, 0);  m->textureCoord(uv[3]*mul + ofs);  if (clr)  m->colour(1,0,0);
	}
	if (cnt > 1)
	for (int i=0; i < cnt; ++i)
	{	int n = i*4;
		m->quad(n,n+1,n+3,n+2);
	}
	m->end();
 
	AxisAlignedBox aabInf;	aabInf.setInfinite();
	m->setBoundingBox(aabInf);  // always visible
	m->setVisibilityFlags(vis);
	m->setRenderQueueGroup(rndQue);  //RQG_Hud2
	return m;
}


//  HUD utils
//---------------------------------------------------------------------------------------------------------------
void CHud::UpdMiniTer()
{
	MaterialPtr mm = MaterialManager::getSingleton().getByName("circle_minimap");
	Pass* pass = mm->getTechnique(0)->getPass(0);
	if (!pass)  return;
	try
	{	GpuProgramParametersSharedPtr par = pass->getFragmentProgramParameters();
		bool ter = app->scn->sc->ter;
		if (par->_findNamedConstantDefinition("showTerrain",false))
			par->setNamedConstant("showTerrain", pSet->mini_terrain && ter ? 1.f : 0.f);
		if (par->_findNamedConstantDefinition("showBorder",false))
			par->setNamedConstant("showBorder", pSet->mini_border && ter ? 1.f : 0.f);
		if (par->_findNamedConstantDefinition("square",false))
			par->setNamedConstant("square", pSet->mini_zoomed && ter ? 0.f : 1.f);
	}
	catch(...){  }
}


Vector3 CHud::projectPoint(const Camera* cam, const Vector3& pos)
{
	Vector3 pos2D = cam->getProjectionMatrix() * (cam->getViewMatrix() * pos);

	//std::min(1.f, std::max(0.f,  ));  // leave on screen edges
	Real x =  pos2D.x * 0.5f + 0.5f;
	Real y = -pos2D.y * 0.5f + 0.5f;
	bool out = !cam->isVisible(pos);

	return Vector3(x * app->mWindow->getWidth(), y * app->mWindow->getHeight(), out ? -1.f : 1.f);
}

using namespace MyGUI;
TextBox* CHud::CreateNickText(int carId, String text)
{
	TextBox* txt = app->mGui->createWidget<TextBox>("TextBox",
		100,100, 360,32, Align::Center, "Back", "NickTxt"+toStr(carId));
	txt->setVisible(false);
	txt->setFontHeight(28);  //par 24..32
	txt->setTextShadow(true);  txt->setTextShadowColour(Colour::Black);
	txt->setCaption(text);
	return txt;
}

//  get color as text eg. #C0E0FF
String CHud::StrClr(ColourValue c)
{
	char hex[16];
	sprintf(hex, "#%02x%02x%02x", int(c.r * 255.f), int(c.g * 255.f), int(c.b * 255.f));
	return String(hex);
}

void CHud::UpdDbgTxtClr()
{
	ColourValue c = pSet->car_dbgtxtclr ? ColourValue::Black : ColourValue::White;
	for (int i=0; i < ov.size(); ++i)
	{
		if (ov[i].oU)  ov[i].oU->setColour(c);
		if (ov[i].oX)  ov[i].oX->setColour(c);
	}
}


///  Bullet profiling text
//--------------------------------------------------------------------------------------------------------------

void CHud::bltDumpRecursive(CProfileIterator* pit, int spacing, std::stringstream& os)
{
	pit->First();
	if (pit->Is_Done())
		return;

	float accumulated_time=0,parent_time = pit->Is_Root() ? CProfileManager::Get_Time_Since_Reset() : pit->Get_Current_Parent_Total_Time();
	int i,j;
	int frames_since_reset = CProfileManager::Get_Frame_Count_Since_Reset();
	for (i=0; i<spacing; ++i)  os << ".";
	os << "----------------------------------\n";
	for (i=0; i<spacing; ++i)  os << ".";
	std::string s = "Profiling: "+String(pit->Get_Current_Parent_Name())+" (total running time: "+fToStr(parent_time,3)+" ms) ---\n";
	os << s;
	//float totalTime = 0.f;

	int numChildren = 0;
	
	for (i = 0; !pit->Is_Done(); ++i,pit->Next())
	{
		++numChildren;
		float current_total_time = pit->Get_Current_Total_Time();
		accumulated_time += current_total_time;
		float fraction = parent_time > SIMD_EPSILON ? (current_total_time / parent_time) * 100 : 0.f;

		for (j=0;j<spacing;j++)	os << ".";
		double ms = (current_total_time / (double)frames_since_reset);
		s = toStr(i)+" -- "+pit->Get_Current_Name()+" ("+fToStr(fraction,2)+" %) :: "+fToStr(ms,3)+" ms / frame ("+toStr(pit->Get_Current_Total_Calls())+" calls)\n";
		os << s;
		//totalTime += current_total_time;
		//recurse into children
	}

	if (parent_time < accumulated_time)
	{
		os << "what's wrong\n";
	}
	for (i=0; i<spacing; ++i)  os << ".";
	double unaccounted=  parent_time > SIMD_EPSILON ? ((parent_time - accumulated_time) / parent_time) * 100 : 0.f;
	s = "Unaccounted: ("+fToStr(unaccounted,3)+" %) :: "+fToStr(parent_time - accumulated_time,3)+" ms\n";
	os << s;
	
	for (i=0; i<numChildren; ++i)
	{
		pit->Enter_Child(i);
		bltDumpRecursive(pit, spacing+3, os);
		pit->Enter_Parent();
	}
}

void CHud::bltDumpAll(std::stringstream& os)
{
	CProfileIterator* profileIterator = 0;
	profileIterator = CProfileManager::Get_Iterator();

	bltDumpRecursive(profileIterator, 0, os);

	CProfileManager::Release_Iterator(profileIterator);
}
