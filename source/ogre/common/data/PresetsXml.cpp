#include "pch.h"
#include "../Def_Str.h"
#include "PresetsXml.h"
#include "tinyxml2.h"
using namespace std;
using namespace Ogre;
using namespace tinyxml2;


///  Presets

PSky::PSky()
	:mtr("sky"), clr("#C0E0FF")
	,ldYaw(0.f), ldPitch(0.f)
{	}

PTer::PTer()
	:tiling(8.f), triplanar(false)
	,dust(0.f), dustS(0.2f), mud(0.f)
	,tclr(0.2f, 0.2f, 0.1f, 0.6f)
	,angMin(0.f), angMax(90.f), dmg(0.f)
	,surfName("Default")
{	}

PRoad::PRoad()
	:dust(0.f), dustS(0.2f), mud(0.f)
	,tclr(0.2f, 0.2f, 0.1f, 0.6f)
{	}

PGrass::PGrass()
	:clr("GrassClrJungle")
	,minSx(1.2f), minSy(1.2f), maxSx(1.6f), maxSy(1.6f)
{	}

PVeget::PVeget()
	:minScale(0.6f), maxScale(1.f)
	,windFx(0.02f), windFy(0.002f)
	,addRdist(1)
	,maxTerAng(30.f)
	,maxDepth(0.f)
{	}


const PSky* Presets::GetSky(std::string mtr)
{
	int id = isky[mtr]-1;
	return id >= 0 ? &sky[id] : 0;
}

const PTer* Presets::GetTer(std::string tex)
{
	int id = iter[tex]-1;
	return id >= 0 ? &ter[id] : 0;
}

const PRoad* Presets::GetRoad(std::string mtr)
{
	int id = ird[mtr]-1;
	return id >= 0 ? &rd[id] : 0;
}

const PGrass* Presets::GetGrass(std::string mtr)
{
	int id = igr[mtr]-1;
	return id >= 0 ? &gr[id] : 0;
}

const PVeget* Presets::GetVeget(std::string mesh)
{
	int id = iveg[mesh]-1;
	return id >= 0 ? &veg[id] : 0;
}


///  Load Presets
//--------------------------------------------------------------------------------------------------------------------------------------

bool Presets::LoadXml(string file)
{
	XMLDocument doc;
	XMLError e = doc.LoadFile(file.c_str());
	if (e != XML_SUCCESS)
	{	LogO("!Can't load presets.xml: "+file);  return false;  }
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	ter.clear();  iter.clear();
	rd.clear();  ird.clear();
	gr.clear();  igr.clear();
	veg.clear();  iveg.clear();

	//  read
	XMLElement* eSky,*eTex,*eRd,*eVeg,*eGr;
	const char* a;

	///  sky
	eSky = root->FirstChildElement("s");
	while (eSky)
	{
		PSky s;
		a = eSky->Attribute("m");	if (a)  s.mtr = String(a);
		a = eSky->Attribute("c");	if (a)  s.clr = String(a);

		a = eSky->Attribute("y");	if (a)  s.ldYaw = s2r(a);
		a = eSky->Attribute("p");	if (a)  s.ldPitch = s2r(a);

		sky.push_back(s);  isky[s.mtr] = sky.size();
		eSky = eSky->NextSiblingElement("s");
	}

	///  terrain
	eTex = root->FirstChildElement("t");
	while (eTex)
	{
		PTer l;
		a = eTex->Attribute("t");	if (a)  l.texFile = String(a);
		a = eTex->Attribute("n");	if (a)  l.texNorm = String(a);
		a = eTex->Attribute("s");	if (a)  l.tiling = s2r(a);
		a = eTex->Attribute("su");	if (a)  l.surfName = string(a);

		a = eTex->Attribute("sc");	if (a)  l.sc = String(a);
		a = eTex->Attribute("z");	if (a)  l.scn = string(a);

		a = eTex->Attribute("du");	if (a)  l.dust = s2r(a);
		a = eTex->Attribute("ds");	if (a)  l.dustS = s2r(a);
		a = eTex->Attribute("md");	if (a)  l.mud = s2r(a);
		a = eTex->Attribute("tr");	if (a)  l.tclr.Load(a);
		a = eTex->Attribute("d");	if (a)  l.dmg = s2r(a);

		a = eTex->Attribute("aa");	if (a)  l.angMin = s2r(a);
		a = eTex->Attribute("ab");	if (a)  l.angMax = s2r(a);
		a = eTex->Attribute("tp");	if (a)  l.triplanar = s2i(a)>0;

		ter.push_back(l);  iter[l.texFile] = ter.size();
		eTex = eTex->NextSiblingElement("t");
	}

	///  road
	eRd = root->FirstChildElement("r");
	while (eRd)
	{
		PRoad l;
		a = eRd->Attribute("m");	if (a)  l.mtr = String(a);
		a = eRd->Attribute("su");	if (a)  l.surfName = string(a);

		a = eRd->Attribute("sc");	if (a)  l.sc = String(a);
		a = eRd->Attribute("z");	if (a)  l.scn = string(a);

		a = eRd->Attribute("du");	if (a)  l.dust = s2r(a);
		a = eRd->Attribute("ds");	if (a)  l.dustS = s2r(a);
		a = eRd->Attribute("md");	if (a)  l.mud = s2r(a);
		a = eRd->Attribute("tr");	if (a)  l.tclr.Load(a);

		rd.push_back(l);  ird[l.mtr] = rd.size();
		eRd = eRd->NextSiblingElement("r");
	}
		
	///  grass
	eGr = root->FirstChildElement("g");
	while (eGr)
	{
		PGrass g;
		a = eGr->Attribute("g");	if (a)  g.mtr = String(a);
		a = eGr->Attribute("c");	if (a)  g.clr = String(a);

		a = eGr->Attribute("sc");	if (a)  g.sc = String(a);
		a = eGr->Attribute("z");	if (a)  g.scn = string(a);

		a = eGr->Attribute("xa");	if (a)  g.minSx = s2r(a);
		a = eGr->Attribute("xb");	if (a)  g.maxSx = s2r(a);
		a = eGr->Attribute("ya");	if (a)  g.minSy = s2r(a);
		a = eGr->Attribute("yb");	if (a)  g.maxSy = s2r(a);

		gr.push_back(g);  igr[g.mtr] = gr.size();
		eGr = eGr->NextSiblingElement("g");
	}
	
	///  veget
 	eVeg = root->FirstChildElement("v");
	while (eVeg)
	{
		PVeget l;
		a = eVeg->Attribute("p");	if (a)  l.name = String(a);
		a = eVeg->Attribute("sa");	if (a)  l.minScale = s2r(a);
		a = eVeg->Attribute("sb");	if (a)  l.maxScale = s2r(a);

		a = eVeg->Attribute("sc");	if (a)  l.sc = String(a);
		a = eVeg->Attribute("z");	if (a)  l.scn = string(a);

		a = eVeg->Attribute("wx");	if (a)  l.windFx = s2r(a);
		a = eVeg->Attribute("wy");	if (a)  l.windFy = s2r(a);

		a = eVeg->Attribute("ab");	if (a)  l.maxTerAng = s2r(a);
		a = eVeg->Attribute("rd");	if (a)  l.addRdist = s2i(a);
		a = eVeg->Attribute("fd");	if (a)  l.maxDepth = s2r(a);

		veg.push_back(l);  iveg[l.name] = veg.size();
		eVeg = eVeg->NextSiblingElement("v");
	}

	return true;
}
