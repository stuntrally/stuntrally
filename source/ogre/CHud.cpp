#include "pch.h"
#include "common/Defines.h"
#include "CGame.h"
#include "CHud.h"
using namespace Ogre;


CHud::CHud(App* ap1, SETTINGS* pSet1)
	:ap(ap1), pSet(pSet1)

	,asp(1)//,  xcRpm(0), ycRpm(0), xcVel(0), ycVel(0)
	,scX(1),scY(1), minX(0),maxX(0), minY(0),maxY(0)

	,arrowNode(0),arrowRotNode(0)
	,ndLine(0)

	,txMsg(0), bckMsg(0)
	,txCamInfo(0)

	,txDbgCar(0),txDbgTxt(0),txDbgExt(0)

	,ovCarDbg(0),ovCarDbgTxt(0),ovCarDbgExt(0)
{
	hud.resize(4);  ov.resize(5);
	
	for (int i=0; i<4; ++i)
	{	ndTireVis[i]=0;  moTireVis[i]=0;  }
}


///  HUD utilities

ManualObject* CHud::Create2D(const String& mat, SceneManager* sceneMgr,
	Real s,  // scale pos
	bool dyn, bool clr,
	Real mul, Vector2 ofs,
	uint32 vis, uint8 rndQue, bool comb)
{
	ManualObject* m = sceneMgr->createManualObject();
	m->setDynamic(dyn);
	m->setUseIdentityProjection(true);
	m->setUseIdentityView(true);
	m->setCastShadows(false);

	m->estimateVertexCount(comb ? 8 : 4);
	m->begin(mat, comb ? RenderOperation::OT_TRIANGLE_LIST : RenderOperation::OT_TRIANGLE_STRIP);
	const static Vector2 uv[4] = { Vector2(0.f,1.f),Vector2(1.f,1.f),Vector2(0.f,0.f),Vector2(1.f,0.f) };
	int n = comb ? 2 : 1;
	for (int i=0; i < n; ++i)
	{	m->position(-s,-s*asp, 0);  m->textureCoord(uv[0]*mul + ofs);  if (clr)  m->colour(0,1,0);
		m->position( s,-s*asp, 0);  m->textureCoord(uv[1]*mul + ofs);  if (clr)  m->colour(0,0,0);
		m->position(-s, s*asp, 0);  m->textureCoord(uv[2]*mul + ofs);  if (clr)  m->colour(1,1,0);
		m->position( s, s*asp, 0);  m->textureCoord(uv[3]*mul + ofs);  if (clr)  m->colour(1,0,0);
	}
	if (comb)
	{	m->quad(0,1,3,2);
		m->quad(4,5,7,6);
	}
	m->end();
 
	AxisAlignedBox aabInf;	aabInf.setInfinite();
	m->setBoundingBox(aabInf);  // always visible
	m->setVisibilityFlags(vis);
	m->setRenderQueueGroup(rndQue);  //RQG_Hud2
	return m;
}


//  hud util
String CHud::GetTimeString(float time)
{
	int min = (int) time / 60;
	float secs = time - min*60;

	if (time != 0.f)
	{
		String ss;
		ss = toStr(min)+":"+fToStr(secs,2,5,'0');
		return ss;
	}else
		return "-:--.--";
}
String CHud::GetTimeShort(float time)
{
	int min = (int) time / 60;
	float secs = time - min*60;

	if (time != 0.0)
	{
		String ss;
		ss = toStr(min)+":"+fToStr(secs,0,2,'0');
		return ss;
	}else
		return "-:--";
}



//  HUD utils
//---------------------------------------------------------------------------------------------------------------
void CHud::UpdMiniTer()
{
	MaterialPtr mm = MaterialManager::getSingleton().getByName("circle_minimap");
	Pass* pass = mm->getTechnique(0)->getPass(0);
	if (!pass)  return;
	try
	{	Ogre::GpuProgramParametersSharedPtr par = pass->getFragmentProgramParameters();
		if (par->_findNamedConstantDefinition("showTerrain",false))
			par->setNamedConstant("showTerrain", ap->pSet->mini_terrain && ap->sc->ter ? 1.f : 0.f);
		if (par->_findNamedConstantDefinition("showBorder",false))
			par->setNamedConstant("showBorder", ap->pSet->mini_border && ap->sc->ter ? 1.f : 0.f);
		if (par->_findNamedConstantDefinition("square",false))
			par->setNamedConstant("square", ap->pSet->mini_zoomed && ap->sc->ter ? 0.f : 1.f);
	}
	catch(...){  }
}


Vector3 CHud::projectPoint(const Camera* cam, const Vector3& pos)
{
	Vector3 pos2D = cam->getProjectionMatrix() * (cam->getViewMatrix() * pos);

	//Real x = std::min(1.f, std::max(0.f,  pos2D.x * 0.5f + 0.5f ));  // leave on screen edges
	//Real y = std::min(1.f, std::max(0.f, -pos2D.y * 0.5f + 0.5f ));
	Real x =  pos2D.x * 0.5f + 0.5f;
	Real y = -pos2D.y * 0.5f + 0.5f;
	bool out = !cam->isVisible(pos);

	return Vector3(x * ap->mWindow->getWidth(), y * ap->mWindow->getHeight(), out ? -1.f : 1.f);
}

using namespace MyGUI;
TextBox* CHud::CreateNickText(int carId, String text)
{
	TextBox* txt = ap->mGUI->createWidget<TextBox>("TextBox",
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
	for (i=0;i<spacing;i++)	os << ".";
	os << "----------------------------------\n";
	for (i=0;i<spacing;i++)	os << ".";
	std::string s = "Profiling: "+String(pit->Get_Current_Parent_Name())+" (total running time: "+fToStr(parent_time,3)+" ms) ---\n";
	os << s;
	//float totalTime = 0.f;

	int numChildren = 0;
	
	for (i = 0; !pit->Is_Done(); i++,pit->Next())
	{
		numChildren++;
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
	for (i=0;i<spacing;i++)	os << ".";
	double unaccounted=  parent_time > SIMD_EPSILON ? ((parent_time - accumulated_time) / parent_time) * 100 : 0.f;
	s = "Unaccounted: ("+fToStr(unaccounted,3)+" %) :: "+fToStr(parent_time - accumulated_time,3)+" ms\n";
	os << s;
	
	for (i=0;i<numChildren;i++)
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
