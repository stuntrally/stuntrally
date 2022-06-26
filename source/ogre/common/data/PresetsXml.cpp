#include "pch.h"
#include "../Def_Str.h"
#include "PresetsXml.h"
#include "tinyxml2.h"
using namespace std;
using namespace Ogre;
using namespace tinyxml2;


///  Get Presets

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
	XMLError er = doc.LoadFile(file.c_str());
	if (er != XML_SUCCESS)
	{	LogO("!Can't load presets.xml: "+file);  return false;  }
		
	XMLElement* root = doc.RootElement();
	if (!root)  return false;

	//  clear
	ter.clear();  iter.clear();
	rd.clear();  ird.clear();
	gr.clear();  igr.clear();
	veg.clear();  iveg.clear();

	//  read
	XMLElement* e;
	const char* a;

	///  sky
	e = root->FirstChildElement("s");
	while (e)
	{
		PSky s;
		a = e->Attribute("r");	if (a)  s.rate = s2i(a);
		a = e->Attribute("m");	if (a)  s.mtr = String(a);
		a = e->Attribute("c");	if (a)  s.clr = String(a);

		a = e->Attribute("y");	if (a)  s.ldYaw = s2r(a);
		a = e->Attribute("p");	if (a)  s.ldPitch = s2r(a);

		sky.push_back(s);  isky[s.mtr] = sky.size();
		e = e->NextSiblingElement("s");
	}

	///  terrain
	e = root->FirstChildElement("t");
	while (e)
	{
		PTer l;
		a = e->Attribute("r");	if (a)  l.rate = s2i(a);
		a = e->Attribute("t");	if (a)  l.texFile = String(a);
		a = e->Attribute("n");	if (a)  l.texNorm = String(a);
		a = e->Attribute("s");	if (a)  l.tiling = s2r(a);
		a = e->Attribute("su");	if (a)  l.surfName = string(a);

		a = e->Attribute("sc");	if (a)  l.sc = String(a);
		a = e->Attribute("z");	if (a)  l.scn = string(a);

		a = e->Attribute("du");	if (a)  l.dust = s2r(a);
		a = e->Attribute("ds");	if (a)  l.dustS = s2r(a);
		a = e->Attribute("md");	if (a)  l.mud = s2r(a);
		a = e->Attribute("tr");	if (a)  l.tclr.Load(a);
		a = e->Attribute("d");	if (a)  l.dmg = s2r(a);

		a = e->Attribute("aa");	if (a)  l.angMin = s2r(a);
		a = e->Attribute("ab");	if (a)  l.angMax = s2r(a);
		a = e->Attribute("tp");	if (a)  l.triplanar = s2i(a)>0;

		ter.push_back(l);  iter[l.texFile] = ter.size();
		e = e->NextSiblingElement("t");
	}

	///  road
	e = root->FirstChildElement("r");
	while (e)
	{
		PRoad l;
		a = e->Attribute("r");	if (a)  l.rate = s2i(a);
		a = e->Attribute("m");	if (a)  l.mtr = String(a);
		a = e->Attribute("su");	if (a)  l.surfName = string(a);

		a = e->Attribute("sc");	if (a)  l.sc = String(a);
		a = e->Attribute("z");	if (a)  l.scn = string(a);

		a = e->Attribute("du");	if (a)  l.dust = s2r(a);
		a = e->Attribute("ds");	if (a)  l.dustS = s2r(a);
		a = e->Attribute("md");	if (a)  l.mud = s2r(a);
		a = e->Attribute("tr");	if (a)  l.tclr.Load(a);

		rd.push_back(l);  ird[l.mtr] = rd.size();
		e = e->NextSiblingElement("r");
	}
		
	///  ass
	e = root->FirstChildElement("g");
	while (e)
	{
		PGrass g;
		a = e->Attribute("r");	if (a)  g.rate = s2i(a);
		a = e->Attribute("g");	if (a)  g.mtr = String(a);
		a = e->Attribute("c");	if (a)  g.clr = String(a);

		a = e->Attribute("sc");	if (a)  g.sc = String(a);
		a = e->Attribute("z");	if (a)  g.scn = string(a);

		a = e->Attribute("xa");	if (a)  g.minSx = s2r(a);
		a = e->Attribute("xb");	if (a)  g.maxSx = s2r(a);
		a = e->Attribute("ya");	if (a)  g.minSy = s2r(a);
		a = e->Attribute("yb");	if (a)  g.maxSy = s2r(a);

		gr.push_back(g);  igr[g.mtr] = gr.size();
		e = e->NextSiblingElement("g");
	}
	
	///  veget
 	e = root->FirstChildElement("v");
	while (e)
	{
		PVeget l;
		a = e->Attribute("r");	if (a)  l.rate = s2i(a);
		a = e->Attribute("p");	if (a)  l.name = String(a);

		a = e->Attribute("sc");	if (a)  l.sc = String(a);
		a = e->Attribute("z");	if (a)  l.scn = string(a);

		a = e->Attribute("sa");	if (a)  l.minScale = s2r(a);
		a = e->Attribute("sb");	if (a)  l.maxScale = s2r(a);

		a = e->Attribute("wx");	if (a)  l.windFx = s2r(a);
		a = e->Attribute("wy");	if (a)  l.windFy = s2r(a);

		a = e->Attribute("ab");	if (a)  l.maxTerAng = s2r(a);
		a = e->Attribute("rd");	if (a)  l.addRdist = s2i(a);
		a = e->Attribute("fd");	if (a)  l.maxDepth = s2r(a);

		veg.push_back(l);  iveg[l.name] = veg.size();
		e = e->NextSiblingElement("v");
	}

	return true;
}
